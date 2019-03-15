/**
 * @file  otp_v20.c
 * @brief OTP API funcitons for Secure Boot, Secure JTAG, ETC
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <debug.h>
#include <sys/types.h>
#include <string.h>
#include <platform/otp_v20.h>

typedef struct otp_data {
	uint8_t key[BANK_SIZE_IN_BYTE]; /* 128B */
	uint32_t key_len;
	uint32_t checksum;
	uint64_t reserved1;
	uint64_t reserved2;
	uint64_t reserved3;
} OTP_DATA_T __attribute__((__aligned__(CACHE_WRITEBACK_GRANULE)));

static inline uint64_t exynos_cm_smc(uint64_t *cmd, uint64_t *arg1,
                                     uint64_t *arg2, uint64_t *arg3)
{
	register uint64_t reg0 __asm__ ("x0") = *cmd;
	register uint64_t reg1 __asm__ ("x1") = *arg1;
	register uint64_t reg2 __asm__ ("x2") = *arg2;
	register uint64_t reg3 __asm__ ("x3") = *arg3;

	__asm__ volatile (
                "smc\t0\n"
                : "+r"(reg0), "+r"(reg1), "+r"(reg2), "+r"(reg3)
        );

	*cmd = reg0;
	*arg1 = reg1;
	*arg2 = reg2;
	*arg3 = reg3;

	if (reg0 == RV_OTP_LDFW_NOT_LOADED)
		printf("[OTP] LDFW hasn't been loaded\n");

	return reg0;
}

uint64_t string_to_dec(char str[])
{
	uint64_t result = 0;
	int dec_trans = 1;
	int len, i;

	len = strlen(str);

	for (i = len - 1; i >= 0; i--) {
		if (str[i] >= '0' && str[i] <= '9')
			result += ((str[i] - 0x30) * dec_trans);
		else
			return -1;

		dec_trans *= 10;
	}

	return result;
}

static uint32_t cm_otp_get_checksum(uint8_t *otp_data, uint32_t otp_data_len)
{
	uint32_t i, j;
	uint32_t byte, crc, mask;

	i = 0;
	crc = 0xFFFFFFFF;

	while (otp_data_len--) {
		byte = *(otp_data + i); /* get next byte */
		crc = crc ^ byte;

		for (j = 0; j < 8; j++) { /*  do eight times */
			mask = -(crc & 1);
			crc = (crc >> 1) ^ (0xEDB88320 & mask);
		}
		i++;
	}

	return ~crc;
}

uint64_t cm_otp_smc_test(uint64_t test_num)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t i = 0;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_R_SMC_TEST;
	r2 = 0xAAAA0000;
	r3 = test_num;
	i = test_num;

	printf("[OTP] SMC Test- r0: %llX\n", r0);
	printf("[OTP] SMC Test- r1: %llX\n", r1);
	printf("[OTP] SMC Test- r2: %llX\n", r2);
	printf("[OTP] SMC Test- r3: %llX\n", r3);

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);

	printf("[OTP] SMC Test- r0: %llX\n", r0);
	printf("[OTP] SMC Test- r1: %llX\n", r1);
	printf("[OTP] SMC Test- r2: %llX\n", r2);
	printf("[OTP] SMC Test- r3: %llX\n", r3);

	if ((ret != RV_SUCCESS) ||
	    (r2 != 0xAAAAAAAA) ||
	    (i != (r3 - 0xFFFF0000)))
		printf("[OTP] SMC test was failed!!\n");
	else
		printf("[OTP] SMC test was passed~^^\n");

	return RV_SUCCESS;
}

uint64_t cm_otp_read_chip_id(uint64_t *chip_id)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_R_CHIP_ID;
	r2 = 0;
	r3 = 0;

	printf("[OTP] CHIP_ID read start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] CHIP_ID read failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[OTP] CHIP_ID read value: 0x%x%x\n",
	       (uint32_t)(r3 & 0x3FF), (uint32_t)r2);

	*chip_id = (((r3 & 0x3FF) << 32) | r2);

	return RV_SUCCESS;
}

uint64_t cm_otp_sw_block_enable(void)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_W_OTP_SW_BLOCK;
	r2 = 0;
	r3 = 0;

	printf("[OTP] OTP_SW_BLOCK enable start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] OTP_SW_BLOCK enable failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[OTP] OTP_SW_BLOCK enable success\n");

	return RV_SUCCESS;
}

/*
 ****************************************************************************
 * OTP functions for Secure Boot*/

