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

#ifndef DRIVER_USB_USB30_DRD_USB30_REG_H_
#define DRIVER_USB_USB30_DRD_USB30_REG_H_

#include "sys/types.h"

// Global Registers (Gxxxx)
//-----------------------------------------
// Global Common Registers
#define	rGSBUSCFG0		(0xc100)
#define	rGSBUSCFG1		(0xc104)
#define	rGTXTHRCFG		(0xc108)
#define	rGRXTHRCFG		(0xc10c)

#define	rGCTL			(0xc110)
#define	rGPMSTS			(0xc114)
#define	rGSTS			(0xc118)
#define	rGUCTL1			(0xc11c)

#define	rGSNPSID		(0xc120)
#define	rGGPIO			(0xc124)
#define	rGUID			(0xc128)
#define	rGUCTL			(0xc12c)

#define	rGBUSERRADDR_LO		(0xc130)
#define	rGBUSERRADDR_HI		(0xc134)

//-----------------------------------------
// Global Port to USB Instance Mapping Registers
#define	rGPRTBIMAP_LO		(0xc138)
#define	rGPRTBIMAP_HI		(0xc13c)
#define	rGDBGLSPMUX_DEV		(0xc170)
#define	rGPRTBIMAP_HS_LO	(0xc180)
#define	rGPRTBIMAP_HS_HI	(0xc184)
#define	rGPRTBIMAP_FS_LO	(0xc188)
#define	rGPRTBIMAP_FS_HI	(0xc18c)

//-----------------------------------------
// Global Hardware Parameter Registers
#define	rGHWPARAMS0		(0xc140)
#define	rGHWPARAMS1		(0xc144)
#define	rGHWPARAMS2		(0xc148)
#define	rGHWPARAMS3		(0xc14c)
#define	rGHWPARAMS4		(0xc150)
#define	rGHWPARAMS5		(0xc154)
#define	rGHWPARAMS6		(0xc158)
#define	rGHWPARAMS7		(0xc15c)
#define	rGHWPARAMS8		(0xc600)

//-----------------------------------------
// Global Debug Registers
#define	rGDBGFIFOSPACE		(0xc160)
#define	rGDBGLTSSM		(0xc164)
#define	rGDBGLNMCC		(0xc168)
#define	rGDBGBMU		(0xc16c)

#define	rGDBGLSPMUX		(0xc170)
#define	rGDBGLSP		(0xc174)
#define	rGDBGEPINFO0		(0xc178)
#define	rGDBGEPINFO1		(0xc17c)

#define	rGERRINJCTL_1		(0xc194)
#define	rGERRINJCTL_2		(0xc198)
#define	rGUCTL2			(0xc19c)
#define rGPCIEL1EXTLAT	(0xc1b4)
//-----------------------------------------
// Global PHY Registers
#define	rGUSB2PHYCFG		(0xc200)
#define	rGUSB2PHYCFG_NUM(_x)	(0xc200+(_x*4))
#define	rGUSB2I2CCTL		(0xc240)
#define	rGUSB2PHYACC		(0xc280)
#define	rGUSB2PHYACC_NUM(_x)	(0xc280+(_x*4))
#define	rGUSB3PIPECTL		(0xc2c0)
#define	rGUSB3PIPECTL_NUM(_x)	(0xc2c0+(_x*4))
#define	rLU3LFPSRXTIM		(0xD010)		/* STAR 9001276244B2 */
#define	rLU3LFPSRXTIM_NUM(_x)	(0xD010+(_x * 0x80))	/* STAR 9001276244B2 */
#define	rLSKIPFREQ		(0xD020)
#define	rLSKIPFREQ_NUM(_x)	(0xD020+(_x * 0x80))
#define	rLLUCTL			(0xD024)
#define	rLLUCTL_NUM(_x)		(0xD024+(_x * 0x80))

// Global TX Fifo Size Register ( 0 ~ 31 )
#define	rGTXFIFOSIZ0		(0xC300)

// Global RX Fifo Size Register ( 0 ~ 31 )
#define	rGRXFIFOSIZ0		(0xC380)

//-----------------------------------------
// Global Event Buffer Registers (DWC_USB3_DEVICE_NUM_INT = 1 @C510, GHWPARAMS1[20:15])
#define	rGEVNTADR_LO0		(0xc400)
#define	rGEVNTADR_HI0		(0xc404)
#define	rGEVNTSIZ0		(0xc408)
#define	rGEVNTCOUNT0		(0xc40c)

#define rGEVNTADR_LO(_EVT_NUM)	(0xc400 + 0x10*_EVT_NUM)
#define rGEVNTADR_HI(_EVT_NUM)	(0xc404 + 0x10*_EVT_NUM)
#define rGEVNTSIZ(_EVT_NUM)	(0xc408 + 0x10*_EVT_NUM)
#define rGEVNTCOUNT(_EVT_NUM)	(0xc40c + 0x10*_EVT_NUM)

//-----------------------------------------
// Global Fifo DMA Priority (TX/RX Host/Dev)
#define	rGTXFIFOPRIDEV		(0xC610)
#define	rGTXFIFOPRIHST		(0xC618)
#define	rGRXFIFOPRIHST		(0xC61C)
#define	rGFIFOPRIDBC		(0xC620)
#define	rGDMAHLRATIO		(0xC624)

//-----------------------------------------
// Global Frame Length Adjustment Register
#define	rGFLADJ			(0xC630)

//==============================================================================================
// Device Registers (Dxxxx)
//-----------------------------------------
// Device Common Registers
#define rDCFG			(0xc700)
#define rDCTL			(0xc704)
#define rDEVTEN			(0xc708)
#define rDSTS			(0xc70c)
#define rDGCMDPAR		(0xc710)
#define rDGCMD			(0xc714)
#define rDALEPENA		(0xc720)

//-----------------------------------------
// Device Endpoint Registers (0 <= ep <= 31)
#define rDEPCMDPAR2(ep)		(0xc800 + 0x10*ep)
#define rDEPCMDPAR1(ep)		(0xc804 + 0x10*ep)
#define rDEPCMDPAR0(ep)		(0xc808 + 0x10*ep)
#define rDEPCMD(ep)		(0xc80c + 0x10*ep)

#define rDIMOD(_EVT_NUM)	(0xca00 + 0x4*(_EVT_NUM))

typedef enum {
	USB3_INTCEN = (1 << 0),
	USB3_INCR4 = (1 << 1),
	USB3_INCR8 = (1 << 2),
	USB3_INCR16 = (1 << 3),

	USB3_INCR32 = (1 << 4),
	USB3_INCR64 = (1 << 5),
	USB3_INCR128 = (1 << 6),
	USB3_INCR256 = (1 << 7),
} USB3_BURSTLENGTH;

typedef enum {
	USB3_RAMCLK_BUSCLK = 0,
	USB3_RAMCLK_PIPE = 1,
	USB3_RAMCLK_PIPE_DIV2 = 2,
	USB3_RAMCLK_MAC2 = 3
} USB3_RAMCLK;

//-----------------------
// Global Registers (Gxxxx)
//-----------------------
// Global Common Registers

