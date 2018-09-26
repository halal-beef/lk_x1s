/*
 * (C) Copyright 2018 SAMSUNG Electronics
 * Kyounghye Yun <k-hye.yun@samsung.com>
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted
 * transcribed, stored in a retrieval system or translated into any human or computer language in an
 * form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 *
 */

#include <debug.h>
#include <reg.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <lib/console.h>
#include <lib/font_display.h>
#include "fastboot.h"
#include <pit.h>
#include <platform/sfr.h>
#include <platform/smc.h>
#include <platform/lock.h>
#include <platform/ab_update.h>
#include <platform/environment.h>
#include <platform/if_pmic_s2mu004.h>
#include <platform/dfd.h>
#include <dev/boot.h>
#include <dev/rpmb.h>
#include <dev/scsi.h>

#define FB_RESPONSE_BUFFER_SIZE 128

unsigned int download_size;
unsigned int downloaded_data_size;
int extention_flag;
static unsigned int is_ramdump = 0;

static int rx_handler (const unsigned char *buffer, unsigned int buffer_size);
int fastboot_tx_mem(u64 buffer, u64 buffer_size);

/* cmd_fastboot_interface	in fastboot.h	*/
struct cmd_fastboot_interface interface =
{
	.rx_handler            = rx_handler,
	.reset_handler         = NULL,
	.product_name          = NULL,
	.serial_no             = NULL,
	.nand_block_size       = 0,
	.transfer_buffer       = (unsigned char *)0xffffffff,
	.transfer_buffer_size  = 0,
};

struct cmd_fastboot {
	const char *cmd_name;
	int (*handler)(const char *);
};

int fb_do_getvar(const char *cmd_buffer)
{
	char buf[FB_RESPONSE_BUFFER_SIZE];
	char *response = (char *)(((unsigned long)buf + 8) & ~0x07);

	sprintf(response,"OKAY");

	if (!memcmp(cmd_buffer + 7, "version", strlen("version")))
	{
		sprintf(response + 4, FASTBOOT_VERSION);
	}
	else if (!memcmp(cmd_buffer + 7, "product", strlen("product")))
	{
		if (interface.product_name)
			sprintf(response + 4, interface.product_name);
	}
	else if (!memcmp(cmd_buffer + 7, "serialno", strlen("serialno")))
	{
		if (interface.serial_no)
			sprintf(response + 4, interface.serial_no);
	}
	else if (!memcmp(cmd_buffer + 7, "downloadsize", strlen("downloadsize")))
	{
		if (interface.transfer_buffer_size)
			sprintf(response + 4, "%08x", interface.transfer_buffer_size);
	}
	else if (!memcmp(cmd_buffer + 7, "partition-type", strlen("partition-type")))
	{
		char *key = (char *)cmd_buffer + 7 + strlen("partition-type:");
		struct pit_entry *ptn = pit_get_part_info(key);

		/*
		 * In case of flashing pit, this should be
		 * passed unconditionally.
		 */
		if (strcmp(key, "pit") && ptn->filesys != FS_TYPE_NONE)
			strcpy(response + 4, "ext4");
	}
	else if (!memcmp(cmd_buffer + 7, "partition-size", strlen("partition-size")))
	{
		char *key = (char *)cmd_buffer + 7 + strlen("partition-size:");
		struct pit_entry *ptn = pit_get_part_info(key);

		/*
		 * In case of flashing pit, this location
		 * would not be passed. So it's unnecessary
		 * to check that this case is pit.
		 */
		if (ptn->filesys != FS_TYPE_NONE)
			sprintf(response + 4, "0x%llx", pit_get_length(ptn));
	}
	else if (!strcmp(cmd_buffer + 7, "slot-count"))
	{
		sprintf(response + 4, "2");
	}
	else if (!strcmp(cmd_buffer + 7, "current-slot"))
	{
		if (ab_current_slot())
			sprintf(response + 4, "_b");
		else
			sprintf(response + 4, "_a");
	}
	else if (!memcmp(cmd_buffer + 7, "slot-successful", strlen("slot-successful")))
	{
		int slot = -1;
		if (!strcmp(cmd_buffer + 7 + strlen("slot-successful:"), "_a"))
			slot = 0;
		else if (!strcmp(cmd_buffer + 7 + strlen("slot-successful:"), "_b"))
			slot = 1;
		else
			sprintf(response, "FAILinvalid slot");
		printf("slot: %d\n", slot);
		if (slot >= 0) {
			if (ab_slot_successful(slot))
				sprintf(response + 4, "yes");
			else
				sprintf(response + 4, "no");
		}
	}
	else if (!memcmp(cmd_buffer + 7, "slot-unbootable", strlen("slot-unbootable")))
	{
		int slot = -1;
		if (!strcmp(cmd_buffer + 7 + strlen("slot-unbootable:"), "_a"))
			slot = 0;
		else if (!strcmp(cmd_buffer + 7 + strlen("slot-unbootable:"), "_b"))
			slot = 1;
		else
			sprintf(response, "FAILinvalid slot");
		if (slot >= 0) {
			if (ab_slot_unbootable(slot))
				sprintf(response + 4, "yes");
			else
				sprintf(response + 4, "no");
		}
	}
	else if (!memcmp(cmd_buffer + 7, "slot-retry-count", strlen("slot-retry-count")))
	{
		int slot = -1;
		if (!strcmp(cmd_buffer + 7 + strlen("slot-retry-count:"), "_a"))
			slot = 0;
		else if (!strcmp(cmd_buffer + 7 + strlen("slot-retry-count:"), "_b"))
			slot = 1;
		else
			sprintf(response, "FAILinvalid slot");
		if (slot >= 0)
			sprintf(response + 4, "%d", ab_slot_retry_count(slot));
		}
	else if (!memcmp(cmd_buffer + 7, "has-slot", strlen("has-slot")))
	{
		if (!strcmp(cmd_buffer + 7 + strlen("has-slot:"), "boot") ||
			!strcmp(cmd_buffer + 7 + strlen("has-slot:"), "dtb") ||
			!strcmp(cmd_buffer + 7 + strlen("has-slot:"), "dtbo") ||
			!strcmp(cmd_buffer + 7 + strlen("has-slot:"), "system") ||
			!strcmp(cmd_buffer + 7 + strlen("has-slot:"), "vendor"))
			sprintf(response + 4, "yes");
		else
			sprintf(response + 4, "no");
	}
	else
	{
		debug_snapshot_getvar_item(cmd_buffer + 7, response + 4);
	}

	fastboot_tx_status(response, strlen(response), FASTBOOT_TX_ASYNC);

	return 0;
}

