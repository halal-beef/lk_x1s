/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <config.h>
#include <string.h>
#include <reg.h>
#include <part.h>
#include <err.h>
#include <arch/arch_ops.h>
#include <dev/boot.h>
#include <lib/console.h>
#include <platform/smc.h>
#include <platform/secure_boot.h>
#include <platform/xct.h>
#include <platform/wdt_recovery.h>
#include <lib/sysparam.h>

#define XCT_SECURE_CHECK_DISABLE
#if !defined(XCT_SECURE_CHECK_DISABLE)
static u32 xct_cs_by_el3(u32 size)
{
	u8 uHash[SHA256_DIGEST_LEN]={0};
	u32 uChkSum = 0;

	el3_sss_hash_digest(XCT_BIN_HEADER_ADDR + XCT_BIN_HEADER_SIZE, size - XCT_BIN_HEADER_SIZE, ALG_SHA256, (void *) uHash);

	uChkSum = (uHash[0]) + ((uHash[1]) << 8) + ((uHash[2]) << 16) + ((uHash[3]) << 24);

	return uChkSum;
}
#endif

static int parse_xct_header(unsigned int *ret_size, unsigned int *ret_cs)
{
	u32 uNumOfSectors;
	u32 uBytesOfFW;
	u32 uChecksumOfFW;

	uNumOfSectors = *((u32 *)(XCT_BIN_HEADER_PARSE_SIZE));
	if((uNumOfSectors < MINNUM_DOWNLOADING_SECTORS) || \
			(uNumOfSectors > MAXIMUM_DOWNLOADING_SECTORS))
		return -1;

	uBytesOfFW = (uNumOfSectors) << 9;	// Current V2.4

	*ret_size = uBytesOfFW;

	uChecksumOfFW = *((u32 *)(XCT_BIN_HEADER_PARSE_CS));
	*ret_cs = uChecksumOfFW;

	// for verification of signatfure V2.4
	*((u32 *)(XCT_BIN_HEADER_PARSE_CS)) = 0;

	return 0;
}

void xct_run_2nd_boot(void)
{
	/* UE_TASK: How to run dnw protocol to donwload xct bin */
}

bool is_xct_boot(void)
{
	u8 xct_on[8] = { 0, };

	if (sysparam_read("xct", &xct_on, 8) < 0)
		return false;

	printf("XCT Value: %s\n", xct_on);

	if (!strcmp((const char *) xct_on, "xct-on"))
		return true;
	else
		return false;
}

extern void arm_generic_timer_disable(void);

int cmd_xct(int argc, const cmd_args *argv)
{
	unsigned int size, ret_sec_xct, cs;
	void *part;
#if !defined(XCT_SECURE_CHECK_DISABLE)
	unsigned int ret_secure_ldfw, cs_from_el3;
#endif
	/* Copy ECT Data from BL2 */
	if (readl(XCT_ECT_SOURCE_ADDR) != 0x41524150 ) {
		/* "PARA" magic number for ECT */
		printf("%c[%d;%dm",27,1,33);
		printf("CAUTION: ECT is not on 0x%08X\n", XCT_ECT_SOURCE_ADDR);
		printf("%c[%dm",27,0);
	} else
		memcpy((void *)XCT_ECT_ADDR, (void *)XCT_ECT_SOURCE_ADDR, XCT_ECT_SIZE);

	/* Check 2nd boot */
	if (!is_first_boot()) {
		xct_run_2nd_boot();
	}
	clear_wdt_recovery_settings();
	/* Load LDFW for xct */
	part = part_get("xct_ldfw");
	if (!part) {
		printf("not found xct ldfw partition\n");
		return ERR_NOT_FOUND;
	}
	part_read(part, (void *)XCT_LDFW_ADDR);
#if !defined(XCT_SECURE_CHECK_DISABLE)
	/* Check Securiy signing */
#if defined(CONFIG_SB40)
	ret_secure_ldfw = el3_verify_signature_using_image(XCT_LDFW_ADDR, XCT_LDFW_SIZE,
			XCT_LDFW_ADDR + XCT_LDFW_SIZE - sizeof(SB_V40_SIGN_FIELD), OTHER_NS_BIN);
#else
	ret_secure_ldfw = el3_verify_signature_using_image(XCT_LDFW_ADDR, XCT_LDFW_SIZE, TRUST_MEASUREMENT_KERNEL);
#endif
	if (ret_secure_ldfw != 0xffffffff) {
		if (ret_secure_ldfw) {
			printf("xct ldfw secure check fail!!\n");
			return ERR_NOT_VALID;
		}
	} else
		ret_secure_ldfw = 0x0;
#endif
	/* Load xct bin */
	part = part_get("xct");
	if (!part) {
		printf("not found xct partition\n");
		return ERR_NOT_FOUND;
	}
	part_read_partial(part, (void *)XCT_BIN_LOAD_ADDR, 0, 512);
	ret_sec_xct = parse_xct_header(&size, &cs);
	if (ret_sec_xct) {
		printf("xct bin header fail\n");
		return ERR_NOT_VALID;
	}
#if !defined(XCT_SECURE_CHECK_DISABLE)
	cs_from_el3 = xct_cs_by_el3(size);
	if (cs_from_el3 != cs) {
		printf("xct bin checkssum fail!!\n");
		printf("rx cs:0x%x, calc cs:0x%x\n", cs, cs_from_el3);
		ret_sec_xct = ERR_CHECKSUM_FAIL;
	} else {
#if defined(CONFIG_SB40)
		ret_sec_xct = el3_verify_signature_using_image(XCT_BIN_LOAD_ADDR, size,
				XCT_BIN_LOAD_ADDR + size - sizeof(SB_V40_SIGN_FIELD), OTHER_NS_BIN);
#else
		ret_sec_xct = el3_verify_signature_using_image(XCT_BIN_LOAD_ADDR, size, TRUST_MEASUREMENT_KERNEL);
#endif
		if (ret_sec_xct == 0xffffffff)
			ret_sec_xct = 0;
	}
	if (ret_sec_xct) {
		printf("xct bin secure check fail, 0x%x!!\n", ret_sec_xct);
		return ERR_NOT_VALID;
	}
#endif
	/* notify EL3 Monitor end of bootloader */
	exynos_smc(SMC_CMD_END_OF_BOOTLOADER, 0, 0, 0);

	/* before jumping to xct. disble arch_timer */
	arm_generic_timer_disable();

	printf("jump to XCT!!\n");
	void (*xct_entry)(void);

	xct_entry = (void (*)(void))XCT_RUN_ADDR;
	xct_entry();

	return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("xct", "start exynos chip test bin", &cmd_xct)
STATIC_COMMAND_END(xct);


