ARCH              = arm64
TARGET            = aarch64-linux-gnu
CROSS_COMPILE     = $(TARGET)-
CC                = $(CROSS_COMPILE)gcc
OBJCOPY           = $(CROSS_COMPILE)objcopy

all: bootshim.elf bootshim.bin

bootshim.bin: bootshim.elf
	$(OBJCOPY) -O binary $< $@

bootshim.elf: bootshim.S
	$(CC) -c $< -o $@ -DCREATE_FDT_POINTER=$(CREATE_FDT_POINTER) -DFDT_POINTER_ADDRESS=$(FDT_POINTER_ADDRESS) -DLK3RD_BASE=$(LK3RD_BASE) -DLK3RD_SIZE=0x200000

bootshim.S:
