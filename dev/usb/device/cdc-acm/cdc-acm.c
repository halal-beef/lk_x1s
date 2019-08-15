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

#include <stdio.h>
#include <string.h>
#include <debug.h>
#include <trace.h>
#include <reg.h>
#include <list.h>
#include <malloc.h>
#include <lk/init.h>
#include <arch/spinlock.h>
#include <platform/delay.h>
#include <lib/cbuf.h>

#include "usb-def.h"
#include "dev/usb/gadget.h"
#include "cdc-acm-def.h"

#define LOCAL_TRACE 0

#define CDC_ACM_CTRL_NOTI_EP

static struct list_node desc_list = LIST_INITIAL_VALUE(desc_list);
static int console_port;
static print_callback_t cdc_acm_print_cb;

#define LOG_BUF_SZ	8192

volatile char log_buffer_temp[LOG_BUF_SZ];
volatile unsigned int index_writer;
volatile unsigned int index_reader;
char rx_data_buf[1024];
static unsigned char panic_mode;

struct cdc_data_queue {
	void *buf;
	int sz;

	struct list_node entry;
};

struct cdcacm_infor {
	unsigned bulk_in_ep;
	int bulk_in_ep_status;
	unsigned bulk_out_ep;
	int bulk_out_ep_status;
	unsigned intr_in_ep;
	int intr_in_ep_status;

	unsigned int cdc_if_num;
	unsigned int dic_if_num;

	unsigned int std_req_buf[256];

	int baud_rate;
	unsigned char stop_bit;
	unsigned char parity_bit;
	unsigned char data_bits;

	volatile int bulkin_done;
	volatile int bulkout_done;

	bool rts;
	bool dtr;

	unsigned int abstract_state;

	bool console_print_mark;

	event_t txdone_event;

	struct list_node tx_data_q_head;
};

static struct __attribute__((__packed__)) cdc_acm_config_desc_hs {
	USB_INTERFACE_ASSOCIATION_DESC if_assoc_desc;
	USB_INTERFACE_DESCRIPTOR ctrl_if_desc;
	CDC_CLASS_INTERFACE_DESC_HEADER header_desc;
	CDC_CLASS_INTERFACE_DESC_CM cm_desc;
	CDC_CLASS_INTERFACE_DESC_ACM acm_desc;
	CDC_CLASS_INTERFACE_DESC_UNION union_desc;
	USB_ENDPOINT_DESCRIPTOR intr_ep_desc;
	USB_INTERFACE_DESCRIPTOR data_if_desc;
	USB_ENDPOINT_DESCRIPTOR bulk_out_ep_desc;
	USB_ENDPOINT_DESCRIPTOR bulk_in_ep_desc;
} hs_config_desc = {
	.if_assoc_desc = {
		.bLength = USB_DESC_SIZE_IF_ASSOC,
		.bDescriptorType = INTERFACE_ASSOCIATION,
		.bInterfaceCount = 2,
		.bFunctionClass = UC_CDC,
		.bFunctionSubClass = CDC_ACM,
		.bFunctionProtocol = CDC_PROT_AT_CMD_COMMON,
	},
	.ctrl_if_desc = {
		.bLength = USB_DESC_SIZE_INTERFACE,
		.bDescriptorType = INTERFACE_DESCRIPTOR,
		.bAlternateSetting = 0x0,
		.bNumEndpoints = 1,
		.bClass = UC_CDC,
		.bSubClass = CDC_ACM,
		.bProtocol = CDC_PROT_AT_CMD_COMMON,
	},
	.header_desc = {
		.bLength = 0x5,
		.bDescType = CLASS_TYPE | INTERFACE_DESCRIPTOR,
		.bDescSubType = CDC_SUBDESC_HEADER,
		.bcdCDC = 0x0110,
	},
	.cm_desc = {
		.bLength = 0x5,
		.bDescType = CLASS_TYPE | INTERFACE_DESCRIPTOR,
		.bDescSubType = CDC_SUBDESC_CM,
		.bmCapabilities = 0x0,
		.bDataInterface = 0x1,
	},
	.acm_desc = {
		.bLength = 0x4,
		.bDescType = CLASS_TYPE | INTERFACE_DESCRIPTOR,
		.bDescSubType = CDC_SUBDESC_ACM,
		.bmCapabilities = 0x2,
	},
	.union_desc = {
		.bLength = 0x5,
		.bDescType = CLASS_TYPE | INTERFACE_DESCRIPTOR,
		.bDescSubType = CDC_SUBDESC_UNION,
		.bMasterInterface = 0x0,
		.bSlaveInterface = 0x1,
	},
	.intr_ep_desc = {
		.bLength = USB_DESC_SIZE_ENDPOINT,
		.bDescriptorType = ENDPOINT_DESCRIPTOR,
		.bEndpointAddress = EP_ADDR_IN,
		.bmAttributes = EP_ATTR_INTERRUPT,
		.wMaxPacketSize = 10,
		.bInterval = 8,
	},
	.data_if_desc = {
		.bLength = USB_DESC_SIZE_INTERFACE,
		.bDescriptorType = INTERFACE_DESCRIPTOR,
		.bAlternateSetting = 0x0,
		.bNumEndpoints = 2,
		.bClass = UC_CDD,
		.bSubClass = 0x0,
		.bProtocol = 0x0,
	},
	.bulk_in_ep_desc = {
		.bLength = USB_DESC_SIZE_ENDPOINT,
		.bDescriptorType = ENDPOINT_DESCRIPTOR,
		.bEndpointAddress = EP_ADDR_IN,
		.bmAttributes = EP_ATTR_BULK,
		.wMaxPacketSize = 512,
	},
	.bulk_out_ep_desc = {
		.bLength = USB_DESC_SIZE_ENDPOINT,
		.bDescriptorType = ENDPOINT_DESCRIPTOR,
		.bEndpointAddress = EP_ADDR_OUT,
		.bmAttributes = EP_ATTR_BULK,
		.wMaxPacketSize = 512,
	},
};