int fb_do_erase(const char *cmd_buffer)
{
	char buf[FB_RESPONSE_BUFFER_SIZE];
	char *response = (char *)(((unsigned long)buf + 8) & ~0x07);
	char *key = (char *)cmd_buffer + 6;
	struct pit_entry *ptn = pit_get_part_info(key);
	int status = 1;

	if (strcmp(key, "pit") && ptn == 0)
	{
		sprintf(response, "FAILpartition does not exist");
		fastboot_tx_status(response, strlen(response), FASTBOOT_TX_ASYNC);
		return 0;
	}

	printf("erasing(formatting) '%s'\n", ptn->name);

	if (ptn->filesys != FS_TYPE_NONE)
		status = pit_access(ptn, PIT_OP_ERASE, 0, 0);

	if (status) {
		sprintf(response,"FAILfailed to erase partition");
	} else {
		printf("partition '%s' erased\n", ptn->name);
		sprintf(response, "OKAY");
	}

	fastboot_tx_status(response, strlen(response), FASTBOOT_TX_ASYNC);

	return 0;
}

static void flash_using_pit(char *key, char *response,
		u32 size, void *addr)
{
	struct pit_entry *ptn;
	unsigned long long length;
	u32 *env_val;

	/*
	 * In case of flashing pit, this should be
	 * passed unconditionally.
	 */
	if (!strcmp(key, "pit")) {
		pit_update(addr, size);
		print_lcd_update(FONT_GREEN, FONT_BLACK, "partition 'pit' flashed");
		sprintf(response, "OKAY");
		return;
	}

	ptn = pit_get_part_info(key);
	if (ptn)
		length = pit_get_length(ptn);

	if (ptn == 0) {
		sprintf(response, "FAILpartition does not exist");
	} else if ((downloaded_data_size > length) && (length != 0)) {
		sprintf(response, "FAILimage too large for partition");
	} else {
		if ((ptn->blknum != 0) && (downloaded_data_size > length)) {
			printf("flashing '%s' failed\n", ptn->name);
			print_lcd_update(FONT_RED, FONT_BLACK, "flashing '%s' failed", ptn->name);
			sprintf(response, "FAILfailed to too large image");
		} else if (pit_access(ptn, PIT_OP_FLASH, (u64)addr, size)) {
			printf("flashing '%s' failed\n", ptn->name);
			print_lcd_update(FONT_RED, FONT_BLACK, "flashing '%s' failed", ptn->name);
			sprintf(response, "FAILfailed to flash partition");
		} else {
			printf("partition '%s' flashed\n\n", ptn->name);
			print_lcd_update(FONT_GREEN, FONT_BLACK, "partition '%s' flashed", ptn->name);
			sprintf(response, "OKAY");
		}
	}

	if (!strcmp(key, "ramdisk")) {
		ptn = pit_get_part_info("env");
		env_val = memalign(0x1000, pit_get_length(ptn));
		pit_access(ptn, PIT_OP_LOAD, (u64)env_val, 0);

		env_val[ENV_ID_RAMDISK_SIZE] = size;
		pit_access(ptn, PIT_OP_FLASH, (u64)env_val, 0);

		free(env_val);
	}
}

