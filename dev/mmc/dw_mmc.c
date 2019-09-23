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

#include <dev/dw_mmc.h>
#include <platform/delay.h>

#define MAX_DIV 0xFF

//#define DEBUG_DWMCI
#ifdef DEBUG_DWMCI
#define dbg(x...)       printf(x)
#else
#define dbg(x...)       do { } while (0)
#endif

static struct dw_mci dw_mmc_host[MMC_MAX_CHANNEL];
static struct dwmci_idmac idmac_desc[0x10000];

/*
 * Set specific bits in SFR
 */
static void dwmci_set(struct dw_mci *host, unsigned int value, unsigned int addr)
{
	dwmci_writel(host, (dwmci_readl(host, addr) | value), addr);
}

/*
 * Clear specific bits in SFR
 */
static void dwmci_clr(struct dw_mci *host, unsigned int value, unsigned int addr)
{
	dwmci_writel(host, (dwmci_readl(host, addr) & ~(value)), addr);
}

static void dwmci_cache_flush(struct dw_mci *host)
{
	if (host->cache_flush)
		host->cache_flush();
}

static void dwmci_dumpregs_err(struct dw_mci *host)
{
	printf("dwmci: ============== REGISTER DUMP ==============\n");
	printf("dwmci: DWMCI_CTRL:      0x%08x\n",
		dwmci_readl(host, DWMCI_CTRL));
	printf("dwmci: DWMCI_PWREN:     0x%08x\n",
		dwmci_readl(host, DWMCI_PWREN));
	printf("dwmci: DWMCI_CLKDIV:    0x%08x\n",
		dwmci_readl(host, DWMCI_CLKDIV));
	printf("dwmci: DWMCI_CLKSRC:    0x%08x\n",
		dwmci_readl(host, DWMCI_CLKSRC));
	printf("dwmci: DWMCI_CLKENA:    0x%08x\n",
		dwmci_readl(host, DWMCI_CLKENA));
	printf("dwmci: DWMCI_TMOUT:     0x%08x\n",
		dwmci_readl(host, DWMCI_TMOUT));
	printf("dwmci: DWMCI_CTYPE:     0x%08x\n",
		dwmci_readl(host, DWMCI_CTYPE));
	printf("dwmci: DWMCI_BLKSIZ:    0x%08x\n",
		dwmci_readl(host, DWMCI_BLKSIZ));
	printf("dwmci: DWMCI_BYTCNT:    0x%08x\n",
		dwmci_readl(host, DWMCI_BYTCNT));
	printf("dwmci: DWMCI_INTMSK:    0x%08x\n",
		dwmci_readl(host, DWMCI_INTMSK));
	printf("dwmci: DWMCI_CMDARG:    0x%08x\n",
		dwmci_readl(host, DWMCI_CMDARG));
	printf("dwmci: DWMCI_CMD:       0x%08x\n",
		dwmci_readl(host, DWMCI_CMD));
	printf("dwmci: DWMCI_RESP0:       0x%08x\n",
		dwmci_readl(host, DWMCI_RESP0));
	printf("dwmci: DWMCI_RESP1:       0x%08x\n",
		dwmci_readl(host, DWMCI_RESP1));
	printf("dwmci: DWMCI_RESP2:       0x%08x\n",
		dwmci_readl(host, DWMCI_RESP2));
	printf("dwmci: DWMCI_RESP3:       0x%08x\n",
		dwmci_readl(host, DWMCI_RESP3));
	printf("dwmci: DWMCI_MINTSTS:   0x%08x\n",
		dwmci_readl(host, DWMCI_MINTSTS));
	printf("dwmci: DWMCI_RINTSTS:   0x%08x\n",
		dwmci_readl(host, DWMCI_RINTSTS));
	printf("dwmci: DWMCI_STATUS:    0x%08x\n",
		dwmci_readl(host, DWMCI_STATUS));
	printf("dwmci: DWMCI_FIFOTH:    0x%08x\n",
		dwmci_readl(host, DWMCI_FIFOTH));
	printf("dwmci: DWMCI_CDETECT:   0x%08x\n",
		dwmci_readl(host, DWMCI_CDETECT));
	printf("dwmci: DWMCI_WRTPRT:    0x%08x\n",
		dwmci_readl(host, DWMCI_WRTPRT));
	printf("dwmci: DWMCI_GPIO:      0x%08x\n",
		dwmci_readl(host, DWMCI_GPIO));
	printf("dwmci: DWMCI_TCBCNT:    0x%08x\n",
		dwmci_readl(host, DWMCI_TCBCNT));
	printf("dwmci: DWMCI_TBBCNT:    0x%08x\n",
		dwmci_readl(host, DWMCI_TBBCNT));
	printf("dwmci: DWMCI_DEBNCE:    0x%08x\n",
		dwmci_readl(host, DWMCI_DEBNCE));
	printf("dwmci: DWMCI_USRID:     0x%08x\n",
		dwmci_readl(host, DWMCI_USRID));
	printf("dwmci: DWMCI_VERID:     0x%08x\n",
		dwmci_readl(host, DWMCI_VERID));
	printf("dwmci: DWMCI_HCON:      0x%08x\n",
		dwmci_readl(host, DWMCI_HCON));
	printf("dwmci: DWMCI_UHS_REG:   0x%08x\n",
		dwmci_readl(host, DWMCI_UHS_REG));
	printf("dwmci: DWMCI_BMOD:      0x%08x\n",
		dwmci_readl(host, DWMCI_BMOD));
	printf("dwmci: DWMCI_PLDMND:   0x%08x\n",
		dwmci_readl(host, DWMCI_PLDMND));
	printf("dwmci: DWMCI_DBADDR:    0x%08x 0x%08x\n",
		dwmci_readl(host, DWMCI_DBADDRL), dwmci_readl(host, DWMCI_DBADDRU));
	printf("dwmci: DWMCI_IDSTS:     0x%08x\n",
		dwmci_readl(host, DWMCI_IDSTS));
	printf("dwmci: DWMCI_IDINTEN:   0x%08x\n",
		dwmci_readl(host, DWMCI_IDINTEN));
	printf("dwmci: DWMCI_DSCADDR:   0x%08x 0x%08x\n",
		dwmci_readl(host, DWMCI_DSCADDRL), dwmci_readl(host, DWMCI_DSCADDRU));
	printf("dwmci: DWMCI_BUFADDR:   0x%08x 0x%08x\n",
		dwmci_readl(host, DWMCI_BUFADDRL), dwmci_readl(host, DWMCI_BUFADDRU));
	printf("dwmci: DWMCI_IDINTEN:   0x%08x\n",
		dwmci_readl(host, DWMCI_IDINTEN));
	printf("dwmci: DWMCI_CLKSEL:   0x%08x\n",
		dwmci_readl(host, DWMCI_CLKSEL));
	printf("dwmci: DWMCI_RESP_TAT:   0x%08x\n",
		dwmci_readl(host, DWMCI_RESP_TAT));
	printf("dwmci: DWMCI_MMC_FORCE_CLK_STOP:   0x%08x\n",
		dwmci_readl(host, DWMCI_MMC_FORCE_CLK_STOP));
	printf("dwmci: DWMCI_AXI_BURST_LENGTH:   0x%08x\n",
		dwmci_readl(host, DWMCI_AXI_BURST_LENGTH));
	printf("dwmci: DWMCI_CARDTHRCTL:   0x%08x\n",
		dwmci_readl(host, DWMCI_CARDTHRCTL));
	printf("dwmci: DWMCI_BACK_END_POWER: 0x%08x\n",
	dwmci_readl(host, DWMCI_BACK_END_POWER));
	printf("dwmci: DWMCI_SECTOR_NUM_INC: 0x%08x\n",
		dwmci_readl(host, DWMCI_SECTOR_NUM_INC));
	printf("dwmci: DWMCI_CLOCKCON:  0x%08x\n",
		dwmci_readl(host, 0xA4));
	printf("dwmci: DWMCI_FIFODAT: 0x%08x\n",
		dwmci_readl(host, 0x200));
	printf("dwmci: ===========================================\n");
}