struct desc_list_node {
	void *hs_desc_ptr;
	void *ss_desc_ptr;
	void *fs_desc_ptr;

	struct list_node entry;
} desc_head = {
	.hs_desc_ptr = &hs_config_desc,
};

static void start_console_tx(struct cdcacm_infor *cdc_acm_h)
{
	int wr_idx, rd_idx, size;
	void *buf;
	volatile bool sts;

	wr_idx = index_writer;
	rd_idx = index_reader;
	sts = cdc_acm_h->bulkin_done;

	if (wr_idx == rd_idx)
		return;
	else if (wr_idx > rd_idx) {
		size = wr_idx - rd_idx;
		buf = (void *) ((addr_t) (log_buffer_temp) + rd_idx);
		rd_idx += size;
		if (!sts)
			gadget_ep_set_buf(cdc_acm_h->bulk_in_ep, buf, size, GADGET_BUF_LAST);
	} else {
		size = LOG_BUF_SZ - rd_idx;
		buf = (void *) ((addr_t) (log_buffer_temp) + rd_idx);
		if (!sts) {
			gadget_ep_set_buf(cdc_acm_h->bulk_in_ep, buf, size, (wr_idx != 0) ? 0 : GADGET_BUF_LAST);
		}
		rd_idx = wr_idx;
		if (!sts && rd_idx != 0)
			gadget_ep_set_buf(cdc_acm_h->bulk_in_ep, (void *) log_buffer_temp, rd_idx, GADGET_BUF_LAST);
	}
	index_reader = rd_idx;

	if (!sts) {
		if (panic_mode)
			mdelay(1);
		cdc_acm_h->bulkin_done = 1;
		gadget_ep_start(cdc_acm_h->bulk_in_ep);
	}
}

static void start_console_rx(struct cdcacm_infor *acm_h)
{
	acm_h->bulkout_done = 1;
	gadget_ep_set_buf(acm_h->bulk_out_ep, rx_data_buf, 1024, GADGET_BUF_LAST);
	gadget_ep_start(acm_h->bulk_out_ep);
}

static void callback_bulk_in(void *handle)
{
	struct cdcacm_infor *acm_h;
	acm_h = handle;

	acm_h->bulkin_done = 0;

	if (acm_h->console_print_mark)
		start_console_tx(acm_h);
}

