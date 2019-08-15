LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_DEPS += dev/usb/device \
	lib/cbuf

GLOBAL_DEFINES += \
	CONSOLE_HAS_INPUT_BUFFER=1

MODULE_SRCS += \
	$(LOCAL_DIR)/cdc-acm.c

include make/module.mk
