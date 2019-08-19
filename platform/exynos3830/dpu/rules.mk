LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/decon_core.c \
	$(LOCAL_DIR)/dpp_drv.c \
	$(LOCAL_DIR)/dsim_drv.c \
	$(LOCAL_DIR)/format.c \
	$(LOCAL_DIR)/cal/decon_reg.c \
    $(LOCAL_DIR)/cal/dpp_reg.c \
    $(LOCAL_DIR)/cal/dsim_reg.c \

include make/module.mk