// GSBUSCFG0		// reset value : 0x00000001
typedef struct {
	/* Bit[0] : Undefined Length INCR Burst Type Enable (INCRBrstEna)
	 * Input to BUS-GM; This bit determines the set of burst lengths the
	 * master interface uses. It works in conjunction with the
	 * GSBUSCFG0[7:1] enables (INCR256/128/64/32/16/8/4).
	 * 0: INCRX burst mode
	 * HBURST (for AHB configurations) and ARLEN/AWLEN (for AXI
	 * configurations) do not use INCR, except in case of non-aligned burst
	 * transfers. In the case of address-aligned transfers, they use only
	 * the following burst lengths:
	 *	■ 1
	 *	■ 4 (if GSBUSCFG0.INCR4BrstEna = 1)
	 *	■ 8 (if GSBUSCFG0.INCR8BrstEna = 1)
	 *	■ 16 (if GSBUSCFG0.INCR16BrstEna = 1)
	 *	■ 32 (if GSBUSCFG0.INCR32BrstEna = 1)
	 *	■ 64 (if GSBUSCFG0.INCR64BrstEna = 1)
	 *	■ 128 (if GSBUSCFG0.INCR128BrstEna = 1)
	 *	■ 256 (if GSBUSCFG0.INCR256BrstEna = 1)
	 * Note: In case of non-address-aligned transfers, INCR may get
	 * generated at the beginning and end of the transfers to align the
	 * address boundaries, even though INCR is disabled.
	 * 1: INCR (undefined length) burst mode
	 *	■ AHB configurations: HBURST uses SINGLE or INCR of any length
	 *	  less than or equal to the largest-enabled burst length of
	 *	  INCR4/8/16.
	 *	■ AXI configurations: ARLEN/AWLEN uses any length less than or
	 *	  equal to the largest-enabled burst length of
	 *	  INCR4/8/16/32/64/128/256.
	 * For cache line-aligned applications, this bit is typically set to 0
	 * to ensure that the master interface uses only power-of-2 burst
	 * lengths (as enabled via GSBUSCFG0[7:0]).
	 */
	unsigned incr_xbrst_ena :1;
	/* Bit[1] : INCR4 Burst Type Enable
	 * Input to BUS-GM; For the AXI configuration, when this bit is enabled
	 * the controller is allowed to do bursts of beat length 1, 2, 3, and 4.
	 * It is highly recommended that this bit is enabled to prevent
	 * descriptor reads and writes from being broken up into separate
	 * transfers.
	 */
	unsigned incr_4brst_ena :1;
	/* Bit[2] : INCR8 Burst Type Enable
	 * Input to BUS-GM; For the AHB/AXI configuration, if software set this
	 * bit to "1", the AHB/AXI master uses INCR to do the 8-beat burst.
	 */
	unsigned incr_8brst_ena :1;
	/* Bit[3] : INCR16 Burst Type Enable
	 * Input to BUS-GM. For the AHB/AXI configuration, if software set this
	 * bit to '1', the AHB/AXI master uses INCR to do the 16-beat burst.
	 */
	unsigned incr_16brst_ena :1;	// bit[3] : INCR16 burst type
	unsigned incr_32brst_ena :1;	// bit[4] : INCR32 burst type
	unsigned incr_64brst_ena :1;	// bit[5] : INCR64 burst type
	unsigned incr_128brst_ena :1;	// bit[6] : INCR128 burst type
	unsigned incr_256brst_ena :1;	// bit[7] : INCR1256 burst type
	unsigned rsvd9_8 :2;	// bit[9:8] :
	unsigned desc_big_end :1;	// bit[10] : Descriptor access is big endian
	unsigned dat_big_end :1;	// bit[11] : data access is big endian
	unsigned sbus_store_and_forward :1;	// bit[12] : bus store-and-forward mode?
	unsigned rsvd15_13 :3;	// bit[15:13] :
	unsigned des_wr_req_infor :4;	// bit[19:16] : Attributte of DMA Wr for descriptor request
	unsigned data_wr_req_infor :4;	// bit[23:20] : Attributte of DMA Wr for data request
	unsigned des_rd_req_infor :4;	// bit[27:24] : Attributte of DMA Rd for descriptor request
	unsigned data_rd_req_infor :4;	// bit[23:20] : Attributte of DMA Rd for data request
} __attribute__((__packed__)) USB3_REG_GSBUSCFG0_BitField;

typedef union {
	u32 data;
	USB3_REG_GSBUSCFG0_BitField b;
} USB3_REG_GSBUSCFG0_o, *USB3_REG_GSBUSCFG0_p;

// GSUBSCFG1
typedef struct {
	unsigned rsvd7_0 :8;	// bit[7:0] :
	unsigned breq_limit :4;	// bit[11:8] : axi burst request limit
	unsigned en_1kpage :1;	// bit[12] : 1k page boundary enable
	unsigned rsvd31_13 :19;	// bit[31:13]
} __attribute__((__packed__)) USB3_REG_GSBUSCFG1_BitField;

typedef union {
	u32 data;	// reset value : 0x00000300
	USB3_REG_GSBUSCFG1_BitField b;
} USB3_REG_GSBUSCFG1_o, *USB3_REG_GSBUSCFG1_p;

// GTXTHRCFG
typedef struct {
	union {
		struct {
			unsigned UsbMaxTxBurstSize_Prd :5;	// bit [4:0]USB Maximum Periodic ESS RX Burst Size - Host Mode Only
			unsigned UsbTxThrNumPkt_Prd :5;		// bit [9:5] :: USB Periodic ESS Receive Packet Threshold Count - Host Mode Only
			unsigned UsbTxThrNumPktSel_Prd :1;	// bit [10 ::USB Periodic ESS Receive Packet Threshold Enable - Host Mode Only
			unsigned reserved_12_11 :2;		// bit [12:11]
			unsigned UsbTxThrNumPkt_HS_Prd :2;	// bit [14:13] USB HS High Bandwidth Periodic Receive Packet Threshold Count
			unsigned UsbTxThrNumPktSel_HS_Prd :1;	// bit[15]USB HS High Bandwidth Periodic Receive Packet Threshold Enable
			unsigned UsbMaxTxBurstSize :5;		// bit[20:16]-USB Async ESS Maximum Receive Burst Size
			unsigned UsbTxPktCnt :5;		// bit[25:21]] -USB Async ESS Receive Packet Threshold Count
			unsigned UsbTxPktCntSel :1;		// bit[26] USB Async ESS Receive Packet Threshold Enable -
			unsigned reserved_31_27 :5;		// bit[31:27]
		} ssp;
		struct {
			unsigned rsvd18_0 :19;		// bit [18:0]
			unsigned USBMaxTxBurstSize :5;	// bit [23:19] :: USB Maximum Receive Burst Size
			unsigned USBTxPktCnt :4;	// bit [27:24] :: USB Receive Packet Count
			unsigned rsvd28 :1;		// bit [28]
			unsigned USBTxPktCntSel :1;	// bit [29]
			unsigned rsvd31_30 :2;		// bit[31:30]
		} ss;
	};
} __attribute__((__packed__)) USB3_REG_GTXTHRCFG_BitField;

typedef union {
	u32 data;
	USB3_REG_GTXTHRCFG_BitField b;
} USB3_REG_GTXTHRCFG_o, *USB3_REG_GTXTHRCFG_p;

typedef struct {
	union {
		struct {
			unsigned UsbMaxRxBurstSize_Prd :5;	// bit [4:0]USB Maximum Periodic ESS RX Burst Size - Host Mode Only
			unsigned UsbRxThrNumPkt_Prd :5;		// bit [9:5] :: USB Periodic ESS Receive Packet Threshold Count - Host Mode Only
			unsigned UsbRxThrNumPktSel_Prd :1;	// bit [10 ::USB Periodic ESS Receive Packet Threshold Enable - Host Mode Only
			unsigned reserved_12_11 :2;		// bit [12:11]
			unsigned UsbRxThrNumPkt_HS_Prd :2;	// bit [14:13] USB HS High Bandwidth Periodic Receive Packet Threshold Count
			unsigned UsbRxThrNumPktSel_HS_Prd :1;	// bit[15]USB HS High Bandwidth Periodic Receive Packet Threshold Enable
			unsigned UsbMaxRxBurstSize :5;		// bit[20:16]-USB Async ESS Maximum Receive Burst Size
			unsigned UsbRxPktCnt :5;		// bit[25:21]] -USB Async ESS Receive Packet Threshold Count
			unsigned UsbRxPktCntSel :1;		// bit[26] USB Async ESS Receive Packet Threshold Enable -
			unsigned reserved_31_27 :5;		// bit[31:27]
		} ssp;
		struct {
			unsigned rsvd18_0 :19;		// bit [18:0]
			unsigned USBMaxRxBurstSize :5;	// bit [23:19] :: USB Maximum Receive Burst Size
			unsigned USBRxPktCnt :4;	// bit [27:24] :: USB Receive Packet Count
			unsigned rsvd28 :1;		// bit [28]
			unsigned USBRxPktCntSel :1;	// bit [29]
			unsigned rsvd31_30 :2;		// bit[31:30]
		} ss;
	};
} __attribute__((__packed__)) USB3_REG_GRXTHRCFG_BitField;

