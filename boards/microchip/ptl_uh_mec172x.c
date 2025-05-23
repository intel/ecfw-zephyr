/*
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <soc.h>
#include <zephyr/drivers/gpio.h>
#include "i2c_hub.h"
#include <zephyr/logging/log.h>
#include "gpio_ec.h"
#include "espi_hub.h"
#include "board.h"
#include "board_config.h"
#include "ptl_uh_mec172x.h"
#include "vci.h"

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
struct gpio_ec_config mecc172x_cfg[] = {
	{ HOME_BUTTON,		GPIO_INPUT },
	{ EC_GPIO_002,		GPIO_DISCONNECTED },
	{ PM_SLP_SUS,		GPIO_DISCONNECTED },
	{ PM_PWRBTN,		GPIO_OUTPUT_HIGH | GPIO_OPEN_DRAIN },
	{ RSMRST_PWRGD_G3SAF_P,	GPIO_INPUT },
	{ RSMRST_PWRGD_MAF_P,	GPIO_INPUT },
	{ CPU_C10_GATE,		GPIO_INPUT },
	{ DNX_FORCE_RELOAD_EC,	GPIO_DISCONNECTED },
	{ EC_GPIO_015,		GPIO_DISCONNECTED },

	/* Not used */
	{ SLP_S0_PLT_EC_N,		GPIO_DISCONNECTED },

	{ ALL_SYS_PWRGD,	GPIO_INPUT },
	{ FAN_PWR_DISABLE_N,	GPIO_OUTPUT_HIGH },
	{ PCA9555_1_R_INT_N,	GPIO_INPUT },
	{ EC_GPIO_063,		GPIO_DISCONNECTED },
	{ EC_GPIO_067,		GPIO_DISCONNECTED },

	{ PCH_PWROK,		GPIO_OUTPUT_LOW | GPIO_OPEN_DRAIN },
	{ PWRBTN_EC_IN_N,	GPIO_INPUT | GPIO_INT_EDGE_BOTH },
	{ EC_GPIO_115,		GPIO_DISCONNECTED },
	{ KBC_CAPS_LOCK,	GPIO_OUTPUT_LOW },
	{ TYPEC_EC_SMBUS_ALERT_0_R, GPIO_INPUT | GPIO_INT_EDGE_FALLING },
	{ EC_PG3_EXIT,		GPIO_OUTPUT_LOW },

	{ KBD_BKLT_CTRL,	GPIO_OUTPUT_LOW },
	{ PM_BAT_STATUS_LED1,	GPIO_OUTPUT_LOW },
	{ PM_BAT_STATUS_LED2,	GPIO_OUTPUT_LOW },
	{ KBC_NUM_LOCK,		GPIO_OUTPUT_LOW },
	{ BC_ACOK,		GPIO_INPUT },
	{ BATT_ID_N,		GPIO_INPUT },
	{ PM_BATLOW,		GPIO_OUTPUT_LOW | GPIO_OPEN_DRAIN },

	{ PROCHOT,		GPIO_OUTPUT_HIGH | GPIO_OPEN_DRAIN },
	{ RETIMER_FORCE_PWR_BTP_EC_R,		GPIO_DISCONNECTED},
	{ EC_SLATEMODE_HALLOUT_SNSR_R, GPIO_INPUT | GPIO_INT_EDGE_BOTH  },
	{ PM_DS3,		GPIO_OUTPUT_LOW },
	{ EC_GPIO_240,		GPIO_DISCONNECTED },
	{ EC_PWRBTN_LED,	GPIO_OUTPUT_LOW },
	{ VOL_UP,		GPIO_INPUT | GPIO_INT_EDGE_BOTH },
	{ STD_ADP_PRSNT,	GPIO_INPUT },
	{ TOP_SWAP_OVERRIDE_GPIO,	GPIO_OUTPUT },
	{ EC_GPIO_245,		GPIO_DISCONNECTED },
	{ VOL_DOWN,		GPIO_INPUT | GPIO_INT_EDGE_BOTH },
	{ STD_ADPT_CNTRL_GPIO,	GPIO_OUTPUT_LOW },
	{ CATERR_LED_DRV,		GPIO_DISCONNECTED},

	/* Not used */
	{ SLP_S0_PLT_EC_N,		GPIO_DISCONNECTED },

};

