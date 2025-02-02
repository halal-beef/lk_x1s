/*
 *
 * (C) Copyright 2018 SAMSUNG Electronics
 * Sanghyun Lee <sh425.lee@samsung.com>
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 *
 */

#ifndef __DWMCI__
#define __DWMCI__

#ifndef __iomem
#define __iomem
#endif

#include <dev/mmc.h>
#include <sys/types.h>
#include <lib/bio.h>
#include <lk/err.h>
#include <string.h>
#include <lk/reg.h>

#define dwmci_readl(host, reg) readl(host->ioaddr + reg)
#define dwmci_writel(host, value, reg) writel(value, host->ioaddr + reg)


#define DWMCI_CTRL		0x00	/* Control Register */
#define DWMCI_PWREN		0x04	/* Power Enable Register */
#define DWMCI_CLKDIV		0x08	/* Clock Divider Register */
#define DWMCI_CLKSRC		0x0C	/* Clock Source Register */
#define DWMCI_CLKENA		0x10	/* Clock Enable Register */
#define DWMCI_TMOUT		0x14	/* Time Out Register */
#define DWMCI_CTYPE		0x18	/* Card Type Register */
#define DWMCI_BLKSIZ		0x1C	/* Block Size Register */
#define DWMCI_BYTCNT		0x20	/* Byte Count Register */
#define DWMCI_INTMSK		0x24	/* Interrupt Mask Register */
#define DWMCI_CMDARG		0x28	/* Command Argument Register */
#define DWMCI_CMD		0x2C	/* Command Register */
#define DWMCI_RESP0		0x30	/* Response-0 Register */
#define DWMCI_RESP1		0x34	/* Response-1 Register */
#define DWMCI_RESP2		0x38	/* Response-2 Register */
#define DWMCI_RESP3		0x3C	/* Response-3 Register */
#define DWMCI_MINTSTS		0x40	/* Masked Interrupt Status Register */
#define DWMCI_RINTSTS		0x44	/* Raw Interrupt Status Register */
#define DWMCI_STATUS		0x48	/* Status Register */
#define DWMCI_FIFOTH		0x4C	/* FIFO Threshold Register */
#define DWMCI_CDETECT		0x50	/* Card Detect Register */
#define DWMCI_WRTPRT		0x54	/* Write Protect Register */
#define DWMCI_GPIO		0x58	/* GPIO Register */
#define DWMCI_TCBCNT		0x5C	/* Transferred CIU Card Byte count */
#define DWMCI_TBBCNT		0x60	/* Transferred host/DMA to/from BIU-FIFO Byte Count */
#define DWMCI_DEBNCE		0x64	/* Card detect Debounce Register */
#define DWMCI_USRID		0x68	/* User ID Register */
#define DWMCI_VERID		0x6C	/* Synopsys Version ID Register */
#define DWMCI_HCON		0x70	/* Hardware Configuration Register */
#define DWMCI_UHS_REG		0x74	/* UHS-1 Register */
#define DWMCI_RST_N		0x78	/* Hardware Reset Register */
#define DWMCI_BMOD		0x80    /* Bus Mode Register */
#define DWMCI_PLDMND		0x84    /* Poll Demand Register */
#define DWMCI_DBADDRL		0x88    /* Descriptor List Lower Base Address Register */
#define DWMCI_DBADDRU		0x8C    /* Descriptor List Upper Base Address Register */
#define DWMCI_IDSTS		0x90    /* Internal DMAC Status Register */
#define DWMCI_IDINTEN		0x94    /* Internal DMAC Interrupt Enable Register */
#define DWMCI_DSCADDRL		0x98    /* Current Host Lower Descriptor Address Register */
#define DWMCI_DSCADDRU		0x9C    /* Current Host Upper Descriptor Address Register */
#define DWMCI_BUFADDRL		0xA0    /* Current Host Lower Buffer Address Register */
#define DWMCI_BUFADDRU		0xA4    /* Current Host Upper Buffer Address Register */
#define DWMCI_CLKSEL		0xA8    /* Drv/Sample Clock Selection Register */
#define DWMCI_RESP_TAT		0xAC	/* Response Turn-Around Time Register */
#define DWMCI_MMC_FORCE_CLK_STOP	0xB0	/* Force Clock Stop Register */
#define DWMCI_AXI_BURST_LENGTH	0xB4	/* AXI Burst Length Setting Register */
#define DWMCI_CARDTHRCTL	0x100	/* Enables Card Read Threshold */
#define DWMCI_BACK_END_POWER	0x104	/* Back-End Power */
#define DWMCI_SECTOR_NUM_INC	0x1F8
#define DWMCI_EMMCP_BASE	0x1000	/* EMMCP base address offset Register */
#define DWMCI_FMPSECURITY	0x1010	/* FMP Security Control */
#define DWMCI_FMPSBEGIN0	0x1200	/* FMP Security Management Unit Begin Sector 0 */
#define DWMCI_FMPSEND0		0x1204	/* FMP Security Management Unit End Sector 0 */
#define DWMCI_FMPSCTRL0		0x120C	/* FMP Security Management Unit Control 0 */


