/* Copyright (c) 2018 Samsung Electronics Co, Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.

 * Copyright@ Samsung Electronics Co. LTD
 * Manseok Kim <manseoks.kim@samsung.com>

 * Alternatively, Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __MIPI_DSI_CMD_H__
#define __MIPI_DSI_CMD_H__

/* MIPI DSI Processor-to-Peripheral transaction types */
enum {
	MIPI_DSI_V_SYNC_START				= 0x01,
	MIPI_DSI_V_SYNC_END				= 0x11,
	MIPI_DSI_H_SYNC_START				= 0x21,
	MIPI_DSI_H_SYNC_END				= 0x31,

	MIPI_DSI_COLOR_MODE_OFF				= 0x02,
	MIPI_DSI_COLOR_MODE_ON				= 0x12,
	MIPI_DSI_SHUTDOWN_PERIPHERAL			= 0x22,
	MIPI_DSI_TURN_ON_PERIPHERAL			= 0x32,

	MIPI_DSI_GENERIC_SHORT_WRITE_0_PARAM		= 0x03,
	MIPI_DSI_GENERIC_SHORT_WRITE_1_PARAM		= 0x13,
	MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM		= 0x23,

	MIPI_DSI_GENERIC_READ_REQUEST_0_PARAM		= 0x04,
	MIPI_DSI_GENERIC_READ_REQUEST_1_PARAM		= 0x14,
	MIPI_DSI_GENERIC_READ_REQUEST_2_PARAM		= 0x24,

	MIPI_DSI_DCS_SHORT_WRITE			= 0x05,
	MIPI_DSI_DCS_SHORT_WRITE_PARAM			= 0x15,

	MIPI_DSI_DCS_READ				= 0x06,
	MIPI_DSI_DSC_PRA				= 0x07,
	MIPI_DSI_DSC_PPS				= 0x0a,

	MIPI_DSI_SET_MAXIMUM_RETURN_PACKET_SIZE		= 0x37,

	MIPI_DSI_END_OF_TRANSMISSION			= 0x08,

	MIPI_DSI_NULL_PACKET				= 0x09,
	MIPI_DSI_BLANKING_PACKET			= 0x19,
	MIPI_DSI_GENERIC_LONG_WRITE			= 0x29,
	MIPI_DSI_DCS_LONG_WRITE				= 0x39,

	MIPI_DSI_LOOSELY_PACKED_PIXEL_STREAM_YCBCR20	= 0x0c,
	MIPI_DSI_PACKED_PIXEL_STREAM_YCBCR24		= 0x1c,
	MIPI_DSI_PACKED_PIXEL_STREAM_YCBCR16		= 0x2c,

	MIPI_DSI_PACKED_PIXEL_STREAM_30			= 0x0d,
	MIPI_DSI_PACKED_PIXEL_STREAM_36			= 0x1d,
	MIPI_DSI_PACKED_PIXEL_STREAM_YCBCR12		= 0x3d,

	MIPI_DSI_PACKED_PIXEL_STREAM_16			= 0x0e,
	MIPI_DSI_PACKED_PIXEL_STREAM_18			= 0x1e,
	MIPI_DSI_PIXEL_STREAM_3BYTE_18			= 0x2e,
	MIPI_DSI_PACKED_PIXEL_STREAM_24			= 0x3e,
};

/* MIPI DSI Peripheral-to-Processor transaction types */
enum {
	MIPI_DSI_RX_ACKNOWLEDGE_AND_ERROR_REPORT	= 0x02,
	MIPI_DSI_RX_END_OF_TRANSMISSION			= 0x08,
	MIPI_DSI_RX_GENERIC_SHORT_READ_RESPONSE_1BYTE	= 0x11,
	MIPI_DSI_RX_GENERIC_SHORT_READ_RESPONSE_2BYTE	= 0x12,
	MIPI_DSI_RX_GENERIC_LONG_READ_RESPONSE		= 0x1a,
	MIPI_DSI_RX_DCS_LONG_READ_RESPONSE		= 0x1c,
	MIPI_DSI_RX_DCS_SHORT_READ_RESPONSE_1BYTE	= 0x21,
	MIPI_DSI_RX_DCS_SHORT_READ_RESPONSE_2BYTE	= 0x22,
};

/* MIPI DCS commands */
enum {
	MIPI_DCS_NOP					= 0x00,
	MIPI_DCS_SOFT_RESET				= 0x01,
	MIPI_DCS_GET_DISPLAY_ID				= 0x04,
};

#endif /* __MIPI_DSI_CMD_H__ */
