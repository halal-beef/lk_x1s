/*
 * Copyright (c) 2024 Igor Belwon <igor.belwon@mentallysanemainliners.org>
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 *
 */

#include <part.h>
#include <lk3rd/persistent_storage.h>

#define PERSISTENT_STORAGE_OFFSET 0x175000
#define PERSISTENT_STORAGE_SIZE 0x2000
#define PERSISTENT_STORAGE_ENTRIES sizeof(persistent_storage_entries)
#define TEMP_BUFFER_OFFSET 0x94000000

int persistent_storage_read(int entry)
{
	void *part;
	int *buffer = (int *)TEMP_BUFFER_OFFSET;

	part = part_get("lk3rd");

	part_read_partial(part, (void*)TEMP_BUFFER_OFFSET,
				PERSISTENT_STORAGE_OFFSET,
				PERSISTENT_STORAGE_SIZE);

	return buffer[entry];
}

int persistent_storage_write(int entry, int value)
{
	void *part;
	int *buffer = (int *)TEMP_BUFFER_OFFSET;

	part = part_get("lk3rd");

	part_read_partial(part, (void*)TEMP_BUFFER_OFFSET,
				PERSISTENT_STORAGE_OFFSET,
				PERSISTENT_STORAGE_SIZE);

	buffer[entry] = value;

	part_write_partial(part, (void*)TEMP_BUFFER_OFFSET,
				 PERSISTENT_STORAGE_OFFSET,
				 PERSISTENT_STORAGE_SIZE);

	printf("Wrote %i to %x\n", value, *buffer);

	return 0;
}
