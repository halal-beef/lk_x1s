ARCH := arm64
ARM_CPU := cortex-a53
TARGET := universal9630

WITH_KERNEL_VM := 0
WITH_LINKER_GC := 1

MODULES += \
	app/exynos_main

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
	lib/lock \
	lib/fastboot \
	dev/scsi \
	dev/rpmb \
	dev/acpm_ipc \
	dev/debug \
	dev/adc \

GLOBAL_DEFINES += \
	INPUT_GPT_AS_PT=1 \
	FIRST_GPT_VERIFY=0\
	GPT_PART=1 \
	BOOT_IMG_HDR_V2=1

export INPUT_GPT_AS_PT=1
export GPT_PART=1
