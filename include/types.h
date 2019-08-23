/*
 * (C) Copyright 2019 SAMSUNG Electronics
 * Kiwoong Kim <kwmad.kim@samsung.com>
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 *
 */

#ifndef __PIT_TYPE_H__
#define __PIT_TYPE_H__

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef unsigned char __u8;
typedef unsigned short __u16;
typedef unsigned int __u32;
typedef unsigned long long __u64;

typedef __u16  __le16;
typedef __u16  __be16;
typedef __u32  __le32;
typedef __u32  __be32;
typedef __u64  __le64;
typedef __u64  __be64;

#define be16_to_cpu(x) \
		((((x) & 0xff00) >> 8) | \
		 (((x) & 0x00ff) << 8))
#define cpu_to_be16(x) be16_to_cpu(x)

#define be32_to_cpu(x) \
		((((x) & 0xff000000) >> 24) | \
		 (((x) & 0x00ff0000) >>  8) | \
		 (((x) & 0x0000ff00) <<  8) | \
		 (((x) & 0x000000ff) << 24))
#define cpu_to_be32(x) be32_to_cpu(x)

#define be64_to_cpu(x) \
		((((x) & 0x00000000000000ff) << 56) | \
		 (((x) & 0x000000000000ff00) << 40) | \
		 (((x) & 0x0000000000ff0000) << 24) | \
		 (((x) & 0x00000000ff000000) << 8) | \
		 (((x) & 0x000000ff00000000) >> 8) | \
		 (((x) & 0x0000ff0000000000) >> 24) | \
		 (((x) & 0x00ff000000000000) >> 40) | \
		 (((x) & 0xff00000000000000) >> 56))
#define cpu_to_be64(x) be64_to_cpu(x)

#endif
