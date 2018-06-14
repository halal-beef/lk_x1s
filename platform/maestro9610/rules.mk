LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := ${LOCAL_DIR}

WITH_SMP := 0

MODULE_SRCS += \
    $(LOCAL_DIR)/platform.c \
	$(LOCAL_DIR)/debug.c \
	$(LOCAL_DIR)/uart_simple.c \
	$(LOCAL_DIR)/delay.c \
	$(LOCAL_DIR)/ufs.c \
	$(LOCAL_DIR)/ufs-cal-9610.c \
	$(LOCAL_DIR)/fastboot/usbd3-ss.c \
	$(LOCAL_DIR)/fastboot/phy-exynos-usb3p1.c \
	$(LOCAL_DIR)/fastboot/fastboot_cmd.c \
	$(LOCAL_DIR)/fastboot/decompress_ext4.c \
	$(LOCAL_DIR)/boot/cmd_scatter_load_boot.c \
	$(LOCAL_DIR)/boot/cmd_boot.c \
	$(LOCAL_DIR)/security/smc_aarch64.c \
	$(LOCAL_DIR)/security/ldfw.c \
	$(LOCAL_DIR)/speedy/speedy.c \
	$(LOCAL_DIR)/pmic/pmic_s2mpu09.c \
	$(LOCAL_DIR)/pmic/fg_s2mu004.c \
	$(LOCAL_DIR)/pmic/if_pmic_s2mu004.c \
	$(LOCAL_DIR)/gpio_init.S \

MODULE_DEPS += \
    dev/interrupt/arm_gic \
	dev/timer/arm_generic

LINKER_SCRIPT += $(BUILDDIR)/system-onesegment.ld

include make/module.mk
