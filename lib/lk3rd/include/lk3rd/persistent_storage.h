/*
 * Copyright (c) 2024 Igor Belwon <igor.belwon@mentallysanemainliners.org>
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 *
 */

#ifndef LK3RD_PERSISTENTSTORAGE_H
#define LK3RD_PERSISTENTSTORAGE_H

int persistent_storage_read(int entry);
int persistent_storage_write(int entry, int value);

enum persistent_storage_entries {
        LK3RD_MAINLINE_QUIRKS = 0x0,
};

#endif // LK3RD_PERSISTENTSTORAGE_H