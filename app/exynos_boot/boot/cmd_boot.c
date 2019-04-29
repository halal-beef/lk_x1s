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
#include <ctype.h>
#include <stdlib.h>
#include <reg.h>
#include <libfdt.h>
#include <lib/bio.h>
#include <lib/console.h>
#include <lib/fastboot.h>
#include <part_gpt.h>
#include <dev/boot.h>
#include <dev/rpmb.h>
#include <platform/exynos9830.h>
#include <platform/smc.h>
#include <platform/sfr.h>
#include <platform/ldfw.h>
#include <platform/charger.h>
#include <lib/ab_update.h>
#include <platform/secure_boot.h>
#include <platform/sizes.h>
#include <platform/bootimg.h>
#include <platform/fdt.h>
#include <platform/chip_id.h>
#include <pit.h>
#include <dev/scsi.h>
#include <dev/mmc.h>

/* Memory node */
#define SIZE_2GB		(0x80000000)
#define SIZE_1GB		(0x40000000)
#define SIZE_500MB		(0x20000000)
#define MASK_1MB		(0x100000 - 1)

#define BUFFER_SIZE		2048

#define REBOOT_MODE_RECOVERY	0xFF
#define REBOOT_MODE_FACTORY	0xFD

void configure_ddi_id(void);
void arm_generic_timer_disable(void);

#if defined(CONFIG_USE_AVB20)
static char cmdline[AVB_CMD_MAX_SIZE];
static char verifiedbootstate[AVB_VBS_MAX_SIZE] = "androidboot.verifiedbootstate=";
#endif

struct bootargs_prop {
	char prop[64];
	char val[64];
};
static struct bootargs_prop prop[32] = { { { 0, }, { 0, } }, };
static int prop_cnt = 0;

static int bootargs_init(void)
{
	u32 i = 0;
	u32 len = 0;
	u32 cur = 0;
	u32 is_val = 0;
	char bootargs[BUFFER_SIZE];
	char bootargs_ext[BUFFER_SIZE];
	int len_bootargs;
	int len_bootargs_ext;
	const char *np;
	int noff;
	int ret;

	ret = fdt_check_header(fdt_dtb);
	if (ret) {
		printf("libfdt fdt_check_header(): %s\n", fdt_strerror(ret));
		return ret;
	}

	memset(bootargs, 0, BUFFER_SIZE);
	memset(bootargs_ext, 0, BUFFER_SIZE);

	noff = fdt_path_offset(fdt_dtb, "/chosen");
	if (noff >= 0) {
		np = fdt_getprop(fdt_dtb, noff, "bootargs", &len_bootargs);
		if (len_bootargs >= 0) {
			memcpy(bootargs, np, len_bootargs);
		}
		np = fdt_getprop(fdt_dtb, noff, "bootargs_ext", &len_bootargs_ext);
		if (len_bootargs_ext >= 0) {
			memcpy(bootargs_ext, np, len_bootargs_ext);
		}

	}

	printf("\ndefault bootargs: %s\n", bootargs);

	len = strlen(bootargs_ext);
	for (i = 0; i < len; i++) {
		if (bootargs_ext[i] == '=') {
			prop[prop_cnt].prop[cur++] = '\0';
			is_val = 1;
			cur = 0;
		} else if (bootargs_ext[i] == ' ') {
			prop[prop_cnt].val[cur++] = '\0';
			is_val = 0;
			cur = 0;
			prop_cnt++;
		} else {
			if (is_val)
				prop[prop_cnt].val[cur++] = bootargs_ext[i];
			else
				prop[prop_cnt].prop[cur++] = bootargs_ext[i];
		}
	}

	cur = 0;
	is_val = 0;
	prop_cnt++;

	len = strlen(bootargs);
	for (i = 0; i < len; i++) {
		if (bootargs[i] == '=') {
			prop[prop_cnt].prop[cur++] = '\0';
			is_val = 1;
			cur = 0;
		} else if (bootargs[i] == ' ') {
			prop[prop_cnt].val[cur++] = '\0';
			is_val = 0;
			cur = 0;
			prop_cnt++;
		} else {
			if (is_val)
				prop[prop_cnt].val[cur++] = bootargs[i];
			else
				prop[prop_cnt].prop[cur++] = bootargs[i];
		}
	}

	return 0;
}

