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

#ifndef _CDC_ACM_CDC_ACM_DEF_H_
#define _CDC_ACM_CDC_ACM_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif	//__cplusplus

//Comm Device Control Class
typedef enum {
	CDC_DLCM = 0x01,	// Direct Line Control Model
	CDC_ACM = 0x02,		// Abstract Control Model
	CDC_TCM = 0x03,		// Telephone Control Model
	CDC_MCCM = 0x04,	// Multi Channel (ISDN) Control Model
	CDC_CAPI = 0x05,	// Common ISDN API
	CDC_ENCM = 0x06,	// Ethernet Networking Control Model
	CDC_ANCM = 0x07,	// ATM Networking Control Model
	CDC_WHCM = 0x08,	// Wireless Handset Control Model
	CDC_DM = 0x09,		// Device Management
	CDC_MDLM = 0x0a,	// Mobile Direct Line Model
	CDC_OBEX = 0x0b,	// OBEX
	CDC_EEM = 0x0c,		// Ethernet emulation model
	CDC_NCM = 0x0d,		// Network Control model
} USB_SUBCLASS_CDC;

enum CDC_PROT_CODE {
	CDC_PROT_NO_SPECIFIC = 0x0,
	CDC_PROT_AT_CMD_COMMON = 0x1,
	CDC_PROT_AT_CMD_PCCA = 0x2,
	CDC_PROT_AT_CMD_PCCA_ANNEX_O = 0x3,
	CDC_PROT_AT_CMD_GSM = 0x4,
	CDC_PROT_AT_CMD_3GPP = 0x5,
	CDC_PROT_AT_CMD_TIA = 0x6,
	CDC_PROT_EEM = 0x7,
	CDC_PROT_VENDOR = 0xff,
};

enum DIC_PROT_CODE {
	DIC_PROT_NO = 0x0,
	DIC_PROT_NCM = 0x1,
	DIC_PROT_ISDN = 0x30,
	DIC_PROT_HDLC = 0x31,
};

enum CDC_SUBTYPE_DESC_CODE {
	CDC_SUBDESC_HEADER = 0x0,
	CDC_SUBDESC_CM = 0x1,			// Call Management
	CDC_SUBDESC_ACM = 0x2,			// Abstract control management
	CDC_SUBDESC_DLM = 0x3,			// Direct Line management
	CDC_SUBDESC_TR = 0x4,			// Telephone Ringer
	CDC_SUBDESC_TCLSRC = 0x5,		// Telephone Call and Line sate Reporting Capa
	CDC_SUBDESC_UNION = 0x6,		// Union
	CDC_SUBDESC_COUNTRY_SEL = 0x7,		// Country selection
	CDC_SUBDESC_TOM = 0x8,			// Telephone operational modes
	CDC_SUBDESC_USB_TERM = 0x9,		// USB Terminal
	CDC_SUBDESC_NC_TERM = 0xa,		// Network Channel Terminal
	CDC_SUBDESC_PROT_UNIT = 0xb,		// Protocol unit
	CDC_SUBDESC_EXT_UNIT = 0xc,		// Extension unit
	CDC_SUBDESC_MCM = 0xd,			// Multi channel management
	CDC_SUBDESC_CAPI_CM = 0xe,		// CAPI Control
	CDC_SUBDESC_ETHERNET = 0xf,		// Ethernet Networking
	CDC_SUBDESC_ATM_NET = 0x10,		// ATM Networking
	CDC_SUBDESC_WCM = 0x11,			// Wireless Headset control model
	CDC_SUBDESC_MDLM = 0x12,		// Mobile Direct LIne Model
	CDC_SUBDESC_MDLM_DETAIL = 0x13,		// MDLM Detail
	CDC_SUBDESC_DMM = 0x14,			// Device Management model
	CDC_SUBDESC_OBEX = 0x15,		// OBEX
	CDC_SUBDESC_CMD_SET = 0x16,		// Command Set
	CDC_SUBDESC_CMD_SET_DETAIL = 0x17,	// Command Set Detail
	CDC_SUBDESC_TCM = 0x18,			// Telephone control model
	CDC_SUBDESC_OBEX_SI = 0x19,		// OBEX service identifier
	CDC_SUBDESC_NCM = 0x1a,			// NCM
};

#define CDC_PROTOCOL_CATC 0x01	// Common AT Commands

