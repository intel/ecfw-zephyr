/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include "gpio_ec.h"
#include <zephyr/drivers/espi.h>
#include "board.h"
#include "board_config.h"
#include "deepsx.h"
#include "dswmode.h"
#ifdef CONFIG_SOC_FAMILY_MEC
#include "vci.h"
#endif
#include "system.h"
#include "espi_hub.h"
#include "pwrplane.h"
#include "pwrbtnmgmt.h"
#include "pwrseq_timeouts.h"
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(deepsx, CONFIG_PWRMGMT_DEEPSX_LOG_LEVEL);

/* Delay to wait for SUS_WARN after PMIC change */
#define DEEPSX_SUS_WARN_DELAY      20

#define SUS_WRN_LOW		0
#define SUS_WRN_HIGH		1

static bool sys_deep_sx;
static uint8_t slp_m;

/* DeepSx is enabled by BIOS, EC gets notified about via EC command about which
 * mode is selected.
 *
 * There are several DSx modes divided in 2 main categories:
 *   Always on.  Platform attempts deep sx involve AC conditions.
 *   Battery only.  Platform attempts deep sx do no involve AC conditions.
 *
 * In each sub-mode EC enter DSx under same conditions to equivalen Sx state.
 * i.e. to enter DS4 like in regular S4, SLP_S4 VWire has to be asserted.
 * and to exit DS4 same than S4, SLP_S4 VWire has to be de-asserted
 *
 * Also note that DS3 is a subfeature for only certain boards.
 */

bool dsx_entered(void)
{
	return sys_deep_sx;
}

static int ack_deep_sleep_transition(uint8_t exp_sus_wrn)
{
	uint8_t sus_wrn;
	int ret;

	ret = espihub_retrieve_vw(ESPI_VWIRE_SIGNAL_SUS_WARN, &sus_wrn);
	if (ret) {
		LOG_ERR(" Unable to receive sus_wrn");
		return ret;
	}

	if (sus_wrn != exp_sus_wrn) {
		LOG_ERR(" Received %s instead of %s",
				sus_wrn?"SUS_WRN_HIGH":"SUS_WRN_LOW",
				exp_sus_wrn?"SUS_WRN_HIGH":"SUS_WRN_LOW");
		return -EINVAL;
	}

	ret = espihub_send_vw(ESPI_VWIRE_SIGNAL_SUS_ACK, sus_wrn);
	if (ret) {
		LOG_ERR("Unable to send VW SUS_ACK");
		return ret;
	}

	return ret;
}

#ifdef CONFIG_PWRMGMT_DEEPSX_S3
static bool enter_deeps3(void)
{
	/* Check if this is S3 entry.
	 * If SLP_S3 is not de-asserted means there is no transition to S3,
	 * similarly SLP_S4 asserted indicates transition to S4.
	 * Do nothing on both cases.
	 */
	if (pwrseq_system_state() != SYSTEM_S3_STATE) {
		LOG_DBG("Invalid sleep state");
		return false;
	}

	k_msleep(DEEPSX_SUS_WARN_DELAY);
	ret = ack_deep_sleep_transition(SUS_WRN_LOW);
	if (ret) {
		LOG_WRN("%s:failed ack deep sleep transition %d",
				__func__, ret);
		return false;
	}

	sys_deep_sx = true;

	/* Note this requires to disable automatic SUS_ACK from eSPI driver */
	wait_for_pin(gpio_deva, PM_SLP_SUS, PM_SLP_SUS_TIMEOUT, 0);

	/* Assert RSMRST# */
	gpio_write_pin(PM_RSMRST, 0);

	return true;
}

