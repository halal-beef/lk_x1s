LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_DEPS += dev/usb/device

MODULE_SRCS += \
	$(LOCAL_DIR)/fastboot-device.c

include make/module.mk
