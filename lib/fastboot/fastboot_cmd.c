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
#include <trace.h>
#include <reg.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <lib/console.h>
#include <lib/font_display.h>
//#include <lib/fastboot.h>
#include <part.h>
#include <pit.h>
#include <platform/sfr.h>
#include <platform/smc.h>
#include <platform/ldfw.h>
#include <lib/lock.h>
#include <lib/ab_update.h>
#include <platform/environment.h>
#include <platform/bootimg.h>
#include <platform/mmu/mmu_func.h>
#include <dev/debug/dss.h>
#include <dev/debug/dss_store_ramdump.h>
#include <dev/usb/fastboot.h>
#include <target/board_info.h>
#include <dev/boot.h>
#include <dev/rpmb.h>
#include <dev/scsi.h>
#include <dev/pmucal_local.h>

#include "usb-def.h"

extern void fastboot_send_info(char *response, unsigned int len);
extern void fastboot_send_payload(void *buf, unsigned int len);
extern void fastboot_send_status(char *response, unsigned int len, int sync);
extern void fastboot_set_payload_data(int dir, void *buf, unsigned int len);
extern void fasboot_set_rx_sz(unsigned int prot_req_sz);
extern void fastboot_tx_event_init(void);

#define FB_RESPONSE_BUFFER_SIZE 128
#define REBOOT_MODE_RECOVERY	0xFF
#define LOCAL_TRACE 0

unsigned int download_size;
unsigned int downloaded_data_size;
int extention_flag;
static unsigned int is_ramdump = 0;
unsigned int s_fb_on_diskdump = 0;
static char resp_data[FB_RESPONSE_BUFFER_SIZE];

int rx_handler(const unsigned char *buffer, unsigned int buffer_size);
int fastboot_tx_mem(u64 buffer, u64 buffer_size);

/* cmd_fastboot_interface	in fastboot.h	*/
struct cmd_fastboot_interface interface = {
	.rx_handler            = rx_handler,
	.reset_handler         = NULL,
	.product_name          = NULL,
	.serial_no             = NULL,
	.nand_block_size       = CFG_FASTBOOT_PAGESIZE * 64,
	.transfer_buffer       = (unsigned char *)CFG_FASTBOOT_TRANSFER_BUFFER,
	.transfer_buffer_size  = CFG_FASTBOOT_TRANSFER_BUFFER_SIZE,
};

struct cmd_fastboot {
	const char *cmd_name;
	int (*handler)(char *, unsigned int);
};

const char *blocked_partitions[] = {
	"bootloader",
	"efs",
	"blcmd",
	"bota",
	"cp_debug",
	"cpefs",
	"dqmdbg",
	"efs",
	"harx",
	"keyrefuge",
	"nad_refer",
	"omr",
	"param",
	"persistent",
	"radio",
	"sec_efs",
	"spu",
	"steady",
	"uh",
	"uhcfg"
};

const int blocked_count = sizeof(blocked_partitions) /
						  sizeof(blocked_partitions[0]);

__attribute__((weak)) void platform_prepare_reboot(void)
{
	/* WARNING : NOT MODIFY THIS FUNCTION
	 * If you need to sw reset by reboot command,
	 * please implementate on the platform.
	 */
}

__attribute__((weak)) void platform_do_reboot(const char *cmd_buf)
{
	/* WARNING : NOT MODIFY THIS FUNCTION
	 * If you need to sw reset by reboot command,
	 * please implementate on the platform.
	 */
	return;
}

__attribute__((weak)) const char * fastboot_get_product_string(void)
{
	/* WARNING : NOT MODIFY THIS FUNCTION
	 * If you need to get product string address
	 * please implementate on the platform.
	 */
	return NULL;
}

__attribute__((weak)) const char * fastboot_get_serialno_string(void)
{
	/* WARNING : NOT MODIFY THIS FUNCTION
	 * If you need to get product string address
	 * please implementate on the platform.
	 */
	return NULL;
}

__attribute__((weak)) struct cmd_fastboot_variable *fastboot_get_var_head(void)
{
	/* WARNING : NOT MODIFY THIS FUNCTION
	 * If you need to get product string address
	 * please implementate on the platform.
	 */
	return NULL;
}

__attribute__((weak)) int fastboot_get_var_num(void)
{
	/* WARNING : NOT MODIFY THIS FUNCTION
	 * If you need to get product string address
	 * please implementate on the platform.
	 */
	return -1;
}

