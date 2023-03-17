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
#include "board.h"
#include "board_config.h"
#include "mtl_s_mec172x.h"
#include "vci.h"

uint8_t platformskutype;

LOG_MODULE_DECLARE(board, CONFIG_BOARD_LOG_LEVEL);
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
	{ PM_SLP_SUS,		GPIO_INPUT },
	{ EC_SPI_CS1_N,		GPIO_OUTPUT_HIGH},
	{ EC_GPIO_011,		GPIO_INPUT },
	{ RSMRST_PWRGD_G3SAF_P,	GPIO_INPUT },
	{ RSMRST_PWRGD_MAF_P,	GPIO_INPUT },
	{ ATX_DETECT,		GPIO_INPUT },
	{ KBD_BKLT_CTRL,	GPIO_INPUT },
	{ EC_GPIO_015,		GPIO_INPUT },
	{ POWER_STATE,		GPIO_OUTPUT_HIGH },
	{ EC_GPIO_023,		GPIO_INPUT },
	{ EC_GPIO_025,		GPIO_INPUT },
	{ SMC_LID,		GPIO_INPUT },
	{ EC_GPIO_036,		GPIO_INPUT },
	{ SYS_PWROK,		GPIO_OUTPUT_LOW | GPIO_OPEN_DRAIN },
	{ SLP_S0_PLT_EC_N,	GPIO_INPUT },
	{ EC_GPIO_052,		GPIO_INPUT },
	{ ALL_SYS_PWRGD,	GPIO_INPUT },
	{ FAN_PWR_DISABLE_N,	GPIO_OUTPUT_HIGH },
	{ KBC_SCROLL_LOCK,	GPIO_OUTPUT_LOW },
	{ EC_GPIO_067,		GPIO_INPUT },
	{ EC_GPIO_100,		GPIO_INPUT },
	{ PCH_PWROK,		GPIO_OUTPUT_LOW },
	{ WAKE_SCI,		GPIO_OUTPUT_HIGH | GPIO_OPEN_DRAIN },
	{ DNX_FORCE_RELOAD_EC,	GPIO_INPUT },
	{ KBC_CAPS_LOCK,	GPIO_OUTPUT_LOW },
	{ TYPEC_EC_SMBUS_ALERT_0_R,	GPIO_INPUT },
	/* PM_BATLOW NA for S platfroms, so make it input */
	{ PM_BATLOW,		GPIO_INPUT },
	{ CATERR_LED_DRV,	GPIO_INPUT },
	{ CS_INDICATE_LED,	GPIO_OUTPUT_LOW },
	{ PS_ON_IN_EC_N,	GPIO_INPUT },
	{ PECI_MUX_CTRL,	GPIO_OUTPUT_LOW },
	{ PWRBTN_EC_IN_N,	GPIO_INPUT | GPIO_INT_EDGE_BOTH },
	{ PS_ON_OUT,		GPIO_OUTPUT_HIGH },
	{ CPU_C10_GATE,		GPIO_INPUT },
	{ EC_SMI,		GPIO_OUTPUT_HIGH | GPIO_OPEN_DRAIN },
	{ PM_SLP_S0_CS,		GPIO_INPUT },
	{ RECOVERY_INDICATOR_N,	GPIO_OUTPUT_HIGH | GPIO_OPEN_DRAIN },
	{ WAKE_CLK,		GPIO_INPUT },
/* <work around> Volup/Down requires internal pull up because external
 * pull up is on wrong power rail, causing auto wake in connected standby.
 */
	{ VOL_UP,		GPIO_INPUT | GPIO_INT_EDGE_BOTH},
	{ TOP_SWAP_OVERRIDE_GPIO,	GPIO_OUTPUT },
	{ VOL_DOWN,		GPIO_INPUT | GPIO_INT_EDGE_BOTH},
	{ PROCHOT,		GPIO_OUTPUT_HIGH | GPIO_OPEN_DRAIN },
	{ EC_M_2_SSD_PLN,	GPIO_OUTPUT_HIGH },
	{ EC_S0IX_ENTRY_REQ,		GPIO_INPUT},
	{ EC_S0IX_ENTRY_ACK,		GPIO_OUTPUT_HIGH},
	{ EC_GPIO_036,		GPIO_DISCONNECTED },
};

