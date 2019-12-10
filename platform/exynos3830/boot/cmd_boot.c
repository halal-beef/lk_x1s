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
#include <lib/font_display.h>
#include <lib/fdtapi.h>
#include <dev/boot.h>
#include <dev/rpmb.h>
#include <dev/usb/gadget.h>
#include <platform/mmu/mmu_func.h>
#include <platform/sfr.h>
#include <platform/smc.h>
#include <platform/sfr.h>
#include <platform/ldfw.h>
#include <platform/charger.h>
#include <platform/ab_update.h>
#include <platform/secure_boot.h>
#include <platform/sizes.h>
#include <lib/fastboot.h>
#include <platform/bootimg.h>
#include <platform/chip_id.h>
#include <platform/gpio.h>
#include <part.h>
#include <dev/scsi.h>

/* Memory node */
#define SIZE_2GB 	(0x80000000)
#define SIZE_500MB	(0x20000000)
#define MASK_1MB	(0x100000 - 1)

#define BUFFER_SIZE 2048
#define BOOTARGS_ITEM_SIZE	128

#define be32_to_cpu(x) \
		((((x) & 0xff000000) >> 24) | \
		 (((x) & 0x00ff0000) >>  8) | \
		 (((x) & 0x0000ff00) <<  8) | \
		 (((x) & 0x000000ff) << 24))

void configure_ddi_id(void);
void arm_generic_timer_disable(void);

#if defined(CONFIG_USE_AVB20)
static char cmdline[AVB_CMD_MAX_SIZE];
static char verifiedbootstate[AVB_VBS_MAX_SIZE]="androidboot.verifiedbootstate=";
#endif

static void update_boot_reason(char *buf)
{
	u32 val = readl(CONFIG_RAMDUMP_REASON);
	unsigned int rst_stat = readl(EXYNOS_POWER_RST_STAT);

	if (rst_stat & WARM_RESET)
		snprintf(buf, 16, "hard_reset");
	else if (rst_stat & PIN_RESET)
		snprintf(buf, 16, "coldboot");
	else if (rst_stat & (LITTLE_WDT_RESET | BIG_WDT_RESET))
		snprintf(buf, 16, "watchdog");
	else if (rst_stat & SWRESET)
		if (val == RAMDUMP_SIGN_PANIC)
			snprintf(buf, 16, "kernel_panic");
		else
			snprintf(buf, 16, "reboot");
	else
		snprintf(buf, 16, "unknown");
}

struct bootargs_prop {
	char prop[BOOTARGS_ITEM_SIZE];
	char val[BOOTARGS_ITEM_SIZE];
};
static struct bootargs_prop prop[64] = { { {0, }, {0, } }, };
static int prop_cnt = 0;
extern char dtbo_idx[4];

