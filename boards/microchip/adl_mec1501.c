/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <soc.h>
#include <drivers/gpio.h>
#include <drivers/i2c.h>
#include <logging/log.h>
#include "gpio_ec.h"
#include "espi_hub.h"
#include "board.h"
#include "board_config.h"
#include "adl_mec1501.h"

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
struct gpio_ec_config mecc1501_cfg[] = {
	{ PM_SLP_SUS,		GPIO_INPUT },
	{ REAR_FAN_CTRL,	GPIO_OUTPUT_LOW },
	{ EC_GPIO_011,		GPIO_INPUT },
	{ RSMRST_PWRGD_G3SAF_P,	GPIO_INPUT },
	{ RSMRST_PWRGD_MAF_P,	GPIO_INPUT },
	{ ATX_DETECT,		GPIO_INPUT },
	{ KBD_BKLT_CTRL,	GPIO_INPUT },
	{ EC_GPIO_015,		GPIO_INPUT },
	{ EC_GPIO_023,		GPIO_INPUT },
	{ EC_GPIO_025,		GPIO_INPUT },
	{ SMC_LID,		GPIO_INPUT },
	{ EC_GPIO_035,		GPIO_INPUT },
	{ EC_GPIO_036,		GPIO_INPUT },
	{ SYS_PWROK,		GPIO_OUTPUT_LOW | GPIO_OPEN_DRAIN },
	{ EC_GPIO_050,		GPIO_INPUT },
	{ SLP_S0_PLT_EC_N,	GPIO_INPUT },
	{ EC_GPIO_052,		GPIO_INPUT },
	{ PM_RSMRST_G3SAF_P,	GPIO_OUTPUT_LOW },
/* In MAF, boot ROM already made this pin 1, so we must to keep it like that
 * during the boot phase in order to avoid ESPI_RESETs
 */
	{ PM_RSMRST_MAF_P,	GPIO_OUTPUT_HIGH },
	{ ALL_SYS_PWRGD,	GPIO_INPUT },
	{ FAN_PWR_DISABLE_N,	GPIO_OUTPUT_HIGH },
	{ KBC_SCROLL_LOCK,	GPIO_OUTPUT_LOW },
	{ HOME_BUTTON,		GPIO_INPUT },
	{ EC_GPIO_067,		GPIO_INPUT },
	{ EC_GPIO_100,		GPIO_INPUT },
	{ EC_GPIO_104,		GPIO_INPUT },
	{ PCH_PWROK,		GPIO_OUTPUT_LOW },
	{ WAKE_SCI,		GPIO_OUTPUT_HIGH | GPIO_OPEN_DRAIN },
	{ DNX_FORCE_RELOAD_EC,	GPIO_INPUT },
	{ KBC_CAPS_LOCK,	GPIO_OUTPUT_LOW },
	{ I2C_ALERT_P1,		GPIO_INPUT },
	/* PM_BATLOW NA for S platfroms, so make it input */
	{ PM_BATLOW,		GPIO_INPUT },
	{ CATERR_LED_DRV,	GPIO_INPUT },
	{ CS_INDICATE_LED,	GPIO_OUTPUT_LOW },
	{ C10_GATE_LED,		GPIO_OUTPUT_LOW },
	{ EC_GPIO_161,		GPIO_INPUT },
	{ PECI_MUX_CTRL,	GPIO_OUTPUT_LOW },
	{ PWRBTN_EC_IN_N,	GPIO_INPUT | GPIO_INT_EDGE_BOTH },
	{ SOC_VR_CNTRL_PE,	GPIO_OUTPUT_LOW },
	{ BC_ACOK,		GPIO_INPUT },
	{ PS_ON_OUT,		GPIO_INPUT },
	{ CPU_C10_GATE,		GPIO_INPUT },
	{ EC_SMI,		GPIO_OUTPUT_HIGH | GPIO_OPEN_DRAIN },
	{ PM_SLP_S0_CS,		GPIO_INPUT },
	{ PM_DS3,		GPIO_INPUT },
	{ WAKE_CLK,		GPIO_INPUT },
	{ VOL_UP,		GPIO_INPUT | GPIO_INT_EDGE_BOTH },
	{ TOP_SWAP_OVERRIDE,	GPIO_INPUT },
	{ VOL_DOWN,		GPIO_INPUT | GPIO_INT_EDGE_BOTH },
	{ PM_PWRBTN,		GPIO_OUTPUT_HIGH | GPIO_OPEN_DRAIN },
	{ PROCHOT,		GPIO_OUTPUT_HIGH },
	{ EC_M_2_SSD_PLN,	GPIO_OUTPUT_HIGH },
	{ KBC_NUM_LOCK,		GPIO_OUTPUT_LOW },
};

/* Any IO expanders pins should be defined here */
struct gpio_ec_config expander_cfg[] = {
#ifdef CONFIG_GPIO_PCA95XX
	{ SPD_PRSNT,		GPIO_INPUT },
	{ G3_SAF_DETECT,	GPIO_INPUT },
	{ THERM_STRAP,		GPIO_INPUT },
	{ PECI_OVER_ESPI,	GPIO_INPUT },
	{ TIMEOUT_DISABLE,	GPIO_INPUT },
#endif
};