/*
 * Reset CTRL register only checking CTRL regiseter
 */
static bool dwmci_reset_ctrl(struct dw_mci *host, unsigned int ctrl)
{
	unsigned int timeout = 1000;

	/* Reset */
	dwmci_set(host, ctrl, DWMCI_CTRL);

	/* wait till resets clear */
	while (timeout--)
		if (!(dwmci_readl(host, DWMCI_CTRL) & ctrl))
			break;

	if (timeout == 0) {
		dbg("dwmci : TIMEOUT resetting block (ctrl %08x)\n", ctrl);
		return false;
	}

	dbg("dwmci : success resetting block (ctrl %08x)\n", ctrl);

	return true;
}

/*
 * Reset CTRL register with resetting others
 */
static bool dwmci_reset_ctrl_once(struct dw_mci *host, unsigned int ctrl)
{
	unsigned int timeout = 1000;
	unsigned int clksel_reg;

	/* set Rx timing to 0 */
	clksel_reg = dwmci_readl(host, DWMCI_CLKSEL);
	dwmci_writel(host, clksel_reg & ~CLKSEL_SAMPLE_CLK_ALL, DWMCI_CLKSEL);

	/* Reset */
	dwmci_set(host, ctrl, DWMCI_CTRL);

	/* All interrupt clear */
	dwmci_writel(host, INTMSK_ALL, DWMCI_RINTSTS);

	/* wait till resets clear */
	while (timeout--)
		if (!(dwmci_readl(host, DWMCI_CTRL) & ctrl))
			break;

	/* restore Rx timing */
	dwmci_writel(host, clksel_reg, DWMCI_CLKSEL);
	if (timeout == 0) {
		dbg("dwmci : TIMEOUT resetting block (ctrl %08x)\n", ctrl);
		return false;
	}
	dbg("dwmci : success resetting block (ctrl %08x)\n", ctrl);

	return true;
}

