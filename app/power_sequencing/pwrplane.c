/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/espi.h>
#include <logging/log.h>
#include "gpio_ec.h"
#include "espi_hub.h"
#include "system.h"
#include "pwrplane.h"
#include "board_config.h"
#include "postcodemgmt.h"
#include "port80display.h"
#include "sci.h"
#include "smc.h"
#include "scicodes.h"
#include "deepsx.h"
#include "dswmode.h"
#include "pwrseq_timeouts.h"
#include "errcodes.h"
#include "vci.h"
#include "kbchost.h"

LOG_MODULE_REGISTER(pwrmgmt, CONFIG_PWRMGT_LOG_LEVEL);

/* For more details for below signals refer to platform design guidelines
 * If not explicitly indicated delays are in miliseconds.
 */

/* VCCST_PWRGD & PM_PCH_PWROK to be generated together 2ms after
 * ALL_SYS_PWRGD
 */
#define VR_ON_RAMP_DELAY_US    2000U

#define PM_PWROFF_DELAY_US     500U

/* PCH_PWROK to be generated T36 = 99 ms after SYS_PWROK */
#define SYS_PWR_OK_DELAY       99U

/* PM_RSMRST_ should be released 200 ms after PM_RSMRST_PWRGD */
#define PM_RSMRST_DELAY        200U

/* Track power sequencing events */
struct pwr_flags g_pwrflags;
static bool pwrseq_timeout_disabled;

/* System state machine */
static enum system_power_state current_state;
static enum system_power_state next_state;

/* Handle S5 entry/exit and G3 exit */
static void power_off(void);
static int power_on(void);
static void suspend(void);
static int resume(void);

/* This global variable is used mostly for pin configuration in board init */
u8_t boot_mode_maf;

enum system_power_state pwrseq_system_state(void)
{
	return current_state;
}

static void pwrseq_slp_handler(u32_t signal, u32_t status)
{
	/* De-assert always indicates transition to S0 */
	if (status) {
		switch (current_state) {
		case SYSTEM_G3_STATE:
		case SYSTEM_S3_STATE:
		case SYSTEM_S4_STATE:
		case SYSTEM_S5_STATE:
			next_state = SYSTEM_S0_STATE;
			break;
		default:
			LOG_WRN("SLP_SX=1 while at %x", current_state);
			break;
		}
	} else  {
		/* SLPx assertion indicates a specific power system state */
		switch (current_state) {
		case SYSTEM_S0_STATE:
			if (signal == ESPI_VWIRE_SIGNAL_SLP_S3) {
				next_state = SYSTEM_S3_STATE;
			} else if (signal == ESPI_VWIRE_SIGNAL_SLP_S4) {
				next_state = SYSTEM_S4_STATE;
			} else if (signal == ESPI_VWIRE_SIGNAL_SLP_S5) {
				next_state = SYSTEM_S5_STATE;
			}
			break;
		default:
			LOG_WRN("SLP_SX=1 while at %x", current_state);
		}
	}
}

/* This is required since workaround for DeepSx is not yet removed
 * Even if DSx is enabled, the ACK is not send from deepsx module
 */
static void pwrseq_sus_handler(u8_t status)
{
	if (!dsw_enabled() || current_state == SYSTEM_G3_STATE ||
		!dsx_entered()) {
		LOG_DBG("Send ACK SUS WARN %d", status);
		espihub_send_vw(ESPI_VWIRE_SIGNAL_SUS_ACK, status);
	}
}

static void handle_spi_sharing(u8_t boot_mode)
{
	switch (boot_mode) {
	case FLASH_BOOT_MODE_SAF:
		LOG_DBG("Booted in SAF mode");
		/* TODO: set FET HIGH, not possible in MECC card */
		break;
	case FLASH_BOOT_MODE_G3_SHARING:
		LOG_DBG("Booted in G3 mode");
		/* TODO: set FET LOW, not possible in MECC card */
		break;
	case FLASH_BOOT_MODE_MAF:
		boot_mode_maf = 1;
		LOG_DBG("Booted in MAF mode");
		break;
	default:
		LOG_DBG("Boot in %x mode", boot_mode);
		break;
	}
}

