/*
 * phy-exynos-usb.c
 *
 *  Created on: 2019. 3. 26.
 *      Author: sunghyun.na
 */

#include <list.h>
#include <lk/init.h>
#include <debug.h>
#include <trace.h>
#include <lib/heap.h>

#include "dev/phy-usb.h"
#include "dev/usb/phy-samsung-usb-cal.h"
#include "phy-exynos-usb3p1.h"

#define LOCAL_TRACE 0

struct phy_usb_exynos_list {
	struct exynos_usbphy_info *cal_info;
	int register_num;

	struct list_node entry;
};

struct phy_usb_samsung_infor {
	int register_phy_cnt;

	struct list_node cal_infor_head;
};

static struct phy_usb_samsung_infor phy_infor;

__attribute__((weak)) void phy_usb_exynos_system_init(int num_phy_port, bool en){return;}

int phy_usb_exynos_initiate(void *handle)
{
	struct list_node *head;
	struct phy_usb_exynos_list *pos, *tmp;
	struct exynos_usbphy_info *cal_info;
#if 0
	unsigned int maj_verion, minor_version1, minor_version2;
#endif
	head = &phy_infor.cal_infor_head;
	if (list_is_empty(head)) {
		LTRACEF("Not found Any PHY Cal HEAD\n");
		return -1;
	}
	list_for_every_entry_safe (head, pos, tmp, struct phy_usb_exynos_list, entry) {
		cal_info = pos->cal_info;
#if 0
		maj_verion = cal_info->version & EXYNOS_USBCON_VER_MAJOR_VER_MASK;
		minor_version1 = cal_info->version & EXYNOS_USBCON_VER_MINOR1_VER_MASK;
		minor_version2 = cal_info->version & EXYNOS_USBCON_VER_MINOR2_VER_MASK;

		if (maj_verion <= EXYNOS_USBCON_VER_01_MAX) {

		} else if (maj_verion > EXYNOS_USBCON_VER_01_MAX) {

		} else if (maj_verion > EXYNOS_USBCON_VER_02_MAX) {

		} else if (maj_verion > EXYNOS_USBCON_VER_03_MAX) {
			phy_exynos_usb_v3p1_enable(pos);
		} else if (maj_verion > EXYNOS_USBCON_VER_04_MAX) {

		} else if (maj_verion > EXYNOS_USBCON_VER_05_MAX) {

		} else {

		}
#else
		phy_usb_exynos_system_init(pos->register_num, true);
		phy_exynos_usb_v3p1_enable(cal_info);
		phy_exynos_usb_v3p1_pipe_ovrd(cal_info);

		phy_exynos_usb_v3p1_tune(cal_info);
#endif
	}
	return 0;
}

int phy_usb_exynos_terminate(void *handle)
{
	struct list_node *head;
	struct phy_usb_exynos_list *pos, *tmp;
	struct exynos_usbphy_info *cal_info;

	head = &phy_infor.cal_infor_head;
	if (list_is_empty(head)) {
		LTRACEF("Not found Any PHY Cal HEAD\n");
		return -1;
	}
	list_for_every_entry_safe_reverse(head, pos, tmp, struct phy_usb_exynos_list, entry) {
		cal_info = pos->cal_info;
#if 0
		maj_verion = pos->version & EXYNOS_USBCON_VER_MAJOR_VER_MASK;
		minor_version1 = pos->version & EXYNOS_USBCON_VER_MINOR1_VER_MASK;
		minor_version2 = pos->version & EXYNOS_USBCON_VER_MINOR2_VER_MASK;

		if (maj_verion <= EXYNOS_USBCON_VER_01_MAX) {

		} else if (maj_verion > EXYNOS_USBCON_VER_01_MAX) {

		} else if (maj_verion > EXYNOS_USBCON_VER_02_MAX) {

		} else if (maj_verion > EXYNOS_USBCON_VER_03_MAX) {
			phy_exynos_usb_v3p1_enable(pos);
		} else if (maj_verion > EXYNOS_USBCON_VER_04_MAX) {

		} else if (maj_verion > EXYNOS_USBCON_VER_05_MAX) {

		} else {

		}
#else
		phy_exynos_usb_v3p1_disable(cal_info);
		phy_usb_exynos_system_init(pos->register_num, false);
#endif
	}

	return 0;
}

void phy_usb_exynos_register_cal_infor(struct exynos_usbphy_info *cal_info)
{
	struct phy_usb_exynos_list *node;

	if (phy_infor.cal_infor_head.next == NULL)
		list_initialize(&phy_infor.cal_infor_head);
	node = malloc(sizeof(struct phy_usb_exynos_list));
	node->cal_info = cal_info;
	node->register_num =  phy_infor.register_phy_cnt++;
	list_add_tail(&phy_infor.cal_infor_head, &node->entry);
}

/* Static Functions */
static struct phy_usb_ops phy_usb_exynos_ops = {
	.type = phy_usb2_utmi | phy_usb3_pipe3,
	.initate = phy_usb_exynos_initiate,
	.terminate = phy_usb_exynos_terminate,
};

static void phy_usb_exynos_dev_init(uint level)
{
	list_initialize(&phy_infor.cal_infor_head);
}

static void phy_usb_exynos_probe(uint level)
{
	phy_usb_register_ops(&phy_usb_exynos_ops);
}

LK_INIT_HOOK(phy_usb_exynos_dev_init, &phy_usb_exynos_dev_init, LK_INIT_LEVEL_PLATFORM_EARLY);
LK_INIT_HOOK(phy_usb_exynos_probe, &phy_usb_exynos_probe, LK_INIT_LEVEL_KERNEL);

