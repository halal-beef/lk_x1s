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
#include <libfdt_env.h>
#include <fdt.h>
#include <libfdt.h>
#include <lib/bio.h>
#include <lib/console.h>
#include <part_gpt.h>
#include <dev/boot.h>
#include <dev/rpmb.h>
#include <platform/smc.h>
#include <platform/sfr.h>
#include <platform/ldfw.h>
#include <platform/charger.h>
#include <platform/ab_update.h>
#include <platform/secure_boot.h>
#include <pit.h>

/* Memory node */
#define SIZE_2GB (0x80000000)
#define MASK_1MB (0x100000 - 1)
#define MASK_4GB (0xFFFFFFFF)

#define BOOT_BASE 0x94000000
#define KERNEL_BASE 0x80080000
#define RAMDISK_BASE 0x84000000
#define DT_BASE 0x8A000000
#define DT_RESERVE_MEM 0x23000
#define ECT_BASE 0x90000000
#define ECT_SIZE 0x32000
#define BUFFER_SIZE 1024

unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base);

void resize_dt(void *base)
{
	fdt_set_totalsize(base, DT_RESERVE_MEM);

	fdt_add_mem_rsv(base, (uint64_t)base, DT_RESERVE_MEM);
}

int make_fdt_node(char *path, char *node)
{
	int offset;
	int ret;

	offset = fdt_path_offset (DT_BASE, path);
	if (offset < 0) {
		printf ("libfdt fdt_path_offset() returned %s\n",
			fdt_strerror(offset));
		return 1;
	}

	ret = fdt_add_subnode(DT_BASE, offset, node);
	if (ret < 0) {
		printf ("libfdt fdt_add_subnode(): %s\n",
			fdt_strerror(ret));
		return 1;
	}

	return 0;
}

int set_fdt_val(char *path, char *property, char *value)
{
	char parsed_value[BUFFER_SIZE];
	int offset;
	int len = 0;
	int ret;
	char *tp;
	char *np;
	unsigned long tmp;
	char *dp = parsed_value;

	np = value;

	if (*np == '<') {
		np++;
		while (*np != '>') {
			tp = np;
			tmp = simple_strtoul(tp, &np, 0);
			*(uint32_t *)dp = cpu_to_be32(tmp);
			dp  += 4;
			len += 4;

			while (*np == ' ')
				np++;
		}
	} else {
		strcpy(parsed_value, value);
		len += strlen(value) + 1;
	}

	offset = fdt_path_offset (DT_BASE, path);
	if (offset < 0) {
		printf ("libfdt fdt_path_offset() returned %s\n",
		fdt_strerror(offset));
		return 1;
	}

	ret = fdt_setprop(DT_BASE, offset, property, parsed_value, len);
	if (ret < 0) {
		printf ("libfdt fdt_setprop(): %s\n", fdt_strerror(ret));
		return 1;
	}

	return 0;
}

struct bootargs_prop {
	char prop[64];
	char val[64];
};
static struct bootargs_prop prop[32] = { 0, };
static int prop_cnt = 0;

static void bootargs_init(void)
{
	u32 i = 0;
	u32 len = 0;
	u32 cur = 0;
	u32 is_val = 0;
	char bootargs[BUFFER_SIZE];
	int len2;
	const char *np;
	int noff;

	noff = fdt_path_offset (DT_BASE, "/chosen");
	if (noff >= 0) {
		np = fdt_getprop(DT_BASE, noff, "bootargs", &len2);
		if (len2 >= 0) {
			memset(bootargs, 0, BUFFER_SIZE);
			memcpy(bootargs, np, len2);
		}
	}

	printf("\ndefault bootargs: %s\n", bootargs);

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
}
static char *get_bootargs_val(const char *name)
{
       u32 i = 0;

       for (i = 0; i <= prop_cnt; i++) {
               if (strncmp(prop[i].prop, name, strlen(name)) == 0)
                       return prop[i].val;
       }

       return NULL;
}

