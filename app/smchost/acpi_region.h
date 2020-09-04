/*
 * Copyright (c) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __ACPI_REGION_H__
#define __ACPI_REGION_H__

#include <soc.h>

/** Maximum value allowed for ACPI accesses */
#define ACPI_MAX_DATA        0xFF
/** Max Number of ACPI Bytes */
#define ACPI_MAX_INDEX       253ul

struct acpi_state_flags {
	u8_t acpi_mode:1;
	u8_t smi_notify:1;
	u8_t smi_enabled:1;
	u8_t sci_enabled:1;
};
struct acpi_status_flags {
	u8_t ac_prsnt:1;
	u8_t usb_prsnt:1;
	u8_t lpc_docked:1;
	u8_t fan_on:1;
	u8_t ri_wake:1;
	u8_t sleep_s3:1;
	u8_t lid_open:1;
	u8_t pme_active:1;
};
struct acpi_status2_flags {
	u8_t pcie_docked:1;
	u8_t pcie_pwr_down:1;
	u8_t exp_card_prsnt:1;
	u8_t pwr_btn:1;
	u8_t vb_sw_closed:1;
	u8_t dimm_ts:1;
	u8_t bt_attach:1;
	u8_t bt_pwr_off:1;
};
struct acpi_bat_flags {
	u8_t dischrg:1;
	u8_t chrg:1;
	u8_t crit:1;
	u8_t prsnt:1;
	u8_t unused4:1;
	u8_t unused5:1;
	u8_t unused6:1;
	u8_t unused7:1;
};
struct acpi_power_source {
	u8_t psrc:4;
	u8_t pdscsn:4;
	u8_t pdscsn_rcvd;
};
struct acpi_concept_flags {
	u8_t acpi_cs_dbg_led_en:1;
	u8_t acpi_winb_pwr_btn_en:1;
	u8_t unused2:1;
	u8_t unused3:1;
	u8_t unused4:1;
	u8_t unused5:1;
	u8_t unused6:1;
	u8_t unused7:1;
};

