/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <soc.h>
#include "npcx_pin.h"
#include "common_npcx.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/dt-bindings/gpio/nuvoton-npcx-gpio.h>
#include "i2c_hub.h"
#include <zephyr/logging/log.h>
#include "gpio_ec.h"
#include "espi_hub.h"
#include "board.h"
#include "board_config.h"
#include "npcx4m8f_aic_on_ptl.h"

LOG_MODULE_DECLARE(board, CONFIG_BOARD_LOG_LEVEL);

uint8_t platformskutype;
uint8_t pd_i2c_addr_set;

/** @brief EC FW app owned gpios list.
 *
 * This list is not exhaustive, it do not include driver-owned pins,
 * the initialization is done as part of corresponding Zephyr pinmux driver.
 * BSP drivers are responsible to control gpios in soc power transitions and
 * system transitions.
 *
 * Note: Pins not assigned to any app function are identified with their
 * original pin number instead of signal
 *
 */

static const struct gpio_ec_config mecc_npcx9_cfg[] =  {
/*      Port Signal			Config       */
	{ PWRBTN_EC_IN_N,		GPIO_INPUT | GPIO_INT_EDGE_BOTH },
	{ SMC_LID,			GPIO_INPUT  | GPIO_INT_EDGE_BOTH },
	{ PM_SLP_SUS,			GPIO_INPUT },
	{ BATT_ID_N,			GPIO_INPUT },
	{ PM_SLP_S0_CS,			GPIO_INPUT },
	{ STD_ADPT_CNTRL_GPIO,		GPIO_OUTPUT_LOW },
	/* Not used
	 * { PS_ON_OUT,			GPIO_INPUT },
	 * { PS_ON_IN_EC_N,		GPIO_INPUT },
	 */

	{ PROCHOT,			GPIO_OUTPUT_HIGH | GPIO_OPEN_DRAIN },
	{ PCH_PWROK,			GPIO_OUTPUT_LOW | GPIO_OPEN_DRAIN },
	{ STD_ADP_PRSNT,		GPIO_INPUT },
	{ ALL_SYS_PWRGD,		GPIO_INPUT },
	{ PM_PWRBTN,			GPIO_OUTPUT_HIGH | GPIO_OPEN_DRAIN },
	{ RSMRST_PWRGD,			GPIO_INPUT | NPCX_GPIO_VOLTAGE_1P8 },
	{ EC_SLATEMODE_HALLOUT_SNSR_R,	GPIO_INPUT | GPIO_INT_EDGE_BOTH },
	{ FAN_PWR_DISABLE_N,		GPIO_OUTPUT_HIGH },
	{ EC_SLATEMODE_HALLOUT_SNSR_R,	GPIO_INPUT | GPIO_INT_EDGE_BOTH },
	{ EC_PCH_DEBUG,			GPIO_INPUT | GPIO_INT_EDGE_BOTH },
	{ WAKE_SCI,			GPIO_OUTPUT_HIGH | GPIO_OPEN_DRAIN },
	{ PM_BATLOW,			GPIO_OUTPUT_LOW | GPIO_OPEN_DRAIN },
	{ KBC_NUM_LOCK,			GPIO_OUTPUT_LOW },
	{ KBD_BKLT_CTRL,		GPIO_OUTPUT_LOW },
	{ KBC_CAPS_LOCK,		GPIO_OUTPUT_LOW },
	{ PM_DS3,			GPIO_OUTPUT_LOW },
	{ BC_ACOK,			GPIO_INPUT },
	{ TYPEC_EC_SMBUS_ALERT_0_R,	GPIO_INPUT | GPIO_INT_EDGE_BOTH },
	{ VOL_DOWN,			GPIO_INPUT | GPIO_INT_EDGE_BOTH },
	{ VOL_UP,			GPIO_INPUT | GPIO_INT_EDGE_BOTH },
	{ PM_RSMRST,			GPIO_OUTPUT_LOW },
	{ EC_PWRBTN_LED,		GPIO_OUTPUT_LOW },
	{ CPU_C10_GATE,			GPIO_INPUT },
};

/* Any IO expanders pins should be defined here */
static const struct gpio_ec_config expander_cfg[] = {
#ifdef CONFIG_GPIO_PCA95XX
	{ SPD_PRSNT,		GPIO_INPUT },
	{ MOD_TCSS1_DETECT,	GPIO_INPUT },
	{ MOD_TCSS2_DETECT,	GPIO_INPUT },
	{ VIRTUAL_BAT,		GPIO_INPUT },
	{ VIRTUAL_DOCK,		GPIO_INPUT },
	{ TIMEOUT_DISABLE,	GPIO_INPUT },
	{ NPCX_CAF_EMULATION,	GPIO_INPUT },
	{ EC_M_2_SSD_PLN,	GPIO_OUTPUT_HIGH },
	{ THERM_STRAP,		GPIO_INPUT },
	{ PD_AIC_DETECT_SLOT_ID,	GPIO_INPUT },
	{ KBC_SCROLL_LOCK,	GPIO_OUTPUT_LOW },
	{ CS_INDICATE_LED,	GPIO_OUTPUT_LOW },
	{ C10_GATE_LED,		GPIO_OUTPUT_LOW },
#endif
};