int wait_for_pin(u32_t port_pin, u16_t timeout,
			u32_t exp_level)
{
	u16_t loop_cnt = timeout;
	int level;

	do {
		/* Passes the enconded gpio(port_pin) to the gpio driver */
		level = gpio_read_pin(port_pin);
		if (level < 0) {
			LOG_ERR("Failed to read %x ", gpio_get_pin(port_pin));
			return -EIO;
		}

		if (exp_level == level) {
			LOG_DBG("Pin [%x]: %x",
				gpio_get_pin(port_pin), exp_level);
			break;
		}

		k_usleep(100);
		loop_cnt--;
	} while (loop_cnt > 0 || pwrseq_timeout_disabled);

	if (loop_cnt == 0) {
		LOG_DBG("Timeout [%x]: %x", gpio_get_pin(port_pin), level);
		return -ETIMEDOUT;
	}

	return 0;
}

static int check_slp_signals(void)
{
	int ret;

	/* Check for SLPS5#, SLPS4#, SLPS3# signals */
	ret = wait_for_vwire(ESPI_VWIRE_SIGNAL_SLP_S5, SLPS5_TIMEOUT,
			     ESPIHUB_VW_HIGH, false);
	if (ret) {
		pwrseq_error(ERR_PM_SLPS5);
		return ret;
	}

	ret = wait_for_vwire(ESPI_VWIRE_SIGNAL_SLP_S4, SLPS4_TIMEOUT,
			     ESPIHUB_VW_HIGH, false);
	if (ret) {
		pwrseq_error(ERR_PM_SLPS5);
		return ret;
	}

	ret = wait_for_vwire(ESPI_VWIRE_SIGNAL_SLP_S3, SLPS3_TIMEOUT,
			     ESPIHUB_VW_HIGH, false);
	if (ret) {
		pwrseq_error(ERR_PM_SLPS3);
		return ret;
	}

	return 0;
}

void pwrseq_error(u8_t error_code)
{
	LOG_ERR("%s: %d", __func__, error_code);

	update_error(error_code);
	k_msleep(100);
	gpio_write_pin(PCH_PWROK, 0);
	gpio_write_pin(SYS_PWROK, 0);
	gpio_write_pin(PM_RSMRST, 0);
}

static inline int pwrseq_task_init(void)
{
	int ret;

	LOG_DBG("Power initialization...");

	/* SW strap always takes precedence */
#ifdef CONFIG_POWER_SEQUENCE_DISABLE_TIMEOUTS
	pwrseq_timeout_disabled = true;
#else
	int level;

	level = gpio_read_pin(TIMEOUT_DISABLE);
	if (level < 0) {
		LOG_ERR("Fail to read timeout HW strap");
	} else {
		pwrseq_timeout_disabled = !level;
		LOG_WRN("Timeouts disabled: %d", pwrseq_timeout_disabled);
	}
#endif

	handle_spi_sharing(espihub_boot_mode());
	gpio_write_pin(PM_PWRBTN, 1);

	/* Disable VBAT powered VCI logic */
	vci_disable();

	ret = wait_for_pin(RSMRST_PWRGD,
			   RSMRST_PWRDG_TIMEOUT, 1);
	if (ret) {
		pwrseq_error(ERR_RSMRST_PWRGD);
		return ret;
	}

	if (espihub_boot_mode() == FLASH_BOOT_MODE_MAF) {
		/* If the board is in MAF mode and the execution reached this
		 * point, then it is obvious ESPI_RESET already ocurred.
		 * However, we read the reset signal as GPIO061 even when the
		 * pin is in alt mode 1(ESPI_RST#). Bear in mind this is may
		 * not be portable across SOCs.
		 */
		ret = wait_for_pin(ESPI_RESET_MAF,
				   ESPI_RST_TIMEOUT, 1);
		if (ret) {
			pwrseq_error(ERR_ESPIRESET);
			return ret;
		}
	} else {
		ret = gpio_write_pin(PM_RSMRST, 1);
		if (ret) {
			LOG_ERR("Failed to write PM_RSMRST");
			return ret;
		}

		ret = wait_for_espi_reset(ESPI_WAIT_TRUE, ESPI_RST_TIMEOUT);
		if (ret) {
			pwrseq_error(ERR_ESPIRESET);
			return ret;
		}
	}

	/* In MAF mode, manually send the SUS_ACK if automatic ACK
	 * is diabled in the espi driver. This is because the SUS_WARN
	 * isr event arrives before the ESPI callbacks are configured
	 * in the EC app; therefore the notification never reaches the
	 * EC application.
	 * For G3 exit same approach can be be used to simplify logic.
	 */
#ifndef CONFIG_ESPI_AUTOMATIC_WARNING_ACKNOWLEDGE
	ret = wait_for_vwire(ESPI_VWIRE_SIGNAL_SUS_WARN, SUSWARN_TIMEOUT,
			     ESPIHUB_VW_HIGH, true);
	if (ret) {
		pwrseq_error(ERR_PM_SUS_WARN);
		return ret;
	}
#endif

	espihub_add_state_handler(pwrseq_slp_handler);
	espihub_add_warn_handler(ESPIHUB_SUSPEND_WARNING, pwrseq_sus_handler);

	ret = gpio_write_pin(PM_BATLOW, 1);
	if (ret) {
		LOG_ERR("Failed to write PM_BATLOW");
	}

	return ret;
}

