/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __GUID_H__
#define __GUID_H__

#define UID_STR_STD		0
#define	UID_STR_GUID		1

#define UID_STR_LEN		36
#define UID_BIN_LEN		16

unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base);
unsigned long long simple_strtoull (const char *cp, char **endp, unsigned int base);
int uid_str_valid(const char *uid);
int uid_str_to_bin(char *uid_str, unsigned char *uid_bin, int str_format);
void uid_bin_to_str(unsigned char *uid_bin, char *uid_str, int str_format);
#endif
