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


#ifndef __DISPLAY_SFR_H__
#define __DISPLAY_SFR_H__


/* DMA & DPP SFR Base Address */
#define DPP_IDMAG0_BASE_ADDR		0x19041000
#define DPP_IDMAG0_DMA_ADDR		0x19091000
#define DPP_IDMAG0_DMA_COM_ADDR		0x19090000

#define DPP_IDMAVGRFS_BASE_ADDR		0x19042000
#define DPP_IDMAVGRFS_DMA_ADDR		0x19092000

#define DPP_IDMAG1_BASE_ADDR		0x19043000
#define DPP_IDMAG1_DMA_ADDR		0x19093000

/* DECON SFR Base Address */
#define DECON0_BASE_ADDR		0x19050000
#define SYSMMU_DPU0_BASE_ADDR		0x19100000
#define SYSMMU_DPU1_BASE_ADDR		0x19110000
#define SYSMMU_DPU2_BASE_ADDR		0x19120000
#define DPU_SYSREG_BASE_ADDR		0x19021000

/* DSIM & PHY SFR Base Address */
#define DSIM0_BASE_ADDR			0x190C0000
#define DPHY_BASE_ADDR			0x190E0100
#define DPHY_EX_BASE_ADDR		0x190E0000

#endif /* __DISPLAY_SFR_H__ */
