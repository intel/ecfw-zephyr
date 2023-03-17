/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <soc.h>
#include "gpio_ec.h"
#include <zephyr/logging/log.h>
#include "smc.h"
#include "sci.h"
#include "smchost.h"
#include "system.h"
#include "acpi.h"
#include "board_config.h"
#include "fan.h"
#include "scicodes.h"

LOG_MODULE_REGISTER(smc, CONFIG_SMCHOST_LOG_LEVEL);

#define ACPI_ATTR_READ_ONLY	0u
#define ACPI_ATTR_READ_WRITE	1u

static const bool acpi_tbl_attr[256] = {
	/* [0 / 0] uint8_t acpi_space; */
	ACPI_ATTR_READ_ONLY,

	/* [1 / 1] uint8_t acpi_remote_temp; */
	ACPI_ATTR_READ_ONLY,

	/* [2 / 2] uint8_t acpi_gpu_temp; */
	ACPI_ATTR_READ_ONLY,

	/* [3 / 3] struct acpi_status_flags acpi_flags; */
	ACPI_ATTR_READ_ONLY,

	/* [4 / 4] uint8_t acpi_smb_buffer[40]; */
	ACPI_ATTR_READ_WRITE,

	/* [5 / 5]  */
	ACPI_ATTR_READ_WRITE,

	/* [6 / 6]  */
	ACPI_ATTR_READ_WRITE,

	/* [7 / 7]  */
	ACPI_ATTR_READ_WRITE,

	/* [8 / 8]  */
	ACPI_ATTR_READ_WRITE,

	/* [9 / 9]  */
	ACPI_ATTR_READ_WRITE,

	/* [10 / 0A]  */
	ACPI_ATTR_READ_WRITE,

	/* [11 / 0B]  */
	ACPI_ATTR_READ_WRITE,

	/* [12 / 0C]  */
	ACPI_ATTR_READ_WRITE,

	/* [13 / 0D]  */
	ACPI_ATTR_READ_WRITE,

	/* [14 / 0E]  */
	ACPI_ATTR_READ_WRITE,

	/* [15 / 0F]  */
	ACPI_ATTR_READ_WRITE,

	/* [16 / 10]  */
	ACPI_ATTR_READ_WRITE,

	/* [17 / 11]  */
	ACPI_ATTR_READ_WRITE,

	/* [18 / 12]  */
	ACPI_ATTR_READ_WRITE,

	/* [19 / 13]  */
	ACPI_ATTR_READ_WRITE,

	/* [20 / 14]  */
	ACPI_ATTR_READ_WRITE,

	/* [21 / 15]  */
	ACPI_ATTR_READ_WRITE,

	/* [22 / 16]  */
	ACPI_ATTR_READ_WRITE,

	/* [23 / 17]  */
	ACPI_ATTR_READ_WRITE,

	/* [24 / 18]  */
	ACPI_ATTR_READ_WRITE,

	/* [25 / 19]  */
	ACPI_ATTR_READ_WRITE,

	/* [26 / 1A]  */
	ACPI_ATTR_READ_WRITE,

	/* [27 / 1B]  */
	ACPI_ATTR_READ_WRITE,

	/* [28 / 1C]  */
	ACPI_ATTR_READ_WRITE,

	/* [29 / 1D]  */
	ACPI_ATTR_READ_WRITE,

	/* [30 / 1E]  */
	ACPI_ATTR_READ_WRITE,

	/* [31 / 1F]  */
	ACPI_ATTR_READ_WRITE,

	/* [32 / 20]  */
	ACPI_ATTR_READ_WRITE,

	/* [33 / 21]  */
	ACPI_ATTR_READ_WRITE,

	/* [34 / 22]  */
	ACPI_ATTR_READ_WRITE,

	/* [35 / 23]  */
	ACPI_ATTR_READ_WRITE,

	/* [36 / 24]  */
	ACPI_ATTR_READ_WRITE,

	/* [37 / 25]  */
	ACPI_ATTR_READ_WRITE,

	/* [38 / 26]  */
	ACPI_ATTR_READ_WRITE,

	/* [39 / 27]  */
	ACPI_ATTR_READ_WRITE,

	/* [40 / 28]  */
	ACPI_ATTR_READ_WRITE,

	/* [41 / 29]  */
	ACPI_ATTR_READ_WRITE,

	/* [42 / 2A]  */
	ACPI_ATTR_READ_WRITE,

	/* [43 / 2B]  */
	ACPI_ATTR_READ_WRITE,

	/* [44 / 2C] uint8_t acpi_thermal_policy; */
	ACPI_ATTR_READ_WRITE,

	/* [45 / 2D] uint8_t acpi_passive_temp; */
	ACPI_ATTR_READ_WRITE,

	/* [46 / 2E] uint8_t acpi_active_temp; */
	ACPI_ATTR_READ_WRITE,

	/* [47 / 2F] uint8_t acpi_crit_temp; */
	ACPI_ATTR_READ_WRITE,

	/* [48 / 30] struct acpi_status2_flags acpi_flags2; */
	ACPI_ATTR_READ_ONLY,

	/* [49 / 31] uint8_t acpi_hotkey_scan; */
	ACPI_ATTR_READ_ONLY,

	/* [50 / 32] struct acpi_bat_flags acpi_bat0_flags; */
	ACPI_ATTR_READ_ONLY,

	/* [51 / 33] uint8_t acpi_bat0_rate; */
	ACPI_ATTR_READ_ONLY,

	/* [52 / 34] uint8_t acpi_bat0_chg_sts; */
	ACPI_ATTR_READ_ONLY,

	/* [53 / 35] uint8_t acpi_bat0_volts; */
	ACPI_ATTR_READ_ONLY,

	/* [54 / 36] struct acpi_bat_flags acpi_bat1_flags; */
	ACPI_ATTR_READ_ONLY,

	/* [55 / 37] uint8_t acpi_bat1_rate; */
	ACPI_ATTR_READ_ONLY,

	/* [56 / 38] uint8_t acpi_bat1_chg_sts; */
	ACPI_ATTR_READ_ONLY,

	/* [57 / 39] uint8_t acpi_bat1_volts; */
	ACPI_ATTR_READ_ONLY,

	/* [58 / 3A] uint8_t acpi_host_command; */
	ACPI_ATTR_READ_WRITE,

	/* [59 / 3B] uint8_t acpi_lux_l; */
	ACPI_ATTR_READ_ONLY,

	/* [60 / 3C] uint8_t acpi_lux_h; */
	ACPI_ATTR_READ_ONLY,

	/* [61 / 3D] uint8_t acpi_als_raw_ch0; */
	ACPI_ATTR_READ_ONLY,

	/* [62 / 3E] uint8_t acpi_als_raw_ch1; */
	ACPI_ATTR_READ_ONLY,

	/* [63 / 3F] uint8_t acpi_new_card_dt_st; */
	ACPI_ATTR_READ_ONLY,

	/* [64 / 40] uint8_t acpi_fan_capability; */
	ACPI_ATTR_READ_WRITE,

	/* [65 / 41] uint8_t acpi_fan_idx; */
	ACPI_ATTR_READ_WRITE,

	/* [66 / 42] uint8_t free0; */
	ACPI_ATTR_READ_WRITE,

	/* [67 / 43] uint8_t acpi_pwm_init_val; */
	ACPI_ATTR_READ_WRITE,

	/* [68 / 44] uint8_t acpi_pwm_end_val; */
	ACPI_ATTR_READ_WRITE,

	/* [69 / 45] uint8_t acpi_pwm_step; */
	ACPI_ATTR_READ_WRITE,

	/* [70 / 46] uint8_t acpi_fan_opr_mode; */
	ACPI_ATTR_READ_WRITE,

	/* [71 / 47] uint16_t acpi_fan_rpm_high_threshold [LSB]; */
	ACPI_ATTR_READ_WRITE,

	/* [72 / 48] uint16_t acpi_fan_rpm_high_threshold [MSB]; */
	ACPI_ATTR_READ_WRITE,

	/* [73 / 49] uint16_t acpi_fan_rpm_low_threshold [LSB]; */
	ACPI_ATTR_READ_WRITE,

	/* [74 / 4A] uint16_t acpi_fan_rpm_low_threshold [MSB]; */
	ACPI_ATTR_READ_WRITE,

	/* [75 / 4B] uint8_t fan_rpm_trip_status; */
	ACPI_ATTR_READ_WRITE,

	/* [76 / 4C] uint16_t free1; [LSB] */
	ACPI_ATTR_READ_ONLY,

	/* [77 / 4D] uint16_t free1; [MSB] */
	ACPI_ATTR_READ_ONLY,

	/* [78 / 4E] struct acpi_power_source acpi_pwr_src; */
	ACPI_ATTR_READ_WRITE,

	/* [79 / 4F]  */
	ACPI_ATTR_READ_WRITE,

	/* [80 / 50] uint8_t acpi_temp_snsr_select; */
	ACPI_ATTR_READ_WRITE,

	/* [81 / 51] uint8_t acpi_temp_thrshld_h_lsb; */
	ACPI_ATTR_READ_WRITE,

	/* [82 / 52] uint8_t acpi_temp_thrshld_h_msb; */
	ACPI_ATTR_READ_WRITE,

	/* [83 / 53] uint8_t acpi_temp_thrshld_l_lsb; */
	ACPI_ATTR_READ_WRITE,

	/* [84 / 54] uint8_t acpi_temp_thrshld_l_msb; */
	ACPI_ATTR_READ_WRITE,

	/* [85 / 55] uint8_t acpi_therm_snsr_sts_lsb; */
	ACPI_ATTR_READ_WRITE,

	/* [86 / 56] uint8_t acpi_therm_snsr_sts_msb; */
	ACPI_ATTR_READ_WRITE,

	/* [87 / 57] uint8_t acpi_bat0_design_cap_l; */
	ACPI_ATTR_READ_ONLY,

	/* [88 / 58] uint8_t acpi_bat0_design_cap_h; */
	ACPI_ATTR_READ_ONLY,

	/* [89 / 59] uint8_t acpi_bat0_rem_cap_l; */
	ACPI_ATTR_READ_ONLY,

	/* [90 / 5A] uint8_t acpi_bat0_rem_cap_h; */
	ACPI_ATTR_READ_ONLY,

	/* [91 / 5B] uint8_t acpi_bat0_full_chg_l; */
	ACPI_ATTR_READ_ONLY,

	/* [92 / 5C] uint8_t acpi_bat0_full_chg_h; */
	ACPI_ATTR_READ_ONLY,

	/* [93 / 5D] uint8_t acpi_bat0_v_l; */
	ACPI_ATTR_READ_ONLY,

	/* [94 / 5E] uint8_t acpi_bat0_v_h; */
	ACPI_ATTR_READ_ONLY,

	/* [95 / 5F] uint8_t acpi_bat0_i_dischg_l; */
	ACPI_ATTR_READ_ONLY,

	/* [96 / 60] uint8_t acpi_bat0_i_dischg_h; */
	ACPI_ATTR_READ_ONLY,

	/* [97 / 61] uint8_t acpi_bat0_i_chg_l; */
	ACPI_ATTR_READ_ONLY,

	/* [98 / 62] uint8_t acpi_bat0_i_chg_h; */
	ACPI_ATTR_READ_ONLY,

	/* [99 / 63] uint8_t acpi_bat1_rem_cap_l; */
	ACPI_ATTR_READ_ONLY,

	/* [100 / 64] uint8_t acpi_bat1_rem_cap_h; */
	ACPI_ATTR_READ_ONLY,

	/* [101 / 65] uint8_t acpi_bat1_full_chg_l; */
	ACPI_ATTR_READ_ONLY,

	/* [102 / 66] uint8_t acpi_bat1_full_chg_h; */
	ACPI_ATTR_READ_ONLY,

	/* [103 / 67] uint8_t acpi_bat1_v_l; */
	ACPI_ATTR_READ_ONLY,

	/* [104 / 68] uint8_t acpi_bat1_v_h; */
	ACPI_ATTR_READ_ONLY,

	/* [105 / 69] uint8_t acpi_bat1_i_dischg_l; */
	ACPI_ATTR_READ_ONLY,

	/* [106 / 6A] uint8_t acpi_bat1_i_dischg_h; */
	ACPI_ATTR_READ_ONLY,

	/* [107 / 6B] uint8_t acpi_bat1_i_chg_l; */
	ACPI_ATTR_READ_ONLY,

	/* [108 / 6C] uint8_t acpi_bat1_i_chg_h; */
	ACPI_ATTR_READ_ONLY,

	/* [109 / 6D] uint8_t acpi_vmin_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [110 / 6E] uint8_t acpi_vmin_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [111 / 6F] uint8_t acpi_prop_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [112 / 70] uint8_t acpi_prop_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [113 / 71] uint8_t acpi_apkp_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [114 / 72] uint8_t acpi_apkp_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [115 / 73] uint8_t acpi_cpu_fan_rpm_l; */
	ACPI_ATTR_READ_ONLY,

	/* [116 / 74] uint8_t acpi_cpu_fan_rpm_h; */
	ACPI_ATTR_READ_ONLY,

	/* [117 / 75] uint8_t acpi_apkt_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [118 / 76] uint8_t acpi_apkt_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [119 / 77] uint8_t acpi_device_id; */
	ACPI_ATTR_READ_ONLY,

	/* [120 / 78] struct acpi_concept_flags acpi_concept_flags1; */
	ACPI_ATTR_READ_WRITE,

	/* [121 / 79] uint8_t free2; */
	ACPI_ATTR_READ_ONLY,

	/* [122 / 7A] uint8_t acpi_avol_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [123 / 7B] uint8_t acpi_avol_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [124 / 7C] uint8_t acpi_cpu_gfx_timer; */
	ACPI_ATTR_READ_WRITE,

	/* [125 / 7D] uint8_t acpi_acur_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [126 / 7E] uint8_t acpi_acur_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [127 / 7F] uint8_t acpi_dimm1_temp; */
	ACPI_ATTR_READ_ONLY,

	/* [128 / 80] uint8_t acpi_cpu_pch_max_temp; */
	ACPI_ATTR_READ_ONLY,

	/* [129 / 81] uint8_t acpi_pch_dts_temp; */
	ACPI_ATTR_READ_ONLY,

	/* [130 / 82] uint8_t acpi_cpu_dts_low; */
	ACPI_ATTR_READ_ONLY,

	/* [131 / 83] uint8_t acpi_cpu_dts_high; */
	ACPI_ATTR_READ_ONLY,

	/* [132 / 84] uint8_t acpi_npwr_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [133 / 85] uint8_t acpi_npwr_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [134 / 86] uint8_t acpi_lsoC; */
	ACPI_ATTR_READ_ONLY,

	/* [135 / 87] uint8_t acpi_artg_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [136 / 88] uint8_t acpi_artg_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [137 / 89] uint8_t acpi_ctype_value; */
	ACPI_ATTR_READ_WRITE,

	/* [138 / 8A] uint8_t acpi_ap01; */
	ACPI_ATTR_READ_ONLY,

	/* [139 / 8B] uint8_t acpi_ap02; */
	ACPI_ATTR_READ_ONLY,

	/* [140 / 8C] uint8_t acpi_ap10; */
	ACPI_ATTR_READ_ONLY,

	/* [141 / 8D] uint8_t acpi_pbss_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [142 / 8E] uint8_t acpi_pbss_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [143 / 8F] uint8_t acpi_bixcc_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [144 / 90] uint8_t acpi_bixcc_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [145 / 91] uint8_t acpi_sen1_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [146 / 92] uint8_t acpi_sen1_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [147 / 93] uint8_t acpi_sen2_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [148 / 94] uint8_t acpi_sen2_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [149 / 95] uint8_t acpi_sen3_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [150 / 96] uint8_t acpi_sen3_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [151 / 97] uint8_t acpi_sen4_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [152 / 98] uint8_t acpi_sen4_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [153 / 99] uint8_t acpi_sen5_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [154 / 9A] uint8_t acpi_sen5_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [155 / 9B] uint32_t acpi_repeat_cycles; */
	ACPI_ATTR_READ_WRITE,

	/* [156 / 9C]  */
	ACPI_ATTR_READ_WRITE,

	/* [157 / 9D]  */
	ACPI_ATTR_READ_WRITE,

	/* [158 / 9E]  */
	ACPI_ATTR_READ_WRITE,

	/* [159 / 9F] uint8_t acpi_repeat_period; */
	ACPI_ATTR_READ_WRITE,

	/* [160 / A0] uint8_t acpi_stop_on_err; */
	ACPI_ATTR_READ_WRITE,

	/* [161 / A1] uint8_t acpi_peci_packet[20]; */
	ACPI_ATTR_READ_WRITE,

	/* [162 / A2]  */
	ACPI_ATTR_READ_WRITE,

	/* [163 / A3]  */
	ACPI_ATTR_READ_WRITE,

	/* [164 / A4]  */
	ACPI_ATTR_READ_WRITE,

	/* [165 / A5]  */
	ACPI_ATTR_READ_WRITE,

	/* [166 / A6]  */
	ACPI_ATTR_READ_WRITE,

	/* [167 / A7]  */
	ACPI_ATTR_READ_WRITE,

	/* [168 / A8]  */
	ACPI_ATTR_READ_WRITE,

	/* [169 / A9]  */
	ACPI_ATTR_READ_WRITE,

	/* [170 / AA]  */
	ACPI_ATTR_READ_WRITE,

	/* [171 / AB]  */
	ACPI_ATTR_READ_WRITE,

	/* [172 / AC]  */
	ACPI_ATTR_READ_WRITE,

	/* [173 / AD]  */
	ACPI_ATTR_READ_WRITE,

	/* [174 / AE]  */
	ACPI_ATTR_READ_WRITE,

	/* [175 / AF]  */
	ACPI_ATTR_READ_WRITE,

	/* [176 / B0]  */
	ACPI_ATTR_READ_WRITE,

	/* [177 / B1]  */
	ACPI_ATTR_READ_WRITE,

	/* [178 / B2]  */
	ACPI_ATTR_READ_WRITE,

	/* [179 / B3]  */
	ACPI_ATTR_READ_WRITE,

	/* [180 / B4]  */
	ACPI_ATTR_READ_WRITE,

	/* [181 / B5] uint8_t acpi_turbo_pl2_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [182 / B6] uint8_t acpi_turbo_pl2_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [183 / B7] uint8_t acpi_turbo_tdp_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [184 / B8] uint8_t acpi_turbo_tdp_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [185 / B9] uint8_t acpi_plt_pwr_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [186 / BA] uint8_t acpi_plt_pwr_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [187 / BB] uint8_t acpi_peci_status; */
	ACPI_ATTR_READ_ONLY,

	/* [188 / BC] uint32_t acpi_peci_err; */
	ACPI_ATTR_READ_ONLY,

	/* [189 / BD]  */
	ACPI_ATTR_READ_ONLY,

	/* [190 / BE]  */
	ACPI_ATTR_READ_ONLY,

	/* [191 / BF]  */
	ACPI_ATTR_READ_ONLY,

	/* [192 / C0] uint8_t acpi_cpu_gt_vr_temp; */
	ACPI_ATTR_READ_ONLY,

	/* [193 / C1] uint8_t acpi_mincard_temp; */
	ACPI_ATTR_READ_ONLY,

	/* [194 / C2] uint8_t free3[2]; */
	ACPI_ATTR_READ_ONLY,

	/* [195 / C3]  */
	ACPI_ATTR_READ_WRITE,

	/* [196 / C4] uint32_t acpi_wake_timer; */
	ACPI_ATTR_READ_WRITE,

	/* [197 / C5]  */
	ACPI_ATTR_READ_WRITE,

	/* [198 / C6]  */
	ACPI_ATTR_READ_WRITE,

	/* [199 / C7]  */
	ACPI_ATTR_READ_WRITE,

	/* [200 / C8] uint8_t acpi_dev_pwr_cntrl; */
	ACPI_ATTR_READ_WRITE,

	/* [201 / C9] uint8_t acpi_dis_btn_sci; */
	ACPI_ATTR_READ_WRITE,

	/* [202 / CA] uint8_t acpi_unused_v[7]; */
	ACPI_ATTR_READ_ONLY,

	/* [203 / CB]  */
	ACPI_ATTR_READ_ONLY,

	/* [204 / CC]  */
	ACPI_ATTR_READ_ONLY,

	/* [205 / CD]  */
	ACPI_ATTR_READ_ONLY,

	/* [206 / CE]  */
	ACPI_ATTR_READ_ONLY,

	/* [207 / CF]  */
	ACPI_ATTR_READ_ONLY,

	/* [208 / D0]  */
	ACPI_ATTR_READ_ONLY,

	/* [209 / D1] uint8_t acpi_bat_1_design_cap_l; */
	ACPI_ATTR_READ_ONLY,

	/* [210 / D2] uint8_t acpi_bat_1_design_cap_h; */
	ACPI_ATTR_READ_ONLY,

	/* [211 / D3] uint8_t acpi_bat0_design_v_l; */
	ACPI_ATTR_READ_ONLY,

	/* [212 / D4] uint8_t acpi_bat0_design_v_h; */
	ACPI_ATTR_READ_ONLY,

	/* [213 / D5] uint8_t acpi_bat1_design_v_l; */
	ACPI_ATTR_READ_ONLY,

	/* [214 / D6] uint8_t acpi_bat1_design_v_h; */
	ACPI_ATTR_READ_ONLY,

	/* [215 / D7] uint8_t acpi_bat0_pmax_L; */
	ACPI_ATTR_READ_ONLY,

	/* [216 / D8] uint8_t acpi_bat0_pmax_H; */
	ACPI_ATTR_READ_ONLY,

	/* [217 / D9] uint8_t acpi_bat_1_pmax_L; */
	ACPI_ATTR_READ_ONLY,

	/* [218 / DA] uint8_t acpi_bat_1_pmax_H; */
	ACPI_ATTR_READ_ONLY,

	/* [219 / DB] uint8_t batt_threshold; */
	ACPI_ATTR_READ_WRITE,

	/* [220 / DC] uint8_t batt_trip_point_lsb; */
	ACPI_ATTR_READ_WRITE,

	/* [221 / DD] uint8_t batt_trip_point_msb; */
	ACPI_ATTR_READ_WRITE,

	/* [222 / DE] uint8_t kb_bklt_pwm_duty; */
	ACPI_ATTR_READ_WRITE,

	/* [223 / DF] uint8_t batt_chrg_lmt_lsb; */
	ACPI_ATTR_READ_WRITE,

	/* [224 / E0] uint8_t batt_chrg_lmt_msb; */
	ACPI_ATTR_READ_WRITE,

	/* [225 / E1] uint8_t thermistor1; */
	ACPI_ATTR_READ_ONLY,

	/* [226 / E2] uint8_t thermistor2; */
	ACPI_ATTR_READ_ONLY,

	/* [227 / E3] uint8_t thermistor3; */
	ACPI_ATTR_READ_ONLY,

	/* [228 / E4] uint8_t thermistor4; */
	ACPI_ATTR_READ_ONLY,

	/* [229 / E5] uint8_t thermistor5; */
	ACPI_ATTR_READ_ONLY,

	/* [230 / E6] uint8_t thermistor6; */
	ACPI_ATTR_READ_ONLY,

	/* [231 / E7] uint8_t cas_hotkey; */
	ACPI_ATTR_READ_ONLY,

	/* [232 / E8] uint8_t acpi_pmic_rw; */
	ACPI_ATTR_READ_ONLY,

	/* [233 / E9] uint8_t fast_charge_capable; */
	ACPI_ATTR_READ_WRITE,

	/* [234 / EA] uint8_t usbc_control[8]; */
	ACPI_ATTR_READ_WRITE,

	/* [235 / EB]  */
	ACPI_ATTR_READ_WRITE,

	/* [236 / EC]  */
	ACPI_ATTR_READ_WRITE,

	/* [237 / ED]  */
	ACPI_ATTR_READ_WRITE,

	/* [238 / EE]  */
	ACPI_ATTR_READ_WRITE,

	/* [239 / EF]  */
	ACPI_ATTR_READ_WRITE,

	/* [240 / F0]  */
	ACPI_ATTR_READ_WRITE,

	/* [241 / F1]  */
	ACPI_ATTR_READ_WRITE,

	/* [242 / F2] uint8_t usbc_mailbox_cmd; */
	ACPI_ATTR_READ_WRITE,

	/* [243 / F3] uint8_t usbc_mailbox_data; */
	ACPI_ATTR_READ_WRITE,

	/* [244 / F4] uint8_t usbc_atch_dtch_wa; */
	ACPI_ATTR_READ_WRITE,

	/* [245 / F5] uint8_t acpi_bat_0_rbhf_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [246 / F6] uint8_t acpi_bat_0_rbhf_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [247 / F7] uint8_t acpi_bat_0_vbnl_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [248 / F8] uint8_t acpi_bat_0_vbnl_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [249 / F9] uint8_t acpi_bat_0_cmpp_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [250 / FA] uint8_t acpi_bat_0_cmpp_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [251 / FB] uint8_t acpi_chrg_cntr_uvth_l; */
	ACPI_ATTR_READ_ONLY,

	/* [252 / FC] uint8_t acpi_chrg_cntr_uvth_h; */
	ACPI_ATTR_READ_ONLY,

	/* [253 / FD]  */
	ACPI_ATTR_READ_ONLY,

	/* [254 / FE] uint8_t acpi_space_end; */
	ACPI_ATTR_READ_ONLY,

	/* [255 / FF] uint8_t data[ACPI_MAX_DATA - ACPI_MAX_INDEX - 1]; */
	ACPI_ATTR_READ_ONLY,
};

