LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := ${LOCAL_DIR}

GLOBAL_DEFINES += \
	$(LOCAL_DIR)/include

include make/module.mk