static bool exit_deeps3(void)
{
	int slp_sus;
	int rsmrst_pwrgd;

	slp_sus = gpio_read_pin(PM_SLP_SUS);
	rsmrst_pwrgd = gpio_read_pin(RSMRST_PWRGD);

	if (slp_sus < 0 || rsmrst_pwrgd < 0) {
		LOG_ERR("Fail to read %s pins", __func__);
		return false;
	}

	/* Check if while exiting there is any errors
	 * RSMRST_PWRGD status will indicate if there is power rail problem.
	 * SLP_SUS not de-asserted indicate PCH has exited suspend.
	 */
	if (!slp_sus) {
		LOG_DBG("Fail slp_sus: %d", slp_sus);
		return false;
	}

	if (!rsmrst_pwrgd) {
		LOG_DBG("Fail rsmrst_pwrgd: %d", rsmrst_pwrgd);
		return false;
	}

	k_msleep(DEEPSX_SUS_WARN_DELAY);
	gpio_write_pin(PM_RSMRST, 1);

	/* Acknowledge suspend warning */
	ret = ack_deep_sleep_transition(SUS_WRN_HIGH);
	if (ret) {
		LOG_WRN("%s:failed ack deep sleep transition %d",
				__func__, ret);
		return false;
	}

	return true;
}
#endif

static bool exit_deeps4(void)
{
	int ret;
	int rsmrst_pwrgd;
	int slp_sus;

	LOG_DBG("%s", __func__);
	rsmrst_pwrgd = gpio_read_pin(RSMRST_PWRGD);
	slp_sus = gpio_read_pin(PM_SLP_SUS);

	if (slp_sus < 0 || rsmrst_pwrgd < 0) {
		LOG_ERR("Fail to read %s pins", __func__);
		return false;
	}

	/* Check if while exiting there is any errors
	 * RSMRST_PWRGD status will indicate if there is power rail problem.
	 * SLP_SUS not de-asserted indicate PCH has exited suspend.
	 */
	if (!slp_sus) {
		LOG_DBG("Fail slp_sus: %d", slp_sus);
		return false;
	}

	if (!rsmrst_pwrgd) {
		LOG_DBG("Fail rsmrst_pwrgd: %d", rsmrst_pwrgd);
		return false;
	}

	k_msleep(DEEPSX_SUS_WARN_DELAY);
	gpio_write_pin(PM_RSMRST, 1);

	/* Check eSPI reset is de-asserted otherwise this is a failure */
	ret = espihub_wait_for_espi_reset(1, ESPI_RST_TIMEOUT);
	if (ret) {
		LOG_ERR("ESPI_RST timeout");
		return false;
	}

	/* Acknowledge suspend warning */
	ret = ack_deep_sleep_transition(SUS_WRN_HIGH);
	if (ret) {
		LOG_WRN("%s:failed ack deep sleep transition %d",
				__func__, ret);
		return false;
	}

	sys_deep_sx = false;

	return true;
}

#ifdef CONFIG_PWRMGMT_DEEPSX_S3
bool manage_deep_s3(void)
{
	bool status;

	/* Nothing to be done, return immediately */
	if (!dsw_enabled()) {
		return false;
	}

	/* Check if deepSx is enabled */
	switch (dsw_mode()) {
	case ENABLE_IN_S3S4S5_DC:
	case ENABLE_FF_IN_S3S4S5_DC:
		if (!sys_deep_sx)  {
			status = enter_deeps3();
		} else {
			status = exit_deeps3();
		}
		break;
	default:
		status = false;
		break;
	}

	return status;
}
#endif

static bool check_deepsx_conds(void)
{
	bool can_enter;

	espihub_retrieve_vw(ESPI_VWIRE_SIGNAL_SLP_A, &slp_m);
	switch (dsw_mode()) {
	case ENABLE_IN_S5_DC:
		can_enter = check_s5_battonly_condition();
		break;
	case ENABLE_IN_S5_ACDC:
		can_enter = check_s5_alwayson_condition();
		break;
	case ENABLE_IN_S4S5_DC:
	case ENABLE_IN_S3S4S5_DC:
	case ENABLE_FF_IN_S4S5_DC:
	case ENABLE_FF_IN_S3S4S5_DC:
		can_enter = check_s4s5_battonly_condition();
		break;
	case ENABLE_IN_S4S5_ACDC:
		can_enter = check_s4s5_alwayson_condition();
		break;
	default:
		can_enter = false;
		break;
	}

	return can_enter;
}


