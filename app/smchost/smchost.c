/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
#include "board.h"
#include "board_config.h"
#include "smc.h"
#include "smchost.h"
#include "smchost_commands.h"
#include "scicodes.h"
#include "sci.h"
#include "acpi.h"
#include "pwrplane.h"
#include "pwrbtnmgmt.h"
#include "periphmgmt.h"
#include "espi_hub.h"
#ifdef CONFIG_THERMAL_MANAGEMENT
#include "thermalmgmt.h"
#endif
LOG_MODULE_REGISTER(smchost, CONFIG_SMCHOST_LOG_LEVEL);

u8_t host_req[SMCHOST_MAX_BUF_SIZE];
u8_t host_res[SMCHOST_MAX_BUF_SIZE];
u8_t host_req_len;
u8_t host_res_len;
u8_t host_res_idx;

struct acpi_tbl g_acpi_tbl;

static void proc_host_send(void);
static void proc_acpi_burst(void);
static void service_system_acpi_cmds(void);
static u8_t smchost_req_length(u8_t command);
static void smchost_cmd_handler(u8_t command);


static u8_t acpi_burst_flag;

void smchost_err_handler(void)
{
	if (acpi_get_flag(ACPI_EC_0, ACPI_FLAG_LRST)) {
		acpi_set_flag(ACPI_EC_0, ACPI_FLAG_LRST, 0);
	}

	if (acpi_get_flag(ACPI_EC_0, ACPI_FLAG_ABRT)) {
		acpi_set_flag(ACPI_EC_0, ACPI_FLAG_ABRT, 0);
	}

	if (acpi_get_flag(ACPI_EC_0, ACPI_FLAG_SWDN)) {
		acpi_set_flag(ACPI_EC_0, ACPI_FLAG_ABRT, 0);
	}
}

static void smchost_acpi_handler(void)
{
	while (acpi_get_flag(ACPI_EC_0, ACPI_FLAG_IBF)) {
		if (acpi_get_flag(ACPI_EC_0, ACPI_FLAG_CD)) {
			/* It is a command */
			host_req_len = 0;

			/* Read the command byte */
			host_req[host_req_len] = acpi_read_idr(ACPI_EC_0);
			LOG_INF("Rcv EC cmd: %02X",
				host_req[host_req_len]);

#ifdef CONFIG_POWER_MANAGEMENT
			/* Exit deep idle if burst mode is requested */
			if (host_req[host_req_len] == EC_BURST) {
				exit_deep_idle();
			}
#endif
		} else {
			/* It is data */
			if (host_req[0] == EC_WRITE) {
				generate_sci();
			}

			if (host_req_len < SMCHOST_MAX_BUF_SIZE) {
				/* Read the data byte */
				host_req[host_req_len] =
				       acpi_read_idr(ACPI_EC_0);
				LOG_INF("Host Rcvdata[%d] = %02X",
					host_req_len, host_req[host_req_len]);
			} else {
				LOG_WRN("Exceeds Rcvdata buf size! Ignored");
				return;
			}
		}

		/* When a command is received check if the command corresponds
		 * to a ACPI read/write operation then ackwnowledge the OS
		 * before performing the operation.
		 */
		if (host_req[0]) {
			if ((host_req[0] == EC_READ) ||
			    (host_req[0] == EC_WRITE)) {
				generate_sci();
			}

			if (smchost_req_length(host_req[0]) == host_req_len) {
				LOG_INF("EC Command: %02X", host_req[0]);
				smchost_cmd_handler(host_req[0]);
				host_req[0] = 0;
			}
		}

		host_req_len++;
	}
}


static void smchost_volbtnup_handler(u8_t volbtn_sts)
{
	LOG_DBG("%s", __func__);

	if (g_acpi_state_flags.acpi_mode) {
		if (volbtn_sts) {
			enqueue_sci(SCI_VU_REL);
		} else {
			enqueue_sci(SCI_VU_PRES);
		}
	}
}

static void smchost_volbtndown_handler(u8_t volbtn_sts)
{
	LOG_DBG("%s", __func__);

	if (g_acpi_state_flags.acpi_mode) {
		if (volbtn_sts) {
			enqueue_sci(SCI_VD_REL);
		} else {
			enqueue_sci(SCI_VD_PRES);
		}
	}
}

