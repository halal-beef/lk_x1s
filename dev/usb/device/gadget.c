/*
 * (C) Copyright 2019 SAMSUNG Electronics
 * Author: sunghyun.na@samsung.com (Sung-Hyun Na)
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted
 * transcribed, stored in a retrieval system or translated into any human or computer language in an
 * form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 *
 */
#include <lk/debug.h>
#include <lk/trace.h>
#include <string.h>
#include <malloc.h>
#include <lk/init.h>
#include <lk/list.h>
#include <usb-def.h>
#include <kernel/thread.h>
#include <lk3rd/fastboot_menu.h>

#include "dev/usb/gadget.h"

#define LOCAL_TRACE 0

struct usb_dev_string_desc {
	void *desc_buf;
	int sz;
	int str_id;
	struct list_node entry;
};

struct _udev_gadget {
	u32 out_ep_alloc_bm;
	u32 in_ep_alloc_bm;
	unsigned int intf_num;

	struct list_node dev_infor_list_head;

	unsigned int total_hs_desc_sz;
	unsigned int total_ss_desc_sz;

	struct list_node string_desc_list_head;
	int sting_cnt;

	unsigned short platform_vid;
	unsigned short platform_pid;
	unsigned short platform_bcd_dev_verion;
	unsigned char vendor_str_num;
	unsigned char product_str_num;
	unsigned char serial_str_num;

	void *dev_handle;
	struct gadget_dev_ops *dev_ops;

	enum usb_dev_state state;
};

static struct _udev_gadget udev_gadget;

/* APIs for class */
int gadget_ep_req_cfg(USB_ENDPOINT_DESCRIPTOR *ep_desc, USB_EPCOMP_DESCRIPTOR *ep_comp_desc)
{
	LTRACE_ENTRY;

	if (!udev_gadget.dev_ops)
		return -1;
	if (!udev_gadget.dev_ops->ep_req_cfg)
		return -1;

	LTRACE_EXIT;

	return udev_gadget.dev_ops->ep_req_cfg(udev_gadget.dev_handle, ep_desc, ep_comp_desc);
}
void gadget_ep_set_buf(unsigned char ep_id, void *buf_addr, unsigned int xfer_sz, unsigned int option)
{
	if (!udev_gadget.dev_ops)
		return;
	if (!udev_gadget.dev_ops->ep_xfer_set_buf)
		return;

	udev_gadget.dev_ops->ep_xfer_set_buf(udev_gadget.dev_handle, ep_id, buf_addr, xfer_sz, option);
}

void gadget_ep_start(unsigned char ep_id)
{
	if (!udev_gadget.dev_ops)
		return;
	if (!udev_gadget.dev_ops->ep_xfer_start)
		return;
	udev_gadget.dev_ops->ep_xfer_start(udev_gadget.dev_handle, ep_id);
}

void gadget_ep_set_cb_xferdone(unsigned char ep_id, void (*ep_cb)(void *), void *cb_arg)
{
	if (!udev_gadget.dev_ops)
		return;
	if (!udev_gadget.dev_ops->ep_set_cb_xferdone)
		return;
	udev_gadget.dev_ops->ep_set_cb_xferdone(udev_gadget.dev_handle, ep_id, ep_cb, cb_arg);
}

void gadget_ep_is_xfer_done(unsigned char ep_id)
{
	if (!udev_gadget.dev_ops)
		return;
	if (!udev_gadget.dev_ops->ep_set_cb_xferdone)
		return;
	udev_gadget.dev_ops->ep_xfer_is_done(udev_gadget.dev_handle, ep_id);
}

int gadget_ep_wait_xfer_done(unsigned char ep_id, int timeout)
{
	if (!udev_gadget.dev_ops)
		return -1;
	if (!udev_gadget.dev_ops->ep_xfer_wait_done)
		return -1;
	return udev_gadget.dev_ops->ep_xfer_wait_done(udev_gadget.dev_handle, ep_id, timeout);
}

