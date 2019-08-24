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

typedef struct _DWC3_DEV_EP0_HANDLER {
	/* Platform device of Root USB3.0 Device */
	DWC3_DEV_HANDLER dwc3_dev_h;

	/* EP Handler for Control Transfer */
	DWC3_DEV_EP_HANDLER m_hEP;

	/* Control Transfer State Machine */
	volatile USB3_DEV_TRB_TYPE_e m_eNextCtrlPkt;
	volatile USB3_DEV_EPEVT_TYPE_e m_eNextEvt;
	u8(*fn_NextControlXfer)(struct _DWC3_DEV_EP0_HANDLER *, void **, u32 *);

	/* Some Actcions at Status Stage In Control Transfer */
	void (*fn_DoAtEndOfTransfer)(DWC3_DEV_HANDLER , u32);
	u32 cn_para_ctrl_done;

	/* Standard Request buffer, must be word-aligned */
	USB_STD_REQUEST *m_pDeviceRequest;

	/* Buffer for GET_STATUS & GET_DESCRIPTOR up to 512 bytes in length */
	u8 *m_pStatusBuf;
	u32 m_uSELBuffer[2];

	// Will be optimized
	u16 m_ucDeviceSelector;
	u16 m_ucInterfaceSelector;
	u8 AlternateSetting;
} DWC3_DEV_EP0, *DWC3_DEV_EP0_HANDLER;

//static DWC3_DEV_EP0_HANDLER g_hUSB30_EP0;

static void _DWC3_DEV_EP0_FreeMemAtCmpe(DWC3_DEV_HANDLER dwc3_dev_h, u32 uPara)
{
	free((void *)((unsigned long) uPara));
}

static void dwc3_dev_ep0_set_cb_at_xfercmpl(DWC3_DEV_EP0_HANDLER dwc3_dev_ep0_h, void func(DWC3_DEV_HANDLER, u32),
		u32 uPara)
{
	dwc3_dev_ep0_h->fn_DoAtEndOfTransfer = func;
	dwc3_dev_ep0_h->cn_para_ctrl_done = uPara;
}

static void dwc3_dev_ep0_cb_at_ctrl_xfer_done(DWC3_DEV_EP0_HANDLER dwc3_dev_ep0_h)
{
	if (dwc3_dev_ep0_h->fn_DoAtEndOfTransfer) {
		/* Run user call back */
		dwc3_dev_ep0_h->fn_DoAtEndOfTransfer(dwc3_dev_ep0_h->dwc3_dev_h,
						     dwc3_dev_ep0_h->cn_para_ctrl_done);
		/* Clear User Call Back */
		dwc3_dev_ep0_h->fn_DoAtEndOfTransfer = NULL;
		dwc3_dev_ep0_h->cn_para_ctrl_done = 0;
	}
}

static void dwc3_dev_ep0_req_stall(DWC3_DEV_EP0_HANDLER dwc3_dev_ep0_h)
{
	DWC3_DEV_HANDLER dwc3_dev_h = dwc3_dev_ep0_h->dwc3_dev_h;
	dwc3_dev_h->m_bEPHalted |= (1 << 0);
}

static u8 dwc3_dev_ep0_SetUpStage(DWC3_DEV_EP0_HANDLER dwc3_dev_ep0_h, void **pNextAddr, u32 *pLen)
{
	/* call User layer for control stage complete */
	gadget_call_cb_class_std_done(dwc3_dev_ep0_h->m_pDeviceRequest);
	*pNextAddr = (void *)(dwc3_dev_ep0_h->m_pDeviceRequest);
	*pLen = dwc3_dev_ep0_h->m_hEP->m_uMaxPktSize;
	dwc3_dev_ep0_cb_at_ctrl_xfer_done(dwc3_dev_ep0_h);
	return true;
}

static u8 DWC3_DEV_EP0_StatusStage(DWC3_DEV_EP0_HANDLER dwc3_dev_ep0_h, void **pNextAddr, u32 *pLen)
{
	u8 bRet = true;

	//after data stage complete event, Status stage must be transited to
	//next state in Trnasfer Not Ready Event(XFER_NRDY) of status stage request
	if (dwc3_dev_ep0_h->m_eNextEvt != DEPEVT_EVT_XFER_NRDY) {
		dwc3_dev_ep0_h->m_eNextEvt = DEPEVT_EVT_XFER_NRDY;
		dwc3_dev_ep0_h->m_hEP->m_eDir = (USB_DIR)(~(u8)(dwc3_dev_ep0_h->m_hEP->m_eDir) & 0x1);
		bRet = false;
	}
	*pNextAddr = (void *)(dwc3_dev_ep0_h->m_pStatusBuf);
	*pLen = 0;
	return bRet;
}