typedef enum {
	SEND_ENCAPSULATED_COMMAND = 0x00,
	GET_ENCAPSULATED_RESPONSE = 0x01,
	SET_COMM_FEATURE = 0x02,
	GET_COMM_FEATURE = 0x03,
	CLEAR_COMM_FEATURE = 0x04,

	SET_AUX_LINE_STATE = 0x10,
	SET_HOOK_STATE = 0x11,
	PULSE_SETUP = 0x12,
	SEND_PULSE = 0x13,
	SET_PULSE_TIME = 0x14,
	RING_AUX_JACK = 0x15,

	SET_LINE_CODING = 0x20,
	GET_LINE_CODING = 0x21,
	SET_CONTROL_LINE_STATE = 0x22,
	SEND_BREAK = 0x23,

	SET_RINGER_PARMS = 0x30,
	GET_RINGER_PARMS = 0x31,
	SET_OPERATION_PARMS = 0x32,
	GET_OPERATION_PARMS = 0x33,
	SET_LINE_PARMS = 0x34,
	GET_LINE_PARMS = 0x35,
	DIAL_DIGITS = 0x36,
	SET_UNIT_PARAMETER = 0x37,
	GET_UNIT_PARAMETER = 0x38,
	CLEAR_UNIT_PARAMETER = 0x39,
	GET_PROFILE = 0x3A,

	SET_ETHERNET_MULTICAST_FILTERS = 0x40,
	SET_ETHERNET_POWER_MANAGEMENT_PATTERN_FILTER = 0x41,
	GET_ETHERNET_POWER_MANAGEMENT_PATTERN_FILTER = 0x42,
	SET_ETHERNET_PACKET_FILTER = 0x43,
	GET_ETHERNET_STATISTIC = 0x44,

	SET_ATM_DATA_FORMAT = 0x50,
	GET_ATM_DEVICE_STATISTICS = 0x51,
	SET_ATM_DEFAULT_VC = 0x52,
	GET_ATM_VC_STATISTICS = 0x53,
} CDC_CLASS_REQUEST;

typedef struct {
	unsigned char bLength;
	unsigned char bDescType;
	unsigned char bDescSubType;
	/* bcdCDC
	 * 	USB Class Definitions for Communications Devices Specification
	 * 	release number in binary-coded decimal.
	 */
	unsigned short bcdCDC;
} __attribute__((__packed__)) CDC_CLASS_INTERFACE_DESC_HEADER;

typedef struct {
	unsigned char bLength;
	unsigned char bDescType;
	unsigned char bDescSubType;
	/* bmCapabilities
	 *	The capabilities that this configuration supports:
	 *	D7..D2: RESERVED (Reset to zero)
	 *	D1: 0 - Device sends/receives call management information
	 *		only over the Communications Class interface.
	 *	    1 - Device can send/receive call management information
	 *		over a Data Class interface.
	 *	D0: 0 - Device does not handle call management itself.
	 *	    1 - Device handles call management itself.
	 *	The previous bits, in combination, identify which call
	 *	management scenario is used. If bit D0 is reset to 0,
	 *	then the value of bit D1 is ignored. In this case, bit D1
	 *	is reset to zero for future compatibility. */
	unsigned char bmCapabilities;
	/* bDataInterface
	 *	Interface number of Data Class interface optionally used for
	 *	call management.
	 *	(Zero based index of the interface in this configuration (bInterfaceNum).)
	 */
	unsigned char bDataInterface;
} __attribute__((__packed__)) CDC_CLASS_INTERFACE_DESC_CM;

typedef struct {
	unsigned char bLength;
	unsigned char bDescType;
	unsigned char bDescSubType;
	/* bmCapabilities
	 *	The capabilities that this configuration supports.
	 *	(A bit value of zero means that the request is not supported.)
	 *		D7..D4: RESERVED (Reset to zero)
	 *		D3: 1 - Device supports the notification Network_Connection.
	 *		D2: 1 - Device supports the request Send_Break
	 *		D1: 1 - Device supports the request combination of
	 *			Set_Line_Coding, Set_Control_Line_State, Get_Line_Coding,
	 *			and the notification Serial_State.
	 *		D0: 1 - Device supports the request combination of Set_Comm_Feature,
	 *			Clear_Comm_Feature, and Get_Comm_Feature.
	 *	The previous bits, in combination, identify which requests/notifications are
	 *	supported by a CommunicationsClass interface with the SubClass code
	 *	of Abstract Control Model. */
	unsigned char bmCapabilities;
} __attribute__((__packed__)) CDC_CLASS_INTERFACE_DESC_ACM;

typedef struct {
	unsigned char bLength;
	unsigned char bDescType;
	unsigned char bDescSubType;
	unsigned char bMasterInterface;
	unsigned char bSlaveInterface;
} __attribute__((__packed__)) CDC_CLASS_INTERFACE_DESC_UNION;

typedef struct {
	CDC_CLASS_INTERFACE_DESC_HEADER oHeader;
	CDC_CLASS_INTERFACE_DESC_CM oCM;
	CDC_CLASS_INTERFACE_DESC_ACM oACM;
	CDC_CLASS_INTERFACE_DESC_UNION oUNION;
} __attribute__((__packed__)) CDC_ADDITIONAL_INTERFACE_DESC;

#ifdef __cplusplus
}
#endif	//__cplusplus

#endif /* DEV_USB_DEVICE_CDC_ACM_CDC_ACM_DEF_H_ */
