/*
 * phy-usb.h
 *
 *  Created on: 2019. 3. 22.
 *      Author: sunghyun.na
 */

#ifndef _PHY_USB_H_
#define _PHY_USB_H_

#include <list.h>

enum usb_phy_type {
	phy_usb2_utmi = 0x11,
	phy_usb2_ulpi = 0x12,
	phy_usb3_pipe3 = 0x2100,
};

struct phy_usb_ops {
	enum usb_phy_type type;
	int id;
	int port_id;

	void *(*get_handle)(int id);
	int (*initate)(void *handle);
	int (*terminate)(void *handle);
	void (*set_dp_pull_up)(void *handle, bool dp_pullup);

	void *handle;
	struct list_node entry;
};

int phy_usb_init(void);
int phy_usb_terminate(void);
void phy_usb_register_ops(struct phy_usb_ops *ops);

#endif /* _PHY_USB_H_ */