struct gpio_ec_config mecc1501_cfg_sus[] =  {
};

struct gpio_ec_config mecc1501_cfg_res[] =  {
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
	{ PWM_CH_00,	TACH_CH_00 }, /* CPU Fan */
};


/**
 * @brief Thermal sensor table.
 *
 * This table lists the thermal sensors connected to the board and their
 * respective ACPI location field it is mapped to update the temperature value.
 */
static struct therm_sensor therm_sensor_tbl[] = {
};

static struct therm_sensor therm_sensor_tbl_adl_s[] = {
/*      ADC_CH_##	ACPI_LOC		dtt_threshold */
	{ADC_CH_05,	ACPI_THRM_SEN_VR,	{0} },	/* ADC_VR */
	{ADC_CH_06,	ACPI_THRM_SEN_DDR,	{0} },	/* ADC_DDR*/
};

void board_therm_sensor_tbl_init(u8_t *p_max_adc_sensors,
		struct therm_sensor **p_therm_sensor_tbl)
{
	switch (get_board_id()) {
	case BRD_ID_ADL_S_ERB:
	case BRD_ID_ADL_S_S01_TGP_H_SODIMM_DRR4:
	case BRD_ID_ADL_S_S01_TGP_H_SODIMM_DRR4_CRB:
	case BRD_ID_ADL_S_S01_TGP_H_SODIMM_DRR4_PPV:
	case BRD_ID_ADL_S_S02_TGP_H_SODIMM_DRR4_CRB:
	case BRD_ID_ADL_S_S03_ADP_S_UDIMM_DRR4_ERB1:
	case BRD_ID_ADL_S_S03_ADP_S_UDIMM_DRR4_CRB:
	case BRD_ID_ADL_S_S04_ADP_S_UDIMM_DRR4_CRB_EVCRB:
	case BRD_ID_ADL_S_S05_ADP_S_UDIMM_DRR4_CRB_CPV:
	case BRD_ID_ADL_S_S06_ADP_S_UDIMM_DRR4_CRB:
	case BRD_ID_ADL_S_S09_ADP_S_UDIMM_DRR4_CRB_PPV:
	case BRD_ID_ADL_S_S07_ADP_S_UDIMM_DRR4_CPV:
	case BRD_ID_ADL_S_S08_ADP_S_SODIMM_DRR5_CRB:
		*p_therm_sensor_tbl = therm_sensor_tbl_adl_s;
		*p_max_adc_sensors = ARRAY_SIZE(therm_sensor_tbl_adl_s);
		break;
	default:
		*p_therm_sensor_tbl = therm_sensor_tbl;
		*p_max_adc_sensors = ARRAY_SIZE(therm_sensor_tbl);
		break;
	}
}

void board_fan_dev_tbl_init(u8_t *pmax_fan, struct fan_dev **pfan_tbl)
{
	*pfan_tbl = fan_tbl;
	*pmax_fan = ARRAY_SIZE(fan_tbl);
}
#endif

static int configure_devices(void)
{
	int ret;
	struct device *i2c_dev0;

	i2c_dev0 = device_get_binding(I2C_BUS_0);

	if (!i2c_dev0) {
		LOG_ERR("%s not found", I2C_BUS_0);
		return -EINVAL;
	}

	ret = i2c_configure(i2c_dev0, I2C_SPEED_SET(I2C_SPEED_STANDARD) |
				I2C_MODE_MASTER);
	if (ret) {
		LOG_ERR("Error: %d failed to configure i2c device", ret);
		return ret;
	}

	return 0;
}

int board_init(void)
{
	int ret;

	/* Configure non gpio devices */
	ret = configure_devices();
	if (ret) {
		return ret;
	}

	ret = read_board_id();
	if (ret) {
		LOG_ERR("Failed to fetch brd id: %d", ret);
		return ret;
	}

	ret = gpio_init();
	if (ret) {
		LOG_ERR("Failed to initialize gpio devs: %d", ret);
		return ret;
	}

	ret = gpio_configure_array(expander_cfg, ARRAY_SIZE(expander_cfg));
	if (ret) {
		LOG_ERR("%s: %d", __func__, ret);
		return ret;
	}

	detect_boot_mode();

	ret = gpio_configure_array(mecc1501_cfg, ARRAY_SIZE(mecc1501_cfg));
	if (ret) {
		LOG_ERR("%s: %d", __func__, ret);
		return ret;
	}

	return 0;
}

int board_suspend(void)
{
	int ret;

	ret = gpio_configure_array(mecc1501_cfg_sus,
				   ARRAY_SIZE(mecc1501_cfg_sus));
	if (ret) {
		LOG_ERR("%s: %d", __func__, ret);
		return ret;
	}

	return 0;
}

int board_resume(void)
{
	int ret;

	ret = gpio_configure_array(mecc1501_cfg_res,
				   ARRAY_SIZE(mecc1501_cfg_res));
	if (ret) {
		LOG_ERR("%s: %d", __func__, ret);
		return ret;
	}

	return 0;
}