bool manage_deep_s4s5(void)
{
	bool status;

	if (!dsw_enabled()) {
		return false;
	}

	LOG_DBG("%s %d", __func__, sys_deep_sx);
	if (sys_deep_sx) {
		status = exit_deeps4();
	} else {
		status = false;
		if (check_deepsx_conds()) {
			deep_sx_enter();
			status = true;
		}
	}

	return status;
}

bool check_s5_battonly_condition(void)
{
	LOG_DBG("%s", __func__);
	if (pwrseq_system_state() != SYSTEM_S5_STATE) {
		LOG_DBG("Invalid sleep state");
		return false;
	}

	if (slp_m) {
		LOG_DBG("Invalid sleep signal status: %d", slp_m);
		return false;
	}

	return true;
}

bool check_s5_alwayson_condition(void)
{
	LOG_DBG("%s", __func__);

	/* Check for correct state */
	if (pwrseq_system_state() != SYSTEM_S5_STATE) {
		LOG_DBG("Invalid sleep state");
		return false;
	}

	if (slp_m) {
		LOG_DBG("Invalid sleep signal status: %d", slp_m);
		return false;
	}

	return true;
}

bool check_s4s5_battonly_condition(void)
{
	LOG_DBG("%s", __func__);

	if (slp_m) {
		LOG_DBG("Invalid sleep signal status: %d", slp_m);
		return false;
	}

	return true;
}

bool check_s4s5_alwayson_condition(void)
{
	LOG_DBG("%s", __func__);

	if (slp_m) {
		LOG_DBG("Invalid sleep signal status: %d", slp_m);
		return false;
	}

	return true;
}

void deep_sx_enter(void)
{
	int ret;
	uint8_t sus_wrn;

	espihub_retrieve_vw(ESPI_VWIRE_SIGNAL_SUS_WARN, &sus_wrn);
	if (espihub_reset_status() && !sus_wrn) {
		gpio_write_pin(PM_DS3, 1);

		#ifdef CONFIG_SOC_FAMILY_MEC
		vci_enable();
		#endif

		ret = ack_deep_sleep_transition(SUS_WRN_LOW);
		if (ret) {
			LOG_WRN("%s:failed ack deep sleep transition %d",
					__func__, ret);
			return;
		}

		ret = wait_for_pin_monitor_vwire(PM_SLP_SUS, 0,
						 PM_SLP_SUS_TIMEOUT,
						 ESPI_VWIRE_SIGNAL_SUS_WARN,
						 ESPIHUB_VW_HIGH);
		if (ret) {
			LOG_WRN("wait for deep suspend failed %d", ret);
			return;
		}

		LOG_DBG("DeepSx enter got Slp Sus");
		gpio_write_pin(PM_RSMRST, 0);

		if ((dsw_mode() == ENABLE_IN_S5_DC) ||
		    (dsw_mode() == ENABLE_IN_S4S5_DC)) {
			LOG_INF("EC PowerOff");
			/* Assert A-Rail VR */
			gpio_write_pin(PM_DS3, 0);

			/* Delay for 100ms for RSMRST to ramp down */
			k_msleep(100);

			/* Assert A-Rail VR */
			gpio_write_pin(PM_DS3, 1);
		}
		sys_deep_sx = true;
	}
}

void deep_sx_exit(void)
{
	#ifdef CONFIG_SOC_FAMILY_MEC
	switch (vci_wake_reason()) {
	case VCI_POWER_BUTTON:
		/* If power button press detected by VCI, wake the system */
		LOG_INF("Power button DSx Wake");
		pwrbtn_trigger_wake();
		break;
	case VCI_AC:
		LOG_INF("AC DSx Wake");
		break;
	default:
		LOG_WRN("DSx wake not supported");
		break;
	}
	#endif
}
