LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/display.c \
	$(LOCAL_DIR)/fastboot_menu.c \
	$(LOCAL_DIR)/keys.c \

include make/module.mk
