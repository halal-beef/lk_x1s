LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_DEPS += dev/usb \
	dev/usb/phy \
	dev/usb/device

MODULE_SRCS += \
	$(LOCAL_DIR)/dwc3-global.c \
	$(LOCAL_DIR)/dwc3-dev.c

include make/module.mk
