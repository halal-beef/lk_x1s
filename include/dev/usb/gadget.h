/*
 * (C) Copyright 2019 SAMSUNG Electronics
 * Sung-Hyun Na <sunghyun.na@samsung.com>
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted
 * transcribed, stored in a retrieval system or translated into any human or computer language in an
 * form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 *
 */

#ifndef _USB_GADGET_H_
#define _USB_GADGET_H_

#include <list.h>
#include <stdbool.h>
#include "usb-def.h"

#define USB_STDREQ_OK  0
#define	USB_STDREQ_NOT_MINE 1
#define	USB_STDREQ_ERROR -1

enum gadget_buf_opt {
	GADGET_BUF_NO_OPTION = 0,

	/* Last option */
	GADGET_BUF_LAST = (0xC << 16),

	/* for Circular Ring */
	GADGET_BUF_RING_BACK = (0x10 << 16),

	GADGET_BUF_LAST_RING = (0x1C << 16),

	/* Cache Operation */
	GADGET_BUF_NON_CACHED = (0x2 << 16),
};

enum usb_dev_state {
	USB_DEV_STATE_OFF = -1,
	USB_DEV_STATE_DEFAULT = 0,
	USB_DEV_STATE_CONNECT,
	USB_DEV_STATE_ADDRESSED,
	USB_DEV_STATE_CONFIGURED,
};

struct usb_dev_infor {
	void *hs_desc_addr;
	int hs_desc_sz;

	void *fs_desc_addr;
	int fs_desc_sz;

	void *ss_desc_addr;
	int ss_desc_sz;

	int start_if_num;
	int num_if;

	void *class_handle;
	bool ctrl_xfer_owne;
	int (*cb_stdreq)(void *class_handle, USB_STD_REQUEST *std_req, void **data_stage_buf_addr, unsigned int *data_stage_buf_sz);
	void (*cb_stdreq_done)(void *class_handle, USB_STD_REQUEST *std_req);
	void (*config)(void *class_handle, USB_SPEED speed, unsigned int if_num, unsigned int set_num);

	struct list_node entry;
};

struct gadget_dev_ops {
	/* Device APIs */
	void *(*get_dev_handle)(void);
	int (*dev_start)(void *dev_handle);
	void (*dev_stop)(void *dev_handle);
	bool (*re_connect)(void *dev_handle);
	void (*dev_handle_isr)(void *dev_handle);
	void (*set_test_mode)(void *dev_handle, USB_TEST);

	/* EP Transfer APIs */
	int (*ep_req_cfg)(void *dev_handle, USB_ENDPOINT_DESCRIPTOR *ep_desc, USB_EPCOMP_DESCRIPTOR *ep_comp_desc);
	void (*ep_xfer_set_buf)(void *dev_handle, unsigned char ep_id, void *buf_addr, unsigned int xfer_sz, unsigned int option);
	void (*ep_set_cb_xferdone)(void *dev_handle, unsigned char ep_id, void (*cb)(void *), void *cb_arg);
	bool (*ep_xfer_start)(void *dev_handle, unsigned char ep_id);
	void (*ep_xfer_stop)(void *dev_handle, unsigned char ep_id);
	bool (*ep_xfer_is_done)(void *dev_handle, unsigned char ep_id);
	int (*ep_xfer_wait_done)(void *dev_handle, unsigned char ep_id, int timeout);
	unsigned int (*ep_get_rx_sz)(void *dev_handle, unsigned char ep_id);
};


/* APIs for class */
int gadget_ep_req_cfg(USB_ENDPOINT_DESCRIPTOR *ep_desc, USB_EPCOMP_DESCRIPTOR *ep_comp_desc);
void gadget_ep_set_buf(u8 ep_id, void *buf_addr, u32 xfer_sz, u32 option);
void gadget_ep_start(u8 ep_id);
void gadget_ep_set_cb_xferdone(u8 ep_id, void (*ep_cb)(void *), void *cb_arg);
void gadget_ep_is_xfer_done(u8 ep_id);
int gadget_ep_wait_xfer_done(u8 ep_id, int timeout);
int gadget_ep_get_rx_sz(unsigned char ep_id, unsigned int *rx_sz);
int get_ep_id(USB_DIR direction);
unsigned char get_intf_num(int req_if_cnt);
void register_desc_buf(struct usb_dev_infor *infor);
int get_str_id(const char *desc_addr, int sz);
/* functions for device driver */
int gadget_call_cb_class_std_req(USB_STD_REQUEST *std_req, void **data_stage_buf_addr, unsigned int *data_stage_len);
void gadget_call_cb_class_std_done(USB_STD_REQUEST *std_req);
int get_str_desc_buf_addr(int str_id, void **str_buf_ptr, unsigned int *data_stage_len);
int make_dev_desc(USB_SPEED speed, USB_DEVICE_DESCRIPTOR *dev_desc);
int make_bos_desc(USB_SPEED speed, void *bos_desc_addr, int sz);
int make_configdesc(USB_SPEED speed, void *desc_buf_addr, unsigned int desc_sz);
void notify_config_to_class(USB_SPEED speed, int if_num, int set_num);
void probe_dev_api_ops(struct gadget_dev_ops *ops);
void gadget_chg_state(enum usb_dev_state state);
enum usb_dev_state gadget_get_state(void);
void gadget_notify_disconnect(void);
/* APIs for application */
int start_usb_gadget(void);
void stop_usb_gadget(void);
void gadgeg_dev_polling_handle(void);

#endif /* INCLUDE_DEV_USB_GADGET_H_ */
