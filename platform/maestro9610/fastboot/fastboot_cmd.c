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
#include <string.h>
#include <stdlib.h>
#include <lib/console.h>
#include "fastboot.h"

unsigned int download_size;
unsigned int download_bytes;
unsigned int download_error;
int extention_flag;

static int rx_handler (const unsigned char *buffer, unsigned int buffer_size);
static void reset_handler (void);

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

		printf("fb %s\n", cmdbuf);	// test: probing protocol

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

			ret = 0;
			strcpy(response,"OKAY");
			fastboot_tx_status(response, strlen(response), FASTBOOT_TX_ASYNC);
		}

		if (memcmp(cmdbuf, "getvar:", 7) == 0)
		{
			ret = 0;
			strcpy(response,"OKAY");
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
