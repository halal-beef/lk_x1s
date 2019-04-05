/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <sys/types.h>
#include <platform/smc.h>
#include <platform/sfr.h>
#include <dev/boot.h>

u64 exynos_smc(u64 cmd, u64 arg1, u64 arg2, u64 arg3)
{
	register u64 reg0 __asm__ ("x0") = cmd;
	register u64 reg1 __asm__ ("x1") = arg1;
	register u64 reg2 __asm__ ("x2") = arg2;
	register u64 reg3 __asm__ ("x3") = arg3;

	__asm__ volatile (
                "smc\t0\n"
                : "+r"(reg0), "+r"(reg1), "+r"(reg2), "+r"(reg3)

        );

	return reg0;
}

inline u64 exynos_smc_read(u64 cmd, u64 arg1)
{
	register u64 reg0 __asm__ ("x0") = cmd;
	register u64 reg1 __asm__ ("x1") = arg1;
	register u64 reg2 __asm__ ("x2") = 0;

	__asm__ volatile (
                "dsb\tsy\n"
                "smc\t0\n"
                : "+r"(reg0), "+r"(reg1), "+r"(reg2)

        );

	return reg2;
}

inline u64 exynos_smc_read_otp(u64 *cmd, u64 *arg1, u64 *arg2, u64 *arg3)
{
	register u64 reg0 __asm__ ("x0") = *cmd;
	register u64 reg1 __asm__ ("x1") = *arg1;
	register u64 reg2 __asm__ ("x2") = *arg2;
	register u64 reg3 __asm__ ("x3") = *arg3;

	__asm__ volatile (
                "dsb\tsy\n"
                "smc\t0\n"
                : "+r"(reg0), "+r"(reg1), "+r"(reg2), "+r"(reg3)

        );

	*cmd = (u64)reg0;
	*arg1 = (u64)reg1;
	*arg2 = (u64)reg2;
	*arg3 = (u64)reg3;

	return reg0;
}

int load_keystorage(u64 addr, u64 size)
{
	return exynos_smc(SMC_CMD_KEYSTORAGE,
	                  addr, size, 0);
}

int load_ssp(u64 addr, u64 size)
{
	return exynos_smc(SMC_CMD_LOAD_SSP,
	                  addr, size, 0);
}

int init_ldfw(u64 addr, u64 size)
{
	return exynos_smc(SMC_CMD_LOAD_LDFW,
	                  addr, size, 0);
}

unsigned long load_sp_image(u32 boot_device)
{
	if (boot_device == BOOT_MMCSD)
		return 0; /*exynos_smc(SMC_CMD_LOAD_SECURE_PAYLOAD,
	                   * (u64)boot_device, MOVI_TZSW_POS, 0);
	                   */
	else if (boot_device == BOOT_UFS)
		return exynos_smc(SMC_CMD_LOAD_SECURE_PAYLOAD,
		                  (u64)boot_device, UFS_TZSW_POS, 0);
	else
		return exynos_smc(SMC_CMD_LOAD_SECURE_PAYLOAD,
		                  (u64)boot_device, 0, 0);
}

u64 load_sp(u64 addr, u64 size)
{
	return exynos_smc(SMC_CMD_LOAD_SECURE_PAYLOAD2,
	                  addr, size, 0);
}

u64 load_image_by_usb(u64 bin_list, u64 addr, u64 size)
{
	return exynos_smc(SMC_CMD_LOAD_IMAGE_BY_USB,
				bin_list, addr, size);
}

unsigned int find_second_boot(void)
{
	return exynos_smc_read(SMC_CMD_CHECK_SECOND_BOOT, 0);
}

void emmc_endbootop(void)
{
	exynos_smc(SMC_CMD_EMMC_ENDBOOTOP, 0, 0, 0);
}

void sdmmc_enumerate(void)
{
	exynos_smc(SMC_CMD_SDMMC_ENUMERATE, 0, 0, 0);
}

void set_secure_reg(u64 reg_val, u32 num)
{
	exynos_smc(SMC_CMD_SET_SECURE_REG, reg_val, (u64)num, reg_val);
}

#ifdef CONFIG_RAMDUMP_GPR
u64 cpu_boot(u64 id, u64 cpu, u64 fn)
{
	return exynos_smc(id, cpu, fn, 0);
}
#endif

void get_el3_mon_version(char *ptr, u32 string_size)
{
	exynos_smc(SMC_CMD_GET_EL3_MON_VERSION,
	           (u64)ptr, (u64)string_size, 0);
}

u64 load_ect_head(void)
{
	return exynos_smc(SMC_CMD_LOAD_ECT_HEAD, 0, 0, 0);
}

u64 load_ect_all(void)
{
	return exynos_smc(SMC_CMD_LOAD_ECT_ALL, 0, 0, 0);
}

u64 skip_ect(void)
{
	return exynos_smc(SMC_CMD_SKIP_ECT, 0, 0, 0);
}

u64 set_tzasc_action(u32 val)
{
	return exynos_smc(SMC_CMD_SET_TZASC_ACTION, (u64)val, 0, 0);
}

u64 sdm_encrypt_secdram(void)
{
	return exynos_smc(SMC_CMD_ENCRYPT_SECDRAM, 0, 0, 0);
}

u64 dumpgpr_flush_secdram(u32 cache_level, u32 core)
{
	return exynos_smc(SMC_CMD_DUMPGPR_FLUSH_SECDRAM, (u64)cache_level,
	                  (u64)core, 0);
}