static void callback_bulk_out(void *handle)
{
	struct cdcacm_infor *acm_h;
	unsigned int rx_sz = 0;
	int ret;

	acm_h = handle;
	acm_h->bulkout_done = 0;

	if (acm_h->console_print_mark) {
		ret = gadget_ep_get_rx_sz(acm_h->bulk_out_ep, &rx_sz);
		if (ret < 0) {
			LTRACEF("Not found rx function");
			return;
		}
#if CONSOLE_HAS_INPUT_BUFFER
		cbuf_write(&console_input_cbuf, rx_data_buf, rx_sz, false);
		start_console_rx(acm_h);
#endif
	}
}

static void cdc_acm_stddone_cb(void *handle, USB_STD_REQUEST *std_req)
{
	struct cdcacm_infor *acm_h;

	acm_h = handle;

	switch ((CDC_CLASS_REQUEST) std_req->bRequest) {
	case SET_LINE_CODING:
		/* Data Terminal Rate */
		acm_h->baud_rate = *((unsigned int *) acm_h->std_req_buf);
		/* Number of Stop bit */
		acm_h->stop_bit = *(u8 *) ((u8 *) acm_h->std_req_buf + 4);
		/* Parity Type */
		acm_h->parity_bit = *(u8 *) ((u8 *) acm_h->std_req_buf + 5);
		/* Data Bits */
		acm_h->data_bits = *(u8 *) ((u8 *) acm_h->std_req_buf + 6);
		LTRACEF("[cdc-acm]Received baud rate:%d\n", acm_h->baud_rate);
		LTRACEF("[cdc-acm]Received stop bit:%s\n",
			(acm_h->stop_bit == 1) ? "1.5" :
			(acm_h->stop_bit == 2) ? "2" : "0");
		LTRACEF("[cdc-acm]Received parity type:%s\n",
			(acm_h->parity_bit == 1) ? "Odd" :
			(acm_h->parity_bit == 2) ? "Even" :
			(acm_h->parity_bit == 3) ? "Mark" :
			(acm_h->parity_bit == 4) ? "Space" : "None");
		LTRACEF("[cdc-acm]Received data bits:%d\n", acm_h->data_bits);
		break;
	case SET_COMM_FEATURE:
		acm_h->abstract_state = *((unsigned int *) acm_h->std_req_buf);
		LTRACEF("[cdc-acm]Received abstract state:0x%x\n", acm_h->abstract_state);
		break;
	case GET_LINE_CODING:
		if ((acm_h->rts == 1) && (acm_h->console_print_mark)) {
			if (acm_h->bulk_in_ep_status > 0)
				start_console_tx(acm_h);
			if (acm_h->bulk_out_ep_status > 0)
				start_console_rx(acm_h);
		}
		break;
	default:
		break;
	}
}

static int cdc_acm_stdreq_cb(void *handle, USB_STD_REQUEST *std_req, void **data_stage_buf_addr, unsigned int *data_stage_buf_sz)
{
	int ret = USB_STDREQ_OK;
	struct cdcacm_infor *acm_h;

	acm_h = handle;
	if ((std_req->wIndex != acm_h->cdc_if_num) &&
			(std_req->wIndex != acm_h->dic_if_num)) {
		return USB_STDREQ_NOT_MINE;
	}
	switch ((CDC_CLASS_REQUEST) std_req->bRequest) {
	case SET_LINE_CODING:
		*data_stage_buf_addr = acm_h->std_req_buf;
		*data_stage_buf_sz = 7;
		break;
	case SET_COMM_FEATURE:
		*data_stage_buf_addr = acm_h->std_req_buf;
		*data_stage_buf_sz = 2;
		break;
	case GET_LINE_CODING:
		/* Data Terminal Rate */
		*((unsigned int *) acm_h->std_req_buf) = acm_h->baud_rate;
		/* Number of Stop bit */
		*(u8 *) ((u8 *) acm_h->std_req_buf + 4) = acm_h->stop_bit;
		/* Parity Type */
		*(u8 *) ((u8 *) acm_h->std_req_buf + 5) = acm_h->parity_bit;
		/* Data Bits */
		*(u8 *) ((u8 *) acm_h->std_req_buf + 6) = acm_h->data_bits;
		*data_stage_buf_addr = acm_h->std_req_buf;
		break;
	case SET_CONTROL_LINE_STATE:
		/* Mark Data Terminal Ready signal */
		if (std_req->wValue & 0x1) {
			acm_h->dtr = 1;
			LTRACEF("[cdc-acm]DTR Present\n");
		} else {
			acm_h->dtr = 0;
			LTRACEF("[cdc-acm]DTR Absent\n");
		}
		/* Mark Request To Send signal */
		if (std_req->wValue & 0x2) {
			LTRACEF("[cdc-acm]RTS Activate\n");
			acm_h->rts = 1;
		} else {
			LTRACEF("[cdc-acm]RTS Deactivate\n");
			acm_h->rts = 0;
		}
		break;
	case GET_COMM_FEATURE:
		if (std_req->wValue == 0x1) {
			/* ABSTRACT_STATE only support */
			*((unsigned int *) acm_h->std_req_buf) = acm_h->abstract_state;
			*data_stage_buf_addr = acm_h->std_req_buf;
		} else
			ret = USB_STDREQ_ERROR;
		break;
	case SEND_BREAK:
		break;
	case SEND_ENCAPSULATED_COMMAND:
	case GET_ENCAPSULATED_RESPONSE:
	case CLEAR_COMM_FEATURE:
		break;
	default:
		ret = USB_STDREQ_ERROR;
		break;
	}
	return ret;
}