int fb_do_flash(const char *cmd_buffer)
{
	char buf[FB_RESPONSE_BUFFER_SIZE];
	char *response = (char *)(((unsigned long)buf + 8) & ~0x07);

	if(is_first_boot()) {
		uint32_t lock_state;
		rpmb_get_lock_state(&lock_state);
		printf("Lock state: %d\n", lock_state);
		if(lock_state) {
			sprintf(response, "FAILDevice is locked");
			fastboot_tx_status(response, strlen(response), FASTBOOT_TX_ASYNC);
			return 1;
		}
	}

	dprintf(ALWAYS, "flash\n");

	flash_using_pit((char *)cmd_buffer + 6, response,
			downloaded_data_size, (void *)interface.transfer_buffer);

	strcpy(response,"OKAY");
	fastboot_tx_status(response, strlen(response), FASTBOOT_TX_ASYNC);

	return 0;
}

int fb_do_reboot(const char *cmd_buffer)
{
	char buf[FB_RESPONSE_BUFFER_SIZE];
	char *response = (char *)(((unsigned long)buf + 8) & ~0x07);
	bdev_t *dev;
	status_t ret;

	/* Send SSU to UFS */
	dev = bio_open("scsissu");
	if (!dev) {
		printf("error opening block device\n");
	}

	ret = scsi_start_stop_unit(dev);
	if (ret != NO_ERROR)
		printf("scsi ssu error!\n");

	bio_close(dev);

	sprintf(response,"OKAY");
	fastboot_tx_status(response, strlen(response), FASTBOOT_TX_SYNC);

	if(!memcmp(cmd_buffer, "reboot-bootloader", strlen("reboot-bootloader")))
		writel(CONFIG_RAMDUMP_MODE, CONFIG_RAMDUMP_SCRATCH);
	else
		writel(0, CONFIG_RAMDUMP_SCRATCH);

	writel(0x1, EXYNOS9610_SWRESET);

	return 0;
}

int fb_do_download(const char *cmd_buffer)
{
	char buf[FB_RESPONSE_BUFFER_SIZE];
	char *response = (char *)(((unsigned long)buf + 8) & ~0x07);

	/* Get download size */
	download_size = (unsigned int)strtol(cmd_buffer + 9, NULL, 16);
	downloaded_data_size = 0;

	printf("Downloaing. Download size is %d bytes\n", download_size);

	if (download_size > 0x100000 * 10) {
		extention_flag = 1;
		exynos_extend_trb_buf();
	}

	if (download_size > interface.transfer_buffer_size) {
		download_size = 0;
		sprintf(response, "FAILdownload data size is bigger than buffer size");
	} else {
		sprintf(response, "DATA%08x", download_size);
	}

	fastboot_tx_status(response, strlen(response), FASTBOOT_TX_ASYNC);

	return 0;
}

