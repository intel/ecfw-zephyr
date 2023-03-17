/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/espi.h>
#include <zephyr/logging/log.h>
#include "gpio_ec.h"
#include "espi_hub.h"
#include "espioob_mngr.h"
#ifdef CONFIG_ESPI_SAF
#include "saf_config.h"
#endif
#include "system.h"
#include "pwrplane.h"
#include "pwrseq_utils.h"
#include "board_config.h"
#include "postcodemgmt.h"
#include "port80display.h"
#include "sci.h"
#include "smc.h"
#include "scicodes.h"
#include "deepsx.h"
#include "dswmode.h"
#include "pseudog3.h"
#ifdef CONFIG_SOC_DEBUG_AWARENESS
#include "soc_debug.h"
#endif
#include "pwrseq_timeouts.h"
#include "errcodes.h"
#ifdef CONFIG_SOC_FAMILY_MEC
#include "vci.h"
#endif
#include "fan.h"
#include "kbchost.h"
#include "task_handler.h"
#include "softstrap.h"
#include "smchost.h"
#ifdef CONFIG_DNX_SUPPORT
#include "dnx.h"
#endif

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

/* Wait time for global reset after pulling PCH_PWROK : 2 sec*/
#define GLOBAL_RESET_WAIT_TIME_CNT	20000U

/* Different types of chargers that will be used to indicate over 0x89.
 * 0x01 -> Traditional Type
 * 0x02 -> Hybrid Type
 * 0x03 -> NVDC
 */
#define CTYPE_NVDC	0x03
#define PWR_PLANE_RSMRST_DELAY_MS	100

/* Track power sequencing events */
struct pwr_flags g_pwrflags;
static bool pwrseq_timeout_disabled;
static bool pwrseq_failure;
static bool in_therm_shutdown;

/* System state machine */
static enum system_power_state current_state;
static enum system_power_state next_state;

/* Handle S5 entry/exit and G3 exit */
static void power_off(void);
static int power_on(void);
static void suspend(void);
static int resume(void);
static void pwrseq_reset(void);

/* This global variable is used mostly for pin configuration in board init */
uint8_t boot_mode_maf;

static uint8_t shutdown_reason;

uint8_t read_shutdown_reason(void)
{
	return shutdown_reason;
}

void set_shutdown_reason(uint8_t reason)
{
	shutdown_reason = reason;
}

enum system_power_state pwrseq_system_state(void)
{
	return current_state;
}

void disable_ec_timeout(void)
{
	if (!pwrseq_timeout_disabled) {
		pwrseq_timeout_disabled = true;
	}
}

bool ec_timeout_status(void)
{
	return pwrseq_timeout_disabled;
}

void ec_evaluate_timeout(void)
{
	int level;

	level = gpio_read_pin(TIMEOUT_DISABLE);
	LOG_WRN("%s pmc_gpio: %d", __func__, level);
	if (level < 0) {
		LOG_ERR("Fail to read timeout HW strap");
	} else if (level == 0) {
		pwrseq_timeout_disabled = true;
		LOG_WRN("EC WDT disabled");
	} else {
		pwrseq_timeout_disabled = false;
		LOG_WRN("EC WDT enabled");
	}
}

static void pwrseq_slp_handler(uint32_t signal, uint32_t status)
{
	/* De-assert always indicates transition to S0 */
	if (status) {
		switch (current_state) {
		case SYSTEM_S5_STATE:
		case SYSTEM_S4_STATE:
			if (signal == ESPI_VWIRE_SIGNAL_SLP_S5) {
				if (pseudo_g3_get_prev_state()
					|| pseudo_g3_get_state()) {
					next_state = SYSTEM_S4_STATE;
					LOG_DBG("PG3->S4");
					break;
				}
			}
			/* Else continue */
		case SYSTEM_G3_STATE:
		case SYSTEM_S3_STATE:
			LOG_DBG("SLP S0 asserted");
			next_state = SYSTEM_S0_STATE;
			break;
		default:
			LOG_WRN("SLP_SX=1 while at %x", current_state);
			break;
		}
	} else  {
		/* SLPx assertion indicates a specific power system state */
		switch (signal) {
		case ESPI_VWIRE_SIGNAL_SLP_S3:
			LOG_DBG("SLP S3 asserted");
			next_state = SYSTEM_S3_STATE;
			break;
		case ESPI_VWIRE_SIGNAL_SLP_S4:
			LOG_DBG("SLP S4 asserted");
			next_state = SYSTEM_S4_STATE;
			break;
		case ESPI_VWIRE_SIGNAL_SLP_S5:
			LOG_DBG("SLP S5 asserted");
			next_state = SYSTEM_S5_STATE;
			break;
		default:
			break;
		}
	}
}

