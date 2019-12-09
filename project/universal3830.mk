ARCH := arm64
ARM_CPU := cortex-a53
TARGET := universal3830

WITH_KERNEL_VM := 0
WITH_LINKER_GC := 1

MODULES += \
	app/exynos_main \
	dev/acpm_ipc \
	dev/mmc

MODULE_DEPS += \
	platform/exynos \
	lib/gpt \
	lib/font \
	lib/logo \
	lib/libavb \
	lib/lock \
	lib/fastboot \
	lib/fdtapi \
	dev/rpmb \
	external/lib/fdt \
	external/lib/ufdt \
	dev/adc \
	dev/debug \
	dev/power/pmic/s2mpu12 \
	dev/power/pmic/s2mu106 \

GLOBAL_DEFINES += \
	INPUT_GPT_AS_PT=1 \
	FIRST_GPT_VERIFY=0 \
	GPT_PART=1 \
	BOOT_IMG_HDR_V2=1

export INPUT_GPT_AS_PT=1
export GPT_PART=1
