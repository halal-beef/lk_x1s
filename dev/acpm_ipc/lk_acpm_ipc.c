
/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
 *
 * Functions for sending message to APM
 */

#include <lk/reg.h>
#include <dev/lk_acpm_ipc.h>
#include "lk_acpm_mfd.h"

void acpm_ipc_send_data_async(unsigned int ipc_ch, unsigned int cmd0, unsigned int cmd1)
{
	unsigned int acpm_ipc = 0;

	acpm_ipc = (1 << ipc_ch) << INTR_FLAG_OFFSET;

	writel(cmd0, EXYNOS_MESSAGE_APM + SR0);
	writel(cmd1, EXYNOS_MESSAGE_APM + SR1);
	__asm__ volatile ("dsb	sy");
	writel(acpm_ipc, EXYNOS_MAILBOX_AP2APM + INTGR0);	//dsb
}

void acpm_ipc_send_data_wait(unsigned int ipc_ch, unsigned int *cmd0, unsigned int *cmd1)
{
	unsigned int reg = 0;

	do  {
		reg = readl(EXYNOS_MAILBOX_AP2APM + INTSR1) & (0x1 << ipc_ch);
	} while (!(reg));

	writel(0x1 << ipc_ch, EXYNOS_MAILBOX_AP2APM + INTCR1);

	*cmd0 = readl(EXYNOS_MESSAGE_APM + SR2);
	*cmd1 = readl(EXYNOS_MESSAGE_APM + SR3);
}

void i3c_read(unsigned char channel, unsigned char type, unsigned char reg, unsigned char *dest)
{
	unsigned int command[4] = {0,};
	int ret = 0;

	command[0] = set_protocol(type, TYPE) | set_protocol(reg, REG) |
			set_protocol(channel, CHANNEL);
	command[1] = set_protocol(FUNC_READ, FUNC);


	acpm_ipc_send_data_async(IPC_AP_I3C, command[0], command[1]);

	acpm_ipc_send_data_wait(IPC_AP_I3C, &command[2], &command[3]);

	*dest = read_protocol(command[3], DEST);
	ret = read_protocol(command[3], RETURN);
	if (ret) {
		;// while(1);
	}
}

void i3c_write(unsigned char channel, unsigned char type, unsigned char reg, unsigned char value)
{
	unsigned int command[4] = {0,};
	int ret = 0;

	command[0] = set_protocol(type, TYPE) | set_protocol(reg, REG) |
			set_protocol(channel, CHANNEL);
	command[1] = set_protocol(FUNC_WRITE, FUNC) | set_protocol(value, WRITE_VAL);


	acpm_ipc_send_data_async(IPC_AP_I3C, command[0], command[1]);

	acpm_ipc_send_data_wait(IPC_AP_I3C, &command[2], &command[3]);

	ret = read_protocol(command[3], RETURN);
	if (ret) {
		;// while(1);
	}
}
