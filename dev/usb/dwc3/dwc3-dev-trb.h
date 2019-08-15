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

#define TRB_BLOCK_MAX		4		// org value : 16
#define TRB_BUF_SIZ_LIMIT	16777215	//2^24 - 1 (16MB -1byte)

typedef enum {
	TRB_CTRL_NORMAL = 1,		// Control-Data-2+ / bulk / Interrupt
	TRB_CTRL_SETUP = 2,		// Control-Setup
	TRB_CTRL_STATUS_2 = 3,
	TRB_CTRL_STATUS_3 = 4,
	TRB_CTRL_CTLDATA_1ST = 5,	// 1st TRB of Data stage
	TRB_CTRL_ISOC_1ST = 6,		// 1st TRB of Service Interval
	TRB_CTRL_ISOC = 7,
	TRB_CTRL_LINK = 8,		// Link TRB
} USB3_DEV_TRB_TYPE_e;

typedef enum {
	TRB_STATUS_OK = 0,		// Transfer OK
	TRB_STATUS_MISSED_ISOC = 1,	// ISO Interval missed
	TRB_STATUS_SetUpPending = 2,	// Durring Current Control transfer data/status phase, another Set-Up Was Received
} USB3_DEV_TRB_STATUS_e;

typedef enum {
	//TRB_BUFFER_CACHED = (0x1 << 31),
	TRB_NO_OPTION = 0,
	/* make circular buffer */
	TRB_CIRCULAR_RING = (0x10 << 16),
	// Last and Interrrupt Enable Option
	TRB_LAST = (0x8 << 16),
	TRB_INTR_EN = (0x4 << 16),
	TRB_LAST_INTR_EN = TRB_LAST | TRB_INTR_EN,
	TRB_LAST_INTR_CIRCULAR = TRB_LAST_INTR_EN | TRB_CIRCULAR_RING,
	// Cache Operation
	TRB_NONCACHED = (0x2 << 16),
} USB3_DEV_TRB_OPTION;

typedef struct {
	unsigned buf_siz :24;	// bit[23:0] : buffer size
	unsigned pkt_cnt_m1 :2;	// bit[25:24] : packet count minus 1
	unsigned rsvd27_26 :2;	// bit[27:26]
	USB3_DEV_TRB_STATUS_e trb_sts : 4;	// bit[31:28] : TRB status
} USB3_DEV_TRB_STS_BitField;

typedef union {
	u32 data;
	USB3_DEV_TRB_STS_BitField b;
} USB3_DEV_TRB_STS_o;

// control field of TRB

typedef struct {
	unsigned hwo :1;		// bit[0] : h/w owner of descriptor
	unsigned lst :1;		// bit[1] : last TRB
	unsigned chn :1;		// bit[2] : chain buffers
	unsigned csp :1;		// bit[3] : continue on short packet
	unsigned trb_ctrl :6;		// bit[9:4] : TRB control
	unsigned isp_imi :1;		// bit[10] : interrupt on short packet/ interrupt on missed ISOC
	unsigned ioc :1;		// bit[11] : interrupt on complete
	unsigned rsvd13_12 :2;		// bit[13:12]
	unsigned strmid_sofn :16;		// bit[29:14] : stream ID/ SOF #
	unsigned rsvd31_30 :2;		// bit[31:30]
} USB3_DEV_TRB_CTRL_BitField;

typedef union {
	u32 data;
	USB3_DEV_TRB_CTRL_BitField b;
} USB3_DEV_TRB_CTRL_o;

// TRB structure
typedef struct {
	u32 buf_ptr_l; // buffer pointer low
	u32 buf_ptr_h; // buffer pointer high
	USB3_DEV_TRB_STS_o status;
	USB3_DEV_TRB_CTRL_o control;
} USB3_DEV_TRB_o, *USB3_DEV_TRB_p;

typedef struct _usb3_dev_trb_block_o {
	USB3_DEV_TRB_o m_a_oTRB[TRB_BLOCK_MAX];
	u32 m_a_uXferSize[TRB_BLOCK_MAX];
	u32 input_cnt;
	struct _usb3_dev_trb_block_o *m_p_oNext;

	u8 m_bCacheCleand;
} *USB3_DEV_TRB_BLOCK_p;

