ARCH := arm64
ARM_CPU := cortex-a53
TARGET := beyond2lte

WITH_KERNEL_VM := 0
WITH_LINKER_GC := 1

GLOBAL_DEFINES += \
	SYSPARAM_ALLOW_WRITE=1 \
	WITH_LIB_CONSOLE=1 \
	INPUT_GPT_AS_PT=0 \
	GPT_PART=0 \
	BOOTLOADER_FB_ADDRESS=0xca000000 \
	LCD_WIDTH=1440 \
	LCD_HEIGHT=3040
export INPUT_GPT_AS_PT=0
export GPT_PART=0

MODULES += \
	app/exynos_boot

MODULE_DEPS += \
	platform/exynos \
	lib/font \
	lib/logo \
	lib/cksum \
	lib/fdt \
	lib/ufdt \
	lib/lock \
	lib/block \
	lib/gpt \
