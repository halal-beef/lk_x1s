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
#include <platform/hvc.h>
#include <platform/sfr.h>
#include <platform/ldfw.h>
#include <platform/h-arx.h>
#include <platform/charger.h>
#include <lib/ab_update.h>
#include <platform/secure_boot.h>
#include <platform/sizes.h>
#include <platform/bootimg.h>
#include <lib/fdtapi.h>
#include <platform/chip_id.h>
#include <platform/mmu/mmu_func.h>
#include <pit.h>
#include <dev/scsi.h>
#include <dev/mmc.h>
#include <arch/arch_ops.h>

/* Memory node */
#define SIZE_4GB		(0x100000000)
#define SIZE_2GB		(0x80000000)
#define SIZE_1GB		(0x40000000)
#define SIZE_500MB		(0x20000000)
#define MASK_1MB		(0x100000 - 1)

#define BUFFER_SIZE		4096

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

static void set_bootargs(void)
{
	bootargs_init();

	/* update_val("console", "ttySAC0,115200"); */

	bootargs_update();
}
void start_usb_gadget(void);
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

	unsigned long harx_info_ver = 0;
	unsigned long harx_base = 0;
	unsigned long harx_size = 0;

	unsigned int soc_rev = (readl(EXYNOS9830_PRO_ID + CHIPID_REV_OFFSET) >>
				MAIN_REVISION_SHIFT) & REVISION_MASK;

	int ret;
	char path[BUFFER_SIZE];
	char reg_value[BUFFER_SIZE];

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

	/* Get H-Arx base address and size to carve-out */
	if (is_harx_initialized == false)
		goto skip_carve_out_harx;

	if (soc_rev < SOC_REVISION_EVT1)
		goto skip_carve_out_harx;

	harx_info_ver = exynos_hvc(HVC_CMD_GET_HARX_INFO,
				   HARX_INFO_TYPE_VERSION,
				   0, 0, 0);
	if (harx_info_ver == HARX_INFO_VERSION) {
		harx_base = exynos_hvc(HVC_CMD_GET_HARX_INFO,
					HARX_INFO_TYPE_HARX_BASE,
					0, 0, 0);
		if (harx_base == (unsigned long)ERROR_INVALID_TYPE) {
			printf("[ERROR] Invalid H-Arx base address\n");
			harx_base = 0;
		}

		printf("HARX_BASE[%#lx]\n", harx_base);

		harx_size = exynos_hvc(HVC_CMD_GET_HARX_INFO,
					HARX_INFO_TYPE_HARX_SIZE,
					0, 0, 0);
		if (harx_size == (unsigned long)ERROR_INVALID_TYPE) {
			printf("[ERROR] Invalid H-Arx size\n");
			harx_size = 0;
		}

		printf("HARX_SIZE[%#lx]\n", harx_size);
	} else {
		printf("[ERROR] H-Arx info version is old version [%lx]\n",
			harx_info_ver);

		harx_base = 0;
		harx_size = 0;
	}