static void cdcacm_config(void *class_handle, USB_SPEED speed, unsigned int if_num, unsigned int set_num)
{
	struct cdcacm_infor *acm_h;
	struct desc_list_node *pos, *tmp;

	acm_h = class_handle;
	list_for_every_entry_safe(&desc_list, pos, tmp, struct desc_list_node, entry) {
		if (speed >= USBSPEED_SUPER) {

		} else if (speed == USBSPEED_HIGH) {
			struct cdc_acm_config_desc_hs *hs_desc;

			hs_desc = pos->hs_desc_ptr;
			if (if_num == hs_desc->ctrl_if_desc.bInterfaceNumber) {
				acm_h->intr_in_ep_status = gadget_ep_req_cfg(&hs_desc->intr_ep_desc, NULL);
			} else if (if_num == hs_desc->data_if_desc.bInterfaceNumber) {
				acm_h->bulk_out_ep_status = gadget_ep_req_cfg(&hs_desc->bulk_out_ep_desc, NULL);
				acm_h->bulk_in_ep_status = gadget_ep_req_cfg(&hs_desc->bulk_in_ep_desc, NULL);

				gadget_ep_set_cb_xferdone(acm_h->bulk_in_ep, callback_bulk_in, acm_h);
				gadget_ep_set_cb_xferdone(acm_h->bulk_out_ep, callback_bulk_out, acm_h);
			}
		} else {

		}
	}
}

__attribute__((weak)) int cdc_acm_get_num_port(void){return 1;}
__attribute__((weak)) void cdc_acm_get_serial_config(int port_id, int *baud_rate, unsigned char *stop_bit, unsigned char *parity_bit, unsigned char *data_bits)
{
	*baud_rate = 115200;
	*stop_bit = 0;
	*parity_bit = 0;
	*data_bits = 8;
}

