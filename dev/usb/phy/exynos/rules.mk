LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_DEPS += dev/usb/phy

MODULE_SRCS += \
	$(LOCAL_DIR)/phy-exynos-usb.c \
	$(LOCAL_DIR)/phy-exynos-usb3p1.c

include make/module.mk
