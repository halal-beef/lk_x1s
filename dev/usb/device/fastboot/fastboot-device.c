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
#include <lk/debug.h>
#include <lk/trace.h>
#include <lk/reg.h>
#include <malloc.h>
#include <kernel/thread.h>
#include <kernel/event.h>
#include <lk/init.h>
#include <platform/delay.h>

#include "usb-def.h"
#include "dev/usb/gadget.h"
#include "dev/usb/fastboot.h"

#define LOCAL_TRACE 0

extern int usb_config_state;

struct fastboot_infor {
	unsigned int bulk_in_ep;
	int bulk_in_ep_status;

	unsigned int bulk_out_ep;
	int bulk_out_ep_status;

	void *rx_cmd_buf;
	int rx_cmd_buf_sz;

	void *tx_response_buf;
	int tx_response_buf_sz;

	volatile bool tx_sts_done;
	volatile bool just_info;

	event_t rx_done_event;
	event_t tx_done_event;

	struct cmd_fastboot_interface *interface;

	volatile int payload_phase;
	int payload_dir;
	void *paylod_buf;
	unsigned int payload_req_len;

	unsigned int prot_req_rx_sz;
} fastboot_h;

enum {
	FASTBOOT_PAYLOAD_NONE = 0,
	FASTBOOT_PAYLOAD_IN_PROGRESS = 1,
	FASTBOOT_PAYLOAD_START_MARK = 2,
};

extern unsigned int s_fb_on_diskdump;

void fastboot_send_status(char *response, unsigned int len, int sync);

static void ready_to_rx_cmd(void)
{
	unsigned int rx_sz;

	rx_sz = (fastboot_h.prot_req_rx_sz) ? fastboot_h.prot_req_rx_sz : 64;
	gadget_ep_set_buf(fastboot_h.bulk_out_ep, fastboot_h.rx_cmd_buf, rx_sz, 0);
	gadget_ep_start(fastboot_h.bulk_out_ep);
}

static void do_payload(void)
{
	unsigned int ep_num;

	LTRACE_ENTRY;

	ep_num = (fastboot_h.payload_dir == USBDIR_OUT) ? fastboot_h.bulk_out_ep : fastboot_h.bulk_in_ep;

	fastboot_h.payload_phase = FASTBOOT_PAYLOAD_IN_PROGRESS;
	if (fastboot_h.payload_dir == USBDIR_IN)
		fastboot_h.tx_sts_done = 1;
	/* Check Reqeust Sz for transfer */
	LTRACEF("Payload %s done=>Addr:%p,Sz:0x%x\n",
		(fastboot_h.payload_dir == USBDIR_OUT) ? "download" : "upload",
		fastboot_h.paylod_buf, fastboot_h.payload_req_len);
	gadget_ep_set_buf(ep_num, (void *)fastboot_h.paylod_buf, fastboot_h.payload_req_len, GADGET_BUF_LAST);
	gadget_ep_start(ep_num);

	LTRACE_EXIT;
}

static void check_payload_done(unsigned int xfer_sz)
{
	LTRACE_ENTRY;

	LTRACEF("Payload %s req=>Addr:%p,Sz:0x%x\n",
			(fastboot_h.payload_dir == USBDIR_OUT) ? "download" : "upload",
			fastboot_h.paylod_buf, xfer_sz);
	LTRACEF("Payload Req Sz:0x%x,Xfer done Sz:0x%x\n", fastboot_h.payload_req_len, xfer_sz);
	fb_cmd_set_downloaded_sz(xfer_sz);

	if (fastboot_h.payload_req_len < xfer_sz)
		fastboot_h.payload_req_len = 0;
	else
		fastboot_h.payload_req_len -= xfer_sz;

	if (fastboot_h.payload_req_len == 0) {
		fastboot_h.payload_phase = FASTBOOT_PAYLOAD_NONE;
		if (fastboot_h.payload_dir == USBDIR_OUT)
			fastboot_send_status((char *) "OKAY", 4, FASTBOOT_TX_ASYNC);
		else
			ready_to_rx_cmd();
	} else {
		fastboot_h.paylod_buf = (void *) ((addr_t) fastboot_h.paylod_buf + xfer_sz);
		do_payload();
	}

	LTRACE_EXIT;
}

