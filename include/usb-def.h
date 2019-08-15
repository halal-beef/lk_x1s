/*
 * usb-def.h
 *
 *  Created on: 2019. 3. 21.
 *      Author: sunghyun.na
 */

#ifndef INCLUDE_USB_DEF_H_
#define INCLUDE_USB_DEF_H_

#include <sys/types.h>

#ifndef HI_BYTE
#define HI_BYTE(__X)		((__X&0xFF00)>>8)
#endif

#ifndef LO_BYTE
#define LO_BYTE(__X)		((__X)&0xFF)
#endif

#ifndef SWAP16
#define SWAP16(data)		(data>>8)+((data&0xFF)<<8) //BBAA -> AABB
#endif

#ifndef SWAP32
#define SWAP32(data)		((data&0xFF)<<24)+((data&0xFF00)<<8)+((data&0xFF0000)>>8)+((data&0xFF000000)>>24) // DDCCBBAA->AABBCCDD
#endif

#define LOW_SPEED_CONTROL_PKT_SIZE		8

#define FULL_SPEED_CONTROL_PKT_SIZE		64
#define FULL_SPEED_BULK_PKT_SIZE		64

#define HIGH_SPEED_CONTROL_PKT_SIZE		64
#define HIGH_SPEED_BULK_PKT_SIZE		512

#define SUPER_SPEED_CONTROL_PKT_SIZE	512
#define SUPER_SPEED_BULK_PKT_SIZE		1024

/* struct about usb */
typedef enum {
	USB_OPMODE_NONE = 0x0,
	USB_DEV = 0x10,
	USB_HOST = 0x20,
	/* Extention Feature - not specified in the usb */
	USB_BYPASS_UART = 0x800,
	/* Extention Feature - not specified in the usb */
	USB_TESTMODE_PHY = 0x1000,
} USB_OPMODE;

/* usb spec defines */
typedef enum {
	USBSPEED_LOW = 0x2,
	USBSPEED_FULL = 0x1,
	USBSPEED_HIGH = 0x0,
	USBSPEED_HIGH_HOST = 0x3,
	USBSPEED_SUPER = 0x4,
	USBSPEED_SUPERPLUS = 0x5,
} USB_SPEED;

typedef enum {
	USBEP_CTRL = 0, USBEP_ISOC = 1, USBEP_BULK = 2, USBEP_INTR = 3
} USB_EP;
typedef enum {
	USBDIR_OUT = 0, /* Host to device */
	USBDIR_IN = 1 /* Device to Host */
} USB_DIR;
typedef enum {
	USBTEST_DISABLED = 0,
	USBTEST_J = 1,
	USBTEST_K = 2,
	USBTEST_SE0_NAK = 3,
	USBTEST_PACKET = 4,
	USBTEST_FORCE_ENABLE = 5,
} USB_TEST;
typedef enum {
	DEV_NOTI_FUNCTION_WAKE = 0x1,
	DEV_NOTI_LTM = 0x2,
	DEV_NOTI_BUS_INTERVAL_ADJUST = 0x3,
	DEV_NOTI_HOST_ROLE_REQ = 0x4,
} USB_DEV_NOTI;

/* usb standard request */
typedef enum {
	STANDARD_TYPE = 0x00, CLASS_TYPE = 0x20, VENDOR_TYPE = 0x40, RESERVED_TYPE = 0x60
} USB_STDREQ_TYPE;
typedef enum {
	HOST_TO_DEVICE = 0x00, DEVICE_TO_HOST = 0x80
} USB_STDREQ_DIRECTION;
typedef enum {
	DEVICE_RECIPIENT = 0, INTERFACE_RECIPIENT = 1, ENDPOINT_RECIPIENT = 2, OTHER_RECIPIENT = 3
} USB_STDREQ_RECIPIENT;
typedef enum {
	/* Endpoint */
	ENDPOINT_HALT = 0,
	/* Interface */
	FUNCTION_SUSPEND = 0,
	/* Device */
	REMOTEWAKEUP_ENABLE = 1,
	TEST_MODE_ENABLE = 2,
	U1_ENABLE = 48,
	U2_ENABLE = 49,
	LTM_ENABLE = 50,
	/* Add by USB3.0 Spec with ECN May 1, 2011 */
	B3_NTF_HOST_REL = 50,
} USB_STDREQ_FEATURE;
typedef enum {
	STANDARD_GET_STATUS = 0,
	STANDARD_CLEAR_FEATURE = 1,
	STANDARD_RESERVED_1 = 2,
	STANDARD_SET_FEATURE = 3,
	STANDARD_RESERVED_2 = 4,
	STANDARD_SET_ADDRESS = 5,
	STANDARD_GET_DESCRIPTOR = 6,
	STANDARD_SET_DESCRIPTOR = 7,
	STANDARD_GET_CONFIGURATION = 8,
	STANDARD_SET_CONFIGURATION = 9,
	STANDARD_GET_INTERFACE = 10,
	STANDARD_SET_INTERFACE = 11,
	STANDARD_SYNCH_FRAME = 12,
	STANDARD_SET_SEL = 48,
	STANDARD_ISOCH_DELY = 49,
} USB_STDREQ_CODE;
typedef struct {
	u8 bmRequestType;
	u8 bRequest;
	u16 wValue;
	u16 wIndex;
	u16 wLength;
} __attribute__((__packed__)) USB_STD_REQUEST;