typedef union {
	u32 data;
	USB3_REG_GRXTHRCFG_BitField b;
} USB3_REG_GRXTHRCFG_o, *USB3_REG_GRXTHRCFG_p;

//3 GCTL : 0xC110
typedef struct {
	unsigned dis_clk_gating :1;	// bit[0] : Disable Clock Gating in LP Mode ( 0:Enable, 1:disable )
	unsigned GblHibernationEn :1;	// bit[1] : Enable Hibernation at the Global Level (Not Support)
	unsigned U2EXIT_LFPS :1;	// bit[2] : (Add at 2.50a) U2 LFPS Exit Error
	unsigned DisScramble :1;	// bit[3] : Disable Data Scrambling in SS ( 0:enable, 1:disable )
	unsigned ScaleDown :2;	// bit[5:4] : Scale Down : for simulation
	unsigned ram_clk_sel :2;	// bit[7:6] : ram clock select (0:bus, 1:pipe, 2:pipe/2, 3:rsvd)
	unsigned debug_attach :1;	// bit[8] : debug attach
	unsigned DisU1U2TimerScaleDown :1;	// bit[9] : (Add at 2.50a) Disable U1/U2 Timer Scale Down
	unsigned SOFITPSYNC :1;	// bit[10] : (Add at 2.50a) SOF ITP SYNC
	unsigned core_soft_reset :1;	// bit[11] : core soft reset
	unsigned port_capdir :2;	// bit[13:12] : port cabpbility direction (1:host, 2:device, 3:otg configuration)
	unsigned frm_scale_down :2;	// bit[15:14] :
	unsigned u2rst_ecn :1;	// bit[16] :
	unsigned BYPSSETADDR :1;	// bit[17] : (Add at 2.50a)
	unsigned MasterFiltBypass :1;	// bit[18] : (Add at 2.50a) Master Filter bypass
	unsigned pwr_down_scale :13;	// bit[31:19] : power down scale
} __attribute__((__packed__)) USB3_REG_GCTL_BitField;

typedef union {
	u32 data;	// reset value : 0x30c02000
	USB3_REG_GCTL_BitField b;
} USB3_REG_GCTL_o, *USB3_REG_GCTL_p;

//3 GSTS : 0xC118
typedef struct {
	unsigned CurMod :2;	// bit[1:0] : current mode of operation
	unsigned rsvd3_2 :2;	// bit[3:2] : rsvd 3:2
	unsigned BusErrAddVld :1;	// bit[4] : Master Bus Error Detection
	unsigned CSRTimeout :1;	// bit[5] : CSR Time Out
	unsigned DevIP :1;	// bit[6] : Device Interrupt Pending
	unsigned HostIP :1;	// bit[7] : Host Interrupt Pending
	unsigned ADPIP :1;	// bit[8] : ADP Interrupt Pending
	unsigned BCIP :1;	// bit[9] : BC Interrupt Pending
	unsigned OTGIP :1;	// bit[10] : OTG Interrupt Pending
	unsigned rsvd19_11 :9;	// bit[19:11] : rsvd 19:11
	unsigned CBELT :12;	// bit[31:20] : Current BELT Value
} USB3_REG_GSTS_BitField;

typedef union {
	u32 data;	// reset value : 0x
	USB3_REG_GSTS_BitField b;
} USB3_REG_GSTS_o;