/*****************************************************
* IP Version Control
 *****************************************************/
#define DWMCI_220A_FIFODAT	0x100	/* FIFO data read write */
#define DWMCI_240A_FIFODAT	0x200	/* FIFO data read write */

#define VERID_240A	0x240a
#define GET_VERID(x)	((x) & 0xFFFF)

/*****************************************************
 *  Control Register  Register
 *  DWMCI_CTRL - offset 0x00
 *****************************************************/

#define CTRL_RESET	(0x1<<0)	/* Reset DWC_mobile_storage controller */
#define FIFO_RESET	(0x1<<1)	/* Reset FIFO */
#define DMA_RESET	(0x1<<2)	/* Reset DMA interface */
#define INT_ENABLE	(0x1<<4)	/* Global interrupt enable/disable bit */
#define DMA_ENABLE	(0x1<<5)	/* DMA transfer mode enable/disable bit */
#define READ_WAIT	(0x1<<6)	/* For sending read-wait to SDIO cards */
#define SEND_IRQ_RESP	(0x1<<7)	/* Send auto IRQ response */
#define ABRT_READ_DATA	(0x1<<8)
#define SEND_CCSD	(0x1<<9)
#define SEND_AS_CCSD	(0x1<<10)
#define CEATA_INTSTAT	(0x1<<11)
#define CARD_VOLA	(0xF<<16)
#define CARD_VOLB	(0xF<<20)
#define ENABLE_OD_PULLUP	(0x1<<24)
#define ENABLE_IDMAC    (0x1<<25)

#define RESET_ALL	(CTRL_RESET |\
			FIFO_RESET |\
			DMA_RESET)


/*****************************************************
 *  Power Enable Register
 *  DWMCI_PWREN - offset 0x04
 *****************************************************/
#define POWER_ENABLE	(0x1<<0)

/*****************************************************
 *  Clock Divider Register
 *  DWMCI_CLKDIV - offset 0x08
 *****************************************************/
#define CLK_DIVIDER0	(0xFF<<0)
#define CLK_DIVIDER1	(0xFF<<8)
#define	CLK_DIVIDER2	(0xFF<<16)
#define CLK_DIVIDER3	(0xFF<<24)

/*****************************************************
 *  Clock Enable Register
 *  DWMCI_CLKENA - offset 0x10
 *****************************************************/
#define CLK_SDMMC_MAX	(48000000) /* 96Mhz. it SHOULDBE optimized */
#define CLK_ENABLE	(0x1<<0)
#define CLK_DISABLE	(0x0<<0)

/*****************************************************
 *  Timeout  Register
 *  DWMCI_TMOUT - offset 0x14
 *****************************************************/
#define RSP_TIMEOUT	(0xFF<<0)
#define DATA_TIMEOUT	(0xFFFFFF<<8)

/*****************************************************
 *  Card Type Register
 *  DWMCI_CTYPE - offset 0x18
 *****************************************************/
#define CARD_WIDTH14	(0xFFFF<<0)
#define CARD_WIDTH8	(0xFFFF<<16)

/*****************************************************
 *  Block Size Register
 *  DWMCI_BLKSIZ - offset 0x1C
 *****************************************************/
#define BLK_SIZ		(0xFFFF<<0)

/*****************************************************
 *  Interrupt Mask Register
 *  DWMCI_INTMSK - offset 0x24
 *****************************************************/
