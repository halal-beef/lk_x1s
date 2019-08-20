LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := ${LOCAL_DIR}

WITH_SMP := 0

MODULE_SRCS += \
    $(LOCAL_DIR)/platform.c \
	$(LOCAL_DIR)/debug.c \
	$(LOCAL_DIR)/uart_simple.c \
	$(LOCAL_DIR)/delay.c \
	$(LOCAL_DIR)/ufs.c \
	$(LOCAL_DIR)/ufs-cal-9830.c \
	$(LOCAL_DIR)/usb/usbd3-ss.c \
	$(LOCAL_DIR)/usb/phy-exynos-usb3p1.c \
	$(LOCAL_DIR)/security/smc_aarch64.c \
	$(LOCAL_DIR)/security/ldfw.c \
	$(LOCAL_DIR)/security/avb_ops.c \
	$(LOCAL_DIR)/security/avb_main.c \
	$(LOCAL_DIR)/security/sb_api.c \
	$(LOCAL_DIR)/security/cm_api.c \
	$(LOCAL_DIR)/security/otp_v20.c \
	$(LOCAL_DIR)/security/h-arx.c \
	$(LOCAL_DIR)/security/exynos-hvc.S \
	$(LOCAL_DIR)/gpio_init.S \
	$(LOCAL_DIR)/gpio.c \
	$(LOCAL_DIR)/fdt.c \
	$(LOCAL_DIR)/flexpmu_dbg.c \
	$(LOCAL_DIR)/tmu.c \
	$(LOCAL_DIR)/mmc.c \
	$(LOCAL_DIR)/dw_mmc.c \
	$(LOCAL_DIR)/mmu/cpu_a.S \
	$(LOCAL_DIR)/mmu/mmu.c \
	$(LOCAL_DIR)/debug/dfd.c \
	$(LOCAL_DIR)/debug/dfd_cache_flush.c \
	$(LOCAL_DIR)/debug/dfd_asm_helper.S \
	$(LOCAL_DIR)/debug/dfd_verify.c \
	$(LOCAL_DIR)/debug/dss.c \
	$(LOCAL_DIR)/debug/dss_store_ramdump.c \
	$(LOCAL_DIR)/dvfs_info.c

MODULE_DEPS += \
    dev/interrupt/arm_gic \
	dev/timer/arm_generic

LINKER_SCRIPT += $(BUILDDIR)/system-onesegment.ld

GLOBAL_COMPILEFLAGS += -mfix-cortex-a53-835769
GLOBAL_COMPILEFLAGS += -mfix-cortex-a53-843419

include make/module.mk
include $(LOCAL_DIR)/dpu/rules.mk
