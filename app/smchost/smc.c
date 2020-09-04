/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr.h>
#include <device.h>
#include <soc.h>
#include "gpio_ec.h"
#include <logging/log.h>
#include "smc.h"
#include "sci.h"
#include "smchost.h"
#include "system.h"
#include "acpi.h"
#include "board_config.h"
#include "mec_cpu.h"
#include "fan.h"
#include "scicodes.h"

LOG_MODULE_REGISTER(smc, CONFIG_SMCHOST_LOG_LEVEL);

#define ACPI_ATTR_READ_ONLY	0u
#define ACPI_ATTR_READ_WRITE	1u

static const bool acpi_tbl_attr[256] = {
	/* [0 / 0] u8_t acpi_space; */
	ACPI_ATTR_READ_ONLY,

	/* [1 / 1] u8_t acpi_remote_temp; */
	ACPI_ATTR_READ_ONLY,

	/* [2 / 2] u8_t acpi_local_temp; */
	ACPI_ATTR_READ_ONLY,

	/* [3 / 3] struct acpi_status_flags acpi_flags; */
	ACPI_ATTR_READ_ONLY,

	/* [4 / 4] u8_t acpi_smb_buffer[40]; */
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

	/* [44 / 2C] u8_t acpi_thermal_policy; */
	ACPI_ATTR_READ_WRITE,

	/* [45 / 2D] u8_t acpi_passive_temp; */
	ACPI_ATTR_READ_WRITE,

	/* [46 / 2E] u8_t acpi_active_temp; */
	ACPI_ATTR_READ_WRITE,

	/* [47 / 2F] u8_t acpi_crit_temp; */
	ACPI_ATTR_READ_WRITE,

	/* [48 / 30] struct acpi_status2_flags acpi_flags2; */
	ACPI_ATTR_READ_ONLY,

	/* [49 / 31] u8_t acpi_hotkey_scan; */
	ACPI_ATTR_READ_ONLY,

	/* [50 / 32] struct acpi_bat_flags acpi_bat0_flags; */
	ACPI_ATTR_READ_ONLY,

	/* [51 / 33] u8_t acpi_bat0_rate; */
	ACPI_ATTR_READ_ONLY,

	/* [52 / 34] u8_t acpi_bat0_chg_sts; */
	ACPI_ATTR_READ_ONLY,

	/* [53 / 35] u8_t acpi_bat0_volts; */
	ACPI_ATTR_READ_ONLY,

	/* [54 / 36] struct acpi_bat_flags acpi_bat1_flags; */
	ACPI_ATTR_READ_ONLY,

	/* [55 / 37] u8_t acpi_bat1_rate; */
	ACPI_ATTR_READ_ONLY,

	/* [56 / 38] u8_t acpi_bat1_chg_sts; */
	ACPI_ATTR_READ_ONLY,

	/* [57 / 39] u8_t acpi_bat1_volts; */
	ACPI_ATTR_READ_ONLY,

	/* [58 / 3A] u8_t acpi_host_command; */
	ACPI_ATTR_READ_WRITE,

	/* [59 / 3B] u8_t acpi_lux_l; */
	ACPI_ATTR_READ_ONLY,

	/* [60 / 3C] u8_t acpi_lux_h; */
	ACPI_ATTR_READ_ONLY,

	/* [61 / 3D] u8_t acpi_als_raw_ch0; */
	ACPI_ATTR_READ_ONLY,

	/* [62 / 3E] u8_t acpi_als_raw_ch1; */
	ACPI_ATTR_READ_ONLY,

	/* [63 / 3F] u8_t acpi_new_card_dt_st; */
	ACPI_ATTR_READ_ONLY,

	/* [64 / 40] u8_t acpi_periph_cntrl; */
	ACPI_ATTR_READ_WRITE,

	/* [65 / 41] u8_t acpi_fan_idx; */
	ACPI_ATTR_READ_WRITE,

	/* [66 / 42] u8_t free0; */
	ACPI_ATTR_READ_WRITE,

	/* [67 / 43] u8_t acpi_pwm_init_val; */
	ACPI_ATTR_READ_WRITE,

	/* [68 / 44] u8_t acpi_pwm_end_val; */
	ACPI_ATTR_READ_WRITE,

	/* [69 / 45] u8_t acpi_pwm_step; */
	ACPI_ATTR_READ_WRITE,

	/* [70 / 46] u8_t acpi_tm_kbc_irq_data; */
	ACPI_ATTR_READ_ONLY,

	/* [71 / 47] u8_t acpi_cpu_pwr_l; */
	ACPI_ATTR_READ_ONLY,

	/* [72 / 48] u8_t acpi_cpu_pwr_h; */
	ACPI_ATTR_READ_ONLY,

	/* [73 / 49] u8_t acpi_mch_pwr_l; */
	ACPI_ATTR_READ_ONLY,

	/* [74 / 4A] u8_t acpi_mch_pwr_h; */
	ACPI_ATTR_READ_ONLY,

	/* [75 / 4B] u8_t acpi_system_pwr_l; */
	ACPI_ATTR_READ_ONLY,

	/* [76 / 4C] u8_t acpi_system_pwr_h; */
	ACPI_ATTR_READ_ONLY,

	/* [77 / 4D] u8_t free1; */
	ACPI_ATTR_READ_ONLY,

	/* [78 / 4E] struct acpi_power_source acpi_pwr_src; */
	ACPI_ATTR_READ_WRITE,

	/* [79 / 4F]  */
	ACPI_ATTR_READ_WRITE,

	/* [80 / 50] u8_t acpi_temp_snsr_select; */
	ACPI_ATTR_READ_WRITE,

	/* [81 / 51] u8_t acpi_temp_thrshld_h_lsb; */
	ACPI_ATTR_READ_WRITE,

	/* [82 / 52] u8_t acpi_temp_thrshld_h_msb; */
	ACPI_ATTR_READ_WRITE,

	/* [83 / 53] u8_t acpi_temp_thrshld_l_lsb; */
	ACPI_ATTR_READ_WRITE,

	/* [84 / 54] u8_t acpi_temp_thrshld_l_msb; */
	ACPI_ATTR_READ_WRITE,

	/* [85 / 55] u8_t acpi_therm_snsr_sts_lsb; */
	ACPI_ATTR_READ_WRITE,

	/* [86 / 56] u8_t acpi_therm_snsr_sts_msb; */
	ACPI_ATTR_READ_WRITE,

	/* [87 / 57] u8_t acpi_bat0_design_cap_l; */
	ACPI_ATTR_READ_ONLY,

	/* [88 / 58] u8_t acpi_bat0_design_cap_h; */
	ACPI_ATTR_READ_ONLY,

	/* [89 / 59] u8_t acpi_bat0_rem_cap_l; */
	ACPI_ATTR_READ_ONLY,

	/* [90 / 5A] u8_t acpi_bat0_rem_cap_h; */
	ACPI_ATTR_READ_ONLY,

	/* [91 / 5B] u8_t acpi_bat0_full_chg_l; */
	ACPI_ATTR_READ_ONLY,

	/* [92 / 5C] u8_t acpi_bat0_full_chg_h; */
	ACPI_ATTR_READ_ONLY,

	/* [93 / 5D] u8_t acpi_bat0_v_l; */
	ACPI_ATTR_READ_ONLY,

	/* [94 / 5E] u8_t acpi_bat0_v_h; */
	ACPI_ATTR_READ_ONLY,

	/* [95 / 5F] u8_t acpi_bat0_i_dischg_l; */
	ACPI_ATTR_READ_ONLY,

	/* [96 / 60] u8_t acpi_bat0_i_dischg_h; */
	ACPI_ATTR_READ_ONLY,

	/* [97 / 61] u8_t acpi_bat0_i_chg_l; */
	ACPI_ATTR_READ_ONLY,

	/* [98 / 62] u8_t acpi_bat0_i_chg_h; */
	ACPI_ATTR_READ_ONLY,

	/* [99 / 63] u8_t acpi_bat1_rem_cap_l; */
	ACPI_ATTR_READ_ONLY,

	/* [100 / 64] u8_t acpi_bat1_rem_cap_h; */
	ACPI_ATTR_READ_ONLY,

	/* [101 / 65] u8_t acpi_bat1_full_chg_l; */
	ACPI_ATTR_READ_ONLY,

	/* [102 / 66] u8_t acpi_bat1_full_chg_h; */
	ACPI_ATTR_READ_ONLY,

	/* [103 / 67] u8_t acpi_bat1_v_l; */
	ACPI_ATTR_READ_ONLY,

	/* [104 / 68] u8_t acpi_bat1_v_h; */
	ACPI_ATTR_READ_ONLY,

	/* [105 / 69] u8_t acpi_bat1_i_dischg_l; */
	ACPI_ATTR_READ_ONLY,

	/* [106 / 6A] u8_t acpi_bat1_i_dischg_h; */
	ACPI_ATTR_READ_ONLY,

	/* [107 / 6B] u8_t acpi_bat1_i_chg_l; */
	ACPI_ATTR_READ_ONLY,

	/* [108 / 6C] u8_t acpi_bat1_i_chg_h; */
	ACPI_ATTR_READ_ONLY,

	/* [109 / 6D] u8_t acpi_vmin_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [110 / 6E] u8_t acpi_vmin_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [111 / 6F] u8_t acpi_prop_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [112 / 70] u8_t acpi_prop_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [113 / 71] u8_t acpi_apkp_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [114 / 72] u8_t acpi_apkp_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [115 / 73] u8_t acpi_cpu_fan_rpm_l; */
	ACPI_ATTR_READ_ONLY,

	/* [116 / 74] u8_t acpi_cpu_fan_rpm_h; */
	ACPI_ATTR_READ_ONLY,

	/* [117 / 75] u8_t acpi_apkt_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [118 / 76] u8_t acpi_apkt_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [119 / 77] u8_t acpi_device_id; */
	ACPI_ATTR_READ_ONLY,

	/* [120 / 78] struct acpi_concept_flags acpi_concept_flags1; */
	ACPI_ATTR_READ_WRITE,

	/* [121 / 79] u8_t free2; */
	ACPI_ATTR_READ_ONLY,

	/* [122 / 7A] u8_t acpi_avol_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [123 / 7B] u8_t acpi_avol_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [124 / 7C] u8_t acpi_cpu_gfx_timer; */
	ACPI_ATTR_READ_WRITE,

	/* [125 / 7D] u8_t acpi_acur_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [126 / 7E] u8_t acpi_acur_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [127 / 7F] u8_t acpi_dimm1_temp; */
	ACPI_ATTR_READ_ONLY,

	/* [128 / 80] u8_t acpi_cpu_pch_max_temp; */
	ACPI_ATTR_READ_ONLY,

	/* [129 / 81] u8_t acpi_pch_dts_temp; */
	ACPI_ATTR_READ_ONLY,

	/* [130 / 82] u8_t acpi_cpu_dts_low; */
	ACPI_ATTR_READ_ONLY,

	/* [131 / 83] u8_t acpi_cpu_dts_high; */
	ACPI_ATTR_READ_ONLY,

	/* [132 / 84] u8_t acpi_npwr_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [133 / 85] u8_t acpi_npwr_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [134 / 86] u8_t acpi_lsoC; */
	ACPI_ATTR_READ_ONLY,

	/* [135 / 87] u8_t acpi_artg_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [136 / 88] u8_t acpi_artg_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [137 / 89] u8_t acpi_ctype_value; */
	ACPI_ATTR_READ_ONLY,

	/* [138 / 8A] u8_t acpi_ap01; */
	ACPI_ATTR_READ_ONLY,

	/* [139 / 8B] u8_t acpi_ap02; */
	ACPI_ATTR_READ_ONLY,

	/* [140 / 8C] u8_t acpi_ap10; */
	ACPI_ATTR_READ_ONLY,

	/* [141 / 8D] u8_t acpi_pbss_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [142 / 8E] u8_t acpi_pbss_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [143 / 8F] u8_t acpi_bixcc_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [144 / 90] u8_t acpi_bixcc_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [145 / 91] u8_t acpi_sen1_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [146 / 92] u8_t acpi_sen1_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [147 / 93] u8_t acpi_sen2_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [148 / 94] u8_t acpi_sen2_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [149 / 95] u8_t acpi_sen3_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [150 / 96] u8_t acpi_sen3_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [151 / 97] u8_t acpi_sen4_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [152 / 98] u8_t acpi_sen4_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [153 / 99] u8_t acpi_sen5_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [154 / 9A] u8_t acpi_sen5_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [155 / 9B] u32_t acpi_repeat_cycles; */
	ACPI_ATTR_READ_WRITE,

	/* [156 / 9C]  */
	ACPI_ATTR_READ_WRITE,

	/* [157 / 9D]  */
	ACPI_ATTR_READ_WRITE,

	/* [158 / 9E]  */
	ACPI_ATTR_READ_WRITE,

	/* [159 / 9F] u8_t acpi_repeat_period; */
	ACPI_ATTR_READ_WRITE,

	/* [160 / A0] u8_t acpi_stop_on_err; */
	ACPI_ATTR_READ_WRITE,

	/* [161 / A1] u8_t acpi_peci_packet[20]; */
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

	/* [181 / B5] u8_t acpi_turbo_pl2_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [182 / B6] u8_t acpi_turbo_pl2_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [183 / B7] u8_t acpi_turbo_tdp_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [184 / B8] u8_t acpi_turbo_tdp_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [185 / B9] u8_t acpi_plt_pwr_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [186 / BA] u8_t acpi_plt_pwr_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [187 / BB] u8_t acpi_peci_status; */
	ACPI_ATTR_READ_ONLY,

	/* [188 / BC] u32_t acpi_peci_err; */
	ACPI_ATTR_READ_ONLY,

	/* [189 / BD]  */
	ACPI_ATTR_READ_ONLY,

	/* [190 / BE]  */
	ACPI_ATTR_READ_ONLY,

	/* [191 / BF]  */
	ACPI_ATTR_READ_ONLY,

	/* [192 / C0] u8_t acpi_cpu_gt_vr_temp; */
	ACPI_ATTR_READ_ONLY,

	/* [193 / C1] u8_t acpi_mincard_temp; */
	ACPI_ATTR_READ_ONLY,

	/* [194 / C2] u8_t free3[2]; */
	ACPI_ATTR_READ_ONLY,

	/* [195 / C3]  */
	ACPI_ATTR_READ_ONLY,

	/* [196 / C4] u32_t acpi_wake_timer; */
	ACPI_ATTR_READ_ONLY,

	/* [197 / C5]  */
	ACPI_ATTR_READ_ONLY,

	/* [198 / C6]  */
	ACPI_ATTR_READ_ONLY,

	/* [199 / C7]  */
	ACPI_ATTR_READ_ONLY,

	/* [200 / C8] u8_t acpi_dev_pwr_cntrl; */
	ACPI_ATTR_READ_WRITE,

	/* [201 / C9] u8_t acpi_dis_btn_sci; */
	ACPI_ATTR_READ_WRITE,

	/* [202 / CA] u8_t acpi_unused_v[7]; */
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

	/* [209 / D1] u8_t acpi_bat_1_design_cap_l; */
	ACPI_ATTR_READ_ONLY,

	/* [210 / D2] u8_t acpi_bat_1_design_cap_h; */
	ACPI_ATTR_READ_ONLY,

	/* [211 / D3] u8_t acpi_bat0_design_v_l; */
	ACPI_ATTR_READ_ONLY,

	/* [212 / D4] u8_t acpi_bat0_design_v_h; */
	ACPI_ATTR_READ_ONLY,

	/* [213 / D5] u8_t acpi_bat1_design_v_l; */
	ACPI_ATTR_READ_ONLY,

	/* [214 / D6] u8_t acpi_bat1_design_v_h; */
	ACPI_ATTR_READ_ONLY,

	/* [215 / D7] u8_t acpi_bat0_pmax_L; */
	ACPI_ATTR_READ_ONLY,

	/* [216 / D8] u8_t acpi_bat0_pmax_H; */
	ACPI_ATTR_READ_ONLY,

	/* [217 / D9] u8_t acpi_bat_1_pmax_L; */
	ACPI_ATTR_READ_ONLY,

	/* [218 / DA] u8_t acpi_bat_1_pmax_H; */
	ACPI_ATTR_READ_ONLY,

	/* [219 / DB] u8_t batt_threshold; */
	ACPI_ATTR_READ_WRITE,

	/* [220 / DC] u8_t batt_trip_point_lsb; */
	ACPI_ATTR_READ_WRITE,

	/* [221 / DD] u8_t batt_trip_point_msb; */
	ACPI_ATTR_READ_WRITE,

	/* [222 / DE] u8_t kb_bklt_pwm_duty; */
	ACPI_ATTR_READ_ONLY,

	/* [223 / DF] u8_t batt_chrg_lmt_lsb; */
	ACPI_ATTR_READ_WRITE,

	/* [224 / E0] u8_t batt_chrg_lmt_msb; */
	ACPI_ATTR_READ_WRITE,

	/* [225 / E1] u8_t thermistor1; */
	ACPI_ATTR_READ_ONLY,

	/* [226 / E2] u8_t thermistor2; */
	ACPI_ATTR_READ_ONLY,

	/* [227 / E3] u8_t thermistor3; */
	ACPI_ATTR_READ_ONLY,

	/* [228 / E4] u8_t thermistor4; */
	ACPI_ATTR_READ_ONLY,

	/* [229 / E5] u8_t thermistor5; */
	ACPI_ATTR_READ_ONLY,

	/* [230 / E6] u8_t thermistor6; */
	ACPI_ATTR_READ_ONLY,

	/* [231 / E7] u8_t cas_hotkey; */
	ACPI_ATTR_READ_ONLY,

	/* [232 / E8] u8_t acpi_pmic_rw; */
	ACPI_ATTR_READ_ONLY,

	/* [233 / E9] u8_t fast_charge_capable; */
	ACPI_ATTR_READ_WRITE,

	/* [234 / EA] u8_t usbc_control[8]; */
	ACPI_ATTR_READ_ONLY,

	/* [235 / EB]  */
	ACPI_ATTR_READ_ONLY,

	/* [236 / EC]  */
	ACPI_ATTR_READ_ONLY,

	/* [237 / ED]  */
	ACPI_ATTR_READ_ONLY,

	/* [238 / EE]  */
	ACPI_ATTR_READ_ONLY,

	/* [239 / EF]  */
	ACPI_ATTR_READ_ONLY,

	/* [240 / F0]  */
	ACPI_ATTR_READ_ONLY,

	/* [241 / F1]  */
	ACPI_ATTR_READ_ONLY,

	/* [242 / F2] u8_t unused1; */
	ACPI_ATTR_READ_ONLY,

	/* [243 / F3] u8_t unused2; */
	ACPI_ATTR_READ_ONLY,

	/* [244 / F4] u8_t usbc_atch_dtch_wa; */
	ACPI_ATTR_READ_WRITE,

	/* [245 / F5] u8_t acpi_bat_0_rbhf_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [246 / F6] u8_t acpi_bat_0_rbhf_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [247 / F7] u8_t acpi_bat_0_vbnl_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [248 / F8] u8_t acpi_bat_0_vbnl_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [249 / F9] u8_t acpi_bat_0_cmpp_lsb; */
	ACPI_ATTR_READ_ONLY,

	/* [250 / FA] u8_t acpi_bat_0_cmpp_msb; */
	ACPI_ATTR_READ_ONLY,

	/* [251 / FB] u8_t acpi_chrg_cntr_uvth_l; */
	ACPI_ATTR_READ_ONLY,

	/* [252 / FC] u8_t acpi_chrg_cntr_uvth_h; */
	ACPI_ATTR_READ_ONLY,

	/* [253 / FD]  */
	ACPI_ATTR_READ_ONLY,

	/* [254 / FE] u8_t acpi_space_end; */
	ACPI_ATTR_READ_ONLY,

	/* [255 / FF] u8_t data[ACPI_MAX_DATA - ACPI_MAX_INDEX - 1]; */
	ACPI_ATTR_READ_ONLY,
};