static void pwrseq_sus_handler(uint8_t status)
{
	/*For PG3, send SUS ACK for SUS WARN*/
	if (!dsw_enabled()) {
		LOG_DBG("Send ACK SUS WARN %d", status);
		espihub_send_vw(ESPI_VWIRE_SIGNAL_SUS_ACK, status);
	}
}

void espi_bus_reset_handler(uint8_t status)
{
#ifdef CONFIG_DNX_SUPPORT
	dnx_espi_bus_reset_handler(status);
#endif

	LOG_DBG("ESPI RST status: %d", status);

	if (!status) {
		set_next_state_to_S5();
	}
}

static void handle_spi_sharing(uint8_t boot_mode)
{
	switch (boot_mode) {
	case FLASH_BOOT_MODE_SAF:
		LOG_DBG("Booted in SAF mode");
#ifdef CONFIG_ESPI_SAF
		initialize_saf_bridge();
#endif
		break;
	case FLASH_BOOT_MODE_G3_SHARING:
		LOG_DBG("Booted in G3 mode");
		break;
	case FLASH_BOOT_MODE_MAF:
		boot_mode_maf = 1;
		LOG_DBG("Booted in MAF mode");
		break;
	default:
		LOG_ERR("Boot in %x mode", boot_mode);
		break;
	}
}

static int wait_for_pin_level(uint32_t port_pin, uint16_t timeout,
			uint32_t exp_level)
{
	uint16_t loop_cnt = timeout;
	int level;

	do {
		/* Passes the enconded gpio(port_pin) to the gpio driver */
		level = gpio_read_pin(port_pin);
		if (level < 0) {
			LOG_ERR("Failed to read %x ", gpio_get_pin(port_pin));
			return -EIO;
		}

		if (exp_level == level) {
			LOG_DBG("Pin [%o]: %x",
				get_absolute_gpio_num(port_pin), exp_level);
			break;
		}

		k_usleep(100);
		loop_cnt--;
	} while (loop_cnt > 0 || (timeout == PWR_SEQ_TIMEOUT_FOREVER) ||
		ec_timeout_status());

	if (loop_cnt == 0) {
		LOG_DBG("Timeout [%x]: %x", gpio_get_pin(port_pin), level);
		return -ETIMEDOUT;
	}

	return 0;
}

static inline int wait_for_pin(uint32_t port_pin, uint16_t timeout,
			       uint32_t exp_level)
{
	if (pwrseq_timeout_disabled) {
		timeout = PWR_SEQ_TIMEOUT_FOREVER;
	}

	return wait_for_pin_level(port_pin, timeout, exp_level);
}

static inline int wait_for_vwire(uint8_t signal, uint16_t timeout,
				uint8_t exp_level, bool ack_req)
{
	if (pwrseq_timeout_disabled) {
		timeout = PWR_SEQ_TIMEOUT_FOREVER;
	}

	return espihub_wait_for_vwire(signal, timeout, exp_level, ack_req);
}

static inline int wait_for_espi_reset(uint8_t exp_sts, uint16_t timeout)
{
	if (pwrseq_timeout_disabled) {
		timeout = PWR_SEQ_TIMEOUT_FOREVER;
	}

	return espihub_wait_for_espi_reset(exp_sts, timeout);
}