__attribute__((weak)) int init_fastboot_variables(void)
{
	/* WARNING : NOT MODIFY THIS FUNCTION
	 * If you need to get product string address
	 * please implementate on the platform.
	 */
	return -1;
}

__attribute__((weak)) void get_serialno(int *chip_id)
{
	/* WARNING : NOT MODIFY THIS FUNCTION
	 * If you need to get product string address
	 * please implementate on the platform.
	 */
}


const char *fastboot_variables[] = 
{
	"version",
	"product",
	"serialno",
	"downloadsize",
	"max-download-size",
	"partition-type",
	"partition-size",
	"erase-block-size",
	"logical-block-size",
	"slot-count",
	"current-slot",
	"slot-successful",
	"slot-unbootable",
	"slot-retry-count",
	"has-slot",
	"unlocked",
	"uid",
	"str_ram"
};

enum fastboot_variable_id
{
	VERSION = 0,
	PRODUCT,
	SERIALNO,
	DOWNLOAD_SIZE,
	MAX_DOWNLOAD_SIZE,
	PARTITION_TYPE,
	PARTITION_SIZE,
	ERASE_BLOCK_SIZE,
	LOGICAL_BLOCK_SIZE,
	SLOT_COUNT,
	CURRENT_SLOT,
	SLOT_SUCESSFUL,
	SLOT_UNBOOTABLE,
	SLOT_RETRY_COUNT,
	HAS_SLOT,
	BL_UNLOCKED,
	UID,
	STR_RAM,
	FASTBOOT_VARIABLE_END,
	ALL = 0xA11,
};

const char *oem_commands[] = 
{
	"str_ram",
	"reboot-download",
};

enum oem_commands_id
{
	OEM_STR_RAM = 0,
	OEM_REBOOT_DOWNLOAD,
	OEM_CMD_END,
};

enum fastboot_variable_id fastboot_variable_to_id(char *variable)
{
	for(int i = 0; i < FASTBOOT_VARIABLE_END; i++)
		if(!strncmp(fastboot_variables[i], variable, strlen(fastboot_variables[i])))
			return i;
		else if(!strcmp("all", variable))
			return ALL;
	return -1;
}

enum oem_commands_id oem_command_to_id(char *oem_cmd)
{
	for(int i = 0; i < OEM_CMD_END; i++)
		if(!strncmp(oem_commands[i], oem_cmd, strlen(oem_commands[i])))
			return i;
	return -1;
}

static void simple_byte_hextostr(u8 hex, char *str)
{
	int i;

	for (i = 0; i < 2; i++) {
		if ((hex & 0xF) > 9)
			*--str = 'a' + (hex & 0xF) - 10;
		else
			*--str = '0' + (hex & 0xF);

		hex >>= 4;
	}
}

static void hex2str(u8 *buf, char *str, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		simple_byte_hextostr(buf[i], str + (i + 1) * 2);
	}
}

bool partition_is_blocked(const char* partition)
{
	for (int i = 0; i < blocked_count; i++)
		if (stricmp(partition, blocked_partitions[i]) == 0)
			return true; // Partition is blocked

	return false; // Partition is not blocked
}