/*
 * Initialize fifo setting
 */
static void dwmci_fifo_init(struct dw_mci *host)
{
	unsigned int fifo_val, rx_wmark, tx_wmark, fifo_threshold;
	unsigned int msize;

	if (host->fifo_depth)
		host->fifo_depth = 0x20;

	fifo_val = dwmci_readl(host, DWMCI_FIFOTH);
	fifo_threshold = host->fifo_depth / 2;

	rx_wmark = (fifo_threshold - 1)<<16;
	tx_wmark = fifo_threshold;
	msize = MSIZE_8;

	rx_wmark &= RX_WMARK;
	tx_wmark &= TX_WMARK;
	msize &= MSIZE_MASK;

	fifo_val &= ~(FIFOTH_ALL);
	fifo_val |= (rx_wmark | tx_wmark | msize);

	dbg("dwmci : set fifoth value (fifoth %08x)\n", fifo_val);

	dwmci_writel(host, fifo_val, DWMCI_FIFOTH);
}


/*
 * Update changing clock and check changing clock success
 */
static int dwmci_check_clock_change(struct dw_mci *host)
{
	unsigned int loop_count = 1000;
	unsigned int retry = 10;

	/* Checking clock used by CMD register. */
	dwmci_writel(host, 0, DWMCI_CMD);
	dwmci_writel(host, CMD_ONLY_CLK, DWMCI_CMD);
	while (dwmci_readl(host, DWMCI_CMD) & CMD_STRT_BIT) {
		loop_count--;
		if (!loop_count) {
			if (retry) {
				dbg("dwmci : update clock retry\n");
				loop_count = 1000;
				retry--;
				dwmci_reset_ctrl(host, CTRL_RESET);
				dwmci_writel(host, CMD_ONLY_CLK, DWMCI_CMD);
			} else {
				dbg("dwmci : change clock fail\n");
				return ERR_TIMED_OUT;
			}
		}
	}
	dbg("dwmci : change clock success\n");

	return 0;
}

/*
 * Enable or Disable clock and check
 */
static int dwmci_control_clken(struct dw_mci *host, unsigned int ctrl)
{
	dwmci_writel(host, ctrl, DWMCI_CLKENA);

	return dwmci_check_clock_change(host);
}

/*
 * Calculate host clock divider value
 */
static unsigned int dwmci_get_clock_div(unsigned int board_clock, unsigned int target_clock)
{
	int i;

	/* Use board_clock orginally */
	if (target_clock >= board_clock) {
		dbg("dwmci : [board clock : %d / target clock : %d] present clock : %d\n", board_clock, target_clock, board_clock);
		return 0;
	}

	/* Calculate host clock div value */
	for (i = 1 ; i <= MAX_DIV ; i++) {
		if (target_clock >= (board_clock / (2 * i))) {
			dbg("dwmci : [board clock : %d / target clock : %d] present clock : %d\n", board_clock, target_clock, (board_clock / (2 * i)));
			return i;
		}
	}

	return MAX_DIV;
}

/*
 * Change clock frequncy to target clock
 */
static int dwmci_change_clock(struct mmc *mmc, unsigned int target_clock)
{
	unsigned int board_clock, div, err = NO_ERROR;
	struct dw_mci *host = (struct dw_mci *)mmc->host;

	/* If Board can change clock, change clock to platform function.*/
	if (host->set_clk) {
		host->set_clk(target_clock);
		dbg("dwmci : change clock success to set_clk\n");
		return NO_ERROR;
	}
	if (host->get_clk) {
		board_clock = host->get_clk();
		if (board_clock != host->bus_clock)
			host->bus_clock = board_clock;
	} else if (host->bus_clock) {
		board_clock = host->bus_clock;
	} else {
		dbg("dwmci : can't get board clock frequency\n");
		return ERR_GENERIC;
	}
	dwmci_control_clken(host, CLK_DISABLE);
	div = dwmci_get_clock_div(board_clock / host->phase_devide, target_clock);

	dwmci_writel(host, div, DWMCI_CLKDIV);
	dwmci_check_clock_change(host);
	err = dwmci_control_clken(host, CLK_ENABLE);

	return err;
}