typedef struct {
	USB3_DEV_TRB_BLOCK_p m_p_oStartTRBBlock;
	u32 m_uCurrentBlock;
	s32 m_sNumBlock;

	USB3_DEV_TRB_p m_p_oLastTRB;

	u32 m_uLastBlockCnt;
	u32 m_uTRBUsageCnt;

	u8 m_bCachedXfer;
	bool non_cachable;
	bool on_demand;

} _usb3_dev_trb_handler, *USB3_DEV_TRB_HANDLER;

#include <stddef.h>

USB3_DEV_TRB_BLOCK_p dwc3_dev_trb_make_blk(void)
{
	USB3_DEV_TRB_BLOCK_p pRet = dwc3_calloc_align(sizeof(struct _usb3_dev_trb_block_o), 64);
	pRet->input_cnt = 0;
	pRet->m_p_oNext = NULL;
	return pRet;
}

u32 __usb3_dev_trb_CompleteOP(USB3_DEV_TRB_HANDLER hTRB, u8 bEnClear, USB_DIR eEpDir)
{
	USB3_DEV_TRB_BLOCK_p trb_block = NULL;
	u32 uRetRealSize = 0;
	USB3_DEV_TRB_p trb;

	for (trb_block = hTRB->m_p_oStartTRBBlock; trb_block; trb_block = trb_block->m_p_oNext) {
		u32 uCnt2 = 0;

		// Cache Invalid
		if (trb_block->m_bCacheCleand && !hTRB->non_cachable) {
			trb_block->m_bCacheCleand = 0;
#if 0
			U3DBG_CACHE("TRB INVALID\n");
			U3DBG_CACHE("ADDR : 0x%p Size : %lud\n", trb_block->m_a_oTRB,
					sizeof(USB3_DEV_TRB_o) * trb_block->input_cnt);
			InvalidateDCache((u64) (trb_block->m_a_oTRB),
					sizeof(USB3_DEV_TRB_o) *
					trb_block->input_cnt);
#endif
		}

		/* Calculate Real Transfer Size and cache operation */
		for (uCnt2 = 0; uCnt2 < trb_block->input_cnt; uCnt2++) {
			trb = &trb_block->m_a_oTRB[uCnt2];
			if (trb->control.b.trb_ctrl == TRB_CTRL_LINK)
				continue;
			uRetRealSize += trb_block->m_a_uXferSize[uCnt2] - trb->status.b.buf_siz;
			if (!(trb->status.b.buf_siz))
				continue;
			if (!hTRB->m_bCachedXfer || hTRB->non_cachable)
				continue;
			if (eEpDir != USBDIR_OUT)
				continue;
#if 0
			if (trb->status.b.buf_siz) {
				U3DBG_CACHE("DATA BUFFER INAVLID\n");
				U3DBG_CACHE("ADDR : 0x%x Size : %d\n",
					    trb->buf_ptr_l,
					    trb->status.b.buf_siz);
				t_val = get_timer(0);
				InvalidateDCache(trb->buf_ptr_l,
						 trb->status.b.buf_siz);
				U3DBG_CACHE("Time : %d\n",
					    ticks2usec(get_timer(t_val)));
			}
#endif
		}

		if (!trb_block->m_p_oNext) {
			trb = &trb_block->m_a_oTRB[0];
			if (trb_block->input_cnt)
				trb += trb_block->input_cnt - 1;
			hTRB->m_p_oLastTRB = trb;
		}

		if (!bEnClear)
			continue;
		// Clear TRBs
		trb_block->input_cnt = 0;
		memset(trb_block->m_a_oTRB, 0x00, sizeof(USB3_DEV_TRB_o) * TRB_BLOCK_MAX);
	}
	return uRetRealSize;
}

