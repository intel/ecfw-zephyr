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
	uint8_t acpi_mode:1;
	uint8_t smi_notify:1;
	uint8_t smi_enabled:1;
	uint8_t sci_enabled:1;
};
struct acpi_status_flags {
	uint8_t ac_prsnt:1;
	uint8_t usb_prsnt:1;
	uint8_t lpc_docked:1;
	uint8_t fan_on:1;
	uint8_t ri_wake:1;
	uint8_t sleep_s3:1;
	uint8_t lid_open:1;
	uint8_t pme_active:1;
};
struct acpi_status2_flags {
	uint8_t pcie_docked:1;
	uint8_t pcie_pwr_down:1;
	uint8_t exp_card_prsnt:1;
	uint8_t pwr_btn:1;
	uint8_t vb_sw_closed:1;
	uint8_t dimm_ts:1;
	uint8_t bt_attach:1;
	uint8_t bt_pwr_off:1;
};
struct acpi_bat_flags {
	uint8_t dischrg:1;
	uint8_t chrg:1;
	uint8_t crit:1;
	uint8_t prsnt:1;
	uint8_t unused4:1;
	uint8_t unused5:1;
	uint8_t unused6:1;
	uint8_t unused7:1;
};
struct acpi_power_source {
	uint8_t psrc:4;
	uint8_t pdscsn:4;
	uint8_t pdscsn_rcvd;
};
struct acpi_concept_flags {
	uint8_t acpi_cs_dbg_led_en:1;
	uint8_t acpi_winb_pwr_btn_en:1;
	uint8_t unused2:1;
	uint8_t pg3_exit_after_counter_exp:1;
	uint8_t pg3_entered_successfully:1;
	uint8_t unused5:1;
	uint8_t unused6:1;
	uint8_t unused7:1;
};

struct acpi_hid_btn_sci {
	uint8_t pwr_btn_en_dis:1;
	uint8_t win_btn_en_dis:1;
	uint8_t vol_up_en_dis:1;
	uint8_t vol_down_en_dis:1;
	uint8_t rot_lock_en_dis:1;
	uint8_t rsvd0:3;
};