/*
 * Change bus_width
 */
static void dwmci_change_bus_width(struct mmc *mmc)
{
	struct dw_mci *host = (struct dw_mci *)mmc->host;

	if (mmc->bus_width == MMC_BOOT_BUS_8BIT) {
		dbg("dwmci : MMC_BUS_WIDTH_8\n");
		dwmci_writel(host, (0x1<<16), DWMCI_CTYPE);
	} else if (mmc->bus_width == MMC_BOOT_BUS_4BIT) {
		dbg("dwmci : MMC_BUS_WIDTH_4\n");
		dwmci_writel(host, (0x1<<0), DWMCI_CTYPE);
	} else {
		dbg("dwmci : MMC_BUS_WIDTH_1\n");
		dwmci_writel(host, (0x0<<0), DWMCI_CTYPE);
	}
}

/*
 * Change bus_mode
 */
static void dwmci_change_bus_mode(struct mmc *mmc)
{
	struct dw_mci *host = (struct dw_mci *)mmc->host;

	if (mmc->bus_mode == MMC_BOOT_BUS_DDR) {
		dbg("dwmci : MMC_BUS_DDR\n");
		dwmci_set(host, (0x1<<16), DWMCI_UHS_REG);
	} else {
		dbg("dwmci : MMC_BUS_SDR\n");
		dwmci_clr(host, ~(0x1<<16), DWMCI_UHS_REG);
	}
}

/*
 * Change bus_mode, bus_width, clksel, clock
 */
static int dwmci_set_ios(struct mmc *mmc)
{
	struct dw_mci *host = (struct dw_mci *)mmc->host;
	int err = NO_ERROR;
	int reg = 0;

	if (mmc->clock < host->min_clock)
		mmc->clock = host->min_clock;
	if (mmc->clock > host->max_clock)
		mmc->clock = host->max_clock;

	err = dwmci_change_clock(mmc, mmc->clock);
	dwmci_change_bus_width(mmc);
	dwmci_change_bus_mode(mmc);

	if (mmc->bus_mode == MMC_BOOT_BUS_DDR)
		dwmci_writel(host, host->ddr, DWMCI_CLKSEL);
	else {
		reg = host->sdr;
		if (mmc->clock <= 400000)
			reg |= (0x7<<16);
		dwmci_writel(host, reg, DWMCI_CLKSEL);
	}

	return err;
}

/*
 * Check data line is busy.
 */
static int dwmci_check_data_busy(struct dw_mci *host, struct mmc_cmd *cmd)
{
	unsigned int timeout;

	if (cmd != NULL && cmd->cmdidx == CMD13_SEND_STATUS)
		return 0;

	for (timeout = 0; timeout < 1000; timeout++) {
		if (!(dwmci_readl(host, DWMCI_STATUS) & DATA_BUSY)) {
			dwmci_writel(host, INTMSK_ALL, DWMCI_RINTSTS);
			return NO_ERROR;
		}
		mdelay(1);
	}

	printf("dwmci : TIMEOUT data busy\n");

	return ERR_TIMED_OUT;
}

/*
 * Set descriptor
 */
static void dwmci_set_idma_desc(struct dwmci_idmac *desc,
		u64 control, u64 buffer_size, u64 buffer_addr)
{
	((struct dwmci_idmac *)(desc))->des0 = control;
	((struct dwmci_idmac *)(desc))->des1 = 0;
	((struct dwmci_idmac *)(desc))->des2 = buffer_size;
	((struct dwmci_idmac *)(desc))->des3 = 0;
	((struct dwmci_idmac *)(desc))->des4 = buffer_addr;
	((struct dwmci_idmac *)(desc))->des5 = (buffer_addr >> 32);
	((struct dwmci_idmac *)(desc))->des6 = (u64)desc +
					sizeof(struct dwmci_idmac);
	((struct dwmci_idmac *)(desc))->des7 = (((u64)desc + sizeof(struct dwmci_idmac)) >> 32);
	((struct dwmci_idmac *)(desc))->des8 = 0;
	((struct dwmci_idmac *)(desc))->des9 = 0;
	((struct dwmci_idmac *)(desc))->des10 = 0;
	((struct dwmci_idmac *)(desc))->des11 = 0;
	((struct dwmci_idmac *)(desc))->des12[0] = 0;
	((struct dwmci_idmac *)(desc))->des12[1] = 0;
	((struct dwmci_idmac *)(desc))->des12[2] = 0;
	((struct dwmci_idmac *)(desc))->des12[3] = 0;
}