USB3_DEV_TRB_p __usb3_dev_trb_GetNext(USB3_DEV_TRB_HANDLER hTRB, u32 uSize)
{
	USB3_DEV_TRB_BLOCK_p p_oCurrentBlock = hTRB->m_p_oStartTRBBlock;
	USB3_DEV_TRB_p p_oRetTRB = NULL;
	u32 uCnt = 1;

	for (; uCnt < hTRB->m_uCurrentBlock; uCnt++)
		p_oCurrentBlock = p_oCurrentBlock->m_p_oNext;

	if (++p_oCurrentBlock->input_cnt == TRB_BLOCK_MAX) {
		// Make New Block and Link It
		USB3_DEV_TRB_BLOCK_p p_oNewBlock = dwc3_dev_trb_make_blk();
		void **tmp_ptr;
		void *tmp;

		// Set Next Chain Buffer
		p_oRetTRB = &p_oCurrentBlock->m_a_oTRB[(TRB_BLOCK_MAX - 1)];
		tmp_ptr = (void *) (&p_oRetTRB->buf_ptr_l);
		tmp = (&p_oNewBlock->m_a_oTRB[0]);
		*tmp_ptr = tmp;
		p_oRetTRB->status.b.buf_siz = 0x0;
		// Set Control
		p_oRetTRB->control.data = 0;
		p_oRetTRB->control.b.trb_ctrl = TRB_CTRL_LINK;
		p_oRetTRB->control.b.lst = 0;
		p_oRetTRB->control.b.chn = 0;
		// for Test
		p_oRetTRB->control.b.ioc = 0;
		// Set H/W Own
		p_oRetTRB->control.b.hwo = 1;

		// Link New Block and Block Cnt
		p_oCurrentBlock->m_p_oNext = p_oNewBlock;
		hTRB->m_uCurrentBlock++;
		hTRB->m_sNumBlock++;

		p_oRetTRB = &(p_oNewBlock->m_a_oTRB[0]);
		p_oNewBlock->m_a_uXferSize[0] = uSize;
		p_oNewBlock->input_cnt++;
	} else {
		p_oRetTRB = &(p_oCurrentBlock->m_a_oTRB[p_oCurrentBlock->input_cnt - 1]);
		p_oCurrentBlock->m_a_uXferSize[p_oCurrentBlock->input_cnt - 1] = uSize;
	}
	return p_oRetTRB;
}

void dwc3_dev_trb_check_last_trb(USB3_DEV_TRB_p last_trb)
{
	last_trb->control.b.lst = 1;
	last_trb->control.b.chn = 0;
	last_trb->control.b.ioc = 1;
}

void __usb3_dev_trb_MakeLinkTRB(USB3_DEV_TRB_HANDLER hTRB,
	USB3_DEV_TRB_p pLinkTargetAddr)
{
	USB3_DEV_TRB_p p_oTRB = __usb3_dev_trb_GetNext(hTRB, 0);

	// Set TRB : Address and Size
	if (sizeof(USB3_DEV_TRB_p) > 4) {
		p_oTRB->buf_ptr_l = (unsigned long) ((u64) pLinkTargetAddr & 0xffffffff);
		p_oTRB->buf_ptr_h = (unsigned long) (((u64) pLinkTargetAddr >> 32) & 0xffffffff);
	} else {
		p_oTRB->buf_ptr_l = (unsigned long) pLinkTargetAddr;
		p_oTRB->buf_ptr_h = 0x0;
	}
	p_oTRB->status.b.buf_siz = 0x0;
	// Set Control
	p_oTRB->control.data = 0;
	p_oTRB->control.b.trb_ctrl = TRB_CTRL_LINK;
	p_oTRB->control.b.lst = 0;
	p_oTRB->control.b.chn = 0;
	// for Test
	p_oTRB->control.b.ioc = 0;
	// Set H/W Own
	p_oTRB->control.b.hwo = 1;
}

void dwc3_dev_trb_release(USB3_DEV_TRB_HANDLER hTRB, USB_DIR eDir)
{
	// Clear TRB Block
	__usb3_dev_trb_CompleteOP(hTRB, 1, eDir);
	// Set Init Value for Next Transfer
	hTRB->m_bCachedXfer = 0;
	hTRB->m_uCurrentBlock = 1;
}

USB3_DEV_TRB_p dwc3_dev_trb_pop_last(USB3_DEV_TRB_HANDLER hTRB, USB_DIR eEpDir)
{
	if (!hTRB->m_p_oLastTRB)
		__usb3_dev_trb_CompleteOP(hTRB, 0, eEpDir);
	return hTRB->m_p_oLastTRB;
}

