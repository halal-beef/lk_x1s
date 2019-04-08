LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/pmucal_rae.c \
	$(LOCAL_DIR)/pmucal_local.c

include make/module.mk