static void smchost_pltrst_handler(u8_t pltrst_sts)
{
	LOG_DBG("PLT_RST status changed %d", pltrst_sts);
	g_acpi_state_flags.sci_enabled = pltrst_sts;
	LOG_DBG("SCI enabled %d", g_acpi_state_flags.sci_enabled);

#ifdef CONFIG_THERMAL_MANAGEMENT
	if (pltrst_sts) {
		peci_start_delay_timer();
	}
#endif
}

static void smchost_host_rst_warn_handler(u8_t host_rst_wrn_sts)
{
	if (host_rst_wrn_sts) {
		g_acpi_state_flags.sci_enabled = 0;
		LOG_DBG("Clear queues");
		sci_queue_init();
	}
}

static inline int smchost_task_init(void)
{
	host_req_len = 0;
	host_res_len = 0;

#ifdef CONFIG_ESPI_PERIPHERAL_HOST_IO_PVT
	smchost_pvt_init();
#endif

	/* Clear Queues */
	sci_queue_init();

	/* Register event handler */
	pwrbtn_register_handler(smchost_pwrbtn_handler);
	periph_register_button(VOL_UP, smchost_volbtnup_handler);
	periph_register_button(VOL_DOWN, smchost_volbtndown_handler);

	espihub_add_acpi_handler(ESPIHUB_ACPI_PUBLIC, smchost_acpi_handler);
	espihub_add_warn_handler(ESPIHUB_RESET_WARNING,
				 smchost_host_rst_warn_handler);
	espihub_add_warn_handler(ESPIHUB_PLATFORM_RESET,
				 smchost_pltrst_handler);

	g_acpi_tbl.acpi_flags2.bt_pwr_off = 1;
	g_acpi_tbl.acpi_flags2.vb_sw_closed = 1;
	g_acpi_tbl.acpi_flags2.pwr_btn = 1;
	g_acpi_tbl.acpi_flags2.pcie_docked = 1;
	g_acpi_tbl.acpi_flags.lid_open = 1;

	/* Default power button behavir is to suspend/resume switch */
	g_pwrflags.pwr_sw_suspend_resume = 1;

	return 0;
}

void smchost_thread(void *p1, void *p2, void *p3)
{
	u32_t period = *(u32_t *)p1;

	smchost_task_init();

	while (true) {
		k_msleep(period);

		if (acpi_burst_flag) {
			proc_acpi_burst();
		}

		/* Check for SMI/SCI pending and service any commands
		 * from the host
		 */
		check_sci_queue();
		service_system_acpi_cmds();
		proc_host_send();

#ifdef CONFIG_ESPI_PERIPHERAL_HOST_IO_PVT
		pvt_port_proc_host_send();
#endif
	}
}

static void proc_host_send(void)
{
	if (host_res_len > 0) {
		u8_t flag = acpi_get_flag(ACPI_EC_0, ACPI_FLAG_OBF);

		LOG_DBG("ACPI OBF flag %x", flag);
		if (flag) {
			return;
		}

		LOG_DBG("WriteODR %x",  host_res[host_res_idx]);

		acpi_write_odr(ACPI_EC_0, host_res[host_res_idx]);
		host_res_idx++;
		host_res_len--;

	}
}

/**
 * @brief Send data to the host.
 *
 * @param ptr pointer to the data.
 * @param len data length in bytes.
 */
void send_to_host(u8_t *pdata, u8_t Len)
{
	int i;

	for (i = 0; i < Len; i++) {
		host_res[i] = *(pdata + i);
		LOG_INF("Snd data: %02X",  host_res[i]);
	}

	host_res_len = Len;
	host_res_idx = 0;
}

static void service_system_acpi_cmds(void)
{
	if (!g_acpi_state_flags.acpi_mode) {
		return;
	}

	if (!g_acpi_tbl.acpi_host_command) {
		return;
	}

	/* Call the appropriate function from the table */
	LOG_INF("Srcv ACPI CMD %x",  g_acpi_tbl.acpi_host_command);
	smchost_cmd_handler(g_acpi_tbl.acpi_host_command);

	/* Clear the command after execution */
	g_acpi_tbl.acpi_host_command = 0;
}

void host_cmd_default(u8_t command)
{
	/* Log the execution to know BIOS send an unsupported command */
	LOG_WRN("%s: command 0x%X without handler", __func__, command);
}

static void read_byte(void)
{
	u8_t data = 0;

	send_to_host(&data, 1);
}

