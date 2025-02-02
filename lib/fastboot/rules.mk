LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/fastboot_cmd.c \
	$(LOCAL_DIR)/decompress_ext4.c

MODULE_DEPS += app/exynos_boot

include make/module.mk