int fb_do_getvar(char *cmd_buffer, unsigned int rx_sz)
{
	char buf[FB_RESPONSE_BUFFER_SIZE];
	char *response = (char *)(((unsigned long)buf + 8) & ~0x07);
	char *variable = cmd_buffer + 7;
	const char *tmp;
	int ret;

	int slot = -1;

	LTRACEF("fast received cmd:%s\n", cmd_buffer);

	sprintf(response,"OKAY");

	switch (fastboot_variable_to_id(variable))
	{
		case VERSION: {
			sprintf(response + 4, FASTBOOT_VERSION);
			break;
		}

		case PRODUCT: {
			tmp = fastboot_get_product_string();
			if (tmp)
				sprintf(response + 4, tmp);
			break;
		}

		case SERIALNO: {
			tmp = fastboot_get_serialno_string();
			if (tmp)
				sprintf(response + 4, tmp);
			break;
		}

		case DOWNLOAD_SIZE: {
			if (interface.transfer_buffer_size)
				sprintf(response + 4, "%08x", interface.transfer_buffer_size);
			break;
		}

		case MAX_DOWNLOAD_SIZE: {
			if (interface.transfer_buffer_size)
				sprintf(response + 4, "%d", interface.transfer_buffer_size);
			break;
		}

		case ERASE_BLOCK_SIZE: {
			sprintf(response + 4, "0x%x", part_get_block_size());
			break;
		}

		case LOGICAL_BLOCK_SIZE: {
			sprintf(response + 4, "0x%x", part_get_erase_size());
			break;
		}

		case SLOT_COUNT: {
			sprintf(response + 4, "0");
			break;
		}

		case CURRENT_SLOT: {
			sprintf(response + 4, " ");
			break;
		}

		case SLOT_SUCESSFUL: {
			slot = -1;

			if (!strcmp(cmd_buffer + 7 + strlen("slot-successful:"), "_a"))
				slot = 0;
			else if (!strcmp(cmd_buffer + 7 + strlen("slot-successful:"), "_b"))
				slot = 1;
			else
				sprintf(response, "FAILinvalid slot");

			if (slot >= 0) {
				if (ab_slot_successful(slot))
					sprintf(response + 4, "yes");
				else
					sprintf(response + 4, "no");
			}
			break;
		}

		case SLOT_UNBOOTABLE: {
			slot = -1;

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
			break;
		}

		case SLOT_RETRY_COUNT: {
			slot = -1;

			if (!strcmp(cmd_buffer + 7 + strlen("slot-retry-count:"), "_a"))
				slot = 0;
			else if (!strcmp(cmd_buffer + 7 + strlen("slot-retry-count:"), "_b"))
				slot = 1;
			else
				sprintf(response, "FAILinvalid slot");

			if (slot >= 0)
				sprintf(response + 4, "%d", ab_slot_retry_count(slot));
			break;
		}

		case BL_UNLOCKED: {
			if (get_ldfw_load_flag()) {
				int lock_state;
				lock_state = get_lock_state();

				if (!lock_state)
					sprintf(response + 4, "yes");
				else
					sprintf(response + 4, "no");
			} else {
				sprintf(response + 4, "ignore");
			}
			break;
		}

		case UID: {
			char uid_str[33] = {0};
			uint32_t *p;
			/* default is faked UID, for test purpose only */
			uint8_t uid_buf[] = {0x41, 0xDC, 0x74, 0x4B,	\
									0x00, 0x00, 0x00, 0x00,	\
									0x00, 0x00, 0x00, 0x00,	\
									0x00, 0x00, 0x00, 0x00};
			int chip_id[2];

			get_serialno(chip_id);
			p = (uint32_t *)&uid_buf[0];
			*p = ntohl(chip_id[1]);
			p = (uint32_t *)&uid_buf[4];
			*p = ntohl(chip_id[0]);

			hex2str(uid_buf, uid_str, 16);

			sprintf(response + 4, uid_str);
			break;
		}

		case STR_RAM: {
			debug_store_ramdump_getvar(cmd_buffer + 15, response + 4);
			break;
		}

		case PARTITION_TYPE: {
			char *key;
			void *part;
			const char *type;

#if (INPUT_GPT_AS_PT == 0)
#ifdef CONFIG_USE_F2FS
			const char *str_f2fs = "f2fs";
#endif
#endif // (INPUT_GPT_AS_PT == 0)

			key = (char *)cmd_buffer + 7 + strlen("partition-type:");

			if (!part_get_pt_type(key) && strcmp(key, "wipe")) {
				part = part_get(key);
				if (!part) {
					sprintf(response, "FAILpartition does not exist");
					fastboot_send_status(response, strlen(response), FASTBOOT_TX_ASYNC);
					return 0;
				}

				type = part_get_fs_type(part);

#if (INPUT_GPT_AS_PT == 0)
				/*
				 * With pit binary change, too many process troubles must follow
				 * in old projects. So I kept partition type of userdata, F2FS,
				 * to prevent from the troubles.
				 */
#ifdef CONFIG_USE_F2FS
				if (!strcmp(key, "userdata"))
					type = str_f2fs;
#endif
#endif // (INPUT_GPT_AS_PT == 0)

				if (type)
					strcpy(response + 4, type);
				break;
		}

		case PARTITION_SIZE: {
			char *key;
			void *part;
			u64 size;

			key = (char *)cmd_buffer + 7 + strlen("partition-size:");

			part = part_get(key);
			if (part) {
				size = part_get_size_in_bytes(part);
				sprintf(response + 4, "0x%llx", size);
			}
			break;
		}

		case ALL: {
			int i, var_cnt;
			struct cmd_fastboot_variable *var_head;

			var_head = fastboot_get_var_head();
			var_cnt = fastboot_get_var_num();

			if (var_cnt == -1 || var_head == NULL) {
				strcpy(response,"FAIL");
			} else {
				for (i = 0; i < var_cnt; i++) {
					strncpy(response,"INFO", 4);
					strncpy(response + 4, var_head[i].name, strlen(var_head[i].name));
					strncpy(response + 4 + strlen(var_head[i].name), ":", 1);
					strcpy(response + 4 + strlen(var_head[i].name) + 1, var_head[i].string);

					fastboot_send_info(response, strlen(response));
				}

				strcpy(response,"OKAY");
				strcpy(response + 4,"Done!");
			}
			break;
		}

		default: {
			ret = dss_getvar_item(cmd_buffer + 7, response + 4);

			if (ret != 0)
				sprintf(response, "FAIL");
			break;
		}
	}
}

	fastboot_send_status(response, strlen(response), FASTBOOT_TX_ASYNC);
	return 0;
}