static void write_byte(void)
{
	u32_t addr;

	addr = host_req[1];
	addr = (addr << 8) | host_req[2];
	addr = (addr << 8) | host_req[3];
	addr = (addr << 8) | host_req[4];

	*((u8_t *)addr) = host_req[5];
}

static void acpi_read_ec(void)
{
	u8_t acpi_idx = host_req[1];
	u8_t data;

	if (acpi_idx <= ACPI_MAX_INDEX) {
		data = *((u8_t *)&g_acpi_tbl + acpi_idx);
		LOG_DBG("ACPI ECR Data [%02x]: %02x", acpi_idx, data);
	} else {
		data = acpi_idx;
	}

	if (acpi_send_byte(ACPI_EC_0, data) == 0) {
		generate_sci();
	}
}

static void acpi_write_ec(void)
{
	u8_t acpi_idx = host_req[1];
	u8_t data = host_req[2];

	if (acpi_idx <= ACPI_MAX_INDEX) {
		if (smc_is_acpi_offset_write_permitted(acpi_idx)) {
			*((u8_t *)&g_acpi_tbl + acpi_idx) = data;
			LOG_DBG("ACPI ECW Data [%02x]: %02x", acpi_idx, data);
		} else {
			LOG_WRN("ACPI WR not permitted at offset: %02x",
				acpi_idx);
		}
	}
}

static void acpi_burst_ec(void)
{
	acpi_burst_flag = 1;
}

static void proc_acpi_burst(void)
{
	acpi_burst_flag = 0;
	/* Tell host that we're burst */
	acpi_set_flag(ACPI_EC_0, ACPI_FLAG_ACPIBURST, 1);
	if (!acpi_send_byte(ACPI_EC_0, SCI_BURST_ACK)) {
		/* Do not process burst mode command to avoid EC timeout
		 * errors in OS
		 */
		generate_sci();
		return;
	}

	/* Abort burst */
	acpi_set_flag(ACPI_EC_0, ACPI_FLAG_ACPIBURST, 0);
	generate_sci();
}

static void acpi_normal_ec(void)
{
	acpi_set_flag(ACPI_EC_0, ACPI_FLAG_ACPIBURST, 0);
	generate_sci();
}

static void acpi_query_ec(void)
{
	/* Send any pending events */
	send_sci_events();
}

static void enable_acpi(void)
{
	g_acpi_state_flags.acpi_mode = 1;

	/* Disengage throttling */
	gpio_write_pin(PROCHOT, 1);

	/* Set ACPI sleep state level to S3 */
	g_acpi_tbl.acpi_flags.sleep_s3 = 1;

	/* Clear the event flag in host status */
	acpi_set_flag(ACPI_EC_0, ACPI_FLAG_SCIEVENT, 0);
}

static void disable_acpi(void)
{
	g_acpi_state_flags.acpi_mode = 0;

	LOG_DBG("Disable ACPI");
	/* Clear the event flag in host status */
	acpi_set_flag(ACPI_EC_0, ACPI_FLAG_SCIEVENT, 0);
}

static void read_acpi_space(void)
{
	send_to_host((u8_t *)&g_acpi_tbl + host_req[1], 1);
}

static void write_acpi_space(void)
{
	*((u8_t *) &g_acpi_tbl + host_req[1]) = host_req[2];
}

static u8_t smchost_req_length(u8_t command)
{
	switch (command) {
	case SMCHOST_SET_DSW_MODE:
		return 1;
	case SMCHOST_READ_BYTE:
		return 4;
	case SMCHOST_WRITE_BYTE:
		return 5;
	case SMCHOST_UPDATE_SYSPWR_STATE:
		return 1;
	case SMCHOST_ACPI_READ:
		return 1;
	case SMCHOST_ACPI_WRITE:
		return 2;
	case SMCHOST_PFAT_CMD_PROV_EAV:
		return 4;
	case SMCHOST_PFAT_CMD_OPEN:
		return 2;
	case SMCHOST_PFAT_CMD_PORT_TEST:
		return 4;
	case SMCHOST_INJECT_SCI:
		return 1;
	case SMCHOST_READ_ACPI_SPACE:
		return 1;
	case SMCHOST_WRITE_ACPI_SPACE:
		return 2;
	default:
		return 0;
	}
}