static void dwc3_dev_ep0_get_desc_req(DWC3_DEV_EP0_HANDLER dwc3_ep0_h, USB_STDDESC_TYPE eDescType, void **pNextAddr, u32 *pLen)
{
	DWC3_DEV_HANDLER dwc3_dev_h = dwc3_ep0_h->dwc3_dev_h;
	unsigned int strdesc_sz = 0;
	int ret;
#if 0

	USB_CONFIGURATION_DESCRIPTOR *pConfigDesc = NULL;
#endif

	switch (eDescType) {
	case DEVICE_DESCRIPTOR:
		U3DBG_STDREQ("Get Device Descriptor = 0x%x \n", *pLen);
		*pNextAddr = &dwc3_dev_h->dec_desc;
		if (*pLen >= USB_DESC_SIZE_DEVICE)
			*pLen = USB_DESC_SIZE_DEVICE;
		break;
	case BOS_DESC:
		U3DBG_STDREQ("Get BOS_DESCRIPTOR \n");
		*pNextAddr = dwc3_dev_h->bos_desc_buf;
		if (*pLen >= dwc3_dev_h->bos_desc_total_sz)
			*pLen = dwc3_dev_h->bos_desc_total_sz;
		break;
	case CONFIGURATION_DESCRIPTOR:
		/* TODO: Find Configuration descritor buffer address from gadget */
#if 0
		U3DBG_STDREQ("Current Configration Number : %d\n",
			     UDEV_GetCurrentConfigNumber(dwc3_dev_h->udev_p));
		pConfigDesc = UDEV_GetConfigDesc(dwc3_dev_h->udev_p);
#endif
		/* Get Configuration Desc */
		*pNextAddr = dwc3_dev_h->config_desc_buf;
		if (*pLen >= dwc3_dev_h->config_desc_total_sz)
			*pLen = dwc3_dev_h->config_desc_total_sz;
		U3DBG_STDREQ("Get Configuration Descriptor = 0x%x \n", *pLen);
		break;
	case STRING_DESCRIPTOR:
		/* TODO: Find String descriptor buffer from dedicated number */
		ret = get_str_desc_buf_addr(LO_BYTE(dwc3_ep0_h->m_pDeviceRequest->wValue), pNextAddr, &strdesc_sz);
		if (ret < 0)
			dwc3_dev_ep0_req_stall(dwc3_ep0_h);
		else {
			if (*pLen >= strdesc_sz)
				*pLen = strdesc_sz;
		}
		U3DBG_STDREQ("Get String Descriptor,addr:0x%p,sz:%d\n", *pNextAddr, *pLen);
		break;
	case DEVICE_QUALIFIER:
		// only supported in over 2.0
#if 0
		U3DBG_STDREQ("Get Device Qualifier Descriptor = 0x%x \n", *pLen);
		if ((dwc3_dev_h->m_eEnumeratedSpeed != USBSPEED_SUPER) ||
				(dwc3_dev_h->m_eEnumeratedSpeed != USBSPEED_SUPERPLUS)) {
			USB_DEVICE_QUALIFIER_DESCRIPTOR *pQualDesc = _usb_alloc(
						sizeof(USB_DEVICE_QUALIFIER_DESCRIPTOR));
			memcpy((void *) pQualDesc, (void *) &(dwc3_ep0_h->dev_attr.oDevice),
				   USB_DESC_SIZE_DEVICE_QUALIFIER);
			pQualDesc->bLength = USB_DESC_SIZE_DEVICE_QUALIFIER;
			pQualDesc->bDescriptorType = DEVICE_QUALIFIER;
			pQualDesc->bNumConfigurations =
				dwc3_ep0_h->dev_attr.oDevice.bNumConfigurations;
			pQualDesc->bReserved = 0x0;
			*pNextAddr = (void *)(pQualDesc);
			if (*pLen > USB_DESC_SIZE_DEVICE_QUALIFIER)
				*pLen = USB_DESC_SIZE_DEVICE_QUALIFIER;
			dwc3_dev_ep0_set_cb_at_xfercmpl(dwc3_ep0_h, _DWC3_DEV_EP0_FreeMemAtCmpe,
												   (unsigned long)(pQualDesc));
		} else {
			U3DBG_STDREQ("Super Speed is not supported\n");
			dwc3_dev_ep0_req_stall(dwc3_ep0_h);
		}
#endif
		break;
	case OTHER_SPEED_CONFIGURATION:
#if 0
		U3DBG_STDREQ("Get Configuration Descriptor = 0x%x \n", *pLen);
		U3DBG_STDREQ("Current Configration Number : %d\n",
					 UDEV_GetCurrentConfigNumber(dwc3_dev_h->udev_p));
		U3DBG_STDREQ("Get OTHER_SPEED_CONFIGURATION \n");
		pConfigDesc = UDEV_GetConfigDesc(dwc3_dev_h->udev_p);
		if ((dwc3_dev_h->m_eEnumeratedSpeed != USBSPEED_SUPER) ||
				(dwc3_dev_h->m_eEnumeratedSpeed != USBSPEED_SUPER)) {
			u32 uBufferLength = 0;
			s32 uTotalSize = pConfigDesc->m_oDesc.wTotalLength;
			USB_CONFIGURATION_DESCRIPTOR *pOtherSpeedConfigfifo = dwc3_calloc_align(uTotalSize, 64);
			u8 *p_ucAddr = (u8 *) pOtherSpeedConfigfifo;
			UDEV_GetFuncDesc(dwc3_dev_h->udev_p, pNextAddr, &uBufferLength);
			//1 1. Copy Configuration DESC
			memcpy((void *) pOtherSpeedConfigfifo, (void *) &(pConfigDesc->m_oDesc),
				   USB_DESC_SIZE_OTHER_SPEED_CFG);
			memcpy((void *)((unsigned long) pOtherSpeedConfigfifo
						 + USB_DESC_SIZE_OTHER_SPEED_CFG), (void *)(*pNextAddr),
				uBufferLength);
			//1 2. Set Transfer Address and Size
			*pNextAddr = (void *) pOtherSpeedConfigfifo;
			//1 3. Change Max Packet Size in EP DESC
			while (uTotalSize) {
				if (p_ucAddr[1] == CONFIGURATION_DESCRIPTOR)
					p_ucAddr[1] = OTHER_SPEED_CONFIGURATION;
				else if (p_ucAddr[1] == ENDPOINT_DESCRIPTOR) {
					switch ((USB_EP)(p_ucAddr[3] & 0x3)) {
					case USBEP_BULK:
						switch (dwc3_dev_h->m_eEnumeratedSpeed) {
						case USBSPEED_FULL:
							p_ucAddr[4] = LO_BYTE(512);
							p_ucAddr[5] = HI_BYTE(512);
							USBDBG("%d%d", p_ucAddr[4], p_ucAddr[5]);
							break;
						case USBSPEED_HIGH:
							p_ucAddr[4] = LO_BYTE(64);
							p_ucAddr[5] = HI_BYTE(64);
							break;
						default:
							break;
						}
						break;
					case USBEP_INTR:
					case USBEP_ISOC:
					default:
						break;
					}
				}
				uTotalSize -= p_ucAddr[0];
				p_ucAddr += p_ucAddr[0];
			}
			//Free Buffer
			dwc3_dev_ep0_set_cb_at_xfercmpl(dwc3_ep0_h, _DWC3_DEV_EP0_FreeMemAtCmpe,
								(unsigned long)(pOtherSpeedConfigfifo));

		} else {
			// super speed
			U3DBG_STDREQ("Super Speed is not supported\n");
			dwc3_dev_ep0_req_stall(dwc3_ep0_h);
		}
#endif
		break;
	default:
		dwc3_dev_ep0_req_stall(dwc3_ep0_h);
		break;
	}
}