/* 3 GUCTL1 : 0xC11C (Add at 2.50a) */
typedef struct {
	/* bit[0] LOA_FILTER_EN
	 * USB 2.0 Port Babble is checked at least 3 times before Port is
	 * disabled
	 */
	unsigned LOA_FILTER_EN :1;
	/* bit[1] OVRLD_L1_SUSP_COM (ss version only)
	 * the utmi_l1_suspend_com_n is overloaded with the
	 * utmi_sleep_n signal
	 */
	unsigned OVRLD_L1_SUSP_COM :1;
	/* bit[2] Host Parameter Check Disable
	 * When this bit is set to '0', the xHC checks that the input slot/EP
	 * context fields comply to the xHCI Specification. Upon detection of a
	 * parameter error during command execution, the xHC generates an event
	 * TRB with completion code indicating 'PARAMETER ERROR'.
	 * When the bit is set to '1', the xHC does not perform parameter checks
	 * and does not generate 'PARAMETER ERROR' completion code.
	 */
	unsigned HParChkDisable :1;
	/* bit[3] Host ELD Enable
	 * When this bit is set to 1, it enables the Exit Latency Delta (ELD)
	 * support defined in the xHCI 1.1.
	 * This bit is used only in the host mode.
	 * This bit has to be set to 1 in Host mode
	 */
	unsigned HELDEn :1;
	/* bit[7:4] L1_SUSP_THRLD_FOR_HOST
	 * This field is effective only when the L1_SUSP_THRLD_EN_FOR_HOST bit
	 * is set to 1. For more details, refer to the description of the
	 * L1_SUSP_THRLD_EN_FOR_HOST bit.
	 * Note: Program this register field based on the UTMI/ULPI wakeup time
	 * in L1 suspend/sleep. In addition, for PCIe-based implementation,
	 * consider the DBESLD register value also.
	 */
	unsigned L1_SUSP_THRLD_FOR_HOST :4;
	/* bit[8] L1_SUSP_THRLD_EN_FOR_HOST
	 * This bit is used only in host mode.
	 * The host controller asserts the utmi_l1_suspend_n and utmi_sleep_n
	 * output signals (see Table 5-18 on page 363) as follows:
	 * The controller asserts the utmi_l1_suspend_n signal to put the PHY
	 * into deep low-power mode in L1 when both of the following are true:
	 *	■ The device accepted BESL/BESLD value is greater than or equal
	 *	  to the value in L1_SUSP_THRLD_FOR_HOST field.
	 *	■ The L1_SUSP_THRLD_EN_FOR_HOST bit is set to 1'b1.
	 *  The controller asserts utmi_sleep_n on L1 when one of the following
	 *  is true:
	 *	■ The device accepted BESL/BESLD value is less than the value
	 *	  in L1_SUSP_THRLD_FOR_HOST field.
	 *	■ The L1_SUSP_THRLD_EN_FOR_HOST bit is set to 1'b0.
	 */
	unsigned L1_SUSP_THRLD_EN_FOR_HOST :1;
	/* bit[9] DEV_HS_NYET_BULK_SPR (ss only)
	 *	■ 0: Default behavior, no change in device response
	 *	■ 1: Feature enabled, HS bulk OUT short packet gets NYET response
	 * This bit is applicable for device mode only (and ignored in host mode)
	 *  to be used in 2.0 operation.
	 *  If this bit is set, the device core sends NYET response instead of
	 *  ACK response for a successfully received bulk OUT short packet.
	 *  If NYET is sent after receiving short packet, then the host would
	 *  PING before sending the next OUT; this improves the performance as
	 *  well as clears up the buffer/cache on the host side. Internal to
	 *  the device core, short packet (SPR=1) processing takes some time,
	 *  and during this time, the USB is flow controlled. With NYET response
	 *  instead of ACK on short packet, the host does not send another
	 *  OUT-DATA without pinging in HS mode.
	 *  This bit is quasi-static, that is, it must not be changed during
	 *  device operation.
	 */
	unsigned DEV_HS_NYET_BULK_SPR :1;
	/* bit[10] RESUME_OPMODE_HS_HOST (ssp only)
	 * This bit is used only in host mode, and is for 2.0 opmode behaviour
	 * in HS Resume
	 *	■ When this bit is set to '1', the utmi/ulpi opmode will be
	 *	  changed to "normal" along with HS terminations after EOR.
	 *	  This is to support certain legacy UTMI/ULPI PHYs.
	 *	■ When this bit is set to '0', the utmi/ulpi opmode will be
	 *	  changed to "normal" 2us after HS terminations change after EOR.
	 */
	unsigned RESUME_OPMODE_HS_HOST :1;
	/* bit[11] Disable ref_clk gating (SSP only)
	 * If the ref_clk gating is disabled then input ref_clk cannot be turned
	 * off to SSPHY and controller. This is independent of GCTL[DisClkGtng]
	 * setting
	 */
	unsigned DISREFCLKGTNG :1;
	/* bit[12] Disable ref_clk gating for USB 2.0 PHY
	 * If ref_clk gating is disabled, then the ref_clk input cannot be
	 * turned off to the USB 2.0 PHY and controller. This is independent of
	 * the GCTL[DisClkGtng] setting.
	 *	■ 1'b0: ref_clk gating enabled for USB 2.0 PHY
	 *	■ 1'b1: ref_clk gating disabled for USB 2.0 PHY
	 */
	unsigned DISUSB20REFCLKGTNG :1;
	/* bit[13] Disable hardware LPM functioDisable hardware LPM function in
	 * all USB 2.0 ports
	 * When set, it disables hardware LPM function in all USB 2.0 ports.
	 */
	unsigned HW_LPM_HLE_DISABLE :1;
	/* bit[14] Disable hardware LPM capability in the xHCI capability
	 * register
	 * When set, it disables hardware LPM capability in xHCI capability
	 * register.
	 */
	unsigned HW_LPM_CAP_DISABLE :1;
	/* bit[15] PARKMODE_DISABLE_FSLS (SS Only)
	 * This bit is used only in host mode, and is for debug purpose only.
	 * When this bit is set to '1' all FS/LS bus instances in park mode
	 * disabled.
	 */
	unsigned PARKMODE_DISABLE_FSLS :1;
	/* bit[16] PARKMODE_DISABLE_HS (SS Only)
	 * This bit is used only in host mode.
	 * When this bit is set to '1' all HS bus instances park mode are
	 * disabled.
	 * To improve performance in park mode, the xHCI scheduler queues in
	 * three requests of 4 packets each for High Speed asynchronous
	 * endpoints in a micro-frame. But if a device is slow and if it NAKs
	 * more than 3 times, then it is rescheduled only in the next
	 * micro-frame. This could decrease the performance of a slow device
	 * even further.
	 * In a few high speed devices (such as Sandisk Cruzer Blade 4GB
	 * VID:1921, PID:21863 and Flex Drive VID:3744, PID:8552) when an IN
	 * request is sent within 900ns of the ACK of the previous packet,
	 * these devices send a NAK. When connected to these devices, if
	 * required, the software can disable the park mode if you see
	 * performance drop in your system. When park mode is disabled,
	 * pipelining of multiple packet is disabled and instead one packet at
	 * a time is requested by the scheduler. This allows up to 12 NAKs in a
	 * micro-frame and improves performance of these slow devices.
	 */
	unsigned PARKMODE_DISABLE_HS :1;
	/* bit[17] PARKMODE_DISABLE_SS (SS Only)
	 * This bit is used only in host mode, and is for debug purpose only.
	 * When this bit is set to '1' all SS bus instances in park mode are
	 * disabled.
	 */
	unsigned PARKMODE_DISABLE_SS :1;
	/* bit[18] Enable HS Async Endpoint for Host Controller (SS only)
	 *	■ 1: Enables performance enhancement for HS async endpoints in
	 *	     the presence of NAKs
	 *	■ 0: Enhancement not applied
	 * If a periodic endpoint is present, and if a bulk endpoint which is
	 * also active is being NAKed by the device, then this could result in
	 * decrease in performance of other High Speed bulk endpoint which is
	 * ACked by the device. Setting this bit to 1, will enable the host
	 * controller to schedule more transactions to the async endpoints
	 * (bulk/ control) and hence will improve the performance of the bulk
	 * endpoint. This control bit should be enabled only if the existing
	 * performance with the default setting is not sufficient for your
	 * HighSpeed application. Setting this bit will only control, and is
	 * only required for High Speed transfers.
	 */
	unsigned NAK_PER_ENH_HS :1;
	/* bit[19] Enable FS Async Endpoint for Host Controller (SS only)
	 *	■ 1: Enables performance enhancement for FS async endpoints in
	 *	     the presence of NAKs
	 *	■ 0: Enhancement not applied
	 * If a periodic endpoint is present , and if a bulk endpoint which is
	 * also active is being NAKed by the device, then this could result in
	 * a decrease in performance of other Full Speed bulk endpoint which is
	 * ACked by the device. Setting this bit to 1, will enable the host
	 * controller to schedule more transactions to the async endpoints
	 * (bulk/ control) and hence will improve the performance of the bulk
	 * endpoint. This control bit should be enabled only if the existing
	 * performance with the default setting is not sufficient for your
	 * FullSpeed application. Setting this bit will only control, and is
	 *  only required for Full Speed transfers.
	 */
	unsigned NAK_PER_ENH_FS :1;
	/* bit[20] DEV_LSP_TAIL_LOCK_DIS
	 *	■ 0: Default behavior, enables device lsp lock logic for tail
	 *	     TRB update
	 *	■ 1: Fix disabled
	 * This is a bug fix for STAR 9000716195 that affects the CSP mode for
	 * OUT endpoints in device mode. The issue is that tail TRB index is not
	 * synchronized with the cache Scratchpad bytecount update. If the
	 * fast-forward request comes in-between the bytecount update on a newly
	 * fetched TRB and the tail-index write update in TPF, the RDP works on
	 * an incorrect tail index and misses the byte count decrement for the
	 * newly fetched TRB in the fast-forwarding process. This fix needs to
	 * be present all the times.
	 */
	unsigned DEV_LSP_TAIL_LOCK_DIS :1;
	/* bit[23:21]
	 * This register field is used to add on to the default inter packet
	 * gap setting in the USB 2.0 MAC.
	 * It should be programmed to a non-zero value only in case where you
	 * need to increase the default inter packet delay calculations in the
	 * USB 2.0 MAC module.
	 * The inter packet delay is increased by number of utmi/ulpi clock
	 * cycles of this field value.
	 */
	unsigned IP_GAP_ADD_ON :3;
	/* bit[24] HW L1 Remote Wake-up Enable
	 *	■ 0: Default behavior, disables device L1 hardware exit logic
	 *	■ 1: feature enabled
	 * This bit is applicable for device mode (2.0) only. This field enables
	 * device controller sending remote wakeup for L1 if the device becomes
	 * ready for sending/accepting data when in L1 state. If the host
	 * expects the device to send remote wakeup signaling to resume after
	 * going into L1 in flow controlled state, then this bit can be set to
	 * send the remote wake signal automatically when the device controller
	 * becomes ready. This hardware remote wake feature is applicable only
	 * to bulk and interrupt transfers, and not for Isoch/Control
	 *	■ When control transfers are in progress, the LPM will be
	 *	  rejected (NYET response). Only after control transfers are
	 *	  completed (either with ACK/STALL), LPM will be accepted
	 *	■ For Isoch transfers, the host needs to do the wake-up and
	 *	  start the transfer. Device controller will not do
	 *	  remote-wakeup when Isoch endpoints get ready. The device SW
	 *	  needs to keep the GUSB2PHYCFG[EnblSlpM] reset in order to
	 *	  keep the PHY clock to be running for keeping track of SOF
	 *	  intervals.
	 *	■ When L1 hibernation is enabled, the controller will not do
	 *	  automatic exit for hibernation requests thru L1.
	 * This bit is quasi-static, that is, it must not be changed during
	 * device operation
	 */
	unsigned DEV_L1_EXIT_BY_HW :1;
	/* bit[25] PHY P3 in U2 (SS only)
	 *	■ 0: Default behavior, When SuperSpeed link is in U2,
	 *	  PowerState P2 is attempted on the PIPE Interface.
	 *	■ 1: When SuperSpeed link is in U2, PowerState P3 is attempted
	 *	  if GUSB3PIPECTL[17] is set.
	 * Setting this bit enables P3 Power State when the SuperSpeed link is
	 * in U2. Another Power Saving option. Check with your PHY vendor before
	 * enabling this option.When setting this bit to 1 to enable P3 in P2,
	 * GUSB3PIPECTL[27] should be set to 0 to make sure that the U2 exit is
	 * attempted in P0. This bit should be set only when GCTL.SOFITPSYNC=1
	 * or GFLADJ.GFLADJ_REFCLK_LPM_SEL=1.
	 */
	unsigned P3_IN_U2 :1;
	/* bit[26] Force UTMI clk to PIPE3
	 *	■ 0: Default behavior, Uses 3.0 clock when operating in 2.0 mode
	 *	■ 1: Feature enabled
	 * This bit is applicable (and to be set) for device mode
	 * (DCFG.Speed != SS) only. In the 3.0 device core, if the core is
	 * programmed to operate in 2.0 only (that is, Device Speed is
	 * programmed to 2.0 speeds in DCFG[Speed]), then setting this bit
	 * makes the internal 2.0 (utmi/ulpi) clock to be routed as the
	 * 3.0 (pipe) clock. Enabling this feature allows the pipe3 clock to
	 * be not-running when forcibly operating in 2.0 device mode.
	 * Note:
	 *	■ When using this feature, all pipe3 inputs must be in inactive
	 *	  mode. In particular, the pipe3 clocks must not be running and
	 *	  the pipe3_phystatus_async must be tied to 0. This bit should
	 *	  not be set if the core is programmed to operate in SuperSpeed
	 *	  mode (even when it falls back to 2.0).
	 *	■ This bit is quasi-static, that is, it must not be changed
	 *	  during operation.
	 *	■ If the parameter "DWC_USB3_REMOVE_PIPE_CLK_MUX_FOR_20_MODE"
	 *	  is enabled, then muxing 2.0 clock/signals to pipe_clk/signals
	 *	  have to be done outside the controller for this feature to
	 *	  work.
	 */
	unsigned DEV_FORCE_20_CLK_FOR_30_CLK :1;
	/* bit[27]  OUT TRB Status indicates short packet
	 *	■ 0: Default behavior, no change in TRB status dword
	 *	■ 1: Feature enabled, OUT TRB status indicates Short Packet
	 * This bit is applicable for device mode only (and ignored in host
	 * mode). If the device application (software/hardware) wants to know
	 * if a short packet was received for an OUT in the TRB status itself,
	 * then this feature can be enabled, so that a bit is set in the TRB
	 * writeback in the buf_size dword. Bit[26] - SPR of the {trbstatus,
	 * RSVD, SPR, PCM1, bufsize} dword will be set during an OUT transfer
	 * TRB write back if this is the last TRB used for that transfer
	 * descriptor. This bit is quasi-static, that is, it must not be changed
	 * during device operation.
	 */
	unsigned DEV_TRB_OUT_SPR_IND :1;
	/* bit[28] disable Line state check during HS transmit
	 *	■ 0: Default behavior, no change in Linestate check
	 *	■ 1: Feature enabled, 2.0 MAC disables Linestate check during
	 *	     HS transmit
	 * This bit is applicable for HS operation of u2mac. If this feature is
	 * enabled, then the 2.0 mac operating in HS ignores the UTMI/ULPI
	 * Linestate during the transmit of a token (during token-to-token and
	 * token-to-data IPGAP). When enabled, the controller implements a fixed
	 * 40-bit TxEndDelay after the packet is given on UTMI and ignores the
	 * Linestate during this time. This feature is applicable only in HS
	 * mode of operation.
	 * Device mode: If GUCTL1.TX_IPGAP_LINECHECK_DIS is set, then for
	 * device LPM handshake, the core will ignore the linestate after TX and
	 * wait for fixed clocks (40 bit times equivalent) after transmitting ACK
	 * on utmi.
	 * Host mode: If GUCTL1.TX_IPGAP_LINECHECK_DIS is set, then the ipgap
	 * between (tkn to tkn/data) is added by 40 bit times of TXENDDELAY, and
	 * linestate is ignored during this 40 bit times delay.
	 * Enable this bit if the LineState will not reflect the expected line
	 * state (J) during transmission. This bit is quasi-static, that is,
	 * it must not be changed during device operation.
	 */
	unsigned TX_IPGAP_LINECHECK_DIS :1;
	/* bit[29] FS/LS SE0 filtered for 2 clocks for detect EOP
	 *	■ 0: Single sampling (utmi/ulpi clock) of linestate is checked
	 *	     for SE0 detection.
	 *	■ 1: Feature enabled; Two samplings of linestate are checked for
	 *	     SE0 detection
	 * This bit is applicable for FS/LS operation. If this feature is
	 * enabled, SE0 on the linestate is validated for two consecutive
	 * utmi/ulpi clock edges for EOP detection. This feature is applicable
	 * only in FS in device mode and FS/LS mode of operation in host mode.
	 *	■ Device mode (FS): If GUCTL1.FILTER_SE0_FSLS_EOP is set,
	 *	  then for device LPM handshake, the controller ignores single
	 *	  SE0 glitch on the linestate during transmit. Only two or more
	 *	  SE0 is considered as a valid EOP on FS port.
	 *	■ Host mode (FS/LS): If GUCTL1.FILTER_SE0_FSLS_EOP is set,
	 *	  then the controller ignores single SE0 glitch on the linestate
	 *	  during transmit. Only two or more SE0 is considered as a valid
	 *	  EOP on FS/LS port.
	 * Enable this feature if linestate has SE0 glitches during
	 * transmission. This bit is quasi-static, that is, it must not be
	 * changed during operation.
	 */
	unsigned FILTER_SE0_FSLS_EOP :1;
	/* bit[30] change tRxDetectTimeoutDFP
	 * This bit is used to control the tRxDetectTimeoutDFP timer for the
	 * SuperSpeed link.
	 *	■ 0: Default behavior; 12ms is used as tRxDetectTimeoutDFP.
	 *	■ 1: 120ms is used as the tRxDetectTimeoutDFP.
	 * This bit is used only in host mode. For more details, refer to
	 * ECN020 for USB 3.0 Specification.
	 */
	unsigned DS_RXDET_MAX_TOUT_CTRL :1;
	/* bit[31] L1 and L2 event are seperated when 2.0 mode
	 * DEV_DECOUPLE_L1L2_EVT Enable this bit if you want to use L1 (LPM)
	 * events separately and not combine it with L2 events when operating
	 * in USB 2.0 speeds.
	 *	■ 0: L1 and L2 events (suspend and resume) are not separated.
	 *	     For both L1 and L2 events, common suspend and resume events
	 *	     are generated.
	 *	■ 1: L1 and L2 events are separated when operating in USB 2.0
	 *	     mode. For L1 and L2 events, separate suspend and resume
	 *	     events are generated.
	 */
	unsigned DEV_DECOUPLE_L1L2_EVT :1;
} __attribute__((__packed__)) USB3_REG_GUCTL1_BitField;