int gadget_ep_get_rx_sz(unsigned char ep_id, unsigned int *rx_sz)
{
	if (!udev_gadget.dev_ops)
		return -1;
	if (!udev_gadget.dev_ops->ep_get_rx_sz)
		return -1;
	if (!rx_sz)
		return -1;
	*rx_sz = udev_gadget.dev_ops->ep_get_rx_sz(udev_gadget.dev_handle, ep_id);

	return 0;
}

int get_ep_id(USB_DIR direction)
{
	int cnt;
	u32 *ep_map_bm = NULL;

	ep_map_bm = (direction == USBDIR_OUT) ? &udev_gadget.out_ep_alloc_bm : &udev_gadget.in_ep_alloc_bm;
	for (cnt = 0; cnt < 16; cnt++) {
		if (((1 << cnt) & *ep_map_bm) == 0x0) {
			*ep_map_bm |= (1 << cnt);
			if (direction == USBDIR_OUT)
				return (cnt + 1);
			else
				return (cnt + 1) | 0x80;
		}
	}
	return -1;
}

unsigned char get_intf_num(int req_if_cnt)
{
	unsigned int ret;

	ret = udev_gadget.intf_num;
	udev_gadget.intf_num = udev_gadget.intf_num + req_if_cnt;
	return ret;
}

int get_str_id(const char *desc_addr, int sz)
{
	struct list_node *head;
	struct usb_dev_string_desc *tmp;
	char *target;
	int cnt;

	head = &udev_gadget.string_desc_list_head;
	if (head->next == NULL) {
		list_initialize(head);
		/* */

	}
	tmp = calloc(1, sz * 2 + 2 + sizeof(struct usb_dev_string_desc));
	tmp->desc_buf = (void *) ((unsigned long long) tmp + sizeof(struct usb_dev_string_desc));
	tmp->sz = sz * 2 + 2;
	tmp->str_id = udev_gadget.sting_cnt + 1;
	udev_gadget.sting_cnt++;
	target = tmp->desc_buf;
	target[0] = sz * 2 + 2;
	target[1] = STRING_DESCRIPTOR;
	target = tmp->desc_buf + 2;
	for (cnt = 0; cnt < sz; cnt++) {
		target[2*cnt] = desc_addr[cnt];
		target[2*cnt + 1] = 0;
	}
	list_add_tail(head, &tmp->entry);
	return tmp->str_id;
}

void register_desc_buf(struct usb_dev_infor *infor)
{
	if (udev_gadget.dev_infor_list_head.next == NULL)
		list_initialize(&udev_gadget.dev_infor_list_head);

	list_add_tail(&udev_gadget.dev_infor_list_head, &infor->entry);
}

/* functions for device driver */
int gadget_call_cb_class_std_req(USB_STD_REQUEST *std_req, void **data_stage_buf_addr, unsigned int *data_stage_len)
{
	struct list_node *head;
	struct usb_dev_infor *pos, *tmp;
	int ret;

	/* Check list is empty */
	head = &udev_gadget.dev_infor_list_head;
	if (list_is_empty(head))
		return -1;
	ret = USB_STDREQ_ERROR;
	list_for_every_entry_safe(head, pos, tmp, struct usb_dev_infor, entry) {
		if (!pos->cb_stdreq)
			continue;
		ret = pos->cb_stdreq(pos->class_handle, std_req, data_stage_buf_addr, data_stage_len);
		if (ret != USB_STDREQ_NOT_MINE) {
			if (ret == USB_STDREQ_OK) {
				/* Clear control transfer owne mark */
				pos->ctrl_xfer_owne = 1;
			}
			break;
		}
	}
	return ret;
}

void gadget_call_cb_class_std_done(USB_STD_REQUEST *std_req)
{
	struct list_node *head;
	struct usb_dev_infor *pos, *tmp;

	LTRACE_ENTRY;

	/* Check list is empty */
	head = &udev_gadget.dev_infor_list_head;
	if (list_is_empty(head))
		return;
	list_for_every_entry_safe(head, pos, tmp, struct usb_dev_infor, entry) {
		if (pos->ctrl_xfer_owne == 0)
			continue;
		/* Clear control transfer owne mark */
		pos->ctrl_xfer_owne = 0;
		if (!pos->cb_stdreq_done)
			continue;
		pos->cb_stdreq_done(pos->class_handle, std_req);
	}
}

