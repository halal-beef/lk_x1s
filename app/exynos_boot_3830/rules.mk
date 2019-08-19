LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/exynos_boot.c \
	$(LOCAL_DIR)/recovery.c \
	$(LOCAL_DIR)/drex_v3_3.c \
	$(LOCAL_DIR)/almighty.c	\
	$(LOCAL_DIR)/mct.c \
	$(LOCAL_DIR)/xct.c

include make/module.mk