static int check_slp_signals(void)
{
	int ret;

	/* Check for SLPS5#, SLPS4#, SLPS3# signals */
	ret = wait_for_vwire(ESPI_VWIRE_SIGNAL_SLP_S5,
			     TIMEOUT_TO_US(sw_strps()->timeouts.slp_s5),
			     ESPIHUB_VW_HIGH, false);
	if (ret) {
		pwrseq_error(ERR_PM_SLPS5);
		return ret;
	}

	ret = wait_for_vwire(ESPI_VWIRE_SIGNAL_SLP_S4,
			     TIMEOUT_TO_US(sw_strps()->timeouts.slp_s4),
			     ESPIHUB_VW_HIGH, false);
	if (ret) {
		pwrseq_error(ERR_PM_SLPS4);
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

void pwrseq_error(uint8_t error_code)
{
	LOG_ERR("%s: %d", __func__, error_code);

	pwrseq_failure = true;
#ifdef CONFIG_POSTCODE_MANAGEMENT
	update_error(error_code);
#endif
	k_msleep(100);
	gpio_write_pin(PCH_PWROK, 0);
	gpio_write_pin(SYS_PWROK, 0);
}

void pwrseq_reset(void)
{
	LOG_DBG("%s", __func__);

	pwrseq_failure = false;

#ifdef CONFIG_POSTCODE_MANAGEMENT
	/* Clear port 80 */
	update_error(ERR_NONE);
#endif
}

static inline int pwrseq_task_init(void)
{
	int ret;

	LOG_DBG("Power initialization...");

#ifdef CONFIG_SOC_DEBUG_AWARENESS
	soc_debug_init();
#endif
#ifdef CONFIG_DNX_EC_ASSISTED_TRIGGER
	dnx_ec_assisted_init();
#endif

	/* the charger type as of now is updated with a static value */
	g_acpi_tbl.acpi_ctype_value = CTYPE_NVDC;

	/* SW strap always takes precedence */
#ifdef CONFIG_POWER_SEQUENCE_DISABLE_TIMEOUTS
	pwrseq_timeout_disabled = true;
	LOG_WRN("SW timeouts disabled: %d", pwrseq_timeout_disabled);
#else
#ifndef CONFIG_SOC_DEBUG_CONSENT_GPIO
	/* Intel SoC debug consent is communicated via SoC-bootstall pin and
	 * connected to same path for legacy EC WDT HW strap.
	 * However, Soc-bootstall is stable only have predetermined time,
	 * EC WDT can be sampled immediately only if SoC debug consent is not
	 * supported.
	 */
	ec_evaluate_timeout();
#endif /* CONFIG_SOC_DEBUG_CONSENT_GPIO */
#endif /* CONFIG_POWER_SEQUENCE_DISABLE_TIMEOUTS */

	handle_spi_sharing(espihub_boot_mode());
	gpio_write_pin(PM_PWRBTN, 1);

	#ifdef CONFIG_SOC_FAMILY_MEC
	/* Disable VBAT powered VCI logic */
	vci_disable();
	#endif

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
				   TIMEOUT_TO_US(sw_strps()->timeouts.espi_rst),
				   1);
		if (ret) {
			pwrseq_error(ERR_ESPIRESET);
			return ret;
		}
	} else {
		LOG_DBG("PM_RSMRST_G3SAF_P");
		ret = gpio_write_pin(PM_RSMRST, 1);
		if (ret) {
			LOG_ERR("Failed to write PM_RSMRST");
			return ret;
		}

		ret = wait_for_espi_reset(ESPI_WAIT_TRUE,
				TIMEOUT_TO_US(sw_strps()->timeouts.espi_rst));
		if (ret) {
			pwrseq_error(ERR_ESPIRESET);
			return ret;
		}
	}

#ifdef CONFIG_DNX_SUPPORT
	dnx_handle_early_handshake();
#endif

	/* In MAF mode, manually send the SUS_ACK if automatic ACK
	 * is diabled in the espi driver. This is because the SUS_WARN
	 * isr event arrives before the ESPI callbacks are configured
	 * in the EC app; therefore the notification never reaches the
	 * EC application.
	 * For G3 exit same approach can be be used to simplify logic.
	 */

#ifndef CONFIG_ESPI_AUTOMATIC_WARNING_ACKNOWLEDGE
	/* This function performs when required SUS_ACK */
	ret = wait_for_vwire(ESPI_VWIRE_SIGNAL_SUS_WARN,
			TIMEOUT_TO_US(sw_strps()->timeouts.sus_wrn),
			ESPIHUB_VW_HIGH, true);
	/* Only perform SUS_WRN timeout if DSx is enabled */
	if (ret && dsw_enabled()) {
		pwrseq_error(ERR_PM_SUS_WARN);
		return ret;
	}
#endif
	espihub_add_state_handler(pwrseq_slp_handler);
	espihub_add_warn_handler(ESPIHUB_SUSPEND_WARNING, pwrseq_sus_handler);
	espihub_add_warn_handler(ESPIHUB_BUS_RESET, espi_bus_reset_handler);

	return ret;
}

