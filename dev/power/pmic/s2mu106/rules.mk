LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/if_pmic_s2mu106.c \
	$(LOCAL_DIR)/fg_s2mu106.c

include make/module.mk