static void start_ramdump(void *buf)
{
	struct fastboot_ramdump_hdr *hdr = buf;
	static uint32_t ramdump_cnt = 0;

	printf("\nramdump start address is [0x%lx]\n", hdr->base);
	printf("ramdump size is [0x%lx]\n", hdr->size);
	printf("version is [0x%lx]\n", hdr->version);

	if (hdr->version != 2) {
		printf("you are using wrong version of fastboot!!!\n");
	}

	/* dont't generate DECERR even if permission failure of ASP occurs */
	if (ramdump_cnt++ == 0)
		set_tzasc_action(0);

	if (!fastboot_tx_mem(hdr->base, hdr->size)) {
		printf("Failed ramdump~! \n");
	} else {
		printf("Finished ramdump~! \n");
	}
}

int fb_do_ramdump(const char *cmd_buffer)
{
	char buf[FB_RESPONSE_BUFFER_SIZE];
	char *response = (char *)(((unsigned long)buf + 8) & ~0x07);

	printf("\nGot ramdump command\n");
	print_lcd_update(FONT_GREEN, FONT_BLACK, "Got ramdump command.");
	is_ramdump = 1;
	/* Get download size */
	download_size = (unsigned int)strtol(cmd_buffer + 8, NULL, 16);
	downloaded_data_size = 0;

	printf("Downloaing. Download size is %d bytes\n", download_size);

	if (download_size > interface.transfer_buffer_size) {
		download_size = 0;
		sprintf(response, "FAILdownload data size is bigger than buffer size");
	} else {
		sprintf(response, "DATA%08x", download_size);
	}

	fastboot_tx_status(response, strlen(response), FASTBOOT_TX_SYNC);

	return 0;
}

int fb_do_set_active(const char *cmd_buffer)
{
	char buf[FB_RESPONSE_BUFFER_SIZE];
	char *response = (char *)(((unsigned long)buf + 8) & ~0x07);

	printf("set_active\n");

	sprintf(response,"OKAY");
	if (!strcmp(cmd_buffer + 11, "a")) {
		printf("Set slot 'a' active.\n");
		print_lcd_update(FONT_GREEN, FONT_BLACK, "Set slot 'a' active.");
		ab_set_active(0);
	} else if (!strcmp(cmd_buffer + 11, "b")) {
		printf("Set slot 'b' active.\n");
		print_lcd_update(FONT_GREEN, FONT_BLACK, "Set slot 'b' active.");
		ab_set_active(1);
	} else {
		sprintf(response, "FAILinvalid slot");
	}

	fastboot_tx_status(response, strlen(response), FASTBOOT_TX_ASYNC);

	return 0;
}

/* Lock/unlock device */
int fb_do_flashing(const char *cmd_buffer)
{
	char buf[FB_RESPONSE_BUFFER_SIZE];
	char *response = (char *)(((unsigned long)buf + 8) & ~0x07);

	sprintf(response,"OKAY");
	if (!strcmp(cmd_buffer + 9, "lock")) {
		printf("Lock this device.\n");
		print_lcd_update(FONT_GREEN, FONT_BLACK, "Lock this device.");
		if(rpmb_set_lock_state(1))
			sprintf(response, "FAILRPBM error: failed to change lock state on RPMB");
	} else if (!strcmp(cmd_buffer + 9, "unlock")) {
		if (get_unlock_ability()) {
			printf("Unlock this device.\n");
			print_lcd_update(FONT_GREEN, FONT_BLACK, "Unlock this device.");
			if(rpmb_set_lock_state(0))
				sprintf(response, "FAILRPBM error: failed to change lock state on RPMB");
		} else {
			sprintf(response, "FAILunlock_ability is 0");
		}
	} else if (!strcmp(cmd_buffer + 9, "lock_critical")) {
		printf("Lock critical partitions of this device.\n");
		print_lcd_update(FONT_GREEN, FONT_BLACK, "Lock critical partitions of this device.");
		lock_critical(0);
	} else if (!strcmp(cmd_buffer + 9, "unlock_critical")) {
		printf("Unlock critical partitions of this device.\n");
		print_lcd_update(FONT_GREEN, FONT_BLACK, "Unlock critical partitions of this device.");
		lock_critical(0);
	} else if (!strcmp(cmd_buffer + 9, "get_unlock_ability")) {
		printf("Get unlock_ability.\n");
		print_lcd_update(FONT_GREEN, FONT_BLACK, "Get unlock_ability.");
		sprintf(response + 4, "%d", get_unlock_ability());
	} else {
		sprintf(response, "FAILunsupported command");
	}

	fastboot_tx_status(response, strlen(response), FASTBOOT_TX_ASYNC);

	return 0;
}

