LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
    $(LOCAL_DIR)/ufdt_convert.c \
    $(LOCAL_DIR)/ufdt_node.c \
    $(LOCAL_DIR)/ufdt_overlay.c \
    $(LOCAL_DIR)/ufdt_prop_dict.c \
    $(LOCAL_DIR)/libufdt_sysdeps_posix.c

MODULE_COMPILEFLAGS += -Wno-sign-compare

include make/module.mk
