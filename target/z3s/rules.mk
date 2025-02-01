LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

PLATFORM := exynos9830

MEMBASE := 0xe1900000
MEMSIZE := 0x00F00000

# BootShim
CREATE_FDT_POINTER := 1
FDT_POINTER_ADDRESS := 0x95e00000

BOOTLOADER_FB_ADDRESS := 0xf1000000
LCD_WIDTH := 1440
LCD_HEIGHT := 3200

# Boot image creation arguments
MKBOOTIMG_ARGS := \
                 --dtb $(BUILDROOT)/Resources/DTBs/exynos9830.dtb \
                 --tags_offset 0x00000100 \
                 --second_offset 0xf0000000 \
                 --pagesize 2048 \
                 --os_version "13.0.0" \
                 --os_patch_level "2099-12" \
                 --kernel_offset 0x00008000 \
                 --header_version 2 \
                 --dtb_offset 0x00000000 \
                 --cmdline "LK3RD BOOT IMAGE" \
                 --board "" \
                 --base 0x10000000

MODULE_SRCS += \
	$(LOCAL_DIR)/target.c \
	$(LOCAL_DIR)/dpu_io/dpu_gpio.S \
	$(LOCAL_DIR)/dpu_io/dpu_io_ctrl.c \
	$(LOCAL_DIR)/dpu_panels/s6e3fa0_lcd_ctrl.c \
	$(LOCAL_DIR)/dpu_panels/s6e3fa0_mipi_lcd.c \
	$(LOCAL_DIR)/dpu_panels/nt36672a_lcd_ctrl.c \
	$(LOCAL_DIR)/dpu_panels/nt36672a_mipi_lcd.c \
	$(LOCAL_DIR)/dpu_panels/s6e3ha9_lcd_ctrl.c \
	$(LOCAL_DIR)/dpu_panels/s6e3ha9_mipi_lcd.c \
	$(LOCAL_DIR)/dpu_panels/lcd_module.c \

include make/module.mk
