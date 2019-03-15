/**
 * @file  otp_v20.h
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

#ifndef _OTP_V20_H_
#define _OTP_V20_H_

/*
 ****************************************************************************
 * defines for general purpose
 *****************************************************************************/
#define VIRT_TO_PHYS(_virt_addr_) ((uint64_t)(_virt_addr_))
#define FLUSH_DCACHE_RANGE(addr, length)

#define CACHE_WRITEBACK_SHIFT				6
#define CACHE_WRITEBACK_GRANULE				(1 << CACHE_WRITEBACK_SHIFT)

#define MASK_LSB8					0x00000000000000FFULL
#define MASK_LSB16					0x000000000000FFFFULL
#define MASK_LSB32					0x00000000FFFFFFFFULL

/* SMC ID for OTP control */
#define SMC_AARCH64_PREFIX				(0xC2000000)
#define SMC_CM_OTP_CONTROL				(0x1014)

#define BANK_SIZE_IN_BYTE				128
#define OTP_ROM_SEC_BOOT_KEY_LEN			32
#define OTP_SEC_JTAG_KEY_LEN				64
#define OTP_SW_CUSTOM_CONFIG1_LEN			32
#define OTP_MAX_ANTIRBK_NS_AP0_LEN			16
#define OTP_MAX_ANTIRBK_NS_AP1_LEN			12

#define OTP_MAX_ANTIRBK_S_AP_CNT			32
#define OTP_MAX_ANTIRBK_NS_AP0_CNT			128

/*
 ****************************************************************************
 * ERROR Codes
 *****************************************************************************/
#define RV_SUCCESS					0

#define RV_OTP_TEST_BIT_SET_ALREADY			0x3020A
#define RV_OTP_COMMERCIAL_BIT_SET_ALREADY		0x3020B

#define RV_OTP_LDFW_NOT_LOADED				0xFFFFFFFF
#define RV_OTP_CHECK_KEY_CMP_FAIL			0x30213
#define RV_OTP_CHECK_KEY_NOT_FUSED			0x30214

#define RV_OTP_PARAM_NULL_POINTER			0x85000
#define RV_OTP_PARAM_INVALID_KEY_LEN			0x85001
#define RV_OTP_PARAM_INVALID_ANTIRBK_NS_AP0_LEN		0x85002
#define RV_OTP_PARAM_INVALID_ANTIRBK_NS_AP1_LEN		0x85003

#define RV_OTP_UPDATE_RNTIRBK_SEC_AP_INVALID_COUNT1	0x85010
#define RV_OTP_UPDATE_RNTIRBK_SEC_AP_INVALID_COUNT2	0x85011
#define RV_OTP_UPDATE_RNTIRBK_NS_AP0_INVALID_COUNT1	0x85012
#define RV_OTP_UPDATE_RNTIRBK_NS_AP0_INVALID_COUNT2	0x85013


/*
 ****************************************************************************
 * OTP commands
 ****************************************************************************
 * commands for program*/
enum {
	CMD_W_ROM_SEC_BOOT_KEY = 0x1,
	CMD_W_USE_ROM_SEC_BOOT_KEY,
	CMD_W_BAN_ROM_SEC_BOOT_KEY,
	CMD_W_ENABLE_AES,
	CMD_W_SEC_JTAG_KEY,
	CMD_W_SEC_JTAG_SW_LOCK,
	CMD_W_ANTIRBK_NS_AP0,
	CMD_W_ANTIRBK_NS_AP1,
	CMD_W_ANTIRBK_S_AP,
	CMD_W_ANTIRBK_S_CP,
	CMD_W_SW_CUSTOM_CONFIG1,
	CMD_W_CUSTOM_FLAG,
	CMD_W_PREORDER,
	CMD_W_MODEL_ID,
	CMD_W_COMMERCIAL,
	CMD_W_TEST,
	CMD_W_WARRANTY,
	CMD_W_CP_DISABLE,
	CMD_W_USE_PREORDER_KEY,
	CMD_W_ENABLE_ANTIRBK,
	CMD_W_ENABLE_MODEL_ID,
};

/* commands for read & verification */
enum {
	CMD_C_ROM_SEC_BOOT_KEY = 0x100,
	CMD_R_USE_ROM_SEC_BOOT_KEY,
	CMD_R_BAN_ROM_SEC_BOOT_KEY,
	CMD_R_ENABLE_AES,
	CMD_C_SEC_JTAG_KEY,
	CMD_R_SEC_JTAG_SW_LOCK,
	CMD_R_ANTIRBK_NS_AP0,
	CMD_R_ANTIRBK_NS_AP1,
	CMD_R_ANTIRBK_S_AP,
	CMD_R_ANTIRBK_S_CP,
	CMD_C_SW_CUSTOM_CONFIG1,
	CMD_R_CUSTOM_FLAG,
	CMD_R_PREORDER,
	CMD_R_MODEL_ID,
	CMD_R_COMMERCIAL,
	CMD_R_TEST,
	CMD_R_WARRANTY,
	CMD_R_USE_PREORDER_KEY,
	CMD_R_ENABLE_ANTIRBK,
	CMD_R_ENABLE_MODEL_ID,
};