static void dwc3_dev_ep0_stdreq(DWC3_DEV_EP0_HANDLER dwc3_dev_ep0_h, USB_STDREQ_CODE eReqCode, void **pNextAddr, u32 *pLen)
{
	DWC3_DEV_HANDLER dwc3_dev_h = dwc3_dev_ep0_h->dwc3_dev_h;
	struct dwc3_dev_config *p_oDevConfig = dwc3_dev_h->p_oDevConfig;
	u32 uCnt;
	DWC3_DEV_EP_HANDLER *hUSB30_EP = (DWC3_DEV_EP_HANDLER *)dwc3_dev_h->p_EPArrayAddress;

	switch (eReqCode) {
	case STANDARD_GET_DESCRIPTOR:
		dwc3_dev_ep0_get_desc_req(dwc3_dev_ep0_h,
					  (USB_STDDESC_TYPE)((dwc3_dev_ep0_h->m_pDeviceRequest->wValue & 0xff00) >> 8),
					  pNextAddr, pLen);
		break;
	case STANDARD_SET_ADDRESS:
		U3DBG_STDREQ("Set Address : %d \n", dwc3_dev_ep0_h->m_pDeviceRequest->wValue);
		_dwc3_dev_setusbaddr(dwc3_dev_h, dwc3_dev_ep0_h->m_pDeviceRequest->wValue);
		// When Status Stage complete, Device should change its usb address sended from Host
		//USB3_DEV_CtrlXfer_SetFuncAtTransferComplete(_dwc3_dev_setusbaddr, (u32)dwc3_dev_ep0_h->m_pDeviceRequest->wValue_L);
		//dwc3_dev_ep0_h->m_ucCurrentConfigurationNum = 0;
		break;
	case STANDARD_SET_CONFIGURATION:
		/* TODO: Notiy to gadget to configured state and call back */
		U3DBG_STDREQ("Set Configuration \n");

		for (uCnt = 1; uCnt < DWC3_DEV_TOTAL_EP_CNT; uCnt++) {
			if (hUSB30_EP[uCnt]->eState != DEV30_EP_STATE_INIT)
				dwc3_dev_ep_deinit(hUSB30_EP[uCnt]);
		}
		if (dwc3_dev_ep0_h->m_pDeviceRequest->wValue == 0) {
			U3DBG_STDREQ("Clear Current Configuration\n");
			gadget_chg_state(USB_DEV_STATE_ADDRESSED);
		} else {
			// EP Re-Configuration Command
			dwc3_dev_ep_cmd(dwc3_dev_ep0_h->m_hEP, DEPCMD_StartNewCfg, 2, 0);
			gadget_chg_state(USB_DEV_STATE_CONFIGURED);
			/* Notify to class layer to config endpoint */
			notify_config_to_class(dwc3_dev_h->m_eEnumeratedSpeed, -1, 0);
#if 0
			u8 ucIFLoopCnt, ucEpLoopCnt;
			u8 ucIFNum = UDEV_GetNumOfIF(dwc3_dev_h->udev_p);

			for (ucIFLoopCnt = 0; ucIFLoopCnt < ucIFNum; ucIFLoopCnt++) {
				u8 ucEpNum = UDEV_GetNumOfEPEachSet(dwc3_dev_h->udev_p, ucIFLoopCnt, 0);
				for (ucEpLoopCnt = 0; ucEpLoopCnt < ucEpNum; ucEpLoopCnt++) {
					USB_ENDPOINT_DESCRIPTOR *pEpDesc;
					USB_EPCOMP_DESCRIPTOR *p_oEpComp = NULL;

					pEpDesc = UDEV_GetDesc_EP(udev,
								  ucIFLoopCnt,
								  0,
								  ucEpLoopCnt);
					if (dwc3_dev_get_conn_speed(dwc3_dev_h) == USBSPEED_SUPERPLUS) {
						int ep_addr;

						ep_addr = pEpDesc->bEndpointAddress & 0x7f;
						p_oEpComp = UDEV_GetDesc_EPCOMP(udev,
										ucIFLoopCnt,
										0,
										ep_addr);
					} else if (dwc3_dev_get_conn_speed(dwc3_dev_h) == USBSPEED_SUPER) {
						int ep_addr;

						ep_addr = pEpDesc->bEndpointAddress & 0x7f;
						p_oEpComp = UDEV_GetDesc_EPCOMP(udev,
										ucIFLoopCnt,
										0,
										ep_addr);
					}

					USB3_DEV_EP_InitByDesc(dwc3_dev_h, pEpDesc, p_oEpComp);
				}
			}
			// Notify to Device Platform when Set-configuration transfer is done
			usb30_dev_ep0_set_cb_ctrl_xfer(dwc3_dev_ep0_h,
						       UDEV_RunEnumDoneFunc,
						       (void *) dwc3_dev_h->udev_p, 0);
			/* Strat debugging thread */
			usb30_dev_glue_exit_start_debug_trace(dwc3_dev_h);
#endif
		}
		break;
	case STANDARD_GET_CONFIGURATION:
		/* Check current status is configured, and get currnet configuration value */
		// When Device State is Configure state, USB sends current configuration number
#if 0
		if (gadget_get_state() == USB_DEV_STATE_CONFIGURED)
			*pNextAddr = 1;
#else
		dwc3_dev_ep0_req_stall(dwc3_dev_ep0_h);
#endif
		break;
	case STANDARD_SET_INTERFACE: {
		/* TODO: Find Current Interface and alternate set number from gadget */
		if (gadget_get_state() != USB_DEV_STATE_CONFIGURED)
			dwc3_dev_ep0_req_stall(dwc3_dev_ep0_h);
		else {
			notify_config_to_class(dwc3_dev_h->m_eEnumeratedSpeed,
					LO_BYTE(dwc3_dev_ep0_h->m_pDeviceRequest->wIndex),
					LO_BYTE(dwc3_dev_ep0_h->m_pDeviceRequest->wValue));
		}
#if 0
		u8 ucIFNum = dwc3_dev_ep0_h->m_pDeviceRequest->wIndex & 0xff;
		u8 ucSetNum = dwc3_dev_ep0_h->m_pDeviceRequest->wValue & 0xff;
		USB_DESC_INTERFACE *p_oIFDesc = UDEV_GetDesc_IF(dwc3_dev_h->udev_p, ucIFNum, ucSetNum);

		if (UDEV_IsDevState(dwc3_dev_h->udev_p) == UDEV_STATE_CONFIGURED && p_oIFDesc) {
			if (p_oIFDesc->bNumEndpoints) {
				for (uCnt = 0; uCnt < p_oIFDesc->bNumEndpoints; uCnt++) {
					USB_ENDPOINT_DESCRIPTOR *pEpDesc = UDEV_GetDesc_EP(dwc3_dev_h->udev_p, ucIFNum,
													   ucSetNum, uCnt);
					USB_EPCOMP_DESCRIPTOR *p_oEpComp = NULL;
					if (dwc3_dev_get_conn_speed(dwc3_dev_h) == USBSPEED_SUPERPLUS) {
						int ep_addr;

						ep_addr = pEpDesc->bEndpointAddress & ~0x7f;
						p_oEpComp = UDEV_GetDesc_EPCOMP(dwc3_dev_h->udev_p, ucIFNum, ucSetNum, ep_addr);
					} else if (dwc3_dev_get_conn_speed(dwc3_dev_h) == USBSPEED_SUPER) {
						int ep_addr;

						ep_addr = pEpDesc->bEndpointAddress & ~0x7f;
						p_oEpComp = UDEV_GetDesc_EPCOMP(dwc3_dev_h->udev_p, ucIFNum, ucSetNum, ep_addr);
					}

					USB3_DEV_EP_InitByDesc(dwc3_dev_h, pEpDesc, p_oEpComp);
				}
			} else {
				// NO EP Desc Criptor : De Init EP
				/* TODO: Find Current Interface set number */
#if 0
				u8 ucGetCurrentIFSet = UDEV_GetCurrentIFSet(dwc3_dev_h->udev_p, ucIFNum);
				USB_DESC_INTERFACE *p_oLastIF = UDEV_GetDesc_IF(dwc3_dev_h->udev_p, ucIFNum,
												ucGetCurrentIFSet);

				for (uCnt = 0; uCnt < p_oLastIF->bNumEndpoints; uCnt++) {
					/* TODO: Find Endpoint descriptor */
#if 0
					USB_ENDPOINT_DESCRIPTOR *pEpDesc = UDEV_GetDesc_EP(dwc3_dev_h->udev_p, ucIFNum,
													   ucGetCurrentIFSet, uCnt);
					DWC3_DEV_EP_HANDLER hEP = hUSB30_EP[pEpDesc->bEndpointAddress & 0xf];
					if (hEP->eState & DEV30_EP_STATE_RUN_XFER) {
						dwc3_dev_ep_cmd(hEP, DEPCMD_EndXfer, 0, 0);
						hEP->eState &= ~DEV30_EP_STATE_RUN_XFER;
#endif
					}
				}
#endif
			}
			/* TODO: Notify to gadget layer alternate set number */
#if 0
			UDEV_SetIFAltSetNumber(dwc3_dev_h->udev_p, ucIFNum, ucSetNum);
#endif
			/*TODO: Register Callback for gadget layer */
#if 0
			usb30_dev_ep0_set_cb_ctrl_xfer(dwc3_dev_ep0_h, UDF_CallBack_SetIfReq,
					(void *) dwc3_dev_h->udev_p, ((ucIFNum << 16) | (ucSetNum)));
#endif
			dwc3_dev_ep0_h->AlternateSetting =
				(u8)(dwc3_dev_ep0_h->m_pDeviceRequest->wValue & 0xff);
		} else
			// Support only Configured State
			dwc3_dev_ep0_req_stall(dwc3_dev_ep0_h);
#endif
	}
	break;
	case STANDARD_GET_INTERFACE: {
#if (0)
		u8 ucIFNum = dwc3_dev_ep0_h->m_pDeviceRequest->wIndex_L;
		USB_DESC_INTERFACE *p_oIFDesc = UDF_GetDesc_IFCurrentSet(ucIFNum);
		if (UDF_IsDevState() == USBDEV_STATE_CONFIGURED)
			*pNextAddr = (u32)(&dwc3_dev_ep0_h->AlternateSetting);
		else
			dwc3_dev_ep0_req_stall(dwc3_dev_ep0_h);
		*pNextAddr = (void *)(&dwc3_dev_ep0_h->AlternateSetting);
#else
		dwc3_dev_ep0_req_stall(dwc3_dev_ep0_h);
#endif // 0

	}
	break;
	case STANDARD_SET_DESCRIPTOR:
		// Set Desc from Host :: Must be Not Supported current Spec
		U3DBG_STDREQ("MCU >> Set Descriptor \n");
		dwc3_dev_ep0_req_stall(dwc3_dev_ep0_h);
		break;

	case STANDARD_CLEAR_FEATURE:
		U3DBG_STDREQ("MCU >> Clear Feature \n");
		switch ((USB_STDREQ_RECIPIENT) dwc3_dev_ep0_h->m_pDeviceRequest->bmRequestType) {
		case DEVICE_RECIPIENT:
			switch ((USB_STDREQ_FEATURE)(dwc3_dev_ep0_h->m_pDeviceRequest->wValue & 0xff)) {
			case REMOTEWAKEUP_ENABLE:
				dwc3_dev_ep0_h->m_ucDeviceSelector &= ~(1 << 1);
				break;
			case U1_ENABLE:
				if (p_oDevConfig->m_bU1Support) {
					USB3_REG_DCTL_o DCTL;
					DCTL.data = DWC3_REG_RD32(rDCTL);
					DCTL.b.InitU1Ena = false;
					DCTL.b.AcceptU1Ena = false;
					DWC3_REG_WR32(rDCTL, DCTL.data);
					dwc3_dev_ep0_h->m_ucDeviceSelector &= ~(1 << 2);
					U3DBG_STDREQ("U1 false\n");
				} else {
					dwc3_dev_ep0_req_stall(dwc3_dev_ep0_h);
					U3DBG_STDREQ("U1 is not Supported!!\n");
				}
				break;
			case U2_ENABLE:
				if (p_oDevConfig->m_bU2Support) {
					USB3_REG_DCTL_o DCTL;
					DCTL.data = DWC3_REG_RD32(rDCTL);
					DCTL.b.InitU2Ena = false;
					DCTL.b.AcceptU2Ena = false;
					DWC3_REG_WR32(rDCTL, DCTL.data);
					dwc3_dev_ep0_h->m_ucDeviceSelector &= ~(1 << 3);
					U3DBG_STDREQ("U2 false\n");
				} else {
					dwc3_dev_ep0_req_stall(dwc3_dev_ep0_h);
					U3DBG_STDREQ("U2 is not Supported!!\n");
				}
				break;
			case LTM_ENABLE:
				if (p_oDevConfig->m_bLTMSupport) {
					// TODO: Disable LTM Fuction
					U3DBG_STDREQ("LTM disable\n");
					dwc3_dev_ep0_h->m_ucDeviceSelector &= ~(1 << 4);
				} else {
					U3DBG_STDREQ("LTM is not supported\n");
					dwc3_dev_ep0_req_stall(dwc3_dev_ep0_h);
				}
				break;
			default:
				dwc3_dev_ep0_req_stall(dwc3_dev_ep0_h);
				U3DBG_STDREQ("Undefined Feature\n");
				break;
			}
			break;
		case ENDPOINT_RECIPIENT:
			if (dwc3_dev_ep0_h->m_pDeviceRequest->wValue == 0) {
				u32 uEpNum = dwc3_dev_ep0_h->m_pDeviceRequest->wIndex & 0x7f;
				U3DBG_STDREQ("EP%d Clear Halt\n", uEpNum);
				dwc3_dev_h->m_bEPHalted &= ~(1 << uEpNum);
				// TODO: Reset EP for Sequencing
	                        uEpNum = uEpNum << 1;
				if (dwc3_dev_ep0_h->m_pDeviceRequest->wIndex & 0x80)
                                      uEpNum += 1;
				dwc3_dev_ep_cmd(hUSB30_EP[uEpNum], DEPCMD_ClrStall, 0, 0);
			}
			break;
		case INTERFACE_RECIPIENT:
			break;
		default:
			break;
		}
		break;
	case STANDARD_SET_FEATURE:
		U3DBG_STDREQ("Set Feature \n");
		/* TODO: Check Address state from gadget layer */
		if (gadget_get_state() == USB_DEV_STATE_ADDRESSED) {
			U3DBG_STDREQ("Address State in not Supported\n");
			dwc3_dev_ep0_req_stall(dwc3_dev_ep0_h);
		} else {
			switch ((USB_STDREQ_RECIPIENT) dwc3_dev_ep0_h->m_pDeviceRequest->bmRequestType) {
			case DEVICE_RECIPIENT:
				switch ((USB_STDREQ_FEATURE)(dwc3_dev_ep0_h->m_pDeviceRequest->wValue & 0xff)) {
				case REMOTEWAKEUP_ENABLE:
					dwc3_dev_ep0_h->m_ucDeviceSelector |= (1 << 1);
					break;
				case TEST_MODE_ENABLE:
					// USB Device Must be send ACK ZLP for status stage.
					// After ACK Sequence, Device enters to the Test Mode.
					// USB Link does not transfered data from memory, when receive Test Mode Command
					dwc3_dev_ep0_set_cb_at_xfercmpl(dwc3_dev_ep0_h,
									dwc3_dev_SetTestMode,
									(dwc3_dev_ep0_h->m_pDeviceRequest->wIndex & 0xff00) >> 8);
					break;
				case U1_ENABLE:
					if (p_oDevConfig->m_bU1Support) {
						USB3_REG_DCTL_o DCTL;
						DCTL.data = DWC3_REG_RD32(rDCTL);
						DCTL.b.InitU1Ena = true;
						DCTL.b.AcceptU1Ena = true;
						DWC3_REG_WR32(rDCTL, DCTL.data);
						dwc3_dev_ep0_h->m_ucDeviceSelector |= (1 << 2);
						U3DBG_STDREQ("U1 true\n");
						//USB3_DEV_CtrlXfer_SetFuncAtTransferComplete(USB3_DEV_SetSystemExitLatency, (u32)(dwc3_dev_ep0_h->m_uSELBuffer));
					} else {
						U3DBG_STDREQ("U1 is not Supported\n");
						dwc3_dev_ep0_req_stall(dwc3_dev_ep0_h);
					}
					break;
				case U2_ENABLE:
					if (p_oDevConfig->m_bU2Support) {
						USB3_REG_DCTL_o DCTL;
						DCTL.data = DWC3_REG_RD32(rDCTL);
						DCTL.b.InitU2Ena = true;
						DCTL.b.AcceptU2Ena = true;
						DWC3_REG_WR32(rDCTL, DCTL.data);
						dwc3_dev_ep0_h->m_ucDeviceSelector |= (1 << 3);
						U3DBG_STDREQ("U2 true\n");
					} else {
						U3DBG_STDREQ("U2 is not Supported\n");
						dwc3_dev_ep0_req_stall(dwc3_dev_ep0_h);
					}
					break;
				case LTM_ENABLE:
					if (p_oDevConfig->m_bLTMSupport) {
						// TODO: Enable LTM Function
						U3DBG_STDREQ("LTM Enable\n");
						dwc3_dev_ep0_h->m_ucDeviceSelector |= (1 << 4);
					} else {
						U3DBG_STDREQ("LTM is not Supported\n");
						dwc3_dev_ep0_req_stall(dwc3_dev_ep0_h);
					}
					break;
				default:
					U3DBG_STDREQ("Undefined Feature\n");
					dwc3_dev_ep0_req_stall(dwc3_dev_ep0_h);
					break;
				}
				break;
			case ENDPOINT_RECIPIENT:
				if (dwc3_dev_ep0_h->m_pDeviceRequest->wValue == ENDPOINT_HALT) {
					u32 uEpNum = dwc3_dev_ep0_h->m_pDeviceRequest->wIndex & 0x7f;
					U3DBG_STDREQ("Set EP%d Halt\n", uEpNum);
					dwc3_dev_h->m_bEPHalted |= (1 << uEpNum);
				} else {
					U3DBG_STDREQ("Undefined Feature\n");
					dwc3_dev_ep0_req_stall(dwc3_dev_ep0_h);
				}
				break;
			case INTERFACE_RECIPIENT:
				// Will be Added
				break;
			default:
				break;
			}
		}
		break;
	case STANDARD_GET_STATUS:
		switch ((USB_STDREQ_RECIPIENT)(dwc3_dev_ep0_h->m_pDeviceRequest->bmRequestType & 0x3)) {
		case DEVICE_RECIPIENT:
			U3DBG_STDREQ("Get Status : Device\n");
			*pNextAddr = (void *)(&(dwc3_dev_ep0_h->m_ucDeviceSelector));
			break;
		case INTERFACE_RECIPIENT:
			U3DBG_STDREQ("Get Status : Interface\n");
			*(u32 *)(dwc3_dev_ep0_h->m_pStatusBuf) = 0;
			*pNextAddr = (void *)(dwc3_dev_ep0_h->m_pStatusBuf);
			break;
		case ENDPOINT_RECIPIENT: {
			u32 uEpNum = dwc3_dev_ep0_h->m_pDeviceRequest->wIndex & 0x7f;
			U3DBG_STDREQ("Get Status : EP %d\n", uEpNum);
			if (dwc3_dev_h->m_bEPHalted & (1 << uEpNum))
				*(u32 *)(dwc3_dev_ep0_h->m_pStatusBuf) = 1;
			else
				*(u32 *)(dwc3_dev_ep0_h->m_pStatusBuf) = 0;
			*pNextAddr = (void *)(dwc3_dev_ep0_h->m_pStatusBuf);
			break;
		}
		default:
			break;
		}
		break;
	case STANDARD_SET_SEL:
		U3DBG_STDREQ("Standard Req : SET SEL\n");
		*pNextAddr = (void *)(dwc3_dev_ep0_h->m_uSELBuffer);
		dwc3_dev_ep0_set_cb_at_xfercmpl(dwc3_dev_ep0_h,
						dwc3_dev_SetSystemExitLatency,
						(unsigned long)(dwc3_dev_ep0_h->m_uSELBuffer));
		break;
	case STANDARD_SYNCH_FRAME:
		U3DBG_STDREQ("Standard Req : Sync Frame\n");
		break;
	case STANDARD_ISOCH_DELY:
		U3DBG_STDREQ("Standard Req : ISOCH Delay\n");
		break;
	default:
		U3DBG_STDREQ("Error : This Request(%d) is not implemented yet\n",
					 dwc3_dev_ep0_h->m_pDeviceRequest->bRequest);
		dwc3_dev_ep0_req_stall(dwc3_dev_ep0_h);
		break;
	}
}

