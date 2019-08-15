/*
 * (C) Copyright 2019 SAMSUNG Electronics
 * Author: sunghyun.na@samsung.com (Sung-Hyun Na)
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted
 * transcribed, stored in a retrieval system or translated into any human or computer language in an
 * form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 *
 */

#ifndef INCLUDE_USB_CONFIG_H_
#define INCLUDE_USB_CONFIG_H_

struct dwc3_global_config {
	// Global Link Configuration
	unsigned int m_eBurstLength;
	unsigned int m_b1KPageBD;
	unsigned int m_ucOutstandingDepth;
	unsigned m_bDisableAutoClkGating :1;
	unsigned m_bIoUsed :1;
	/* System configuration */
	unsigned cache_coherency_enable :1;
	unsigned additional_refclk :1;
	unsigned refclk_lpm :1;
	/* Host Option */
	unsigned sparse_en :1;
	unsigned host_loa_filter :1;
	/* Device Option */
	unsigned dev_hw_l1_wakeup :1;
	unsigned dev_l1_evt_split :1;
	/* Global PHY Control Selection */
	/* SS PHY Type */
	unsigned m_bEnableU3Ctrl :1;
	unsigned m_bEnableRxDetectP3 :1;
	unsigned m_bP3WhenU2 :1;
	unsigned m_bP3P2DirectTransition :1;
	unsigned bForceU3Ctrl :1;
	unsigned u1u2exitfail_to_recov:1;
	unsigned p3exsigp2:1;
	unsigned UxExitInPx:1;
	unsigned ElasticBufferMode:1;

	unsigned int ss_phy_type;
	/* HS PHY */
	unsigned m_bEnableL2SusCtrl :1;
	unsigned m_bUTMIIFSel :1;
	/* WA option */
	unsigned ss_gen1_phy_ux_exit_wa :1;
	/* Debug and Test Feature */
	unsigned int enable_logic_trace_thread;
	unsigned scale_down_time :1;
};

struct dwc3_dev_config {
	/* Speed Limitation */
	char speed[16];
	/* U1/U2 Exit Time Value */
	unsigned char m_ucU1ExitValue;
	unsigned short m_usU2ExitValue;
	/* Buffer Depth */
	unsigned int m_uEventBufDepth;
	unsigned int m_uCtrlBufSize;
	/* Interrupt used */
	unsigned char used_intr_num;
	/* Device Feature */
	unsigned m_bRemoteWake:1;
	unsigned m_bU1Support:1;
	unsigned m_bU2Support:1;
	unsigned m_bLTMSupport:1;
	unsigned m_bContainID:1;
	unsigned m_bBESLSupport:1;
	/* TRB Start Feature */
	unsigned on_demand:1;
};

struct dwc3_plat_config {
	void *base;
	unsigned int ref_clk;
	unsigned int suspend_clk;

	/* Multiple interrupt */
	int num_intr;
	unsigned int *array_intr;

	int num_hs_phy;

	int num_ss_phy;
	char ssphy_type[32];

	struct dwc3_global_config config;
};

#endif /* INCLUDE_USB_CONFIG_H_ */