#define INT_MASK	(0xFFFF<<0)
#define	SDIO_INT_MASK	(0xFFFF<<16)
#define SDIO_INT_ENABLE (0x1<<16)

/* interrupt bits */
#define INTMSK_ALL	0xFFFFFFFF
#define INTMSK_CDETECT	(0x1<<0)
#define INTMSK_RE	(0x1<<1)
#define INTMSK_CDONE	(0x1<<2)
#define INTMSK_DTO	(0x1<<3)
#define INTMSK_TXDR     (0x1<<4)
#define INTMSK_RXDR     (0x1<<5)
#define INTMSK_RCRC     (0x1<<6)
#define INTMSK_DCRC     (0x1<<7)
#define INTMSK_RTO      (0x1<<8)
#define INTMSK_DRTO	(0x1<<9)
#define INTMSK_HTO      (0x1<<10)
#define INTMSK_FRUN     (0x1<<11)
#define INTMSK_HLE      (0x1<<12)
#define INTMSK_SBE      (0x1<<13)
#define INTMSK_ACD      (0x1<<14)
#define INTMSK_EBE	(0x1<<15)
#define INTMSK_DMA	(INTMSK_ACD|INTMSK_RXDR|INTMSK_TXDR)

#define INT_SRC_IDMAC   (0x0)
#define INT_SRC_MINT    (0x1)


/*****************************************************
 *  Command Register
 *  DWMCI_CMD - offset 0x2C
 *****************************************************/

#define CMD_RESP_EXP_BIT	(0x1<<6)
#define CMD_RESP_LENGTH_BIT	(0x1<<7)
#define CMD_CHECK_CRC_BIT	(0x1<<8)
#define CMD_DATA_EXP_BIT	(0x1<<9)
#define CMD_RW_BIT		(0x1<<10)
#define CMD_TRANSMODE_BIT	(0x1<<11)
#define CMD_SENT_AUTO_STOP_BIT	(0x1<<12)
#define CMD_WAIT_PRV_DAT_BIT	(0x1<<13)
#define CMD_ABRT_CMD_BIT	(0x1<<14)
#define CMD_SEND_INIT_BIT	(0x1<<15)
#define CMD_CARD_NUM_BITS	(0x1F<<16)
#define CMD_SEND_CLK_ONLY	(0x1<<21)
#define CMD_READ_CEATA		(0x1<<22)
#define CMD_CCS_EXPECTED	(0x1<<23)
#define CMD_USE_HOLD_REG	(0x1<<29)
#define CMD_STRT_BIT		(0x1<<31)
#define CMD_ONLY_CLK		(CMD_STRT_BIT | CMD_SEND_CLK_ONLY | \
						CMD_WAIT_PRV_DAT_BIT)

/*****************************************************
 *  Masked Interrupt Status Register
 *  DWMCI_MINTSTS - offset 0x40
 *****************************************************/
/*****************************************************
 *  Raw Interrupt Register
 *  DWMCI_RINTSTS - offset 0x44
 *****************************************************/
#define INT_STATUS	(0xFFFF<<0)
#define SDIO_INTR	(0xFFFF<<16)
#define DATA_ERR	(INTMSK_EBE|INTMSK_SBE|INTMSK_HLE|INTMSK_FRUN|\
					INTMSK_EBE|INTMSK_DCRC)
#define DATA_TOUT	(INTMSK_HTO|INTMSK_DRTO)
#define DATA_STATUS	(DATA_ERR|DATA_TOUT|INTMSK_RXDR|INTMSK_TXDR|INTMSK_DTO)
#define CMD_STATUS	(INTMSK_RTO|INTMSK_RCRC|INTMSK_CDONE|INTMSK_RE)
#define CMD_ERROR       (INTMSK_RCRC|INTMSK_RTO|INTMSK_RE)

/*****************************************************
 *  Status Register
 *  DWMCI_STATUS - offset 0x48
 *****************************************************/
#define FIFO_RXWTRMARK		(0x1<<0)
#define FIFO_TXWTRMARK		(0x1<<1)
#define FIFO_EMPTY		(0x1<<2)
#define FIFO_FULL		(0x1<<3)
#define CMD_FSMSTAT		(0xF<<4)
#define	DATA_3STATUS		(0x1<<8)
#define DATA_BUSY		(0x1<<9)
#define DATA_MCBUSY		(0x1<<10)
#define RSP_INDEX		(0x3F<<11)
#define FIFO_COUNT		(0x1FFF<<17)
#define	DMA_ACK			(0x1<<30)
#define	DMA_REQ			(0x1<<31)
#define FIFO_WIDTH		(0x4)
#define FIFO_DEPTH		(0x20)