skip_carve_out_harx:

	/* DT control code must write after this function call. */
	merge_dto_to_main_dtb(board_id, board_rev);
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

	snprintf(path, sizeof(path), "/reserved-memory/kaslr");
	ret = make_fdt_node("/reserved-memory", "kaslr");
	if (ret) {
		printf("Failed to create /reserved-memory/kaslr node\n");
		print_lcd_update(FONT_RED, FONT_BLACK, "Failed to create kaslr");
    	}
	set_fdt_val(path, "compatible", "kernel-kaslr");
	snprintf(reg_value, sizeof(reg_value), "<0x00 0x80001000 0x1000>");
	set_fdt_val(path, "reg", reg_value);

	snprintf(path, sizeof(path), "/reserved-memory/el2_earlymem");
	ret = make_fdt_node("/reserved-memory", "el2_earlymem");
	if (ret) {
		printf("Failed to create /reserved-memory/el2_earlymem node\n");
                print_lcd_update(FONT_RED, FONT_BLACK, "Failed to create el2_earlymem");
	}
	set_fdt_val(path, "compatible", "el2,uh");
	snprintf(reg_value, sizeof(reg_value), "<0x0a 0xfe800000 0x1800000>");
	set_fdt_val(path, "reg", reg_value);

	snprintf(path, sizeof(path), "/reserved-memory/el2_code");
	ret = make_fdt_node("/reserved-memory", "el2_code");
	if (ret) {
		printf("Failed to create /reserved-memory/el2_code node\n");
                print_lcd_update(FONT_RED, FONT_BLACK, "Failed to create el2_code");
	}
    	set_fdt_val(path, "compatible", "el2,uh");
    	snprintf(reg_value, sizeof(reg_value), "<0x00 0xc1400000 0x200000>");
    	set_fdt_val(path, "reg", reg_value);

	print_lcd_update(FONT_GREEN, FONT_BLACK, "EL2 and KASLR nodes created!");

	/* Secure memories are carved-out in case of EVT1 */
	/*
	 * 1st DRAM node
	 */
	add_dt_memory_node(DRAM_BASE,
	                   sec_dram_base - DRAM_BASE);
	print_lcd_update(FONT_GREEN, FONT_BLACK, "First node");
	/*
	 * 2nd DRAM node
	 */
	if ((harx_base != 0) && (harx_size != 0)) {
		add_dt_memory_node(sec_dram_end,
				   harx_base - sec_dram_end);

		sec_dram_end = harx_base + harx_size;
	}
        print_lcd_update(FONT_GREEN, FONT_BLACK, "second node");

	if (sec_pt_base && sec_pt_size) {
		/*
		 * HACK: On retail Exynos9830 (atleast) 0xC0000000 is an empty memory region,
		 * the current behaviour of this bootloader makes it map it as a non-empty
		 * memory and also skips out a memory region in the process, making the
		 * kernel crash.
		 */
		add_dt_memory_node(sec_dram_end,
		                   0x0);

                add_dt_memory_node(0xc1200000,
                                   0x1ee00000);
        print_lcd_update(FONT_GREEN, FONT_BLACK, "0th if");
		if (dram_size >= SIZE_2GB) {
			add_dt_memory_node(sec_pt_end,
			                   (DRAM_BASE + SIZE_2GB)
			                   - sec_pt_end);
        print_lcd_update(FONT_GREEN, FONT_BLACK, "1st if");
		} else {
			add_dt_memory_node(sec_pt_end,
			                   (DRAM_BASE + dram_size)
			                   - sec_pt_end);
        print_lcd_update(FONT_GREEN, FONT_BLACK, "2nd if");
		}
	} else {
		if (dram_size >= SIZE_2GB) {
			add_dt_memory_node(sec_dram_end,
			                   (DRAM_BASE + SIZE_2GB)
			                   - sec_dram_end);
        print_lcd_update(FONT_GREEN, FONT_BLACK, "3rd if");
		} else {
			add_dt_memory_node(sec_dram_end,
			                   (DRAM_BASE + dram_size)
			                   - sec_dram_end);
        print_lcd_update(FONT_GREEN, FONT_BLACK, "4th if");
		}
	}

	/*
	 * 3rd DRAM node
	 */
	if (dram_size <= SIZE_2GB)
		goto mem_node_out;

	for (u64 i = 0; i < dram_size - SIZE_2GB; i += SIZE_500MB) {
		/* HACK: unaccessible range(B_0000_0000 ~ B_FFFF_FFFF)*/
		u64 ua_size;
		if (((DRAM_BASE2 + i) >> 32) >= 0xb)
			ua_size = SIZE_4GB;
		else
			ua_size = 0;

		/* add 500MB mem node */
		add_dt_memory_node(DRAM_BASE2 + i + ua_size, SIZE_500MB);
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
	/* set AVB args */
	get_ops_addr(&ops);
	ops->read_is_device_unlocked(ops, &unlock);
	noff = fdt_path_offset (fdt_dtb, "/chosen");
	np = fdt_getprop(fdt_dtb, noff, "bootargs", &len);
	snprintf(str, BUFFER_SIZE, "%s %s %s", np, cmdline, verifiedbootstate);
	fdt_setprop(fdt_dtb, noff, "bootargs", str, strlen(str) + 1);
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

	/*
	 * HACK: FORCE BOOTARGS
	 */
	snprintf(str, BUFFER_SIZE, "%s", "cgroup_disable=pressure androidboot.boot_devices=13100000.ufs root=/dev/ram0 androidboot.boot_devices=13100000.ufs androidboot.hardware=exynos990 bcm_setup=0xffffff80f8e00000 firmware_class.path=/vendor/firmware reserve-fimc=0xffffff90f9fe0000 nohugeiomap epx_activate=true fpsimd_check_context=y rcupdate.rcu_expedited=1 corememsize=8G cgroup.memory=nokmem loop.max_part=7 androidboot.bore_cnt=634 sec_debug.pcb_offset=7346944 sec_debug.smd_offset=7348992 sec_debug.lpddr4_size=12.0 sec_debug.dram_info=01,07,00,12G sec_debug.pwrsrc_rs=0x0000000820000000 sec_debug.reset_reason=7 sec_reset.reset_reason=7 sec_debug.reset_rwc=0 console=ram loglevel=4 sec_debug.level=0 sec_watchdog.sec_pet=5 androidboot.debug_level=0x4f4c androidboot.force_upload=0x0 sec_audio_debug.debug_level=0x4f4c sec_debug.dump_sink=0x0 sec_debug.upload_count=0 androidboot.dram_info=01,07,00,12G androidboot.ddr_size=12 androidboot.ap_serial=0x0A8D0EB243C0 sec_debug.charging_offset=7340592 sec_debug.wireless_offset=7340632 sec_debug.pd_hv_offset=7340644 androidboot.fmm_lock=0 sec_debug.fmm_lock_offset=7340628 softdog.soft_margin=100 softdog.soft_panic=1 androidboot.sn.param.offset=7343024 androidboot.im.param.offset=7342864 androidboot.me.param.offset=7342944 androidboot.pr.param.offset=7343104 androidboot.sku.param.offset=7343184 androidboot.prototype.param.offset=7351040 androidboot.recovery_offset=7355136 ess_setup=0xfd900000 sec_debug_next=0x1000000@0x91200000 charging_mode=0x3130 wireless_ic=0x4102010 pd_disable=0x30 s3cfb.bootloaderfb=0xf1000000 lcdtype=8462596 mcd-panel.boot_panel_id=8462596 androidboot.carrierid.param.offset=7340596 androidboot.carrierid=EUX androidboot.prodcode.param.offset=7363328 consoleblank=0 ehci_hcd.park=3 oops=panic pmic_info=43 ccic_info=1 fg_reset=0 androidboot.emmc_checksum=3 androidboot.sales.param.offset=7340568 sales_code=OPV androidboot.bootloader=G988BXXSNHXL1 androidboot.selinux=enforcing androidboot.ucs_mode=0 androidboot.revision=23 androidboot.wb.hs=0e03 androidboot.rp=23 androidboot.wb.snapQB=83553235R sec_debug.bin=C androidboot.hmac_mismatch=0 androidboot.sec_atd.tty=/dev/ttySAC0 androidboot.serialno=R5CN406MY2B snd_soc_core.pmdown_time=1000 androidboot.cp_reserved_mem=off androidboot.dtbo_idx=8 androidboot.em.did=200a8d0eb243c011 androidboot.em.model=SM-G988B androidboot.em.status=0x0 androidboot.em.rdx_dump=false androidboot.sb.debug0=0x0 androidboot.svb.ver=SVB1.0 androidboot.ulcnt=1 androidboot.subpcb=0 androidboot.slavepcb=1 androidboot.sysup.edtbo=0@7381760 androidboot.sysup.param=7361280 androidboot.hdm_status=NONE androidboot.vup=0 androidboot.asb=0 sec_bootstat.boot_time_bl1=100 sec_bootstat.boot_time_bl2=230 sec_bootstat.boot_time_bl3=1777    androidboot.verifiedbootstate=green androidboot.warranty_bit=0 androidboot.fmp_config=1");
	fdt_setprop(fdt_dtb, noff, "bootargs", str, strlen(str) + 1);

	printf("\nbootargs\n");
	noff = fdt_path_offset(fdt_dtb, "/chosen");
	printf("\fdt_getprop\n");
	np = fdt_getprop(fdt_dtb, noff, "bootargs", &len);
	printf("\nbootargs: %s\n", np);

	if (change_dt_psci_method(fdt_dtb))
		printf("Do not change PSCI method\n");
	else
		harx_print_with_lcd("Change PSCI method to HVC - this will be a succesful boot, LMAO\n");

	resize_dt(0);
}

