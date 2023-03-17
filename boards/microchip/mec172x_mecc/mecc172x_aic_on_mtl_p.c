/*
 * Copyright (c) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <soc.h>
#include <zephyr/drivers/gpio.h>
#include "i2c_hub.h"
#include <zephyr/logging/log.h>
#include "gpio_ec.h"
#include "espi_hub.h"
#include "vci.h"
#include "board.h"
#include "board_config.h"
#include "mecc172x_aic_on_mtl_p.h"

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

/* APP-owned gpios */
struct gpio_ec_config mecc172x_cfg[] =  {
/*      Port Signal			Config       */
	{ PROCHOT,			GPIO_OUTPUT_LOW },
	{ RSMRST_PWRGD_G3SAF_P,		GPIO_INPUT },
	{ RSMRST_PWRGD_MAF_P,		GPIO_INPUT },
	{ PM_PWRBTN,			GPIO_OUTPUT_HIGH | GPIO_OPEN_DRAIN },
	{ PM_SLP_SUS,			GPIO_INPUT },
	{ VOL_UP,			GPIO_INPUT | GPIO_INT_EDGE_BOTH },
	{ WAKE_SCI,			GPIO_OUTPUT_HIGH | GPIO_OPEN_DRAIN },
	{ ALL_SYS_PWRGD,		GPIO_INPUT },
	{ PM_SLP_S0_CS,			GPIO_INPUT },
	{ KBC_SCROLL_LOCK,		GPIO_OUTPUT_LOW },
	{ KBC_NUM_LOCK,			GPIO_OUTPUT_LOW },
	{ PCH_PWROK,			GPIO_OUTPUT_LOW },
	{ PM_BATLOW,			GPIO_OUTPUT_HIGH | GPIO_OPEN_DRAIN },
	{ FAN_PWR_DISABLE_N,		GPIO_OUTPUT_HIGH },
	{ TYPEC_ALERT_2,		GPIO_INPUT },
	{ BC_ACOK,			GPIO_INPUT },
	{ PWRBTN_EC_IN_N,		GPIO_INPUT | GPIO_INT_EDGE_BOTH },
	{ TYPEC_ALERT_1,		GPIO_INPUT },
	{ PM_DS3,			GPIO_OUTPUT_HIGH },
	{ KBC_CAPS_LOCK,		GPIO_OUTPUT_LOW },
	{ BATT_ID_N,			GPIO_INPUT },
	{ VOL_DOWN,			GPIO_INPUT | GPIO_INT_EDGE_BOTH },
	{ SMC_LID,			GPIO_INPUT | GPIO_INT_EDGE_BOTH },
	{ STD_ADPT_CNTRL_GPIO,		GPIO_OUTPUT_LOW },
	{ STD_ADP_PRSNT,		GPIO_INPUT },
	{ PM_BAT_STATUS_LED1,		GPIO_OUTPUT_LOW },
	{ TYPEC_EC_SMBUS_ALERT_0_R,	GPIO_INPUT | GPIO_INT_EDGE_FALLING },
};

/* Any IO expanders pins should be defined here */
struct gpio_ec_config expander_cfg[] = {
#ifdef CONFIG_GPIO_PCA95XX
	{ SPD_PRSNT,		GPIO_INPUT },
	{ VIRTUAL_BAT,		GPIO_INPUT },
	{ VIRTUAL_DOCK,		GPIO_INPUT },
	{ PECI_OVER_ESPI,	GPIO_INPUT },
	{ PD_AIC_DETECT1,	GPIO_INPUT },
	{ PD_AIC_DETECT2,	GPIO_INPUT },
	{ TIMEOUT_DISABLE,	GPIO_INPUT },
	{ SOC_VR_CNTRL,		GPIO_INPUT },
	{ EC_M_2_SSD_PLN,	GPIO_OUTPUT_HIGH },
	{ RETIMER_BYPASS,	GPIO_INPUT },
	{ THERM_STRAP,		GPIO_INPUT },
#endif
};

/* This action is performed explicitly, just adding here as reference */
struct gpio_ec_config mecc172x_cfg_sus[] =  {
	{ PCH_PWROK,			GPIO_OUTPUT_LOW },
};

struct gpio_ec_config mecc172x_cfg_res[] =  {
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
	{ PWM_CH_08,	TACH_CH_00 }, /* CPU Fan */
};

