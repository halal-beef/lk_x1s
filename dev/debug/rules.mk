LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/dfd.c \
	$(LOCAL_DIR)/dss.c \
	$(LOCAL_DIR)/dss_store_ramdump.c

include make/module.mk
