LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/exynos_hsi2c.c \
	$(LOCAL_DIR)/exynos_i2c.c

include make/module.mk