/*****************************************************************************/
uint64_t cm_otp_write_rom_sec_boot_key(uint8_t *rom_sec_boot_key_ptr,
                                       uint32_t rom_sec_boot_key_len)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;
	OTP_DATA_T otp_data;

	if (rom_sec_boot_key_ptr == NULL) {
		printf("[OTP] invalid ROM_SECURE_BOOT_KEY addresss\n");
		return RV_OTP_PARAM_NULL_POINTER;
	}

	if (rom_sec_boot_key_len != OTP_ROM_SEC_BOOT_KEY_LEN) {
		printf("[OTP] invalid ROM_SECURE_BOOT_KEY length\n");
		return RV_OTP_PARAM_INVALID_KEY_LEN;
	}

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_W_ROM_SEC_BOOT_KEY;
	r2 = VIRT_TO_PHYS(&otp_data);
	r3 = 0;

	printf("[OTP] ROM_SECURE_BOOT_KEY program start\n");

	memset((uint8_t *)&otp_data, 0, sizeof(OTP_DATA_T));
	memcpy(otp_data.key, rom_sec_boot_key_ptr, rom_sec_boot_key_len);
	otp_data.key_len = rom_sec_boot_key_len;
	otp_data.checksum = cm_otp_get_checksum(rom_sec_boot_key_ptr,
	                                        rom_sec_boot_key_len);

	FLUSH_DCACHE_RANGE(&otp_data, sizeof(OTP_DATA_T));

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] ROM_SECURE_BOOT_KEY program fail: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[OTP] ROM_SECURE_BOOT_KEY program success\n");

	return ret;
}

uint64_t cm_otp_check_rom_sec_boot_key(uint8_t *rom_sec_boot_key_ptr,
                                       uint32_t rom_sec_boot_key_len)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;
	OTP_DATA_T otp_data;

	if (rom_sec_boot_key_ptr == NULL) {
		printf("[OTP] invalid ROM_SECURE_BOOT_KEY addresss\n");
		return RV_OTP_PARAM_NULL_POINTER;
	}

	if (rom_sec_boot_key_len != OTP_ROM_SEC_BOOT_KEY_LEN) {
		printf("[OTP] invalid ROM_SECURE_BOOT_KEY length\n");
		return RV_OTP_PARAM_INVALID_KEY_LEN;
	}

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_C_ROM_SEC_BOOT_KEY;
	r2 = VIRT_TO_PHYS(&otp_data);
	r3 = 0;

	printf("[OTP] ROM_SECURE_BOOT_KEY check start\n");

	memset((uint8_t *)&otp_data, 0, sizeof(OTP_DATA_T));
	memcpy(otp_data.key, rom_sec_boot_key_ptr, rom_sec_boot_key_len);
	otp_data.key_len = rom_sec_boot_key_len;
	otp_data.checksum = cm_otp_get_checksum(rom_sec_boot_key_ptr,
	                                        rom_sec_boot_key_len);

	FLUSH_DCACHE_RANGE(&otp_data, sizeof(OTP_DATA_T));

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret == RV_SUCCESS)
		printf("[OTP] ROM_SECURE_BOOT_KEY check: valid key\n");
	else if (ret == RV_OTP_CHECK_KEY_NOT_FUSED)
		printf("[OTP] ROM_SECURE_BOOT_KEY hasn't been programed yet\n");
	else if (ret == RV_OTP_CHECK_KEY_CMP_FAIL)
		printf("[OTP] ROM_SECURE_BOOT_KEY check: invalid key\n");
	else
		printf("[OTP] ROM_SECURE_BOOT_KEY check failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);

	return ret;
}

uint64_t cm_otp_write_use_rom_sec_boot_key(void)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_W_USE_ROM_SEC_BOOT_KEY;
	r2 = 0;
	r3 = 0;

	printf("[OTP] USE_ROM_SEC_BOOT_KEY program start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] USE_ROM_SEC_BOOT program failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[OTP] USE_ROM_SEC_BOOT_KEY program success\n");

	return RV_SUCCESS;
}

uint64_t cm_otp_read_use_rom_sec_boot_key(uint64_t *output)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_R_USE_ROM_SEC_BOOT_KEY;
	r2 = 0;
	r3 = 0;

	printf("[OTP] USE_ROM_SEC_BOOT_KEY read: start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] USE_ROM_SEC_BOOT_KEY read failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	*output = r2;

	printf("[OTP] USE_ROM_SEC_BOOT_KEY read value: 0x%llx\n", r2);

	return RV_SUCCESS;
}

uint64_t cm_otp_write_ban_rom_sec_boot_key(void)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_W_BAN_ROM_SEC_BOOT_KEY;
	r2 = 0;
	r3 = 0;

	printf("[OTP] BAN_ROM_SEC_BOOT_KEY program start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] BAN_ROM_SEC_BOOT_KEY program failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return r0;
	}

	printf("[OTP] BAN_ROM_SEC_BOOT_KEY program success\n");

	return RV_SUCCESS;
}