/*Command FSM status */
#define FSM_IDLE		(0<<4)
#define FSM_SEND_INIT_SEQ	(1<<4)
#define FSM_TX_CMD_STARTBIT	(2<<4)
#define FSM_TX_CMD_TXBIT	(3<<4)
#define	FSM_TX_CMD_INDEX_ARG	(4<<4)
#define	FSM_TX_CMD_CRC7		(5<<4)
#define FSM_TX_CMD_ENDBIT	(6<<4)
#define	FSM_RX_RESP_STARTBIT	(7<<4)
#define FSM_RX_RESP_IRQRESP	(8<<4)
#define FSM_RX_RESP_TXBIT	(9<<4)
#define FSM_RX_RESP_CMDIDX	(10<<4)
#define FSM_RX_RESP_DATA	(11<<4)
#define FSM_RX_RESP_CRC7	(12<<4)
#define FSM_RX_RESP_ENDBIT	(13<<4)
#define FSM_CMD_PATHWAITNCC	(14<<4)
#define FSM_WAIT		(15<<4)

/*****************************************************
 *  FIFO Threshold Watermark Register
 *  DWMCI_FIFOTH - offset 0x4C
 *****************************************************/
#define TX_WMARK	(0xFFF<<0)
#define RX_WMARK	(0xFFF<<16)
#define MSIZE_MASK	(0x7<<28)
#define FIFOTH_ALL	(TX_WMARK | RX_WMARK | MSIZE_MASK)

/* DW DMA Mutiple Transaction Size */
#define MSIZE_1		(0<<28)
#define MSIZE_4		(1<<28)
#define MSIZE_8		(2<<28)
#define MSIZE_16	(3<<28)
#define MSIZE_32	(4<<28)
#define MSIZE_64	(5<<28)
#define MSIZE_128	(6<<28)
#define MSIZE_256	(7<<28)

/*****************************************************
 *  Bus Mode Register
 *  DWMCI_BMOD - offset 0x80
 *****************************************************/
#define BMOD_IDMAC_RESET        (0x1<<0)
#define BMOD_IDMAC_FB           (0x1<<1)
#define BMOD_IDMAC_ENABLE       (0x1<<7)

/*****************************************************
 *  Hardware Configuration  Register
 *  DWMCI_HCON - offset 0x70
 *****************************************************/
#define CARD_TYPE		(0x1<<0)
#define NUM_CARDS		(0x1F<<1)
#define H_BUS_TYPE		(0x1<<6)
#define H_DATA_WIDTH		(0x7<<7)
#define H_ADDR_WIDTH		(0x3F<<10)
#define DMA_INTERFACE		(0x3<<16)
#define	GE_DMA_DATA_WIDTH	(0x7<<18)
#define FIFO_RAM_INSIDE		(0x1<<21)
#define UMPLEMENT_HOLD_REG	(0x1<<22)
#define SET_CLK_FALSE_PATH	(0x1<<23)
#define NUM_CLK_DIVIDER		(0x3<<24)

/*****************************************************
 *  Hardware Configuration  Register
 *  DWMCI_IDSTS - offset 0x8c
 *****************************************************/
#define IDSTS_FSM               (0xf<<13)
#define IDSTS_EB                (0x7<<10)
#define IDSTS_AIS               (0x1<<9)
#define IDSTS_NIS               (0x1<<8)
#define IDSTS_CES               (0x1<<5)
#define IDSTS_DU                (0x1<<4)
#define IDSTS_FBE               (0x1<<2)
#define IDSTS_RI                (0x1<<1)
#define IDSTS_TI                (0x1<<0)

/*****************************************************
 *  AXI burst length
 *  DWMCI_AXI_BUSRT_LENGTH - offset 0xB4
 *****************************************************/