int get_str_desc_buf_addr(int str_id, void **str_buf_ptr, unsigned int *data_stage_len)
{
	int ret = -1;
	struct usb_dev_string_desc *pos;
	struct list_node *head;

	head = &udev_gadget.string_desc_list_head;

	if (head->next == NULL)
		return ret;
	if (list_is_empty(head))
		return ret;
	list_for_every_entry(head, pos, struct usb_dev_string_desc, entry) {
		if (pos->str_id == str_id) {
			*str_buf_ptr = pos->desc_buf;
			*data_stage_len = pos->sz;
			ret = 0;
			break;
		}
	}
	if ((ret == -1) && (str_id == 0)) {
		struct usb_dev_string_desc *tmp;
		char *target;

		tmp = calloc(1, 4 + sizeof(struct usb_dev_string_desc));
		tmp->desc_buf = (void *) ((unsigned long long) tmp + sizeof(struct usb_dev_string_desc));
		tmp->sz = 4;
		tmp->str_id = 0;
		target = tmp->desc_buf;
		target[0] = 4;
		target[1] = STRING_DESCRIPTOR;
		target[2] = LO_BYTE(LANGID_US);
		target[3] = HI_BYTE(LANGID_US);
		list_add_tail(head, &tmp->entry);
		*str_buf_ptr = tmp->desc_buf;
		*data_stage_len = tmp->sz;
		ret = 0;
	}
	return ret;
}

int make_dev_desc(USB_SPEED speed, USB_DEVICE_DESCRIPTOR *dev_desc)
{
	if (!dev_desc)
		return -1;

	/* Device Descriptor */
	dev_desc->bLength = USB_DESC_SIZE_DEVICE;
	dev_desc->bDescriptorType = DEVICE_DESCRIPTOR;
	dev_desc->bDeviceClass = UC_DEVICE;	// class defined in interface des
	dev_desc->bDeviceSubClass = UC_DEVICE;
	dev_desc->bDeviceProtocol = 0x0;
	if (speed >= USBSPEED_SUPER) {
		dev_desc->bcdUSB = 0x0310;	// Ver 3.00
		dev_desc->bMaxPacketSize0 = 9;	// 2^9 = 512
	} else {
		dev_desc->bcdUSB = 0x0210;	// Ver 2.10
		dev_desc->bMaxPacketSize0 = 64;
	}
	dev_desc->idVendor = udev_gadget.platform_vid;
	dev_desc->idProduct = udev_gadget.platform_pid;
	dev_desc->bcdDevice = udev_gadget.platform_bcd_dev_verion;

	dev_desc->iManufacturer = udev_gadget.vendor_str_num;
	dev_desc->iProduct = udev_gadget.product_str_num;
	dev_desc->iSerialNumber = udev_gadget.serial_str_num;
	/* Current only 1 configuration support */
	dev_desc->bNumConfigurations = 1;

	return 0;
}