/* otp command id for debug and test */
enum {
	CMD_DEBUG_BASE_ID = 0x1000,
	CMD_R_SMC_TEST,
	CMD_R_CHIP_ID,
	CMD_W_OTP_SW_BLOCK,
};

/*
 ****************************************************************************
 * OTP functions for Test
 *****************************************************************************/
uint64_t string_to_dec(char str[]);
uint64_t cm_otp_smc_test(uint64_t test_num);
uint64_t cm_otp_read_chip_id(uint64_t *chip_id);
uint64_t cm_otp_sw_block_enable(void);

/*
 ****************************************************************************
 * OTP functions for Secure Boot
 *****************************************************************************/
uint64_t cm_otp_write_rom_sec_boot_key(uint8_t *rom_sec_boot_key_ptr,
                                       uint32_t rom_sec_boot_key_len);
uint64_t cm_otp_check_rom_sec_boot_key(uint8_t *rom_sec_boot_key_ptr,
                                       uint32_t rom_sec_boot_key_len);
uint64_t cm_otp_write_use_rom_sec_boot_key(void);
uint64_t cm_otp_read_use_rom_sec_boot_key(uint64_t *output);
uint64_t cm_otp_write_ban_rom_sec_boot_key(void);
uint64_t cm_otp_read_ban_rom_sec_boot_key(uint64_t *output);
uint64_t cm_otp_write_enable_aes(void);
uint64_t cm_otp_read_enable_aes(uint64_t *output);
uint64_t cm_otp_enable_secure_boot(uint8_t *rom_sec_boot_key_ptr,
                                   uint32_t rom_sec_boot_key_len);

/*
 ****************************************************************************
 * OTP functions for Anti-Rollback Protection
 *****************************************************************************/
uint64_t cm_otp_write_enable_antirbk(void);
uint64_t cm_otp_read_enable_antirbk(uint64_t *output);
uint64_t cm_otp_write_antirbk_non_sec_ap0(uint8_t *antirbk,
                                          uint32_t antirbk_len);
uint64_t cm_otp_read_antirbk_non_sec_ap0(uint64_t *antirbk);
uint64_t cm_otp_update_antirbk_non_sec_ap0(uint64_t new_count_dec);
uint64_t cm_otp_write_antirbk_non_sec_ap1(uint8_t *antirbk,
                                          uint32_t antirbk_len);
uint64_t cm_otp_read_antirbk_non_sec_ap1(uint64_t *antirbk);
uint64_t cm_otp_write_antirbk_sec_ap(uint64_t antirbk_sec);
uint64_t cm_otp_read_antirbk_sec_ap(uint64_t *antirbk_sec);
uint64_t cm_otp_write_antirbk_sec_cp(uint64_t antirbk_sec);
uint64_t cm_otp_read_antirbk_sec_cp(uint64_t *antirbk_sec);
uint64_t cm_otp_update_antirbk_sec_ap(uint64_t new_count_dec);

/*
 ****************************************************************************
 * OTP functions for Secure JTAG
 *****************************************************************************/
uint64_t cm_otp_write_sec_jtag_key(uint8_t *jtag_key_ptr,
                                   uint32_t jtag_key_len);
uint64_t cm_otp_check_sec_jtag_key(uint8_t *jtag_key_ptr,
                                   uint32_t jtag_key_len);
uint64_t cm_otp_write_jtag_sw_lock(void);
uint64_t cm_otp_read_jtag_sw_lock(uint64_t *output);
uint64_t cm_otp_enable_secure_jtag(uint8_t *jtag_key_ptr, uint32_t jtag_key_len);

/*
 ****************************************************************************
 * OTP functions for customer specific key & flags
 *****************************************************************************/
uint64_t cm_otp_write_sw_custom_config1(uint8_t *sw_custom_config1_ptr,
                                        uint32_t sw_custom_config1_len);
uint64_t cm_otp_check_sw_custom_config1(uint8_t *sw_custom_config1_ptr,
                                        uint32_t sw_custom_config1_len);
uint64_t cm_otp_write_custom_flag(uint64_t index);
uint64_t cm_otp_read_custom_flag(uint64_t index, uint64_t *output);

/*
 ****************************************************************************
 * OTP functions for MCD use cases
 *****************************************************************************/
uint64_t cm_otp_write_use_preorder(void);
uint64_t cm_otp_read_use_preorder(uint64_t *output);
uint64_t cm_otp_write_preorder(uint64_t preorder);
uint64_t cm_otp_read_preorder(uint64_t *preorder);
uint64_t cm_otp_write_enable_modelid(void);
uint64_t cm_otp_read_enable_modelid(uint64_t *output);
uint64_t cm_otp_write_modelid(uint64_t model_id);
uint64_t cm_otp_read_modelid(uint64_t *model_id);
uint64_t cm_otp_write_commercial(void);
uint64_t cm_otp_read_commercial(uint64_t *output);
uint64_t cm_otp_write_test(void);
uint64_t cm_otp_read_test(uint64_t *output);
uint64_t cm_otp_write_warranty(void);
uint64_t cm_otp_read_warranty(uint64_t *output);
uint64_t cm_otp_write_cp_disable(void);

#endif /* _OTP_V20_H_ */
