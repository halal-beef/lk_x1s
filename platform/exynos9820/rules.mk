LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := ${LOCAL_DIR}

WITH_SMP := 0

MODULE_SRCS += \
	$(LOCAL_DIR)/platform.c \
	$(LOCAL_DIR)/debug.c \
	$(LOCAL_DIR)/mmu/cpu_a.S \
	$(LOCAL_DIR)/mmu/mmu.c \
	$(LOCAL_DIR)/usb/usb.c \

MODULE_DEPS += \
   	dev/interrupt/arm_gic \
	dev/timer/arm_generic \
	dev/usb/dwc3 \
	dev/usb/phy/exynos \
	dev/usb/device/fastboot \
	lib/font \

LINKER_SCRIPT += $(BUILDDIR)/system-onesegment.ld

GLOBAL_COMPILEFLAGS += -mfix-cortex-a53-835769
GLOBAL_COMPILEFLAGS += -mfix-cortex-a53-843419

include make/module.mk
#include $(LOCAL_DIR)/dpu/rules.mk