static u8 dwc3_dev_ep0_datastage(DWC3_DEV_EP0_HANDLER dwc3_dev_ep0_h, void **pNextAddr, u32 *pLen)
{
	u8 bRet = true;
	int ret;

	// Find length requested data and decide whether control xfer is 2 stage or 3 stage
	*(u32 *)(dwc3_dev_ep0_h->m_pStatusBuf) = 0;
	*pNextAddr = (void *)(dwc3_dev_ep0_h->m_pStatusBuf);
	*pLen = (u32) dwc3_dev_ep0_h->m_pDeviceRequest->wLength;
	U3DBG_STDREQ("Req Length %d\n", *pLen);
	// Select Request type
	switch ((USB_STDREQ_TYPE)(dwc3_dev_ep0_h->m_pDeviceRequest->bmRequestType & 0x60)) {
	case STANDARD_TYPE:
		dwc3_dev_ep0_stdreq(dwc3_dev_ep0_h,
				    (USB_STDREQ_CODE) dwc3_dev_ep0_h->m_pDeviceRequest->bRequest,
				    pNextAddr, pLen);
		break;
	case CLASS_TYPE:
	case VENDOR_TYPE:
		ret = gadget_call_cb_class_std_req(dwc3_dev_ep0_h->m_pDeviceRequest, pNextAddr, pLen);
		if (ret == USB_STDREQ_ERROR)
			dwc3_dev_ep0_req_stall(dwc3_dev_ep0_h);
		break;
	default:
		U3DBG_STDREQ("0x%02x Type Request is not supported yet\n",
					 dwc3_dev_ep0_h->m_pDeviceRequest->bmRequestType & 0x60);
		dwc3_dev_ep0_req_stall(dwc3_dev_ep0_h);
		break;
	}
	// If control transfer is 2 stage, it must be wait not ready event to transit status stage
	if ((*pLen == 0)) {
		dwc3_dev_ep0_h->m_eNextEvt = DEPEVT_EVT_XFER_NRDY;
		dwc3_dev_ep0_h->m_hEP->m_eDir = (USB_DIR)(~((u8) dwc3_dev_ep0_h->m_hEP->m_eDir) & 0x1);
		bRet = false;
	}
	return bRet;
}