int make_bos_desc(USB_SPEED speed, void *bos_desc_addr, int sz)
{
	USB_BOS_DESCRIPTOR *bos_desc;
	USB_CAP_20EXT_DESCRIPTOR *p_oUsb20ExtCap;
	u16 bos_total_sz;
	u8 num_dev_cap;
#if 0
	USB_CAP_SUPERSPEED_DESCRIPTOR *p_oSuperCap;
	USB_CAP_SUPERSPEEDPLUS_DESCRIPTOR *p_oSuperSpeedPlusCap;
#endif
	bos_desc = bos_desc_addr;
	num_dev_cap = 0;
	bos_total_sz = 5;

	/* BOS(Binary Object Store) on Device Desc */
	bos_desc->bLength = USB_DESC_SIZE_BOS;
	bos_desc->bDescriptorType = BOS_DESC;

	/* Bos Descriptor */
	/* USB 2.0 Extension descriptor */
	p_oUsb20ExtCap = (USB_CAP_20EXT_DESCRIPTOR *)((addr_t) bos_desc + USB_DESC_SIZE_BOS);
	p_oUsb20ExtCap->bLength = USB_DESC_SIZE_CAP_20EXT;
	p_oUsb20ExtCap->bDescriptorType = DEVICE_CAPABILITY_DESC;
	p_oUsb20ExtCap->bDevCapabilityType = USB_CAP_20_EXT;
	p_oUsb20ExtCap->bmAttributes.b.LPM = 1;
	p_oUsb20ExtCap->bmAttributes.b.BESL_Support = 1;
	p_oUsb20ExtCap->bmAttributes.b.BESL_BASELINE_VALID = 1;
	p_oUsb20ExtCap->bmAttributes.b.BESL_BASELINE_VALUE = 0x8;
	p_oUsb20ExtCap->bmAttributes.b.BESL_DEEP_VALID = 0;
	p_oUsb20ExtCap->bmAttributes.b.BESL_DEEP_VALUE = 0;
	bos_total_sz += USB_DESC_SIZE_CAP_20EXT;
	num_dev_cap++;

// SuperSpeed USB Device Capability descriptor
#if 0
	p_oSuperCap = (USB_CAP_SUPERSPEED_DESCRIPTOR *)(bos_add_infor +
				  USB_DESC_SIZE_CAP_20EXT);
	p_oSuperCap->bLength = USB_DESC_SIZE_CAP_SUPERSPEED;
	p_oSuperCap->bDescriptorType = DEVICE_CAPABILITY_DESC;
	p_oSuperCap->bDevCapabilityType = USB_CAP_SS;
	if (p_oDevConfig->m_bLTMSupport)
		p_oSuperCap->bmAttributes = 0x2;
	else
		p_oSuperCap->bmAttributes = 0x0;
	p_oSuperCap->wSpeedsSupported = 0x000e;
	p_oSuperCap->bFunctionalitySupport = 2;
	/* TODO: Get proper vaule from HW */
	if (p_oDevConfig->m_bU1Support)
		p_oSuperCap->bU1DevExitLat = p_oDevConfig->m_ucU1ExitValue;
	else
		p_oSuperCap->bU1DevExitLat = 0;
	if (p_oDevConfig->m_bU1Support)
		p_oSuperCap->wU2DevExitLat = p_oDevConfig->m_usU2ExitValue;
	else
		p_oSuperCap->wU2DevExitLat = 0;
	bos_total_sz += USB_DESC_SIZE_CAP_SUPERSPEED;
	num_dev_cap++;

	/* SuperSpeedPlus USB Device Capability descriptor */
	p_oSuperSpeedPlusCap = (USB_CAP_SUPERSPEEDPLUS_DESCRIPTOR *)(bos_add_infor +
						   USB_DESC_SIZE_CAP_20EXT +
						   USB_DESC_SIZE_CAP_SUPERSPEED);
	p_oSuperSpeedPlusCap->bLength = USB_DESC_SIZE_CAP_SUPERSPEEDPLUS;
	p_oSuperSpeedPlusCap->bDescriptorType = DEVICE_CAPABILITY_DESC;
	p_oSuperSpeedPlusCap->bDevCapabilityType = USB_CAP_SSP;
	p_oSuperSpeedPlusCap->bReserved = 0x00;
	p_oSuperSpeedPlusCap->bmAttributes = 0x00000001;
	p_oSuperSpeedPlusCap->wFunctionalitySupport = 0x1100;
	p_oSuperSpeedPlusCap->wReserved = 0x0000;
	p_oSuperSpeedPlusCap->bmSublinkSpeedAttr[0] = 0x000A4030;
	p_oSuperSpeedPlusCap->bmSublinkSpeedAttr[1] = 0x000A40B0;
	bos_total_sz += USB_DESC_SIZE_CAP_SUPERSPEEDPLUS;
	num_dev_cap++;

	/* Container ID descriptor */
	if (p_oDevConfig->m_bContainID) {
		USB_CAP_CONTAINID_DESCRIPTOR *p_oContiainID;

		p_oContiainID = (USB_CAP_CONTAINID_DESCRIPTOR *)
						(bos_add_infor +
						 USB_DESC_SIZE_CAP_20EXT +
						 USB_DESC_SIZE_CAP_SUPERSPEED +
						 USB_DESC_SIZE_CAP_SUPERSPEEDPLUS);
		p_oContiainID->bLength = USB_DESC_SIZE_CAP_CONTAINID;
		p_oContiainID->bDescriptorType = DEVICE_CAPABILITY_DESC;
		p_oContiainID->bDevCapabilityType = USB_CAP_CID;
		p_oContiainID->bReserved = 0x0;
		bos_total_sz += USB_DESC_SIZE_CAP_CONTAINID;
		num_dev_cap++;
	}
#endif
	bos_desc->bNumDeviceCaps = num_dev_cap;
	bos_desc->wTotalLength = bos_total_sz;

	if (bos_total_sz > sz)
		return -1;
	else
		return bos_total_sz;
}
int make_configdesc(USB_SPEED speed, void *desc_buf_addr, unsigned int desc_sz)
{
	unsigned int desc_sz_offset;
	struct list_node *head;
	struct usb_dev_infor *pos, *tmp;
	void *buf_target;
	USB_CONFIGURATION_DESCRIPTOR *cfg_desc;

	/* Check list is empty */
	head = &udev_gadget.dev_infor_list_head;

	cfg_desc = desc_buf_addr;
	cfg_desc->bLength = USB_DESC_SIZE_CONFIG;
	cfg_desc->bDescriptorType = CONFIGURATION_DESCRIPTOR;
	cfg_desc->bNumInterfaces = udev_gadget.intf_num;
	cfg_desc->bConfigurationValue = 1;
	cfg_desc->iConfiguration = 0;
	cfg_desc->bmAttributes = CONF_ATTR_DEFAULT | CONF_ATTR_SELFPOWERED;
	cfg_desc->wTotalLength = USB_DESC_SIZE_CONFIG;
	cfg_desc->maxPower = 50;

	desc_sz_offset = USB_DESC_SIZE_CONFIG;
	if (list_is_empty(head))
		return desc_sz_offset;
	buf_target = (void *) ((addr_t) desc_buf_addr + USB_DESC_SIZE_CONFIG);
	list_for_every_entry_safe(head, pos, tmp, struct usb_dev_infor, entry) {
		if (desc_sz_offset > desc_sz) {
			return -1;
		}
		if (speed >= USBSPEED_SUPER) {
			memcpy(buf_target, pos->ss_desc_addr, pos->ss_desc_sz);
			buf_target = (void *) ((addr_t) buf_target + pos->ss_desc_sz);
			desc_sz_offset += pos->ss_desc_sz;
		} else {
			memcpy(buf_target, pos->hs_desc_addr, pos->hs_desc_sz);
			buf_target = (void *) ((addr_t) buf_target + pos->hs_desc_sz);
			desc_sz_offset += pos->hs_desc_sz;
		}
	}
	cfg_desc->wTotalLength = desc_sz_offset;
	return desc_sz_offset;
}