int fb_do_erase(char *cmd_buffer, unsigned int rx_sz)
{
	char buf[FB_RESPONSE_BUFFER_SIZE];
	char *response = (char *)(((unsigned long)buf + 8) & ~0x07);
	char *key = (char *)cmd_buffer + 6;
	void *part;
	int status = 1;

	if (!strcmp(key, "wipe")) {
		status = part_wipe_boot();
	} else {
		part = part_get(key);

		if (!part_get_pt_type(key) && !part) {
			sprintf(response, "FAILpartition does not exist");
			fastboot_send_status(response, strlen(response), FASTBOOT_TX_ASYNC);
			return 0;
		}

		if(partition_is_blocked(key))
		{
			strcpy(response, "FAILPartition is blocked from being erased!");
			fastboot_send_status(response, strlen(response), FASTBOOT_TX_ASYNC);
			return 0;
		}

		printf("erasing(formatting) '%s'\n", key);

		status = part_erase(part);
	}

	if (status) {
		sprintf(response,"FAILfailed to erase partition");
	} else {
		printf("partition '%s' erased\n", key);
		sprintf(response, "OKAY");
	}

	fastboot_send_status(response, strlen(response), FASTBOOT_TX_ASYNC);

	return 0;
}

static void flash_using_part(const char *key, char *response,
		u32 size, void *addr)
{
	void *part;
	unsigned long long length;
	u32 *env_val;

	/* Partiton APIs can gets data in only 512 aligned size */
	size = ROUNDUP(size, PART_SECTOR_SIZE);

	/*
	 * In case of flashing part, this should be
	 * passed unconditionally.
	 */
	if (part_get_pt_type(key)) {
		part_update(addr, size);
		print_lcd_update(FONT_GREEN, FONT_BLACK, "partition '%s' flashed", key);
		sprintf(response, "OKAY");
		return;
	}

	part = part_get(key);
	if (part)
		length = part_get_size_in_bytes(part);

	if (!part) {
		sprintf(response, "FAILpartition does not exist");
	} else if ((size > length) && (length != 0)) {
		sprintf(response, "FAILimage too large for partition");
	} else {
		if ((length != 0) && (size > length)) {
			printf("flashing '%s' failed\n", key);
			print_lcd_update(FONT_RED, FONT_BLACK, "flashing '%s' failed", key);
			sprintf(response, "FAILfailed to too large image");
		} else if (part_write_partial(part, addr, 0, size)) {
			printf("flashing '%s' failed\n", key);
			print_lcd_update(FONT_RED, FONT_BLACK, "flashing '%s' failed", key);
			sprintf(response, "FAILfailed to flash partition");
		} else {
			printf("partition '%s' flashed\n\n", key);
			print_lcd_update(FONT_GREEN, FONT_BLACK, "partition '%s' flashed", key);
			sprintf(response, "OKAY");
		}
	}

	if (!strcmp(key, "ramdisk")) {
		part = part_get("env");
		env_val = memalign(0x1000, part_get_size_in_bytes(part));
		part_read(part, env_val);

		env_val[ENV_ID_RAMDISK_SIZE] = size;
		part_write(part, env_val);

		free(env_val);
	}
}

