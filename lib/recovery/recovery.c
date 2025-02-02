/* Copyright (c) 2010-2017, The Linux Foundation. All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Copyright@ Samsung Electronics Co. LTD
 */
#include <lk/debug.h>
#include <string.h>
#include <lk/reg.h>
#include <part.h>
#include <platform/sfr.h>

#include "recovery.h"

#undef RECOVERY_DBG

#define RECOVERY_BLOCK_SIZE	4096

static char buf[RECOVERY_BLOCK_SIZE];

#ifdef RECOVERY_DBG
static void print_debug_buffer(void)
{
	int i=0 ;
	printf("== buf contents : ===\n");
	for (i=0 ; i<512 ; i++) {
		if (i % 16 == 0)
			printf("\n %04d : ", i);

		printf("%02x ", buf[i-1]);
	}
	printf("\n===================\n");
}

static void print_debug_recovery_struct(const struct recovery_message *pr)
{
	printf("command  [%s]\n", pr->command);
	printf("status   [%s]\n", pr->status);
	printf("recovery [%s]\n", pr->recovery);
}
#endif

static int get_recovery_message(struct recovery_message *out)
{
	void *part = NULL;
	int ret = 0;

	part = part_get("misc");

	if (part == NULL) {
		printf("ERROR: No misc partition found\n");
		return -1;
	}

	ret = part_read_partial(part, (void *)buf, 0, RECOVERY_BLOCK_SIZE);
	if (ret) {
		printf("ERROR: misc partition read error\n");
		return -1;
	}

	memcpy(out, buf, sizeof(*out));

#ifdef RECOVERY_DBG
	printf("get_recovery_msg : \n");
	print_debug_recovery_struct(out);
	print_debug_buffer();
#endif
	return 0;
}

static int set_recovery_message(const struct recovery_message *in)
{
	void *part = NULL;
	int ret = 0;

	part = part_get("misc");

	if (part == NULL) {
		printf("ERROR: No misc partition found\n");
		return -1;
	}

	ret = part_read_partial(part, (void *)buf, 0, RECOVERY_BLOCK_SIZE);
	if (ret) {
		printf("ERROR: misc partition read error\n");
		return -1;
	}

	memcpy(buf, (void *)in, sizeof(*in));
	ret = part_write_partial(part, (void *)buf, 0, RECOVERY_BLOCK_SIZE);
	if (ret) {
		printf("ERROR: Cannot write recovery_header\n");
		return -1;
	}

#ifdef RECOVERY_DBG
	printf("set_recovery_msg : \n");
	print_debug_recovery_struct(in);
	print_debug_buffer();
#endif
	return 0;
}

void set_recovery_boot(int force)
{
	unsigned int boot_val = 0;
	boot_val = readl(EXYNOS_POWER_SYSIP_DAT0);

	printf("%s: set bootval [0x%02X]\n", __func__, boot_val);
	if (force) {
		printf("- force set bootval to RECOVERY\n");
		writel(REBOOT_MODE_RECOVERY, EXYNOS_POWER_SYSIP_DAT0);
	} else {
		printf("- leave it to preset value\n");
	}

}

/* Bootloader / Recovery Flow
 *
 * On every boot, the bootloader will read the recovery_message
 * from flash and check the command field.  The bootloader should
 * deal with the command field not having a 0 terminator correctly
 * (so as to not crash if the block is invalid or corrupt).
 *
 * The bootloader will have to publish the partition that contains
 * the recovery_message to the linux kernel so it can update it.
 *
 * if command == "boot-recovery" -> boot recovery.img
 * else -> boot boot.img (normal boot)
 *
 * The bootloader will not modify or erase the cache partition.
 * It is recovery's responsibility to clean up the mess afterwards.
 */

int recovery_init (void)
{
	struct recovery_message msg;

	if (get_recovery_message(&msg))
		return -1;

	msg.command[sizeof(msg.command)-1] = '\0'; //Ensure termination
	msg.status[sizeof(msg.status)-1] = '\0';   //Ensure termination

	if (msg.command[0] != 0 && msg.command[0] != 255) {
		printf("Recovery: command: %ld %s\n",
			sizeof(msg.command), msg.command);
	}

	if (!strcmp("boot-recovery",msg.command))
	{
		strlcpy(msg.command, "", sizeof(msg.command));	// to safe against multiple reboot into recovery
		strlcpy(msg.status, "OKAY", sizeof(msg.status));
		set_recovery_message(&msg);	// set recovery message
		set_recovery_boot(1);		// Boot in recovery mode

		return 0;
	}

	return 0;
}