/* usb standard descriptor */
typedef enum {
	DEVICE_DESCRIPTOR = 1,
	CONFIGURATION_DESCRIPTOR = 2,
	STRING_DESCRIPTOR = 3,
	INTERFACE_DESCRIPTOR = 4,
	ENDPOINT_DESCRIPTOR = 5,
	DEVICE_QUALIFIER = 6,
	OTHER_SPEED_CONFIGURATION = 7,
	INTERFACE_POWER_DESC = 8,
	OTG_DESC = 9,
	DEBUG_DESC = 10,
	INTERFACE_ASSOCIATION = 11,
	BOS_DESC = 15,
	DEVICE_CAPABILITY_DESC = 16,
	SUPERSPEED_USB_EP_COMPANION_DESC = 48,
} USB_STDDESC_TYPE;
typedef enum {
	USB_DESC_SIZE_DEVICE = 18,
	USB_DESC_SIZE_CONFIG = 9,
	USB_DESC_SIZE_INTERFACE = 9,
	USB_DESC_SIZE_ENDPOINT = 7,
	USB_DESC_SIZE_DEVICE_QUALIFIER = 10,
	USB_DESC_SIZE_OTHER_SPEED_CFG = 9,

	USB_DESC_SIZE_OTG = 10,
	USB_DESC_SIZE_DEBUG = 10,

	USB_DESC_SIZE_IF_ASSOC = 8,
	USB_DESC_SIZE_BOS = 5,
	USB_DESC_SIZE_CAP_20EXT = 7,
	USB_DESC_SIZE_CAP_SUPERSPEED = 10,
	USB_DESC_SIZE_CAP_SUPERSPEEDPLUS = 20,
	USB_DESC_SIZE_CAP_CONTAINID = 20,

	USB_DESC_SIZE_EP = 7,
	//USB_DESC_SIZE_EP_ISO = 9,
	USB_DESC_SIZE_EP_COMP = 6,
} USB_STDDESC_SIZE;
typedef enum {
	WIRELESS_CAPABLE = 0x01,
	USB20EXTENTION_CAPABLE = 0x02,
	SUPERSPEED_CAPABLE = 0x03,
	CONTAINTERID_CAPABLE = 0x04,
} USB_STDDESC_DEVICE_CAPABILITY_TYPE;
typedef enum {
	CONF_ATTR_DEFAULT = 0x80, // in Spec 1.0, it was BUSPOWERED bit.
	CONF_ATTR_REMOTE_WAKEUP = 0x20,
	CONF_ATTR_SELFPOWERED = 0x40
} USB_STDDESC_CONFIG_ATTRIBUTES;
typedef enum {
	EP_ADDR_IN = 0x80,
	EP_ADDR_OUT = 0x00,
	EP_ATTR_CONTROL = 0x0,
	EP_ATTR_ISOCHRONOUS = 0x1,
	EP_ATTR_BULK = 0x2,
	EP_ATTR_INTERRUPT = 0x3
} USB_STDDESC_EP_ATTRIBUTES;

typedef enum {
	UC_DEVICE = 0x00,		// Class defined in Interface Descriptor
	UC_AUDIO = 0x01,		// audio
	UC_CDC = 0x02,			// Communication Device Control
	UC_HID = 0x03,			// Human Interface Device
	UC_PHYSICAL = 0x05,		// Physical
	UC_SI = 0x06,			// Still Image
	UC_PRINTER = 0x07,		// Printer
	UC_MS = 0x08,			// Mass Storage
	UC_HUB = 0x09,			// Hub
	UC_CDD = 0x0a,			// Communication Device Data
	UC_SMTCARD = 0x0b,		// Smart Card
	UC_SECURITY = 0x0d,		// Contents Security
	UC_VIDEO = 0x0e,		// Video
	UC_HEALTHCARE = 0x0f,		// Health Care
	UC_AV = 0x10,			// Audio Video
	UC_DIAGNOSTIC = 0xdc,		// Diagnostic Device
	UC_WIRELESS = 0xe0,		// Wireless controller
	UC_MISC = 0xef,			// Miscellaneous
	UC_APPLICATION = 0xfe,		// Application Defined
	UC_VENDOR = 0xff,		// Vendor Defined
} USB_CLASS;

