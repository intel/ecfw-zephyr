/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include <zephyr/kernel.h>
#include <soc.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/adc.h>
#include "adc_sensors.h"
#include "board_config.h"
LOG_MODULE_REGISTER(adcsens, CONFIG_ADC_SENSORS_LOG_LEVEL);

/* For cases where ADC failure occurs during LPM exit, PLL takes 3ms to lock */
#define MAX_ADC_READ_RETRIES 3
#define ADC_RETRY_DELAY_MS   1

/* ADC device */
static const struct device *adc_dev;

static uint8_t adc_ch_bits;

int16_t adc_temp_val[ADC_CH_TOTAL];

static uint8_t num_of_adc_ch;


static void conv_adc_temp(uint16_t adc_raw_val, int16_t *temperature)
{
	/* Conversion lookup table index for 0 degree celsius temperature */
#define CONV_TBL_ZERO_C_IDX	80

	/* Conversion Lookup table maximum negative temperature */
#define MAX_NEGATIVE_TEMP	-40

	/* Conversion lookup table maximum positive temperature index */
#define MAX_POSITIVE_TEMP_IDX	330

	struct adc_temp_conv_table {
		int16_t temperature;
		uint16_t raw_val;
	};

	int cnt;

/**
 *
 * Thermistor ADC Voltage Calculation :
 * -------------------------------------------
 *
 * +--------------------------------------------------------+
 * |                                                        |
 * |    ADC_VREF(3.0v)        Theoretical Voltage           |
 * |         +                --------------------          |
 * |         |                                    Rth       |
 * |         |                  Vadc = Vref * -----------   |
 * |         |                                 (Rp + Rth)   |
 * |         X                                              |
 * |     Rp  X 22.6k                                        |
 * |         X                                              |
 * |         X                10 Bit ADC Value              |
 * |         |                --------------------          |
 * |         |    Vadc                      Vadc            |
 * |         +------>           Vadc_10b = ------ * 1024    |
 * |         |                              Vref            |
 * |         X                                              |
 * |   Rth   X  47k                                         |
 * |       +--->                                            |
 * |         X                12 Bit ADC Value              |
 * |         |                --------------------          |
 * |         |                              Vadc            |
 * |        +--+                Vadc_12b = ------ * 4096    |
 * |         ++                             Vref            |
 * |          +                                             |
 * |                                                        |
 * +--------------------------------------------------------+
 *
 *
 * This thermistor lookup table is driven for Murata thermistor part:
 * NCP15WB473F03RC with below circuit parameters:
 *
 *  Thermistor	> Rth: 47 kOhm [at 25c]- This value varies with temperature.
 *  Pull up Rest> Rp = 22.6 kOhm
 *  ADC_VREF	> Vref = 3.0 v
 *  Resolution	> 10 bit
 *
 * Any of above parameters ( / part) changed, this table has to be updated.
 * The temperature values are in order of 10, ranging from -40c to 125c.
 *
 * Note:
 * Thermistor datasheet provides the values of Rth only at few limited
 * temperature values ranging from -40c to 125c. Hence for more granular
 * range, liner interpolation is used to calculate the rest of the Rth
 * values.
 *
 */

	static const struct adc_temp_conv_table temp_conv_tbl[] = {
		/* Temp,RAW Value. -400 -> -40 deg, -395 -> -39.5 deg, etc */

		/* Negative temperature readings */
		{-400, 0x3F3}, {-395, 0x3F2}, {-390, 0x3F2}, {-385, 0x3F2},
		{-380, 0x3F1}, {-375, 0x3F1}, {-370, 0x3F0}, {-365, 0x3EF},
		{-360, 0x3EF}, {-355, 0x3EE}, {-350, 0x3EE}, {-345, 0x3ED},
		{-340, 0x3ED}, {-335, 0x3EC}, {-330, 0x3EB}, {-325, 0x3EB},
		{-320, 0x3EA}, {-315, 0x3E9}, {-310, 0x3E8}, {-305, 0x3E8},
		{-300, 0x3E7}, {-295, 0x3E6}, {-290, 0x3E5}, {-285, 0x3E4},
		{-280, 0x3E4}, {-275, 0x3E3}, {-270, 0x3E2}, {-265, 0x3E1},
		{-260, 0x3E0}, {-255, 0x3DF}, {-250, 0x3DE}, {-245, 0x3DD},
		{-240, 0x3DC}, {-235, 0x3DB}, {-230, 0x3DA}, {-225, 0x3D8},
		{-220, 0x3D7}, {-215, 0x3D6}, {-210, 0x3D5}, {-205, 0x3D4},
		{-200, 0x3D2}, {-195, 0x3D1}, {-190, 0x3D0}, {-185, 0x3CE},
		{-180, 0x3CD}, {-175, 0x3CB}, {-170, 0x3CA}, {-165, 0x3C8},
		{-160, 0x3C7}, {-155, 0x3C5}, {-150, 0x3C4}, {-145, 0x3C2},
		{-140, 0x3C0}, {-135, 0x3BF}, {-130, 0x3BD}, {-125, 0x3BB},
		{-120, 0x3B9}, {-115, 0x3B7}, {-110, 0x3B5}, {-105, 0x3B3},
		{-100, 0x3B1}, { -95, 0x3AF}, { -90, 0x3AD}, { -85, 0x3AB},
		{ -80, 0x3A9}, { -75, 0x3A7}, { -70, 0x3A4}, { -65, 0x3A2},
		{ -60, 0x3A0}, { -55, 0x39D}, { -50, 0x39B}, { -45, 0x398},
		{ -40, 0x396}, { -35, 0x393}, { -30, 0x391}, { -25, 0x38E},
		{ -20, 0x38B}, { -15, 0x389}, { -10, 0x386}, { -05, 0x383},

		/* Positive temperature readings */
		{  00, 0x380}, {  05, 0x37D}, {  10, 0x37A}, {  15, 0x377},
		{  20, 0x374}, {  25, 0x371}, {  30, 0x36E}, {  35, 0x36A},
		{  40, 0x367}, {  45, 0x364}, {  50, 0x360}, {  55, 0x35D},
		{  60, 0x359}, {  65, 0x356}, {  70, 0x352}, {  75, 0x34F},
		{  80, 0x34B}, {  85, 0x347}, {  90, 0x343}, {  95, 0x33F},
		{ 100, 0x33C}, { 105, 0x338}, { 110, 0x334}, { 115, 0x330},
		{ 120, 0x32C}, { 125, 0x327}, { 130, 0x323}, { 135, 0x31F},
		{ 140, 0x31B}, { 145, 0x317}, { 150, 0x312}, { 155, 0x30E},
		{ 160, 0x309}, { 165, 0x305}, { 170, 0x300}, { 175, 0x2FC},
		{ 180, 0x2F7}, { 185, 0x2F3}, { 190, 0x2EE}, { 195, 0x2E9},
		{ 200, 0x2E5}, { 205, 0x2E0}, { 210, 0x2DB}, { 215, 0x2D6},
		{ 220, 0x2D1}, { 225, 0x2CC}, { 230, 0x2C8}, { 235, 0x2C3},
		{ 240, 0x2BE}, { 245, 0x2B9}, { 250, 0x2B3}, { 255, 0x2AE},
		{ 260, 0x2A9}, { 265, 0x2A4}, { 270, 0x29F}, { 275, 0x29A},
		{ 280, 0x295}, { 285, 0x290}, { 290, 0x28A}, { 295, 0x285},
		{ 300, 0x280}, { 305, 0x27B}, { 310, 0x275}, { 315, 0x270},
		{ 320, 0x26B}, { 325, 0x265}, { 330, 0x260}, { 335, 0x25B},
		{ 340, 0x255}, { 345, 0x250}, { 350, 0x24B}, { 355, 0x345},
		{ 360, 0x240}, { 365, 0x23B}, { 370, 0x235}, { 375, 0x230},
		{ 380, 0x22B}, { 385, 0x226}, { 390, 0x220}, { 395, 0x21B},
		{ 400, 0x216}, { 405, 0x210}, { 410, 0x20B}, { 415, 0x206},
		{ 420, 0x201}, { 425, 0x1FB}, { 430, 0x1F6}, { 435, 0x1F1},
		{ 440, 0x1EC}, { 445, 0x1E7}, { 450, 0x1E1}, { 455, 0x1DC},
		{ 460, 0x1D7}, { 465, 0x1D2}, { 470, 0x1CD}, { 475, 0x1C8},
		{ 480, 0x1C3}, { 485, 0x1BE}, { 490, 0x1B9}, { 495, 0x1B4},
		{ 500, 0x1AF}, { 505, 0x1AA}, { 510, 0x1A5}, { 515, 0x1A1},
		{ 520, 0x19C}, { 525, 0x197}, { 530, 0x192}, { 535, 0x18E},
		{ 540, 0x189}, { 545, 0x184}, { 550, 0x180}, { 555, 0x17B},
		{ 560, 0x177}, { 565, 0x172}, { 570, 0x16E}, { 575, 0x169},
		{ 580, 0x165}, { 585, 0x160}, { 590, 0x15C}, { 595, 0x158},
		{ 600, 0x153}, { 605, 0x14F}, { 610, 0x14B}, { 615, 0x147},
		{ 620, 0x143}, { 625, 0x13F}, { 630, 0x13B}, { 635, 0x137},
		{ 640, 0x133}, { 645, 0x12F}, { 650, 0x12B}, { 655, 0x127},
		{ 660, 0x124}, { 665, 0x120}, { 670, 0x11C}, { 675, 0x118},
		{ 680, 0x115}, { 685, 0x111}, { 690, 0x10E}, { 695, 0x10A},
		{ 700, 0x107}, { 705, 0x103}, { 710, 0x100}, { 715, 0x0FC},
		{ 720, 0x0F9}, { 725, 0x0F6}, { 730, 0x0F3}, { 735, 0x0EF},
		{ 740, 0x0EC}, { 745, 0x0E9}, { 750, 0x0E6}, { 755, 0x0E3},
		{ 760, 0x0E0}, { 765, 0x0DD}, { 770, 0x0DA}, { 775, 0x0D7},
		{ 780, 0x0D4}, { 785, 0x0D1}, { 790, 0x0CE}, { 795, 0x0CC},
		{ 800, 0x0C9}, { 805, 0x0C6}, { 810, 0x0C4}, { 815, 0x0C1},
		{ 820, 0x0BE}, { 825, 0x0BC}, { 830, 0x0B9}, { 835, 0x0B7},
		{ 840, 0x0B4}, { 845, 0x0B2}, { 850, 0x0AF}, { 855, 0x0AD},
		{ 860, 0x0AB}, { 865, 0x0A8}, { 870, 0x0A6}, { 875, 0x0A4},
		{ 880, 0x0A2}, { 885, 0x09F}, { 890, 0x09D}, { 895, 0x09B},
		{ 900, 0x099}, { 905, 0x097}, { 910, 0x095}, { 915, 0x093},
		{ 920, 0x091}, { 925, 0x08F}, { 930, 0x08D}, { 935, 0x08B},
		{ 940, 0x089}, { 945, 0x087}, { 950, 0x085}, { 955, 0x084},
		{ 960, 0x082}, { 965, 0x080}, { 970, 0x07E}, { 975, 0x07D},
		{ 980, 0x07B}, { 985, 0x079}, { 990, 0x078}, { 995, 0x076},
		{1000, 0x075}, {1005, 0x073}, {1010, 0x071}, {1015, 0x070},
		{1020, 0x06E}, {1025, 0x06D}, {1030, 0x06B}, {1035, 0x06A},
		{1040, 0x069}, {1045, 0x067}, {1050, 0x066}, {1055, 0x064},
		{1060, 0x063}, {1065, 0x062}, {1070, 0x060}, {1075, 0x05F},
		{1080, 0x05E}, {1085, 0x05D}, {1090, 0x05B}, {1095, 0x05A},
		{1100, 0x059}, {1105, 0x058}, {1110, 0x057}, {1115, 0x055},
		{1120, 0x054}, {1125, 0x053}, {1130, 0x052}, {1135, 0x051},
		{1140, 0x050}, {1145, 0x04F}, {1150, 0x04E}, {1155, 0x04D},
		{1160, 0x04C}, {1165, 0x04B}, {1170, 0x04A}, {1175, 0x049},
		{1180, 0x048}, {1185, 0x047}, {1190, 0x046}, {1195, 0x045},
		{1200, 0x044}, {1205, 0x043}, {1210, 0x043}, {1215, 0x042},
		{1220, 0x041}, {1225, 0x040}, {1230, 0x03F}, {1235, 0x03E},
		{1240, 0x03E}, {1245, 0x03D}, {1250, 0x03C},
	};

	if (adc_raw_val > temp_conv_tbl[0].raw_val ||
		adc_raw_val < temp_conv_tbl[MAX_POSITIVE_TEMP_IDX].raw_val) {
		/* The sensor register is given out of range raw value.
		 * Hence the raw value cannot be converted to temperature.
		 *
		 * Since do not update or change the last successful
		 * converted temperature value.
		 *
		 * Just return false.
		 */
		LOG_ERR("Raw temperature of thermal sensor is out of range (0x%x)",
				adc_raw_val);
		return;
	}


	if (adc_raw_val > temp_conv_tbl[CONV_TBL_ZERO_C_IDX].raw_val) {
		/* -ve temperature */
		for (cnt = 0; cnt <= CONV_TBL_ZERO_C_IDX; cnt++) {
			if (adc_raw_val >= temp_conv_tbl[cnt].raw_val) {
				*temperature = temp_conv_tbl[cnt].temperature;
				break;
			}
		}
	} else {
		/* +ve temperature */
		for (cnt = CONV_TBL_ZERO_C_IDX; cnt <= MAX_POSITIVE_TEMP_IDX;
				cnt++) {
			if (adc_raw_val >= temp_conv_tbl[cnt].raw_val) {
				*temperature = temp_conv_tbl[cnt].temperature;
				break;
			}
		}
	}

	return;
}