static int bootargs_init(void)
{
	u32 i = 0;
	u32 len = 0;
	u32 cur = 0;
	u32 cnt = 0;
	u32 is_val = 0;
	char bootargs[BUFFER_SIZE];
	int len2;
	const char *np;
	int noff;
	int ret;

	ret = fdt_check_header(fdt_dtb);
	if (ret) {
		printf("libfdt fdt_check_header(): %s\n", fdt_strerror(ret));
		return ret;
	}

	noff = fdt_path_offset(fdt_dtb, "/chosen");
	if (noff >= 0) {
		np = fdt_getprop(fdt_dtb, noff, "bootargs", &len2);
		if (len2 >= 0) {
			memset(bootargs, 0, BUFFER_SIZE);
			memcpy(bootargs, np, len2);
		}
	}

	printf("\ndefault bootargs: %s\n", bootargs);

	len = strlen(bootargs);
	for (i = 0; i < len; i++) {
		if (bootargs[i] == '=') {
			is_val = 1;
			if (cnt > 0)
				prop[prop_cnt].val[cur++] = bootargs[i];
			else {
				cnt++;
				prop[prop_cnt].prop[cur++] = '\0';
				cur = 0;
			}
		} else if (bootargs[i] == ' ') {
			prop[prop_cnt].val[cur++] = '\0';
			is_val = 0;
			cur = 0;
			prop_cnt++;
			cnt = 0;
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
		if (0 == strlen(prop[i].prop) && 0 == strlen(prop[i].val)) {
			/* No prop and val pair. ignore it */
			continue;
		}

		if (0 == strlen(prop[i].val)) {
			sprintf(bootargs + cur, "%s", prop[i].prop);
			cur += strlen(prop[i].prop);
			snprintf(bootargs + cur, 2, " ");
			cur += 1;
		} else if (0 != strlen(prop[i].prop)) {
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

static int add_val(const char *key, const char *val)
{
	if (key == NULL) {
		printf("Wrong input property is NULL\n");
		return -1;
	}

	prop_cnt++;
	strcpy(prop[prop_cnt].prop, key);

	if (val == NULL)
		prop[prop_cnt].val[0] = '\0';
	else
		strcpy(prop[prop_cnt].val, val);

	return 0;
}

static int remove_val(const char *key, const char *val)
{
	int i;
	int find = -1;

	for (i=0; i<=prop_cnt; i++) {
		if (strcmp(key, prop[i].prop) == 0) {
			if ((val == NULL) && (strlen(prop[i].val) == 0)) {
				memset(prop[i].prop, 0, BOOTARGS_ITEM_SIZE);
				memset(prop[i].val, 0, BOOTARGS_ITEM_SIZE);
				find = 0;
			} else if ((val != NULL) &&
				   (strcmp(val, prop[i].val) == 0)) {
				memset(prop[i].prop, 0, BOOTARGS_ITEM_SIZE);
				memset(prop[i].val, 0, BOOTARGS_ITEM_SIZE);
				find = 0;
			}
		}
	}

	return find;
}

static void print_val(void)
{
	int i;

	printf("Total number of ITEMs : %d\n", prop_cnt);
	for (i=0 ; i<=prop_cnt ; i++) {
		printf("  [%02d]: key(%s), val(%s)\n", i, prop[i].prop, prop[i].val);
	}
}

/*
 * Remove_string_from_bootargs
 * This function is will be depricated
 */
static void remove_string_from_bootargs(const char *str)
{
	char bootargs[BUFFER_SIZE];
	const char *np;
	int noff;
	int bootargs_len;
	int str_len;
	int i;
	int flag = 0;

	noff = fdt_path_offset(fdt_dtb, "/chosen");
	np = fdt_getprop(fdt_dtb, noff, "bootargs", &bootargs_len);

	str_len = strlen(str);

	for (i = 0; i <= bootargs_len - str_len; i++) {
		if(!strncmp(str, (np + i), str_len)) {
			flag = 1;
			break;
		}
	}

	if (!flag) {
		printf("%sis not in bootargs\n", str);
		return;
	}

	memset(bootargs, 0, BUFFER_SIZE);
	memcpy(bootargs, np, i);
	memcpy(bootargs + i, np + i + str_len, bootargs_len - i - str_len);

	fdt_setprop(fdt_dtb, noff, "bootargs", bootargs, strlen(bootargs) + 1);
}

static int bootargs_process(void)
{
	char buf[16];
	int ret = 0;

	/* Below console value can be used for bootargs change */
	/* update_val("console", "ttySAC0,115200n8"); */
	update_val("androidboot.dtbo_idx", dtbo_idx);

	/* reason */
	memset(buf, 0, sizeof(buf));
	update_boot_reason(buf);
	if (add_val("androidboot.bootreason", buf)) {
		printf("Add bootreason failed\n");
		return -1;
	}

	/* mode: Factory mode */
	if (readl(EXYNOS_POWER_SYSIP_DAT0) == REBOOT_MODE_FACTORY) {
		if (add_val("androidboot.mode", "sfactory")) {
			printf("bootmode set sfactory failed\n");
			return -1;
		}
		printf("Enter samsung factory mode...");
		print_lcd_update(FONT_GREEN, FONT_BLACK, "Enter samsung factory mode...");
	}

	/* mode: Charger mode - decision : Pin reset && ACOK && !Factory mode */
	if (get_charger_mode() && readl(EXYNOS_POWER_SYSIP_DAT0) != REBOOT_MODE_FACTORY) {
		if (add_val("androidboot.mode", "charger")) {
			printf("bootmode set charger failed\n");
			return -1;
		}
		printf("Enter charger mode...");
		print_lcd_update(FONT_GREEN, FONT_BLACK, "Enter charger mode...");
	}

	/* slot_suffix: AB booting slot */
	ret = ab_current_slot();
	if (ret != AB_ERROR_NOT_SUPPORT) {
		memset(buf, 0, sizeof(buf));
		snprintf(buf, 15, "%s", (ret == AB_SLOT_B) ? "_b" : "_a");
		if (add_val("androidboot.slot_suffix", buf)) {
			printf("slot_suffix set failed\n");
			return -1;
		}
		printf("AB Slot suffix set %s", buf);
		print_lcd_update(FONT_GREEN, FONT_BLACK, "AB Slot suffix set %s", buf);
	}

	/* Add Board_rev to bootargs */
	memset(buf, 0, sizeof(buf));
	snprintf(buf, 5, "0x%x", board_rev);
	if (add_val("revision", buf)) {
	        printf("Board Revision set failed\n");
	        return -1;
	}

	/* Recovery */
	if (readl(EXYNOS_POWER_SYSIP_DAT0) == REBOOT_MODE_RECOVERY) {
		/* remove some bootargs to Set recovery boot mode */
		if(remove_val("skip_initramfs", NULL))
			printf("bootargs cannot delete, checkit: skip_initramfs\n");
		if(remove_val("ro", NULL))
			printf("bootargs cannot delete, checkit: ro\n");
		if(remove_val("init", "/init"))
			printf("bootargs cannot delete, checkit: init\n");

		/* set root to ramdisk */
		if (add_val("root", "/dev/ram0")) {
			printf("reocvery ramdisk set failed\n");
			return -1;
		}
	}

	return 0;
}

static void set_bootargs(void)
{
	bootargs_init();

	/* add bootargs for bootmode reason, etc */
	if (bootargs_process()) {
		printf("ERR: bootargs process failed!");
	}

	/* bootargs can be checked with print func */
	/* print_val(); */
	bootargs_update();
}

static void set_usb_serialno(void)
{
	char str[BUFFER_SIZE];
	const char *np;
	int len;
	int noff;
	unsigned long tmp_serial_id = 0;
	char *ret;

	tmp_serial_id = ((unsigned long)s5p_chip_id[1] << 32) | (s5p_chip_id[0]);

	printf("Set USB serial number in bootargs.(%016lx)\n", tmp_serial_id);

	ret = get_bootargs_val("androidboot.serialno");

	if(!ret) {
		printf("no serial number prop");
		noff = fdt_path_offset(fdt_dtb, "/chosen");
		np = fdt_getprop(fdt_dtb, noff, "bootargs", &len);
		snprintf(str, BUFFER_SIZE, "%s androidboot.serialno=%016lx",
							np, tmp_serial_id);
		fdt_setprop(fdt_dtb, noff, "bootargs", str, strlen(str) + 1);
	} else {
		printf("already have serial prop: ret = %s\n", ret);
	}

	noff = fdt_path_offset (fdt_dtb, "/chosen");
	np = fdt_getprop(fdt_dtb, noff, "bootargs", &len);
	snprintf(str, BUFFER_SIZE, "%s androidboot.serialno=%016lx",
						np, tmp_serial_id);
	fdt_setprop(fdt_dtb, noff, "bootargs", str, strlen(str) + 1);
}

static void configure_dtb(void)
{
	char str[BUFFER_SIZE];

	const char *np;
	int len, noff;
	struct boot_img_hdr *b_hdr = (boot_img_hdr *)BOOT_BASE;

	u32 soc_ver = 0;
	u64 dram_size = *(u64 *)BL_SYS_INFO_DRAM_SIZE;
	unsigned long sec_dram_base = 0;
	unsigned int sec_dram_size = 0;
	unsigned long sec_dram_end = 0;
	unsigned long sec_pt_base = 0;
	unsigned int sec_pt_size = 0;
	unsigned long sec_pt_end = 0;

	/*
	 * In this here, it is enabled cache. So you don't use blk read/write function
	 * If you modify dtb, you must use under set_bootargs function.
	 * And if you modify bootargs, you will modify in set_bootargs function.
	 */
	merge_dto_to_main_dtb(board_id, board_rev);
	resize_dt(SZ_4K);


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

mem_node_out:

	sprintf(str, "<0x%x>", ECT_BASE);
	set_fdt_val("/ect", "parameter_address", str);

	sprintf(str, "<0x%x>", ECT_SIZE);
	set_fdt_val("/ect", "parameter_size", str);

	/* add initrd-start end value */
	memset(str, 0, BUFFER_SIZE);
	sprintf(str, "<0x%x>", RAMDISK_BASE);
	set_fdt_val("/chosen", "linux,initrd-start", str);
	printf("initrd-start: %s\n", str);

	memset(str, 0, BUFFER_SIZE);
	sprintf(str, "<0x%x>", RAMDISK_BASE + b_hdr->ramdisk_size);
	set_fdt_val("/chosen", "linux,initrd-end", str);
	printf("initrd-end: %s\n", str);

	noff = fdt_path_offset(fdt_dtb, "/reserved-memory/cp_rmem");
	if (noff >= 0) {
		np = fdt_getprop(fdt_dtb, noff, "reg", &len);
		if (len >= 0) {
			void *part = part_get_ab("modem");
			u32 addr_s;
			u64 addr_r;

			memset(str, 0, BUFFER_SIZE);
			memcpy(str, np, len);

			/* load modem header */
			addr_s = be32_to_cpu(*(((const u32 *)str) + 1));
			addr_r = (u64)addr_s;
			part_read_partial(part, (void *)addr_r, 0, 8 * 1024);
		}
	}

	if (b_hdr->cmdline[0] && (!b_hdr->cmdline[BOOT_ARGS_SIZE - 1])) {
		noff = fdt_path_offset(fdt_dtb, "/chosen");
		np = fdt_getprop(fdt_dtb, noff, "bootargs", &len);
		snprintf(str, BUFFER_SIZE, "%s %s", np, b_hdr->cmdline);
		fdt_setprop(fdt_dtb, noff, "bootargs", str, strlen(str) + 1);
	}

	noff = fdt_path_offset (fdt_dtb, "/chosen");
	np = fdt_getprop(fdt_dtb, noff, "bootargs", &len);
	printf("\nbootargs: %s\n", np);

	set_bootargs();

#if defined(CONFIG_USE_AVB20)
	/* set AVB args */
	noff = fdt_path_offset (fdt_dtb, "/chosen");
	np = fdt_getprop(fdt_dtb, noff, "bootargs", &len);
	snprintf(str, BUFFER_SIZE, "%s %s %s", np, cmdline, verifiedbootstate);
	fdt_setprop(fdt_dtb, noff, "bootargs", str, strlen(str) + 1);
	printf("\nupdated avb bootargs: %s\n", np);
#endif
	resize_dt(0);
}

int cmd_scatter_load_boot(int argc, const cmd_args *argv);

/*
 * load images from boot.img / recovery.img / dtbo.img partition
 *   kernel & dtb load from boot or recovery,
 *   depending on normal or Recovery booting.
 *   Below 2 case can be different by AB or Non AB Partition support.
 *
 * Non AB Case
 *   Normal booting : load boot from boot.img & dtbo from dtbo.img
 *   Recovery booting : load boot from recovery.img & dtbo from recovery.img
 *
 * AB Case
 *   Normal booting : load boot from boot_x.img & dtbo from dtbo_x.img
 *   Recovery booting : load boot from boot_x.img & dtbo from boot_x.img
 */
int load_boot_images(void)
{
#if defined(CONFIG_BOOT_IMAGE_SUPPORT)
	cmd_args argv[6];
	void *part;
	char boot_part_name[16] = "";
	unsigned int ab_support = 0;
	unsigned int boot_val = 0;

	ab_support = ab_update_support();
	boot_val = readl(EXYNOS_POWER_SYSIP_DAT0);
	printf("%s: AB[%d], boot_val[0x%02X]\n", __func__, ab_support, boot_val);
	if (ab_support)
		print_lcd_update(FONT_WHITE, FONT_BLACK, "AB Update support");

	argv[1].u = BOOT_BASE;
	argv[2].u = KERNEL_BASE;
	argv[3].u = RAMDISK_BASE;
	argv[4].u = DT_BASE;

	if (boot_val == REBOOT_MODE_RECOVERY) {
		argv[5].u = DTBO_BASE;
		if (!ab_support)
			sprintf(boot_part_name, "recovery");
		else
			sprintf(boot_part_name, "boot");
	} else {
		argv[5].u = 0x0;
		part = part_get_ab("dtbo");
		if (part == 0) {
			printf("Partition 'dtbo' does not exist\n");
			return -1;
		}
		part_read(part, (void *)DTBO_BASE);
		printf("DTBO loaded from dtbo partition\n");
		sprintf(boot_part_name, "boot");
	}

	printf("%s: loading from '%s' partition\n", __func__, boot_part_name);
	part = part_get_ab(boot_part_name);
	if (part == 0) {
		printf("Partition '%s' does not exist\n", boot_part_name);
		return -1;
	}

	/* ensure ramdisk image loaded in 0 initialized area */
	memset((void *)RAMDISK_BASE, 0, 0x200000);

	part_read(part, (void *)BOOT_BASE);

	cmd_scatter_load_boot(5, argv);
#else
	void *part;

	part = part_get("kernel");
	part_read(part, (void *)KERNEL_BASE);

	part = part_get("dtb");
	part_read(part, (void *)DT_BASE);

	part = part_get("dtbo");
	part_read(part, (void *)DTBO_BASE);

	part = part_get("ramdisk");
	part_read(part, (void *)RAMDISK_BASE);
#endif
	return 0;
}

int cmd_boot(int argc, const cmd_args *argv)
{
#if defined(CONFIG_FACTORY_MODE)
	struct exynos_gpio_bank *bank = (struct exynos_gpio_bank *)EXYNOS9630_GPA1CON;

	int gpio = 5;	/* Volume Up */
#endif
	unsigned int val;

	fdt_dtb = (struct fdt_header *)DT_BASE;
	dtbo_table = (struct dt_table_header *)DTBO_BASE;
	uint32_t recovery_mode = 0;
#if defined(CONFIG_USE_AVB20)
	int avb_ret = 0;
	uint32_t lock_state;
	char ab_suffix[8] = {'\0'};
#endif

#if defined(CONFIG_FACTORY_MODE)
	val = exynos_gpio_get_value(bank, gpio);
	if (!val) {
		writel(REBOOT_MODE_FACTORY, EXYNOS9630_POWER_SYSIP_DAT0);
		printf("Pressed key combination to enter samsung factory mode!\n");
		print_lcd_update(FONT_GREEN, FONT_BLACK,
			"Pressed key combination to enter samsung factory mode!");
	}
#endif

#if defined(CONFIG_AB_UPDATE) || defined(CONFIG_USE_AVB20)
	int ab_ret = 0;
#endif
#if defined(CONFIG_AB_UPDATE)
	ab_ret = ab_update_slot_info();
	if ((ab_ret < 0) && (ab_ret != AB_ERROR_NOT_SUPPORT)) {
		printf("AB update error! Error code: %d\n", ab_ret);
		print_lcd_update(FONT_RED, FONT_WHITE,
			"AB Update fail(%d), Entering fastboot...", ab_ret);
		start_usb_gadget();
		do {
			asm volatile("wfi");
		} while(1);
	}
#endif

	load_boot_images();

	val = readl(EXYNOS_POWER_SYSIP_DAT0);
	if (val == REBOOT_MODE_RECOVERY)
		recovery_mode = 1;

#if defined(CONFIG_USE_AVB20)
	ab_ret = ab_current_slot();
	if (ab_ret == AB_ERROR_NOT_SUPPORT)
		avb_ret = avb_main("", cmdline, verifiedbootstate, recovery_mode);
	else if (ab_ret == AB_SLOT_B)
		avb_ret = avb_main("_b", cmdline, verifiedbootstate, recovery_mode);
	else
		avb_ret = avb_main("_a", cmdline, verifiedbootstate, recovery_mode);

	printf("AVB: suffix[%s], boot/dtbo image verification result: 0x%X\n", ab_suffix, avb_ret);

	rpmb_get_lock_state(&lock_state);
	printf("lock state: %d\n", lock_state);
	if(lock_state) {
		if (avb_ret == AVB_SLOT_VERIFY_RESULT_ERROR_PUBLIC_KEY_REJECTED) {
			printf("AVB key invalid!\n");
		} else if (avb_ret) {
			printf("AVB failed! Resetting!\n");
			/* Delay for data write HW operation of ab_update_slot_info()
				on AB_SLOTINFO_PART partition. */
			mdelay(500);
			writel(readl(EXYNOS_POWER_SYSTEM_CONFIGURATION) | 0x2, EXYNOS_POWER_SYSTEM_CONFIGURATION);
			do {
				asm volatile("wfi");
			} while(1);
		}
	}
#endif

	configure_dtb();
	configure_ddi_id();

#if 0
	val = readl(EXYNOS9630_POWER_SYSIP_DAT0);
	if (val == REBOOT_MODE_RECOVERY || val == REBOOT_MODE_FACTORY) {
		writel(0, EXYNOS9630_POWER_SYSIP_DAT0);
	}
#endif

	/* notify EL3 Monitor end of bootloader */
	exynos_smc(SMC_CMD_END_OF_BOOTLOADER, 0, 0, 0);

	/* before jumping to kernel. disble arch_timer */
	arm_generic_timer_disable();

#if defined(CONFIG_MMU_ENABLE)
	clean_invalidate_dcache_all();
	disable_mmu_dcache();
#endif

	void (*kernel_entry)(int r0, int r1, int r2, int r3);

	kernel_entry = (void (*)(int, int, int, int))KERNEL_BASE;
	kernel_entry(DT_BASE, 0, 0, 0);

	return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("boot", "start kernel booting", &cmd_boot)
STATIC_COMMAND_END(boot);