#define USB_CAP_20_EXT  0x2
#define USB_CAP_SS      0x3
#define USB_CAP_SSP     0xA
#define USB_CAP_CID     0x4

/* Structure define of desciprtor */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u16 bcdUSB;
	u8 bDeviceClass;
	u8 bDeviceSubClass;
	u8 bDeviceProtocol;
	u8 bMaxPacketSize0;
	u16 idVendor;
	u16 idProduct;
	u16 bcdDevice;
	u8 iManufacturer;
	u8 iProduct;
	u8 iSerialNumber;
	u8 bNumConfigurations;
} __attribute__((__packed__)) USB_DEVICE_DESCRIPTOR;
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u16 bcdUSB;
	u8 bDeviceClass;
	u8 bDeviceSubClass;
	u8 bDeviceProtocol;
	u8 bMaxPacketSize0;
	u8 bNumConfigurations;
	u8 bReserved;
} __attribute__((__packed__)) USB_DEVICE_QUALIFIER_DESCRIPTOR;
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u16 wTotalLength;
	u8 bNumDeviceCaps;
} __attribute__((__packed__)) USB_BOS_DESCRIPTOR;
typedef struct {
	unsigned RSVD0 :1;
	unsigned LPM :1;
	unsigned BESL_Support :1;
	unsigned BESL_BASELINE_VALID :1;
	unsigned BESL_DEEP_VALID :1;
	unsigned RSVD7_5 :3;
	unsigned BESL_BASELINE_VALUE :4;
	unsigned BESL_DEEP_VALUE :4;
	unsigned RSVD31_16 :16;
} __attribute__((__packed__)) USB_CAP_DESC_20EXT_ATTRI_BitField;
typedef union {
	USB_CAP_DESC_20EXT_ATTRI_BitField b;
	u32 uData;
} __attribute__((__packed__)) USB_CAP_DESC_20EXT_ATTRI;
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDevCapabilityType;
	USB_CAP_DESC_20EXT_ATTRI bmAttributes;
} __attribute__((__packed__)) USB_CAP_20EXT_DESCRIPTOR;
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDevCapabilityType;
	u8 bmAttributes;
	u16 wSpeedsSupported;
	u8 bFunctionalitySupport;
	u8 bU1DevExitLat;
	u16 wU2DevExitLat;
} __attribute__((__packed__)) USB_CAP_SUPERSPEED_DESCRIPTOR;
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDevCapabilityType;
	u8 bReserved;
	u32 bmAttributes;
	u16 wFunctionalitySupport;
	u16 wReserved;
	u32 bmSublinkSpeedAttr[2];
} __attribute__((__packed__)) USB_CAP_SUPERSPEEDPLUS_DESCRIPTOR;
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDevCapabilityType;
	u8 bReserved;
	u8 containerID[16];
} __attribute__((__packed__)) USB_CAP_CONTAINID_DESCRIPTOR;
typedef struct {
	USB_DEVICE_DESCRIPTOR oDevice;
	USB_BOS_DESCRIPTOR oBos;
	void *add_infor;
} __attribute__((__packed__)) USB_DESC_DEVICE_ATTRIBUTES;
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 *pString;
} __attribute__((__packed__)) USB_STRING_DESCRIPTOR;
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u16 wTotalLength;
	u8 bNumInterfaces;
	u8 bConfigurationValue;
	u8 iConfiguration;
	u8 bmAttributes;
	u8 maxPower;
} __attribute__((__packed__)) USB_CONFIGURATION_DESCRIPTOR;
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bInterfaceNumber;
	u8 bAlternateSetting;
	u8 bNumEndpoints;
	u8 bClass;
	u8 bSubClass;
	u8 bProtocol;
	u8 iInterface;
} __attribute__((__packed__)) USB_INTERFACE_DESCRIPTOR, USB_DESC_INTERFACE;
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bFirstInterface;
	u8 bInterfaceCount;
	u8 bFunctionClass;
	u8 bFunctionSubClass;
	u8 bFunctionProtocol;
	u8 iFunction;
} __attribute__((__packed__)) USB_INTERFACE_ASSOCIATION_DESC;

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bEndpointAddress;
	u8 bmAttributes;
	u16 wMaxPacketSize;
	u8 bInterval;
	// for ISO EP
	//u8 bRefresh;
	//u8 bSyncAddress;
} __attribute__((__packed__)) USB_ENDPOINT_DESCRIPTOR;

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bMaxBurst;
	u8 bmAttributes;
	u16 wBytesPerInterval;
} __attribute__((__packed__)) USB_EPCOMP_DESCRIPTOR;

#define LANGID_US	0x0409



#endif /* INCLUDE_USB_DEF_H_ */
