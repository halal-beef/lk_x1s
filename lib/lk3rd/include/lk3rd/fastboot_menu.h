/*
 * Copyright (c) 2024 Igor Belwon <igor.belwon@mentallysanemainliners.org>
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 *
 */

#ifndef FASTBOOT_MENU
#define FASTBOOT_MENU

enum action {
	ACTION_START,
	ACTION_REBOOT_RECOVERY,
	ACTION_REBOOT_BOOTLOADER,
	ACTION_REBOOT_FASTBOOTD,
	ACTION_REBOOT_DOWNLOAD,
	ACTION_POWEROFF = 5,
	ACTION_END,
};

int fastboot_menu_entry(void*);
void notify_action_switch(int modifier);

#endif /* FASTBOOT_MENU */