struct acpi_tbl {
	/* Start of ACPI space. */
	uint8_t acpi_space;
	/* [1] ACPI Celsius temperature (remote). CPU ADT7421RemoteTemp */
	uint8_t acpi_remote_temp;
	/* [2] ACPI GPU temperature (Celsius) */
	uint8_t acpi_gpu_temp;
	/* [3] ACPI status flags */
	struct acpi_status_flags acpi_flags;
	/* [04] ACPI SMBus buffer storage */
	uint8_t acpi_smb_buffer[40];
	/* [44/2C] ACPI thermal policy setting */
	uint8_t acpi_thermal_policy;
	/* [45/2D] ACPI passive thermal temperature */
	uint8_t acpi_passive_temp;
	/* [46/2E] ACPI active thermal temperature */
	uint8_t acpi_active_temp;
	/* [47/2F] ACPI critical thermal temperature */
	uint8_t acpi_crit_temp;
	/* [48/30] ACPI status flags (second byte). */
	struct acpi_status2_flags acpi_flags2;
	/* [49/31] ACPI hotkey scan code */
	uint8_t acpi_hotkey_scan;
	/* [50/32] ACPI Battery 0 status flags. */
	struct acpi_bat_flags acpi_bat0_flags;
	/* [51/33] ACPI Battery 0 current rate. */
	uint8_t acpi_bat0_rate;
	/* [52/34] ACPI Battery 0 remaining capacity */
	uint8_t acpi_bat0_chg_sts;
	/* [53/35] ACPI Battery 0 voltage. */
	uint8_t acpi_bat0_volts;
	/* [54/36] ACPI Battery 1 status flags. */
	struct acpi_bat_flags acpi_bat1_flags;
	/* [55/37] ACPI Battery 1 current rate */
	uint8_t acpi_bat1_rate;
	/* [56/38] ACPI Battery 1 remaining capacity */
	uint8_t acpi_bat1_chg_sts;
	/* [57/39] ACPI Battery 1 voltage */
	uint8_t acpi_bat1_volts;
	/* [58/3A] Host command for SMC */
	uint8_t acpi_host_command;
	/* [59/3B] ALS Lux word */
	uint16_t acpi_lux;
	/* [61/3D] ALS Ch0 raw value */
	uint8_t acpi_als_raw_ch0;
	/* [62/3E] ALS Ch1 raw value */
	uint8_t acpi_als_raw_ch1;
	/* [63/3F] New card detect bits */
	uint8_t acpi_new_card_dt_st;
	/* [64/40] fan capability */
	uint8_t acpi_fan_capability;
	/* [65/41] fan device index select */
	uint8_t acpi_fan_idx;
	uint8_t free0;
	/* [67/43] initial and final PWM values */
	uint8_t acpi_pwm_init_val;
	uint8_t acpi_pwm_end_val;
	/* [69/45] ms delay between PWM steps */
	uint8_t acpi_pwm_step;
	/* [70/46] fan operating mode */
	uint8_t acpi_fan_opr_mode;
	/* [71/47] Fan RPM high threshold */
	uint16_t acpi_fan_rpm_high_threshold;
	/* [73/49] Fan RPM low threshold */
	uint16_t acpi_fan_rpm_low_threshold;
	/* [75/4B] Fan RPM Trip status */
	uint8_t fan_rpm_trip_status;
	/* [76/4c] Free */
	uint16_t free1;
	/* [78/4E] ACPI power source */
	struct acpi_power_source acpi_pwr_src;
	/* [80/50] Select thermal sensor */
	uint8_t acpi_temp_snsr_select;
	/* [81/51] Sensor High trip point */
	uint16_t acpi_temp_high_thrshld;
	/* [83/53] Sensor Low trip point */
	uint16_t acpi_temp_low_thrshld;
	/* [85/55] Sensor trip point flags */
	uint16_t acpi_therm_snsr_sts;
	/* [87/57] Bat A design capacity in mW */
	uint16_t acpi_bat0_design_cap;
	/* [89/59] Bat A remaining capacity in mW */
	uint16_t acpi_bat0_rem_cap;
	/* [91/5B] Bat A full charge capacity in mW */
	uint16_t acpi_bat0_full_chg;
	/* [93/5D] Bat A maximum voltage in mV */
	uint16_t acpi_bat0_v;
	/* [95/5F] Bat A discharging current rate */
	uint16_t acpi_bat0_i_dischg;
	/* [97/61] Bat A charging current rate */
	uint16_t acpi_bat0_i_chg;
	/* [99/63] Bat B remaining capacity in mW */
	uint16_t acpi_bat1_rem_cap;
	/* [101/65] Bat B full charge capacity in mW */
	uint16_t acpi_bat1_full_chg;
	/* [103/67] Bat B maximum voltage in mV */
	uint16_t acpi_bat1_v;
	/* [105/69] Bat B discharging current rate */
	uint16_t acpi_bat1_i_dischg;
	/* [107/6B] Bat B charging current rate */
	uint16_t acpi_bat1_i_chg;
	/* [109/6D] Mininmum voltage below which platform activates UVP
	 * and shuts down
	 */
	uint16_t acpi_vmin;
	/* [111/6F] Worst case rest of platform power in mW */
	uint16_t acpi_prop;
	/* [113/71] maximum (peak) TypeC adapter power output in mW */
	uint16_t acpi_apkp;
	/* [115/73] CPU fan speed */
	uint16_t acpi_cpu_fan_rpm;
	/* [117/75] Max time the DC Barrel can maintain peak power in ms */
	uint16_t acpi_apkt;
	/* [119/77] ETM DeviceID and sys time */
	uint8_t acpi_device_id;
	/* [120/78] ACPI Concept stuff flags. */
	struct acpi_concept_flags acpi_concept_flags1;
	uint8_t free2;
	/* [122/7A] TypeC source nominal voltage in milliVolts */
	uint16_t acpi_avol;
	/* [124/7C] CPU,GFX pwr sample timer */
	uint8_t acpi_cpu_gfx_timer;
	/* [125/7D] TypeC source operational current in milliAmps */
	uint16_t acpi_acur;
	/* [127/7F] DIMM1 On-board TS temperature */
	uint8_t acpi_dimm1_temp;
	/* [128/80] CPU or PCH maximum temperature */
	uint8_t acpi_cpu_pch_max_temp;
	/* [129/81] PCH DTS reading */
	uint8_t acpi_pch_dts_temp;
	/* [130/82] CPU DTS reading */
	uint16_t acpi_cpu_dts;
	/* [132/84] actual platform power consumption in mW */
	uint16_t acpi_npwr;
	/* [134/86] Long term battery charge is based on c/20 SoC (in %) */
	uint8_t acpi_lsoC;
	/* [135/87] AC adapter rating in 10 mW. Max adapter power supported.
	 * low byte which is hard coded
	 */
	uint16_t acpi_artg;
	/* [137/89] charger type, traditional or hybrid */
	uint8_t acpi_ctype_value;
	/* [138/8A] TypeC source 1ms period percentage overload in 1% unit */
	uint8_t acpi_ap01;
	/* [139/8B] TypeC source 2ms period percentage overload in 1% unit */
	uint8_t acpi_ap02;
	/* [140/8C] TypeC source 10ms period percentage overload in 1% unit */
	uint8_t acpi_ap10;
	/* [141/8D] Max sustained power for battery in mW (power battery
	 * steady state )
	 */
	uint16_t acpi_pbss;
	/* [143/8F] OS _BIX. cycle Count */
	uint16_t acpi_bixcc;
	/* [145/91] Sensors */
	uint16_t acpi_sen1;
	uint16_t acpi_sen2;
	uint16_t acpi_sen3;
	uint16_t acpi_sen4;
	uint16_t acpi_sen5;
	/* [155/9B] */
	uint32_t acpi_repeat_cycles;
	uint8_t acpi_repeat_period;
	/* [160/A0] */
	uint8_t acpi_stop_on_err;
	/* [161-180] Free Space for peci_ Interface */
	uint8_t acpi_peci_packet[20];
	/* [181/B5] Turbo PL */
	uint16_t acpi_turbo_pl2;
	/* [183/B7] Turbo TDP setting */
	uint16_t acpi_turbo_tdp;
	/* [185/B9] */
	uint16_t acpi_plt_pwr;
	/* [187/BB] peci_ status and error */
	uint8_t acpi_peci_status;
	uint32_t acpi_peci_err;
	/* [192/C0] */
	uint8_t acpi_cpu_gt_vr_temp;
	/* [193/C1] */
	uint8_t acpi_mincard_temp;
	/* [194/C2] */
	uint8_t free3;
	/* [195/C3] */
	uint8_t acpi_pg3_counter;
	/* [196/C4] PG3 Wake timer value */
	uint32_t acpi_pg3_wake_timer;
	/* [200/C8] */
	uint8_t acpi_dev_pwr_cntrl;
	/* [201/C9] button SCIs enable/disable offset */
	struct acpi_hid_btn_sci acpi_btn_cntrl;
	/* [202/CA] */
	uint8_t acpi_unused_v[7];
	/* [209/D1] Battery B design capacity in mW */
	uint16_t acpi_bat1_design_cap;
	/* [211/D3] Battery A design capacity in mW */
	uint16_t acpi_bat0_design_v;
	/* [213/D5] Battery B design voltage in mV */
	uint16_t acpi_bat1_design_v;
	/* [215/D7] Battery A pmax_ value */
	uint16_t acpi_bat0_pmax;
	/* [217/D9] Battery pmax_ value low byte */
	uint16_t acpi_bat1_pmax;
	/* [219/DB] Battery thresh hold */
	uint8_t batt_threshold;
	/* [220/DC] Batt trip point byte L for _BTP */
	uint16_t batt_trip_point;
	/* [222/DE] */
	uint8_t kb_bklt_pwm_duty;
	/* [223/DF] Battery charge rate value */
	uint16_t batt_chrg_lmt;
	/* [225/E1] Thermistors */
	uint8_t thermistor1;
	uint8_t thermistor2;
	uint8_t thermistor3;
	uint8_t thermistor4;
	uint8_t thermistor5;
	uint8_t thermistor6;
	/* [231/E7] */
	uint8_t cas_hotkey;
	/* [232/E8] */
	uint8_t acpi_pmic_rw;
	/* [233/E9] */
	uint8_t fast_charge_capable;
	/* [234/EA] USB-C control */
	uint8_t usbc_control[8];
	/* [242/F2] USBC misc mailbox command and data */
	uint8_t usbc_mailbox_cmd;
	uint8_t usbc_mailbox_data;
	/* [244/F4] USB-C attach/detach XDCI workaround */
	uint8_t usbc_atch_dtch_wa;
	/* [245/F5] Bat A smart charging parameters */
	uint16_t acpi_bat0_rbhf;
	uint16_t acpi_bat0_vbnl;
	uint16_t acpi_bat0_cmpp;
	uint16_t acpi_chrg_cntr_uvth;
	/* End of ACPI space. */
	uint8_t acpi_space_end;
	uint8_t data[ACPI_MAX_DATA - ACPI_MAX_INDEX - 1];
} __attribute__((__packed__));
extern struct acpi_tbl g_acpi_tbl;

extern struct acpi_state_flags g_acpi_state_flags;

#endif /* __ACPI_REGION_H__ */
