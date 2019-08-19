/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.


 * Alternatively, this program is free software in case of open source project
 * you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.

*/

#ifndef __DREX_V3_R3_H__
#define __DREX_V3_R3_H__

#ifndef __ASSEMBLY__

typedef volatile unsigned int rw_bf_t;
typedef const unsigned int ro_bf_t;
typedef const unsigned int wo_bf_t;

/* drex ge registers */

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t mrr_1st_byte                         :(1 - 0 + 1);
		rw_bf_t mrr_2nd_byte                         :(3 - 2 + 1);
		rw_bf_t mrr_2nd_byte_valid                   :(7 - 4 + 1);
		rw_bf_t mrr_1st_byte_reverse                 :(8 - 8 + 1);
		rw_bf_t mrr_2nd_byte_reverse                 :(9 - 9 + 1);
		rw_bf_t ca_reverse                           :(10 - 10 + 1);
		rw_bf_t reserved_11_31                          :(31 - 11 + 1);
	} bitfield;
} drex_ge_package_config_t;                // 0x0000

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t dfi_init_start                       :(0 - 0 + 1);
		rw_bf_t reserved_1_31                           :(31 - 1 + 1);
	} bitfield;
} drex_ge_dfi_init_start_t;                // 0x0004

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t update_mode                          :(0 - 0 + 1);
		rw_bf_t reserved_1_1                            :(1 - 1 + 1);
		rw_bf_t pause_sref                           :(2 - 2 + 1);
		rw_bf_t ignore_dic                           :(3 - 3 + 1);
		rw_bf_t same_arid_sched_window               :(9 - 4 + 1);
		rw_bf_t reserved_10_15                          :(15 - 10 + 1);
		rw_bf_t max_tr_per_to_gen                    :(23 - 16 + 1);
		rw_bf_t max_tick_per_to_gen                  :(31 - 24 + 1);
	} bitfield;
} drex_ge_controller_control_t;            // 0x0008

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t lpix_resync_en                       :(0 - 0 + 1);
		rw_bf_t pause_resync_en                      :(1 - 1 + 1);
		rw_bf_t prohibit_phyupd_ack_lpi_q            :(2 - 2 + 1);
		rw_bf_t start_mc_update                      :(3 - 3 + 1);
		rw_bf_t t_ctrlupd_max                        :(10 - 4 + 1);
		rw_bf_t qchx_resync_en                       :(11 - 11 + 1);
		rw_bf_t rec_gate_cyc                         :(15 - 12 + 1);
		rw_bf_t dfi_freq_ratio                       :(17 - 16 + 1);
		rw_bf_t reserved_18_19                          :(19 - 18 + 1);
		rw_bf_t phy_cg_cyc                           :(27 - 20 + 1);
		rw_bf_t reserved_28_28                          :(28 - 28 + 1);
		rw_bf_t ctrl_pd                              :(29 - 29 + 1);
		rw_bf_t io_pd_con                            :(31 - 30 + 1);
	} bitfield;
} drex_ge_phy_control_t;                   // 0x000C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t clk_stop_en                          :(0 - 0 + 1);
		rw_bf_t dpwrdn_en                            :(1 - 1 + 1);
		rw_bf_t dpwrdn_type                          :(3 - 2 + 1);
		rw_bf_t dsref_en                             :(4 - 4 + 1);
		rw_bf_t pzq_en                               :(5 - 5 + 1);
		rw_bf_t dbi_en                               :(6 - 6 + 1);
		rw_bf_t mrr_bl4                              :(7 - 7 + 1);
		rw_bf_t mem_type                             :(11 - 8 + 1);
		rw_bf_t mem_width                            :(15 - 12 + 1);
		rw_bf_t mem_bl                               :(19 - 16 + 1);
		rw_bf_t mem_bl_otf2x                         :(20 - 20 + 1);
		rw_bf_t t_pbr2pbr_en                         :(21 - 21 + 1);
		rw_bf_t sr_abort_en                          :(22 - 22 + 1);
		rw_bf_t reserved_23_23                          :(23 - 23 + 1);
		rw_bf_t global_t_rrd                         :(24 - 24 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_memory_control0_t;               // 0x0010

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t aref_en                              :(0 - 0 + 1);
		rw_bf_t aref_postpone_limit                  :(3 - 1 + 1);
		rw_bf_t ref_rate_con_en                      :(4 - 4 + 1);
		rw_bf_t ref_rate_mr48                        :(5 - 5 + 1);
		rw_bf_t ban_cas_at_refab_o                   :(6 - 6 + 1);
		rw_bf_t reserved_7_7                            :(7 - 7 + 1);
		rw_bf_t ref_rate_sw                          :(11 - 8 + 1);
		rw_bf_t hightemp_th                          :(15 - 12 + 1);
		rw_bf_t derate_th                            :(19 - 16 + 1);
		rw_bf_t ref_rate_monitoring_window           :(25 - 20 + 1);
		rw_bf_t refab_per_rank_en                    :(26 - 26 + 1);
		rw_bf_t ref_before_sref                      :(27 - 27 + 1);
		rw_bf_t hightemp_intr_en                     :(28 - 28 + 1);
		rw_bf_t normtemp_intr_en                     :(29 - 29 + 1);
		rw_bf_t temperr_intr_en                      :(30 - 30 + 1);
		rw_bf_t derate_intr_en                       :(31 - 31 + 1);
	} bitfield;
} drex_ge_refresh_control_t;               // 0x0014

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t dpwrdn_cyc                           :(7 - 0 + 1);
		rw_bf_t dcg_cyc                              :(15 - 8 + 1);
		rw_bf_t dsref_cyc                            :(28 - 16 + 1);
		rw_bf_t reserved_29_31                          :(31 - 29 + 1);
	} bitfield;
} drex_ge_power_down_config_t;             // 0x0018

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t vcf_cg_en                            :(0 - 0 + 1);
		rw_bf_t vcf_wr_cg_en                         :(1 - 1 + 1);
		rw_bf_t vcf_rd_cg_en                         :(2 - 2 + 1);
		rw_bf_t apb_cg_en                            :(3 - 3 + 1);
		rw_bf_t secure_apb_cg_en                     :(4 - 4 + 1);
		rw_bf_t reserved_5_7                            :(7 - 5 + 1);
		rw_bf_t sch_cg_en                            :(8 - 8 + 1);
		rw_bf_t sch_slice0_cg_en                     :(9 - 9 + 1);
		rw_bf_t sch_slice1_cg_en                     :(10 - 10 + 1);
		rw_bf_t reserved_11_15                          :(15 - 11 + 1);
		rw_bf_t memif_cg_en                          :(16 - 16 + 1);
		rw_bf_t memif_wr_cg_en                       :(17 - 17 + 1);
		rw_bf_t memif_rd_cg_en                       :(18 - 18 + 1);
		rw_bf_t reserved_19_23                          :(23 - 19 + 1);
		rw_bf_t phy_cg_en                            :(24 - 24 + 1);
		rw_bf_t pclk_cg_en                           :(25 - 25 + 1);
		rw_bf_t pclk_secure_cg_en                    :(26 - 26 + 1);
		rw_bf_t pclk_ppmpu_cg_en                     :(27 - 27 + 1);
		rw_bf_t ppmpu_apb_cg_en                      :(28 - 28 + 1);
		rw_bf_t ppmpu_cg_en                          :(29 - 29 + 1);
		rw_bf_t reserved_30_31                          :(31 - 30 + 1);
	} bitfield;
} drex_ge_cg_control_t;                    // 0x001C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t dcmd_ma                              :(7 - 0 + 1);
		rw_bf_t dcmd_op                              :(15 - 8 + 1);
		rw_bf_t reserved_16_19                          :(19 - 16 + 1);
		rw_bf_t dcmd_bank                            :(22 - 20 + 1);
		rw_bf_t reserved_23_23                          :(23 - 23 + 1);
		rw_bf_t dcmd_rank                            :(25 - 24 + 1);
		rw_bf_t dcmd_rank_all                        :(26 - 26 + 1);
		rw_bf_t reserved_27_27                          :(27 - 27 + 1);
		rw_bf_t dcmd_type                            :(31 - 28 + 1);
	} bitfield;
} drex_ge_direct_command_t;                // 0x0020

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t ca_1st                              :(5 - 0 + 1);
		rw_bf_t ca_2nd                              :(11 - 6 + 1);
		rw_bf_t ca_3rd                              :(17 - 12 + 1);
		rw_bf_t ca_4th                              :(23 - 18 + 1);
		rw_bf_t target_rank                         :(25 - 24 + 1);
		rw_bf_t rank_all                            :(26 - 26 + 1);
		rw_bf_t udc_short                           :(27 - 27 + 1);
		rw_bf_t dcmd_type                           :(31 - 28 + 1);
	} bitfield;
} drex_ge_direct_command_tmrs_t;           // 0x0020

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t timing_set_sw                        :(0 - 0 + 1);
		rw_bf_t reserved_1_31                           :(31 - 1 + 1);
	} bitfield;
} drex_ge_timing_set_sw_t;                 // 0x0024

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t port_policy                          :(7 - 0 + 1);
		rw_bf_t tp_en                                :(15 - 8 + 1);
		rw_bf_t reserved_16_31                          :(31 - 16 + 1);
	} bitfield;
} drex_ge_precharge_control_t;             // 0x0028

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t tp_cnt_p0                            :(7 - 0 + 1);
		rw_bf_t tp_cnt_p1                            :(15 - 8 + 1);
		rw_bf_t tp_cnt_p2                            :(23 - 16 + 1);
		rw_bf_t tp_cnt_p3                            :(31 - 24 + 1);
	} bitfield;
} drex_ge_precharge_config0_t;             // 0x002C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t tp_cnt_p4                            :(7 - 0 + 1);
		rw_bf_t tp_cnt_p5                            :(15 - 8 + 1);
		rw_bf_t tp_cnt_p6                            :(23 - 16 + 1);
		rw_bf_t tp_cnt_p7                            :(31 - 24 + 1);
	} bitfield;
} drex_ge_precharge_config1_t;             // 0x0030

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t write_flush_brb_low                  :(7 - 0 + 1);
		rw_bf_t write_flush_brb_high                 :(15 - 8 + 1);
		rw_bf_t write_flush_wdbuf_low                :(23 - 16 + 1);
		rw_bf_t write_flush_wdbuf_high               :(31 - 24 + 1);
	} bitfield;
} drex_ge_write_flush_config0_t;           // 0x0034

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t no_read_to_write_delay               :(7 - 0 + 1);
		rw_bf_t min_num_rd_cas                       :(15 - 8 + 1);
		rw_bf_t min_num_wr_cas                       :(23 - 16 + 1);
		rw_bf_t early_write_flush_rdbuf_th           :(31 - 24 + 1);
	} bitfield;
} drex_ge_write_flush_config1_t;           // 0x0038

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t axid_or_axmarker                     :(0 - 0 + 1);
		rw_bf_t reserved_1_31                           :(31 - 1 + 1);
	} bitfield;
} drex_ge_info_config_t;                   // 0x003C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_zqcs                               :(7 - 0 + 1);
		rw_bf_t t_zqoper                             :(16 - 8 + 1);
		rw_bf_t t_zqinit                             :(26 - 17 + 1);
		rw_bf_t reserved_27_31                          :(31 - 27 + 1);
	} bitfield;
} drex_ge_timing_zq_t;                     // 0x0040

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_pzq                                :(23 - 0 + 1);
		rw_bf_t reserved_24_31                          :(31 - 24 + 1);
	} bitfield;
} drex_ge_timing_pzq_t;                    // 0x0044

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_mrr                                :(3 - 0 + 1);
		rw_bf_t t_fc                                 :(13 - 4 + 1);
		rw_bf_t reserved_14_31                          :(31 - 14 + 1);
	} bitfield;
} drex_ge_timing_etc0_t;                   // 0x0048

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_refi                               :(14 - 0 + 1);
		rw_bf_t reserved_15_15                          :(15 - 15 + 1);
		rw_bf_t t_refipb                             :(27 - 16 + 1);
		rw_bf_t reserved_28_31                          :(31 - 28 + 1);
	} bitfield;
} drex_ge_timing_ref_0_t;                  // 0x0050

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_refi                               :(14 - 0 + 1);
		rw_bf_t reserved_15_15                          :(15 - 15 + 1);
		rw_bf_t t_refipb                             :(27 - 16 + 1);
		rw_bf_t reserved_28_31                          :(31 - 28 + 1);
	} bitfield;
} drex_ge_timing_ref_1_t;                  // 0x0054

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_refi                               :(14 - 0 + 1);
		rw_bf_t reserved_15_15                          :(15 - 15 + 1);
		rw_bf_t t_refipb                             :(27 - 16 + 1);
		rw_bf_t reserved_28_31                          :(31 - 28 + 1);
	} bitfield;
} drex_ge_timing_ref_2_t;                  // 0x0058

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_refi                               :(14 - 0 + 1);
		rw_bf_t reserved_15_15                          :(15 - 15 + 1);
		rw_bf_t t_refipb                             :(27 - 16 + 1);
		rw_bf_t reserved_28_31                          :(31 - 28 + 1);
	} bitfield;
} drex_ge_timing_ref_3_t;                  // 0x005C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_refi                               :(14 - 0 + 1);
		rw_bf_t reserved_15_15                          :(15 - 15 + 1);
		rw_bf_t t_refipb                             :(27 - 16 + 1);
		rw_bf_t reserved_28_31                          :(31 - 28 + 1);
	} bitfield;
} drex_ge_timing_ref_4_t;                  // 0x0060

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_refi                               :(14 - 0 + 1);
		rw_bf_t reserved_15_15                          :(15 - 15 + 1);
		rw_bf_t t_refipb                             :(27 - 16 + 1);
		rw_bf_t reserved_28_31                          :(31 - 28 + 1);
	} bitfield;
} drex_ge_timing_ref_5_t;                  // 0x0064

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_refi                               :(14 - 0 + 1);
		rw_bf_t reserved_15_15                          :(15 - 15 + 1);
		rw_bf_t t_refipb                             :(27 - 16 + 1);
		rw_bf_t reserved_28_31                          :(31 - 28 + 1);
	} bitfield;
} drex_ge_timing_ref_6_t;                  // 0x0068

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_refi                               :(14 - 0 + 1);
		rw_bf_t reserved_15_15                          :(15 - 15 + 1);
		rw_bf_t t_refipb                             :(27 - 16 + 1);
		rw_bf_t reserved_28_31                          :(31 - 28 + 1);
	} bitfield;
} drex_ge_timing_ref_7_t;                  // 0x006C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_refi                               :(14 - 0 + 1);
		rw_bf_t reserved_15_15                          :(15 - 15 + 1);
		rw_bf_t t_refipb                             :(27 - 16 + 1);
		rw_bf_t reserved_28_31                          :(31 - 28 + 1);
	} bitfield;
} drex_ge_timing_ref_8_t;                  // 0x0070

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_refi                               :(14 - 0 + 1);
		rw_bf_t reserved_15_15                          :(15 - 15 + 1);
		rw_bf_t t_refipb                             :(27 - 16 + 1);
		rw_bf_t reserved_28_31                          :(31 - 28 + 1);
	} bitfield;
} drex_ge_timing_ref_9_t;                  // 0x0074

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_refi                               :(14 - 0 + 1);
		rw_bf_t reserved_15_15                          :(15 - 15 + 1);
		rw_bf_t t_refipb                             :(27 - 16 + 1);
		rw_bf_t reserved_28_31                          :(31 - 28 + 1);
	} bitfield;
} drex_ge_timing_ref_10_t;                 // 0x0078

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_refi                               :(14 - 0 + 1);
		rw_bf_t reserved_15_15                          :(15 - 15 + 1);
		rw_bf_t t_refipb                             :(27 - 16 + 1);
		rw_bf_t reserved_28_31                          :(31 - 28 + 1);
	} bitfield;
} drex_ge_timing_ref_11_t;                 // 0x007C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_refi                               :(14 - 0 + 1);
		rw_bf_t reserved_15_15                          :(15 - 15 + 1);
		rw_bf_t t_refipb                             :(27 - 16 + 1);
		rw_bf_t reserved_28_31                          :(31 - 28 + 1);
	} bitfield;
} drex_ge_timing_ref_12_t;                 // 0x0080

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_refi                               :(14 - 0 + 1);
		rw_bf_t reserved_15_15                          :(15 - 15 + 1);
		rw_bf_t t_refipb                             :(27 - 16 + 1);
		rw_bf_t reserved_28_31                          :(31 - 28 + 1);
	} bitfield;
} drex_ge_timing_ref_13_t;                 // 0x0084

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_refi                               :(14 - 0 + 1);
		rw_bf_t reserved_15_15                          :(15 - 15 + 1);
		rw_bf_t t_refipb                             :(27 - 16 + 1);
		rw_bf_t reserved_28_31                          :(31 - 28 + 1);
	} bitfield;
} drex_ge_timing_ref_14_t;                 // 0x0088

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_refi                               :(14 - 0 + 1);
		rw_bf_t reserved_15_15                          :(15 - 15 + 1);
		rw_bf_t t_refipb                             :(27 - 16 + 1);
		rw_bf_t reserved_28_31                          :(31 - 28 + 1);
	} bitfield;
} drex_ge_timing_ref_15_t;                 // 0x008C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t ma                                   :(7 - 0 + 1);
		rw_bf_t op                                   :(15 - 8 + 1);
		rw_bf_t reserved_16_23                          :(23 - 16 + 1);
		rw_bf_t rank                                 :(25 - 24 + 1);
		rw_bf_t rank_all                             :(26 - 26 + 1);
		rw_bf_t reserved_27_29                          :(29 - 27 + 1);
		rw_bf_t pre                                  :(30 - 30 + 1);
		rw_bf_t valid                                :(31 - 31 + 1);
	} bitfield;
} drex_ge_pause_mrs0_t;                    // 0x0090

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t ma                                   :(7 - 0 + 1);
		rw_bf_t op                                   :(15 - 8 + 1);
		rw_bf_t reserved_16_23                          :(23 - 16 + 1);
		rw_bf_t rank                                 :(25 - 24 + 1);
		rw_bf_t rank_all                             :(26 - 26 + 1);
		rw_bf_t reserved_27_29                          :(29 - 27 + 1);
		rw_bf_t pre                                  :(30 - 30 + 1);
		rw_bf_t valid                                :(31 - 31 + 1);
	} bitfield;
} drex_ge_pause_mrs1_t;                    // 0x0094

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t ma                                   :(7 - 0 + 1);
		rw_bf_t op                                   :(15 - 8 + 1);
		rw_bf_t reserved_16_23                          :(23 - 16 + 1);
		rw_bf_t rank                                 :(25 - 24 + 1);
		rw_bf_t rank_all                             :(26 - 26 + 1);
		rw_bf_t reserved_27_29                          :(29 - 27 + 1);
		rw_bf_t pre                                  :(30 - 30 + 1);
		rw_bf_t valid                                :(31 - 31 + 1);
	} bitfield;
} drex_ge_pause_mrs2_t;                    // 0x0098

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t ma                                   :(7 - 0 + 1);
		rw_bf_t op                                   :(15 - 8 + 1);
		rw_bf_t reserved_16_23                          :(23 - 16 + 1);
		rw_bf_t rank                                 :(25 - 24 + 1);
		rw_bf_t rank_all                             :(26 - 26 + 1);
		rw_bf_t reserved_27_29                          :(29 - 27 + 1);
		rw_bf_t pre                                  :(30 - 30 + 1);
		rw_bf_t valid                                :(31 - 31 + 1);
	} bitfield;
} drex_ge_pause_mrs3_t;                    // 0x009C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t ma                                   :(7 - 0 + 1);
		rw_bf_t op                                   :(15 - 8 + 1);
		rw_bf_t reserved_16_23                          :(23 - 16 + 1);
		rw_bf_t rank                                 :(25 - 24 + 1);
		rw_bf_t rank_all                             :(26 - 26 + 1);
		rw_bf_t reserved_27_29                          :(29 - 27 + 1);
		rw_bf_t pre                                  :(30 - 30 + 1);
		rw_bf_t valid                                :(31 - 31 + 1);
	} bitfield;
} drex_ge_pause_mrs4_t;                    // 0x00A0

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t zq_status                            :(2 - 0 + 1);
		rw_bf_t reserved_3_31                           :(31 - 3 + 1);
	} bitfield;
} drex_ge_zq_status_r0_t;                  // 0x00D0

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t zq_status                            :(2 - 0 + 1);
		rw_bf_t reserved_3_31                           :(31 - 3 + 1);
	} bitfield;
} drex_ge_zq_status_r1_t;                  // 0x00D4

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t mrr_1st_data                         :(7 - 0 + 1);
		rw_bf_t mrr_2nd_data                         :(15 - 8 + 1);
		rw_bf_t mrr_2nd_data_valid                   :(16 - 16 + 1);
		rw_bf_t reserved_17_31                          :(31 - 17 + 1);
	} bitfield;
} drex_ge_mr_status_t;                     // 0x00EC

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t dfi_init_complete                    :(0 - 0 + 1);
		rw_bf_t init_train_complete                  :(1 - 1 + 1);
		rw_bf_t timing_set_sw                        :(2 - 2 + 1);
		rw_bf_t reserved_3_31                           :(31 - 3 + 1);
	} bitfield;
} drex_ge_phy_status_t;                    // 0x00F0

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t ref_rate                             :(3 - 0 + 1);
		rw_bf_t changed_to_refab                     :(4 - 4 + 1);
		rw_bf_t derate_timing                        :(5 - 5 + 1);
		rw_bf_t burned                               :(6 - 6 + 1);
		rw_bf_t hightemp_intr                        :(7 - 7 + 1);
		rw_bf_t normtemp_intr                        :(8 - 8 + 1);
		rw_bf_t temperr_intr                         :(9 - 9 + 1);
		rw_bf_t derate_intr                          :(10 - 10 + 1);
		rw_bf_t htemp_fatal                          :(11 - 11 + 1);
		rw_bf_t ltemp_fatal                          :(12 - 12 + 1);
		rw_bf_t reserved_13_31                          :(31 - 13 + 1);
	} bitfield;
} drex_ge_memory_status_t;                 // 0x00F4

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t hightemp_intr_test                   :(0 - 0 + 1);
		rw_bf_t normtemp_intr_test                   :(1 - 1 + 1);
		rw_bf_t temperr_intr_test                    :(2 - 2 + 1);
		rw_bf_t derate_intr_test                     :(3 - 3 + 1);
		rw_bf_t reserved_4_30                           :(30 - 4 + 1);
		rw_bf_t temp_intr_test_mode_en               :(31 - 31 + 1);
	} bitfield;
} drex_ge_temp_intr_test_t;                // 0x00F8