static void smchost_cmd_handler(u8_t command)
{
	switch (command) {

	/* Handlers for commands 00h to 0Fh */
	case SMCHOST_READ_BYTE:
		read_byte();
		break;
	case SMCHOST_WRITE_BYTE:
		write_byte();
		break;

	case SMCHOST_QUERY_SYSTEM_STS:
	case SMCHOST_GET_SMC_MODE:
	case SMCHOST_GET_SWITCH_STS:
	case SMCHOST_GET_FAB_ID:
	case SMCHOST_GET_DOCK_STS:
	case SMCHOST_GET_PMIC_VID:
	case SMCHOST_READ_REVISION:
	case SMCHOST_READ_PLAT_SIGNATURE:
	case SMCHOST_UCSI_READ_VERSION:
	case SMCHOST_GET_KSC_ID:
		smchost_cmd_info_handler(command);
		break;

	case SMCHOST_SUSPEND_SMC:
	case SMCHOST_RESUME_SMC:
	case SMCHOST_SET_DSW_MODE:
	case SMCHOST_GET_DSW_MODE:
	case SMCHOST_CS_ENTRY:
	case SMCHOST_CS_EXIT:
	case SMCHOST_GET_LEGACY_WAKE_STS:
	case SMCHOST_CLEAR_LEGACY_WAKE_STS:
	case SMCHOST_PWR_BTN_SCI_CONTROL:
	case SMCHOST_UPDATE_SYSPWR_STATE:
	case SMCHOST_READ_WAKE_STS:
	case SMCHOST_CLEAR_WAKE_STS:
	case SMCHOST_SX_ENTRY:
	case SMCHOST_SX_EXIT:
	case SMCHOST_ENABLE_PWR_BTN_NOTIFY:
	case SMCHOST_DISABLE_PWR_BTN_NOTIFY:
	case SMCHOST_SYSTEM_POWER_OFF:
	case SMCHOST_ENABLE_PWR_BTN_SW:
	case SMCHOST_DISABLE_PWR_BTN_SW:
	case SMCHOST_ENABLE_SOFT_PWR_BTN:
	case SMCHOST_DISABLE_SOFT_PWR_BTN:
	case SMCHOST_RESET_KSC:
		smchost_cmd_pm_handler(command);
		break;

#ifdef CONFIG_PFAT_SUPPORT
	case SMCHOST_PFAT_CMD_DISCOVERY:
	case SMCHOST_PFAT_CMD_PROV_EAV:
	case SMCHOST_PFAT_CMD_LOCK:
	case SMCHOST_PFAT_CMD_GET_SVN:
	case SMCHOST_PFAT_CMD_OPEN:
	case SMCHOST_PFAT_CMD_CLOSE:
	case SMCHOST_PFAT_CMD_PORT_TEST:
		smchost_cmd_pfat_handler(command);
		break;
#endif

#ifdef CONFIG_BATTERY_MANAGEMENT
	case SMCHOST_SET_SYS_CHRG_CFG:
	case SMCHOST_SET_CHRG_METHOD:
	case SMCHOST_UPDATE_UVTH:
	case SMCHOST_SET_BATT_THRSLD:
	case SMCHOST_CHRG_RATE_CHG_NOTIFY:
	case SMCHOST_BATT_SHIP_MODE:
		smchost_cmd_bmc_handler(command);
		break;
#endif

	/* Handlers for commands 80h to 8Fh */
	case SMCHOST_ACPI_READ:
		acpi_read_ec();
		break;
	case SMCHOST_ACPI_WRITE:
		acpi_write_ec();
		break;
	case SMCHOST_ACPI_BURST_MODE:
		acpi_burst_ec();
		break;
	case SMCHOST_ACPI_NORMAL_MODE:
		acpi_normal_ec();
		break;
	case SMCHOST_ACPI_QUERY:
		acpi_query_ec();
		break;

		break;

	/* Handlers for commands A0h to AFh */
	case SMCHOST_ENABLE_ACPI:
		enable_acpi();
		break;
	case SMCHOST_DISABLE_ACPI:
		disable_acpi();
		break;


	/* Handlers for commands E0h to EFh */
	case SMCHOST_READ_ACPI_SPACE:
		read_acpi_space();
		break;
	case SMCHOST_WRITE_ACPI_SPACE:
		write_acpi_space();
		break;
	default:
		host_cmd_default(command);
		break;
	}
}
