ARCH := arm64
ARM_CPU := cortex-a53
TARGET := universal9630

WITH_KERNEL_VM := 0
WITH_LINKER_GC := 1

MODULES += \
	app/exynos_main

MODULE_DEPS += \
	lib/gpt \
	lib/libavb \
	lib/cksum \
	lib/fdt \
	lib/ufdt \
	lib/lock \
	lib/fastboot \
	dev/scsi \
	dev/rpmb \
	dev/boot \
	dev/acpm_ipc

GLOBAL_DEFINES += \
	INPUT_GPT_AS_PT=1 \
	FIRST_GPT_VERIFY=0
export INPUT_GPT_AS_PT=1