typedef union {
	volatile unsigned int data;
	struct {
		ro_bf_t hightemp_intr_clear                  :(0 - 0 + 1);
		ro_bf_t normtemp_intr_clear                  :(1 - 1 + 1);
		ro_bf_t temperr_intr_clear                   :(2 - 2 + 1);
		ro_bf_t derate_intr_clear                    :(3 - 3 + 1);
		rw_bf_t reserved_4_31                           :(31 - 4 + 1);
	} bitfield;
} drex_ge_temp_intr_clear_t;               // 0x00FC

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t tick_granularity                     :(4 - 0 + 1);
		rw_bf_t reserved_5_31                           :(31 - 5 + 1);
	} bitfield;
} drex_ge_tick_granularity_s0_t;           // 0x0100

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t refpb_en                             :(0 - 0 + 1);
		rw_bf_t reserved_1_3                            :(3 - 1 + 1);
		rw_bf_t refpb_th                             :(7 - 4 + 1);
		rw_bf_t geardown_en                          :(8 - 8 + 1);
		rw_bf_t dll_on                               :(9 - 9 + 1);
		rw_bf_t reserved_10_14                          :(14 - 10 + 1);
		rw_bf_t use_t_rppb_w                         :(15 - 15 + 1);
		rw_bf_t write_odt_en                         :(16 - 16 + 1);
		rw_bf_t write_odt_type                       :(18 - 17 + 1);
		rw_bf_t reserved_19_19                          :(19 - 19 + 1);
		rw_bf_t t_odte                               :(23 - 20 + 1);
		rw_bf_t ca_odt_en                            :(27 - 24 + 1);
		rw_bf_t ca_stable                            :(28 - 28 + 1);
		rw_bf_t reserved_29_31                          :(31 - 29 + 1);
	} bitfield;
} drex_ge_memory_control1_s0_t;            // 0x0104

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t mr4_sensing_cyc                      :(31 - 0 + 1);
	} bitfield;
} drex_ge_temp_sensing_s0_t;               // 0x0108

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t use_prog_window                      :(0 - 0 + 1);
		rw_bf_t reserved_1_3                            :(3 - 1 + 1);
		rw_bf_t p0_delay                             :(5 - 4 + 1);
		rw_bf_t reserved_6_7                            :(7 - 6 + 1);
		rw_bf_t p1_delay                             :(9 - 8 + 1);
		rw_bf_t reserved_10_11                          :(11 - 10 + 1);
		rw_bf_t p0_extend                            :(13 - 12 + 1);
		rw_bf_t reserved_14_15                          :(15 - 14 + 1);
		rw_bf_t p1_extend                            :(17 - 16 + 1);
		rw_bf_t reserved_18_31                          :(31 - 18 + 1);
	} bitfield;
} drex_ge_winconfig_write_odt_s0_t;        // 0x0118

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_pbr2pbr                            :(5 - 0 + 1);
		rw_bf_t reserved_6_7                            :(7 - 6 + 1);
		rw_bf_t t_xsr_abort                          :(15 - 8 + 1);
		rw_bf_t reserved_16_31                          :(31 - 16 + 1);
	} bitfield;
} drex_ge_timing_row_lpddr4_s0_t;          // 0x013C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_faw                                :(5 - 0 + 1);
		rw_bf_t reserved_6_7                            :(7 - 6 + 1);
		rw_bf_t t_ras                                :(13 - 8 + 1);
		rw_bf_t reserved_14_15                          :(15 - 14 + 1);
		rw_bf_t t_rcd                                :(19 - 16 + 1);
		rw_bf_t t_rrd                                :(23 - 20 + 1);
		rw_bf_t t_rppb_w                             :(29 - 24 + 1);
		rw_bf_t reserved_30_31                          :(31 - 30 + 1);
	} bitfield;
} drex_ge_timing_row0_s0_t;                // 0x0140

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_rpab                               :(5 - 0 + 1);
		rw_bf_t reserved_6_7                            :(7 - 6 + 1);
		rw_bf_t t_rppb                               :(13 - 8 + 1);
		rw_bf_t reserved_14_15                          :(15 - 14 + 1);
		rw_bf_t t_rfcab                              :(23 - 16 + 1);
		rw_bf_t t_rfcpb                              :(30 - 24 + 1);
		rw_bf_t reserved_31_31                          :(31 - 31 + 1);
	} bitfield;
} drex_ge_timing_row1_s0_t;                // 0x0144

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t rd_lat                               :(5 - 0 + 1);
		rw_bf_t reserved_6_7                            :(7 - 6 + 1);
		rw_bf_t t_dqsck_min                          :(11 - 8 + 1);
		rw_bf_t t_dqsck_max                          :(15 - 12 + 1);
		rw_bf_t t_rtp                                :(20 - 16 + 1);
		rw_bf_t reserved_21_23                          :(23 - 21 + 1);
		rw_bf_t t_rpste                              :(24 - 24 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_timing_read_s0_t;                // 0x0148

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t wr_lat                               :(4 - 0 + 1);
		rw_bf_t reserved_5_7                            :(7 - 5 + 1);
		rw_bf_t t_wtr                                :(12 - 8 + 1);
		rw_bf_t reserved_13_15                          :(15 - 13 + 1);
		rw_bf_t t_wr                                 :(21 - 16 + 1);
		rw_bf_t reserved_22_23                          :(23 - 22 + 1);
		rw_bf_t t_ccdmw                              :(29 - 24 + 1);
		rw_bf_t reserved_30_31                          :(31 - 30 + 1);
	} bitfield;
} drex_ge_timing_write_s0_t;               // 0x014C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t odtlon                               :(3 - 0 + 1);
		rw_bf_t reserved_4_7                            :(7 - 4 + 1);
		rw_bf_t odtloff                              :(12 - 8 + 1);
		rw_bf_t reserved_13_15                          :(15 - 13 + 1);
		rw_bf_t t_odton_min                          :(19 - 16 + 1);
		rw_bf_t reserved_20_23                          :(23 - 20 + 1);
		rw_bf_t t_odtoff_max                         :(27 - 24 + 1);
		rw_bf_t reserved_28_31                          :(31 - 28 + 1);
	} bitfield;
} drex_ge_timing_write_odt_s0_t;           // 0x0150

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t r2w_add_gap                          :(3 - 0 + 1);
		rw_bf_t w2r_add_gap                          :(7 - 4 + 1);
		rw_bf_t r2w_r2r_add_gap                      :(11 - 8 + 1);
		rw_bf_t w2r_r2r_add_gap                      :(15 - 12 + 1);
		rw_bf_t r2r_r2r_add_gap                      :(19 - 16 + 1);
		rw_bf_t w2w_r2r_add_gap                      :(23 - 20 + 1);
		rw_bf_t reserved_24_31                          :(31 - 24 + 1);
	} bitfield;
} drex_ge_timing_data_gap_s0_t;            // 0x0154

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_rddata_en                          :(6 - 0 + 1);
		rw_bf_t reserved_7_7                            :(7 - 7 + 1);
		rw_bf_t t_phy_rdlat                          :(11 - 8 + 1);
		rw_bf_t reserved_12_15                          :(15 - 12 + 1);
		rw_bf_t t_phy_wrdata                         :(18 - 16 + 1);
		rw_bf_t reserved_19_19                          :(19 - 19 + 1);
		rw_bf_t t_phy_wrcsgap                        :(23 - 20 + 1);
		rw_bf_t t_phy_wrlat                          :(28 - 24 + 1);
		rw_bf_t reserved_29_31                          :(31 - 29 + 1);
	} bitfield;
} drex_ge_timing_data_phy_s0_t;            // 0x0158

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_dqsck_add                          :(3 - 0 + 1);
		rw_bf_t t_rpab_add                           :(7 - 4 + 1);
		rw_bf_t t_rppb_add                           :(11 - 8 + 1);
		rw_bf_t t_ras_add                            :(15 - 12 + 1);
		rw_bf_t t_rcd_add                            :(19 - 16 + 1);
		rw_bf_t t_rrd_add                            :(23 - 20 + 1);
		rw_bf_t reserved_24_27                          :(27 - 24 + 1);
		rw_bf_t t_rppb_w_add                         :(31 - 28 + 1);
	} bitfield;
} drex_ge_timing_derate_s0_t;              // 0x015C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_cke                                :(3 - 0 + 1);
		rw_bf_t reserved_4_7                            :(7 - 4 + 1);
		rw_bf_t t_ckesr                              :(13 - 8 + 1);
		rw_bf_t reserved_14_15                          :(15 - 14 + 1);
		rw_bf_t t_xp                                 :(20 - 16 + 1);
		rw_bf_t reserved_21_23                          :(23 - 21 + 1);
		rw_bf_t t_xsr                                :(31 - 24 + 1);
	} bitfield;
} drex_ge_timing_power_s0_t;               // 0x0160

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_xp_dll                             :(7 - 0 + 1);
		rw_bf_t t_xs_dll                             :(19 - 8 + 1);
		rw_bf_t t_xs_fast                            :(27 - 20 + 1);
		rw_bf_t reserved_28_31                          :(31 - 28 + 1);
	} bitfield;
} drex_ge_timing_power_ddr_s0_t;           // 0x0164

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_cal                                :(2 - 0 + 1);
		rw_bf_t reserved_3_3                            :(3 - 3 + 1);
		rw_bf_t t_ccd_l                              :(7 - 4 + 1);
		rw_bf_t t_rrd_l                              :(11 - 8 + 1);
		rw_bf_t t_wtr_l                              :(16 - 12 + 1);
		rw_bf_t reserved_17_31                          :(31 - 17 + 1);
	} bitfield;
} drex_ge_timing_ddr4_s0_t;                // 0x0168

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_mrd                                :(3 - 0 + 1);
		rw_bf_t t_vrcg_disable                       :(11 - 4 + 1);
		rw_bf_t t_zqlat                              :(16 - 12 + 1);
		rw_bf_t reserved_17_31                          :(31 - 17 + 1);
	} bitfield;
} drex_ge_timing_etc1_s0_t;                // 0x016C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_rdlvl_en                           :(3 - 0 + 1);
		rw_bf_t t_rdlvl_rr                           :(9 - 4 + 1);
		rw_bf_t reserved_10_11                          :(11 - 10 + 1);
		rw_bf_t t_wrtrn_en                           :(15 - 12 + 1);
		rw_bf_t t_wrtrn_wr                           :(21 - 16 + 1);
		rw_bf_t reserved_22_23                          :(23 - 22 + 1);
		rw_bf_t t_wrtrn_rw                           :(29 - 24 + 1);
		rw_bf_t reserved_30_31                          :(31 - 30 + 1);
	} bitfield;
} drex_ge_timing_train0_s0_t;              // 0x0170

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_periodic_period                    :(23 - 0 + 1);
		rw_bf_t reserved_24_31                          :(31 - 24 + 1);
	} bitfield;
} drex_ge_timing_train1_s0_t;              // 0x0174

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t tick_granularity                     :(4 - 0 + 1);
		rw_bf_t reserved_5_31                           :(31 - 5 + 1);
	} bitfield;
} drex_ge_tick_granularity_s1_t;           // 0x0180

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t refpb_en                             :(0 - 0 + 1);
		rw_bf_t reserved_1_3                            :(3 - 1 + 1);
		rw_bf_t refpb_th                             :(7 - 4 + 1);
		rw_bf_t geardown_en                          :(8 - 8 + 1);
		rw_bf_t dll_on                               :(9 - 9 + 1);
		rw_bf_t reserved_10_14                          :(14 - 10 + 1);
		rw_bf_t use_t_rppb_w                         :(15 - 15 + 1);
		rw_bf_t write_odt_en                         :(16 - 16 + 1);
		rw_bf_t write_odt_type                       :(18 - 17 + 1);
		rw_bf_t reserved_19_19                          :(19 - 19 + 1);
		rw_bf_t t_odte                               :(23 - 20 + 1);
		rw_bf_t ca_odt_en                            :(27 - 24 + 1);
		rw_bf_t ca_stable                            :(28 - 28 + 1);
		rw_bf_t reserved_29_31                          :(31 - 29 + 1);
	} bitfield;
} drex_ge_memory_control1_s1_t;            // 0x0184

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t mr4_sensing_cyc                      :(31 - 0 + 1);
	} bitfield;
} drex_ge_temp_sensing_s1_t;               // 0x0188

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t use_prog_window                      :(0 - 0 + 1);
		rw_bf_t reserved_1_3                            :(3 - 1 + 1);
		rw_bf_t p0_delay                             :(5 - 4 + 1);
		rw_bf_t reserved_6_7                            :(7 - 6 + 1);
		rw_bf_t p1_delay                             :(9 - 8 + 1);
		rw_bf_t reserved_10_11                          :(11 - 10 + 1);
		rw_bf_t p0_extend                            :(13 - 12 + 1);
		rw_bf_t reserved_14_15                          :(15 - 14 + 1);
		rw_bf_t p1_extend                            :(17 - 16 + 1);
		rw_bf_t reserved_18_31                          :(31 - 18 + 1);
	} bitfield;
} drex_ge_winconfig_write_odt_s1_t;        // 0x0198

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_pbr2pbr                            :(5 - 0 + 1);
		rw_bf_t reserved_6_7                            :(7 - 6 + 1);
		rw_bf_t t_xsr_abort                          :(15 - 8 + 1);
		rw_bf_t reserved_16_31                          :(31 - 16 + 1);
	} bitfield;
} drex_ge_timing_row_lpddr4_s1_t;          // 0x01BC

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_faw                                :(5 - 0 + 1);
		rw_bf_t reserved_6_7                            :(7 - 6 + 1);
		rw_bf_t t_ras                                :(13 - 8 + 1);
		rw_bf_t reserved_14_15                          :(15 - 14 + 1);
		rw_bf_t t_rcd                                :(19 - 16 + 1);
		rw_bf_t t_rrd                                :(23 - 20 + 1);
		rw_bf_t t_rppb_w                             :(29 - 24 + 1);
		rw_bf_t reserved_30_31                          :(31 - 30 + 1);
	} bitfield;
} drex_ge_timing_row0_s1_t;                // 0x01C0

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_rpab                               :(5 - 0 + 1);
		rw_bf_t reserved_6_7                            :(7 - 6 + 1);
		rw_bf_t t_rppb                               :(13 - 8 + 1);
		rw_bf_t reserved_14_15                          :(15 - 14 + 1);
		rw_bf_t t_rfcab                              :(23 - 16 + 1);
		rw_bf_t t_rfcpb                              :(30 - 24 + 1);
		rw_bf_t reserved_31_31                          :(31 - 31 + 1);
	} bitfield;
} drex_ge_timing_row1_s1_t;                // 0x01C4

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t rd_lat                               :(5 - 0 + 1);
		rw_bf_t reserved_6_7                            :(7 - 6 + 1);
		rw_bf_t t_dqsck_min                          :(11 - 8 + 1);
		rw_bf_t t_dqsck_max                          :(15 - 12 + 1);
		rw_bf_t t_rtp                                :(20 - 16 + 1);
		rw_bf_t reserved_21_23                          :(23 - 21 + 1);
		rw_bf_t t_rpste                              :(24 - 24 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_timing_read_s1_t;                // 0x01C8

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t wr_lat                               :(4 - 0 + 1);
		rw_bf_t reserved_5_7                            :(7 - 5 + 1);
		rw_bf_t t_wtr                                :(12 - 8 + 1);
		rw_bf_t reserved_13_15                          :(15 - 13 + 1);
		rw_bf_t t_wr                                 :(21 - 16 + 1);
		rw_bf_t reserved_22_23                          :(23 - 22 + 1);
		rw_bf_t t_ccdmw                              :(29 - 24 + 1);
		rw_bf_t reserved_30_31                          :(31 - 30 + 1);
	} bitfield;
} drex_ge_timing_write_s1_t;               // 0x01CC

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t odtlon                               :(3 - 0 + 1);
		rw_bf_t reserved_4_7                            :(7 - 4 + 1);
		rw_bf_t odtloff                              :(12 - 8 + 1);
		rw_bf_t reserved_13_15                          :(15 - 13 + 1);
		rw_bf_t t_odton_min                          :(19 - 16 + 1);
		rw_bf_t reserved_20_23                          :(23 - 20 + 1);
		rw_bf_t t_odtoff_max                         :(27 - 24 + 1);
		rw_bf_t reserved_28_31                          :(31 - 28 + 1);
	} bitfield;
} drex_ge_timing_write_odt_s1_t;           // 0x01D0

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t r2w_add_gap                          :(3 - 0 + 1);
		rw_bf_t w2r_add_gap                          :(7 - 4 + 1);
		rw_bf_t r2w_r2r_add_gap                      :(11 - 8 + 1);
		rw_bf_t w2r_r2r_add_gap                      :(15 - 12 + 1);
		rw_bf_t r2r_r2r_add_gap                      :(19 - 16 + 1);
		rw_bf_t w2w_r2r_add_gap                      :(23 - 20 + 1);
		rw_bf_t reserved_24_31                          :(31 - 24 + 1);
	} bitfield;
} drex_ge_timing_data_gap_s1_t;            // 0x01D4

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_rddata_en                          :(6 - 0 + 1);
		rw_bf_t reserved_7_7                            :(7 - 7 + 1);
		rw_bf_t t_phy_rdlat                          :(11 - 8 + 1);
		rw_bf_t reserved_12_15                          :(15 - 12 + 1);
		rw_bf_t t_phy_wrdata                         :(18 - 16 + 1);
		rw_bf_t reserved_19_19                          :(19 - 19 + 1);
		rw_bf_t t_phy_wrcsgap                        :(23 - 20 + 1);
		rw_bf_t t_phy_wrlat                          :(28 - 24 + 1);
		rw_bf_t reserved_29_31                          :(31 - 29 + 1);
	} bitfield;
} drex_ge_timing_data_phy_s1_t;            // 0x01D8

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_dqsck_add                          :(3 - 0 + 1);
		rw_bf_t t_rpab_add                           :(7 - 4 + 1);
		rw_bf_t t_rppb_add                           :(11 - 8 + 1);
		rw_bf_t t_ras_add                            :(15 - 12 + 1);
		rw_bf_t t_rcd_add                            :(19 - 16 + 1);
		rw_bf_t t_rrd_add                            :(23 - 20 + 1);
		rw_bf_t reserved_24_27                          :(27 - 24 + 1);
		rw_bf_t t_rppb_w_add                         :(31 - 28 + 1);
	} bitfield;
} drex_ge_timing_derate_s1_t;              // 0x01DC

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_cke                                :(3 - 0 + 1);
		rw_bf_t reserved_4_7                            :(7 - 4 + 1);
		rw_bf_t t_ckesr                              :(13 - 8 + 1);
		rw_bf_t reserved_14_15                          :(15 - 14 + 1);
		rw_bf_t t_xp                                 :(20 - 16 + 1);
		rw_bf_t reserved_21_23                          :(23 - 21 + 1);
		rw_bf_t t_xsr                                :(31 - 24 + 1);
	} bitfield;
} drex_ge_timing_power_s1_t;               // 0x01E0

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_xp_dll                             :(7 - 0 + 1);
		rw_bf_t t_xs_dll                             :(19 - 8 + 1);
		rw_bf_t t_xs_fast                            :(27 - 20 + 1);
		rw_bf_t reserved_28_31                          :(31 - 28 + 1);
	} bitfield;
} drex_ge_timing_power_ddr_s1_t;           // 0x01E4

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_cal                                :(2 - 0 + 1);
		rw_bf_t reserved_3_3                            :(3 - 3 + 1);
		rw_bf_t t_ccd_l                              :(7 - 4 + 1);
		rw_bf_t t_rrd_l                              :(11 - 8 + 1);
		rw_bf_t t_wtr_l                              :(16 - 12 + 1);
		rw_bf_t reserved_17_31                          :(31 - 17 + 1);
	} bitfield;
} drex_ge_timing_ddr4_s1_t;                // 0x01E8

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_mrd                                :(3 - 0 + 1);
		rw_bf_t t_vrcg_disable                       :(11 - 4 + 1);
		rw_bf_t t_zqlat                              :(16 - 12 + 1);
		rw_bf_t reserved_17_31                          :(31 - 17 + 1);
	} bitfield;
} drex_ge_timing_etc1_s1_t;                // 0x01EC

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_rdlvl_en                           :(3 - 0 + 1);
		rw_bf_t t_rdlvl_rr                           :(9 - 4 + 1);
		rw_bf_t reserved_10_11                          :(11 - 10 + 1);
		rw_bf_t t_wrtrn_en                           :(15 - 12 + 1);
		rw_bf_t t_wrtrn_wr                           :(21 - 16 + 1);
		rw_bf_t reserved_22_23                          :(23 - 22 + 1);
		rw_bf_t t_wrtrn_rw                           :(29 - 24 + 1);
		rw_bf_t reserved_30_31                          :(31 - 30 + 1);
	} bitfield;
} drex_ge_timing_train0_s1_t;              // 0x01F0

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t t_periodic_period                    :(23 - 0 + 1);
		rw_bf_t reserved_24_31                          :(31 - 24 + 1);
	} bitfield;
} drex_ge_timing_train1_s1_t;              // 0x01F4

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t init_gate_train                      :(3 - 0 + 1);
		rw_bf_t init_read_train                      :(7 - 4 + 1);
		rw_bf_t init_write_train                     :(11 - 8 + 1);
		rw_bf_t reserved_12_31                          :(31 - 12 + 1);
	} bitfield;
} drex_ge_init_train_config_t;             // 0x0200

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t init_train_start                     :(0 - 0 + 1);
		rw_bf_t reserved_1_31                           :(31 - 1 + 1);
	} bitfield;
} drex_ge_init_train_control_t;            // 0x0204

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t sw_gate_train                        :(3 - 0 + 1);
		rw_bf_t sw_read_train                        :(7 - 4 + 1);
		rw_bf_t sw_write_train                       :(11 - 8 + 1);
		rw_bf_t reserved_12_31                          :(31 - 12 + 1);
	} bitfield;
} drex_ge_sw_train_config_t;               // 0x0208

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t sw_train_start                       :(0 - 0 + 1);
		rw_bf_t reserved_1_3                            :(3 - 1 + 1);
		rw_bf_t sw_gate_lvl_long                     :(4 - 4 + 1);
		rw_bf_t sw_read_lvl_long                     :(5 - 5 + 1);
		rw_bf_t sw_write_lvl_long                    :(6 - 6 + 1);
		rw_bf_t reserved_7_31                           :(31 - 7 + 1);
	} bitfield;
} drex_ge_sw_train_control_t;              // 0x020C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t periodic_gate_train                  :(3 - 0 + 1);
		rw_bf_t periodic_read_train                  :(7 - 4 + 1);
		rw_bf_t periodic_write_train                 :(11 - 8 + 1);
		rw_bf_t reserved_12_15                          :(15 - 12 + 1);
		rw_bf_t dvfs_gate_train                      :(19 - 16 + 1);
		rw_bf_t dvfs_read_train                      :(23 - 20 + 1);
		rw_bf_t dvfs_write_train                     :(27 - 24 + 1);
		rw_bf_t reserved_28_31                          :(31 - 28 + 1);
	} bitfield;
} drex_ge_ptrain_config_s0_t;              // 0x0220

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t ptrain_en                            :(0 - 0 + 1);
		rw_bf_t dvfs_train_en                        :(1 - 1 + 1);
		rw_bf_t reserved_2_3                            :(3 - 2 + 1);
		rw_bf_t dvfs_gate_lvl_long                   :(4 - 4 + 1);
		rw_bf_t dvfs_read_lvl_long                   :(5 - 5 + 1);
		rw_bf_t dvfs_write_lvl_long                  :(6 - 6 + 1);
		rw_bf_t reserved_7_31                           :(31 - 7 + 1);
	} bitfield;
} drex_ge_hw_ptrain_control_s0_t;          // 0x0224

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t periodic_gate_train                  :(3 - 0 + 1);
		rw_bf_t periodic_read_train                  :(7 - 4 + 1);
		rw_bf_t periodic_write_train                 :(11 - 8 + 1);
		rw_bf_t reserved_12_15                          :(15 - 12 + 1);
		rw_bf_t dvfs_gate_train                      :(19 - 16 + 1);
		rw_bf_t dvfs_read_train                      :(23 - 20 + 1);
		rw_bf_t dvfs_write_train                     :(27 - 24 + 1);
		rw_bf_t reserved_28_31                          :(31 - 28 + 1);
	} bitfield;
} drex_ge_ptrain_config_s1_t;              // 0x0230

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t ptrain_en                            :(0 - 0 + 1);
		rw_bf_t dvfs_train_en                        :(1 - 1 + 1);
		rw_bf_t reserved_2_3                            :(3 - 2 + 1);
		rw_bf_t dvfs_gate_lvl_long                   :(4 - 4 + 1);
		rw_bf_t dvfs_read_lvl_long                   :(5 - 5 + 1);
		rw_bf_t dvfs_write_lvl_long                  :(6 - 6 + 1);
		rw_bf_t reserved_7_31                           :(31 - 7 + 1);
	} bitfield;
} drex_ge_hw_ptrain_control_s1_t;          // 0x0234

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t wrtrn_wrdata_bl1                     :(7 - 0 + 1);
		rw_bf_t wrtrn_wrdata_bl2                     :(15 - 8 + 1);
		rw_bf_t wrtrn_wrdata_bl3                     :(23 - 16 + 1);
		rw_bf_t wrtrn_wrdata_bl4                     :(31 - 24 + 1);
	} bitfield;
} drex_ge_write_train_pattern0_t;          // 0x0240

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t wrtrn_wrdata_bl5                     :(7 - 0 + 1);
		rw_bf_t wrtrn_wrdata_bl6                     :(15 - 8 + 1);
		rw_bf_t wrtrn_wrdata_bl7                     :(23 - 16 + 1);
		rw_bf_t wrtrn_wrdata_bl8                     :(31 - 24 + 1);
	} bitfield;
} drex_ge_write_train_pattern1_t;          // 0x0244

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t wrtrn_wrdata_bl9                     :(7 - 0 + 1);
		rw_bf_t wrtrn_wrdata_bl10                    :(15 - 8 + 1);
		rw_bf_t wrtrn_wrdata_bl11                    :(23 - 16 + 1);
		rw_bf_t wrtrn_wrdata_bl12                    :(31 - 24 + 1);
	} bitfield;
} drex_ge_write_train_pattern2_t;          // 0x0248

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t wrtrn_wrdata_bl13                    :(7 - 0 + 1);
		rw_bf_t wrtrn_wrdata_bl14                    :(15 - 8 + 1);
		rw_bf_t wrtrn_wrdata_bl15                    :(23 - 16 + 1);
		rw_bf_t wrtrn_wrdata_bl16                    :(31 - 24 + 1);
	} bitfield;
} drex_ge_write_train_pattern3_t;          // 0x024C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t wrtrn_wrdata_dm_bl1                  :(0 - 0 + 1);
		rw_bf_t wrtrn_wrdata_dm_bl2                  :(1 - 1 + 1);
		rw_bf_t wrtrn_wrdata_dm_bl3                  :(2 - 2 + 1);
		rw_bf_t wrtrn_wrdata_dm_bl4                  :(3 - 3 + 1);
		rw_bf_t wrtrn_wrdata_dm_bl5                  :(4 - 4 + 1);
		rw_bf_t wrtrn_wrdata_dm_bl6                  :(5 - 5 + 1);
		rw_bf_t wrtrn_wrdata_dm_bl7                  :(6 - 6 + 1);
		rw_bf_t wrtrn_wrdata_dm_bl8                  :(7 - 7 + 1);
		rw_bf_t wrtrn_wrdata_dm_bl9                  :(8 - 8 + 1);
		rw_bf_t wrtrn_wrdata_dm_bl10                 :(9 - 9 + 1);
		rw_bf_t wrtrn_wrdata_dm_bl11                 :(10 - 10 + 1);
		rw_bf_t wrtrn_wrdata_dm_bl12                 :(11 - 11 + 1);
		rw_bf_t wrtrn_wrdata_dm_bl13                 :(12 - 12 + 1);
		rw_bf_t wrtrn_wrdata_dm_bl14                 :(13 - 13 + 1);
		rw_bf_t wrtrn_wrdata_dm_bl15                 :(14 - 14 + 1);
		rw_bf_t wrtrn_wrdata_dm_bl16                 :(15 - 15 + 1);
		rw_bf_t reserved_16_31                          :(31 - 16 + 1);
	} bitfield;
} drex_ge_write_train_pattern_dm_t;        // 0x0250

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t deassert_cke                         :(0 - 0 + 1);
		rw_bf_t reserved_1_3                            :(3 - 1 + 1);
		rw_bf_t t_adr                                :(8 - 4 + 1);
		rw_bf_t reserved_9_11                           :(11 - 9 + 1);
		rw_bf_t dfi_address_p0                       :(31 - 12 + 1);
	} bitfield;
} drex_ge_ca_calibration_config_t;         // 0x02A0

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t cacal_csn                            :(0 - 0 + 1);
		rw_bf_t reserved_1_31                           :(31 - 1 + 1);
	} bitfield;
} drex_ge_ca_calibration_control_t;        // 0x02A4

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t cacal_fsm                            :(4 - 0 + 1);
		rw_bf_t reserved_5_31                           :(31 - 5 + 1);
	} bitfield;
} drex_ge_ca_calibration_status_t;         // 0x02A8

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t cfg_dram_reset_n                     :(0 - 0 + 1);
		rw_bf_t reserved_1_31                           :(31 - 1 + 1);
	} bitfield;
} drex_ge_dram_reset_control_t;            // 0x02B0

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t qos_timer                            :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t brb_timer_th                         :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_qos_timer_0_t;                   // 0x0300

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t qos_timer                            :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t brb_timer_th                         :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_qos_timer_1_t;                   // 0x0304

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t qos_timer                            :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t brb_timer_th                         :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_qos_timer_2_t;                   // 0x0308

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t qos_timer                            :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t brb_timer_th                         :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_qos_timer_3_t;                   // 0x030C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t qos_timer                            :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t brb_timer_th                         :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_qos_timer_4_t;                   // 0x0310

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t qos_timer                            :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t brb_timer_th                         :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_qos_timer_5_t;                   // 0x0314

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t qos_timer                            :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t brb_timer_th                         :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_qos_timer_6_t;                   // 0x0318

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t qos_timer                            :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t brb_timer_th                         :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_qos_timer_7_t;                   // 0x031C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t qos_timer                            :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t brb_timer_th                         :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_qos_timer_8_t;                   // 0x0320

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t qos_timer                            :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t brb_timer_th                         :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_qos_timer_9_t;                   // 0x0324

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t qos_timer                            :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t brb_timer_th                         :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_qos_timer_10_t;                  // 0x0328

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t qos_timer                            :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t brb_timer_th                         :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_qos_timer_11_t;                  // 0x032C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t qos_timer                            :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t brb_timer_th                         :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_qos_timer_12_t;                  // 0x0330

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t qos_timer                            :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t brb_timer_th                         :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_qos_timer_13_t;                  // 0x0334

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t qos_timer                            :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t brb_timer_th                         :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_qos_timer_14_t;                  // 0x0338

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t qos_timer                            :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t brb_timer_th                         :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_qos_timer_15_t;                  // 0x033C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t vc_timer_th_even                     :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t vc_timer_th_odd                      :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_vp_timer_th_0_t;                 // 0x0340

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t vc_timer_th_even                     :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t vc_timer_th_odd                      :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_vp_timer_th_1_t;                 // 0x0344

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t vc_timer_th_even                     :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t vc_timer_th_odd                      :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_vp_timer_th_2_t;                 // 0x0348

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t vc_timer_th_even                     :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t vc_timer_th_odd                      :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_vp_timer_th_3_t;                 // 0x034C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t vc_timer_th_even                     :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t vc_timer_th_odd                      :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_vp_timer_th_4_t;                 // 0x0350

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t vc_timer_th_even                     :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t vc_timer_th_odd                      :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_vp_timer_th_5_t;                 // 0x0354

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t vc_timer_th_even                     :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t vc_timer_th_odd                      :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_vp_timer_th_6_t;                 // 0x0358

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t vc_timer_th_even                     :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t vc_timer_th_odd                      :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_ge_vp_timer_th_7_t;                 // 0x035C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t brb_cutoff_en                        :(7 - 0 + 1);
		rw_bf_t reserved_8_31                           :(31 - 8 + 1);
	} bitfield;
} drex_ge_cutoff_control_t;                // 0x0370

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t brb_cutoff_th_p0                     :(7 - 0 + 1);
		rw_bf_t brb_cutoff_th_p1                     :(15 - 8 + 1);
		rw_bf_t brb_cutoff_th_p2                     :(23 - 16 + 1);
		rw_bf_t brb_cutoff_th_p3                     :(31 - 24 + 1);
	} bitfield;
} drex_ge_brb_cutoff_config0_t;            // 0x0374

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t brb_cutoff_th_p4                     :(7 - 0 + 1);
		rw_bf_t brb_cutoff_th_p5                     :(15 - 8 + 1);
		rw_bf_t brb_cutoff_th_p6                     :(23 - 16 + 1);
		rw_bf_t brb_cutoff_th_p7                     :(31 - 24 + 1);
	} bitfield;
} drex_ge_brb_cutoff_config1_t;            // 0x0378

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t emergent_r_priority                  :(31 - 0 + 1);
	} bitfield;
} drex_ge_emergent_config_r_t;             // 0x0384

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t emergent_w_priority                  :(31 - 0 + 1);
	} bitfield;
} drex_ge_emergent_config_w_t;             // 0x0388

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t disable_vc_emergent                  :(3 - 0 + 1);
		rw_bf_t reserved_4_31                           :(31 - 4 + 1);
	} bitfield;
} drex_ge_vc_emergent_config_t;            // 0x038C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t bp_en                                :(0 - 0 + 1);
		rw_bf_t bp_under_emgerent                    :(1 - 1 + 1);
		rw_bf_t reserved_2_15                           :(15 - 2 + 1);
		rw_bf_t bp_on_th_trb                         :(23 - 16 + 1);
		rw_bf_t bp_off_th_trb                        :(31 - 24 + 1);
	} bitfield;
} drex_ge_bp_control_p0_t;                 // 0x0390

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t bp_on_th_rdbuf                       :(7 - 0 + 1);
		rw_bf_t bp_off_th_rdbuf                      :(15 - 8 + 1);
		rw_bf_t bp_on_th_wdbuf                       :(23 - 16 + 1);
		rw_bf_t bp_off_th_wdbuf                      :(31 - 24 + 1);
	} bitfield;
} drex_ge_bp_config_db_p0_t;               // 0x0394

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t bp_on_th_vcrb                        :(7 - 0 + 1);
		rw_bf_t bp_off_th_vcrb                       :(15 - 8 + 1);
		rw_bf_t bp_on_th_brb                         :(23 - 16 + 1);
		rw_bf_t bp_off_th_brb                        :(31 - 24 + 1);
	} bitfield;
} drex_ge_bp_config_rb_p0_t;               // 0x03D0

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t major                                :(3 - 0 + 1);
		rw_bf_t architectural                        :(7 - 4 + 1);
		rw_bf_t minor                                :(11 - 8 + 1);
		rw_bf_t revision                             :(15 - 12 + 1);
		rw_bf_t patch                                :(19 - 16 + 1);
		rw_bf_t pre_release                          :(20 - 20 + 1);
		rw_bf_t reserved_21_31                          :(31 - 21 + 1);
	} bitfield;
} drex_ge_version_t;                       // 0x0400

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t axi_data_bw                          :(7 - 0 + 1);
		rw_bf_t port_depth                           :(11 - 8 + 1);
		rw_bf_t max_rank_depth                       :(15 - 12 + 1);
		rw_bf_t bresp_fifo_depth                     :(19 - 16 + 1);
		rw_bf_t portfront_apb_integrated             :(20 - 20 + 1);
		rw_bf_t secure_apb_integrated                :(21 - 21 + 1);
		rw_bf_t ppc_integrated                       :(22 - 22 + 1);
		rw_bf_t reserved_23_23                          :(23 - 23 + 1);
		rw_bf_t pclk_async                           :(24 - 24 + 1);
		rw_bf_t reserved_25_25                          :(25 - 25 + 1);
		rw_bf_t dfi_cmd_pipe_depth                   :(27 - 26 + 1);
		rw_bf_t dfi_rd_pipe_depth                    :(29 - 28 + 1);
		rw_bf_t dfi_wr_pipe_depth                    :(31 - 30 + 1);
	} bitfield;
} drex_ge_instance_config0_t;              // 0x0404

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t brb_entry_depth                      :(7 - 0 + 1);
		rw_bf_t vc_reqbuf_depth                      :(15 - 8 + 1);
		rw_bf_t rdbuf_depth                          :(22 - 16 + 1);
		rw_bf_t wdbuf_depth                          :(31 - 23 + 1);
	} bitfield;
} drex_ge_instance_config1_t;              // 0x0408

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t vcf_clk_en                           :(0 - 0 + 1);
		rw_bf_t vcf_wr_clk_en                        :(1 - 1 + 1);
		rw_bf_t vcf_rd_clk_en                        :(2 - 2 + 1);
		rw_bf_t apb_clk_en                           :(3 - 3 + 1);
		rw_bf_t secure_apb_clk_en                    :(4 - 4 + 1);
		rw_bf_t reserved_5_7                            :(7 - 5 + 1);
		rw_bf_t sch_clk_en                           :(8 - 8 + 1);
		rw_bf_t sch_slice0_clk_en                    :(9 - 9 + 1);
		rw_bf_t sch_slice1_clk_en                    :(10 - 10 + 1);
		rw_bf_t reserved_11_15                          :(15 - 11 + 1);
		rw_bf_t memif_clk_en                         :(16 - 16 + 1);
		rw_bf_t memif_wr_clk_en                      :(17 - 17 + 1);
		rw_bf_t memif_rd_clk_en                      :(18 - 18 + 1);
		rw_bf_t reserved_19_31                          :(31 - 19 + 1);
	} bitfield;
} drex_ge_cg_status_t;                     // 0x040C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t empty_all                            :(0 - 0 + 1);
		rw_bf_t vc_reqbuf_empty                      :(1 - 1 + 1);
		rw_bf_t brb_empty_s0                         :(2 - 2 + 1);
		rw_bf_t brb_emtpy_s1                         :(3 - 3 + 1);
		rw_bf_t rdbuf_empty                          :(4 - 4 + 1);
		rw_bf_t wdbuf_empty                          :(5 - 5 + 1);
		rw_bf_t bresp_fifo_empty                     :(6 - 6 + 1);
		rw_bf_t reserved_7_31                           :(31 - 7 + 1);
	} bitfield;
} drex_ge_empty_status_t;                  // 0x0410

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t wf_state                             :(3 - 0 + 1);
		rw_bf_t reserved_4_31                           :(31 - 4 + 1);
	} bitfield;
} drex_ge_wf_status_t;                     // 0x0414

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t phy_upd_req                          :(0 - 0 + 1);
		rw_bf_t phy_upd_ack                          :(1 - 1 + 1);
		rw_bf_t reserved_2_31                           :(31 - 2 + 1);
	} bitfield;
} drex_ge_phy_update_status_t;             // 0x0418

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t all_init_done                        :(0 - 0 + 1);
		rw_bf_t reserved_1_31                           :(31 - 1 + 1);
	} bitfield;
} drex_ge_all_init_indication_t;           // 0x0420

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t rank_busy_state                      :(3 - 0 + 1);
		rw_bf_t rank_pd_state                        :(7 - 4 + 1);
		rw_bf_t reserved_8_11                           :(11 - 8 + 1);
		rw_bf_t rank_sref_state                      :(15 - 12 + 1);
		rw_bf_t cke_low_state                        :(19 - 16 + 1);
		rw_bf_t reserved_20_31                          :(31 - 20 + 1);
	} bitfield;
} drex_ge_rank_status_t;                   // 0x0424

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t csysreq                              :(0 - 0 + 1);
		rw_bf_t csysack                              :(1 - 1 + 1);
		rw_bf_t cwarmreset                           :(2 - 2 + 1);
		rw_bf_t reserved_3_3                            :(3 - 3 + 1);
		rw_bf_t lpi_state                            :(8 - 4 + 1);
		rw_bf_t axlp_agent_state                     :(13 - 9 + 1);
		rw_bf_t reserved_14_15                          :(15 - 14 + 1);
		rw_bf_t pause_req                            :(16 - 16 + 1);
		rw_bf_t pause_ack                            :(17 - 17 + 1);
		rw_bf_t reserved_18_19                          :(19 - 18 + 1);
		rw_bf_t pause_state                          :(24 - 20 + 1);
		rw_bf_t reserved_25_30                          :(30 - 25 + 1);
		rw_bf_t dfi_init_start                       :(31 - 31 + 1);
	} bitfield;
} drex_ge_lpi_pause_status_t;              // 0x0428

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t dfi_rdlvl_gate_en                    :(0 - 0 + 1);
		rw_bf_t dfi_rdlvl_en                         :(1 - 1 + 1);
		rw_bf_t dfi_rdlvl_wr_en                      :(2 - 2 + 1);
		rw_bf_t dfi_lvl_periodic                     :(3 - 3 + 1);
		rw_bf_t dfi_train_csn                        :(7 - 4 + 1);
		rw_bf_t train_state                          :(16 - 8 + 1);
		rw_bf_t reserved_17_31                          :(31 - 17 + 1);
	} bitfield;
} drex_ge_train_status_t;                  // 0x042C

