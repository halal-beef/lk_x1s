#ifndef FASTBOOT_MENU
#define FASTBOOT_MENU

enum action {
	ACTION_START,
	ACTION_REBOOTRECOVERY,
	ACTION_REBOOTBOOTLOADER,
	ACTION_POWEROFF = 3
};

int fastboot_menu_entry(void*);

#endif /* FASTBOOT_MENU */
