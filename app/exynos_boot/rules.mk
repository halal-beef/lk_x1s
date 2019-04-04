LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/exynos_boot.c \
	$(LOCAL_DIR)/boot/cmd_boot.c \
	$(LOCAL_DIR)/boot/cmd_display.c \
	$(LOCAL_DIR)/boot/cmd_scatter_load_boot.c

include make/module.mk