static void dwc3_dev_ep0_init(DWC3_DEV_HANDLER dwc3_dev_h)
{
	DWC3_DEV_EP0_HANDLER dwc3_dev_ep0_h = (DWC3_DEV_EP0_HANDLER)dwc3_dev_h->p_EP0Address;
	DWC3_DEV_EP_HANDLER *hUSB30_EP = (DWC3_DEV_EP_HANDLER *)dwc3_dev_h->p_EPArrayAddress;
	u32 low_trb_addr, high_trb_addr;
	void *trb;

	if (!dwc3_dev_ep0_h->dwc3_dev_h) {
		dwc3_dev_ep_init(dwc3_dev_h, 0, USBDIR_OUT, USBEP_CTRL, 0, 0);
		dwc3_dev_ep0_h->dwc3_dev_h = dwc3_dev_h;
		dwc3_dev_ep0_h->m_hEP = hUSB30_EP[0];
	} else {
		dwc3_dev_ep_cmd(dwc3_dev_ep0_h->m_hEP, DEPCMD_EndXfer, 0, 0);
		dwc3_dev_trb_release(dwc3_dev_ep0_h->m_hEP->trb_h, USBDIR_OUT);
	}
	/* Set Default Direction : OUT */
	dwc3_dev_ep0_h->m_hEP->m_eDir = USBDIR_OUT;
	/* Set Initial States for States Machine */
	dwc3_dev_ep0_h->m_eNextCtrlPkt = TRB_CTRL_SETUP;
	dwc3_dev_ep0_h->fn_NextControlXfer = dwc3_dev_ep0_datastage;
	dwc3_dev_ep0_h->m_eNextEvt = DEPEVT_EVT_XFER_CMPL;
	/* Start to receive trasfer to set-up packet */
	dwc3_dev_trb_set(dwc3_dev_ep0_h->m_hEP->trb_h, (void *) dwc3_dev_ep0_h->m_pDeviceRequest,
					  dwc3_dev_ep0_h->m_hEP->m_uMaxPktSize, TRB_CTRL_SETUP, TRB_LAST_INTR_EN);
	trb = usb3_dev_trb_prepare(dwc3_dev_ep0_h->m_hEP->trb_h, true, USBDIR_OUT, false);
	if (sizeof(void *) > 4) {
		low_trb_addr =
			(unsigned long) ((u64) trb & 0xffffffff);
		high_trb_addr =
			(unsigned long) (((u64) trb >> 32) & 0xffffffff);
	} else {
		low_trb_addr = (unsigned long) trb;
		high_trb_addr = 0x0;
	}
	dwc3_dev_ep_cmd(dwc3_dev_ep0_h->m_hEP, DEPCMD_StartXfer, high_trb_addr, low_trb_addr);
}

