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
#include <lib/console.h>
#include "fastboot.h"
#include <pit.h>
#include <platform/sfr.h>
#include <platform/smc.h>
#include <platform/ab_update.h>
#include <platform/environment.h>
#include <platform/if_pmic_s2mu004.h>

unsigned int download_size;
unsigned int download_bytes;
unsigned int download_error;
int extention_flag;
static unsigned int is_ramdump = 0;

static int rx_handler (const unsigned char *buffer, unsigned int buffer_size);
static void reset_handler (void);
int fastboot_tx_mem(u64 buffer, u64 buffer_size);

/* cmd_fastboot_interface	in fastboot.h	*/
struct cmd_fastboot_interface interface =
{
	.rx_handler            = rx_handler,
	.reset_handler         = reset_handler,
	.product_name          = NULL,
	.serial_no             = NULL,
	.nand_block_size       = 0,
	.transfer_buffer       = (unsigned char *)0xffffffff,
	.transfer_buffer_size  = 0,
};

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
		sprintf(response, "OKAY");
		return;
	}

	ptn = pit_get_part_info(key);
	if (ptn)
		length = pit_get_length(ptn);

	if (ptn == 0) {
		sprintf(response, "FAILpartition does not exist");
	} else if ((download_bytes > length) && (length != 0)) {
		sprintf(response, "FAILimage too large for partition");
	} else {
		if ((ptn->blknum != 0) && (download_bytes > length)) {
			printf("flashing '%s' failed\n", ptn->name);
			sprintf(response, "FAILfailed to too large image");
		} else if (pit_access(ptn, PIT_OP_FLASH, (u64)addr, size)) {
			printf("flashing '%s' failed\n", ptn->name);
			//print_lcd_update(FONT_RED, FONT_BLACK, "flashing '%s' failed", ptn->name);
			sprintf(response, "FAILfailed to flash partition");
		} else {
			printf("partition '%s' flashed\n\n", ptn->name);
			//print_lcd_update(FONT_GREEN, FONT_BLACK, "partition '%s' flashed", ptn->name);
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

static void start_ramdump(void *buf)
{
	struct fastboot_ramdump_hdr *hdr = buf;
	static uint32_t ramdump_cnt = 0;

	printf("\nramdump start address is [0x%llx]\n", hdr->base);
	printf("ramdump size is [0x%llx]\n", hdr->size);
	printf("version is [0x%llx]\n", hdr->version);

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

static int rx_handler (const unsigned char *buffer, unsigned int buffer_size)
{
	int ret = 1;

	/* Use 65 instead of 64
	   null gets dropped
	   strcpy's need the extra byte */
	char _res[128];
	char *response = (char *)(((unsigned long)_res+8)&~0x07);

	if (download_size)
	{
		/* Something to download */

		if (buffer_size)
		{
			/* Handle possible overflow */
			unsigned int transfer_size = download_size - download_bytes;
			unsigned int remain = transfer_size;

			if (buffer_size < transfer_size)
				transfer_size = buffer_size;

			if (extention_flag == 1)
				exynos_move_trb_buff(transfer_size, remain);
			else
				/* Save the data to the transfer buffer */
				memcpy (interface.transfer_buffer + download_bytes,
					buffer, transfer_size);

			download_bytes += transfer_size;

			/* Check if transfer is done */
			if (download_bytes >= download_size)
			{
				/* Reset global transfer variable,
				   Keep download_bytes because it will be
				   used in the next possible flashing command */
				download_size = 0;

				if (download_error)
				{
					/* There was an earlier error */
					sprintf(response, "ERROR");
				}
				else
				{
					/* Everything has transferred,
					   send the OK response */
					sprintf(response, "OKAY");
				}
				fastboot_tx_status(response, strlen(response), FASTBOOT_TX_ASYNC);

				printf("\ndownloading of %d bytes finished\n", download_bytes);

				if (extention_flag == 1)
				{
					extention_flag = 0;
					exynos_init_trb_buf();
				}

				if (is_ramdump) {
					is_ramdump = 0;
					start_ramdump((void *)buffer);
				}
			}

			/* Provide some feedback */
			if (download_bytes && download_size &&
			    0 == (download_bytes & (0x100000 - 1)))
			{
				/* Some feeback that the download is happening */
				if (download_error)
					printf("X");
				else
					printf(".");
				if (0 == (download_bytes %
					  (80 * 0x100000)))
					printf("\n");
			}
		}
		else
		{
			/* Ignore empty buffers */
			printf("Warning empty download buffer\n");
			printf("Ignoring\n");
		}
		ret = 0;
	}
	else
	{
		/* A command */

		/* Cast to make compiler happy with string functions */
		const char *cmdbuf = (char *) buffer;

		/* Generic failed response */
		sprintf(response, "FAIL");

#ifdef PIT_DEBUG
		printf("fb %s\n", cmdbuf);	// test: probing protocol
#endif

		/* download
		   download something ..
		   What happens to it depends on the next command after data */
		if (memcmp(cmdbuf, "download:", 9) == 0)
		{
			/* save the size */
			download_size = (unsigned int)strtol(cmdbuf + 9, NULL, 16);
			/* Reset the bytes count, now it is safe */
			download_bytes = 0;
			/* Reset error */
			download_error = 0;

			printf("Starting download of %d bytes\n", download_size);

			if (download_size > 0x100000 * 10)
			{
				extention_flag = 1;
				exynos_extend_trb_buf();
			}

			if (0 == download_size)
			{
				/* bad user input */
				sprintf(response, "FAILdata invalid size");
			}
			else if (download_size > interface.transfer_buffer_size)
			{
				/* set download_size to 0 because this is an error */
				download_size = 0;
				sprintf(response, "FAILdata too large");
			}
			else
			{
				/* The default case, the transfer fits
				   completely in the interface buffer */
				sprintf(response, "DATA%08x", download_size);
			}
			ret = 0;
			fastboot_tx_status(response, strlen(response), FASTBOOT_TX_ASYNC);
		}

		/* getvar
		   Get common fastboot variables
		   Board has a chance to handle other variables */
		if (memcmp(cmdbuf, "getvar:", 7) == 0)
		{
			strcpy(response,"OKAY");

			if (!strcmp(cmdbuf + 7, "version"))
			{
				strcpy(response + 4, FASTBOOT_VERSION);
			}
			else if (!strcmp(cmdbuf + 7, "product"))
			{
				if (interface.product_name)
					strcpy(response + 4, interface.product_name);
			}
			else if (!strcmp(cmdbuf + 7, "serialno"))
			{
				if (interface.serial_no)
					strcpy(response + 4, interface.serial_no);
			}
			else if (!strcmp(cmdbuf + 7, "downloadsize"))
			{
				if (interface.transfer_buffer_size)
					sprintf(response + 4, "%08x", interface.transfer_buffer_size);
			}
			else if (!memcmp(cmdbuf + 7, "partition-type", strlen("partition-type")))
			{
				char *key = (char *)cmdbuf + 7 + strlen("partition-type:");
				struct pit_entry *ptn = pit_get_part_info(key);

				/*
				 * In case of flashing pit, this should be
				 * passed unconditionally.
				 */
				if (strcmp(key, "pit") && ptn->filesys != FS_TYPE_NONE)
					strcpy(response + 4, "ext4");
			}
			else if (!memcmp(cmdbuf + 7, "partition-size", strlen("partition-size")))
			{
				char *key = (char *)cmdbuf + 7 + strlen("partition-size:");
				struct pit_entry *ptn = pit_get_part_info(key);

				/*
				 * In case of flashing pit, this location
				 * would not be passed. So it's unnecessary
				 * to check that this case is pit.
				 */
				if (ptn->filesys != FS_TYPE_NONE)
					sprintf(response + 4, "0x%llx", pit_get_length(ptn));
			}
			else if (!strcmp(cmdbuf + 7, "slot-count"))
			{
				sprintf(response + 4, "2");
			}
			else if (!strcmp(cmdbuf + 7, "current-slot"))
			{
				if (ab_current_slot())
					sprintf(response + 4, "_b");
				else
					sprintf(response + 4, "_a");
			}
			else if (!memcmp(cmdbuf + 7, "slot-successful", strlen("slot-successful")))
			{
				int slot = -1;
				if (!strcmp(cmdbuf + 7 + strlen("slot-successful:"), "_a"))
					slot = 0;
				else if (!strcmp(cmdbuf + 7 + strlen("slot-successful:"), "_b"))
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
			else if (!memcmp(cmdbuf + 7, "slot-unbootable", strlen("slot-unbootable")))
			{
				int slot = -1;
				if (!strcmp(cmdbuf + 7 + strlen("slot-unbootable:"), "_a"))
					slot = 0;
				else if (!strcmp(cmdbuf + 7 + strlen("slot-unbootable:"), "_b"))
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
			else if (!memcmp(cmdbuf + 7, "slot-retry-count", strlen("slot-retry-count")))
			{
				int slot = -1;
				if (!strcmp(cmdbuf + 7 + strlen("slot-retry-count:"), "_a"))
					slot = 0;
				else if (!strcmp(cmdbuf + 7 + strlen("slot-retry-count:"), "_b"))
					slot = 1;
				else
					sprintf(response, "FAILinvalid slot");
				if (slot >= 0)
					sprintf(response + 4, "%d", ab_slot_retry_count(slot));
			}
			else
			{
			}

			ret = 0;
			fastboot_tx_status(response, strlen(response), FASTBOOT_TX_ASYNC);
		}

		/* erase
		   Erase a register flash partition
		   Board has to set up flash partitions */
		if (memcmp(cmdbuf, "erase:", 6) == 0)
		{
			char *key = (char *)cmdbuf + 6;
			struct pit_entry *ptn = pit_get_part_info(key);
			char run_cmd[80];
			char if_name[16] = "mmc";
			int device=0;
			int status = 1;

			if (strcmp(key, "pit") && ptn == 0)
			{
				sprintf(response, "FAILpartition does not exist");
				ret = 0;
				fastboot_tx_status(response, strlen(response), FASTBOOT_TX_ASYNC);
			}

			printf("erasing(formatting) '%s'\n", ptn->name);

			if (ptn->filesys != FS_TYPE_NONE)
				status = pit_access(ptn, PIT_OP_ERASE, 0, 0);

			if (status)
			{
				sprintf(response,"FAILfailed to erase partition");
			}
			else
			{
				printf("partition '%s' erased\n", ptn->name);
				sprintf(response, "OKAY");
			}
			ret = 0;
			fastboot_tx_status(response, strlen(response), FASTBOOT_TX_ASYNC);
		}

		/* flash
		   Flash what was downloaded */
		if (memcmp(cmdbuf, "flash:", 6) == 0)
		{
			dprintf(ALWAYS, "flash\n");
			if (download_bytes == 0)
			{
				sprintf(response, "FAILno image downloaded");
				ret = 0;
				fastboot_tx_status(response, strlen(response), FASTBOOT_TX_ASYNC);
			}

			flash_using_pit((char *)cmdbuf + 6, response,
					download_bytes, (void *)interface.transfer_buffer);
			ret = 0;
			strcpy(response,"OKAY");
			fastboot_tx_status(response, strlen(response), FASTBOOT_TX_ASYNC);
		}

		/* reboot
		   Reboot the board. */
		if (memcmp(cmdbuf, "reboot", 6) == 0)
		{
			ret = 0;
			sprintf(response,"OKAY");
			fastboot_tx_status(response, strlen(response), FASTBOOT_TX_SYNC);
			writel(0, CONFIG_RAMDUMP_SCRATCH);
			writel(0x1, EXYNOS9610_SWRESET);
		}

		if (memcmp(cmdbuf, "ramdump:", 8) == 0)
		{
			printf("\nGot ramdump command\n");
			is_ramdump = 1;
			/* save the size */
			download_size = (unsigned int)strtol(cmdbuf + 8, NULL, 16);
			/* Reset the bytes count, now it is safe */
			download_bytes = 0;
			/* Reset error */
			download_error = 0;

			printf("Starting download of %d bytes\n", download_size);

			if (0 == download_size)
			{
				/* bad user input */
				sprintf(response, "FAILdata invalid size");
			}
			else if (download_size > interface.transfer_buffer_size)
			{
				/* set download_size to 0 because this is an error */
				download_size = 0;
				sprintf(response, "FAILdata too large");
			}
			else
			{
				/* The default case, the transfer fits
				   completely in the interface buffer */
				sprintf(response, "DATA%08x", download_size);
			}
			ret = 0;
			fastboot_tx_status(response, strlen(response), FASTBOOT_TX_SYNC);
		}

		/* set_active
		   Set active slot. */
		if (memcmp(cmdbuf, "set_active:", 11) == 0)
		{
			printf("set_active\n");

			sprintf(response,"OKAY");
			if (!strcmp(cmdbuf + 11, "a")) {
				printf("Set slot 'a' active.\n");
				ab_set_active(0);
			} else if (!strcmp(cmdbuf + 11, "b")) {
				printf("Set slot 'b' active.\n");
				ab_set_active(1);
			} else {
				sprintf(response, "FAILdata invalid size");
			}
			ret = 0;
			fastboot_tx_status(response, strlen(response), FASTBOOT_TX_ASYNC);
		}

	} /* End of command */

	return ret;
}

static void reset_handler ()
{
	/* If there was a download going on, bail */
	download_size = 0;
	download_bytes = 0;
	download_error = 0;
}

int do_fastboot(int argc, const cmd_args *argv)
{
	int continue_from_disconnect = 0;

	dprintf(ALWAYS, "This is do_fastboot\n");

	muic_sw_usb();

	do {
		continue_from_disconnect = 0;

		if(!fastboot_init(&interface))
		{
			int poll_status, board_poll_status;
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