typedef union {
	u32 data;
	USB3_REG_GUCTL1_BitField b;
} USB3_REG_GUCTL1_o, *USB3_REG_GUCTL1_p;

//3 GSNPSID : 0xC120
typedef struct {
	/* Bit[15:0] : Release Number :: IP Version */
	unsigned ReleaseNum :16;
	/* Bit[31:16] product controller identification number
	 *	■ 0x5533 is ASCII for U3 (DWC_usb3).
	 *	■ 0x3331 is ASCII for 31 represents USB 3.1 (DWC_usb31).
	 */
	unsigned CoreID :16;	// bit[31:16]: Core ID :: 0x5533
} __attribute__((__packed__)) USB3_REG_GSNPSID_BitField;

typedef union {
	u32 data;
	USB3_REG_GSNPSID_BitField b;
} USB3_REG_GSNPSID_o;

//3 GUID : 0xC128
typedef struct {
	u32 data;
} USB3_REG_GUID_o;

//3 GUCTL : 0xC12C
typedef struct {
	unsigned DTFT :9;
	unsigned DTCT :2;
	unsigned InsrtExtrFSBODl :1;	// bit[11]		: Insert Extra Delay between FS Bulk Out
	unsigned ExtCapSuptEN :1;	// bit[12]		: External Extended Capability Support Enable
	unsigned EnOverlapChk :1;	// bit[13]		: Enable Check for LFPS Overlap During Remote Ux Exit
	unsigned USBHstInAutoRetryEn :1;	// bit[14]		: Host In Auto Retry
	unsigned CMdevAddr :1;	// bit[15]		: Compliance Mode for Device Address
	unsigned ResBwHSEPS :1;	// bit[16]		: Reserving 85% Bandwidth for HS Periodic EPs
	unsigned SprsCtrlTransEn :1;	// bit[17]		: Sparse HS/FS/LS Control Transaction Enable
	unsigned EN_EXTD_TBC_CAP :1;	// bit[18] When set, the Extended TBC Capability is reported in
	unsigned IgnoreHCETimeout :1;	// bit[19] : ignore HCE timeout
	unsigned DMAIgnoreHCE :1;	// bit[20] : DMA ignore HCE
	unsigned NoExtrDl :1;	// bit[21]			: (Add at 2.50a)No Extra Delay Between SOF and the First Packet
	unsigned REFCLKPER :10;	// bit[31:22]	: (Add at 2.50a)This field indicates in terms of nano seconds the period of ref_clk
} __attribute__((__packed__)) USB3_REG_GUCTL_BitField;

