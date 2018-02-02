ARCH := arm64
ARM_CPU := cortex-a53
TARGET := maestro9610

WITH_KERNEL_VM := 0
WITH_LINKER_GC := 1

MODULES += \
	app/shell