uint64_t cm_otp_read_ban_rom_sec_boot_key(uint64_t *output)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_R_BAN_ROM_SEC_BOOT_KEY;
	r2 = 0;
	r3 = 0;

	printf("[OTP] BAN_ROM_SEC_BOOT_KEY read start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] BAN_ROM_SEC_BOOT_KEY read failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	*output = r2;

	printf("[OTP] BAN_ROM_SEC_BOOT_KEY read value: 0x%llx\n", r2);

	return RV_SUCCESS;
}

uint64_t cm_otp_write_enable_aes(void)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_W_ENABLE_AES;
	r2 = 0;
	r3 = 0;

	printf("[OTP] ENABLE_AES program start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] ENABLE_AES program failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[OTP] ENABLE_AES program success\n");

	return RV_SUCCESS;
}

uint64_t cm_otp_read_enable_aes(uint64_t *output)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_R_ENABLE_AES;
	r2 = 0;
	r3 = 0;

	printf("[OTP] ENABLE_AES read start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] ENABLE_AES read failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	*output = r2;

	printf("[OTP] ENABLE_AES read value: 0x%llx\n", r2);

	return RV_SUCCESS;
}

/*
 ****************************************************************************
 * OTP functions for Anti-Rollback Protection*/

/*****************************************************************************/
uint64_t cm_otp_write_enable_antirbk(void)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_W_ENABLE_ANTIRBK;
	r2 = 0;
	r3 = 0;

	printf("[OTP] ENABLE_ANTIRBK program start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] ENABLE_ANTIRBK program failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[OTP] ENABLE_ANTIRBK program success\n");

	return RV_SUCCESS;
}

uint64_t cm_otp_read_enable_antirbk(uint64_t *output)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_R_ENABLE_ANTIRBK;
	r2 = 0;
	r3 = 0;

	printf("[OTP] ENABLE_ANTIRBK read start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] ENABLE_ANTIRBK read failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	*output = r2;

	printf("[OTP] ENABLE_ANTIRBK read value: 0x%llx\n", r2);

	return RV_SUCCESS;
}

uint64_t cm_otp_write_antirbk_non_sec_ap0(uint8_t *antirbk_ptr,
                                          uint32_t antirbk_len)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;
	OTP_DATA_T otp_data;

	if (antirbk_ptr == NULL) {
		printf("[OTP] invalid ANTIRBK_NS_AP0 addresss\n");
		return RV_OTP_PARAM_NULL_POINTER;
	}

	if (antirbk_len != OTP_MAX_ANTIRBK_NS_AP0_LEN) {
		printf("[OTP] invalid ANTIRBK_NS_AP0 length\n");
		return RV_OTP_PARAM_INVALID_ANTIRBK_NS_AP0_LEN;
	}

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_W_ANTIRBK_NS_AP0;
	r2 = VIRT_TO_PHYS(&otp_data);
	r3 = 0;

	printf("[OTP] ANTIRBK_NS_AP0 program start\n");

	memset((uint8_t *)&otp_data, 0, sizeof(OTP_DATA_T));
	memcpy(otp_data.key, antirbk_ptr, antirbk_len);
	otp_data.key_len = antirbk_len;
	otp_data.checksum = cm_otp_get_checksum(antirbk_ptr, antirbk_len);

	FLUSH_DCACHE_RANGE(&otp_data, sizeof(OTP_DATA_T));

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] ANTIRBK_NS_AP0 program failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[OTP] ANTIRBK_NS_AP0 program success\n");

	return RV_SUCCESS;
}

uint64_t cm_otp_read_antirbk_non_sec_ap0(uint64_t *antirbk)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_R_ANTIRBK_NS_AP0;
	r2 = 0;
	r3 = 0;

	printf("[OTP] ANTIRBK_NS_AP0 read start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] ANTIRBK_NS_AP0 read failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	*antirbk = r2;

	printf("[OTP] ANTIRBK_NS_AP0 read value: 0x%llx\n", r2); /////

	return RV_SUCCESS;
}