/*
 * Prepare data transfer to set DMA
 */
static void dwmci_prepare_data(struct dw_mci *host, struct mmc_data *data)
{
	struct dwmci_idmac *cur_idmac;
	void *buffer_addr;
	unsigned int flags, i, data_cnt, send_bytes, data_bytes, reg;

	if (data->flags == MMC_DATA_READ)
		buffer_addr = (void *)data->dest;
	else
		buffer_addr = (void *)data->src;

	data_cnt = data->block_cnt;
	data_bytes = data->block_size * data->block_cnt;
	reg = dwmci_readl(host, DWMCI_FIFOTH);
	reg &= ~(MSIZE_MASK);
	if (data->block_size == 8)
		reg |= MSIZE_1;
	else
		reg |= MSIZE_8;

	dwmci_writel(host, reg, DWMCI_FIFOTH);
	dwmci_reset_ctrl(host, FIFO_RESET);
	dwmci_set(host, BMOD_IDMAC_RESET, DWMCI_BMOD);
	cur_idmac = idmac_desc;
	for (i = 0;; i++) {
		flags = DWMCI_IDMAC_OWN | DWMCI_IDMAC_CH;
		if (i == 0)
			flags |= DWMCI_IDMAC_FS;
		if (data_cnt <= 8) {
			flags |= DWMCI_IDMAC_LD;
			send_bytes = data->block_size * data_cnt;
			dwmci_set_idma_desc(cur_idmac, flags, send_bytes,
				(u64)buffer_addr + (u64)(i * 0x1000));
			break;
		}
		send_bytes = data->block_size * 8;
		dwmci_set_idma_desc(cur_idmac, flags, send_bytes,
				(u64)buffer_addr + (u64)(i * 0x1000));
		data_cnt -= 8;
		cur_idmac++;
	}
	dwmci_cache_flush(host);

	dwmci_writel(host, (unsigned int)((u64)idmac_desc & 0xFFFFFFFF), DWMCI_DBADDRL);
	dwmci_writel(host, (unsigned int)((u64)idmac_desc >> 32), DWMCI_DBADDRU);
	dwmci_clr(host, SEND_AS_CCSD, DWMCI_CTRL);

	dwmci_set(host, ENABLE_IDMAC | DMA_ENABLE, DWMCI_CTRL);
	dwmci_set(host, BMOD_IDMAC_ENABLE | BMOD_IDMAC_FB, DWMCI_BMOD);

	dwmci_writel(host, data->block_size, DWMCI_BLKSIZ);
	dwmci_writel(host, data_bytes, DWMCI_BYTCNT);
}

/*
 * Ready CMD register flag
 */
static int dwmci_ready_cmd(struct dw_mci *host, struct mmc_cmd *cmd, unsigned int *flag)
{
	dwmci_writel(host, cmd->argument, DWMCI_CMDARG);

	/* This is out of SD spec */
	if ((cmd->resp_type & MMC_RSP_136) && (cmd->resp_type & MMC_RSP_BUSY)) {
		printf("dwmci : RESPONSE TYPE is out of spec\n");
		return ERR_GENERIC;
	}

	if (cmd->resp_type & MMC_RSP_CRC)
		*flag |= CMD_CHECK_CRC_BIT;

	if (cmd->resp_type & MMC_RSP_PRESENT) {
		*flag |= CMD_RESP_EXP_BIT;
		if (cmd->resp_type & MMC_RSP_136)
			*flag |= CMD_RESP_LENGTH_BIT;
	}

	*flag |= (cmd->cmdidx | CMD_STRT_BIT |
		CMD_USE_HOLD_REG | CMD_WAIT_PRV_DAT_BIT);

	if (cmd->data) {
		*flag |= CMD_DATA_EXP_BIT;
		if (cmd->data->flags & MMC_DATA_WRITE)
			*flag |= CMD_RW_BIT;
	}

	return NO_ERROR;
}

/*
 * Set flag for aborting stop command.
 */
static void dwmci_ready_abort_cmd(unsigned int *flag)
{
	*flag &= ~CMD_WAIT_PRV_DAT_BIT;
	*flag |= CMD_ABRT_CMD_BIT;
}