static char *get_bootargs_val(const char *name)
{
	int i = 0;

	for (i = 0; i <= prop_cnt; i++) {
		if (strncmp(prop[i].prop, name, strlen(name)) == 0)
			return prop[i].val;
	}

	return NULL;
}

static void update_val(const char *name, const char *val)
{
	int i = 0;

	for (i = 0; i <= prop_cnt; i++) {
		if (strncmp(prop[i].prop, name, strlen(name)) == 0) {
			sprintf(prop[i].val, "%s", val);
			return;
		}
	}
}

static void bootargs_update(void)
{
	int i = 0;
	int cur = 0;
	char bootargs[BUFFER_SIZE];

	memset(bootargs, 0, sizeof(bootargs));

	for (i = 0; i <= prop_cnt; i++) {
		if (0 == strlen(prop[i].val)) {
			sprintf(bootargs + cur, "%s", prop[i].prop);
			cur += strlen(prop[i].prop);
			snprintf(bootargs + cur, 2, " ");
			cur += 1;
		} else {
			sprintf(bootargs + cur, "%s=%s", prop[i].prop, prop[i].val);
			cur += strlen(prop[i].prop) + strlen(prop[i].val) + 1;
			snprintf(bootargs + cur, 2, " ");
			cur += 1;
		}
	}

	bootargs[cur] = '\0';

	printf("\nupdated bootargs: %s\n", bootargs);

	set_fdt_val("/chosen", "bootargs", bootargs);
}

static void remove_string_from_bootargs(const char *str)
{
	char bootargs[BUFFER_SIZE];
	const char *np;
	int noff;
	int bootargs_len;
	int str_len;
	int i;

	noff = fdt_path_offset(fdt_dtb, "/chosen");
	np = fdt_getprop(fdt_dtb, noff, "bootargs", &bootargs_len);

	str_len = strlen(str);

	for (i = 0; i < bootargs_len - str_len; i++)
		if (!strncmp(str, (np + i), str_len))
			break;

	memset(bootargs, 0, BUFFER_SIZE);
	memcpy(bootargs, np, i);
	memcpy(bootargs + i, np + i + str_len, bootargs_len - i - str_len);

	fdt_setprop(fdt_dtb, noff, "bootargs", bootargs, strlen(bootargs) + 1);
}

static void set_bootargs(void)
{
	bootargs_init();

	/* update_val("console", "ttySAC0,115200"); */

	bootargs_update();
}