static void DWC3_DEV_EP0_ChgStage(DWC3_DEV_HANDLER dwc3_dev_h, void *pAddr, u32 uLen)
{
	DWC3_DEV_EP0_HANDLER dwc3_dev_ep0_h = (DWC3_DEV_EP0_HANDLER)dwc3_dev_h->p_EP0Address;

	U3DBG_ISR_EP0("Befor State %d, %d\n",
		      dwc3_dev_ep0_h->m_eNextCtrlPkt, dwc3_dev_ep0_h->m_hEP->m_eDir);

	if (dwc3_dev_h->m_bEPHalted & (1 << 0)) {
		//Check Stall Condition
		U3DBG_STDREQ("EP0 Must be responsed STALL\n");
		dwc3_dev_ep0_h->m_hEP->m_eDir = USBDIR_OUT;
		dwc3_dev_ep_cmd(dwc3_dev_ep0_h->m_hEP, DEPCMD_SetStall, 0, 0);
		dwc3_dev_h->m_bEPHalted &= ~(1 << 0);
		dwc3_dev_ep0_init(dwc3_dev_h);
	} else {
		USB_DIR *pNextDir = &(dwc3_dev_ep0_h->m_hEP->m_eDir);
		switch (dwc3_dev_ep0_h->m_eNextCtrlPkt) {
		case TRB_CTRL_SETUP:
			if (uLen) {
				U3DBG_STDREQ("Data Stage\n");
				dwc3_dev_ep0_h->m_eNextEvt = DEPEVT_EVT_XFER_CMPL;
				dwc3_dev_ep0_h->m_eNextCtrlPkt = TRB_CTRL_CTLDATA_1ST;
				dwc3_dev_ep0_h->fn_NextControlXfer = DWC3_DEV_EP0_StatusStage;
				if (dwc3_dev_ep0_h->m_pDeviceRequest->bmRequestType & DEVICE_TO_HOST) {
					U3DBG_STDREQ("Dev to Host\n");
					*pNextDir = USBDIR_IN;
				} else {
					u32 uMaxPkt = dwc3_dev_ep0_h->m_hEP->m_uMaxPktSize;
					U3DBG_STDREQ("Host to Dev\n");
					*pNextDir = USBDIR_OUT;
					if (uLen % uMaxPkt) {
						U3DBG_STDREQ("Control Out Packet size must be Muliple EP MaxPktSize\n");
						U3DBG_STDREQ("Change User Set [%d]byte to ", uLen);
						uLen = ((uLen / uMaxPkt) + 1) * uMaxPkt;
						U3DBG_STDREQ("[%d]byte\n", uLen);
					}
				}
			} else {
				U3DBG_STDREQ("Status 2\n");
				U3DBG_STDREQ("EP IN \n");
				dwc3_dev_ep0_h->m_eNextEvt = DEPEVT_EVT_XFER_CMPL;
				dwc3_dev_ep0_h->m_eNextCtrlPkt = TRB_CTRL_STATUS_2;
				dwc3_dev_ep0_h->fn_NextControlXfer = dwc3_dev_ep0_SetUpStage;
			}
			break;
		case TRB_CTRL_CTLDATA_1ST:
		case TRB_CTRL_NORMAL:
			U3DBG_STDREQ("Status 3\n");
			dwc3_dev_ep0_h->m_eNextEvt = DEPEVT_EVT_XFER_CMPL;
			dwc3_dev_ep0_h->m_eNextCtrlPkt = TRB_CTRL_STATUS_3;
			dwc3_dev_ep0_h->fn_NextControlXfer = dwc3_dev_ep0_SetUpStage;
			break;
		default:
		case TRB_CTRL_STATUS_2:
		case TRB_CTRL_STATUS_3:
			U3DBG_STDREQ("Set Up\n");
			dwc3_dev_ep0_h->m_eNextEvt = DEPEVT_EVT_XFER_CMPL;
			dwc3_dev_ep0_h->m_eNextCtrlPkt = TRB_CTRL_SETUP;
			dwc3_dev_ep0_h->fn_NextControlXfer = dwc3_dev_ep0_datastage;
			*pNextDir = USBDIR_OUT;
			break;
		}
		dwc3_dev_trb_set(dwc3_dev_ep0_h->m_hEP->trb_h, pAddr, uLen,
				  dwc3_dev_ep0_h->m_eNextCtrlPkt,
				  TRB_LAST_INTR_EN);

		u32 low_trb_addr, high_trb_addr;
		void *trb;

		trb = usb3_dev_trb_prepare(dwc3_dev_ep0_h->m_hEP->trb_h, true,
				  dwc3_dev_ep0_h->m_hEP->m_eDir, 0);
		if (sizeof(void *) > 4) {
			low_trb_addr =
				(unsigned long) ((u64) trb & 0xffffffff);
			high_trb_addr =
				(unsigned long) (((u64) trb >> 32) & 0xffffffff);
		} else {
			low_trb_addr = (unsigned long) trb;
			high_trb_addr = 0x0;
		}
		dwc3_dev_ep_cmd(dwc3_dev_ep0_h->m_hEP, DEPCMD_StartXfer,
				 high_trb_addr, low_trb_addr);
	}
	U3DBG_ISR_EP0("After State %d, %d\n",
		      dwc3_dev_ep0_h->m_eNextCtrlPkt, dwc3_dev_ep0_h->m_hEP->m_eDir);
}