static int dwmci_check_command_method(struct dw_mci *host, unsigned int flag,
		unsigned int check_status, int *timeout, unsigned int *mask)
{
	while (*timeout > 0) {
		if (check_status == 0) {
			if (!(dwmci_readl(host, DWMCI_CMD) & CMD_STRT_BIT) &&
				!(dwmci_readl(host, DWMCI_RINTSTS) & INTMSK_CDONE)) {
				dwmci_writel(host, flag, DWMCI_CMD);
				break;
			}
		} else if (check_status == 1) {
			if (!(dwmci_readl(host, DWMCI_CMD) & CMD_STRT_BIT))
				break;
		} else if (check_status == 2) {
			if (!(dwmci_readl(host, DWMCI_STATUS) & CMD_FSMSTAT))
				break;
		} else {
			*mask = dwmci_readl(host, DWMCI_RINTSTS);
			if (*mask & INTMSK_CDONE) {
				break;
			}
		}
		(*timeout)--;
	}
	if (*timeout <= 0) {
		if (check_status == 0)
			printf("dwmci : previous cmd start bit is not clear\n");
		else if (check_status == 1)
			printf("dwmci : present cmd start bit is not clear\n");
		else if (check_status == 2)
			printf("dwmci : FSM STATE bits is not clear\n");
		else
			printf("dwmci : failed to get CMD_DONE\n");

		dwmci_reset_ctrl(host, CTRL_RESET);
		return ERR_TIMED_OUT;
	}

	return NO_ERROR;
}


/*
 * Start sending command and check error
 */
static int dwmci_start_cmd(struct dw_mci *host, struct mmc_cmd *cmd, unsigned int flag)
{
	unsigned int mask = 0;
	int timeout = 0x200000;

	dbg("dwmci : cmdidx : %d, cmd_arg : %08x, flag : %08x\n", cmd->cmdidx, cmd->argument, flag);
	if (dwmci_check_command_method(host, flag, 0, &timeout, &mask))
		return ERR_TIMED_OUT;
	if (dwmci_check_command_method(host, flag, 1, &timeout, &mask))
		return ERR_TIMED_OUT;
	if (dwmci_check_command_method(host, flag, 2, &timeout, &mask))
		return ERR_TIMED_OUT;
	if (dwmci_check_command_method(host, flag, 3, &timeout, &mask))
		return ERR_TIMED_OUT;

	if (!cmd->data)
		dwmci_writel(host, mask, DWMCI_RINTSTS);

	if (mask & INTMSK_RTO) {
		printf("dwmci : Response Timed Out\n");
		return ERR_TIMED_OUT;
	} else if (mask & INTMSK_RE) {
		printf("dwmci : Response Error Out\n");
		return ERR_IO;
	} else if (mask & INTMSK_RCRC) {
		printf("dwmci : CRC Error Out\n");
		return ERR_CRC_FAIL;
	}

	return NO_ERROR;
}

/*
 * Get response and Parse to command
 */
static void dwmci_response_parse(struct dw_mci *host, struct mmc_cmd *cmd)
{
	unsigned int timeout = 1000;

	while ((dwmci_readl(host, DWMCI_RINTSTS) & INTMSK_CDONE)
			&& timeout)
		timeout--;
	if (cmd->resp_type & MMC_RSP_PRESENT) {
		if (cmd->resp_type & MMC_RSP_136) {
			cmd->response[0] = dwmci_readl(host, DWMCI_RESP3);
			cmd->response[1] = dwmci_readl(host, DWMCI_RESP2);
			cmd->response[2] = dwmci_readl(host, DWMCI_RESP1);
			cmd->response[3] = dwmci_readl(host, DWMCI_RESP0);
			dbg("raw_reponse [0] : %08x\n", cmd->response[3]);
			dbg("raw_reponse [1] : %08x\n", cmd->response[2]);
			dbg("raw_reponse [2] : %08x\n", cmd->response[1]);
			dbg("raw_reponse [3] : %08x\n", cmd->response[0]);
		} else {
			cmd->response[0] = dwmci_readl(host, DWMCI_RESP0);
			dbg("raw_reponse [0] : %08x\n", cmd->response[0]);
		}
	}
}

/*
 * Reset and disable dma/fifo
 */
static void dwmci_end_data(struct dw_mci *host)
{
	u32 reg;

	dwmci_clr(host, (DMA_ENABLE|ENABLE_IDMAC), DWMCI_CTRL);

	dwmci_reset_ctrl(host, DMA_RESET);
	dwmci_reset_ctrl(host, FIFO_RESET);

	reg = dwmci_readl(host, DWMCI_BMOD);
	reg &= ~(BMOD_IDMAC_FB | BMOD_IDMAC_ENABLE);
	reg |= BMOD_IDMAC_RESET;
	dwmci_writel(host, reg, DWMCI_BMOD);
}

/*
 * Transfer data and check error
 */