void notify_config_to_class(USB_SPEED speed, int if_num, int set_num)
{
	struct list_node *head;
	struct usb_dev_infor *pos, *tmp;

	/* Check list is empty */
	head = &udev_gadget.dev_infor_list_head;
	if (list_is_empty(head))
		return;
	list_for_every_entry_safe(head, pos, tmp, struct usb_dev_infor, entry) {
		if (!pos->config)
			continue;
		if (if_num == -1) {
			int if_cnt;

			for (if_cnt = 0; if_cnt < pos->num_if; if_cnt++)
				pos->config(pos->class_handle, speed, pos->start_if_num + if_cnt, set_num);
		} else {
			if ((pos->start_if_num > if_num) ||
					 ((pos->start_if_num + pos->num_if - 1) < if_num)) {
				continue;
			}
			pos->config(pos->class_handle, speed, if_num, set_num);
		}
	}
}

void probe_dev_api_ops(struct gadget_dev_ops *ops)
{
	udev_gadget.dev_ops = ops;
}

void gadget_chg_state(enum usb_dev_state state)
{
	udev_gadget.state = state;
}

enum usb_dev_state gadget_get_state(void)
{
	return udev_gadget.state;
}

void gadget_notify_disconnect(void)
{
	udev_gadget.state = USB_DEV_STATE_DEFAULT;
	/* TODO: Call disconnect callback to function layer */
}

