/*
 * Copyright (c) 2020 Intel Corporation
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
#include "adl_p_mec1501.h"
#include "vci.h"

LOG_MODULE_DECLARE(board, CONFIG_BOARD_LOG_LEVEL);

uint8_t platformskutype;

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
	{ EC_SPI_CS1_N,		GPIO_OUTPUT_HIGH},
	/* MIC privacy switch, not used */
	{ EC_GPIO_011,		GPIO_DISCONNECTED },
	{ RSMRST_PWRGD_MAF_P,	GPIO_INPUT },
	{ KBD_BKLT_CTRL,	GPIO_OUTPUT_LOW },
	{ PM_BAT_STATUS_LED1,	GPIO_OUTPUT_LOW },
	{ CPU_C10_GATE,		GPIO_INPUT },
	{ HOME_BUTTON,		GPIO_INPUT },
	/* EC control considered security issue. Savings 0.5 mA */
	{ EC_PCH_SPI_OE_N,	GPIO_DISCONNECTED },
	/* Not used. It's connected to 3.3K pull-up */
	{ PECI_MUX_CTRL,	GPIO_DISCONNECTED },
	{ SMC_LID,		GPIO_INPUT  | GPIO_INT_EDGE_BOTH },
	{ PM_BAT_STATUS_LED2,	GPIO_OUTPUT_LOW },
	{ PEG_PLI_N_DG2,	GPIO_OUTPUT_HIGH },
	/* PCH SYS_PWROK is connected to Silego. Savings 0.100 mA */
	{ EC_GPIO_043,		GPIO_DISCONNECTED },
	{ EC_GPIO_050,		GPIO_INPUT },
	/* IO expander 0, has nothing to be handled dynamically */
	{ PCA9555_0_R_INT_N,	GPIO_DISCONNECTED },
	{ STD_ADP_PRSNT,	GPIO_INPUT },
	{ ALL_SYS_PWRGD,	GPIO_INPUT },
	{ FAN_PWR_DISABLE_N,	GPIO_OUTPUT_HIGH },
	{ EC_SLATEMODE_HALLOUT_SNSR_R, GPIO_INPUT | GPIO_INT_EDGE_BOTH  },
	/* VREF2_ADC not connected, floating. Savings 0.6 mA */
	{ EC_GPIO_067,		GPIO_DISCONNECTED },
	/* MIC privacy ec, not used */
	{ EC_GPIO_100,		GPIO_DISCONNECTED },
	{ PM_PWRBTN,		GPIO_OUTPUT_HIGH | GPIO_OPEN_DRAIN },
	{ EC_SMI,			GPIO_DISCONNECTED },
	/* VTR2 strap not used by EC FW */
	{ EC_GPIO_104,		GPIO_DISCONNECTED },
	{ PCH_PWROK,		GPIO_OUTPUT_LOW | GPIO_OPEN_DRAIN },
	{ WAKE_SCI,		GPIO_OUTPUT_HIGH | GPIO_OPEN_DRAIN },
#ifdef CONFIG_DNX_EC_ASSISTED_TRIGGER
	{ DNX_FORCE_RELOAD_EC,	GPIO_OUTPUT_LOW},
#else
	{ DNX_FORCE_RELOAD_EC,	GPIO_DISCONNECTED},
#endif
	{ KBC_CAPS_LOCK,	GPIO_OUTPUT_LOW },
	{ TYPEC_EC_SMBUS_ALERT_0_R, GPIO_INPUT | GPIO_INT_EDGE_FALLING },
	{ PM_BATLOW,		GPIO_OUTPUT_LOW | GPIO_OPEN_DRAIN },
	{ CATERR_LED_DRV,	GPIO_DISCONNECTED },
	{ HB_NVDC_SEL,		GPIO_INPUT },
	{ BATT_ID_N,		GPIO_INPUT },
	{ PWRBTN_EC_IN_N,	GPIO_INPUT | GPIO_INT_EDGE_BOTH },

	{ STD_ADPT_CNTRL_GPIO,	GPIO_OUTPUT_LOW },
	{ BC_ACOK,		GPIO_INPUT },
	/* Path not connected. Savings 0.5 mA) */
	{ SX_EXIT_HOLDOFF_N,	GPIO_DISCONNECTED },
	/* Not used */
	{ PM_SLP_S0_CS,		GPIO_DISCONNECTED },
	{ RETIMER_FORCE_PWR_BTP_EC_R, GPIO_INPUT },
	{ PM_DS3,		GPIO_OUTPUT_LOW },
	/* Not used in new LPM design */
	{ WAKE_CLK,		GPIO_DISCONNECTED },
	{ VOL_UP,		GPIO_INPUT | GPIO_INT_EDGE_BOTH },
	{ TOP_SWAP_OVERRIDE_GPIO,	GPIO_OUTPUT },

	{ VOL_DOWN,		GPIO_INPUT | GPIO_INT_EDGE_BOTH },
	{ EC_PG3_EXIT,		GPIO_OUTPUT_LOW },
	{ EC_PWRBTN_LED,	GPIO_OUTPUT_LOW },
	{ PROCHOT,		GPIO_OUTPUT_HIGH | GPIO_OPEN_DRAIN },
	{ KBC_NUM_LOCK,		GPIO_OUTPUT_LOW },
};


/* APP-owned gpios for ADL-p*/
struct gpio_ec_config mecc1501_cfg_adl_p_divergence[] = {
	{ DG2_PRESENT,		GPIO_INPUT },
	{ PEG_RTD3_COLD_MOD_SW_R, GPIO_INPUT },
	{ PEG_PIM_DG2,		GPIO_OUTPUT_HIGH },
	/* Both PD controller share same interrupt line. Savings 0.2 mA*/
	{ TYPEC_EC_SMBUS_ALERT_1_R, GPIO_DISCONNECTED },
};

