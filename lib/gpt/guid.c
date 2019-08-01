/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <errno.h>
#include <malloc.h>
#include <guid.h>
#include <part.h>
#include <lib/cksum.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <ctype.h>

unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base)
{
	unsigned long result = 0, value;

	if (*cp == '0') {
		cp++;
		if ((*cp == 'x') && isxdigit(cp[1])) {
			base = 16;
			cp++;
		}
		if (!base) {
			base = 8;
		}
	}
	if (!base) {
		base = 10;
	}
	while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
					? toupper(*cp) : *cp)-'A'+10) < base) {
		result = result*base + value;
		cp++;
	}
	if (endp)
		*endp = (char *)cp;
	return result;
}

unsigned long long simple_strtoull(const char *cp, char **endp, unsigned int base)
{
	unsigned long long result = 0, value;

	if (*cp == '0') {
		cp++;
		if ((*cp == 'x') && isxdigit(cp[1])) {
			base = 16;
			cp++;
		}
		if (!base) {
			base = 8;
		}
	}
	if (!base) {
		base = 10;
	}
	while (isxdigit(*cp) && (value = isdigit(*cp)
				? *cp - '0'
				: (islower(*cp) ? toupper(*cp) : *cp) - 'A' + 10) < base) {
		result = result * base + value;
		cp++;
	}
	if (endp)
		*endp = (char *) cp;
		return result;
}

int uid_str_valid(const char *uid)
{
	int i, valid;

	if (!uid)
		return 0;

	for (i = 0, valid = 1; uid[i] && valid; i++) {
		switch (i) {
		case 8:
		case 13:
		case 18:
		case 23:
			valid = (uid[i] == '-');
			break;
		default:
			valid = isxdigit(uid[i]);
			break;
		}
	}

	if ((i != UID_STR_LEN) || !valid)
		return 0;

	return 1;
}

int uid_str_to_bin(char *uid_str, unsigned char *uid_bin, int str_format)
{
	uint16_t tmp16;
	uint32_t tmp32;
	uint64_t tmp64;

	if (!uid_str_valid(uid_str))
		return -EINVAL;

	if (str_format == UID_STR_STD) {
		tmp32 = cpu_to_be32(simple_strtoul(uid_str, NULL, 16));
		memcpy(uid_bin, &tmp32, 4);

		tmp16 = cpu_to_be16(simple_strtoul(uid_str + 9, NULL, 16));
		memcpy(uid_bin + 4, &tmp16, 2);

		tmp16 = cpu_to_be16(simple_strtoul(uid_str + 14, NULL, 16));
		memcpy(uid_bin + 6, &tmp16, 2);
	} else {
		tmp32 = simple_strtoul(uid_str, NULL, 16);
		memcpy(uid_bin, &tmp32, 4);

		tmp16 = simple_strtoul(uid_str + 9, NULL, 16);
		memcpy(uid_bin + 4, &tmp16, 2);

		tmp16 = simple_strtoul(uid_str + 14, NULL, 16);
		memcpy(uid_bin + 6, &tmp16, 2);
	}

	tmp16 = cpu_to_be16(simple_strtoul(uid_str + 19, NULL, 16));
	memcpy(uid_bin + 8, &tmp16, 2);

	tmp64 = cpu_to_be64(simple_strtoull(uid_str + 24, NULL, 16));
	memcpy(uid_bin + 10, (char *)&tmp64 + 2, 6);

	return 0;
}

void uid_bin_to_str(unsigned char *uid_bin, char *uid_str, int str_format)
{
	const u8 uuid_format[UID_BIN_LEN] = {0, 1, 2, 3, 4, 5, 6, 7, 8,
						  9, 10, 11, 12, 13, 14, 15};
	const u8 guid_format[UID_BIN_LEN] = {3, 2, 1, 0, 5, 4, 7, 6, 8,
						  9, 10, 11, 12, 13, 14, 15};
	const u8 *char_order;
	int i;

	if (str_format == UID_STR_STD)
		char_order = uuid_format;
	else
		char_order = guid_format;

	for (i = 0; i < 16; i++) {
		sprintf(uid_str, "%02x", uid_bin[char_order[i]]);
		uid_str += 2;
		switch (i) {
		case 3:
		case 5:
		case 7:
		case 9:
			*uid_str++ = '-';
			break;
		}
	}
}