uint64_t cm_otp_update_antirbk_non_sec_ap0(uint64_t new_count_dec)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t old_count_dec;
	uint64_t byte_index;
	uint8_t new_count_otp[OTP_MAX_ANTIRBK_NS_AP0_CNT];

	if (new_count_dec == 0)
		return RV_SUCCESS;

	if (new_count_dec > OTP_MAX_ANTIRBK_NS_AP0_CNT) {
		printf("[OTP] ANTIRBK_NS_AP0 update invalid new count: %lld\n", new_count_dec);
		return RV_OTP_UPDATE_RNTIRBK_NS_AP0_INVALID_COUNT1;
	}

	ret = cm_otp_read_antirbk_non_sec_ap0(&old_count_dec);
	if (ret != RV_SUCCESS)
		return ret;

	if (new_count_dec < old_count_dec) {
		printf("[OTP] ANTIRBK_NS_AP0 update invalid count: old: %lld, new: %lld\n", \
		       old_count_dec, new_count_dec);
		return RV_OTP_UPDATE_RNTIRBK_NS_AP0_INVALID_COUNT2;
	}

	if (new_count_dec > old_count_dec) {
		memset(new_count_otp, 0, OTP_MAX_ANTIRBK_NS_AP0_CNT);

		byte_index = (new_count_dec - 1) / 8;

		if (new_count_dec % 8)
			new_count_otp[byte_index] = 1 << ((new_count_dec % 8) - 1);
		else
			new_count_otp[byte_index] = 1 << 7;

		ret = cm_otp_write_antirbk_non_sec_ap0(new_count_otp,
		                                       OTP_MAX_ANTIRBK_NS_AP0_LEN);
		if (ret != RV_SUCCESS)
			return ret;

		printf("[OTP] ANTIRBK_NS_AP0 update success: old: %lld, new: %lld\n", \
		       old_count_dec, new_count_dec);
	}

	return RV_SUCCESS;
}

uint64_t cm_otp_write_antirbk_non_sec_ap1(uint8_t *antirbk_ptr,
                                          uint32_t antirbk_len)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;
	OTP_DATA_T otp_data;

	if (antirbk_ptr == NULL) {
		printf("[OTP] invalid ANTIRBK_NS_AP1 addresss\n");
		return RV_OTP_PARAM_NULL_POINTER;
	}

	if (antirbk_len != OTP_MAX_ANTIRBK_NS_AP1_LEN) {
		printf("[OTP] invalid ANTIRBK_NS_AP1 length\n");
		return RV_OTP_PARAM_INVALID_ANTIRBK_NS_AP1_LEN;
	}

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_W_ANTIRBK_NS_AP1;
	r2 = VIRT_TO_PHYS(&otp_data);
	r3 = 0;

	printf("[OTP] ANTIRBK_NS_AP1 program start\n");

	memset((uint8_t *)&otp_data, 0, sizeof(OTP_DATA_T));
	memcpy(otp_data.key, antirbk_ptr, antirbk_len);
	otp_data.key_len = antirbk_len;
	otp_data.checksum = cm_otp_get_checksum(antirbk_ptr, antirbk_len);

	FLUSH_DCACHE_RANGE(&otp_data, sizeof(OTP_DATA_T));

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] ANTIRBK_NS_AP1 program failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[OTP] ANTIRBK_NS_AP1 program success\n");

	return RV_SUCCESS;
}

uint64_t cm_otp_read_antirbk_non_sec_ap1(uint64_t *antirbk)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_R_ANTIRBK_NS_AP1;
	r2 = 0;
	r3 = 0;

	printf("[OTP] ANTIRBK_NS_AP1 read start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] ANTIRBK_NS_AP1 read failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	*antirbk = r2;

	printf("[OTP] ANTIRBK_NS_AP1 read value: 0x%llx\n", r2);

	return RV_SUCCESS;
}

uint64_t cm_otp_write_antirbk_sec_ap(uint64_t antirbk_sec)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_W_ANTIRBK_S_AP;
	r2 = antirbk_sec & MASK_LSB32;
	r3 = 0;

	printf("[OTP] ANTIRBK_SEC_AP program start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] ANTIRBK_SEC_AP program failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[OTP] ANTIRBK_SEC_AP program success\n");

	return RV_SUCCESS;
}

uint64_t cm_otp_read_antirbk_sec_ap(uint64_t *antirbk_sec)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_R_ANTIRBK_S_AP;
	r2 = 0;
	r3 = 0;

	printf("[OTP] ANTIRBK_SEC_AP read start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] ANTIRBK_SEC_AP read failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	*antirbk_sec = r2;

	printf("[OTP] ANTIRBK_SEC_AP read value: 0x%llx\n", r2);

	return RV_SUCCESS;
}