static void configure_dtb(void)
{
	char str[BUFFER_SIZE];
	u32 soc_ver = 0;
	u64 dram_size = *(u64 *)BL_SYS_INFO_DRAM_SIZE;
	unsigned long sec_dram_base = 0;
	unsigned int sec_dram_size = 0;
	unsigned long sec_dram_end = 0;
	unsigned long sec_pt_base = 0;
	unsigned int sec_pt_size = 0;
	unsigned long sec_pt_end = 0;
	int len;
	const char *np;
	int noff;
	struct boot_img_hdr *b_hdr = (boot_img_hdr *)BOOT_BASE;
#if defined(CONFIG_USE_AVB20)
	struct AvbOps *ops;
	bool unlock;
#endif

	/* Get Secure DRAM information */
	soc_ver = exynos_smc(SMC_CMD_GET_SOC_INFO, SOC_INFO_TYPE_VERSION, 0, 0);
	if (soc_ver == SOC_INFO_VERSION(SOC_INFO_MAJOR_VERSION, SOC_INFO_MINOR_VERSION)) {
		sec_dram_base = exynos_smc(SMC_CMD_GET_SOC_INFO,
		                           SOC_INFO_TYPE_SEC_DRAM_BASE,
		                           0,
		                           0);
		if (sec_dram_base == (unsigned long)ERROR_INVALID_TYPE) {
			printf("get secure memory base addr error!!\n");
			while (1)
				;
		}

		sec_dram_size = (unsigned int)exynos_smc(SMC_CMD_GET_SOC_INFO,
		                                         SOC_INFO_TYPE_SEC_DRAM_SIZE,
		                                         0,
		                                         0);
		if (sec_dram_size == (unsigned int)ERROR_INVALID_TYPE) {
			printf("get secure memory size error!!\n");
			while (1)
				;
		}
	} else {
		printf("[ERROR] el3_mon is old version. (0x%x)\n", soc_ver);
		while (1)
			;
	}

	sec_dram_end = sec_dram_base + sec_dram_size;

	printf("SEC_DRAM_BASE[%#lx]\n", sec_dram_base);
	printf("SEC_DRAM_SIZE[%#x]\n", sec_dram_size);

	/* Get secure page table for DRM information */
	sec_pt_base = exynos_smc(SMC_DRM_GET_SOC_INFO,
	                         SOC_INFO_SEC_PGTBL_BASE,
	                         0,
	                         0);
	if (sec_pt_base == ERROR_DRM_INVALID_TYPE) {
		printf("[SEC_PGTBL_BASE] Invalid type\n");
		sec_pt_base = 0;
	} else if (sec_pt_base == ERROR_DRM_FW_INVALID_PARAM) {
		printf("[SEC_PGTBL_BASE] Do not support SMC for SMC_DRM_GET_SOC_INFO\n");
		sec_pt_base = 0;
	} else if (sec_pt_base == (unsigned long)ERROR_NO_DRM_FW_INITIALIZED) {
		printf("[SEC_PGTBL_BASE] DRM LDFW is not initialized\n");
		sec_pt_base = 0;
	} else if (sec_pt_base & MASK_1MB) {
		printf("[SEC_PGTBL_BASE] Not aligned with 1MB\n");
		sec_pt_base = 0;
	}

	sec_pt_size = (unsigned int)exynos_smc(SMC_DRM_GET_SOC_INFO,
	                                       SOC_INFO_SEC_PGTBL_SIZE,
	                                       0,
	                                       0);
	if (sec_pt_size == ERROR_DRM_INVALID_TYPE) {
		printf("[SEC_PGTBL_SIZE] Invalid type\n");
		sec_pt_size = 0;
	} else if (sec_pt_size == ERROR_DRM_FW_INVALID_PARAM) {
		printf("[SEC_PGTBL_SIZE] Do not support SMC for SMC_DRM_GET_SOC_INFO\n");
		sec_pt_size = 0;
	} else if (sec_pt_size == (unsigned int)ERROR_NO_DRM_FW_INITIALIZED) {
		printf("[SEC_PGTBL_SIZE] DRM LDFW is not initialized\n");
		sec_pt_size = 0;
	} else if (sec_pt_base & MASK_1MB) {
		printf("[SEC_PGTBL_SIZE] Not aligned with 1MB\n");
		sec_pt_size = 0;
	}

	sec_pt_end = sec_pt_base + sec_pt_size;

	printf("SEC_PGTBL_BASE[%#lx]\n", sec_pt_base);
	printf("SEC_PGTBL_SIZE[%#x]\n", sec_pt_size);

	/* DT control code must write after this function call. */
	merge_dto_to_main_dtb();
	resize_dt(SZ_4K);

	/* Disable CCI unit for USB */
	exynos_usb_cci_control(0);


	if (readl(EXYNOS9830_POWER_SYSIP_DAT0) == REBOOT_MODE_RECOVERY) {
		sprintf(str, "<0x%x>", RAMDISK_BASE);
		set_fdt_val("/chosen", "linux,initrd-start", str);

		sprintf(str, "<0x%x>", RAMDISK_BASE + b_hdr->ramdisk_size);
		set_fdt_val("/chosen", "linux,initrd-end", str);
	} else if (readl(EXYNOS9830_POWER_SYSIP_DAT0) == REBOOT_MODE_FACTORY) {
		noff = fdt_path_offset (fdt_dtb, "/chosen");
		np = fdt_getprop(fdt_dtb, noff, "bootargs", &len);
		snprintf(str, BUFFER_SIZE, "%s %s", np, "androidboot.mode=factory");
		fdt_setprop(fdt_dtb, noff, "bootargs", str, strlen(str) + 1);
		printf("Enter factory mode...");
	}

	/* Secure memories are carved-out in case of EVT1 */
	/*
	 * 1st DRAM node
	 */
	add_dt_memory_node(DRAM_BASE,
	                   sec_dram_base - DRAM_BASE);
	/*
	 * 2nd DRAM node
	 */
	if (sec_pt_base && sec_pt_size) {
		add_dt_memory_node(sec_dram_end,
		                   sec_pt_base - sec_dram_end);

		if (dram_size >= SIZE_2GB) {
			add_dt_memory_node(sec_pt_end,
			                   (DRAM_BASE + SIZE_2GB)
			                   - sec_pt_end);
		} else {
			add_dt_memory_node(sec_pt_end,
			                   (DRAM_BASE + dram_size)
			                   - sec_pt_end);
		}
	} else {
		if (dram_size >= SIZE_2GB) {
			add_dt_memory_node(sec_dram_end,
			                   (DRAM_BASE + SIZE_2GB)
			                   - sec_dram_end);
		} else {
			add_dt_memory_node(sec_dram_end,
			                   (DRAM_BASE + dram_size)
			                   - sec_dram_end);
		}
	}

	/*
	 * 3rd DRAM node
	 */
	if (dram_size <= SIZE_2GB)
		goto mem_node_out;

	for (u64 i = 0; i < dram_size - SIZE_2GB; i += SIZE_500MB) {
		/* add 500MB mem node */
		add_dt_memory_node(DRAM_BASE2 + i, SIZE_500MB);
	}

	set_bootargs();
mem_node_out:

	sprintf(str, "<0x%x>", RAMDISK_BASE);
	set_fdt_val("/chosen", "linux,initrd-start", str);

	sprintf(str, "<0x%x>", RAMDISK_BASE + b_hdr->ramdisk_size);
	set_fdt_val("/chosen", "linux,initrd-end", str);

	if (b_hdr->cmdline[0] && (!b_hdr->cmdline[BOOT_ARGS_SIZE - 1])) {
		noff = fdt_path_offset(fdt_dtb, "/chosen");
		np = fdt_getprop(fdt_dtb, noff, "bootargs", &len);
		snprintf(str, BUFFER_SIZE, "%s %s", np, b_hdr->cmdline);
		fdt_setprop(fdt_dtb, noff, "bootargs", str, strlen(str) + 1);
	}

#if defined(CONFIG_USE_AVB20)
	if (readl(EXYNOS9830_POWER_SYSIP_DAT0) != REBOOT_MODE_RECOVERY) {
		/* set AVB args */
		get_ops_addr(&ops);
		ops->read_is_device_unlocked(ops, &unlock);
		noff = fdt_path_offset (fdt_dtb, "/chosen");
		np = fdt_getprop(fdt_dtb, noff, "bootargs", &len);
		snprintf(str, BUFFER_SIZE, "%s %s %s", np, cmdline, verifiedbootstate);
		fdt_setprop(fdt_dtb, noff, "bootargs", str, strlen(str) + 1);
	}
#endif
	if (readl(EXYNOS9830_POWER_SYSIP_DAT0) == REBOOT_MODE_RECOVERY) {
		/* Set bootargs for recovery mode */
		remove_string_from_bootargs("skip_initramfs ");
		remove_string_from_bootargs("ro init=/init ");

		noff = fdt_path_offset (fdt_dtb, "/chosen");
		np = fdt_getprop(fdt_dtb, noff, "bootargs", &len);
		snprintf(str, BUFFER_SIZE, "%s %s", np, "root=/dev/ram0");
		fdt_setprop(fdt_dtb, noff, "bootargs", str, strlen(str) + 1);
	}

	printf("\nbootargs\n");
	noff = fdt_path_offset(fdt_dtb, "/chosen");
	printf("\fdt_getprop\n");
	np = fdt_getprop(fdt_dtb, noff, "bootargs", &len);
	printf("\nbootargs: %s\n", np);

	resize_dt(0);
}