/* Any IO expanders pins should be defined here */
struct gpio_ec_config expander_cfg[] = {
#ifdef CONFIG_GPIO_PCA95XX
	{ SPD_PRSNT,		GPIO_INPUT },
	{ VIRTUAL_BAT,		GPIO_INPUT },
	{ VIRTUAL_DOCK,		GPIO_INPUT },
	{ TIMEOUT_DISABLE,	GPIO_INPUT },
	{ SOC_VR_CNTRL,		GPIO_INPUT },
	{ EC_M_2_SSD_PLN,	GPIO_OUTPUT_HIGH },
	{ THERM_STRAP,		GPIO_INPUT },
	{ KBC_SCROLL_LOCK,	GPIO_OUTPUT_LOW },
	{ CS_INDICATE_LED,	GPIO_OUTPUT_LOW },
	{ C10_GATE_LED,		GPIO_OUTPUT_LOW },
#endif
};

struct gpio_ec_config mecc172x_cfg_sus[] =  {
};

struct gpio_ec_config mecc172x_cfg_res[] =  {
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
	case PLATFORM_PTL_UH_SKUs:
		therm_sensors[ACPI_THRM_SEN_2] = ADC_CH_07;
		therm_sensors[ACPI_THRM_SEN_3] = ADC_CH_04;
		therm_sensors[ACPI_THRM_SEN_4] = ADC_CH_05;
		therm_sensors[ACPI_THRM_SEN_5] = ADC_CH_06;
		break;
	default:
		break;
	}
}
#endif

void board_config_io_buffer(void)
{
	int ret;

	/* PS2 requires additional configuration not possible in pinmux */
	ret = gpio_interrupt_configure_pin(PS2_KB_DATA, GPIO_INT_EDGE_FALLING);
	if (ret) {
		LOG_ERR("Failed to enable PS2 KB interrupt");
	}
}

void update_platform_sku_type(void)
{
	switch (get_board_id()) {
	/* PTL UH Board ID List */
	case BRD_ID_PTL_UH_LP5x_T3_ERB:
	case BRD_ID_PTL_UH_LP5x_T3_RVP1:
	case BRD_ID_PTL_UH_LP5x_T4_RVP3:
		platformskutype = PLATFORM_PTL_UH_SKUs;
		pd_i2c_addr_set = PTL_UH_I2C_SET1;
		break;
	case BRD_ID_PTL_UH_LP5x_CAMM_DTBT_RVP2:
	case BRD_ID_PTL_UH_DDR5_T3_RVP4:
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

	LOG_WRN("%s", __func__);
	bgpo_disable();

	ret = gpio_init();
	if (ret) {
		LOG_ERR("Failed to initialize gpio devs: %d", ret);
		return ret;
	}

	LOG_WRN("%s about to initialize i2c0", __func__);
	ret = i2c_hub_config(I2C_0);
	if (ret) {
		return ret;
	}

	LOG_WRN("%s about to initialize i2c1", __func__);
	ret = i2c_hub_config(I2C_1);
	if (ret) {
		return ret;
	}

	LOG_WRN("%s about to read board id", __func__);
	ret = read_board_id();
	if (ret) {
		LOG_ERR("Failed to fetch brd id: %d", ret);
		return ret;
	}

	LOG_WRN("%s about to update sku", __func__);
	update_platform_sku_type();

	LOG_WRN("%s about to configure expander", __func__);
	ret = gpio_configure_array(expander_cfg, ARRAY_SIZE(expander_cfg));
	if (ret) {
		LOG_ERR("%s: %d", __func__, ret);
		return ret;
	}

	/* In PTL UH G3_SAF HW strap is not in the expander_cfg
	 * MEC172x has by default GPIO input disabled.
	 * Need to configure strap prior to decide boot mode
	 */
	gpio_configure_pin(G3_SAF_DETECT, GPIO_INPUT);

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
		gpio_force_configure_pin(PM_RSMRST_MAF_P, GPIO_OUTPUT_HIGH);
		/* LPM optimizations */
		gpio_force_configure_pin(G3_SAF_DETECT, GPIO_DISCONNECTED);
		gpio_force_configure_pin(PM_RSMRST_G3SAF_P, GPIO_DISCONNECTED);
	} else {
		gpio_configure_pin(RSMRST_PWRGD_G3SAF_P, GPIO_INPUT);
		gpio_configure_pin(PM_RSMRST_G3SAF_P, GPIO_OUTPUT_LOW);
	}

	board_config_io_buffer();

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