typedef union {
	u32 data;	// reset value : 0x
	USB3_REG_GUCTL_BitField b;
} USB3_REG_GUCTL_o, *USB3_REG_GUCTL_p;

//1 Global Debug Registers
//3 GDBGLTSSM : 0xC164
typedef struct {
	unsigned TXOnesZeros :1;	// bit[0]	: when tx SS compliance patterns CP7 or CP8
	unsigned RxTermination :1;	// bit[1]	: Control presence of rx terminations
	unsigned TXSwing :1;	// bit[2]	: Select TX Voltage Level
	unsigned LTDBClkState :3;	// bit[5:3]	: LTSSM Clock State
	unsigned TxDeemphasis :2;	// bit[7:6]	: TX De-Emphasis
	unsigned RxEqTrain :1;	// bit[8]		: RX to bypass normal operation to perform eq training. while performing training the state of the Rx data interface is undefiend
	unsigned PowerDown :2;	// bit[10:9] : Power Up/Down State
	unsigned LTDBPhyCmdState :3;	// bit[13:11] : LTSSM PHY Command State
	unsigned TxDetRxLoopback :1;	// bit[14] : Used to tell the PHY to begin a rx detection operation or to begin loopback or to signal LFPS during P0 for USB Polling State.
	unsigned RxPolarity :1;	// bit[15] : Tells phy to do a polarity inversion on the rx data
	unsigned TxElecIdle :1;	// bit[16] :
	unsigned ElasticBufferMode :1;	// bit[17] : 2.0 PHY Soft Reset
	unsigned LTDBSubState :4;	// bit[21:18] :
	unsigned LTDBLinkState :4;	// bit[25:22] :
	unsigned LTDBTimeout :1;	// bit[26] :
	unsigned PortDirection :1;	// bit[27] : Port Direction
	unsigned rsvd31_28 :2;	// bit[31:28] :
} __attribute__((__packed__)) USB3_REG_GDBGLTSSM_BitField;

typedef union {
	u32 data;						// reset value : 0x
	USB3_REG_GDBGLTSSM_BitField b;
} USB3_REG_GDBGLTSSM_o;

//Global Debug LSP Mux Register - device
//GDBGLSPMUX_DEV : 0xC170
typedef struct {
	unsigned EPSELECT :4;		// bit[3:0]	: Selects the Endpoint debug information presented in the GDBGEPINFO registers in device mode.
	unsigned DEVSELECT :4;		// bit[7:4]	: Selects the LSP debug information presented in the GDBGLSP register in device mode.
	unsigned HOSTSELECT :6;		// bit[13:8]	:
	unsigned RSVD_14 :1;		// bit[14]	:
	unsigned ENDBC :1;		// bit[15]	: Enable debugging of Debug capablity LSP in Host mode. Use HostSelect to select DbC LSP debug information presented in the GDBGLSP register.
	unsigned LOGIC_ANAL_TRACE :8;	// bit[23:16]	: Logic Analyzer Trace Port MUX Select
	unsigned reserved_31_24 :8;	// bit[31:24]	:
} __attribute__((__packed__)) USB3_REG_GDBGLSPMUX_DEV_BitField;

typedef union {
	u32 data;						// reset value : 0x
	USB3_REG_GDBGLSPMUX_DEV_BitField b;
} USB3_REG_GDBGLSPMUX_DEV_o, *USB3_REG_GDBGLSPMUX_DEV_p;

//1 Global PHY Registers
//3 GUSB2PHYCFG (0xC200)
typedef struct {
	unsigned TOutCal :3;	// bit[2:0] : HS/FS Timeout Calibration
	unsigned PHYIf :1;	// bit[3]	: UTMI+ Interface 0 : 8bit, 1 : 16bit
	unsigned ULPI_UTMI_Sel :1;	// bit[4]	: (Always 0)ULPI or UTMI+ Select : Only Support UTMI+
	unsigned FSIntf :1;	// bit[5]	: (Always 0)Full-Speed Serial Interface Select : Not Supported
	unsigned SusPHY :1;	// bit[6]	: Suspend USB2.0 PHY if suspend conditions are valid
	unsigned PHYSel :1;	// bit[7]	: (Always 0)2.0 Transceiver Selection
	unsigned EnblSlpM :1;	// bit[8]	: Enable control sleep command for UIMI and ULPI
	unsigned XCVRDLY :1;	// bit[9]	: Trnasceiver Delay
	unsigned USBTrdTim :4;	// bit[13:10] : Sets turnaround time in PHY Clocks
	unsigned RSVD14 :1;	// bit[16] :
	unsigned ULPIAutoRes :1;	// bit[15] : ULPI Auto Resume
	unsigned RSVD16 :1;	// bit[16] :
	unsigned ULPIExtVbusDrv :1;	// bit[17] : ULPI External VBUS Drive
	unsigned ULPIExtVbusIndicator :1;	// bit[18] : ULPI External VBUS Indicator
	unsigned LSIPD :3;	// bit[21:19] : LS Inter-Packet Time
	unsigned LSTRDTIM :3;	// bit[24:22] : LS Turnaround Time
	unsigned OVRD_FSLS_DISC_TIME :1;	// bit[25] :Overriding the FS/LS disconnect time to 32us
	unsigned INV_SEL_HSIC :1;	// bit[26] :
	unsigned HSIC_CON_WIDTH_ADJ :2;	// bit[28:27] :
	unsigned ULPI_LPM_WITH_OPMODE_CHK :1;	// bit[29] : Support the LPM over ULPI without NOPID token to the ULPI PHY.
	unsigned U2_FREECLK_EXISTS :1;	// bit[30] : Specifies whether your USB 2.0 PHY provides free-running PHY clock
	unsigned PHYSoftRst :1;	// bit[31] : 2.0 PHY Soft Reset
} __attribute__((__packed__)) USB3_REG_GUSB2PHYCFG_BitField;