static void update_val(const char *name, const char *val)
{
	u32 i = 0;
	u32 cur = 0;

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
			sprintf(bootargs + cur, "%s",
				prop[i].prop);
			cur += strlen(prop[i].prop);
			snprintf(bootargs + cur, 2, " ");
			cur += 1;
		} else {
			sprintf(bootargs + cur, "%s=%s",
				prop[i].prop, prop[i].val);
			cur += strlen(prop[i].prop) + strlen(prop[i].val) + 1;
			snprintf(bootargs + cur, 2, " ");
			cur += 1;
		}
	}

	bootargs[cur] = '\0';

	printf("\nupdated bootargs: %s\n", bootargs);

	set_fdt_val("/chosen", "bootargs", bootargs);
}

static void set_bootargs(void)
{
	bootargs_init();

	/* update_val("console", "ttySAC0,115200"); */

	bootargs_update();
}

static void add_dt_memory_node(unsigned long base, unsigned int size)
{
	char str[BUFFER_SIZE];
	char str2[BUFFER_SIZE];
	unsigned int ubase = 0;
	unsigned int lbase = 0;

	ubase = base >> 32;
	lbase = base & MASK_4GB;

	sprintf(str, "memory@%lx", base);
	make_fdt_node("/", str);

	sprintf(str, "/memory@%lx", base);
	sprintf(str2, "<0x%x 0x%x 0x%x>", ubase, lbase, size);
	set_fdt_val(str, "reg", str2);

	sprintf(str, "/memory@%lx", base);
	set_fdt_val(str, "device_type", "memory");
}

static void configure_dtb(void)
{
	char str[BUFFER_SIZE];
	u32 soc_ver = 0;
	unsigned long sec_dram_base = 0;
	unsigned int sec_dram_size = 0;
	unsigned long sec_dram_end = 0;
	unsigned long sec_pt_base = 0;
	unsigned int sec_pt_size = 0;
	unsigned long sec_pt_end = 0;
	u64 dram_size = *(u64 *)BL_SYS_INFO_DRAM_SIZE;
	unsigned long long start, count;
	unsigned char pid;
	struct pit_entry *ptn;
	bdev_t *dev;
	char *name;
	unsigned int boot_dev;
	int len;
	const char *np;
	int noff;

	/* Get Secure DRAM information */
	soc_ver = exynos_smc(SMC_CMD_GET_SOC_INFO, SOC_INFO_TYPE_VERSION, 0, 0);
        if (soc_ver == SOC_INFO_VERSION(SOC_INFO_MAJOR_VERSION, SOC_INFO_MINOR_VERSION)) {
		sec_dram_base = exynos_smc(SMC_CMD_GET_SOC_INFO,
						SOC_INFO_TYPE_SEC_DRAM_BASE,
						0,
						0);
		if (sec_dram_base == ERROR_INVALID_TYPE) {
			printf("get secure memory base addr error!!\n");
			while (1);
		}

		sec_dram_size = (unsigned int)exynos_smc(SMC_CMD_GET_SOC_INFO,
							SOC_INFO_TYPE_SEC_DRAM_SIZE,
							0,
							0);
		if (sec_dram_size == ERROR_INVALID_TYPE) {
			printf("get secure memory size error!!\n");
			while (1);
		}
	} else {
		printf("[ERROR] el3_mon is old version. (0x%lx)\n", soc_ver);
		while (1);
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
	} else if (sec_pt_base == ERROR_NO_DRM_FW_INITIALIZED) {
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
	} else if (sec_pt_size == ERROR_NO_DRM_FW_INITIALIZED) {
		printf("[SEC_PGTBL_SIZE] DRM LDFW is not initialized\n");
		sec_pt_size = 0;
	} else if (sec_pt_base & MASK_1MB) {
		printf("[SEC_PGTBL_SIZE] Not aligned with 1MB\n");
		sec_pt_size = 0;
	}

	sec_pt_end = sec_pt_base + sec_pt_size;

	printf("SEC_PGTBL_BASE[%#lx]\n", sec_pt_base);
	printf("SEC_PGTBL_SIZE[%#x]\n", sec_pt_size);

	/* merge_dto_to_main_dtb(); */

	resize_dt(DT_BASE);

	sprintf(str, "<0x%x>", ECT_BASE);
	set_fdt_val("/ect", "parameter_address", str);

	sprintf(str, "<0x%x>", ECT_SIZE);
	set_fdt_val("/ect", "parameter_size", str);

	if (get_charger_mode()) {
		noff = fdt_path_offset (DT_BASE, "/chosen");
		np = fdt_getprop(DT_BASE, noff, "bootargs", &len);
		snprintf(str, BUFFER_SIZE, "%s %s", np, "androidboot.mode=charger");
		fdt_setprop(DT_BASE, noff, "bootargs", str,
			strlen(str) + 1);

		printf("Enter charger mode...");
	}

	/* Add booting slot */
	noff = fdt_path_offset (DT_BASE, "/chosen");
	np = fdt_getprop(DT_BASE, noff, "bootargs", &len);
	if (ab_current_slot())
		snprintf(str, BUFFER_SIZE, "%s %s", np, "androidboot.slot_suffix=_b");
	else
		snprintf(str, BUFFER_SIZE, "%s %s", np, "androidboot.slot_suffix=_a");
	fdt_setprop(DT_BASE, noff, "bootargs", str, strlen(str) + 1);

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
		add_dt_memory_node(sec_pt_end,
					(DRAM_BASE + SIZE_2GB)
					- sec_pt_end);
	} else {
		add_dt_memory_node(sec_dram_end,
					(DRAM_BASE + SIZE_2GB)
					- sec_dram_end);
	}

	/*
	 * 3rd DRAM node
	 */
	add_dt_memory_node(DRAM_BASE2, SIZE_2GB);
	if (dram_size == 0x180000000) {
		make_fdt_node("/", "memory@900000000");
		set_fdt_val("/memory@900000000", "reg", "<0x9 0x0 0x80000000>");
		set_fdt_val("/memory@900000000", "device_type", "memory");
	}

	noff = fdt_path_offset (DT_BASE, "/reserved-memory/modem_if");
	if (noff >= 0) {
		np = fdt_getprop(DT_BASE, noff, "reg", &len);
		if (len >= 0) {
			memset(str, 0, BUFFER_SIZE);
			memcpy(str, np, len);

			boot_dev = get_boot_device();
			if (boot_dev == BOOT_UFS)
				name = "scsi0";
			else
				;

			/* get modem partition info */
			dev = bio_open(name);
			ptn = pit_get_part_info("modem");
			/* load modem header */
			dev->new_read(dev, be32_to_cpu(((const u32 *)str)[1]), ptn->blkstart, 16);
			bio_close(dev);
		}
	}

	set_bootargs();
}