typedef struct {
	drex_ge_package_config_t                     PACKAGE_CONFIG;                              // 0x0000
	drex_ge_dfi_init_start_t                     DFI_INIT_START;                              // 0x0004
	drex_ge_controller_control_t                 CONTROLLER_CONTROL;                          // 0x0008
	drex_ge_phy_control_t                        PHY_CONTROL;                                 // 0x000C
	drex_ge_memory_control0_t                    MEMORY_CONTROL0;                             // 0x0010
	drex_ge_refresh_control_t                    REFRESH_CONTROL;                             // 0x0014
	drex_ge_power_down_config_t                  POWER_DOWN_CONFIG;                           // 0x0018
	drex_ge_cg_control_t                         CG_CONTROL;                                  // 0x001C
	drex_ge_direct_command_t                     DIRECT_COMMAND;                              // 0x0020
	drex_ge_timing_set_sw_t                      TIMING_SET_SW;                               // 0x0024
	drex_ge_precharge_control_t                  PRECHARGE_CONTROL;                           // 0x0028
	drex_ge_precharge_config0_t                  PRECHARGE_CONFIG0;                           // 0x002C
	drex_ge_precharge_config1_t                  PRECHARGE_CONFIG1;                           // 0x0030
	drex_ge_write_flush_config0_t                WRITE_FLUSH_CONFIG0;                         // 0x0034
	drex_ge_write_flush_config1_t                WRITE_FLUSH_CONFIG1;                         // 0x0038
	drex_ge_info_config_t                        INFO_CONFIG;                                 // 0x003C
	drex_ge_timing_zq_t                          TIMING_ZQ;                                   // 0x0040
	drex_ge_timing_pzq_t                         TIMING_PZQ;                                  // 0x0044
	drex_ge_timing_etc0_t                        TIMING_ETC0;                                 // 0x0048
	ro_bf_t                                      reserved_0x004C;                             // 0x004C
	drex_ge_timing_ref_0_t                       TIMING_REF_0;                                // 0x0050
	drex_ge_timing_ref_1_t                       TIMING_REF_1;                                // 0x0054
	drex_ge_timing_ref_2_t                       TIMING_REF_2;                                // 0x0058
	drex_ge_timing_ref_3_t                       TIMING_REF_3;                                // 0x005C
	drex_ge_timing_ref_4_t                       TIMING_REF_4;                                // 0x0060
	drex_ge_timing_ref_5_t                       TIMING_REF_5;                                // 0x0064
	drex_ge_timing_ref_6_t                       TIMING_REF_6;                                // 0x0068
	drex_ge_timing_ref_7_t                       TIMING_REF_7;                                // 0x006C
	drex_ge_timing_ref_8_t                       TIMING_REF_8;                                // 0x0070
	drex_ge_timing_ref_9_t                       TIMING_REF_9;                                // 0x0074
	drex_ge_timing_ref_10_t                      TIMING_REF_10;                               // 0x0078
	drex_ge_timing_ref_11_t                      TIMING_REF_11;                               // 0x007C
	drex_ge_timing_ref_12_t                      TIMING_REF_12;                               // 0x0080
	drex_ge_timing_ref_13_t                      TIMING_REF_13;                               // 0x0084
	drex_ge_timing_ref_14_t                      TIMING_REF_14;                               // 0x0088
	drex_ge_timing_ref_15_t                      TIMING_REF_15;                               // 0x008C
	drex_ge_pause_mrs0_t                         PAUSE_MRS0;                                  // 0x0090
	drex_ge_pause_mrs1_t                         PAUSE_MRS1;                                  // 0x0094
	drex_ge_pause_mrs2_t                         PAUSE_MRS2;                                  // 0x0098
	drex_ge_pause_mrs3_t                         PAUSE_MRS3;                                  // 0x009C
	drex_ge_pause_mrs4_t                         PAUSE_MRS4;                                  // 0x00A0
	ro_bf_t                                      reserved_0x00A4_0x00CC[11];                  // 0x00A4 ~ 0x00CC
	drex_ge_zq_status_r0_t                       ZQ_STATUS_R0;                                // 0x00D0
	drex_ge_zq_status_r1_t                       ZQ_STATUS_R1;                                // 0x00D4
	ro_bf_t                                      reserved_0x00D8_0x00E8[5];                   // 0x00D8 ~ 0x00E8
	drex_ge_mr_status_t                          MR_STATUS;                                   // 0x00EC
	drex_ge_phy_status_t                         PHY_STATUS;                                  // 0x00F0
	drex_ge_memory_status_t                      MEMORY_STATUS;                               // 0x00F4
	drex_ge_temp_intr_test_t                     TEMP_INTR_TEST;                              // 0x00F8
	drex_ge_temp_intr_clear_t                    TEMP_INTR_CLEAR;                             // 0x00FC
	drex_ge_tick_granularity_s0_t                TICK_GRANULARITY_S0;                         // 0x0100
	drex_ge_memory_control1_s0_t                 MEMORY_CONTROL1_S0;                          // 0x0104
	drex_ge_temp_sensing_s0_t                    TEMP_SENSING_S0;                             // 0x0108
	ro_bf_t                                      reserved_0x010C_0x0114[3];                   // 0x010C ~ 0x0114
	drex_ge_winconfig_write_odt_s0_t             WINCONFIG_WRITE_ODT_S0;                      // 0x0118
	ro_bf_t                                      reserved_0x011C_0x0138[8];                   // 0x011C ~ 0x0138
	drex_ge_timing_row_lpddr4_s0_t               TIMING_ROW_LPDDR4_S0;                        // 0x013C
	drex_ge_timing_row0_s0_t                     TIMING_ROW0_S0;                              // 0x0140
	drex_ge_timing_row1_s0_t                     TIMING_ROW1_S0;                              // 0x0144
	drex_ge_timing_read_s0_t                     TIMING_READ_S0;                              // 0x0148
	drex_ge_timing_write_s0_t                    TIMING_WRITE_S0;                             // 0x014C
	drex_ge_timing_write_odt_s0_t                TIMING_WRITE_ODT_S0;                         // 0x0150
	drex_ge_timing_data_gap_s0_t                 TIMING_DATA_GAP_S0;                          // 0x0154
	drex_ge_timing_data_phy_s0_t                 TIMING_DATA_PHY_S0;                          // 0x0158
	drex_ge_timing_derate_s0_t                   TIMING_DERATE_S0;                            // 0x015C
	drex_ge_timing_power_s0_t                    TIMING_POWER_S0;                             // 0x0160
	drex_ge_timing_power_ddr_s0_t                TIMING_POWER_DDR_S0;                         // 0x0164
	drex_ge_timing_ddr4_s0_t                     TIMING_DDR4_S0;                              // 0x0168
	drex_ge_timing_etc1_s0_t                     TIMING_ETC1_S0;                              // 0x016C
	drex_ge_timing_train0_s0_t                   TIMING_TRAIN0_S0;                            // 0x0170
	drex_ge_timing_train1_s0_t                   TIMING_TRAIN1_S0;                            // 0x0174
	ro_bf_t                                      reserved_0x0178_0x017C[2];                   // 0x0178 ~ 0x017C
	drex_ge_tick_granularity_s1_t                TICK_GRANULARITY_S1;                         // 0x0180
	drex_ge_memory_control1_s1_t                 MEMORY_CONTROL1_S1;                          // 0x0184
	drex_ge_temp_sensing_s1_t                    TEMP_SENSING_S1;                             // 0x0188
	ro_bf_t                                      reserved_0x018C_0x0194[3];                   // 0x018C ~ 0x0194
	drex_ge_winconfig_write_odt_s1_t             WINCONFIG_WRITE_ODT_S1;                      // 0x0198
	ro_bf_t                                      reserved_0x019C_0x01B8[8];                   // 0x019C ~ 0x01B8
	drex_ge_timing_row_lpddr4_s1_t               TIMING_ROW_LPDDR4_S1;                        // 0x01BC
	drex_ge_timing_row0_s1_t                     TIMING_ROW0_S1;                              // 0x01C0
	drex_ge_timing_row1_s1_t                     TIMING_ROW1_S1;                              // 0x01C4
	drex_ge_timing_read_s1_t                     TIMING_READ_S1;                              // 0x01C8
	drex_ge_timing_write_s1_t                    TIMING_WRITE_S1;                             // 0x01CC
	drex_ge_timing_write_odt_s1_t                TIMING_WRITE_ODT_S1;                         // 0x01D0
	drex_ge_timing_data_gap_s1_t                 TIMING_DATA_GAP_S1;                          // 0x01D4
	drex_ge_timing_data_phy_s1_t                 TIMING_DATA_PHY_S1;                          // 0x01D8
	drex_ge_timing_derate_s1_t                   TIMING_DERATE_S1;                            // 0x01DC
	drex_ge_timing_power_s1_t                    TIMING_POWER_S1;                             // 0x01E0
	drex_ge_timing_power_ddr_s1_t                TIMING_POWER_DDR_S1;                         // 0x01E4
	drex_ge_timing_ddr4_s1_t                     TIMING_DDR4_S1;                              // 0x01E8
	drex_ge_timing_etc1_s1_t                     TIMING_ETC1_S1;                              // 0x01EC
	drex_ge_timing_train0_s1_t                   TIMING_TRAIN0_S1;                            // 0x01F0
	drex_ge_timing_train1_s1_t                   TIMING_TRAIN1_S1;                            // 0x01F4
	ro_bf_t                                      reserved_0x01F8_0x01FC[2];                   // 0x01F8 ~ 0x01FC
	drex_ge_init_train_config_t                  INIT_TRAIN_CONFIG;                           // 0x0200
	drex_ge_init_train_control_t                 INIT_TRAIN_CONTROL;                          // 0x0204
	drex_ge_sw_train_config_t                    SW_TRAIN_CONFIG;                             // 0x0208
	drex_ge_sw_train_control_t                   SW_TRAIN_CONTROL;                            // 0x020C
	ro_bf_t                                      reserved_0x0210_0x021C[4];                   // 0x0210 ~ 0x021C
	drex_ge_ptrain_config_s0_t                   PTRAIN_CONFIG_S0;                            // 0x0220
	drex_ge_hw_ptrain_control_s0_t               HW_PTRAIN_CONTROL_S0;                        // 0x0224
	ro_bf_t                                      reserved_0x0228_0x022C[2];                   // 0x0228 ~ 0x022C
	drex_ge_ptrain_config_s1_t                   PTRAIN_CONFIG_S1;                            // 0x0230
	drex_ge_hw_ptrain_control_s1_t               HW_PTRAIN_CONTROL_S1;                        // 0x0234
	ro_bf_t                                      reserved_0x0238_0x023C[2];                   // 0x0238 ~ 0x023C
	drex_ge_write_train_pattern0_t               WRITE_TRAIN_PATTERN0;                        // 0x0240
	drex_ge_write_train_pattern1_t               WRITE_TRAIN_PATTERN1;                        // 0x0244
	drex_ge_write_train_pattern2_t               WRITE_TRAIN_PATTERN2;                        // 0x0248
	drex_ge_write_train_pattern3_t               WRITE_TRAIN_PATTERN3;                        // 0x024C
	drex_ge_write_train_pattern_dm_t             WRITE_TRAIN_PATTERN_DM;                      // 0x0250
	ro_bf_t                                      reserved_0x0254_0x029C[19];                  // 0x0254 ~ 0x029C
	drex_ge_ca_calibration_config_t              CA_CALIBRATION_CONFIG;                       // 0x02A0
	drex_ge_ca_calibration_control_t             CA_CALIBRATION_CONTROL;                      // 0x02A4
	drex_ge_ca_calibration_status_t              CA_CALIBRATION_STATUS;                       // 0x02A8
	ro_bf_t                                      reserved_0x02AC;                             // 0x02AC
	drex_ge_dram_reset_control_t                 DRAM_RESET_CONTROL;                          // 0x02B0
	ro_bf_t                                      reserved_0x02B4_0x02FC[19];                  // 0x02B4 ~ 0x02FC
	drex_ge_qos_timer_0_t                        QOS_TIMER_0;                                 // 0x0300
	drex_ge_qos_timer_1_t                        QOS_TIMER_1;                                 // 0x0304
	drex_ge_qos_timer_2_t                        QOS_TIMER_2;                                 // 0x0308
	drex_ge_qos_timer_3_t                        QOS_TIMER_3;                                 // 0x030C
	drex_ge_qos_timer_4_t                        QOS_TIMER_4;                                 // 0x0310
	drex_ge_qos_timer_5_t                        QOS_TIMER_5;                                 // 0x0314
	drex_ge_qos_timer_6_t                        QOS_TIMER_6;                                 // 0x0318
	drex_ge_qos_timer_7_t                        QOS_TIMER_7;                                 // 0x031C
	drex_ge_qos_timer_8_t                        QOS_TIMER_8;                                 // 0x0320
	drex_ge_qos_timer_9_t                        QOS_TIMER_9;                                 // 0x0324
	drex_ge_qos_timer_10_t                       QOS_TIMER_10;                                // 0x0328
	drex_ge_qos_timer_11_t                       QOS_TIMER_11;                                // 0x032C
	drex_ge_qos_timer_12_t                       QOS_TIMER_12;                                // 0x0330
	drex_ge_qos_timer_13_t                       QOS_TIMER_13;                                // 0x0334
	drex_ge_qos_timer_14_t                       QOS_TIMER_14;                                // 0x0338
	drex_ge_qos_timer_15_t                       QOS_TIMER_15;                                // 0x033C
	drex_ge_vp_timer_th_0_t                      VP_TIMER_TH_0;                               // 0x0340
	drex_ge_vp_timer_th_1_t                      VP_TIMER_TH_1;                               // 0x0344
	drex_ge_vp_timer_th_2_t                      VP_TIMER_TH_2;                               // 0x0348
	drex_ge_vp_timer_th_3_t                      VP_TIMER_TH_3;                               // 0x034C
	drex_ge_vp_timer_th_4_t                      VP_TIMER_TH_4;                               // 0x0350
	drex_ge_vp_timer_th_5_t                      VP_TIMER_TH_5;                               // 0x0354
	drex_ge_vp_timer_th_6_t                      VP_TIMER_TH_6;                               // 0x0358
	drex_ge_vp_timer_th_7_t                      VP_TIMER_TH_7;                               // 0x035C
	ro_bf_t                                      reserved_0x0360_0x036C[4];                   // 0x0360 ~ 0x036C
	drex_ge_cutoff_control_t                     CUTOFF_CONTROL;                              // 0x0370
	drex_ge_brb_cutoff_config0_t                 BRB_CUTOFF_CONFIG0;                          // 0x0374
	drex_ge_brb_cutoff_config1_t                 BRB_CUTOFF_CONFIG1;                          // 0x0378
	ro_bf_t                                      reserved_0x037C_0x0380[2];                   // 0x037C ~ 0x0380
	drex_ge_emergent_config_r_t                  EMERGENT_CONFIG_R;                           // 0x0384
	drex_ge_emergent_config_w_t                  EMERGENT_CONFIG_W;                           // 0x0388
	drex_ge_vc_emergent_config_t                 VC_EMERGENT_CONFIG;                          // 0x038C
	drex_ge_bp_control_p0_t                      BP_CONTROL_P0;                               // 0x0390
	drex_ge_bp_config_db_p0_t                    BP_CONFIG_DB_P0;                             // 0x0394
	ro_bf_t                                      reserved_0x0398_0x03CC[14];                  // 0x0398 ~ 0x03CC
	drex_ge_bp_config_rb_p0_t                    BP_CONFIG_RB_P0;                             // 0x03D0
	ro_bf_t                                      reserved_0x03D4_0x03FC[11];                  // 0x03D4 ~ 0x03FC
	drex_ge_version_t                            VERSION;                                     // 0x0400
	drex_ge_instance_config0_t                   INSTANCE_CONFIG0;                            // 0x0404
	drex_ge_instance_config1_t                   INSTANCE_CONFIG1;                            // 0x0408
	drex_ge_cg_status_t                          CG_STATUS;                                   // 0x040C
	drex_ge_empty_status_t                       EMPTY_STATUS;                                // 0x0410
	drex_ge_wf_status_t                          WF_STATUS;                                   // 0x0414
	drex_ge_phy_update_status_t                  PHY_UPDATE_STATUS;                           // 0x0418
	ro_bf_t                                      reserved_0x041C;                             // 0x041C
	drex_ge_all_init_indication_t                ALL_INIT_INDICATION;                         // 0x0420
	drex_ge_rank_status_t                        RANK_STATUS;                                 // 0x0424
	drex_ge_lpi_pause_status_t                   LPI_PAUSE_STATUS;                            // 0x0428
	drex_ge_train_status_t                       TRAIN_STATUS;                                // 0x042C
} drex_ge_regs_t;

