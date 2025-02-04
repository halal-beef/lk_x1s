LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

PLATFORM := exynos9820

MEMBASE := 0xe1900000
MEMSIZE := 0x00F00000

# BootShim
CREATE_FDT_POINTER := 0
FDT_POINTER_ADDRESS := 0x0

BOOTLOADER_FB_ADDRESS := 0xca000000
LCD_WIDTH := 1440
LCD_HEIGHT := 3040

GLOBAL_DEFINES += \
        FDT_POINTER_ADDRESS=$(FDT_POINTER_ADDRESS)

# Boot image creation arguments
MKBOOTIMG_ARGS := \
                 --tags_offset 0x00000100 \
                 --second_offset 0xf0000000 \
                 --pagesize 2048 \
                 --os_version "13.0.0" \
                 --os_patch_level "2099-12" \
                 --kernel_offset 0x00008000 \
                 --header_version 1 \
                 --dtb_offset 0x00000000 \
                 --cmdline "LK3RD BOOT IMAGE" \
                 --board "" \
                 --base 0x10000000

MODULE_SRCS += \
	$(LOCAL_DIR)/target.c

include make/module.mk