/* APP-owned gpios for ADL-M*/
struct gpio_ec_config mecc1501_cfg_adl_m_divergence[] = {
	{ CS_INDICATE_LED,		GPIO_OUTPUT_LOW },
	{ C10_GATE_LED,			GPIO_OUTPUT_LOW },
	{ RST_MECC,				GPIO_OUTPUT_HIGH },
	{ PM_SLP_S0_EC_N,		GPIO_OUTPUT_HIGH },
	{ KBC_SCROLL_LOCK_M,		GPIO_OUTPUT_LOW },
};

/* APP-owned gpios for ADL-N*/
struct gpio_ec_config mecc1501_cfg_adl_n_divergence[] = {
	{ PEG_PLI_N_DG2,	GPIO_DISCONNECTED },
	{ DG2_PRESENT,		GPIO_DISCONNECTED },
	{ TYPEC_EC_SMBUS_ALERT_1_R, GPIO_DISCONNECTED },
	{ PEG_RTD3_COLD_MOD_SW_R, GPIO_DISCONNECTED },
	{ PEG_PIM_DG2,		GPIO_DISCONNECTED },
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
	{ KBC_SCROLL_LOCK_P,	GPIO_OUTPUT_LOW },
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

void board_fan_dev_tbl_init(uint8_t *pmax_fan, struct fan_dev **pfan_tbl)
{
	*pfan_tbl = fan_tbl;
	*pmax_fan = ARRAY_SIZE(fan_tbl);
}

void board_therm_sensor_list_init(uint8_t therm_sensors[])
{
	switch (platformskutype) {
	case PLATFORM_ADL_P_SKUs:
	case PLATFORM_ADL_M_SKUs:
	case PLATFORM_ADL_N_SKUs:
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


void update_platform_sku_type(void)
{
	switch (get_board_id()) {
	case BRD_ID_ADL_P_ERB:
	case BRD_ID_ADL_P_LP4_T3_HSIO:
	case BRD_ID_ADL_P_DDR5_T3:
	case BRD_ID_ADL_P_LP5_T4:
	case BRD_ID_ADL_P_DDR4_T3:
	case BRD_ID_ADL_P_LP5_T3:
	case BRD_ID_ADL_P_LP4_BEP:
	case BRD_ID_ADL_P_LP5_AEP:
	case BRD_ID_ADL_P_GCS:
	case BRD_ID_ADL_P_DG2_384EU_AEP:
	case BRD_ID_ADL_P_DG2_128EU_AEP:
		platformskutype = PLATFORM_ADL_P_SKUs;
		break;
	case BRD_ID_ADL_M_LP4:
	case BRD_ID_ADL_M_LP5:
	case BRD_ID_ADL_M_LP5_2PMIC:
	case BRD_ID_ADL_M_RVP2A_PPV:
		platformskutype = PLATFORM_ADL_M_SKUs;
		break;
	case BRD_ID_ADL_N_ERB:
	case BRD_ID_ADL_N_LP5:
	case BRD_ID_ADL_N_DDR4:
	case BRD_ID_ADL_N_DDR5:
		platformskutype = PLATFORM_ADL_N_SKUs;
		break;
	default:
		platformskutype = PLATFORM_ADL_P_SKUs;
		break;
	}
}

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

#if DT_NODE_HAS_STATUS(DT_INST(2, microchip_xec_i2c), okay)
	ret = i2c_hub_config(I2C_2);
	if (ret) {
		LOG_ERR("i2c port not configured, Enable i2c port 5 in dts.");
	}
#endif
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

	/* Only for ADL-P the G3_SAF HW strap is not in the expander_cfg
	 * MEC15xx has by default GPIO input disabled.
	 * Need to configure strap prior to decide boot mode
	 */
	gpio_configure_pin(G3_SAF_DETECT, GPIO_INPUT);

	detect_boot_mode();

	ret = gpio_configure_array(mecc1501_cfg, ARRAY_SIZE(mecc1501_cfg));
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

		/* LPM optimizations */
		gpio_force_configure_pin(G3_SAF_DETECT, GPIO_DISCONNECTED);
		gpio_force_configure_pin(PM_RSMRST_G3SAF_P, GPIO_DISCONNECTED);
		gpio_force_configure_pin(EC_GPIO_002, GPIO_DISCONNECTED);
		gpio_force_configure_pin(EC_GPIO_056, GPIO_DISCONNECTED);
		gpio_force_configure_pin(EC_GPIO_223, GPIO_DISCONNECTED);
		gpio_force_configure_pin(EC_GPIO_224, GPIO_DISCONNECTED);
		gpio_force_configure_pin(EC_GPIO_016, GPIO_DISCONNECTED);
	} else {
		gpio_configure_pin(RSMRST_PWRGD_G3SAF_P, GPIO_INPUT);
		gpio_configure_pin(PM_RSMRST_G3SAF_P, GPIO_OUTPUT_LOW);
	}

	switch (platformskutype) {
	case PLATFORM_ADL_P_SKUs:
		gpio_configure_array(mecc1501_cfg_adl_p_divergence,
			  ARRAY_SIZE(mecc1501_cfg_adl_p_divergence));

		break;
	case PLATFORM_ADL_M_SKUs:
		gpio_configure_array(mecc1501_cfg_adl_m_divergence,
			  ARRAY_SIZE(mecc1501_cfg_adl_m_divergence));

		break;
	case PLATFORM_ADL_N_SKUs:
		gpio_configure_array(mecc1501_cfg_adl_n_divergence,
			  ARRAY_SIZE(mecc1501_cfg_adl_n_divergence));
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