static int dwmci_data_transfer(struct dw_mci *host)
{
	unsigned int mask;
	unsigned int timeout = 100000;

	while (timeout--) {
		mask = dwmci_readl(host, DWMCI_RINTSTS);
		if (mask & (DATA_ERR | DATA_TOUT)) {
			dwmci_end_data(host);
			dwmci_writel(host, 0x0, DWMCI_IDINTEN);
			if (mask & DATA_TOUT) {
				printf("dwmci : data transfer failed : DATA TIMEOUT\n");
				return ERR_TIMED_OUT;
			}
			if (mask & DATA_ERR) {
				printf("dwmci : data transfer failed : DATA ERROR\n");
				return ERR_GENERIC;
			}
		} else if (mask & INTMSK_DTO) {
			dbg("dwmci : data transfer done\n");
			dwmci_end_data(host);
			dwmci_writel(host, 0x0, DWMCI_IDINTEN);
			dwmci_cache_flush(host);
			break;
		}
	}

	if (timeout == 0) {
		printf("dwmci : data transfer sw timeout\n");
		return ERR_TIMED_OUT;
	}

	return NO_ERROR;
}

/*
 * Process command about all sequence include setting and transferring data.
 */
static int dwmci_send_command(struct mmc *mmc, struct mmc_cmd *cmd)
{
	struct dw_mci *host = (struct dw_mci *)mmc->host;
	struct mmc_data *data = cmd->data;
	int err;
	unsigned int flag = 0;

	if (mmc == NULL || cmd == NULL)
		return ERR_GENERIC;

	err = dwmci_check_data_busy(host, cmd);
	if (err)
		goto err;

	if (data)
		dwmci_prepare_data(host, data);

	err = dwmci_ready_cmd(host, cmd, &flag);
	if (err)
		goto err;

	if (&(mmc->abort_cmd) == cmd)
		dwmci_ready_abort_cmd(&flag);

	err = dwmci_start_cmd(host, cmd, flag);
	if (err)
		goto err;

	dwmci_response_parse(host, cmd);
	if (data)
		err = dwmci_data_transfer(host);
	if (err)
		goto err;

	return NO_ERROR;


err:
#ifdef DEBUG_DWMCI
	if (cmd->cmdidx != CMD12_STOP_TRANSMISSION &&
		cmd->cmdidx != CMD19_SEND_TUNING_BLOCK)
	{
		dwmci_dumpregs_err(host);
	}
#endif
	if (data) {
		dwmci_end_data(host);
		if (mmc->abort_cmd.cmdidx) {
			err = dwmci_send_command(mmc, &(mmc->abort_cmd));
			if (err)
				dbg("dwmci : failed stop abort command\n");
		}
	}

	return err;
}

/*
 * Initialize host driver
 */
static void dwmci_host_init(struct mmc *mmc)
{
	struct dw_mci *host = (struct dw_mci *)mmc->host;
	int err;

	if (host->secure) {
		dwmci_writel(host, 0, DWMCI_FMPSBEGIN0);
		dwmci_writel(host, 0xFFFFFFFF, DWMCI_FMPSEND0);
		dwmci_writel(host, MPSCTRL_SECURE_READ_BIT | MPSCTRL_SECURE_WRITE_BIT |
				   MPSCTRL_NON_SECURE_READ_BIT | MPSCTRL_NON_SECURE_WRITE_BIT |
				   MPSCTRL_VALID, DWMCI_FMPSCTRL0);
	}

	if (host->mps_secure)
		dwmci_writel(host, host->mps_secure, DWMCI_FMPSECURITY);

	host->version = dwmci_readl(host, DWMCI_VERID) & 0xFFFF;
	dbg("dwmci : version : %08x\n", host->version);

	dwmci_writel(host, 1<<0, DWMCI_PWREN);
	err = dwmci_check_data_busy(host, NULL);
	if (err)
		dbg("Data busy\n");

	/* CTRL reset */
	dwmci_reset_ctrl_once(host, RESET_ALL);

	/* FIFO initialize */
	dwmci_fifo_init(host);

	/* Interrupt initializ */
	dwmci_writel(host, INTMSK_ALL, DWMCI_RINTSTS);
	dwmci_writel(host, 0, DWMCI_INTMSK);

	host->host_caps = 0;

	/* Clock setting initialize to 400khz */
	dwmci_writel(host, host->sdr, DWMCI_CLKSEL);
	dwmci_change_clock(mmc, 400000);

	/* setting min/max clock frequency */
	host->min_clock = 400000;
	host->max_clock = 50000000;

	/* Auto Stop register setting */
	dwmci_set(host, SEND_AS_CCSD, DWMCI_CTRL);

	/* set debounce filter value*/
	dwmci_writel(host, 0xfffff, DWMCI_DEBNCE);

	/* clear card type. set 1bit mode */
	dwmci_writel(host, 0x0, DWMCI_CTYPE);

	/* set bus mode register for IDMAC */
	dwmci_writel(host, BMOD_IDMAC_RESET, DWMCI_BMOD);

	/* disable all interrupt source of IDMAC */
	dwmci_writel(host, 0x0, DWMCI_IDINTEN);

	/* set max timeout */
	dwmci_writel(host, 0xffffffff, DWMCI_TMOUT);
}