/* drex pf registers */

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t lru_update_th                        :(3 - 0 + 1);
		rw_bf_t port_frontend_cg_en                  :(4 - 4 + 1);
		rw_bf_t pclk_pf_cg_en                        :(5 - 5 + 1);
		rw_bf_t pf_apb_aclk_cg_en                    :(6 - 6 + 1);
		rw_bf_t reserved_7_8                            :(8 - 7 + 1);
		rw_bf_t info_axmarker                        :(9 - 9 + 1);
		rw_bf_t reserved_10_11                          :(11 - 10 + 1);
		rw_bf_t allowed_cont_ar_cnt                  :(15 - 12 + 1);
		rw_bf_t allowed_cont_aw_cnt                  :(19 - 16 + 1);
		rw_bf_t reserved_20_31                          :(31 - 20 + 1);
	} bitfield;
} drex_pf_pf_control_t;                    // 0x0000

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t all_init_done                        :(0 - 0 + 1);
		rw_bf_t reserved_1_31                           :(31 - 1 + 1);
	} bitfield;
} drex_pf_pf_all_init_indication_t;        // 0x0004

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t tick_granularity                     :(4 - 0 + 1);
		rw_bf_t reserved_5_31                           :(31 - 5 + 1);
	} bitfield;
} drex_pf_pf_tick_granularity_s0_t;        // 0x0010

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t tick_granularity                     :(4 - 0 + 1);
		rw_bf_t reserved_5_31                           :(31 - 5 + 1);
	} bitfield;
} drex_pf_pf_tick_granularity_s1_t;        // 0x0014

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_token_cutoff_en_p0                :(0 - 0 + 1);
		rw_bf_t pf_token_cutoff_en_p1                :(1 - 1 + 1);
		rw_bf_t pf_token_cutoff_en_p2                :(2 - 2 + 1);
		rw_bf_t pf_token_cutoff_en_p3                :(3 - 3 + 1);
		rw_bf_t reserved_4_31                           :(31 - 4 + 1);
	} bitfield;
} drex_pf_pf_token_control_t;              // 0x0020

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_token_threshold_p0                :(7 - 0 + 1);
		rw_bf_t pf_token_threshold_p1                :(15 - 8 + 1);
		rw_bf_t pf_token_threshold_p2                :(23 - 16 + 1);
		rw_bf_t pf_token_threshold_p3                :(31 - 24 + 1);
	} bitfield;
} drex_pf_pf_token_threshold0_t;           // 0x0024

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_token_threshold_p4                :(7 - 0 + 1);
		rw_bf_t pf_token_threshold_p5                :(15 - 8 + 1);
		rw_bf_t pf_token_threshold_p6                :(23 - 16 + 1);
		rw_bf_t pf_token_threshold_p7                :(31 - 24 + 1);
	} bitfield;
} drex_pf_pf_token_threshold1_t;           // 0x0028

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_rreq_throttle_en                  :(7 - 0 + 1);
		rw_bf_t monitoring_window                    :(15 - 8 + 1);
		rw_bf_t reserved_16_31                          :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_rreq_throttle_control_t;      // 0x002C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t blocked_cycle_th1                    :(7 - 0 + 1);
		rw_bf_t blocked_cycle_th2                    :(15 - 8 + 1);
		rw_bf_t blocked_cycle_th3                    :(23 - 16 + 1);
		rw_bf_t reserved_24_31                          :(31 - 24 + 1);
	} bitfield;
} drex_pf_pf_rreq_throttle_region_p0_t;      // 0x0030

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t allowed_mo_for_region_0              :(7 - 0 + 1);
		rw_bf_t allowed_mo_for_region_1              :(15 - 8 + 1);
		rw_bf_t allowed_mo_for_region_2              :(23 - 16 + 1);
		rw_bf_t allowed_mo_for_region_3              :(31 - 24 + 1);
	} bitfield;
} drex_pf_pf_rreq_throttle_mo_p0_t;        // 0x0034

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t blocked_cycle_th1                    :(7 - 0 + 1);
		rw_bf_t blocked_cycle_th2                    :(15 - 8 + 1);
		rw_bf_t blocked_cycle_th3                    :(23 - 16 + 1);
		rw_bf_t reserved_24_31                          :(31 - 24 + 1);
	} bitfield;
} drex_pf_pf_rreq_throttle_region_p1_t;      // 0x0038

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t allowed_mo_for_region_0              :(7 - 0 + 1);
		rw_bf_t allowed_mo_for_region_1              :(15 - 8 + 1);
		rw_bf_t allowed_mo_for_region_2              :(23 - 16 + 1);
		rw_bf_t allowed_mo_for_region_3              :(31 - 24 + 1);
	} bitfield;
} drex_pf_pf_rreq_throttle_mo_p1_t;        // 0x003C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_qos_timer_even                    :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t pf_qos_timer_odd                     :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_pf_pf_qos_timer_0_t;                // 0x0070

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_qos_timer_even                    :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t pf_qos_timer_odd                     :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_pf_pf_qos_timer_1_t;                // 0x0074

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_qos_timer_even                    :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t pf_qos_timer_odd                     :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_pf_pf_qos_timer_2_t;                // 0x0078

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_qos_timer_even                    :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t pf_qos_timer_odd                     :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_pf_pf_qos_timer_3_t;                // 0x007C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_qos_timer_even                    :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t pf_qos_timer_odd                     :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_pf_pf_qos_timer_4_t;                // 0x0080

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_qos_timer_even                    :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t pf_qos_timer_odd                     :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_pf_pf_qos_timer_5_t;                // 0x0084

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_qos_timer_even                    :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t pf_qos_timer_odd                     :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_pf_pf_qos_timer_6_t;                // 0x0088

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_qos_timer_even                    :(8 - 0 + 1);
		rw_bf_t reserved_9_15                           :(15 - 9 + 1);
		rw_bf_t pf_qos_timer_odd                     :(24 - 16 + 1);
		rw_bf_t reserved_25_31                          :(31 - 25 + 1);
	} bitfield;
} drex_pf_pf_qos_timer_7_t;                // 0x008C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t port_depth                           :(3 - 0 + 1);
		rw_bf_t ax_fifo_depth                        :(7 - 4 + 1);
		rw_bf_t rdata_fifo_depth                     :(11 - 8 + 1);
		rw_bf_t wdata_fifo_depth                     :(15 - 12 + 1);
		rw_bf_t bresp_fifo_depth                     :(19 - 16 + 1);
		rw_bf_t reserved_20_31                          :(31 - 20 + 1);
	} bitfield;
} drex_pf_pf_instance_config0_t;           // 0x0100

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t csysreq                              :(0 - 0 + 1);
		rw_bf_t csysack                              :(1 - 1 + 1);
		rw_bf_t cwarmreset                           :(2 - 2 + 1);
		rw_bf_t reserved_3_3                            :(3 - 3 + 1);
		rw_bf_t lpi_state                            :(6 - 4 + 1);
		rw_bf_t reserved_7_15                           :(15 - 7 + 1);
		rw_bf_t pause_req                            :(16 - 16 + 1);
		rw_bf_t pause_ack                            :(17 - 17 + 1);
		rw_bf_t reserved_18_19                          :(19 - 18 + 1);
		rw_bf_t pause_state                          :(21 - 20 + 1);
		rw_bf_t reserved_22_31                          :(31 - 22 + 1);
	} bitfield;
} drex_pf_pf_lpi_pause_status_t;           // 0x0104

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_valid_p0                     :(1 - 0 + 1);
		rw_bf_t pf_fifo_valid_p1                     :(3 - 2 + 1);
		rw_bf_t pf_fifo_valid_p2                     :(5 - 4 + 1);
		rw_bf_t pf_fifo_valid_p3                     :(7 - 6 + 1);
		rw_bf_t pf_fifo_valid_p4                     :(9 - 8 + 1);
		rw_bf_t pf_fifo_valid_p5                     :(11 - 10 + 1);
		rw_bf_t pf_fifo_valid_p6                     :(13 - 12 + 1);
		rw_bf_t pf_fifo_valid_p7                     :(15 - 14 + 1);
		rw_bf_t reserved_16_31                          :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_skid_ar_fifo_valid_t;         // 0x0110

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_info_0                       :(15 - 0 + 1);
		rw_bf_t pf_fifo_info_1                       :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_skid_ar_fifo_info_p0_t;       // 0x0120

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_info_0                       :(15 - 0 + 1);
		rw_bf_t pf_fifo_info_1                       :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_skid_ar_fifo_info_p1_t;       // 0x0124

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_info_0                       :(15 - 0 + 1);
		rw_bf_t pf_fifo_info_1                       :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_skid_ar_fifo_info_p2_t;       // 0x0128

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_info_0                       :(15 - 0 + 1);
		rw_bf_t pf_fifo_info_1                       :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_skid_ar_fifo_info_p3_t;       // 0x012C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_info_0                       :(15 - 0 + 1);
		rw_bf_t pf_fifo_info_1                       :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_skid_ar_fifo_info_p4_t;       // 0x0130

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_info_0                       :(15 - 0 + 1);
		rw_bf_t pf_fifo_info_1                       :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_skid_ar_fifo_info_p5_t;       // 0x0134

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_info_0                       :(15 - 0 + 1);
		rw_bf_t pf_fifo_info_1                       :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_skid_ar_fifo_info_p6_t;       // 0x0138

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_info_0                       :(15 - 0 + 1);
		rw_bf_t pf_fifo_info_1                       :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_skid_ar_fifo_info_p7_t;       // 0x013C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_valid_p0                     :(1 - 0 + 1);
		rw_bf_t pf_fifo_valid_p1                     :(3 - 2 + 1);
		rw_bf_t pf_fifo_valid_p2                     :(5 - 4 + 1);
		rw_bf_t pf_fifo_valid_p3                     :(7 - 6 + 1);
		rw_bf_t pf_fifo_valid_p4                     :(9 - 8 + 1);
		rw_bf_t pf_fifo_valid_p5                     :(11 - 10 + 1);
		rw_bf_t pf_fifo_valid_p6                     :(13 - 12 + 1);
		rw_bf_t pf_fifo_valid_p7                     :(15 - 14 + 1);
		rw_bf_t reserved_16_31                          :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_skid_aw_fifo_valid_t;         // 0x0140

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_info_0                       :(15 - 0 + 1);
		rw_bf_t pf_fifo_info_1                       :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_skid_aw_fifo_info_p0_t;       // 0x0150

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_info_0                       :(15 - 0 + 1);
		rw_bf_t pf_fifo_info_1                       :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_skid_aw_fifo_info_p1_t;       // 0x0154

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_info_0                       :(15 - 0 + 1);
		rw_bf_t pf_fifo_info_1                       :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_skid_aw_fifo_info_p2_t;       // 0x0158

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_info_0                       :(15 - 0 + 1);
		rw_bf_t pf_fifo_info_1                       :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_skid_aw_fifo_info_p3_t;       // 0x015C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_info_0                       :(15 - 0 + 1);
		rw_bf_t pf_fifo_info_1                       :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_skid_aw_fifo_info_p4_t;       // 0x0160

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_info_0                       :(15 - 0 + 1);
		rw_bf_t pf_fifo_info_1                       :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_skid_aw_fifo_info_p5_t;       // 0x0164

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_info_0                       :(15 - 0 + 1);
		rw_bf_t pf_fifo_info_1                       :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_skid_aw_fifo_info_p6_t;       // 0x0168

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_info_0                       :(15 - 0 + 1);
		rw_bf_t pf_fifo_info_1                       :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_skid_aw_fifo_info_p7_t;       // 0x016C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_valid_p0                     :(1 - 0 + 1);
		rw_bf_t pf_fifo_valid_p1                     :(3 - 2 + 1);
		rw_bf_t pf_fifo_valid_p2                     :(5 - 4 + 1);
		rw_bf_t pf_fifo_valid_p3                     :(7 - 6 + 1);
		rw_bf_t pf_fifo_valid_p4                     :(9 - 8 + 1);
		rw_bf_t pf_fifo_valid_p5                     :(11 - 10 + 1);
		rw_bf_t pf_fifo_valid_p6                     :(13 - 12 + 1);
		rw_bf_t pf_fifo_valid_p7                     :(15 - 14 + 1);
		rw_bf_t reserved_16_31                          :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_fifo_valid_t;                 // 0x0170

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_write_p0                     :(1 - 0 + 1);
		rw_bf_t pf_fifo_write_p1                     :(3 - 2 + 1);
		rw_bf_t pf_fifo_write_p2                     :(5 - 4 + 1);
		rw_bf_t pf_fifo_write_p3                     :(7 - 6 + 1);
		rw_bf_t pf_fifo_write_p4                     :(9 - 8 + 1);
		rw_bf_t pf_fifo_write_p5                     :(11 - 10 + 1);
		rw_bf_t pf_fifo_write_p6                     :(13 - 12 + 1);
		rw_bf_t pf_fifo_write_p7                     :(15 - 14 + 1);
		rw_bf_t reserved_16_31                          :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_fifo_write_t;                 // 0x0174

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_info_p0_even                 :(15 - 0 + 1);
		rw_bf_t pf_fifo_info_p0_odd                  :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_fifo_info_p0_t;               // 0x0180

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_info_p1_even                 :(15 - 0 + 1);
		rw_bf_t pf_fifo_info_p1_odd                  :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_fifo_info_p1_t;               // 0x0184

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_info_p2_even                 :(15 - 0 + 1);
		rw_bf_t pf_fifo_info_p2_odd                  :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_fifo_info_p2_t;               // 0x0188

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_info_p3_even                 :(15 - 0 + 1);
		rw_bf_t pf_fifo_info_p3_odd                  :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_fifo_info_p3_t;               // 0x018C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_info_p4_even                 :(15 - 0 + 1);
		rw_bf_t pf_fifo_info_p4_odd                  :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_fifo_info_p4_t;               // 0x0190

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_info_p5_even                 :(15 - 0 + 1);
		rw_bf_t pf_fifo_info_p5_odd                  :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_fifo_info_p5_t;               // 0x0194

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_info_p6_even                 :(15 - 0 + 1);
		rw_bf_t pf_fifo_info_p6_odd                  :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_fifo_info_p6_t;               // 0x0198

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t pf_fifo_info_p7_even                 :(15 - 0 + 1);
		rw_bf_t pf_fifo_info_p7_odd                  :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_fifo_info_p7_t;               // 0x019C

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t arvalid                              :(7 - 0 + 1);
		rw_bf_t arready                              :(15 - 8 + 1);
		rw_bf_t awvalid                              :(23 - 16 + 1);
		rw_bf_t awready                              :(31 - 24 + 1);
	} bitfield;
} drex_pf_pf_channel_info_ax_t;            // 0x01A0

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t rvalid                               :(7 - 0 + 1);
		rw_bf_t rready                               :(15 - 8 + 1);
		rw_bf_t wvalid                               :(23 - 16 + 1);
		rw_bf_t wready                               :(31 - 24 + 1);
	} bitfield;
} drex_pf_pf_channel_info_rw_t;            // 0x01A4

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t bvalid                               :(7 - 0 + 1);
		rw_bf_t bready                               :(15 - 8 + 1);
		rw_bf_t reserved_16_31                          :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_channel_info_b_t;             // 0x01A8

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t token_pool                           :(7 - 0 + 1);
		rw_bf_t write_mo_count                       :(15 - 8 + 1);
		rw_bf_t reserved_16_31                          :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_token_info_t;                 // 0x01B0

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t monitoring_window_enabled            :(0 - 0 + 1);
		rw_bf_t reserved_1_7                            :(7 - 1 + 1);
		rw_bf_t blocked_read                         :(15 - 8 + 1);
		rw_bf_t mo_throttleing_region_p0             :(17 - 16 + 1);
		rw_bf_t mo_throttleing_region_p1             :(19 - 18 + 1);
		rw_bf_t mo_throttleing_region_p2             :(21 - 20 + 1);
		rw_bf_t mo_throttleing_region_p3             :(23 - 22 + 1);
		rw_bf_t mo_throttleing_region_p4             :(25 - 24 + 1);
		rw_bf_t mo_throttleing_region_p5             :(27 - 26 + 1);
		rw_bf_t mo_throttleing_region_p6             :(29 - 28 + 1);
		rw_bf_t mo_throttleing_region_p7             :(31 - 30 + 1);
	} bitfield;
} drex_pf_pf_rreq_throttle_info_t;         // 0x01C0

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t read_mo_count                        :(15 - 0 + 1);
		rw_bf_t rready_blocked_cycle                 :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_rreq_throttle_info_p0_t;      // 0x01D0

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t read_mo_count                        :(15 - 0 + 1);
		rw_bf_t rready_blocked_cycle                 :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_rreq_throttle_info_p1_t;      // 0x01D4

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t read_mo_count                        :(15 - 0 + 1);
		rw_bf_t rready_blocked_cycle                 :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_rreq_throttle_info_p2_t;      // 0x01D8

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t read_mo_count                        :(15 - 0 + 1);
		rw_bf_t rready_blocked_cycle                 :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_rreq_throttle_info_p3_t;      // 0x01DC

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t read_mo_count                        :(15 - 0 + 1);
		rw_bf_t rready_blocked_cycle                 :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_rreq_throttle_info_p4_t;      // 0x01E0

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t read_mo_count                        :(15 - 0 + 1);
		rw_bf_t rready_blocked_cycle                 :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_rreq_throttle_info_p5_t;      // 0x01E4

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t read_mo_count                        :(15 - 0 + 1);
		rw_bf_t rready_blocked_cycle                 :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_rreq_throttle_info_p6_t;      // 0x01E8

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t read_mo_count                        :(15 - 0 + 1);
		rw_bf_t rready_blocked_cycle                 :(31 - 16 + 1);
	} bitfield;
} drex_pf_pf_rreq_throttle_info_p7_t;      // 0x01EC