static void cdc_acm_probe(uint level)
{
	u8 start_if_num;
	int out_ep_num, in_ep_num, num_port;
	int intr_in_ep;
	char interface_name[] = "LK USB Serial";
	int func_cnt;
	struct usb_dev_infor *dev_ops;
	struct cdc_acm_config_desc_hs *hs_desc;
	struct cdcacm_infor *acm_h;

	num_port = cdc_acm_get_num_port();
	if (num_port < 1)
		num_port = 1;
	dev_ops = calloc(num_port, sizeof(struct usb_dev_infor));
	acm_h = calloc(num_port, sizeof(struct cdcacm_infor));
	/* Get Interface Number from gadget layer */
	for (func_cnt = 0; func_cnt < num_port; func_cnt++) {
		start_if_num = get_intf_num(2);
		out_ep_num = get_ep_id(USBDIR_OUT);
		in_ep_num = get_ep_id(USBDIR_IN);
		intr_in_ep = get_ep_id(USBDIR_IN);

		list_initialize(&acm_h[func_cnt].tx_data_q_head);
		cdc_acm_get_serial_config(func_cnt,
					  &acm_h[func_cnt].baud_rate,
					  &acm_h[func_cnt].stop_bit,
					  &acm_h[func_cnt].parity_bit,
					  &acm_h[func_cnt].data_bits);
		if (func_cnt != 0) {
			struct desc_list_node *node;

			node = calloc(1, sizeof(struct desc_list_node) + sizeof(struct cdc_acm_config_desc_hs));
			node->hs_desc_ptr = (void *) ((addr_t) node +  sizeof(struct desc_list_node));
			hs_desc =  node->hs_desc_ptr;
			memcpy(hs_desc, &hs_config_desc, sizeof(struct cdc_acm_config_desc_hs));
		} else {
			hs_desc = &hs_config_desc;
			list_add_tail(&desc_list, &desc_head.entry);
		}
		dev_ops[func_cnt].hs_desc_addr = hs_desc;
		dev_ops[func_cnt].hs_desc_sz = sizeof(struct cdc_acm_config_desc_hs);

		dev_ops[func_cnt].class_handle = &acm_h[func_cnt];
		dev_ops[func_cnt].cb_stdreq = cdc_acm_stdreq_cb;
		dev_ops[func_cnt].cb_stdreq_done = cdc_acm_stddone_cb;
		dev_ops[func_cnt].config = cdcacm_config;

		dev_ops[func_cnt].start_if_num = start_if_num;
		dev_ops[func_cnt].num_if = 2;

		acm_h[func_cnt].cdc_if_num = start_if_num;
		acm_h[func_cnt].dic_if_num = start_if_num + 1;
		acm_h[func_cnt].bulk_out_ep = out_ep_num;
		acm_h[func_cnt].bulk_in_ep = in_ep_num;
		acm_h[func_cnt].intr_in_ep = intr_in_ep;
		acm_h[func_cnt].rts = 0;
		acm_h[func_cnt].dtr = 0;
		/* Set Interface Number and EP numbers on the descriptor HS/SS */
		hs_desc->if_assoc_desc.bFirstInterface = start_if_num;
		hs_desc->if_assoc_desc.iFunction = get_str_id(interface_name, strlen(interface_name));
		hs_desc->ctrl_if_desc.bInterfaceNumber = start_if_num;
		hs_desc->data_if_desc.bInterfaceNumber = start_if_num + 1;
		hs_desc->intr_ep_desc.bEndpointAddress = intr_in_ep;
		hs_desc->bulk_in_ep_desc.bEndpointAddress |= in_ep_num;
		hs_desc->bulk_out_ep_desc.bEndpointAddress |= out_ep_num;

		if (console_port == func_cnt) {
			cdc_acm_print_cb.context = &acm_h[func_cnt];
			acm_h[func_cnt].console_print_mark = true;
		} else
			acm_h[func_cnt].console_print_mark = false;

		register_desc_buf(&dev_ops[func_cnt]);
	}
}
LK_INIT_HOOK(cdc_acm_probe, &cdc_acm_probe, LK_INIT_LEVEL_KERNEL);

static void cdc_acm_console_print(print_callback_t *cb, const char *str, size_t len)
{
	struct cdcacm_infor *cdc_acm_h;
	int index, os, bak;
	unsigned int org_wridx;

	org_wridx = index = index_writer;
	if (index + len >= LOG_BUF_SZ) {
		os = index + len - LOG_BUF_SZ;
		bak = index;
		index = os;
		memcpy((void *) (log_buffer_temp + bak), str, len - os);
		memcpy((void *) log_buffer_temp, str + (len - os), os);
	} else {
		bak = index;
		index += len;
		memcpy((void *) (log_buffer_temp + bak), str, len);
	}
	index_writer = index;

	cdc_acm_h = cb->context;
	if (cdc_acm_h == NULL)
		return;
	if (cdc_acm_h->rts == 0)
		return;
	if (cdc_acm_h->bulkin_done)
		return;
	if (org_wridx == index_reader)
		start_console_tx(cdc_acm_h);
}

static void cdc_acm_print_cb_register(uint level)
{
	cdc_acm_print_cb.print = cdc_acm_console_print;
	register_print_callback(&cdc_acm_print_cb);
}
LK_INIT_HOOK(cdc_acm_print_cb_register, cdc_acm_print_cb_register, LK_INIT_LEVEL_PLATFORM_EARLY - 1);
