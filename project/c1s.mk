ARCH := arm64
ARM_CPU := cortex-a53
TARGET := c1s

WITH_KERNEL_VM := 0
WITH_LINKER_GC := 1

GLOBAL_DEFINES += \
	SYSPARAM_ALLOW_WRITE=1 \
	WITH_LIB_CONSOLE=1 \
	INPUT_GPT_AS_PT=0 \
	GPT_PART=0
export INPUT_GPT_AS_PT=0
export GPT_PART=0

MODULES += \
	app/exynos_boot

MODULE_DEPS += \
	platform/exynos \
	lib/gpt \
	lib/font \
	lib/logo \
	lib/libavb \
	lib/cksum \
	lib/fdt \
	lib/ufdt \
	lib/fdtapi \
	lib/ab_update \
	lib/lock \
	lib/fastboot \
	lib/block \
	dev/scsi \
	dev/rpmb \
	dev/adc \
	dev/pmucal \
	dev/speedy \
	dev/power/pmic/s2mps_19_22 \
	dev/power/pmic/s2dos05 \
	dev/battery/charger \
	dev/debug \