typedef struct {
	drex_pf_pf_control_t                         PF_CONTROL;                                  // 0x0000
	drex_pf_pf_all_init_indication_t             PF_ALL_INIT_INDICATION;                      // 0x0004
	ro_bf_t                                      reserved_0x0008_0x000C[2];                   // 0x0008 ~ 0x000C
	drex_pf_pf_tick_granularity_s0_t             PF_TICK_GRANULARITY_S0;                      // 0x0010
	drex_pf_pf_tick_granularity_s1_t             PF_TICK_GRANULARITY_S1;                      // 0x0014
	ro_bf_t                                      reserved_0x0018_0x001C[2];                   // 0x0018 ~ 0x001C
	drex_pf_pf_token_control_t                   PF_TOKEN_CONTROL;                            // 0x0020
	drex_pf_pf_token_threshold0_t                PF_TOKEN_THRESHOLD0;                         // 0x0024
	drex_pf_pf_token_threshold1_t                PF_TOKEN_THRESHOLD1;                         // 0x0028
	drex_pf_pf_rreq_throttle_control_t           PF_RREQ_THROTTLE_CONTROL;                    // 0x002C
	drex_pf_pf_rreq_throttle_region_p0_t         PF_RREQ_THROTTLE_REGION_P0;                  // 0x0030
	drex_pf_pf_rreq_throttle_mo_p0_t             PF_RREQ_THROTTLE_MO_P0;                      // 0x0034
	drex_pf_pf_rreq_throttle_region_p1_t         PF_RREQ_THROTTLE_REGION_P1;                  // 0x0038
	drex_pf_pf_rreq_throttle_mo_p1_t             PF_RREQ_THROTTLE_MO_P1;                      // 0x003C
	ro_bf_t                                      reserved_0x0040_0x006C[12];                  // 0x0040 ~ 0x006C
	drex_pf_pf_qos_timer_0_t                     PF_QOS_TIMER_0;                              // 0x0070
	drex_pf_pf_qos_timer_1_t                     PF_QOS_TIMER_1;                              // 0x0074
	drex_pf_pf_qos_timer_2_t                     PF_QOS_TIMER_2;                              // 0x0078
	drex_pf_pf_qos_timer_3_t                     PF_QOS_TIMER_3;                              // 0x007C
	drex_pf_pf_qos_timer_4_t                     PF_QOS_TIMER_4;                              // 0x0080
	drex_pf_pf_qos_timer_5_t                     PF_QOS_TIMER_5;                              // 0x0084
	drex_pf_pf_qos_timer_6_t                     PF_QOS_TIMER_6;                              // 0x0088
	drex_pf_pf_qos_timer_7_t                     PF_QOS_TIMER_7;                              // 0x008C
	ro_bf_t                                      reserved_0x0090_0x00FC[28];                  // 0x0090 ~ 0x00FC
	drex_pf_pf_instance_config0_t                PF_INSTANCE_CONFIG0;                         // 0x0100
	drex_pf_pf_lpi_pause_status_t                PF_LPI_PAUSE_STATUS;                         // 0x0104
	ro_bf_t                                      reserved_0x0108_0x010C[2];                   // 0x0108 ~ 0x010C
	drex_pf_pf_skid_ar_fifo_valid_t              PF_SKID_AR_FIFO_VALID;                       // 0x0110
	ro_bf_t                                      reserved_0x0114_0x011C[3];                   // 0x0114 ~ 0x011C
	drex_pf_pf_skid_ar_fifo_info_p0_t            PF_SKID_AR_FIFO_INFO_P0;                     // 0x0120
	drex_pf_pf_skid_ar_fifo_info_p1_t            PF_SKID_AR_FIFO_INFO_P1;                     // 0x0124
	drex_pf_pf_skid_ar_fifo_info_p2_t            PF_SKID_AR_FIFO_INFO_P2;                     // 0x0128
	drex_pf_pf_skid_ar_fifo_info_p3_t            PF_SKID_AR_FIFO_INFO_P3;                     // 0x012C
	drex_pf_pf_skid_ar_fifo_info_p4_t            PF_SKID_AR_FIFO_INFO_P4;                     // 0x0130
	drex_pf_pf_skid_ar_fifo_info_p5_t            PF_SKID_AR_FIFO_INFO_P5;                     // 0x0134
	drex_pf_pf_skid_ar_fifo_info_p6_t            PF_SKID_AR_FIFO_INFO_P6;                     // 0x0138
	drex_pf_pf_skid_ar_fifo_info_p7_t            PF_SKID_AR_FIFO_INFO_P7;                     // 0x013C
	drex_pf_pf_skid_aw_fifo_valid_t              PF_SKID_AW_FIFO_VALID;                       // 0x0140
	ro_bf_t                                      reserved_0x0144_0x014C[3];                   // 0x0144 ~ 0x014C
	drex_pf_pf_skid_aw_fifo_info_p0_t            PF_SKID_AW_FIFO_INFO_P0;                     // 0x0150
	drex_pf_pf_skid_aw_fifo_info_p1_t            PF_SKID_AW_FIFO_INFO_P1;                     // 0x0154
	drex_pf_pf_skid_aw_fifo_info_p2_t            PF_SKID_AW_FIFO_INFO_P2;                     // 0x0158
	drex_pf_pf_skid_aw_fifo_info_p3_t            PF_SKID_AW_FIFO_INFO_P3;                     // 0x015C
	drex_pf_pf_skid_aw_fifo_info_p4_t            PF_SKID_AW_FIFO_INFO_P4;                     // 0x0160
	drex_pf_pf_skid_aw_fifo_info_p5_t            PF_SKID_AW_FIFO_INFO_P5;                     // 0x0164
	drex_pf_pf_skid_aw_fifo_info_p6_t            PF_SKID_AW_FIFO_INFO_P6;                     // 0x0168
	drex_pf_pf_skid_aw_fifo_info_p7_t            PF_SKID_AW_FIFO_INFO_P7;                     // 0x016C
	drex_pf_pf_fifo_valid_t                      PF_FIFO_VALID;                               // 0x0170
	drex_pf_pf_fifo_write_t                      PF_FIFO_WRITE;                               // 0x0174
	ro_bf_t                                      reserved_0x0178_0x017C[2];                   // 0x0178 ~ 0x017C
	drex_pf_pf_fifo_info_p0_t                    PF_FIFO_INFO_P0;                             // 0x0180
	drex_pf_pf_fifo_info_p1_t                    PF_FIFO_INFO_P1;                             // 0x0184
	drex_pf_pf_fifo_info_p2_t                    PF_FIFO_INFO_P2;                             // 0x0188
	drex_pf_pf_fifo_info_p3_t                    PF_FIFO_INFO_P3;                             // 0x018C
	drex_pf_pf_fifo_info_p4_t                    PF_FIFO_INFO_P4;                             // 0x0190
	drex_pf_pf_fifo_info_p5_t                    PF_FIFO_INFO_P5;                             // 0x0194
	drex_pf_pf_fifo_info_p6_t                    PF_FIFO_INFO_P6;                             // 0x0198
	drex_pf_pf_fifo_info_p7_t                    PF_FIFO_INFO_P7;                             // 0x019C
	drex_pf_pf_channel_info_ax_t                 PF_CHANNEL_INFO_AX;                          // 0x01A0
	drex_pf_pf_channel_info_rw_t                 PF_CHANNEL_INFO_RW;                          // 0x01A4
	drex_pf_pf_channel_info_b_t                  PF_CHANNEL_INFO_B;                           // 0x01A8
	ro_bf_t                                      reserved_0x01AC;                             // 0x01AC
	drex_pf_pf_token_info_t                      PF_TOKEN_INFO;                               // 0x01B0
	ro_bf_t                                      reserved_0x01B4_0x01BC[3];                   // 0x01B4 ~ 0x01BC
	drex_pf_pf_rreq_throttle_info_t              PF_RREQ_THROTTLE_INFO;                       // 0x01C0
	ro_bf_t                                      reserved_0x01C4_0x01CC[3];                   // 0x01C4 ~ 0x01CC
	drex_pf_pf_rreq_throttle_info_p0_t           PF_RREQ_THROTTLE_INFO_P0;                    // 0x01D0
	drex_pf_pf_rreq_throttle_info_p1_t           PF_RREQ_THROTTLE_INFO_P1;                    // 0x01D4
	drex_pf_pf_rreq_throttle_info_p2_t           PF_RREQ_THROTTLE_INFO_P2;                    // 0x01D8
	drex_pf_pf_rreq_throttle_info_p3_t           PF_RREQ_THROTTLE_INFO_P3;                    // 0x01DC
	drex_pf_pf_rreq_throttle_info_p4_t           PF_RREQ_THROTTLE_INFO_P4;                    // 0x01E0
	drex_pf_pf_rreq_throttle_info_p5_t           PF_RREQ_THROTTLE_INFO_P5;                    // 0x01E4
	drex_pf_pf_rreq_throttle_info_p6_t           PF_RREQ_THROTTLE_INFO_P6;                    // 0x01E8
	drex_pf_pf_rreq_throttle_info_p7_t           PF_RREQ_THROTTLE_INFO_P7;                    // 0x01EC
} drex_pf_regs_t;