static bool pwrseq_handle_transition_to_s0(void)
{
	bool valid_transition = false;
	int ret;

	switch (current_state) {
	case SYSTEM_G3_STATE: /* G3 -> S0 */
	case SYSTEM_S5_STATE: /* S5 -> S0 */
	case SYSTEM_S4_STATE: /* S4 -> S0 */
		ret = check_slp_signals();
		if (ret) {
			LOG_ERR("SLP signal timeout error");
			break;
		}
		ret = power_on();
		if (ret) {
			LOG_ERR("power_on() error");
			break;
		}
		valid_transition = true;
		break;
	case SYSTEM_S3_STATE: /* S3 -> S0 */
		ret = resume();
		if (ret) {
			LOG_ERR("resume() error");
			break;
		}
		valid_transition = true;
		break;
	case SYSTEM_S0_STATE: /* S0 -> S0 */
	default:
		/* No action */
		break;
	}

	return valid_transition;
}

static bool pwrseq_handle_transition_to_s3(void)
{
	bool valid_transition = false;

	switch (current_state) {
	case SYSTEM_S0_STATE: /* S0 -> S3 */
		suspend();
		valid_transition = true;
	case SYSTEM_G3_STATE: /* G3 -> S3 */
	case SYSTEM_S5_STATE: /* S5 -> S3 */
	case SYSTEM_S4_STATE: /* S4 -> S3 */
		/* valid transition with no action */
		valid_transition = true;
		break;
	case SYSTEM_S3_STATE: /* S3 -> S3 */
	default:
		break;
	}

	return valid_transition;
}

static bool pwrseq_handle_transition_to_s4(void)
{
	bool valid_transition = false;

	switch (current_state) {
	case SYSTEM_S3_STATE: /* S3 -> S4 */
	case SYSTEM_S0_STATE: /* S0 -> S4 */
		power_off();
		valid_transition = true;
		break;
	case SYSTEM_G3_STATE: /* G3 -> S4 */
	case SYSTEM_S5_STATE: /* S5 -> S4 */
		/* valid transition with no action */
		valid_transition = true;
		break;
	case SYSTEM_S4_STATE: /* S4 -> S4 */
	default:
		break;
	}

	return valid_transition;
}

static bool pwrseq_handle_transition_to_s5(void)
{
	bool valid_transition = false;

	switch (current_state) {
	case SYSTEM_S3_STATE: /* S3 -> S5 */
	case SYSTEM_S0_STATE: /* S0 -> S5 */
		power_off();
		valid_transition = true;
		break;
	case SYSTEM_G3_STATE: /* G3 -> S5 */
	case SYSTEM_S4_STATE: /* S4 -> S5 */
		/* valid transition with no action */
		valid_transition = true;
		break;
	case SYSTEM_S5_STATE: /* S5 -> S5 */
	default:
		/* No action */
		break;
	}
	return valid_transition;
}

static void pwrseq_update(void)
{
	bool valid_sx_transition = false;

	switch (next_state) {
	case SYSTEM_S0_STATE:
		valid_sx_transition = pwrseq_handle_transition_to_s0();
		break;
	case SYSTEM_S3_STATE:
		valid_sx_transition = pwrseq_handle_transition_to_s3();
		break;
	case SYSTEM_S4_STATE:
		valid_sx_transition = pwrseq_handle_transition_to_s4();
		break;
	case SYSTEM_S5_STATE:
		valid_sx_transition = pwrseq_handle_transition_to_s5();
		break;
	default:
		break;
	}

	if (valid_sx_transition) {
		LOG_INF("System transition %d->%d", current_state, next_state);
		current_state = next_state;
	} else {
		LOG_ERR("Unsupported next state: %d", next_state);
		/* Do not transition to invalid state,
		 * stay in current valid role.
		 */
		next_state = current_state;
	}
}

bool atx_detect(void)
{
#ifdef CONFIG_ATX_SUPPORT
	/* If ATX present, the ATX_DETECT gpio will be pulled low */
	return !gpio_read_pin(ATX_DETECT);
#endif
	/* Return by default as ATX not present */
	return false;
}

static bool pwrpln_check_power_adapter_levels(void)
{
	if (atx_detect() == true) {
		return true;
	}

	if (gpio_read_pin(BC_ACOK)) {
		return true;
	}

	return false;
}

static void pwrpln_check_power_critical_levels(void)
{
	bool power_good = false;

	/* Assert SOC bat low either for power adapter below the
	 * threshold power limit or low battery condition
	 */
	power_good = pwrpln_check_power_adapter_levels();

	gpio_write_pin(PM_BATLOW, (power_good) ? 1 : 0);
}