int adc_sensors_init(uint8_t adc_channel_bits)
{
	int ret = 0;
	int adc_ch = ADC_CH_00;

	num_of_adc_ch = 0;
	adc_ch_bits = adc_channel_bits;

	if (adc_channel_bits == 0) {
		LOG_ERR("No adc sensor to enable");
		return -ENOTSUP;
	}

	adc_dev = DEVICE_DT_GET(ADC_CH_BASE);

	if (!device_is_ready(adc_dev)) {
		LOG_ERR("Sensor device not ready");
		return -ENOTSUP;
	}

	struct adc_channel_cfg adc_cfg;

	adc_cfg.gain = ADC_GAIN_1;
	adc_cfg.reference = ADC_REF_INTERNAL;
	adc_cfg.acquisition_time = ADC_ACQ_TIME_DEFAULT;
	adc_cfg.differential = 0;

	for (adc_ch = ADC_CH_00; adc_ch < ADC_CH_TOTAL; adc_ch++) {

		if (0 == (BIT(adc_ch) & adc_channel_bits)) {
			continue;
		}

		adc_cfg.channel_id = adc_ch;
		ret = adc_channel_setup(adc_dev, &adc_cfg);
		if (ret) {
			LOG_WRN("Sensor ch %d config failed", adc_ch);
		}

		/* Update count of adc channels enabled */
		num_of_adc_ch++;
	}

	return ret;
}


void adc_sensors_read_all(void)
{
	if (!adc_dev) {
		LOG_WRN("ADC: Sensor device Invalid");
		return;
	}

	int ret;
	int16_t adc_raw_val[num_of_adc_ch];
	uint8_t ch, ch_cnt = 0;

	const struct adc_sequence sequence = {
		.channels	= adc_ch_bits,
		.buffer		= adc_raw_val,
		.buffer_size	= sizeof(adc_raw_val),
		.resolution	= 10,
	};

	int retries = 0;

	do {
		ret = adc_read(adc_dev, &sequence);
		if (ret) {
			LOG_WRN("ADC Sensor reading failed %d", ret);
			k_msleep(ADC_RETRY_DELAY_MS);
		}

		retries++;
	} while (retries < MAX_ADC_READ_RETRIES && ret);

	if (ret) {
		LOG_ERR("ADC Sensor reading failed %d", ret);
	}

	for (ch = ADC_CH_00; ch < ADC_CH_TOTAL; ch++) {
		if (adc_ch_bits & BIT(ch)) {
			conv_adc_temp((adc_raw_val[ch_cnt++]), &adc_temp_val[ch]);
		}

		LOG_DBG("ADC Ch %d : %d", ch, adc_temp_val[ch]);
	}
}