static void tx_status_callback(void *handle)
{
	LTRACE_ENTRY;

	fastboot_h.tx_sts_done = 0;
	event_signal(&fastboot_h.tx_done_event, true);
	if (fastboot_h.just_info) {
		fastboot_h.just_info = 0;
		return;
	}
	/* Make bulk out ep to rx command from host if not payload phase*/
	if (!fastboot_h.payload_phase)
		ready_to_rx_cmd();
	else {
		if (fastboot_h.payload_phase == FASTBOOT_PAYLOAD_START_MARK)
			do_payload();
		else {
			unsigned int rx_sz = 0;

			if (fastboot_h.payload_dir == USBDIR_OUT)
				gadget_ep_get_rx_sz(fastboot_h.bulk_out_ep, &rx_sz);
			else
				gadget_ep_get_rx_sz(fastboot_h.bulk_in_ep, &rx_sz);
			check_payload_done(rx_sz);
		}
	}
	LTRACE_EXIT;
}

void fastboot_set_payload_data(int dir, void *buf, unsigned int len)
{
	fastboot_h.payload_phase = FASTBOOT_PAYLOAD_START_MARK;
	fastboot_h.payload_dir = dir;
	fastboot_h.paylod_buf = buf;
	fastboot_h.payload_req_len = len;
}

void fasboot_set_rx_sz(unsigned int prot_req_sz)
{
	fastboot_h.prot_req_rx_sz = prot_req_sz;
}


void fastboot_tx_event_init(void)
{
	event_init(&fastboot_h.tx_done_event , false, 0);
}
void fastboot_send_info(char *response, unsigned int len)
{
	LTRACE_ENTRY;

	/* Check Last TX status transfer done */
	if (fastboot_h.tx_sts_done) {
		event_wait(&fastboot_h.tx_done_event);
		event_unsignal(&fastboot_h.tx_done_event);

		if (s_fb_on_diskdump)
			thread_yield();
	}

	fastboot_h.tx_sts_done = 1;
	/* Mark for not trasiante state mahcine */
	fastboot_h.just_info = 1;
	memcpy(fastboot_h.tx_response_buf, response, len);
	gadget_ep_set_buf(fastboot_h.bulk_in_ep, fastboot_h.tx_response_buf, len, GADGET_BUF_LAST);
	gadget_ep_start(fastboot_h.bulk_in_ep);

	/* Check transfer done */
	event_wait(&fastboot_h.tx_done_event);
	event_unsignal(&fastboot_h.tx_done_event);

	if (s_fb_on_diskdump)
		thread_yield();

	LTRACE_EXIT;
}

void fastboot_send_payload(void *buf, unsigned int len)
{
	LTRACE_ENTRY;

	/* Mark for not trasiante state mahcine */
	fastboot_h.just_info = 1;
	//memcpy(fastboot_h.tx_response_buf, response, len);
	gadget_ep_set_buf(fastboot_h.bulk_in_ep, buf, len, GADGET_BUF_LAST);
	gadget_ep_start(fastboot_h.bulk_in_ep);

	/* Check transfer done */
	event_wait(&fastboot_h.tx_done_event);
	event_unsignal(&fastboot_h.tx_done_event);

	thread_yield();

	LTRACE_EXIT;
}

void fastboot_send_status(char *response, unsigned int len, int sync)
{
	LTRACE_ENTRY;

	/* Check Last TX status transfer done */
	if (fastboot_h.tx_sts_done) {
		event_wait(&fastboot_h.tx_done_event);
		event_unsignal(&fastboot_h.tx_done_event);

		if (s_fb_on_diskdump)
			thread_yield();
	}

	fastboot_h.tx_sts_done = 1;
	memcpy(fastboot_h.tx_response_buf, response, len);
	gadget_ep_set_buf(fastboot_h.bulk_in_ep, fastboot_h.tx_response_buf, len, GADGET_BUF_LAST);
	gadget_ep_start(fastboot_h.bulk_in_ep);
	if (!sync) {
		LTRACE_EXIT;
		return;
	}
	/* Check transfer done */
	event_wait(&fastboot_h.tx_done_event);
	event_unsignal(&fastboot_h.tx_done_event);

	if (s_fb_on_diskdump)
		thread_yield();

	LTRACE_EXIT;
}

static int wait_rx_done(void *arg)
{
	unsigned int rx_sz = 0;
	int ret;

	for (;;) {
		event_wait(&fastboot_h.rx_done_event);

		LTRACE_ENTRY;

		event_unsignal(&fastboot_h.rx_done_event);

		ret = gadget_ep_get_rx_sz(fastboot_h.bulk_out_ep, &rx_sz);
		if (ret < 0) {
			LTRACEF("Not found rx function");
			continue;
		}
		LTRACEF("Receive size is %d\n", rx_sz);
		LTRACEF("Payload Phase:%d\n", fastboot_h.payload_phase);
		if (fastboot_h.payload_phase == FASTBOOT_PAYLOAD_IN_PROGRESS)
			check_payload_done(rx_sz);
		else {
			fastboot_h.prot_req_rx_sz = 0;
			rx_handler(fastboot_h.rx_cmd_buf, rx_sz);
			memset(fastboot_h.rx_cmd_buf, '\0', rx_sz);
		}
		LTRACE_EXIT;
	}
	return 0;
}