/* APP-owned GPIOs for MTL-S CRB */
struct gpio_ec_config mecc172x_cfg_mtl_s_crb_divergence[] = {
	{ PM_PWRBTN_CRB,		GPIO_OUTPUT_HIGH | GPIO_OPEN_DRAIN },
	{ KBC_NUM_LOCK_CRB,		GPIO_OUTPUT_LOW },
};

/* APP-owned GPIOs for MTL-P DDR5 SBS*/
struct gpio_ec_config mecc172x_cfg_mtl_s_erb_divergence[] = {
	{ PM_PWRBTN_ERB,		GPIO_OUTPUT_HIGH | GPIO_OPEN_DRAIN },
	{ KBC_NUM_LOCK_ERB,		GPIO_OUTPUT_LOW },
};


/* Any IO expanders pins should be defined here */
struct gpio_ec_config expander_cfg[] = {
#ifdef CONFIG_GPIO_PCA95XX
	{ SPD_PRSNT,		GPIO_INPUT },
	{ DISPLAY_ID_0,		GPIO_INPUT },
	{ DISPLAY_ID_1,		GPIO_INPUT },
	{ DISPLAY_ID_2,		GPIO_INPUT },
	{ DISPLAY_ID_3,		GPIO_INPUT },
	{ THERM_STRAP,		GPIO_INPUT },
	{ G3_SAF_DETECT,		GPIO_INPUT },
	{ TIMEOUT_DISABLE,		GPIO_INPUT },
	{ ESPI_TESTCRD_DET,		GPIO_INPUT },
	{ MEM_OC_N,		GPIO_INPUT },
	{ PECI_OVER_ESPI,		GPIO_INPUT },

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
	case PLATFORM_MTL_S_ERB_SKUs:
	case PLATFORM_MTL_S_CRB_SKUs:
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
	ret = gpio_interrupt_configure_pin(PS2_MB_DATA, GPIO_INT_EDGE_FALLING);
	if (ret) {
		LOG_ERR("Failed to enable PS2 MB interrupt");
	}

	ret = gpio_interrupt_configure_pin(PS2_KB_DATA, GPIO_INT_EDGE_FALLING);
	if (ret) {
		LOG_ERR("Failed to enable PS2 KB interrupt");
	}
}

void update_platform_sku_type(void)
{
	switch (get_board_id()) {
	/* MTL S ERB PO */
	case BRD_ID_MTL_S_UDIMM_1DPC_ERB:
		platformskutype = PLATFORM_MTL_S_ERB_SKUs;
		break;

	case BRD_ID_MTL_S_UDIMM_1DPC_CRB:
	case BRD_ID_MTL_S_UDIMM_2DPC_ERB:
	case BRD_ID_MTL_S_SODIMM_1DPC_ERB:
	case BRD_ID_MTL_S_SODIMM_1DPC_CRB:
	case BRD_ID_MTL_S_OC_ERB:
	case BRD_ID_MTL_S_OC_EV_CRB:
	case BRD_ID_MTL_S_HSIO_RVP:
	case BRD_ID_MTL_S_SODIMM_2DPC_CRB:
	case BRD_ID_MTL_S_uATX_6L_CRB:
		platformskutype = PLATFORM_MTL_S_CRB_SKUs;
		break;
	default:
		platformskutype = PLATFORM_MTL_S_CRB_SKUs;
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
		gpio_force_configure_pin(PM_RSMRST_MAF_P, GPIO_OUTPUT_HIGH);
	} else {
		gpio_configure_pin(RSMRST_PWRGD_G3SAF_P, GPIO_INPUT);
		gpio_configure_pin(PM_RSMRST_G3SAF_P, GPIO_OUTPUT_LOW);
	}

	switch (platformskutype) {
	case PLATFORM_MTL_S_ERB_SKUs:
		gpio_configure_array(mecc172x_cfg_mtl_s_erb_divergence,
			  ARRAY_SIZE(mecc172x_cfg_mtl_s_erb_divergence));
		break;
	case PLATFORM_MTL_S_CRB_SKUs:
		gpio_configure_array(mecc172x_cfg_mtl_s_crb_divergence,
			  ARRAY_SIZE(mecc172x_cfg_mtl_s_crb_divergence));
		break;
	default:
		break;
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
