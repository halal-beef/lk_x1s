LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

PLATFORM := exynos3830

MEMBASE := 0xF8800000
MEMSIZE := 0x01800000

MODULE_SRCS += \
	$(LOCAL_DIR)/target.c \
	$(LOCAL_DIR)/dpu_io/dpu_gpio.S \
	$(LOCAL_DIR)/dpu_io/dpu_io_ctrl.c \
	$(LOCAL_DIR)/dpu_panels/td4150_lcd_ctrl.c \
	$(LOCAL_DIR)/dpu_panels/td4150_mipi_lcd.c \
	$(LOCAL_DIR)/dpu_panels/ea8076_lcd_ctrl.c \
	$(LOCAL_DIR)/dpu_panels/ea8076_mipi_lcd.c \
	$(LOCAL_DIR)/dpu_panels/s6e3fa0_lcd_ctrl.c \
	$(LOCAL_DIR)/dpu_panels/s6e3fa0_mipi_lcd.c \
	$(LOCAL_DIR)/dpu_panels/nt36672a_lcd_ctrl.c \
	$(LOCAL_DIR)/dpu_panels/nt36672a_mipi_lcd.c \
	$(LOCAL_DIR)/dpu_panels/s6e3ha8_lcd_ctrl.c \
	$(LOCAL_DIR)/dpu_panels/s6e3ha8_mipi_lcd.c \
	$(LOCAL_DIR)/dpu_panels/s6e3ha9_lcd_ctrl.c \
	$(LOCAL_DIR)/dpu_panels/s6e3ha9_mipi_lcd.c \
	$(LOCAL_DIR)/dpu_panels/lcd_module.c \
	$(LOCAL_DIR)/pmic/pmic.c \
	$(LOCAL_DIR)/gpio/exynos_key.c \

include make/module.mk