static void rx_callback(void *handle)
{
	LTRACE_ENTRY;

	event_signal(&fastboot_h.rx_done_event, true);
}

static struct __attribute__((__packed__)) fastboot_config_desc_hs {
	USB_INTERFACE_DESCRIPTOR if_desc;
	USB_ENDPOINT_DESCRIPTOR bulk_inep_desc;
	USB_ENDPOINT_DESCRIPTOR bulk_outep_desc;
} hs_config_desc = {
	.if_desc = {
		.bLength = USB_DESC_SIZE_INTERFACE,
		.bDescriptorType = INTERFACE_DESCRIPTOR,
		.bAlternateSetting = 0x0,
		.bNumEndpoints = 2,
		.bClass = 0xff,
		.bSubClass = 0x42,
		.bProtocol = 0x3,
	},
	.bulk_inep_desc = {
		.bLength = USB_DESC_SIZE_ENDPOINT,
		.bDescriptorType = ENDPOINT_DESCRIPTOR,
		.bEndpointAddress = EP_ADDR_IN,
		.bmAttributes = EP_ATTR_BULK,
		.wMaxPacketSize = 512,
	},
	.bulk_outep_desc = {
		.bLength = USB_DESC_SIZE_ENDPOINT,
		.bDescriptorType = ENDPOINT_DESCRIPTOR,
		.bEndpointAddress = EP_ADDR_OUT,
		.bmAttributes = EP_ATTR_BULK,
		.wMaxPacketSize = 512,
	},
};

static void fastboot_config(void *class_handle, USB_SPEED speed, unsigned int if_num, unsigned int set_num)
{
	LTRACE_ENTRY;

	if (speed >= USBSPEED_SUPER) {

	} else if (speed == USBSPEED_HIGH) {
		fastboot_h.bulk_out_ep_status = gadget_ep_req_cfg(&hs_config_desc.bulk_outep_desc, NULL);
		fastboot_h.bulk_in_ep_status = gadget_ep_req_cfg(&hs_config_desc.bulk_inep_desc, NULL);
	} else {

	}

	/* Make Ready to rx commnad from Host */
	gadget_ep_set_cb_xferdone(fastboot_h.bulk_out_ep, rx_callback, &fastboot_h);
	gadget_ep_set_cb_xferdone(fastboot_h.bulk_in_ep, tx_status_callback, &fastboot_h);
	fastboot_h.tx_sts_done = 0;
	ready_to_rx_cmd();
	usb_config_state = 1;
}

static struct usb_dev_infor fastboot_infor = {
	.hs_desc_addr = &hs_config_desc,
	.hs_desc_sz = sizeof(hs_config_desc),

	.config = fastboot_config,
};

static void fasboot_probe(uint level)
{
	u8 start_if_num;
	int out_ep_num, in_ep_num;
	char interface_name[] = "Android Fastboot";

	fastboot_h.prot_req_rx_sz = 0;
	/* Get Interface Number from gadget layer */
	start_if_num = get_intf_num(1);
	fastboot_h.bulk_out_ep = out_ep_num = get_ep_id(USBDIR_OUT);
	fastboot_h.bulk_in_ep = in_ep_num = get_ep_id(USBDIR_IN);
	/* Set Interface Number and EP numbers on the descriptor HS/SS */
	hs_config_desc.if_desc.bInterfaceNumber = start_if_num;
	hs_config_desc.if_desc.iInterface = get_str_id(interface_name, strlen(interface_name));
	hs_config_desc.bulk_inep_desc.bEndpointAddress |= in_ep_num;
	hs_config_desc.bulk_outep_desc.bEndpointAddress |= out_ep_num;
	fastboot_infor.start_if_num = start_if_num;
	fastboot_infor.num_if = 1;
	/* Allocate RX buffer */
	fastboot_h.rx_cmd_buf = calloc(1, 4096);
	fastboot_h.tx_response_buf = calloc(1, 4096);
	fastboot_h.tx_response_buf_sz = 4096;
	register_desc_buf(&fastboot_infor);

	event_init(&fastboot_h.rx_done_event , false, 0);
	event_init(&fastboot_h.tx_done_event , false, 0);
	thread_resume(thread_create("fastboot rx handle", &wait_rx_done, NULL, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE));
}
LK_INIT_HOOK(fasboot_probe, &fasboot_probe, LK_INIT_LEVEL_KERNEL);
