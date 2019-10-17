LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := ${LOCAL_DIR}

WITH_SMP := 0

MODULE_SRCS += \
    $(LOCAL_DIR)/platform.c \
	$(LOCAL_DIR)/debug.c \
	$(LOCAL_DIR)/usb/usb.c \
	$(LOCAL_DIR)/boot/cmd_scatter_load_boot.c \
	$(LOCAL_DIR)/boot/cmd_boot.c \
	$(LOCAL_DIR)/boot/cmd_display.c \
	$(LOCAL_DIR)/security/smc_aarch64.c \
	$(LOCAL_DIR)/security/ldfw.c \
	$(LOCAL_DIR)/security/avb_ops.c \
	$(LOCAL_DIR)/security/avb_main.c \
	$(LOCAL_DIR)/security/sb_api.c \
	$(LOCAL_DIR)/security/cm_api.c \
	$(LOCAL_DIR)/security/otp_v20.c \
	$(LOCAL_DIR)/pmic/pmic_s2mpu12.c \
	$(LOCAL_DIR)/pmic/if_pmic_s2mu106.c \
	$(LOCAL_DIR)/pmic/fg_s2mu106.c \
	$(LOCAL_DIR)/ab_update/ab_update.c \
	$(LOCAL_DIR)/wdt/wdt_recovery.c \
	$(LOCAL_DIR)/dram_training/dram_training.c \
	$(LOCAL_DIR)/mmu/cpu_a.S \
	$(LOCAL_DIR)/mmu/mmu.c \
	$(LOCAL_DIR)/power.c \
	$(LOCAL_DIR)/tmu.c \
	$(LOCAL_DIR)/flexpmu_dbg.c \
	$(LOCAL_DIR)/mmc.c \
	$(LOCAL_DIR)/dw_mmc.c \
	$(LOCAL_DIR)/debug/dfd.c \
	$(LOCAL_DIR)/debug/dfd_asm_helper.S \
	$(LOCAL_DIR)/board_rev/board_rev.c \

MODULE_DEPS += \
	dev/interrupt/arm_gic \
	dev/timer/arm_generic \
	lib/cksum \
	dev/usb/dwc3 \
	dev/usb/phy/exynos \
	dev/usb/device/fastboot

LINKER_SCRIPT += $(BUILDDIR)/system-onesegment.ld

include make/module.mk
include $(LOCAL_DIR)/dpu/rules.mk