int cmd_scatter_load_boot(int argc, const cmd_args *argv);

int load_boot_images(void)
{
	struct pit_entry *ptn;
	cmd_args argv[6];

	if (readl(EXYNOS9830_POWER_SYSIP_DAT0) == REBOOT_MODE_RECOVERY || readl(EXYNOS9830_POWER_SYSIP_DAT0) == REBOOT_MODE_FACTORY) {
		ptn = pit_get_part_info("recovery");
		if (ptn == 0) {
			printf("Partition 'recovery' does not exist\n");
			return -1;
		} else {
			pit_access(ptn, PIT_OP_LOAD, (u64)BOOT_BASE, 0);
		}
	} else {
		ptn = pit_get_part_info("boot");
		if (ptn == 0) {
			printf("Partition 'boot' does not exist\n");
			return -1;
		} else {
			pit_access(ptn, PIT_OP_LOAD, (u64)BOOT_BASE, 0);
		}
	}

#ifndef CONFIG_DTB_IN_BOOT
	ptn = pit_get_part_info("dtb");
	if (ptn == 0) {
		printf("Partition 'dtb' does not exist\n");
		return -1;
	} else {
		pit_access(ptn, PIT_OP_LOAD, (u64)DT_BASE, 0);
	}
#endif
	ptn = pit_get_part_info("dtbo");
	if (ptn == 0) {
		printf("Partition 'dtbo' does not exist\n");
		return -1;
	} else {
		pit_access(ptn, PIT_OP_LOAD, (u64)DTBO_BASE, 0);
	}

	argv[1].u = BOOT_BASE;
	argv[2].u = KERNEL_BASE;
#if defined(CONFIG_RAMDISK_IN_BOOT)
	argv[3].u = RAMDISK_BASE;
#else
	argv[3].u = 0;
#endif
#if defined(CONFIG_DTB_IN_BOOT)
	argv[4].u = DT_BASE;
#else
	argv[4].u = 0;
#endif
	argv[5].u = 0;
	cmd_scatter_load_boot(6, argv);

	return 0;
}