void *usb3_dev_trb_prepare(USB3_DEV_TRB_HANDLER hTRB, u8 bCacheOp,
	USB_DIR eEpDir, u8 bCircular)
{
	USB3_DEV_TRB_BLOCK_p p_oCurrent =
			(USB3_DEV_TRB_BLOCK_p) (hTRB->m_p_oStartTRBBlock);
	s32 uCnt = 0;

	if (bCircular) {
		/* Make Last TRB Link to first TRB */
		__usb3_dev_trb_MakeLinkTRB(hTRB, (void *) p_oCurrent->m_a_oTRB);
	}

	for (; uCnt < hTRB->m_sNumBlock && p_oCurrent; uCnt++) {
		if (bCacheOp || bCircular) {
			u32 uTRBCnt;
			USB3_DEV_TRB_p trb;

			hTRB->m_bCachedXfer = 1;
			for (uTRBCnt = 0; uTRBCnt < p_oCurrent->input_cnt;
					uTRBCnt++) {
				trb = &p_oCurrent->m_a_oTRB[uTRBCnt];
				/* Skip when link TRB is meet */
				if (trb->control.b.trb_ctrl == TRB_CTRL_LINK)
					continue;
				/* circular trb needs to update when
				 * in-progress event*/
				if (bCircular)
					trb->control.b.ioc = 1;
#if 0
				/* Skip cache operation is not needed if used
				 * does not needed or buf size is zero */
				if (!bCacheOp || !(trb->status.b.buf_siz))
					continue;

				if (eEpDir == USBDIR_IN) {
					U3DBG_CACHE("DATA BUFFER CLEAN\n");
				} else {
					U3DBG_CACHE("DATA BUFFER INVALID\n");
				}
				U3DBG_CACHE("ADDR : 0x%x Size : %d\n",
					    trb->buf_ptr_l,
					    trb->status.b.buf_siz);
				if (eEpDir == USBDIR_IN) {
					CoCleanDCache(trb->buf_ptr_l,
						      trb->status.b.buf_siz);
				} else {
					InvalidateDCache(trb->buf_ptr_l,
							 trb->status.b.buf_siz);
				}
#endif
			}
		} else
			hTRB->m_bCachedXfer = 0;
#if 0
		// Clean TRB block
		if (!p_oCurrent->m_bCacheCleand) {
			p_oCurrent->m_bCacheCleand = 1;
			U3DBG_CACHE("TRB CLEAN\n");
			U3DBG_CACHE("ADDR : 0x%x Size : %d\n",
				    p_oCurrent->m_a_oTRB,
				    sizeof(USB3_DEV_TRB_o) * TRB_BLOCK_MAX);
			CoCleanDCache((unsigned long) (p_oCurrent->m_a_oTRB),
				      sizeof(USB3_DEV_TRB_o) * TRB_BLOCK_MAX);
		}
#endif
		p_oCurrent = p_oCurrent->m_p_oNext;
	}
	hTRB->m_p_oLastTRB = NULL;
	return &hTRB->m_p_oStartTRBBlock->m_a_oTRB[0];
}

u32 usb3_dev_trb_reset_hwo(USB3_DEV_TRB_HANDLER hTRB)
{
	USB3_DEV_TRB_BLOCK_p p_oCurrentBlock = NULL;
	u32 uRetXferSize = 0;

	for (p_oCurrentBlock = hTRB->m_p_oStartTRBBlock;
			p_oCurrentBlock;
			p_oCurrentBlock = p_oCurrentBlock->m_p_oNext) {
		u32 uTrbCnt;
		u32 num_trb = p_oCurrentBlock->input_cnt;
#if 0
		/* Cache Invalid for Updated by DMA */
		if (p_oCurrentBlock->m_bCacheCleand) {
			U3DBG_CACHE("TRB INVALID\n");
			U3DBG_CACHE("ADDR : 0x%p Size : %lud\n",
				    p_oCurrentBlock->m_a_oTRB,
				    sizeof(USB3_DEV_TRB_o)
				    * num_trb);
			InvalidateDCache((unsigned long) (p_oCurrentBlock->m_a_oTRB),
				sizeof(USB3_DEV_TRB_o) * num_trb);
		}
#endif
		p_oCurrentBlock->m_bCacheCleand = 0;
		for (uTrbCnt = 0; uTrbCnt < num_trb; uTrbCnt++) {
			USB3_DEV_TRB_p p_oTRB =
					&(p_oCurrentBlock->m_a_oTRB[uTrbCnt]);
			/* NO TRB */
			if (p_oTRB->control.b.trb_ctrl == 0)
				break;
			/* Skip Link TRB */
			if (p_oTRB->control.b.trb_ctrl == TRB_CTRL_LINK)
				continue;
			/* Not or yet used by HW */
			if (p_oTRB->control.b.hwo)
				continue;
			/* Mark current trb block is dirty */
			p_oCurrentBlock->m_bCacheCleand = 1;
			/* reset transfer size original */
			uRetXferSize += p_oCurrentBlock->m_a_uXferSize[uTrbCnt] - p_oTRB->status.b.buf_siz;

			p_oTRB->status.b.buf_siz = p_oCurrentBlock->m_a_uXferSize[uTrbCnt];
			p_oTRB->control.b.hwo = 1;
		}
		// Cache Invalid for Updated by CPU
#if 0
		if (!p_oCurrentBlock->m_bCacheCleand) {
			p_oCurrentBlock->m_bCacheCleand = 1;
			U3DBG_CACHE("TRB CLEAN\n");
			U3DBG_CACHE("ADDR : 0x%p Size : %lud\n",
				    p_oCurrentBlock->m_a_oTRB,
				    sizeof(USB3_DEV_TRB_o)
				    * num_trb);
			CoCleanDCache((unsigned long) (p_oCurrentBlock->m_a_oTRB),
				sizeof(USB3_DEV_TRB_o) * num_trb);
		}
#endif
	}
	return uRetXferSize;
}