#define AXI_WRDMA_BURST_LEN(x)	((0xF&(x))<<16)
#define AXI_RDDMA_BURST_LEN(x)	((0xF&(x))<<0)
#define AXI_BURST_LEN(x)	(AXI_WRDMA_BURST_LEN(x) |\
				AXI_RDDMA_BURST_LEN(x))
#define AXI_SAMPLING_PATH_SEL	(1<<31)

/*****************************************************
 *  Sector number for XTS
 *  DWMCI_SECTOR_NUM_INC
 *****************************************************/
#define SECTOR_SIZE_MASK	(0x1FFF)
#define SECTOR_SIZE(x)		((SECTOR_SIZE_MASK&(x))<<0)

/*****************************************************
 *  Hardware Configuration  Register
 *  DWMCI_IDSTS - offset 0x8c
 *****************************************************/
#define MPSCTRL_SFR_PROT_ON		(0x1<<29)
#define MPSCTRL_SECURE_READ_BIT		(0x1<<7)
#define MPSCTRL_SECURE_WRITE_BIT	(0x1<<6)
#define MPSCTRL_NON_SECURE_READ_BIT	(0x1<<5)
#define MPSCTRL_NON_SECURE_WRITE_BIT	(0x1<<4)
#define MPSCTRL_USE_FUSE_KEY		(0x1<<3)
#define MPSCTRL_ECB_MODE		(0x1<<2)
#define MPSCTRL_ENCRYPTION		(0x1<<1)
#define MPSCTRL_VALID			(0x1<<0)

/*****************************************************
 *  Hardware Configuration  Register
 *  DWMCI_IDSTS - offset 0x8c
 *****************************************************/
#define CLKSEL_SELCLK_SAMPLE		(0x7<<0)
#define CLKSEL_SAMPLE_CLK_TUNING	(0x3<<6)
#define CLKSEL_SAMPLE_CLK_TUNING_1	(0x1<<6)
#define CLKSEL_SAMPLE_CLK_ALL		(CLKSEL_SELCLK_SAMPLE |\
					CLKSEL_SAMPLE_CLK_TUNING)

/*****************************************************
 *  DWMCI_MPSECURITY - offset 0x1010
 *****************************************************/
#define MMC_SFR_PROT_ON			(0x1<<29)

/*****************************************************
 *  DWMCI_MPSECURITY - offset 0x1010
 *****************************************************/
#define CHECK_CMD_STATE_START		0
#define CHECK_CMD_STATE_SET_START	1
#define CHECK_CMD_STATE_CLR_START	2
#define CHECK_CMD_STATE_CHK_FSM		3
#define CHECK_CMD_STATE_DONE		4


struct dwmci_idmac {
        u32     des0;
        u32     des1;
        u32     des2;
        u32     des3;
        u32     des4;
        u32     des5;
        u32     des6;
        u32     des7;

        u32     des8;
        u32     des9;
        u32     des10;
        u32     des11;
	u32	des12[4];
#define DWMCI_IDMAC_OWN         (1<<31)
#define DWMCI_IDMAC_ER          (1<<5)
#define DWMCI_IDMAC_CH          (1<<4)
#define DWMCI_IDMAC_FS          (1<<3)
#define DWMCI_IDMAC_LD          (1<<2)
#define DWMCI_IDMAC_DIC         (1<<1)
#define INTMSK_IDMAC_ALL        (0x337)
#define INTMSK_IDMAC_ERROR      (0x214)
};


struct dw_mci {
	char host_name[16];
	void __iomem *ioaddr;

	unsigned int host_caps;
	unsigned int version;
	unsigned int clock;
	unsigned int bus_hz;
	unsigned int phase_devide;
	int dev_index;
	int buswidth;
	unsigned int channel;
	struct mmc *mmc;
	unsigned int sdr;
	unsigned int ddr;
	unsigned int fifo_depth;
	unsigned int bus_clock;
	bool use_hold_reg;
	bool secure;
	unsigned int mps_secure;
	unsigned int min_clock;
	unsigned int max_clock;
	void *flush_start_addr;
	void *flush_end_addr;

	unsigned int (*get_clk)(void);
	void (*set_clk)(unsigned int freq);
	void (*sd_voltage_switch)(void);
	void (*cache_flush)(void *start, void *end);
};
int dwmci_init(struct mmc *mmc, int channel);
int dwmci_board_get_host(struct dw_mci *host, int channel);

#endif