static void pwrseq_update(void)
{
	int ret = 0;

	switch (next_state) {
	case SYSTEM_S0_STATE:
		check_slp_signals();
		if (current_state == SYSTEM_G3_STATE ||
		    current_state == SYSTEM_S5_STATE) {
			ret = power_on();
		} else {
			ret = resume();
		}
		break;
	case SYSTEM_S3_STATE:
		if (current_state == SYSTEM_S0_STATE) {
			suspend();
		}
		break;
	case SYSTEM_S4_STATE:
	case SYSTEM_S5_STATE:
		if (current_state == SYSTEM_S0_STATE) {
			power_off();
		}
		break;
	default:
		LOG_ERR("Unsupported state");
		break;
	}

	if (!ret) {
		LOG_INF("System transition %d->%d", current_state, next_state);
		current_state = next_state;
	}
}

void pwrseq_thread(void *p1, void *p2, void *p3)
{
	int level;
	u32_t period = *(u32_t *)p1;

	pwrseq_task_init();

	while (true) {
		k_msleep(period);

		level = gpio_read_pin(RSMRST_PWRGD);

		if (level < 0) {
			LOG_ERR("Failed to read RSMRST_PWRGD %d", level);
			continue;
		} else if (level != g_pwrflags.pm_rsmrst) {
			LOG_DBG("RSMRST_PWRGD=%d", level);
			if (level) {
				/* PM_RSMRST_ should be delayed to allow
				 * enough time for poewr rail rampup.
				 */
				k_msleep(PM_RSMRST_DELAY);
				LOG_DBG("Delayed PM_RSMRST_ HIGH");
			}
		}
		g_pwrflags.pm_rsmrst = level;
		gpio_write_pin(PM_RSMRST, level);

		/* Update AC present ACPI flag */
		g_acpi_tbl.acpi_flags.ac_prsnt = gpio_read_pin(BC_ACOK);

		/* Check if power button was pressed */
		if (g_pwrflags.turn_pwr_on) {
			next_state = SYSTEM_S0_STATE;
			/* Indicate power button request has been processed */
			g_pwrflags.turn_pwr_on = 0;
		}

		if (g_pwrflags.turn_pwr_off) {
			next_state = SYSTEM_S5_STATE;
			/* Indicate power button request has been processed */
			g_pwrflags.turn_pwr_off = 0;
		}
		/* DeepSx is sub-state for S4/S5 */
		if (dsw_enabled() &&
		   ((pwrseq_system_state() == SYSTEM_S4_STATE) ||
		   (pwrseq_system_state() == SYSTEM_S5_STATE))) {
			manage_deep_s4s5();
		}

		if (next_state != current_state) {
			pwrseq_update();
		}
	}
}