struct cmd_fastboot cmd_list[] = {
	{"reboot", fb_do_reboot},
	{"flash:", fb_do_flash},
	{"erase:", fb_do_erase},
	{"download:", fb_do_download},
	{"ramdump:", fb_do_ramdump},
	{"getvar:", fb_do_getvar},
	{"set_active:", fb_do_set_active},
	{"flashing", fb_do_flashing},
};

static int rx_handler (const unsigned char *buffer, unsigned int buffer_size)
{
	unsigned int i;
	unsigned int rx_size;
	unsigned int remain;
	const char *cmd_buffer;

	char buf[FB_RESPONSE_BUFFER_SIZE];
	char *response = (char *)(((unsigned long)buf + 8) & ~0x07);

	if (download_size)
	{
		if (buffer_size == 0) {
			printf("USB download buffer is empty\n");
			return 0;
		}

		remain = download_size - downloaded_data_size;

		if (buffer_size < remain)
			rx_size = buffer_size;
		else
			rx_size = remain;

		if (extention_flag == 1)
			exynos_move_trb_buff(rx_size, remain);
		else
			/* Save the data to the transfer buffer */
			memcpy (interface.transfer_buffer + downloaded_data_size,
				buffer, rx_size);

		downloaded_data_size += rx_size;

		if (downloaded_data_size == download_size) {
			download_size = 0;

			sprintf(response, "OKAY");
			fastboot_tx_status(response, strlen(response), FASTBOOT_TX_ASYNC);

			printf("\nFinished to download %d bytes\n", downloaded_data_size);

			if (extention_flag == 1) {
				extention_flag = 0;
				exynos_init_trb_buf();
			}

			if (is_ramdump) {
				is_ramdump = 0;
				start_ramdump((void *)buffer);
			}

			return 0;
		}

		/* Print download progress */
		if (0 == (downloaded_data_size % 0x100000)) {
			printf("*");

			if (0 == (downloaded_data_size % (80 * 0x100000)))
				printf("\n");
		}
	} else {
		cmd_buffer = (char *)buffer;

#ifdef PIT_DEBUG
		printf("fb %s\n", cmd_buffer);	/* test: probing protocol */
#endif

		for (i = 0; i < (sizeof(cmd_list) / sizeof(struct cmd_fastboot)); i++) {
			if(!memcmp(cmd_buffer, cmd_list[i].cmd_name, strlen(cmd_list[i].cmd_name)))
				cmd_list[i].handler(cmd_buffer);
		}
	}

	return 0;
}

int do_fastboot(int argc, const cmd_args *argv)
{
	int continue_from_disconnect = 0;

	dprintf(ALWAYS, "This is do_fastboot\n");
	print_lcd_update(FONT_GREEN, FONT_BLACK, "Entering fastboot mode.");

	/* display all entries */
	pit_show_info();

	muic_sw_usb();

	do {
		continue_from_disconnect = 0;

		if(!fastboot_init(&interface))
		{
			int poll_status;
			dprintf(ALWAYS, "fastboot_init success!!\n");
			while (1) {
				poll_status = fastboot_poll();

				if (FASTBOOT_ERROR == poll_status) {
					dprintf(ALWAYS, "Fastboot ERROR!\n");
					break;
				} else if (FASTBOOT_DISCONNECT == poll_status) {
					dprintf(ALWAYS, "Fastboot DISCONNECT detected\n");
					continue_from_disconnect = 1;
					break;
				}
			}
		}

	} while (continue_from_disconnect);

	return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("fast", "usb fastboot", &do_fastboot)
STATIC_COMMAND_END(usb_fastboot);