/* drex se registers */

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t chunk_end                            :(11 - 0 + 1);
		rw_bf_t reserved_12_15                          :(15 - 12 + 1);
		rw_bf_t chunk_start                          :(27 - 16 + 1);
		rw_bf_t reserved_28_31                          :(31 - 28 + 1);
	} bitfield;
} drex_se_chunk_config_c0_t;               // 0x0F00

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t chunk_end                            :(11 - 0 + 1);
		rw_bf_t reserved_12_15                          :(15 - 12 + 1);
		rw_bf_t chunk_start                          :(27 - 16 + 1);
		rw_bf_t reserved_28_30                          :(30 - 28 + 1);
		rw_bf_t chunk_en                             :(31 - 31 + 1);
	} bitfield;
} drex_se_chunk_config_c1_t;               // 0x0F04

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t reserved_0_3                            :(3 - 0 + 1);
		rw_bf_t row_bw                               :(7 - 4 + 1);
		rw_bf_t col_bw                               :(11 - 8 + 1);
		rw_bf_t reserved_12_15                          :(15 - 12 + 1);
		rw_bf_t bit_sel0                             :(16 - 16 + 1);
		rw_bf_t bit_sel1                             :(17 - 17 + 1);
		rw_bf_t bit_sel_en0                          :(18 - 18 + 1);
		rw_bf_t bit_sel_en1                          :(19 - 19 + 1);
		rw_bf_t bank_lsb                             :(22 - 20 + 1);
		rw_bf_t reserved_23_23                          :(23 - 23 + 1);
		rw_bf_t rank_inter_en                        :(24 - 24 + 1);
		rw_bf_t reserved_25_30                          :(30 - 25 + 1);
		rw_bf_t rank_en                              :(31 - 31 + 1);
	} bitfield;
} drex_se_rank_config_r0_t;                // 0x0F10

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t reserved_0_3                            :(3 - 0 + 1);
		rw_bf_t row_bw                               :(7 - 4 + 1);
		rw_bf_t col_bw                               :(11 - 8 + 1);
		rw_bf_t reserved_12_15                          :(15 - 12 + 1);
		rw_bf_t bit_sel0                             :(16 - 16 + 1);
		rw_bf_t bit_sel1                             :(17 - 17 + 1);
		rw_bf_t bit_sel_en0                          :(18 - 18 + 1);
		rw_bf_t bit_sel_en1                          :(19 - 19 + 1);
		rw_bf_t bank_lsb                             :(22 - 20 + 1);
		rw_bf_t reserved_23_23                          :(23 - 23 + 1);
		rw_bf_t rank_inter_en                        :(24 - 24 + 1);
		rw_bf_t reserved_25_30                          :(30 - 25 + 1);
		rw_bf_t rank_en                              :(31 - 31 + 1);
	} bitfield;
} drex_se_rank_config_r1_t;                // 0x0F14

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t rank_size                            :(8 - 0 + 1);
		rw_bf_t reserved_9_31                           :(31 - 9 + 1);
	} bitfield;
} drex_se_rank_size_config_r0_t;           // 0x0F20

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t rank_size                            :(8 - 0 + 1);
		rw_bf_t reserved_9_31                           :(31 - 9 + 1);
	} bitfield;
} drex_se_rank_size_config_r1_t;           // 0x0F24

