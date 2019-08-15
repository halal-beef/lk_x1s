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

#ifndef DEV_USB_DWC3_DWC3_GLOBAL_H_
#define DEV_USB_DWC3_DWC3_GLOBAL_H_

#include "dev/usb/dwc3-config.h"

struct dwc3_glb_dev {
	struct dwc3_plat_config plat;

	/* System related resource */
	void *base;
	unsigned int sclk;
	int io_ovcr;
	int io_ovcr_func_num;
	int io_vbus;
	int io_vbus_func_num;

	/* configuration structure */

	/* ISR call back */
	void (*FnDRDCallBack[4])(void *);
	struct platform_device *ArgDRDCallBack[4];

	USB_OPMODE eUSBMode;
	int link_verion;
	int link_id;
	bool init_intr;

	/* Suspend mask sema */
	u8 susp_sema_flag;
	u8 susp_set_flag;
	u8 sleep_set_flag;

	bool suspendclk_dir_osc;
#if 0
	/* Debugging IPC */
	volatile u32 ipc_u1_enter;
	volatile u32 ipc_u1_enter_handshake;
	volatile u32 ipc_u1_enter_req;
	volatile u32 ipc_inactive_enter;
#endif

};


struct dwc3_glb_dev *usb3_glb_init(void);
int dwc3_glb_init_late(struct dwc3_glb_dev *glb_dev, USB_OPMODE mode);

void dwc3_glb_unmask_suspend(struct dwc3_glb_dev *glb_dev, u8 bSetValue);
void dwc3_glb_register_isr(struct dwc3_glb_dev *glb_dev, USB_OPMODE eOpMode, int intr_num, void (fnCallBack(void *)), void *pd_upper);
void dwc3_glb_release_isr(struct dwc3_glb_dev *glb_dev, USB_OPMODE eOpMode, int intr_num);

void dwc3_glb_phy_dp_pullup_en(struct dwc3_glb_dev *glb_dev, bool en_pullup);


#endif /* DEV_USB_DWC3_DWC3_GLOBAL_H_ */