__attribute__((weak)) void target_init_for_usb(void){}
__attribute__((weak)) void target_terminate_for_usb(void){}

int create_fastboot_menu_thread(void)
{
	thread_t *fastboot_menu = thread_create(
		"fastboot_menu",
		&fastboot_menu_entry,
		NULL,
		HIGH_PRIORITY,
		DEFAULT_STACK_SIZE
	);

	thread_resume(fastboot_menu);

	return 0;
}

/* APIs for application */
int start_usb_gadget(void)
{
	struct gadget_dev_ops *dev_ops;

	target_init_for_usb();

	if (udev_gadget.dev_ops == NULL)
		return -1;
	dev_ops = udev_gadget.dev_ops;
	if (udev_gadget.dev_handle == NULL) {
		if (dev_ops->get_dev_handle == NULL)
			return -1;

		udev_gadget.dev_handle = dev_ops->get_dev_handle();
	}
	if (dev_ops->dev_start == NULL)
		return -1;

	if (udev_gadget.state >= USB_DEV_STATE_DEFAULT)
		return 0;

	udev_gadget.state = USB_DEV_STATE_DEFAULT;

	create_fastboot_menu_thread();

	return dev_ops->dev_start(udev_gadget.dev_handle);
}

void stop_usb_gadget(void)
{
	struct gadget_dev_ops *dev_ops;

	if (udev_gadget.dev_ops == NULL)
		return;
	if (udev_gadget.state < USB_DEV_STATE_DEFAULT)
		return;
	dev_ops = udev_gadget.dev_ops;
	dev_ops->dev_stop(udev_gadget.dev_handle);

	udev_gadget.state = USB_DEV_STATE_OFF;

	target_terminate_for_usb();
}

void gadgeg_dev_polling_handle(void)
{
	struct gadget_dev_ops *dev_ops;

	if (udev_gadget.dev_ops == NULL)
		return;
	dev_ops = udev_gadget.dev_ops;
	if (!dev_ops->dev_handle_isr)
		return;
	/* TODO: Check non interrupt mode.
	 *  If Interrupt is used, not working this handle ISR
	 */
	dev_ops->dev_handle_isr(udev_gadget.dev_handle);
}

__attribute__((weak)) void gadget_probe_pid_vid_version(unsigned short *vid, unsigned short *pid, unsigned short *bcd_version){}
__attribute__((weak)) int gadget_get_vendor_string(void){return 0;}
__attribute__((weak)) int gadget_get_product_string(void){return 0;}
__attribute__((weak)) int gadget_get_serial_string(void){return 0;}

/* Static Functions */
static void gadget_init(uint level)
{
	if (udev_gadget.dev_infor_list_head.next == NULL)
		list_initialize(&udev_gadget.dev_infor_list_head);
	if (udev_gadget.string_desc_list_head.next == NULL)
		list_initialize(&udev_gadget.string_desc_list_head);
	/* Get VID, PID and Device Version from platform or target */
	gadget_probe_pid_vid_version(&udev_gadget.platform_vid, &udev_gadget.platform_pid, &udev_gadget.platform_bcd_dev_verion);
	/* Get Vendor(Manufacture) String */
	udev_gadget.vendor_str_num = gadget_get_vendor_string();
	/* Get Product String */
	udev_gadget.product_str_num = gadget_get_product_string();
	/* Get Serial String */
	udev_gadget.serial_str_num = gadget_get_serial_string();

	udev_gadget.state = USB_DEV_STATE_OFF;
}
LK_INIT_HOOK(usb_dev_gadget, &gadget_init, LK_INIT_LEVEL_KERNEL);
