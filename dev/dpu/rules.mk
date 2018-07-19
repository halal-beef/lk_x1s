LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/decon_core.c \
	$(LOCAL_DIR)/dpp_drv.c \
	$(LOCAL_DIR)/dsim_drv.c \

include make/module.mk