int cmd_scatter_load_boot(int argc, const cmd_args *argv);

u32 kaslr_mem_offset;

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

	/* TODO: Move to DTB - and also maybe sanity check? */
	u32 kaslr_mem_magic = readl(0x80001000);
	u32 kaslr_mem_offset = readl(0x80001000 + sizeof(u32));

	argv[1].u = BOOT_BASE;
	argv[2].u = KERNEL_BASE + kaslr_mem_offset;
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

	print_lcd_update(FONT_GREEN, FONT_BLACK, "About to jump to kernel! Good night!\n");

	printf("DECON0: HW_SW_TRIG Restore\n");
	*(int *)(0x19050000 + 0x70) = 0x3070;
	//writel(0x3070, 0x19050070);

	if (readl(EXYNOS9830_POWER_SYSIP_DAT0) == REBOOT_MODE_RECOVERY ||
	    readl(EXYNOS9830_POWER_SYSIP_DAT0) == REBOOT_MODE_FACTORY)
		writel(0, EXYNOS9830_POWER_SYSIP_DAT0);
	/* notify EL3 Monitor end of bootloader */
	exynos_smc(SMC_CMD_END_OF_BOOTLOADER, 0, 0, 0);


	/* before jumping to kernel. disble arch_timer */
	arm_generic_timer_disable();
	/* before jumping to kernel. disable interrupt */
	arch_disable_ints();

	clean_invalidate_dcache_all();
	disable_mmu_dcache();

	printf("Starting kernel...\n");
	void (*kernel_entry)(int r0, int r1, int r2, int r3);

	kernel_entry = (void (*)(int, int, int, int))KERNEL_BASE + kaslr_mem_offset;
	kernel_entry(DT_BASE, 0, 0, 0);

	return 0;
}

STATIC_COMMAND_START
	STATIC_COMMAND("boot", "start kernel booting", &cmd_boot)
STATIC_COMMAND_END(boot);
