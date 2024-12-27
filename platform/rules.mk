LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

# shared platform code
MODULE_SRCS += \
	$(LOCAL_DIR)/debug.c \
	$(LOCAL_DIR)/init.c \
	$(LOCAL_DIR)/power.c

# lk3rd platform code
MODULE_DEPS += \
	lib/lk3rd \
	lib/version

include make/module.mk


