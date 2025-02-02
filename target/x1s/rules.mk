LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

PLATFORM := exynos9830

MEMBASE := 0x90000000
MEMSIZE := 0x00F00000

BOOTLOADER_FB_ADDRESS := 0xf1000000
LCD_WIDTH := 1440
LCD_HEIGHT := 3200

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