/*
 * Reset host
 */
static void dwmci_sw_reset(struct mmc *mmc)
{
	struct dw_mci *host = (struct dw_mci *)mmc->host;
	int err;

	err = dwmci_check_data_busy(host, NULL);
	if (err)
		dbg("Data busy\n");

	dwmci_reset_ctrl_once(host, RESET_ALL);
	dwmci_fifo_init(host);
	dwmci_set_ios(mmc);
}

/*
 * Support SD voltage switching sequence
 */
static int dwmci_voltage_switch(struct mmc *mmc)
{
	struct dw_mci *host = (struct dw_mci *)mmc->host;
	unsigned char reg;

	mdelay(1);
	if (!(dwmci_readl(host, DWMCI_STATUS) & (1 << 9)))
		return ERR_GENERIC;
	dwmci_writel(host, (0x0 << 0), DWMCI_CLKENA);
	host->max_clock = 200000000;
	mdelay(10);
	host->sd_voltage_switch();
	reg = dwmci_readl(host, DWMCI_UHS_REG) | 0x1;
	dwmci_writel(host, reg, DWMCI_UHS_REG);

	mdelay(10);
	dwmci_writel(host, (0x1 << 0), DWMCI_CLKENA);
	mdelay(1);
	if (dwmci_readl(host, DWMCI_STATUS) & DATA_BUSY)
		return ERR_GENERIC;

	return NO_ERROR;
}

/*
 * Change CLKSEL
 */
static int dwmci_change_clksel(struct mmc *mmc, unsigned int pass_index)
{
	struct dw_mci *host = (struct dw_mci *)mmc->host;
	unsigned int original = dwmci_readl(host, DWMCI_CLKSEL);
	unsigned int change;
	unsigned int reg;

	if (pass_index > 15)
		return original;

	change = original & ~CLKSEL_SELCLK_SAMPLE;
	change |= (pass_index / 2);

	/* CLKSEL 7 need AXI_SAMPLING_PATH_SEL */
	reg = dwmci_readl(host, DWMCI_AXI_BURST_LENGTH);
	if ((pass_index / 2) >= 6)
		reg |= AXI_SAMPLING_PATH_SEL;
	else
		reg &= ~AXI_SAMPLING_PATH_SEL;
	dwmci_writel(host, reg, DWMCI_AXI_BURST_LENGTH);

	/* TUNING BIT check and write register */
	if (pass_index % 2 == 0)
		change &= ~CLKSEL_SAMPLE_CLK_TUNING;
	else
		change |= CLKSEL_SAMPLE_CLK_TUNING_1;
	dwmci_writel(host, change, DWMCI_CLKSEL);

	return original;
}

/*
 * Connect Host Driver and MMC Driver
 */
int dwmci_init(struct mmc *mmc, int channel)
{
	int err = NO_ERROR;
	struct dw_mci *host = &dw_mmc_host[channel];

	/* base initialize */
	host->ioaddr = 0;
	host->sdr = 0;
	host->ddr = 0;
	host->phase_devide = 0;
	host->secure = 0;
	host->mps_secure = 0;
	host->set_clk = 0;
	host->get_clk = 0;
	host->bus_clock = 0;
	host->sd_voltage_switch = 0;
	host->fifo_depth = 0;

	/* get host data from platform */
	err = dwmci_board_get_host(host, channel);
	if (err != NO_ERROR) {
		printf("No information about MMC%d channel\n", channel);
		return err;
	}
	mmc->exist = 1;
	mmc->host = (void *)host;
	mmc->send_command = dwmci_send_command;
	mmc->host_init = dwmci_host_init;
	mmc->change_clock = dwmci_change_clock;
	mmc->change_bus_width = dwmci_change_bus_width;
	mmc->change_mode = dwmci_change_bus_mode;
	mmc->set_ios = dwmci_set_ios;
	mmc->voltage_switch = dwmci_voltage_switch;
	mmc->reset = dwmci_sw_reset;
	mmc->change_clksel = dwmci_change_clksel;

	return err;
}