uint64_t cm_otp_update_antirbk_sec_ap(uint64_t new_count_dec)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t old_count_dec;
	uint64_t new_count_otp;

	if (new_count_dec == 0)
		return RV_SUCCESS;

	if (new_count_dec > OTP_MAX_ANTIRBK_S_AP_CNT) {
		printf("[OTP] ANTIRBK_SEC_AP update invalid new count: %lld\n", new_count_dec);
		return RV_OTP_UPDATE_RNTIRBK_SEC_AP_INVALID_COUNT1;
	}

	ret = cm_otp_read_antirbk_sec_ap(&old_count_dec);
	if (ret != RV_SUCCESS)
		return ret;

	if (new_count_dec < old_count_dec) {
		printf("[OTP] ANTIRBK_SEC_AP update invalid count: old: %lld, new: %lld\n", \
		       old_count_dec, new_count_dec);
		return RV_OTP_UPDATE_RNTIRBK_SEC_AP_INVALID_COUNT2;
	}

	if (new_count_dec > old_count_dec) {
		new_count_otp = 1 << (new_count_dec - 1);

		ret = cm_otp_write_antirbk_sec_ap(new_count_otp);
		if (ret != RV_SUCCESS)
			return ret;

		printf("[OTP] ANTIRBK_SEC_AP update success: old: %lld, new: %lld\n", \
		       old_count_dec, new_count_dec);
	}

	return RV_SUCCESS;
}

uint64_t cm_otp_write_antirbk_sec_cp(uint64_t antirbk_sec)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_W_ANTIRBK_S_CP;
	r2 = antirbk_sec & MASK_LSB32;
	r3 = 0;

	printf("[OTP] ANTIRBK_SEC_CP program start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] ANTIRBK_SEC_CP program failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[OTP] ANTIRBK_SEC_CP program success\n");

	return RV_SUCCESS;
}

uint64_t cm_otp_read_antirbk_sec_cp(uint64_t *antirbk_sec)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_R_ANTIRBK_S_CP;
	r2 = 0;
	r3 = 0;

	printf("[OTP] ANTIRBK_SEC_CP read start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] ANTIRBK_SEC_CP read failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	*antirbk_sec = r2;

	printf("[OTP] ANTIRBK_SEC_CP read value: 0x%llx\n", r2);

	return RV_SUCCESS;
}

/*
 ****************************************************************************
 * OTP functions for Secure JTAG*/

/*****************************************************************************/
uint64_t cm_otp_write_sec_jtag_key(uint8_t *jtag_key_ptr,
                                   uint32_t jtag_key_len)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;
	OTP_DATA_T otp_data;

	if (jtag_key_ptr == NULL) {
		printf("[OTP] invalid JTAG_KEY addresss\n");
		return RV_OTP_PARAM_NULL_POINTER;
	}

	if (jtag_key_len != OTP_SEC_JTAG_KEY_LEN) {
		printf("[OTP] invalid JTAG_KEY length\n");
		return RV_OTP_PARAM_INVALID_KEY_LEN;
	}

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_W_SEC_JTAG_KEY;
	r2 = VIRT_TO_PHYS(&otp_data);
	r3 = 0;

	printf("[OTP] JTAG_KEY program start\n");

	memset((uint8_t *)&otp_data, 0, sizeof(OTP_DATA_T));
	memcpy(otp_data.key, jtag_key_ptr, jtag_key_len);
	otp_data.key_len = jtag_key_len;
	otp_data.checksum = cm_otp_get_checksum(jtag_key_ptr, jtag_key_len);

	FLUSH_DCACHE_RANGE(&otp_data, sizeof(OTP_DATA_T));

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] JTAG_KEY program failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[OTP] JTAG_KEY program success\n");

	return ret;
}

uint64_t cm_otp_check_sec_jtag_key(uint8_t *jtag_key_ptr, uint32_t jtag_key_len)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;
	OTP_DATA_T otp_data;

	if (jtag_key_ptr == NULL) {
		printf("[OTP] invalid JTAG_KEY addresss\n");
		return RV_OTP_PARAM_NULL_POINTER;
	}

	if (jtag_key_len != OTP_SEC_JTAG_KEY_LEN) {
		printf("[OTP] invalid JTAG_KEY length\n");
		return RV_OTP_PARAM_INVALID_KEY_LEN;
	}

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_C_SEC_JTAG_KEY;
	r2 = VIRT_TO_PHYS(&otp_data);
	r3 = 0;

	printf("[OTP] JTAG_KEY check start\n");

	memset((uint8_t *)&otp_data, 0, sizeof(OTP_DATA_T));
	memcpy(otp_data.key, jtag_key_ptr, jtag_key_len);
	otp_data.key_len = jtag_key_len;
	otp_data.checksum = cm_otp_get_checksum(jtag_key_ptr, jtag_key_len);

	FLUSH_DCACHE_RANGE(&otp_data, sizeof(OTP_DATA_T));

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret == RV_SUCCESS)
		printf("[OTP] JTAG_KEY check: valid key\n");
	else if (ret == RV_OTP_CHECK_KEY_NOT_FUSED)
		printf("[OTP] JTAG_KEY hasn't been programed yet\n");
	else if (ret == RV_OTP_CHECK_KEY_CMP_FAIL)
		printf("[OTP] JTAG_KEY check: invalid key\n");
	else
		printf("[OTP] JTAG_KEY check failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);

	return ret;
}

