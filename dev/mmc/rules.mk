LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/mmc.c \
	$(LOCAL_DIR)/dw_mmc.c

include make/module.mk