void set_next_state_to_S5(void)
{
	next_state = SYSTEM_S5_STATE;
}

void pwrseq_thread(void *p1, void *p2, void *p3)
{
	int rsmrst_level;
	uint32_t period = *(uint32_t *)p1;

	pwrseq_task_init();
	dsw_read_mode();

	while (true) {
		k_msleep(period);

		rsmrst_level = gpio_read_pin(RSMRST_PWRGD);

		if (rsmrst_level < 0) {
			LOG_ERR("Failed to read RSMRST_PWRGD %d", rsmrst_level);
			continue;
		} else if (rsmrst_level != g_pwrflags.pm_rsmrst) {
			LOG_DBG("RSMRST_PWRGD=%d", rsmrst_level);
			if (rsmrst_level) {
				/* PM_RSMRST_ should be delayed to allow
				 * enough time for poewr rail rampup.
				 */
				k_msleep(PM_RSMRST_DELAY);
				LOG_DBG("Delayed PM_RSMRST_ HIGH");
			} else {
				/* Debug indication to check if this toggles
				 * when power is cut-off in G3 cycling
				 */
				LOG_DBG("Immediate PM_RSMRST LOW");
			}
		}
		g_pwrflags.pm_rsmrst = rsmrst_level;

		if (in_therm_shutdown == false) {
			gpio_write_pin(PM_RSMRST, rsmrst_level);
		}

		/* Update AC present ACPI flag */
		g_acpi_tbl.acpi_flags.ac_prsnt = gpio_read_pin(BC_ACOK);

		/* Check if power button was pressed */
		if (g_pwrflags.turn_pwr_on) {
			next_state = SYSTEM_S0_STATE;
			/* Indicate power button request has been processed */
			g_pwrflags.turn_pwr_on = false;
		}

		if (g_pwrflags.turn_pwr_off) {
			LOG_DBG("turn_pwr_off true");
			next_state = SYSTEM_S5_STATE;
			/* Indicate power button request has been processed */
			g_pwrflags.turn_pwr_off = false;
		}

		/* EC pwr_seq default state is G3.
		 * EC should not be in G3 at any time once rsmrst is high.
		 *
		 * If sleep signals are not released by system (i.e. next
		 * state is still in G3), then set state to S5.
		 */
		if (current_state == SYSTEM_G3_STATE &&
			next_state == SYSTEM_G3_STATE &&
			rsmrst_level) {
			next_state = SYSTEM_S5_STATE;
		}

		/* DeepSx is sub-state for S4/S5 */
		if (dsw_enabled() &&
		   ((pwrseq_system_state() == SYSTEM_S4_STATE) ||
		   (pwrseq_system_state() == SYSTEM_S5_STATE))) {
			manage_deep_s4s5();
		}

		/* Update pwrseq state when state change request is present
		 * and no power sequence failure.
		 */
		if ((next_state != current_state) && (!pwrseq_failure)) {
			pwrseq_update();
		}

		/* Update EEPROM if required */
		dsw_save_mode();

		pwrpln_check_power_critical_levels();

#ifndef CONFIG_ESPI_OOB_CHANNEL_RX_ASYNC
		/*
		 * When eSPI OOB RX callback is not enabled, downstream OOBs
		 * need to be polled within the thread. This is blocking call.
		 */
		oob_rx_cb_handler();
#endif
		manage_pseudog3();
	}
}