typedef union {
	u32 data;	// reset value : 0x
	USB3_REG_GUSB2PHYCFG_BitField b;
} USB3_REG_GUSB2PHYCFG_o;

//3 GUSB3PIPECTL

typedef struct {
	unsigned ElasticBufferMode :1;	// bit[0]	:: Elastic Buffer Mode
	unsigned TxDeemphasis :2;	// bit[2:1] :: TX Deemphasis
	unsigned TxMargin :3;	// bit[5:3] :: Tx Margine[2:0]
	unsigned TxSwing :1;	// bit[6]	:: TX Swing
	unsigned SSICEn :1;	// bit[7]	:: USB3 SSIC ENABLE
	unsigned RX_DETECT_TO_POLLING :1;	// bit[8]	:: RX_DETECT to Polling.LFPS Control
	unsigned LFPSFilt :1;	// bit[9]	:: LFPS Filter
	unsigned P3ExSigP2 :1;	// bit[10]	:: P3 Exit Siganl in P2
	unsigned P3P2TranOK :1;	// bit[11]	:: P3 P2 Transitions OK
	unsigned LFPSP0Algn :1;	// bit[12]	:: LFPS P0 Align
	unsigned SkipRxDet :1;	// bit[13]	:: Skip Rx Detect
	unsigned AbortRxDetInU2 :1;	// bit[14]	:: Abort Rx Detect in U2
	unsigned DatWidth :2;	// bit[16:15]	:: PIPE Data Width
	unsigned suspend_usb3_ss_phy :1;	// bit[17]	:: suspend USB3.0 SS PHY
	unsigned DelayPhyPwrChng :1;	// bit[18]	:: Delay Phy Power Change from P0 to P1/P2/P3
	unsigned DelayP1P2P3 :3;	// bit[21:19]
	unsigned DisRxDetU3RxDet :1;	// bit[22]	:: Disable Reecceiver Detection in U3/Rx.Det
	unsigned StartRxDeU3RxDet :1;	// bit[23]		:: Start Receiver Detection in U3/Rx.Detection
	unsigned request_p1p2p3 :1;	// bit[24]	:: Always Request P1/P2/P3 for U1/U2/U3
	unsigned u1u2exitfail_to_recov :1;	// bit[25]	:: U1U2 Exit Fail to Recovery
	unsigned PingEnhancementEn :1;	// Bit[26]	:: Ping Enhancement
	unsigned UxExitInPx :1;	// Bit[27]	:: UX Exit PX
	unsigned DisRxDetP3 :1;	// Bit[28]	:: Disabled receiver detection in P3
	unsigned U2SSInactP3ok :1;	// Bit[29]	::
	unsigned HstPrtCmpl :1;	// bit[30]	:: This feature tests the PIPE PHY compliance patterns without having to have a test fixture on the USB 3.0 cable.
	unsigned PHYSoftRst :1;	// bit[31]	:: usb3 phy soft reset
} __attribute__((__packed__)) USB3_REG_GUSB3PIPECTL_BitField;

typedef union {
	u32 data;	// reset value : 0x00260002
	USB3_REG_GUSB3PIPECTL_BitField b;
} USB3_REG_GUSB3PIPECTL_o;

//1 6.2.7 Global Event Buffer Registers
//3 6.2.7.1 GEVNTSIZ
typedef struct {
	unsigned event_siz :16;	// bit[15:0] : event buffer size in bytes (must be a multiple of 4)
	unsigned rsvd30_16 :15;	// bit[30:16] :
	unsigned event_int_mask :1;	// bit[31] : event interrupt mask (1 : prevent the interrupt from being generated)
} __attribute__((__packed__)) USB3_REG_GEVNTSIZ_BitField;

typedef union {
	u32 data;	// reset value : 0x00000000
	USB3_REG_GEVNTSIZ_BitField b;
} USB3_REG_GEVNTSIZ_o;

//1 6.2.9.6 Global Frame Length Adjustment Register (GFLADJ)
//3 6.2.7.1 GFLADJ : 0xC630
typedef struct {
	unsigned GFLADJ_30MHz :6;	// bit[5:0]	: This field indicates the value that is used for frame length adjustment instead of considering from the sideband input signal fladj_30mhz_reg.
	unsigned RSVD6 :1;	// bit[6]
	unsigned GFLADJ_30MHZ_REG_SEL :1;	// bit[7]	: This field selects whether to use the input signal fladj_30mhz_reg or the GFLADJ.GFLADJ_30MHZ to adjust the frame length for the SOF/ITP.
	unsigned GFLADJ_REFCLK_FLADJ :14;	// bit[21:8]	: This field indicates the frame length adjustment to be applied when SOF/ITP counter is running on the ref_clk.
	unsigned RSVD22 :1;	// bit[22]
	unsigned GFLADJ_REFCLK_LPM_SEL :1;	// bit[23]	: This bit enables the functionality of running SOF/ITP counters on the ref_clk.
	unsigned GFLADJ_REFCLK_240MHZ_DECR :7;	// bit[30:24]: This field indicates the decrement value that the controller applies for each ref_clk in order to derive a frame timer in terms of a 240-MHz clock.
	unsigned GFLADJ_REFCLK_240MHZDECR_PLS1 :1;	// bit[31]	: This bit enables the functionality of running SOF/ITP counters on the ref_clk.
} __attribute__((__packed__)) USB3_REG_GFLADJ_BitField;

typedef union {
	u32 data;	// reset value : 0x00000000
	USB3_REG_GFLADJ_BitField b;
} USB3_REG_FLADJ_o, *USB3_REG_FLADJ_p;

// GUCTL2 : 0xC19C (Add at 3.10a)
typedef struct {
	unsigned TxPingDuration :5;			// bit[4:0] Transmit Ping Maximum Duration
	unsigned RxPingDuration :6;			// bit[10:5] Receive Ping Maximum Duration
	unsigned DisableCFC :1;				// bit[11] Disable xHCI Errata Featrue Contiguous Frame ID Capability
	unsigned EnableEpCacheEvict :1;			// bit[12] Enable Evicting EP Cache after Flow control for Bulk EP
	unsigned RSVD13 :1;				// bit[13]
	unsigned Rst_actbitlater :1;			// bit[14] Enable clearing of the command active bit for the ENDXFER command
	unsigned RSVD14_9 :17;				// bit[31:15]
} __attribute__((__packed__)) USB3_REG_GUCTL2_BitField;

typedef union {
	u32 data;
	USB3_REG_GUCTL2_BitField b;
} USB3_REG_GUCTL2_o, *USB3_REG_GUCTL2_p;