USB3_DEV_TRB_p dwc3_dev_trb_set(USB3_DEV_TRB_HANDLER hTRB, void *pAddr,
		u32 uLen, USB3_DEV_TRB_TYPE_e eTRBCtl,
		USB3_DEV_TRB_OPTION uOption)
{
	USB3_DEV_TRB_p p_oTRB = __usb3_dev_trb_GetNext(hTRB, uLen);

	U3DBG_TRB("Buffer Address : 0x%p, Size : 0x%x\n", pAddr, uLen);
	// Set TRB : Address and Size
	if (sizeof(void *) > 4) {
		p_oTRB->buf_ptr_l =
			(unsigned long) ((u64) pAddr & 0xffffffff);
		p_oTRB->buf_ptr_h =
			(unsigned long) (((u64) pAddr >> 32) & 0xffffffff);
	} else {
		p_oTRB->buf_ptr_l = (unsigned long) pAddr;
		p_oTRB->buf_ptr_h = 0x0;
	}
	// Set Status
	p_oTRB->status.b.buf_siz = uLen;
	// Set Control
	p_oTRB->control.data = 0;
	p_oTRB->control.b.trb_ctrl = eTRBCtl;
	// Is it Last Buffer?
	if (uOption & TRB_LAST) {
		if (hTRB->on_demand == true)
			p_oTRB->control.b.lst = 0;
		else
			p_oTRB->control.b.lst = 1;
		p_oTRB->control.b.chn = 0;
	} else {
		p_oTRB->control.b.lst = 0;
		p_oTRB->control.b.chn = 1;
	}
	//Non ISO EP
	p_oTRB->control.b.isp_imi = 0;
	p_oTRB->control.b.strmid_sofn = 0;
	p_oTRB->control.b.csp = 0;
	// Enable Interrupt?
	if (uOption & TRB_INTR_EN)
		p_oTRB->control.b.ioc = 1;
	else
		p_oTRB->control.b.ioc = 0;
	// Set H/W Own
	p_oTRB->control.b.hwo = 1;

	return p_oTRB;
}

USB3_DEV_TRB_HANDLER dwc3_trb_create(u8 non_cachable, u8 on_demand)
{
	USB3_DEV_TRB_HANDLER hTRB = dwc3_calloc_align(sizeof(_usb3_dev_trb_handler), 64);

	hTRB->m_p_oStartTRBBlock = dwc3_dev_trb_make_blk();
	hTRB->m_uCurrentBlock = hTRB->m_sNumBlock = 1;
	hTRB->m_uLastBlockCnt = hTRB->m_uTRBUsageCnt = 0;
	hTRB->non_cachable = non_cachable;
	hTRB->on_demand = on_demand;
	return hTRB;
}

void dwc3_dev_trb_destroy(USB3_DEV_TRB_HANDLER hTRB)
{
	USB3_DEV_TRB_BLOCK_p p_oCurrentBlock = hTRB->m_p_oStartTRBBlock;
	s32 uCnt = 0;

	for (; uCnt < hTRB->m_sNumBlock - 1; uCnt++) {
		USB3_DEV_TRB_BLOCK_p p_oNext = p_oCurrentBlock->m_p_oNext;
		dwc3_free(p_oCurrentBlock);
		p_oCurrentBlock = p_oNext;
	}
	hTRB->m_p_oStartTRBBlock = p_oCurrentBlock;

}
