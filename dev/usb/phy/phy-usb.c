/*
 * phy-usb.c
 *
 *  Created on: 2019. 3. 21.
 *      Author: sunghyun.na
 */

#include <debug.h>
#include <trace.h>
#include <list.h>
#include <lk/init.h>

#include "dev/phy-usb.h"

struct phy_infor {
	struct list_node phy_list_head;
};

static struct phy_infor phy_handle;

int phy_usb_init(void)
{
	struct list_node *head;
	struct phy_usb_ops *pos;

	head = &phy_handle.phy_list_head;
	if (head->next == NULL)
		return -1;
	if (list_is_empty(head))
		return 0;
	list_for_every_entry(head, pos, struct phy_usb_ops, entry) {
		if (!pos->initate)
			continue;
		if (!pos->handle && pos->get_handle)
			pos->handle = pos->get_handle(pos->id);
		pos->initate(pos->handle);
	}
	return 0;
}

int phy_usb_terminate(void)
{
	struct list_node *head;
	struct phy_usb_ops *pos;

	head = &phy_handle.phy_list_head;
	if (head->next == NULL)
		return -1;
	if (list_is_empty(head))
		return 0;

	list_for_every_entry(head, pos, struct phy_usb_ops, entry) {
		if (!pos->terminate)
			continue;
		pos->terminate(pos->handle);
	}
	return 0;
}

void phy_usb_register_ops(struct phy_usb_ops *ops)
{
	if (phy_handle.phy_list_head.next == NULL)
		list_initialize(&phy_handle.phy_list_head);

	list_add_tail(&phy_handle.phy_list_head, &ops->entry);
}