static uint8_t g_wake_status;

uint8_t smc_get_wake_sts(void)
{
	return g_wake_status;
}

void smc_clear_wake_sts(void)
{
	g_wake_status = 0;
}

void smc_generate_wake(uint8_t wake_reason)
{
	g_wake_status |= wake_reason;

	gpio_write_pin(WAKE_SCI, 0);
}

void smc_update_thermal_sensor(enum acpi_thrm_sens_idx idx, int16_t temp)
{
	switch (idx) {
	case ACPI_THRM_SEN_1:
		g_acpi_tbl.acpi_sen1 = temp;
		break;

	case ACPI_THRM_SEN_2:
		g_acpi_tbl.acpi_sen2 = temp;
		break;

	case ACPI_THRM_SEN_3:
		g_acpi_tbl.acpi_sen3 = temp;
		break;

	case ACPI_THRM_SEN_4:
		g_acpi_tbl.acpi_sen4 = temp;
		break;

	case ACPI_THRM_SEN_5:
		g_acpi_tbl.acpi_sen5 = temp;
		break;

	default:
		break;
	}
}

void smc_update_gpu_temperature(int temp)
{
	g_acpi_tbl.acpi_gpu_temp = temp;
}

void smc_update_cpu_temperature(int temp)
{
	g_acpi_tbl.acpi_remote_temp = temp;
}