int fb_do_flash(char *cmd_buffer, unsigned int rx_sz)
{
	char buf[FB_RESPONSE_BUFFER_SIZE];
	char *response = (char *)(((unsigned long)buf + 8) & ~0x07);
	const char *dest = cmd_buffer + 6;

	LTRACE_ENTRY;

#if defined(CONFIG_CHECK_LOCK_STATE)
	if(is_first_boot() && get_ldfw_load_flag()) {
		int lock_state;

		lock_state = get_lock_state();
		if (lock_state >= 0)
			LTRACEF_LEVEL(INFO, "Lock state: %d\n", lock_state);
		if (lock_state == 1) {
			sprintf(response, "FAILDevice is locked");
			fastboot_send_status(response, strlen(response), FASTBOOT_TX_ASYNC);
			return 1;
		}
	}
#endif
	dprintf(ALWAYS, "flash\n");

	if(partition_is_blocked(dest))
	{
		strcpy(response, "FAILPartition is blocked from being flashed!");
		fastboot_send_status(response, strlen(response), FASTBOOT_TX_ASYNC);
		return 0;
	}

	strcpy(response,"OKAY");

	if(!strcmp(dest, "boot"))
	{
		print_lcd_update(FONT_ORANGE, FONT_BLACK, "Patching lk3rd, please do not turn off/reboot your device.");

		void *part = part_get("lk3rd");
		struct pit_entry *lk3rd_entry = (struct pit_entry *)part;

		part_read(part, (void *)BOOT_BASE);

		boot_img_hdr *lk3rd = (boot_img_hdr *)BOOT_BASE;
		boot_img_hdr *android = (boot_img_hdr *)interface.transfer_buffer;

		if(strncmp((char*)android->magic, BOOT_MAGIC, 8) || strncmp((char*)lk3rd->magic, BOOT_MAGIC, 8)) {
			print_lcd_update(FONT_RED, FONT_BLACK, "Invalid boot image, skip patching!");
			goto flash;
		}

		lk3rd->os_version = android->os_version;
		print_lcd_update(FONT_ORANGE, FONT_BLACK, "OS Version / Patch Level patched, reflashing...");

		flash_using_part("lk3rd", response, lk3rd_entry->blknum * PIT_UFS_BLK_SIZE, (void *)BOOT_BASE);

		print_lcd_update(FONT_GREEN, FONT_BLACK, "Patch complete!");

		void *part_boot = part_get("boot");
		struct pit_entry *boot_entry = (struct pit_entry *)part_boot;
		if(downloaded_data_size == (boot_entry->blknum * PIT_UFS_BLK_SIZE) + 512 * PIT_UFS_BLK_SIZE)
		{
			print_lcd_update(FONT_ORANGE, FONT_BLACK, "This looks like a raw dump. Trimming off lk3rd VPart size");
			downloaded_data_size = downloaded_data_size - 512 * PIT_UFS_BLK_SIZE;
		}
	}
	else if(!strcmp(dest, "lk3rd"))
	{
		print_lcd_update(FONT_ORANGE, FONT_BLACK, "Patching LK3RD, please do not turn off/reboot your device.");

		void *part = part_get("lk3rd");

		part_read(part, (void *)BOOT_BASE);

		boot_img_hdr *lk3rd_device = (boot_img_hdr *)BOOT_BASE;
		boot_img_hdr *lk3rd_update = (boot_img_hdr *)interface.transfer_buffer;

		if(strncmp((char*)lk3rd_update->magic, BOOT_MAGIC, 8)) {
			print_lcd_update(FONT_RED, FONT_BLACK, "Invalid lk3rd image! Stopping flashing process.");
			sprintf(response, "FAILInvalid lk3rd Image");
        		fastboot_send_status(response, strlen(response), FASTBOOT_TX_ASYNC);
			return -1;
		}

		if(strncmp((char*)lk3rd_device->magic, BOOT_MAGIC, 8)) {
			print_lcd_update(FONT_RED, FONT_BLACK, "Invalid lk3rd image on device, skip patching!");
			goto flash;
		}

		lk3rd_update->os_version = lk3rd_device->os_version;
		print_lcd_update(FONT_ORANGE, FONT_BLACK, "OS Version / Patch Level patched, continuing flash...");

		flash_using_part("lk3rd", response,
			downloaded_data_size, (void *)interface.transfer_buffer);

		print_lcd_update(FONT_GREEN, FONT_BLACK, "Patch complete!");

		fastboot_send_status(response, strlen(response), FASTBOOT_TX_ASYNC);

		LTRACE_EXIT;
		return 0;
	}

flash:
	flash_using_part(dest, response,
			downloaded_data_size, (void *)interface.transfer_buffer);

	fastboot_send_status(response, strlen(response), FASTBOOT_TX_ASYNC);

	LTRACE_EXIT;
	return 0;
}