uint64_t cm_otp_write_jtag_sw_lock(void)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_W_SEC_JTAG_SW_LOCK;
	r2 = 0;
	r3 = 0;

	printf("[OTP] JTAG_SW_LOCK program start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] JTAG_SW_LOCK program failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[OTP] JTAG_SW_LOCK program success\n");

	return RV_SUCCESS;
}

uint64_t cm_otp_read_jtag_sw_lock(uint64_t *output)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_R_SEC_JTAG_SW_LOCK;
	r2 = 0;
	r3 = 0;

	printf("[OTP] JTAG_SW_LOCK read start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] JTAG_SW_LOCK read failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	*output = r2;

	printf("[OTP] JTAG_SW_LOCK read value: 0x%llx\n", r2);

	return RV_SUCCESS;
}

/*
 ****************************************************************************
 * OTP functions for customer specific key & flags*/

/*****************************************************************************/
uint64_t cm_otp_write_sw_custom_config1(uint8_t *sw_custom_config1_ptr,
                                        uint32_t sw_custom_config1_len)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;
	OTP_DATA_T otp_data;

	if (sw_custom_config1_ptr == NULL) {
		printf("[OTP] invalid SW_CUSTOM_CONFIG1 addresss\n");
		return RV_OTP_PARAM_NULL_POINTER;
	}

	if (sw_custom_config1_len != OTP_SW_CUSTOM_CONFIG1_LEN) {
		printf("[OTP] invalid SW_CUSTOM_CONFIG1 length\n");
		return RV_OTP_PARAM_INVALID_KEY_LEN;
	}

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_W_SW_CUSTOM_CONFIG1;
	r2 = VIRT_TO_PHYS(&otp_data);
	r3 = 0;

	printf("[OTP] SW_CUSTOM_CONFIG1 program start\n");

	memset((uint8_t *)&otp_data, 0, sizeof(OTP_DATA_T));
	memcpy(otp_data.key, sw_custom_config1_ptr, sw_custom_config1_len);
	otp_data.key_len = sw_custom_config1_len;
	otp_data.checksum = cm_otp_get_checksum(sw_custom_config1_ptr,
	                                        sw_custom_config1_len);

	FLUSH_DCACHE_RANGE(&otp_data, sizeof(OTP_DATA_T));

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] SW_CUSTOM_CONFIG1 program failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[OTP] SW_CUSTOM_CONFIG1 program success\n");

	return ret;
}

uint64_t cm_otp_check_sw_custom_config1(uint8_t *sw_custom_config1_ptr,
                                        uint32_t sw_custom_config1_len)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;
	OTP_DATA_T otp_data;

	if (sw_custom_config1_ptr == NULL) {
		printf("[OTP] invalid SW_CUSTOM_CONFIG1 addresss\n");
		return RV_OTP_PARAM_NULL_POINTER;
	}

	if (sw_custom_config1_len != OTP_SW_CUSTOM_CONFIG1_LEN) {
		printf("[OTP] invalid SW_CUSTOM_CONFIG1 length\n");
		return RV_OTP_PARAM_INVALID_KEY_LEN;
	}

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_C_SW_CUSTOM_CONFIG1;
	r2 = VIRT_TO_PHYS(&otp_data);
	r3 = 0;

	printf("[OTP] SW_CUSTOM_CONFIG1 check start\n");

	memset((uint8_t *)&otp_data, 0, sizeof(OTP_DATA_T));
	memcpy(otp_data.key, sw_custom_config1_ptr, sw_custom_config1_len);
	otp_data.key_len = sw_custom_config1_len;
	otp_data.checksum = cm_otp_get_checksum(sw_custom_config1_ptr,
	                                        sw_custom_config1_len);

	FLUSH_DCACHE_RANGE(&otp_data, sizeof(OTP_DATA_T));

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret == RV_SUCCESS)
		printf("[OTP] SW_CUSTOM_CONFIG1 check: valid key\n");
	else if (ret == RV_OTP_CHECK_KEY_NOT_FUSED)
		printf("[OTP] SW_CUSTOM_CONFIG1 hasn't been programed yet\n");
	else if (ret == RV_OTP_CHECK_KEY_CMP_FAIL)
		printf("[OTP] SW_CUSTOM_CONFIG1 check: invalid key\n");
	else
		printf("[OTP] SW_CUSTOM_CONFIG1 check failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);

	return ret;
}

