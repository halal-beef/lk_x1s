/* SPDX-License-Identifier: MIT */
/*
 * Copyright (c) 2024, Umer Uddin <umer.uddin@mentallysanemainliners.org>
 */

#include <string.h>
#include <ctype.h>

int stricmp(const char *s1, const char *s2) {
    unsigned char c1 = '\0';
    unsigned char c2 = '\0';

    do {
        c1 = *s1++;
        c2 = *s2++;
        c1 = tolower(c1);
        c2 = tolower(c2);
        if (c1 != c2) {
            return (int)c1 - (int)c2;
        }
    } while (c1 != '\0');

    return 0;
}