static void power_off(void)
{
	int level;

#ifdef VCCST_PWRGD
	gpio_write_pin(VCCST_PWRGD, 0);
#endif
	gpio_write_pin(SYS_PWROK, 0);
	gpio_write_pin(PCH_PWROK, 0);

	k_busy_wait(PM_PWROFF_DELAY_US);

	g_pwrflags.pwr_sw_enabled = 1;

	level = gpio_get_pin(PWRBTN_EC_IN_N);
	if (level < 0) {
		LOG_ERR("Fail to read pwr_btn %d", level);
	}

	LOG_DBG("Shutting down %d", level);
	do {
		level = gpio_get_pin(PWRBTN_EC_IN_N);
	} while (!level);

	board_suspend();
	port80_display_off();

#ifdef CONFIG_ESPI_PERIPHERAL_8042_KBC
	kbc_disable_interface();
#endif
}

static int power_on(void)
{
	int ret;

	LOG_INF("%s", __func__);

	ret = wait_for_pin(RSMRST_PWRGD,
			   RSMRST_PWRDG_TIMEOUT, 1);
	if (ret) {
		pwrseq_error(ERR_RSMRST_PWRGD);
		return ret;
	}

	ret = gpio_write_pin(PM_RSMRST, 1);
	if (ret) {
		LOG_ERR("Unable to initialize %d ", gpio_get_pin(PM_RSMRST));
		return ret;
	}

	port80_display_on();

	ret = gpio_write_pin(PM_PWRBTN, 1);
	if (ret) {
		LOG_ERR("Unable to initialize %d ", PM_PWRBTN);
		return ret;
	}

	/* Check for SLPS5#, SLPS4#, SLPS3# signals */
	ret = check_slp_signals();
	if (ret) {
		return ret;
	}

	ret = wait_for_vwire(ESPI_VWIRE_SIGNAL_SLP_A, SLP_M_TIMEOUT,
			     ESPIHUB_VW_HIGH, false);
	if (ret) {
		pwrseq_error(ERR_PM_SLP_M);
		return ret;
	}


	ret = wait_for_pin(ALL_SYS_PWRGD, ALL_SYS_PWRGD_TIMEOUT, 1);
	if (ret) {
		pwrseq_error(ERR_ALL_SYS_PWRGD);
		return ret;
	}

	k_busy_wait(VR_ON_RAMP_DELAY_US);
#ifdef VCCST_PWRGD
	ret = gpio_write_pin(VCCST_PWRGD, 1);
	if (ret) {
		LOG_ERR("Failed to drive VCCST_PWRGD");
		return ret;
	}
#endif

	ret = gpio_write_pin(PCH_PWROK, 1);
	if (ret) {
		LOG_ERR("Failed to indicate eSPI master to run");
		return ret;
	}

	k_msleep(SYS_PWR_OK_DELAY);

	ret = gpio_write_pin(SYS_PWROK, 1);
	if (ret) {
		LOG_ERR("Failed to indicate system power is ok");
		return ret;
	}

	ret = gpio_write_pin(WAKE_SCI, 1);
	if (ret) {
		LOG_ERR("Failed to indicate eSPI master to wake");
		return ret;
	}

	ret = wait_for_vwire(ESPI_VWIRE_SIGNAL_PLTRST, PLT_RESET_TIMEOUT,
			     ESPIHUB_VW_HIGH, false);
	if (ret) {
		pwrseq_error(ERR_PLT_RST);
		return ret;
	}

#ifdef CONFIG_ESPI_PERIPHERAL_8042_KBC
	kbc_enable_interface();
#endif
	current_state = SYSTEM_S0_STATE;

	LOG_INF("%s new state %d", __func__, current_state);

	return 0;
}

static void suspend(void)
{
	LOG_DBG("%s", __func__);

	gpio_write_pin(PCH_PWROK, 0);
	gpio_write_pin(SYS_PWROK, 0);
	board_suspend();
	port80_display_off();
}

static int resume(void)
{
	int ret;

	LOG_DBG("%s", __func__);

	/* Perform power on sequence. If no errors, notify BIOS */
	ret = power_on();
	if (!ret) {
		enqueue_sci(SCI_RESUME);
	}

	board_resume();
	return ret;
}