/* This action is performed explicitly, just adding here as reference */
static const struct gpio_ec_config mecc_npcx9_cfg_sus[] =  {
	{ PCH_PWROK,			GPIO_OUTPUT_LOW },
};

static const struct gpio_ec_config mecc_npcx9_cfg_res[] =  {
	{ PCH_PWROK,			GPIO_OUTPUT_HIGH },
};

#ifdef CONFIG_THERMAL_MANAGEMENT
/**
 * @brief Fan device table.
 *
 * This table lists the supported fan devices for board. By default, each
 * board is assigned one fan for CPU.
 */
static struct fan_dev fan_tbl[] = {
/*	PWM_CH_##	TACH_CH_##  */
	{ PWM_CH_00,	TACH_CH_01 }, /* CPU Fan */
};

void board_fan_dev_tbl_init(uint8_t *pmax_fan, struct fan_dev **pfan_tbl)
{
	*pfan_tbl = fan_tbl;
	*pmax_fan = ARRAY_SIZE(fan_tbl);
}

void board_therm_sensor_list_init(uint8_t therm_sensors[])
{
	switch (platformskutype) {
	case PLATFORM_PTL_UH_SKUs:
		therm_sensors[ACPI_THRM_SEN_2] = ADC_CH_01;
		therm_sensors[ACPI_THRM_SEN_4] = ADC_CH_00;
		therm_sensors[ACPI_THRM_SEN_5] = ADC_CH_03;
		break;
	default:
		break;
	}
}
#endif

/* ARL P has a different charger IC */
const uint16_t charger_settings_ptl_uh[] = {
	0x0E03,		/* Charger current limit 7.1744 */
	0x32FA,		/* Charger max system voltage 13.05V */
	0x2328,		/* Charger min system voltage 9V */
	0x3340,		/* Charger adapter current limit2 12.8A */
	0x4588,		/* Charger DC prochot 17.8A */
	0x0010,		/* Charger control0 register */
	0x028B,		/* Charger control1 register */
	0x7500,		/* Charger control2 register */
	0x0300,		/* Charger control3 register */
	0x0000,		/* CHARGER_CONTROL4_VAL */
	0x00C8,		/* CHARGER_MIN_INLIMIT 200mA*/
};


void update_platform_sku_type(void)
{
	switch (get_board_id()) {
	/* PTL UH Board ID List */
	case BRD_ID_PTL_UH_LP5x_T3_ERB:
	case BRD_ID_PTL_UH_LP5x_T3_RVP1:
	case BRD_ID_PTL_UH_DDR5_T3_RVP4:
		platformskutype = PLATFORM_PTL_UH_SKUs;
		pd_i2c_addr_set = PTL_UH_I2C_SET1;
		break;
	case BRD_ID_PTL_UH_LP5x_CAMM_DTBT_RVP2:
	case BRD_ID_PTL_UH_LP5x_T4_RVP3:
		platformskutype = PLATFORM_PTL_UH_SKUs;
		pd_i2c_addr_set = PTL_UH_I2C_SET2;
		break;
	default:
		platformskutype = PLATFORM_PTL_UH_SKUs;
		pd_i2c_addr_set = PTL_UH_I2C_SET1;
		break;
	}
}

int board_init(void)
{
	int ret;

	ret = gpio_init();
	if (ret) {
		LOG_ERR("Failed to initialize gpio devs: %d", ret);
		return ret;
	}

	ret = i2c_hub_config(I2C_0);
	if (ret) {
		return ret;
	}

	ret = i2c_hub_config(I2C_1);
	if (ret) {
		return ret;
	}

#if defined(CONFIG_NVME_RECOVERY) && !defined(CONFIG_I3C)
	ret = i2c_hub_config(I2C_PORT_NVME);
	if (ret) {
		LOG_ERR("i2c port for nvme not configured");
		return ret;
	}

	ret = i2c_hub_set_speed(I2C_PORT_NVME, NVME_I2C_SPEED);
	if (ret) {
		LOG_ERR("Failed to set speed, instance:%d", I2C_PORT_NVME);
		return ret;
	}
#endif

	LOG_WRN("%s about to read board id", __func__);
	ret = read_board_id();
	if (ret) {
		LOG_ERR("Failed to fetch brd id: %d", ret);
		return ret;
	}

	update_platform_sku_type();

	ret = gpio_configure_array(expander_cfg, ARRAY_SIZE(expander_cfg));
	if (ret) {
		LOG_ERR("%s: %d", __func__, ret);
		return ret;
	}

	detect_boot_mode();

	ret = gpio_configure_array(mecc_npcx9_cfg, ARRAY_SIZE(mecc_npcx9_cfg));
	if (ret) {
		LOG_ERR("%s: %d", __func__, ret);
		return ret;
	}

	return 0;
}

int board_suspend(void)
{
	int ret;

	ret = gpio_configure_array(mecc_npcx9_cfg_sus,
				   ARRAY_SIZE(mecc_npcx9_cfg_sus));
	if (ret) {
		LOG_ERR("%s: %d", __func__, ret);
		return ret;
	}

	return 0;
}

int board_resume(void)
{
	int ret;

	ret = gpio_configure_array(mecc_npcx9_cfg_res,
				   ARRAY_SIZE(mecc_npcx9_cfg_res));
	if (ret) {
		LOG_ERR("%s: %d", __func__, ret);
		return ret;
	}

	return 0;
}
