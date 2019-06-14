ARCH := arm64
ARM_CPU := cortex-a53
TARGET := universal9830

WITH_KERNEL_VM := 0
WITH_LINKER_GC := 1

GLOBAL_DEFINES += \
	SYSPARAM_ALLOW_WRITE=1 \
	WITH_LIB_CONSOLE=1

MODULES += \
	app/exynos_boot

MODULE_DEPS += \
	lib/gpt \
	lib/font \
	lib/logo \
	lib/libavb \
	lib/cksum \
	lib/fdt \
	lib/ufdt \
	lib/ab_update \
	lib/lock \
	lib/fastboot \
	dev/scsi \
	dev/mmc \
	dev/rpmb \
	dev/boot \
	dev/dpu \
	dev/adc \
	dev/pmucal \
	dev/speedy \
	dev/power/pmic/s2mps_19_22
