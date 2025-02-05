/*
 * Copyright (c) 2024 Igor Belwon <igor.belwon@mentallysanemainliners.org>
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 *
 */

#include <stdbool.h>
#include <stdio.h>
#include <lk3rd/persistent_storage.h>
#include <lk3rd/mainline_quirks.h>

int lk3rd_switch_mainline_quirks(bool enable)
{
	int ret;

	persistent_storage_write(LK3RD_MAINLINE_QUIRKS, enable);

	ret = persistent_storage_read(LK3RD_MAINLINE_QUIRKS);

	if(ret != enable)
	{
		printf("lk3rd: couldn't write to persistent storage, e: %i g: %i\n",
		       enable, ret);
	}

	return ret;
}

int lk3rd_get_mainline_quirks()
{
	return persistent_storage_read(LK3RD_MAINLINE_QUIRKS);
}