extern void fastboot_rx_datapayload(int dir, const unsigned char *addr, unsigned int len);

int fb_do_reboot(char *cmd_buffer, unsigned int rx_sz)
{
	char buf[FB_RESPONSE_BUFFER_SIZE];
	char *response = (char *)(((unsigned long)buf + 8) & ~0x07);

	sprintf(response, "OKAY");
	fastboot_send_status(response, strlen(response), FASTBOOT_TX_SYNC);

	platform_prepare_reboot();
	platform_do_reboot(cmd_buffer);

	return 0;
}

int fb_do_download(char *cmd_buffer, unsigned int rx_sz)
{
	char buf[FB_RESPONSE_BUFFER_SIZE];
	char *response = (char *)(((unsigned long)buf + 8) & ~0x07);

	LTRACE_ENTRY;
	LTRACEF_LEVEL(INFO, "%s---->>>>>  cmd: %s\n", __func__, cmd_buffer);
	/* Get download size */
	download_size = (unsigned int)strtol(cmd_buffer + 9, NULL, 16);
	downloaded_data_size = 0;

	LTRACEF_LEVEL(INFO, "Downloaing. Download size is [%d, %d] bytes\n", download_size, interface.transfer_buffer_size);

	/* Set payload phase to rx data */
	fastboot_set_payload_data(USBDIR_OUT, (void *)CFG_FASTBOOT_TRANSFER_BUFFER, download_size);

	sprintf(response, "DATA%08x", download_size);
	LTRACEF_LEVEL(INFO, "response: %s\n", response);
	fastboot_send_status(response, strlen(response), FASTBOOT_TX_ASYNC);

	LTRACE_EXIT;
	return 0;
}

static void start_ramdump(void *buffer)
{
	struct fastboot_ramdump_hdr *hdr = buffer;
	static uint32_t ramdump_cnt = 0;
	char buf[] = "OKAY";

	LTRACEF_LEVEL(INFO, "\nramdump start address is [0x%lx]\n", hdr->base);
	LTRACEF_LEVEL(INFO, "ramdump size is [0x%lx]\n", hdr->size);
	LTRACEF_LEVEL(INFO, "version is [0x%lx]\n", hdr->version);

	if (hdr->version != 2) {
		LTRACEF_LEVEL(INFO, "you are using wrong version of fastboot!!!\n");
	}

	/* dont't generate DECERR even if permission failure of ASP occurs */
	if (ramdump_cnt++ == 0)
		set_tzasc_action(0);

	fastboot_set_payload_data(USBDIR_IN, (void *)hdr->base, hdr->size);
	fastboot_send_status(buf, strlen(buf), FASTBOOT_TX_SYNC);
}

int fb_do_ramdump(char *cmd_buffer, unsigned int rx_sz)
{
	char buf[FB_RESPONSE_BUFFER_SIZE];
	char *response = (char *)(((unsigned long)buf + 8) & ~0x07);

	TRACE_ENTRY;

	print_lcd_update(FONT_GREEN, FONT_BLACK, "Got ramdump command.");
	is_ramdump = 1;
	/* Get download size */
	download_size = (unsigned int)strtol(cmd_buffer + 8, NULL, 16);
	downloaded_data_size = 0;

	LTRACEF_LEVEL(INFO, "Downloaing. Download size is %d bytes\n", download_size);

	if (download_size > interface.transfer_buffer_size) {
		download_size = 0;
		sprintf(response, "FAILdownload data size is bigger than buffer size");
	} else {
		sprintf(response, "DATA%08x", download_size);
	}
	fasboot_set_rx_sz(download_size);
	fastboot_send_status(response, strlen(response), FASTBOOT_TX_SYNC);

	LTRACE_EXIT;

	return 0;
}

