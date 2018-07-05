LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

PLATFORM := maestro9610

MEMBASE := 0xFF000000
MEMSIZE := 0x00F00000

MODULE_SRCS += \
	$(LOCAL_DIR)/target.c \

include make/module.mk
