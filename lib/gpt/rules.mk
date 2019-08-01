LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_DEPS += \
	lib/sysparam

GLOBAL_DEFINES += \
	SYSPARAM_ALLOW_WRITE=1
ifeq (1,$(INPUT_GPT_AS_PT))
MODULE_SRCS += \
	$(LOCAL_DIR)/gpt.c \
	$(LOCAL_DIR)/guid.c
else
MODULE_SRCS += \
	$(LOCAL_DIR)/pit.c \
	$(LOCAL_DIR)/part_gpt.c \
	$(LOCAL_DIR)/guid.c
endif
MODULE_SRCS += $(LOCAL_DIR)/part.c

include make/module.mk