uint64_t cm_otp_write_custom_flag(uint64_t index)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	if ((index < 5) || (index > 31)) {
		printf("[OTP] CUSTOM_FLAG program: bit index error\n");
		return 0;
	}

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_W_CUSTOM_FLAG;
	r2 = index;
	r3 = 0;

	printf("[OTP] CUSTOM_FLAG program start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] CUSTOM_FLAG program failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[OTP] CUSTOM_FLAG program success\n");

	return RV_SUCCESS;
}

uint64_t cm_otp_read_custom_flag(uint64_t index, uint64_t *output)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	if ((index < 5) || (index > 31)) {
		printf("[OTP] CUSTOM_FLAG read: bit index error\n");
		return 0;
	}

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_R_CUSTOM_FLAG;
	r2 = index;
	r3 = 0;

	printf("[OTP] CUSTOM_FLAG read start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] CUSTOM_FLAG read failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return r0;
	}

	*output = r2;

	printf("[OTP] CUSTOM_FLAG read value: 0x%llx\n", r2);

	return RV_SUCCESS;
}

/*
 ****************************************************************************
 * OTP functions for MCD use cases*/

/*****************************************************************************/
uint64_t cm_otp_write_use_preorder(void)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_W_USE_PREORDER_KEY;
	r2 = 0;
	r3 = 0;

	printf("[OTP] USE_PREORDER_KEY program start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] USE_PREORDER_KEY program failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[OTP] USE_PREORDER_KEY program success\n");

	return RV_SUCCESS;
}

uint64_t cm_otp_read_use_preorder(uint64_t *output)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_R_USE_PREORDER_KEY;
	r2 = 0;
	r3 = 0;

	printf("[OTP] USE_PREORDER_KEY read start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] USE_PREORDER_KEY read failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	*output = r2;

	printf("[OTP] USE_PREORDER_KEY read value: 0x%llx\n", r2);

	return RV_SUCCESS;
}

uint64_t cm_otp_write_preorder(uint64_t preorder)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_W_PREORDER;
	r2 = preorder & MASK_LSB8;
	r3 = 0;

	printf("[OTP] PREORDER program start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] PREORDER program failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[OTP] PREORDER program success\n");

	return RV_SUCCESS;
}

uint64_t cm_otp_read_preorder(uint64_t *preorder)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_R_PREORDER;
	r2 = 0;
	r3 = 0;

	printf("[OTP] PREORDER read start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] PREORDER read failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	*preorder = r2;

	printf("[OTP] PREORDER read value: 0x%llx\n", r2);

	return RV_SUCCESS;
}

uint64_t cm_otp_write_enable_modelid(void)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_W_ENABLE_MODEL_ID;
	r2 = 0;
	r3 = 0;

	printf("[OTP] ENABLE_MODEL_ID program start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] ENABLE_MODEL_ID program failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[OTP] ENABLE_MODEL_ID program success\n");

	return RV_SUCCESS;
}

uint64_t cm_otp_read_enable_modelid(uint64_t *output)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_R_ENABLE_MODEL_ID;
	r2 = 0;
	r3 = 0;

	printf("[OTP] ENABLE_MODEL_ID read start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] ENABLE_MODEL_ID read failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	*output = r2;

	printf("[OTP] ENABLE_MODEL_ID read value: 0x%llx\n", r2);

	return RV_SUCCESS;
}

uint64_t cm_otp_write_modelid(uint64_t model_id)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_W_MODEL_ID;
	r2 = model_id & MASK_LSB32;
	r3 = 0;

	printf("[OTP] MODEL_ID program start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] MODEL_ID program failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[OTP] MODEL_ID program success\n");

	return RV_SUCCESS;
}

uint64_t cm_otp_read_modelid(uint64_t *model_id)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_R_MODEL_ID;
	r2 = 0;
	r3 = 0;

	printf("[OTP] MODEL_ID read start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] MODEL_ID read failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	*model_id = r2;

	printf("[OTP] MODEL_ID read value: 0x%llx\n", r2);

	return RV_SUCCESS;
}

uint64_t cm_otp_write_commercial(void)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_W_COMMERCIAL;
	r2 = 0;
	r3 = 0;

	printf("[OTP] COMMERCIAL program start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret == RV_OTP_TEST_BIT_SET_ALREADY) {
		printf("[OTP] TEST bit set already!!\n");
	} else if (ret != RV_SUCCESS) {
		printf("[OTP] COMMERCIAL program failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[OTP] COMMERCIAL program success\n");

	return RV_SUCCESS;
}

