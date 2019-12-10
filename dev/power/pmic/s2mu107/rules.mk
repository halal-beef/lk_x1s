LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/if_pmic_s2mu107.c \
	$(LOCAL_DIR)/fg_s2mu107.c

include make/module.mk

