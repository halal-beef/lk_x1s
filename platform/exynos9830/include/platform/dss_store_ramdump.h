#ifndef _DEBUG_STORE_RAMDUMP_H_
#define _DEBUG_STORE_RAMDUMP_H_

#include <platform/exynos9830.h>

#define RAMDUMP_STORE_MAGIC	(0xCAFEBABA)
#define METADATA_OFFSET		(0)
#define METADATA_SIZE		(512 * 8)
#define RAMDUMP_OFFSET		METADATA_SIZE
#define DRAM_WRITE_SIZE_DEFAULT	(0x80000000UL)

/* Store ramdump's metadata data format */
#pragma pack(push, 1)
union store_ramdump_metadata {
	struct _metadata {
		unsigned int magic;
		unsigned int flag_data_to_storage;
		unsigned long long dram_size;
		unsigned long long dram_start_addr;
		char file_name[512];
	} data;
	char reserved[METADATA_SIZE];
};
#pragma pack(pop)

int debug_store_ramdump(void);
int debug_store_ramdump_redirection(void *ptr);
int debug_store_ramdump_oem(const char *cmd);
void debug_store_ramdump_getvar(const char *name, char *response);
#endif