void board_fan_dev_tbl_init(uint8_t *pmax_fan, struct fan_dev **pfan_tbl)
{
	*pfan_tbl = fan_tbl;
	*pmax_fan = ARRAY_SIZE(fan_tbl);
}

void board_therm_sensor_list_init(uint8_t therm_sensors[])
{
	switch (platformskutype) {
	case PLATFORM_MTL_P_SKUs:
	case PLATFORM_MTL_P_SBS_SKUs:
	case PLATFORM_MTL_M_SKUs:
		therm_sensors[ACPI_THRM_SEN_2] = ADC_CH_04;
		therm_sensors[ACPI_THRM_SEN_3] = ADC_CH_03;
		therm_sensors[ACPI_THRM_SEN_4] = ADC_CH_00;
		therm_sensors[ACPI_THRM_SEN_5] = ADC_CH_05;
		break;
	default:
		break;
	}
}
#endif


void update_platform_sku_type(void)
{
	switch (get_board_id()) {
	case BRD_ID_MTL_P_DDR5_SBS:
	case BRD_ID_MTL_P_DDR5_SBS_ERB:
	case BRD_ID_MTL_M_LP5_CONF1_ERB:
		platformskutype = PLATFORM_MTL_P_SBS_SKUs;
		pd_i2c_addr_set = MTL_P_I2C_SET1;
		break;
	case BRD_ID_MTL_P_LP5_T4:
	case BRD_ID_MTL_P_DDR5_SOLDERED_DTBT:
		platformskutype = PLATFORM_MTL_P_SKUs;
		pd_i2c_addr_set = MTL_P_I2C_SET1;
		break;
	case BRD_ID_MTL_P_LP5_T3:
	case BRD_ID_MTL_P_DDR5_B2B_HSIO:
		platformskutype = PLATFORM_MTL_P_SKUs;
		pd_i2c_addr_set = MTL_P_I2C_SET2;
		break;
	case BRD_ID_MTL_M_LP5_CONF1:
	case BRD_ID_MTL_M_LP5_CONF2:
	case BRD_ID_MTL_M_LP5_PMIC:
		platformskutype = PLATFORM_MTL_M_SKUs;
		break;
	default:
		platformskutype = PLATFORM_MTL_P_SKUs;
		break;
	}
}

int board_init(void)
{
	int ret;

	bgpo_disable();

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

	ret = gpio_configure_array(mecc172x_cfg, ARRAY_SIZE(mecc172x_cfg));
	if (ret) {
		LOG_ERR("%s: %d", __func__, ret);
		return ret;
	}

	/* In MAF, boot ROM already made this pin output and high, so we must
	 * keep it like that during the boot phase in order to avoid espi reset
	 */
	if (espihub_boot_mode() == FLASH_BOOT_MODE_MAF) {
		/* Ensure GPIO mode for pins reconfigure due to QMSPI device */
		gpio_force_configure_pin(RSMRST_PWRGD_MAF_P,
					 GPIO_INPUT | GPIO_PULL_UP);
		gpio_force_configure_pin(PM_RSMRST_MAF_P, GPIO_OUTPUT_HIGH);
	} else {
		gpio_configure_pin(RSMRST_PWRGD_G3SAF_P, GPIO_INPUT);
		gpio_configure_pin(PM_RSMRST_G3SAF_P, GPIO_OUTPUT_LOW);
	}

	switch (platformskutype) {
	case PLATFORM_MTL_P_SKUs:
	case PLATFORM_MTL_P_SBS_SKUs:
		break;
	case PLATFORM_MTL_M_SKUs:
		break;
	default:
		break;
	}

	return 0;
}

int board_suspend(void)
{
	int ret;

	ret = gpio_configure_array(mecc172x_cfg_sus,
				   ARRAY_SIZE(mecc172x_cfg_sus));
	if (ret) {
		LOG_ERR("%s: %d", __func__, ret);
		return ret;
	}

	return 0;
}

int board_resume(void)
{
	int ret;

	ret = gpio_configure_array(mecc172x_cfg_res,
				   ARRAY_SIZE(mecc172x_cfg_res));
	if (ret) {
		LOG_ERR("%s: %d", __func__, ret);
		return ret;
	}

	return 0;
}
