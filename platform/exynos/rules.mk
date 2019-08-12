LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := ${LOCAL_DIR}

MODULE_SRCS += \
	$(LOCAL_DIR)/boot_info.c \
	$(LOCAL_DIR)/gpio.c \
	$(LOCAL_DIR)/timer/delay.c

include make/module.mk