typedef struct {
	ro_bf_t                                      reserved_0x0000_0x0EFC[960];                 // 0x0000 ~ 0x0EFC
	drex_se_chunk_config_c0_t                    CHUNK_CONFIG_C0;                             // 0x0F00
	drex_se_chunk_config_c1_t                    CHUNK_CONFIG_C1;                             // 0x0F04
	ro_bf_t                                      reserved_0x0F08_0x0F0C[2];                   // 0x0F08 ~ 0x0F0C
	drex_se_rank_config_r0_t                     RANK_CONFIG_R0;                              // 0x0F10
	drex_se_rank_config_r1_t                     RANK_CONFIG_R1;                              // 0x0F14
	ro_bf_t                                      reserved_0x0F18_0x0F1C[2];                   // 0x0F18 ~ 0x0F1C
	drex_se_rank_size_config_r0_t                RANK_SIZE_CONFIG_R0;                         // 0x0F20
	drex_se_rank_size_config_r1_t                RANK_SIZE_CONFIG_R1;                         // 0x0F24
} drex_se_regs_t;

/* sysreg core registers */

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t dmc_secure_ext_en              :(0 - 0 + 1);
		rw_bf_t dmc_secure_ext_in_progress     :(1 - 1 + 1);
		rw_bf_t reserved_2_31                           :(31 - 2 + 1);
	} bitfield;
} sysreg_core_core_asyncsfr_wr_con_t;      // 0x0408

typedef struct {
	sysreg_core_core_asyncsfr_wr_con_t           CORE_ASYNCSFR_WR_CON;                        // 0x0408
} sysreg_core_regs_t;

typedef union {
	volatile unsigned int data;
	struct {
		rw_bf_t mask_dram_resetpad			 :(0 - 0 + 1);
		rw_bf_t reserved_1_31                           :(31 - 1 + 1);
	} bitfield;
} pmu_alive_reset_sequencer_option_t;      // 0x0408

typedef struct {
	pmu_alive_reset_sequencer_option_t           RESET_SEQUENCER_OPTION_SFR;                        // 0x0508
} pmu_alive_regs_t;

#endif

#endif