int cmd_boot(int argc, const cmd_args *argv)
{
	fdt_dtb = (struct fdt_header *)DT_BASE;
	dtbo_table = (struct dt_table_header *)DTBO_BASE;

	load_boot_images();

#if defined(CONFIG_USE_AVB20)
#if defined(CONFIG_AVB_ABUPDATE)
	if (ab_current_slot())
		avb_main("_b", cmdline, verifiedbootstate);
	else
		avb_main("_a", cmdline, verifiedbootstate);
#else
	avb_main("", cmdline, verifiedbootstate);
#endif
#endif

	configure_dtb();
	configure_ddi_id();

	printf("scsi_do_ssu\n");
	/*
	 * PON (Power off notification) to storage
	 *
	 * Even with its failure, subsequential operations should be executed.
	 */
	scsi_do_ssu();

	/* power off sd slot before starting kernel */
	printf("mmc_power_off\n");
	mmc_power_set(2, 0);

	if (readl(EXYNOS9830_POWER_SYSIP_DAT0) == REBOOT_MODE_RECOVERY ||
	    readl(EXYNOS9830_POWER_SYSIP_DAT0) == REBOOT_MODE_FACTORY)
		writel(0, EXYNOS9830_POWER_SYSIP_DAT0);
	/* notify EL3 Monitor end of bootloader */
	exynos_smc(SMC_CMD_END_OF_BOOTLOADER, 0, 0, 0);


	/* before jumping to kernel. disble arch_timer */
	arm_generic_timer_disable();

	printf("Starting kernel...\n");
	void (*kernel_entry)(int r0, int r1, int r2, int r3);

	kernel_entry = (void (*)(int, int, int, int))KERNEL_BASE;
	kernel_entry(DT_BASE, 0, 0, 0);

	return 0;
}

STATIC_COMMAND_START
	STATIC_COMMAND("boot", "start kernel booting", &cmd_boot)
STATIC_COMMAND_END(boot);
