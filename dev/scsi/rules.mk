LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/scsi.c \
	$(LOCAL_DIR)/ufs.c

include make/module.mk