struct acpi_tbl {
	/* Start of ACPI space. */
	u8_t acpi_space;
	/* [1] ACPI Celsius temperature (remote). CPU ADT7421RemoteTemp */
	u8_t acpi_remote_temp;
	/* [2] ACPI Celsius temperature (local). CPU ADT7421LocalTemp */
	u8_t acpi_local_temp;
	/* [3] ACPI status flags */
	struct acpi_status_flags acpi_flags;
	/* [04] ACPI SMBus buffer storage */
	u8_t acpi_smb_buffer[40];
	/* [44/2C] ACPI thermal policy setting */
	u8_t acpi_thermal_policy;
	/* [45/2D] ACPI passive thermal temperature */
	u8_t acpi_passive_temp;
	/* [46/2E] ACPI active thermal temperature */
	u8_t acpi_active_temp;
	/* [47/2F] ACPI critical thermal temperature */
	u8_t acpi_crit_temp;
	/* [48/30] ACPI status flags (second byte). */
	struct acpi_status2_flags acpi_flags2;
	/* [49/31] ACPI hotkey scan code */
	u8_t acpi_hotkey_scan;
	/* [50/32] ACPI Battery 0 status flags. */
	struct acpi_bat_flags acpi_bat0_flags;
	/* [51/33] ACPI Battery 0 current rate. */
	u8_t acpi_bat0_rate;
	/* [52/34] ACPI Battery 0 remaining capacity */
	u8_t acpi_bat0_chg_sts;
	/* [53/35] ACPI Battery 0 voltage. */
	u8_t acpi_bat0_volts;
	/* [54/36] ACPI Battery 1 status flags. */
	struct acpi_bat_flags acpi_bat1_flags;
	/* [55/37] ACPI Battery 1 current rate */
	u8_t acpi_bat1_rate;
	/* [56/38] ACPI Battery 1 remaining capacity */
	u8_t acpi_bat1_chg_sts;
	/* [57/39] ACPI Battery 1 voltage */
	u8_t acpi_bat1_volts;
	/* [58/3A] Host command for SMC */
	u8_t acpi_host_command;
	/* [59/3B] ALS Lux word (little endian) */
	u8_t acpi_lux_l;
	u8_t acpi_lux_h;
	/* [61/3D] ALS Ch0 raw value */
	u8_t acpi_als_raw_ch0;
	/* [62/3E] ALS Ch1 raw value */
	u8_t acpi_als_raw_ch1;
	/* [63/3F] New card detect bits */
	u8_t acpi_new_card_dt_st;
	/* [64/40] Peripheral control bits */
	u8_t acpi_periph_cntrl;
	/* [65/41] fan device index select */
	u8_t acpi_fan_idx;
	u8_t free0;
	/* [67/43] initial and final PWM values */
	u8_t acpi_pwm_init_val;
	u8_t acpi_pwm_end_val;
	/* [69/45] ms delay between PWM steps */
	u8_t acpi_pwm_step;
	/* [70/46] TMKBC IRQ level selection */
	u8_t acpi_tm_kbc_irq_data;
	/* [71/47] CPU power in mW (little endian) */
	u8_t acpi_cpu_pwr_l;
	/* [72/48] CPU power in mW */
	u8_t acpi_cpu_pwr_h;
	/* [73/49] MCH power in mW (little endian) */
	u8_t acpi_mch_pwr_l;
	/* [74/4A] MCH power in mW */
	u8_t acpi_mch_pwr_h;
	/* [75/4B] System power in 0.01 W (little endian) */
	u8_t acpi_system_pwr_l;
	/* [76/4C] System power in 0.01 W */
	u8_t acpi_system_pwr_h;
	/* [77/4D] */
	u8_t free1;
	/* [78/4E] ACPI power source */
	struct acpi_power_source acpi_pwr_src;
	/* [80/50] Select thermal sensor */
	u8_t acpi_temp_snsr_select;
	/* [81/51] Sensor High trip point */
	u8_t acpi_temp_thrshld_h_lsb;
	u8_t acpi_temp_thrshld_h_msb;
	/* [83/53] Sensor Low trip point */
	u8_t acpi_temp_thrshld_l_lsb;
	u8_t acpi_temp_thrshld_l_msb;
	/* [85/55] Sensor trip point flags */
	u8_t acpi_therm_snsr_sts_lsb;
	u8_t acpi_therm_snsr_sts_msb;
	/* [87/57] Bat A design capacity in mW */
	u8_t acpi_bat0_design_cap_l;
	u8_t acpi_bat0_design_cap_h;
	/* [89/59] Bat A remaining capacity in mW */
	u8_t acpi_bat0_rem_cap_l;
	u8_t acpi_bat0_rem_cap_h;
	/* [91/5B] Bat A full charge capacity in mW */
	u8_t acpi_bat0_full_chg_l;
	u8_t acpi_bat0_full_chg_h;
	/* [93/5D] Bat A maximum voltage in mV */
	u8_t acpi_bat0_v_l;
	u8_t acpi_bat0_v_h;
	/* [95/5F] Bat A discharging current rate */
	u8_t acpi_bat0_i_dischg_l;
	u8_t acpi_bat0_i_dischg_h;
	/* [97/61] Bat A charging current rate */
	u8_t acpi_bat0_i_chg_l;
	u8_t acpi_bat0_i_chg_h;
	/* [99/63] Bat B remaining capacity in mW */
	u8_t acpi_bat1_rem_cap_l;
	u8_t acpi_bat1_rem_cap_h;
	/* [101/65] Bat B full charge capacity in mW */
	u8_t acpi_bat1_full_chg_l;
	u8_t acpi_bat1_full_chg_h;
	/* [103/67] Bat B maximum voltage in mV */
	u8_t acpi_bat1_v_l;
	u8_t acpi_bat1_v_h;
	/* [105/69] Bat B discharging current rate */
	u8_t acpi_bat1_i_dischg_l;
	u8_t acpi_bat1_i_dischg_h;
	/* [107/6B] Bat B charging current rate */
	u8_t acpi_bat1_i_chg_l;
	u8_t acpi_bat1_i_chg_h;
	/* [109/6D] Mininmum voltage below which platform activates UVP
	 * and shuts down
	 */
	u8_t acpi_vmin_lsb;
	u8_t acpi_vmin_msb;
	/* [111/6F] Worst case rest of platform power in mW */
	u8_t acpi_prop_lsb;
	u8_t acpi_prop_msb;
	/* [113/71] maximum (peak) TypeC adapter power output in mW */
	u8_t acpi_apkp_lsb;
	u8_t acpi_apkp_msb;
	/* [115/73] CPU fan speed */
	u8_t acpi_cpu_fan_rpm_l;
	u8_t acpi_cpu_fan_rpm_h;
	/* [117/75] Max time the DC Barrel can maintain peak power in ms */
	u8_t acpi_apkt_lsb;
	/* [118/76] */
	u8_t acpi_apkt_msb;
	/* [119/77] ETM DeviceID and sys time */
	u8_t acpi_device_id;
	/* [120/78] ACPI Concept stuff flags. */
	struct acpi_concept_flags acpi_concept_flags1;
	u8_t free2;
	/* [122/7A] TypeC source nominal voltage in milliVolts */
	u8_t acpi_avol_lsb;
	u8_t acpi_avol_msb;
	/* [124/7C] CPU,GFX pwr sample timer */
	u8_t acpi_cpu_gfx_timer;
	/* [125/7D] TypeC source operational current in milliAmps */
	u8_t acpi_acur_lsb;
	u8_t acpi_acur_msb;
	/* [127/7F] DIMM1 On-board TS temperature */
	u8_t acpi_dimm1_temp;
	/* [128/80] CPU or PCH maximum temperature */
	u8_t acpi_cpu_pch_max_temp;
	/* [129/81] PCH DTS reading */
	u8_t acpi_pch_dts_temp;
	/* [130/82] CPU DTS reading */
	u8_t acpi_cpu_dts_low;
	u8_t acpi_cpu_dts_high;
	/* [132/84] actual platform power consumption in mW */
	u8_t acpi_npwr_lsb;
	u8_t acpi_npwr_msb;
	/* [134/86] Long term battery charge is based on c/20 SoC (in %) */
	u8_t acpi_lsoC;
	/* [135/87] AC adapter rating in 10 mW. Max adapter power supported.
	 * low byte which is hard coded
	 */
	u8_t acpi_artg_lsb;
	u8_t acpi_artg_msb;
	/* [137/89] charger type, traditional or hybrid */
	u8_t acpi_ctype_value;
	/* [138/8A] TypeC source 1ms period percentage overload in 1% unit */
	u8_t acpi_ap01;
	/* [139/8B] TypeC source 2ms period percentage overload in 1% unit */
	u8_t acpi_ap02;
	/* [140/8C] TypeC source 10ms period percentage overload in 1% unit */
	u8_t acpi_ap10;
	/* [141/8D] Max sustained power for battery in mW (power battery
	 * steady state )
	 */
	u8_t acpi_pbss_lsb;
	u8_t acpi_pbss_msb;
	/* [143/8F] OS _BIX. cycle Count */
	u8_t acpi_bixcc_lsb;
	u8_t acpi_bixcc_msb;
	/* [145/91] Sensors */
	u8_t acpi_sen1_lsb;
	u8_t acpi_sen1_msb;
	u8_t acpi_sen2_lsb;
	u8_t acpi_sen2_msb;
	u8_t acpi_sen3_lsb;
	u8_t acpi_sen3_msb;
	u8_t acpi_sen4_lsb;
	u8_t acpi_sen4_msb;
	u8_t acpi_sen5_lsb;
	u8_t acpi_sen5_msb;
	/* [155/9B] */
	u32_t acpi_repeat_cycles;
	u8_t acpi_repeat_period;
	/* [160/A0] */
	u8_t acpi_stop_on_err;
	/* [161-180] Free Space for peci_ Interface */
	u8_t acpi_peci_packet[20];
	/* [181/B5] Turbo PL */
	u8_t acpi_turbo_pl2_lsb;
	u8_t acpi_turbo_pl2_msb;
	/* [183/B7] Turbo TDP setting */
	u8_t acpi_turbo_tdp_lsb;
	u8_t acpi_turbo_tdp_msb;
	/* [185/B9] */
	u8_t acpi_plt_pwr_lsb;
	u8_t acpi_plt_pwr_msb;
	/* [187/BB] peci_ status and error */
	u8_t acpi_peci_status;
	u32_t acpi_peci_err;
	/* [192/C0] */
	u8_t acpi_cpu_gt_vr_temp;
	/* [193/C1] */
	u8_t acpi_mincard_temp;
	/* [194/C2] */
	u8_t free3[2];
	/* [196/C4] Wake timer value */
	u32_t acpi_wake_timer;
	/* [200/C8] */
	u8_t acpi_dev_pwr_cntrl;
	/* [201/C9] */
	u8_t acpi_dis_btn_sci;
	/* [202/CA] */
	u8_t acpi_unused_v[7];
	/* [209/D1] Battery B design capacity in mW */
	u8_t acpi_bat_1_design_cap_l;
	u8_t acpi_bat_1_design_cap_h;
	/* [211/D3] Battery A design capacity in mW */
	u8_t acpi_bat0_design_v_l;
	u8_t acpi_bat0_design_v_h;
	/* [213/D5] Battery B design voltage in mV */
	u8_t acpi_bat1_design_v_l;
	u8_t acpi_bat1_design_v_h;
	/* [215/D7] Battery A pmax_ value */
	u8_t acpi_bat0_pmax_L;
	u8_t acpi_bat0_pmax_H;
	/* [217/D9] Battery pmax_ value low byte */
	u8_t acpi_bat_1_pmax_L;
	u8_t acpi_bat_1_pmax_H;
	/* [219/DB] Battery thresh hold */
	u8_t batt_threshold;
	/* [220/DC] Batt trip point byte L for _BTP */
	u8_t batt_trip_point_lsb;
	u8_t batt_trip_point_msb;
	/* [222/DE] */
	u8_t kb_bklt_pwm_duty;
	/* [223/DF] Battery charge rate value low byte */
	u8_t batt_chrg_lmt_lsb;
	/* [224/E0] Battery charge rate value high byte */
	u8_t batt_chrg_lmt_msb;
	/* [225/E1] Thermistors */
	u8_t thermistor1;
	u8_t thermistor2;
	u8_t thermistor3;
	u8_t thermistor4;
	u8_t thermistor5;
	u8_t thermistor6;
	/* [231/E7] */
	u8_t cas_hotkey;
	/* [232/E8] */
	u8_t acpi_pmic_rw;
	/* [233/E9] */
	u8_t fast_charge_capable;
	/* [234/EA] USB-C control */
	u8_t usbc_control[8];
	/* [242/F2] Unused */
	u8_t unused1;
	u8_t unused2;
	/* [244/F4] USB-C attach/detach XDCI workaround */
	u8_t usbc_atch_dtch_wa;
	/* [245/F5] Bat A smart charging parameters */
	u8_t acpi_bat_0_rbhf_lsb;
	u8_t acpi_bat_0_rbhf_msb;
	u8_t acpi_bat_0_vbnl_lsb;
	u8_t acpi_bat_0_vbnl_msb;
	u8_t acpi_bat_0_cmpp_lsb;
	u8_t acpi_bat_0_cmpp_msb;
	u8_t acpi_chrg_cntr_uvth_l;
	u8_t acpi_chrg_cntr_uvth_h;
	/* End of ACPI space. */
	u8_t acpi_space_end;
	u8_t data[ACPI_MAX_DATA - ACPI_MAX_INDEX - 1];
} __attribute__((__packed__));
extern struct acpi_tbl g_acpi_tbl;

extern struct acpi_state_flags g_acpi_state_flags;

#endif /* __ACPI_REGION_H__ */
