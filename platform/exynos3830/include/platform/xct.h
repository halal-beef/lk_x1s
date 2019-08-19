/*
 * (C) Copyright 2019 SAMSUNG Electronics
 * Author: sunghyun.na@samsung.com (Sung-Hyun Na)
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted
 * transcribed, stored in a retrieval system or translated into any human or computer language in an
 * form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 *
 */

#ifndef _INCLUDE_PLATFORM_XCT_H_
#define _INCLUDE_PLATFORM_XCT_H_

#define XCT_CMD_BUF_SZ			64
#define MINNUM_DOWNLOADING_SECTORS	3
#define	MAXIMUM_DOWNLOADING_SECTORS	(100*1024*1024/512)

#define XCT_ECT_ADDR		0x83FE0000
#define XCT_ECT_SOURCE_ADDR	0x90000000
#define XCT_ECT_SIZE		(80 * 1024)

#define XCT_LDFW_ADDR		0x83E00000
#define XCT_LDFW_SIZE		(60 * 1024)

#define XCT_RUN_ADDR		0x80080000
#define XCT_BIN_ADDR		0x8007FC00
#define XCT_BIN_HEADER_SIZE	16

#define XCT_BIN_LOAD_ADDR		XCT_BIN_ADDR - XCT_BIN_HEADER_SIZE
#define XCT_BIN_HEADER_ADDR		(XCT_BIN_LOAD_ADDR)
#define XCT_BIN_HEADER_PARSE_SIZE	(XCT_BIN_HEADER_ADDR+0x0)
#define XCT_BIN_HEADER_PARSE_CS		(XCT_BIN_HEADER_ADDR+0x4)
#define XCT_BIN_HEADER_PARSE_MAGIC	(XCT_BIN_HEADER_ADDR+0x8)

#define XCT_KIND_OF_HASH		ALG_SHA256

extern bool is_xct_boot(void);
extern int cmd_xct(int argc, const cmd_args *argv);

#endif /* _INCLUDE_PLATFORM_XCT_H_ */
