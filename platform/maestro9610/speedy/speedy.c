/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <debug.h>
#include <reg.h>
#include <sys/types.h>
#include <platform/delay.h>
#include <platform/speedy.h>

static void speedy_set_enable(void)
{
	u32 speedy_ctl = readl(SPEEDY_BASE + SPEEDY_CTRL);

	speedy_ctl |= SPEEDY_ENABLE;
	writel(speedy_ctl, SPEEDY_BASE + SPEEDY_CTRL);
}

static void speedy_fifo_reset(void)
{
	u32 speedy_fifo_ctl = readl(SPEEDY_BASE + SPEEDY_FIFO_CTRL);

	speedy_fifo_ctl |= SPEEDY_FIFO_RESET;
	writel(speedy_fifo_ctl, SPEEDY_BASE + SPEEDY_FIFO_CTRL);
	/* delay for speedy fifo reset */
	u_delay(10);
}

static void speedy_set_cmd(int direction, u16 address, int random, int burst_length)
{
	u32 speedy_fifo_ctl = 0;
	u32 speedy_int_en = 0;
	u32 speedy_command = 0;

	speedy_fifo_reset();
	speedy_fifo_ctl = readl(SPEEDY_BASE + SPEEDY_FIFO_CTRL);

	speedy_command |= SPEEDY_ADDRESS(address);

	switch (random) {
	case ACCESS_RANDOM:
		speedy_command |= SPEEDY_ACCESS_RANDOM;
		speedy_fifo_ctl |= (SPEEDY_RX_TRIGGER_LEVEL(1) |
				SPEEDY_TX_TRIGGER_LEVEL(1));
		break;
	case ACCESS_BURST:
		speedy_command |= (SPEEDY_ACCESS_BURST | SPEEDY_BURST_INCR |
				   SPEEDY_BURST_LENGTH(burst_length-1));
		speedy_fifo_ctl |= (
			SPEEDY_RX_TRIGGER_LEVEL(burst_length) |
			SPEEDY_TX_TRIGGER_LEVEL(1)
		);
		break;

	default:
		printf("[SPEEDY] Can not support this ACCESS mode\n");
		break;
	}

	/* configure speedy fifio trigger level */
	writel(speedy_fifo_ctl, SPEEDY_BASE + SPEEDY_FIFO_CTRL);

	speedy_int_en |= (SPEEDY_TIMEOUT_CMD_EN | SPEEDY_TIMEOUT_STANDBY_EN |
			SPEEDY_TIMEOUT_DATA_EN);

	switch (direction) {
	case DIRECTION_READ:
		speedy_command |= SPEEDY_DIRECTION_READ;
		speedy_int_en |= (SPEEDY_FIFO_RX_ALMOST_FULL_EN |
				SPEEDY_RX_FIFO_INT_TRAILER_EN |
				SPEEDY_RX_MODEBIT_ERR_EN |
				SPEEDY_RX_GLITCH_ERR_EN |
				SPEEDY_RX_ENDBIT_ERR_EN |
				SPEEDY_REMOTE_RESET_REQ_EN);
		break;

	case DIRECTION_WRITE:
		speedy_command |= SPEEDY_DIRECTION_WRITE;
		speedy_int_en |= (SPEEDY_TRANSFER_DONE_EN |
				SPEEDY_FIFO_TX_ALMOST_EMPTY_EN |
				SPEEDY_TX_LINE_BUSY_ERR_EN |
				SPEEDY_TX_STOPBIT_ERR_EN |
				SPEEDY_REMOTE_RESET_REQ_EN);
		break;

	default:
		printf("[SPEEDY] Can not support this DIRECTION mode\n");
		break;
	}

	/* clear speedy interrupt status */
	writel(0xFFFFFFFF, SPEEDY_BASE + SPEEDY_INT_STATUS);

	/* speedy interrupt enable */
	writel(speedy_int_en, SPEEDY_BASE + SPEEDY_INT_ENABLE);

	/* configure speedy command */
	writel(speedy_command, SPEEDY_BASE + SPEEDY_CMD);
}

static int speedy_wait_transfer_done(void)
{
	u32 speedy_int_status;
	unsigned long timeout = 1000;
	int ret = 0;

	while (timeout-- > 0) {
		speedy_int_status = readl(SPEEDY_BASE + SPEEDY_INT_STATUS);

		if (speedy_int_status & SPEEDY_TRANSFER_DONE) {
			ret = 0;
			break;
		}
	}

	if (timeout == 0)
		ret = -1;

	writel(0xFFFFFFFF, SPEEDY_BASE + SPEEDY_INT_STATUS);

	return ret;
}

static void speedy_swreset(void)
{
	u32 speedy_ctl = readl(SPEEDY_BASE + SPEEDY_CTRL);

	speedy_ctl |= SPEEDY_SW_RST;
	writel(speedy_ctl, SPEEDY_BASE + SPEEDY_CTRL);
	/* delay for speedy sw_rst */
	u_delay(10);
}

void speedy_init(void)
{
	/* clear speedy interrupt status */
	writel(0xFFFFFFFF, SPEEDY_BASE + SPEEDY_INT_STATUS);

	/* reset speedy ctrl SFR. It may be used by bootloader */
	speedy_swreset();
	speedy_set_enable();
}

void speedy_read(unsigned char slave, unsigned char addr, unsigned char *data)
{
	int result;
	u16 address = addr + ((slave & 0xF) << 8);

	speedy_set_cmd(DIRECTION_READ, address, ACCESS_RANDOM, 1);

	result = speedy_wait_transfer_done();

	if (result == 0)
		*data = (unsigned char)readl(SPEEDY_BASE + SPEEDY_RX_DATA);
	else
		printf("[SPEEDY] timeout at read\n");
}

void speedy_read_burst(unsigned char slave, unsigned char addr,
				unsigned char *data, unsigned char count)
{
	int result;
	u16 address = addr + ((slave & 0xF) << 8);
	int i;

	speedy_set_cmd(DIRECTION_READ, address, ACCESS_BURST, count);

	result = speedy_wait_transfer_done();

	if (result == 0) {
		for(i = 0; i < count; i++)
			data[i] = (unsigned char)readl(SPEEDY_BASE + SPEEDY_RX_DATA);
	}
	else
		printf("[SPEEDY] timeout at burst_read\n");
}

void speedy_write(unsigned char slave, unsigned char addr, unsigned char data)
{
	int result;
	u16 address = addr + ((slave & 0xF) << 8);

	speedy_set_cmd(DIRECTION_WRITE, address, ACCESS_RANDOM, 1);
	writel(data, SPEEDY_BASE + SPEEDY_TX_DATA);

	result = speedy_wait_transfer_done();

	if (result == 0)
		return;
	else
		printf("[SPEEDY] timeout at write\n");
}

void speedy_write_burst(unsigned char slave, unsigned char addr,
                                unsigned char *data, unsigned char count)
{
	int result;
	u16 address = addr + ((slave & 0xF) << 8);
	int i;

	speedy_set_cmd(DIRECTION_WRITE, address, ACCESS_BURST, count);

	for(i = 0; i < count; i++)
		writel(data[i], SPEEDY_BASE + SPEEDY_TX_DATA);

	result = speedy_wait_transfer_done();
	if (result == 0)
		return;
	else
		printf("[SPEEDY] timeout at burst_write\n");
}