// GUCTL3 : 0xC60C (Add at 1.70a or 3.13a)
typedef struct {
	unsigned PERIODIC_UF_THR_FSLS :7;			// bit[6:0] Periodic microseconds threshold
	unsigned INTR_EP_PING_ENABLE :1;			// bit[7] Interrupt EP PING Support
	unsigned BARB_BURST_ENABLE :1;				// bit[8] LSP BARB Burst Support
	unsigned SVC_OPP_PER_HS_SEP  :4;			// bit[12:9] Service opportunities for HS bulk endpoints in single endpoint mode
	unsigned reserved_13 :1;				// bit[13]
	unsigned reserved_14 :1;			// bit[14]
	unsigned reserved_15 :1;				// bit[15]
	unsigned USB20_RETRY_DISABLE :1;				// bit[16] USB2.0 Internal Retry Disable
	unsigned  reserved_31_17 :15;				// bit[31:17]
} __attribute__((__packed__)) USB3_REG_GUCTL3_BitField;

typedef union {
	u32 data;
	USB3_REG_GUCTL3_BitField b;
} USB3_REG_GUCTL3_o, *USB3_REG_GUCTL3_p;

typedef struct {
	/* bit[11:0] gen1 SKP frequency
	 * This field specifies the average number of symbols the link sends
	 * before inserting a SKP OS.
	 */
	unsigned gen1_skp_freq :12;
	/* bit[11:0] gen2 SKP frequency
	 * This field specifies the average number of blocks the link sends
	 * before inserting a SKP OS.
	 */
	unsigned gen2_skp_freq :8;
	/* bit[19:12] Programmable PM_ENTRY_TIMER
	 * This field specifies PM_ENTRY_TIMER value in us.
	 */
	unsigned pm_entry_timer_us :4;
	/* bit[23:20] Programmable PM_LC_TIMER
	 * This field specifies PM_LC_TIMER value in us.
	 */
	unsigned pm_lc_timer_us :3;
	/* bit[26:24] Enable us PM timers
	 * This field enables the use of PM_LC_TIMER and PM_ENTRY_TIMER in us.
	 */
	unsigned en_pm_timer_us :1;
	/* bit[27:27] Programmable U1 MIN RESIDENCY TIMER
	 * This field specifies U1 MIN RESIDENCY TIMER value in us. Set to 0 to
	 * disable the timer.
	 */
	unsigned u1_resid_timer_us :3;
	/* bit[31:28] Reserved_1 */
	unsigned Reserved_1 :1;
} __attribute__((__packed__)) USB3_REG_LSKIPFREQ_BitField;

typedef union {
	u32 data;
	USB3_REG_LSKIPFREQ_BitField b;
} USB3_REG_LSKIPFREQ_o, *USB3_REG_LSKIPFREQ_p;

/* 9001276244 B2  LFPS Handshake Interoperability Issues */
typedef struct {
	/* bit[7:0] gen1 U3_EXIT_RESP_RX_CLKS
	 *  - Locally initiated Ux exit: Minimum LFPS reception from
	 *  remote to consider Ux exit handshake  successful.
	 *  - Default value: 8us
	 *  - When PHY isn't in P3, the value in this field is
	 *  multiplied by 8 to get the U3_EXIT_RESP_RX_CLKS
	 */
	unsigned gen1_u3_exit_rsp_rx_clk :8;
	/* bit[15:8] gen1_u3_lfps_exit_rx_clk
	 */
	unsigned gen1_u3_lfps_exit_rx_clk :8;
	/* bit[23:16] gen2 U3_EXIT_RESP_RX_CLKS
	 *  - Locally initiated Ux exit: Minimum LFPS reception from
	 *  remote to consider Ux exit handshake  successful.
	 *  - Default value: 8us
	 *  - When PHY is not in P3, the value in this field is multiplied by 8
	 *  to get the U3_EXIT_RESP_RX_CLKS
	 */
	unsigned gen2_u2_exit_rsp_rx_clk :8;
	/* bit[31:24] gen2_u2_lfps_exit_rx_clk
	 */
	unsigned gen2_u2_lfps_exit_rx_clk :8;
} __attribute__((__packed__)) USB3_REG_LU3LFPSRXTIM_BitField;

typedef union {
	u32 data;
	USB3_REG_LU3LFPSRXTIM_BitField b;
} USB3_REG_LU3LFPSRXTIM_o, *USB3_REG_LU3LFPSRXTIM_p;

typedef struct {
	/* bit[4:0] Additional TX_TS1_count */
	unsigned tx_ts1_cnt :5;
	/* bit[6:5] Reserved */
	unsigned Reserved_1 :2;
	/* bit[7] Mask pipe reset
	 * If this bit is set, controller will block pipe_reset_n from going to
	 * PHY when DisRxDetU3RxDet=1
	 */
	unsigned mask_pipe_reset :1;
	/* bit[8] Gen1 loopback entry mode.
	 * If this bit is set, the controller will enter loopback in Gen1 after
	 * successfully detecting remote termination
	 */
	unsigned gen1_loopback_entry_mode :1;
	/* bit[9] Gen2 loopback entry mode.
	 * If this bit is set, the controller will enter loopback in Gen2 after
	 * successfully detecting remote termination.
	 */
	unsigned gen2_loopback_entry_mode :1;
	/* bit[10] Force Gen1 */
	unsigned force_gen1 :1;
	/* bit[11] en_reset_pipe_after_phy_mux.
	 * controller issues 3.1 PHY reset after DisRxDetU3RxDet is de-asserted
	 */
	unsigned en_reset_pipe_after_phy_mux :1;
	/* bit[12] Reserved */
	unsigned reserved_12 :1;
	/* bit[14:13] ring_buf_d_delay.
	 * This programs the counter for data valid signal in source ring buffer
	 */
	unsigned  ring_buf_d_delay :2;
	/* bit[15] en_us_hp_timer
	 * This bit enables programmable PENDING_HP_TIMER in us
	 */
	unsigned en_us_hp_timer :1;
	/* bit[20:16] pending_hp_timer_us.
	 * Programmable PENDING_HP_TIMER in us. This field is used when
	 * LUCTL[15] is set.
	 */
	unsigned  pending_hp_timer_us :5;
	/* bit[21] en_dpp_truncate
	 * This bit enables the logic for detecting recovery due to expiring
	 * pending_HP_timer and truncates DPP when neccessary to avoid recovery.
	 */
	unsigned en_dpp_truncate :1;
	/* bit[22] force_dpp_truncate
	 * Truncate Gen1 DPP if there is a pending LGOOD/LBAD and DPP is large.
	 * When this bit is set, the logic for detecting recovery due to
	 * expiring pending_HP_timer is not used.
	 */
	unsigned force_dpp_truncate :1;
	/* bit[23] DisRxDet_LTSSM_Timer_Ovrrd
	 * When DisRxDetU3RxDet is asserted in Polling or U1, the timeout
	 * expires immediately
	 */
	unsigned DisRxDet_LTSSM_Timer_Ovrrd :1;
	/* bit[27:24] delay_tx_gen1_dp
	 * When this field is set to a non-zero value, the link delays the
	 * transmission of a large Gen1 DP for 32 pipe_pclk cycles if it detects
	 * a header being received at the PIU.
	 */
	unsigned delay_tx_gen1_dp :4;
	/* bit[28] Support PHY P3.CPM and P4.
	 * When this bit is set, controller puts PHY into P3.CPM or P4 in
	 * certain states. Used only for Synopsys PHY
	 */
	unsigned support_p4 :1;
	/* bit[28] PHY P4 Power gate mode (PG) is enabled.
	 * Set this bit if PHY support PG mode in P4. Used only for Synopsys PHY
	 */
	unsigned support_p4_pg :1;
	/* bit[31:30] Reserved_3 */
	unsigned Reserved_3 :2;
} __attribute__((__packed__)) USB3_REG_LLUCTL_BitField;

typedef union {
	u32 data;
	USB3_REG_LLUCTL_BitField b;
} USB3_REG_LLUCTL_o, *USB3_REG_LLUCTL_p;

#endif /* DRIVER_USB_USB30_DRD_USB30_REG_H_ */