static u8_t g_wake_status;

u8_t smc_get_wake_sts(void)
{
	return g_wake_status;
}

void smc_clear_wake_sts(void)
{
	g_wake_status = 0;
}

void smc_generate_wake(u8_t wake_reason)
{
	g_wake_status |= wake_reason;

	gpio_write_pin(WAKE_SCI, 1);
}

void smc_update_thermal_sensor(enum acpi_thrm_sens_idx idx, s16_t temp)
{
	switch (idx) {
	case ACPI_THRM_SEN_PCH:
		g_acpi_tbl.acpi_sen1_lsb = (u8_t) temp;
		g_acpi_tbl.acpi_sen1_msb = (u8_t) (temp >> 8);
		break;

	case ACPI_THRM_SEN_SKIN:
		g_acpi_tbl.acpi_sen2_lsb = (u8_t) temp;
		g_acpi_tbl.acpi_sen2_msb = (u8_t) (temp >> 8);
		break;

	case ACPI_THRM_SEN_AMBIENT:
		g_acpi_tbl.acpi_sen3_lsb = (u8_t) temp;
		g_acpi_tbl.acpi_sen3_msb = (u8_t) (temp >> 8);
		break;

	case ACPI_THRM_SEN_VR:
		g_acpi_tbl.acpi_sen4_lsb = (u8_t) temp;
		g_acpi_tbl.acpi_sen4_msb = (u8_t) (temp >> 8);
		break;

	case ACPI_THRM_SEN_DDR:
		g_acpi_tbl.acpi_sen5_lsb = (u8_t) temp;
		g_acpi_tbl.acpi_sen5_msb = (u8_t) (temp >> 8);
		break;

	default:
		break;
	}
}

void smc_update_cpu_temperature(int temp)
{
	g_acpi_tbl.acpi_remote_temp = temp;
}

void smc_update_fan_tach(u8_t fan_idx, u16_t rpm)
{
	switch (fan_idx) {
	case FAN_CPU:
		g_acpi_tbl.acpi_cpu_fan_rpm_l = (u8_t) rpm;
		g_acpi_tbl.acpi_cpu_fan_rpm_h = (u8_t) (rpm >> 8);
		break;

	case FAN_REAR:
	case FAN_GFX:
	default:
		LOG_WRN("Missing acpi fields for fan %d", fan_idx);
		break;
	}
}

void smc_update_therm_trip_status(u16_t status)
{
	if (status != ((g_acpi_tbl.acpi_therm_snsr_sts_msb << 8)
				| g_acpi_tbl.acpi_therm_snsr_sts_lsb)) {

		g_acpi_tbl.acpi_therm_snsr_sts_lsb = (u8_t) status;
		g_acpi_tbl.acpi_therm_snsr_sts_msb =
				(u8_t) (status >> 8);

		enqueue_sci(SCI_THERMTRIP);
	}
}

bool smc_is_acpi_offset_write_permitted(u8_t offset)
{
	return acpi_tbl_attr[offset];
}