uint64_t cm_otp_read_commercial(uint64_t *output)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_R_COMMERCIAL;
	r2 = 0;
	r3 = 0;

	printf("[OTP] COMMERCIAL read start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] COMMERCIAL read failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	*output = r2;

	printf("[OTP] COMMERCIAL read value: 0x%llx\n", r2);

	return RV_SUCCESS;
}

uint64_t cm_otp_write_test(void)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_W_TEST;
	r2 = 0;
	r3 = 0;

	printf("[OTP] TEST program start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret == RV_OTP_COMMERCIAL_BIT_SET_ALREADY) {
		printf("[OTP] COMMERCIAL bit set already!!\n");
	} else if (ret != RV_SUCCESS) {
		printf("[OTP] TEST program failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[OTP] TEST program success\n");

	return RV_SUCCESS;
}

uint64_t cm_otp_read_test(uint64_t *output)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_R_TEST;
	r2 = 0;
	r3 = 0;

	printf("[OTP] TEST read start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] TEST read failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	*output = r2;

	printf("[OTP] TEST read value: 0x%llx\n", r2);

	return RV_SUCCESS;
}

uint64_t cm_otp_write_warranty(void)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_W_WARRANTY;
	r2 = 0;
	r3 = 0;

	printf("[OTP] WARRANTY program start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] WARRANTY program failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[OTP] WARRANTY program success\n");

	return RV_SUCCESS;
}

uint64_t cm_otp_read_warranty(uint64_t *output)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_R_WARRANTY;
	r2 = 0;
	r3 = 0;

	printf("[OTP] WARRANTY read start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] WARRANTY read failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	*output = r2;

	printf("[OTP] WARRANTY read value: 0x%llx\n", r2);

	return RV_SUCCESS;
}

uint64_t cm_otp_write_cp_disable(void)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t r0, r1, r2, r3;

	r0 = SMC_AARCH64_PREFIX | SMC_CM_OTP_CONTROL;
	r1 = CMD_W_CP_DISABLE;
	r2 = 0;
	r3 = 0;

	printf("[OTP] CP_DISABLE program start\n");

	ret = exynos_cm_smc(&r0, &r1, &r2, &r3);
	if (ret != RV_SUCCESS) {
		printf("[OTP] CP_DISABLE program failed: \
\t\tr0:0x%llx, r1:0x%llx, r2:0x%llx, r3:0x%llx\n", r0, r1, r2, r3);
		return ret;
	}

	printf("[OTP] CP_DISABLE program success\n");

	return RV_SUCCESS;
}

uint64_t cm_otp_enable_secure_boot(uint8_t *rom_sec_boot_key_ptr,
                                   uint32_t rom_sec_boot_key_len)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t secure_boot_status = 0;

	printf("[OTP] Secure Boot enable: start\n");

	ret = cm_otp_read_use_rom_sec_boot_key(&secure_boot_status);
	if (ret != RV_SUCCESS)
		goto OUT;

	if (secure_boot_status == 1) {
		printf("[OTP] Secure Boot enabled already\n");
		return RV_SUCCESS;
	}

	ret = cm_otp_write_rom_sec_boot_key(rom_sec_boot_key_ptr,
	                                    rom_sec_boot_key_len);
	if (ret != RV_SUCCESS)
		goto OUT;

	ret = cm_otp_write_use_rom_sec_boot_key();
OUT:
	if (ret != RV_SUCCESS)
		printf("[OTP] Secure Boot enable: failed: 0x%llx\n", ret);
	else
		printf("[OTP] Secure Boot enable: success\n");

	return ret;
}

uint64_t cm_otp_enable_secure_jtag(uint8_t *jtag_key_ptr, uint32_t jtag_key_len)
{
	uint64_t ret = RV_SUCCESS;
	uint64_t secure_jtag_status = 0;

	printf("[OTP] Secure JTAG enable: start\n");

	ret = cm_otp_read_jtag_sw_lock(&secure_jtag_status);
	if (ret != RV_SUCCESS)
		goto OUT;

	if (secure_jtag_status == 1) {
		printf("[OTP] Secure JTAG enabled already\n");
		return RV_SUCCESS;
	}

	ret = cm_otp_write_sec_jtag_key(jtag_key_ptr, jtag_key_len);
	if (ret != RV_SUCCESS)
		goto OUT;

	ret = cm_otp_write_jtag_sw_lock();
OUT:
	if (ret != RV_SUCCESS)
		printf("[OTP] Secure JTAG enable: failed: 0x%llx\n", ret);
	else
		printf("[OTP] Secure JTAG enable: success\n");

	return ret;
}