void therm_shutdown(void)
{
	/* Set the shutdown reason */
	set_shutdown_reason(SHUTDOWN_REASON_CRTITICAL_THERMAL);
	in_therm_shutdown = true;

	/* Turn down SYS and PCH */
	gpio_write_pin(SYS_PWROK, 0);
	gpio_write_pin(PCH_PWROK, 0);

	espihub_wait_for_espi_reset(LOW, GLOBAL_RESET_WAIT_TIME_CNT);

	set_next_state_to_S5();

	/* Assert RSMRST */
	gpio_write_pin(PM_RSMRST, LOW);

	/* Sleep for 100ms as pwr_seq state machine needs to move S5 state */
	k_msleep(PWR_PLANE_RSMRST_DELAY_MS);

	/* System is moved to S5 state and SX state machine also updated.
	 * Now suspend all the tasks
	 */
	suspend_all_tasks();

	while (true) {
		if (gpio_read_pin(PWRBTN_EC_IN_N) == LOW) {
			g_pwrflags.turn_pwr_on = true;
			/* Rotate fan and toggle leds until pwr btn pressed */
			break;
		}

		fan_set_duty_cycle(FAN_CPU, 50);

		/* EC initiated shutdown is indicated by flashing of NUM and
		 * CAPS lock LEDs alternatively.
		 */
#ifdef CONFIG_ESPI_PERIPHERAL_8042_KBC
		kbc_set_leds(BIT(NUM_LOCK_POS));
#endif
		k_msleep(KBC_LED_FLASH_DELAY_MS);
#ifdef CONFIG_ESPI_PERIPHERAL_8042_KBC
		kbc_set_leds(BIT(CAPS_LOCK_POS));
#endif
		k_msleep(KBC_LED_FLASH_DELAY_MS);
	}

	in_therm_shutdown = false;
	/* Pwr btn pressed. Resume suspended tasks to allow boot */
	resume_all_tasks();
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

	level = gpio_read_pin(PWRBTN_EC_IN_N);
	if (level < 0) {
		LOG_ERR("Fail to read pwr_btn %d", level);
	}

#ifdef CONFIG_ESPI_PERIPHERAL_8042_KBC
	kbc_disable_interface();
#endif
	board_suspend();

#ifdef CONFIG_POSTCODE_MANAGEMENT
	port80_display_off();
#endif

	LOG_DBG("Shutting down %d", level);
	do {
		level = gpio_read_pin(PWRBTN_EC_IN_N);
	} while (!level);

	gpio_write_pin(EC_PWRBTN_LED, LOW);
	LOG_DBG("Power off complete");
}


static int power_on(void)
{
	int ret;

	LOG_INF("%s", __func__);

	pwrseq_reset();

#ifdef CONFIG_SOC_DEBUG_AWARENESS
	soc_debug_consent_kbs();
#endif
#ifdef CONFIG_DNX_EC_ASSISTED_TRIGGER
	dnx_ec_assisted_manage();
#endif
	ret = wait_for_pin(RSMRST_PWRGD,
			   RSMRST_PWRDG_TIMEOUT, 1);
	if (ret) {
		LOG_ERR("RSMRST_PWRGD timeout");
		pwrseq_error(ERR_RSMRST_PWRGD);
		return ret;
	}

	ret = gpio_write_pin(PM_RSMRST, 1);
	if (ret) {
		LOG_ERR("Unable to initialize %d ", gpio_get_pin(PM_RSMRST));
		return ret;
	}

#ifdef CONFIG_POSTCODE_MANAGEMENT
	port80_display_on();
#endif

	/* Check for SLPS5#, SLPS4#, SLPS3# signals */
	ret = check_slp_signals();
	if (ret) {
		return ret;
	}

	ret = wait_for_vwire(ESPI_VWIRE_SIGNAL_SLP_A,
			     TIMEOUT_TO_US(sw_strps()->timeouts.slp_m),
			     ESPIHUB_VW_HIGH, false);
	if (ret) {
		pwrseq_error(ERR_PM_SLP_M);
		return ret;
	}


	LOG_DBG("Waiting for ALL_SYS_PWRGD to go HIGH");
	ret = wait_for_pin(ALL_SYS_PWRGD,
			   TIMEOUT_TO_US(sw_strps()->timeouts.all_sys_pwrg),
			   1);
	if (ret) {
		pwrseq_error(ERR_ALL_SYS_PWRGD);
		LOG_ERR("ALL_SYS_PWRGD timed out\n");
		return ret;
	}

	LOG_DBG("ALL_SYS_PWRGD is HIGH");
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

	ret = wait_for_vwire(ESPI_VWIRE_SIGNAL_PLTRST,
			     TIMEOUT_TO_US(sw_strps()->timeouts.plt_rst),
			     ESPIHUB_VW_HIGH, false);
	if (ret) {
		pwrseq_error(ERR_PLT_RST);
		return ret;
	}

	gpio_write_pin(EC_PWRBTN_LED, HIGH);

#ifdef CONFIG_ESPI_PERIPHERAL_8042_KBC
	kbc_enable_interface();
#endif
	LOG_INF("%s new state %d", __func__, current_state);

	return 0;
}

static void suspend(void)
{
	LOG_DBG("%s", __func__);

	gpio_write_pin(PCH_PWROK, 0);
	gpio_write_pin(SYS_PWROK, 0);
	board_suspend();
#ifdef CONFIG_POSTCODE_MANAGEMENT
	port80_display_off();
#endif
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