void smc_update_pch_dts_temperature(int temp)
{
	g_acpi_tbl.acpi_pch_dts_temp = temp;
}

void smc_update_fan_capability(uint8_t fan_capability_val)
{
	g_acpi_tbl.acpi_fan_capability = fan_capability_val;
}

void smc_update_fan_tach(uint8_t fan_idx, uint16_t rpm)
{
	switch (fan_idx) {
	case FAN_CPU:
		g_acpi_tbl.acpi_cpu_fan_rpm = rpm;
		break;

	case FAN_REAR:
	case FAN_GFX:
	default:
		LOG_WRN("Missing acpi fields for fan %d", fan_idx);
		break;
	}
}

void smc_update_therm_trip_status(uint16_t status)
{
	if (status) {
		/* Send Therm_trip SCI only when local copy of sensor trip
		 * status changes i.e. a non zero status.
		 * ACPI sensor status can be read & modified by the host.
		 * Typically DTT clears the status right after its read.
		 */
		LOG_WRN("Trip stat %x %x", status,
			g_acpi_tbl.acpi_therm_snsr_sts);
		g_acpi_tbl.acpi_therm_snsr_sts |= status;
		enqueue_sci(SCI_THERMTRIP);
	}
}

void smc_update_rpm_trip_status(uint8_t status)
{
	if (status) {
		/* Send RPM_trip SCI only when local copy of RPM trip
		 * status changes i.e a non zero status.
		 * ACPI RPM status can be read & modified by the host.
		 * Typically DTT clears the status right after its read.
		 */
		LOG_WRN("Trip stat %x %x", status,
			g_acpi_tbl.fan_rpm_trip_status);
		g_acpi_tbl.fan_rpm_trip_status |= status;
		enqueue_sci(SCI_RPMTRIP);
	}
}

bool smc_is_acpi_offset_write_permitted(uint8_t offset)
{
	return acpi_tbl_attr[offset];
}