static void dwc3_dev_ep0_isr(DWC3_DEV_HANDLER dwc3_dev_h, DWC3_DEV_DEPEVT_o *uEpEvent)
{
	DWC3_DEV_EP0_HANDLER dwc3_dev_ep0_h = (DWC3_DEV_EP0_HANDLER) dwc3_dev_h->p_EP0Address;
	static void *uNextBufferAddr;
	static u32 uNextBufferLen;

	USB_DIR eDir = (USB_DIR)(uEpEvent->b.ep_num & 1);
	USB_DIR eLastDir = dwc3_dev_ep0_h->m_hEP->m_eDir;

	USB3_DEV_EPEVT_TYPE_e eEPEvtType =
		(USB3_DEV_EPEVT_TYPE_e)(uEpEvent->b.evt_type);
	USB3_DEV_EPEVT_STATUS_NRDY_e eNrdyEvtSts =
		(USB3_DEV_EPEVT_STATUS_NRDY_e)(uEpEvent->b.evnt_sts & 0x3);
	USB3_DEV_TRB_p pLastTRB = dwc3_dev_trb_pop_last(dwc3_dev_ep0_h->m_hEP->trb_h, eLastDir);
	USB3_DEV_TRB_STATUS_e eTRBStatus = (USB3_DEV_TRB_STATUS_e) (pLastTRB->status.b.trb_sts);


	if (eDir)
		U3DBG_ISR_EP0_S("I");
	else
		U3DBG_ISR_EP0_S("O");
	if (uEpEvent->b.evt_type == DEPEVT_EVT_XFER_NRDY)
		U3DBG_ISR_EP0_S("N");
	else if (uEpEvent->b.evt_type == DEPEVT_EVT_XFER_CMPL)
		U3DBG_ISR_EP0_S("C");
	if (eTRBStatus == 2)
		U3DBG_ISR_EP0_S("#");

	if (dwc3_dev_ep0_h->m_eNextEvt == DEPEVT_EVT_XFER_NRDY) {
		U3DBG_ISR_EP0("[EP0]Wait Event : NRDY\n");
	} else if (dwc3_dev_ep0_h->m_eNextEvt == DEPEVT_EVT_XFER_CMPL) {
		U3DBG_ISR_EP0("[EP0]Wait Event : CMPL\n");
	}

	if (eTRBStatus == TRB_STATUS_SetUpPending) {
		/* && (dwc3_dev_ep0_h->m_eNextCtrlPkt == TRB_CTRL_CTLDATA_1ST)
		 * && !(uEpEvent->b.evnt_sts) ) */

		/* During Current Control Transfer data/status phase,
		 * anodhter Set-UP Was Received  */
		U3DBG_ISR_EP0("\n\nOTL %d, 0x%x, 0x%x\n\n", eTRBStatus,
			      uEpEvent->b.evnt_sts, uEpEvent->b.evt_type);
		// TODO: What can I do when Back-to-Back Setup Packet??
	}

	if ((dwc3_dev_ep0_h->m_eNextEvt == eEPEvtType) && (eLastDir == eDir)) {
		switch (eEPEvtType) {
		case DEPEVT_EVT_XFER_CMPL:
			U3DBG_ISR_EP0_S("M");
			U3DBG_ISR_EP0("[EP0]Run Event : CMPL\n");
			dwc3_dev_trb_release(dwc3_dev_ep0_h->m_hEP->trb_h,
					      eLastDir);
			if (dwc3_dev_ep0_h->fn_NextControlXfer(dwc3_dev_ep0_h,
							   &uNextBufferAddr,
							   &uNextBufferLen)) {
				DWC3_DEV_EP0_ChgStage(dwc3_dev_h,
						      uNextBufferAddr,
						      uNextBufferLen);
			}
			break;
		case DEPEVT_EVT_XFER_NRDY:
			U3DBG_ISR_EP0_S(  "R");
			U3DBG_ISR_EP0("[EP0]Run Event : NRDY\n");
			if (eNrdyEvtSts == DEPEVT_STS_NRDY_DATA) {
				if (eTRBStatus != TRB_STATUS_SetUpPending)
					dwc3_dev_ep0_req_stall(dwc3_dev_ep0_h);
				else {
					dwc3_dev_ep0_h->m_eNextCtrlPkt =
						TRB_CTRL_CTLDATA_1ST;
					dwc3_dev_ep0_datastage(dwc3_dev_ep0_h,
							       &uNextBufferAddr,
							       &uNextBufferLen);
				}
			} else {
				DWC3_DEV_EP0_ChgStage(dwc3_dev_h,
						      uNextBufferAddr,
						      uNextBufferLen);
			}
			break;
		case DEPEVT_EVT_XFER_IN_PROG:
			U3DBG_ISR_EP0("[EP0]Current Evt : In Progress\n");
			break;
		case DEPEVT_EVT_FIFOXRUN:
			U3DBG_ISR_EP0("[EP0]\n\n$$\n\n");
			break;
		case DEPEVT_EVT_STRM_EVT:
			break;
		case DEPEVT_EVT_EPCMD_CMPL:
			break;
		default:
			U3DBG_ISR_EP0("[EP0]Unknown event!\n");
			break;
		}
	} else if (dwc3_dev_ep0_h->m_eNextEvt != eEPEvtType) {
		if ((dwc3_dev_ep0_h->m_hEP->m_eDir != eDir) ||
				(dwc3_dev_ep0_h->m_eNextCtrlPkt == TRB_CTRL_SETUP) || 0) {
			U3DBG_ISR_EP0_S("X");
			U3DBG_ISR_EP0_S("%d, %d, %d, %d \n",
					dwc3_dev_ep0_h->m_eNextEvt,
					dwc3_dev_ep0_h->m_eNextCtrlPkt,
					dwc3_dev_ep0_h->m_hEP->m_eDir, eNrdyEvtSts);
			U3DBG_ISR_EP0_S("%d, %d, %d,  0x%x\n",
					uEpEvent->b.evt_type,
					eDir, eTRBStatus, uEpEvent->b.evnt_sts);
			if ((dwc3_dev_ep0_h->m_eNextCtrlPkt == TRB_CTRL_SETUP) &&
					(eEPEvtType == DEPEVT_EVT_XFER_NRDY)) {
				if ((eNrdyEvtSts == DEPEVT_STS_NRDY_DATA) ||
						(eNrdyEvtSts == DEPEVT_STS_NRDY_STATUS)) {
					U3DBG_ISR_EP0_S("\n^^@@^^ : %d\n",
							eNrdyEvtSts);
					dwc3_dev_ep0_req_stall(dwc3_dev_ep0_h);
					DWC3_DEV_EP0_ChgStage(dwc3_dev_h,
							      uNextBufferAddr,
							      uNextBufferLen);
				}
			} else if (dwc3_dev_ep0_h->m_hEP->m_eDir != eDir) {
				U3DBG_ISR_EP0_S("\n^^!!^^\n");
				dwc3_dev_ep0_req_stall(dwc3_dev_ep0_h);
				DWC3_DEV_EP0_ChgStage(dwc3_dev_h,
						      uNextBufferAddr,
						      uNextBufferLen);
			}
		}
	}
}