int fb_do_set_active(char *cmd_buffer, unsigned int rx_sz)
{
	char buf[FB_RESPONSE_BUFFER_SIZE];
	char *response = (char *)(((unsigned long)buf + 8) & ~0x07);

	LTRACEF_LEVEL(INFO, "set_active\n");

	sprintf(response, "OKAY");

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

	fastboot_send_status(response, strlen(response), FASTBOOT_TX_ASYNC);

	return 0;
}

/* Lock/unlock device */
int fb_do_flashing(char *cmd_buffer, unsigned int rx_sz)
{
	char buf[FB_RESPONSE_BUFFER_SIZE];
	char *response = (char *)(((unsigned long)buf + 8) & ~0x07);

	sprintf(response,"OKAY");
	if (!strcmp(cmd_buffer + 9, "lock")) {
		printf("Lock this device.\n");
		print_lcd_update(FONT_GREEN, FONT_BLACK, "User has a negative IQ");
		sprintf(response, "FAILAre you serious? Why did you want to do this???");
	} else if (!strcmp(cmd_buffer + 9, "unlock")) {
		printf("Unlock this device.\n");
		print_lcd_update(FONT_GREEN, FONT_BLACK, "[2024] Knoz bypasz 100 per cent workz");
		sprintf(response, "FAILThere is no way you thought this would work.");
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

	fastboot_send_status(response, strlen(response), FASTBOOT_TX_ASYNC);

	return 0;
}

int fb_do_oem(char *cmd_buffer, unsigned int rx_sz)
{
	char buf[FB_RESPONSE_BUFFER_SIZE];
	char *response = (char *)(((unsigned long)buf + 8) & ~0x07);
	char *oem_command = cmd_buffer + 4;

	switch (oem_command_to_id(oem_command))
	{
	case OEM_STR_RAM:
		if (!debug_store_ramdump_oem(cmd_buffer + 12))
			sprintf(response, "OKAY");
		else
			sprintf(response, "FAILunsupported command");
		break;
	
	case OEM_REBOOT_DOWNLOAD:
			sprintf(response, "OKAY");
			fastboot_send_status(response, strlen(response), FASTBOOT_TX_ASYNC);
			platform_prepare_reboot();
			platform_do_reboot("reboot-download");
			break;

	default:
		sprintf(response, "FAILunsupported command");
		break;
	}

	fastboot_send_status(response, strlen(response), FASTBOOT_TX_ASYNC);

	return 0;
}

int fb_do_diskinfo(char *cmd_buffer, unsigned int rx_sz)
{
	char buf[FB_RESPONSE_BUFFER_SIZE];
	char *response = (char *)(((unsigned long)buf + 8) & ~0x07);

	u32 start_in_secs;
	u32 size_in_secs;
	char *p = (char *)cmd_buffer;

	/* offset to get range */
	p += 9;
	memcpy(resp_data, p, 8);
	*(resp_data + 8) = '\0';
	start_in_secs = (u32)strtol(resp_data, NULL, 16);

	p += 8;
	memcpy(resp_data, p, 8);
	*(resp_data + 8) = '\0';
	size_in_secs = (u32)strtol(resp_data, NULL, 16);

	/* If it fails, start_in_secs and size_in_secs would be zero */
	part_get_range_by_range(&start_in_secs, &size_in_secs);

	/* Return response BLKRANGE for 'diskinfo:' command */
	sprintf(response, "BLKRANGE%08x%08x", start_in_secs, size_in_secs);
	printf("BLKRANGE%08x%08x", start_in_secs, size_in_secs);
	fastboot_send_status(response, strlen(response), FASTBOOT_TX_SYNC);

	/* Fastboot in LK doesn't see any result */
	return 0;
}

int fb_do_partinfo(char *cmd_buffer, unsigned int rx_sz)
{
	char buf[FB_RESPONSE_BUFFER_SIZE];
	char *response = (char *)(((unsigned long)buf + 8) & ~0x07);
	char *name = (char *)cmd_buffer;

	u32 start_in_secs;
	u32 size_in_secs;

	/* offset to get partition name */
	name += 9;

	/* If it fails, start_in_secs and size_in_secs would be zero */
	part_get_range_by_name(name, &start_in_secs, &size_in_secs);

	/* Return response BLKRANGE for 'partinfo:' command */
	printf("BLKRANGE%08x%08x\n", start_in_secs, size_in_secs);
	sprintf(response, "BLKRANGE%08x%08x", start_in_secs, size_in_secs);
	fastboot_send_status(response, strlen(response), FASTBOOT_TX_SYNC);

	/* Fastboot in LK doesn't see any result */
	return 0;
}

static u32 start_diskdump(void *buf, u32 start_in_secs, u32 size_in_bytes)
{
	u32 size_in_secs = size_in_bytes / PART_SECTOR_SIZE;

	printf("\ndiskdump start: %p, 0x%x, 0x%x\n", buf, start_in_secs, size_in_bytes);

	/* Read disk */
	if (part_read_raw(buf, start_in_secs, &size_in_secs)) {
		/* Target disk read */
		printf("Failed to read disk !\n");
		return 0xFFFFFFFF;
	}

	/* Transfer data to host */
	fastboot_send_payload(buf, size_in_secs * PART_SECTOR_SIZE);

	return size_in_secs;
}
void muic_sw_usb(void);

int fb_do_diskdump(char *cmd_buffer, unsigned int rx_sz)
{
	char buf[FB_RESPONSE_BUFFER_SIZE];
	char *response = (char *)(((unsigned long)buf + 8) & ~0x07);

	u32 start_in_secs;
	u32 size_in_secs;
	u32 done;
	int res = -1;
	char *p = (char *)cmd_buffer;

	/* Print */
	printf("\nGot diskdump command\n");
	print_lcd_update(FONT_GREEN, FONT_BLACK, "Got diskdump command.");

	/* Get arguments from command */
	p += 9;
	memcpy(resp_data, p, 8);
	*(resp_data + 8) = '\0';
	start_in_secs = (u32)strtol(resp_data, NULL, 16);

	p += 8;
	memcpy(resp_data, p, 8);
	*(resp_data + 8) = '\0';
	size_in_secs = (u32)strtol(resp_data, NULL, 16);

	printf("Starting download of %u blocks from start_in_secs %u\n", size_in_secs, start_in_secs);

	/* Check */
	if (0 == size_in_secs)
		sprintf(response, "FAILdata invalid size");
	else if (size_in_secs * PART_SECTOR_SIZE > interface.transfer_buffer_size)
		sprintf(response, "FAILdata too large: %u > %u",
				size_in_secs * PART_SECTOR_SIZE, interface.transfer_buffer_size);
	else {
		sprintf(response, "OKAY");
		res = 0;
	}

	s_fb_on_diskdump = 1;

	fastboot_tx_event_init();
	/* Return response for 'diskdump:' command */
	fastboot_send_info(response, strlen(response));

	mdelay(200);
#if defined(CONFIG_BOARD_UNIVERSAL9630)
	muic_sw_usb();
#endif

	if (res == 0) {
		done = start_diskdump((void *)CFG_FASTBOOT_TRANSFER_BUFFER, start_in_secs, size_in_secs * PART_SECTOR_SIZE);

		/* Return BLKCOUNT to host */
		printf("BLKCOUNT%08x\n", done);
		sprintf(response, "BLKCOUNT%08x", done);
		s_fb_on_diskdump = 0;
		fastboot_send_status(response, strlen(response), FASTBOOT_TX_SYNC);
	}
	s_fb_on_diskdump = 0;

	/* Fastboot in LK doesn't see any result */
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
	{"oem", fb_do_oem},
	{"diskinfo:", fb_do_diskinfo},
	{"partinfo:", fb_do_partinfo},
	{"diskdump:", fb_do_diskdump},
};

int rx_handler(const unsigned char *buffer, unsigned int buffer_size)
{
	unsigned int i;
	char *cmd_buffer;

	LTRACE_ENTRY;

	cmd_buffer = (char *)buffer;

	printf("fb: %s\n", cmd_buffer);	/* test: probing protocol */

	if (is_ramdump) {
		is_ramdump = 0;

		start_ramdump((void *)buffer);
	} else {
		for (i = 0; i < (sizeof(cmd_list) / sizeof(struct cmd_fastboot)); i++) {
			if(!memcmp(cmd_buffer, cmd_list[i].cmd_name, strlen(cmd_list[i].cmd_name)))
				cmd_list[i].handler(cmd_buffer, buffer_size);
		}
	}

	LTRACE_EXIT;
	return 0;
}

void fb_cmd_set_downloaded_sz(unsigned int sz)
{
	downloaded_data_size += sz;
}