int cmd_scatter_load_boot(int argc, const cmd_args *argv);

int load_boot_images(void)
{
	struct pit_entry *ptn;
	cmd_args argv[5];

	if (ab_current_slot())
		ptn = pit_get_part_info("boot_b");
	else
		ptn = pit_get_part_info("boot_a");

	if (ptn == 0) {
		printf("Partition 'kernel' does not exist\n");
		return -1;
	} else {
		pit_access(ptn, PIT_OP_LOAD, (u64)BOOT_BASE, 0);
	}

	if (ab_current_slot())
		ptn = pit_get_part_info("dtb_b");
	else
		ptn = pit_get_part_info("dtb_a");

	if (ptn == 0) {
		printf("Partition 'dtb' does not exist\n");
		return -1;
	} else {
		pit_access(ptn, PIT_OP_LOAD, (u64)DT_BASE, 0);
	}

	argv[1].u = BOOT_BASE;
	argv[2].u = KERNEL_BASE;
	argv[3].u = 0;
	argv[4].u = 0;
	cmd_scatter_load_boot(5, argv);

	return 0;
}

int cmd_boot(int argc, const cmd_args *argv)
{
	if (!init_keystorage())
		printf("keystorage: init done successfully.\n");
	else
		printf("keystorage: init failed.\n");

	if (!init_ldfws()) {
			printf("ldfw: init done successfully.\n");
	} else {
			printf("ldfw: init failed.\n");
	}

	rpmb_key_programming();
	rpmb_load_boot_table();

	load_boot_images();

	configure_dtb();

#if defined(CONFIG_USE_AVB20)
	if (ab_current_slot())
		avb_main("_b");
	else
		avb_main("_a");
#endif

	/* notify EL3 Monitor end of bootloader */
	exynos_smc(SMC_CMD_END_OF_BOOTLOADER, 0, 0, 0);

	void (*kernel_entry)(int r0, int r1, int r2, int r3);

	kernel_entry = (void (*)(int, int, int, int))KERNEL_BASE;
	kernel_entry(DT_BASE, 0, 0, 0);

	return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("boot", "start kernel booting", &cmd_boot)
STATIC_COMMAND_END(boot);
