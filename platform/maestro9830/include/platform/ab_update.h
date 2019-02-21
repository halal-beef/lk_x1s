/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
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

#ifndef __AB_UPDATE_H__
#define __AB_UPDATE_H__

typedef struct ExynosSlotInfo {
	// Flag mean that the slot can bootable or not.
	uint8_t bootable :1;

	// Set to true when the OS has successfully booted.
	uint8_t boot_successful :1;

	// Priorty number range [0:15], with 15 meaning highest priortiy,
	// 1 meaning lowest priority and 0 mean that the slot is unbootable.
	uint8_t priority;

	// Boot times left range [0:7].
	uint8_t tries_remaining;

	uint8_t reserved[1];
} ExynosSlotInfo;

typedef struct ExynosBootInfo {
	// Magic for identification
	uint8_t magic[3];

	// Version of ExynosBootInfo struct, must be 0 or larger.
	uint8_t version;

	// Information about each slot.
	ExynosSlotInfo slot_info[2];

	uint8_t reserved[20];
} ExynosBootInfo;

#if (__STDC_VERSION__ >= 201112L) || defined(__cplusplus)
static_assert(sizeof(struct ExynosBootInfo) == 32);
#endif

int ab_set_active(int slot);
int ab_current_slot(void);
int ab_slot_successful(int slot);
int ab_slot_unbootable(int slot);
int ab_slot_retry_count(int slot);

#endif	/* __AB_UPDATE_H__ */
