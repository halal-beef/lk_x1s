#ifndef __ASM_ARM_TYPES_H
#define __ASM_ARM_TYPES_H

typedef unsigned short umode_t;
typedef enum {
	CPU_EL0 = 0,
	CPU_EL1 = 1,
	CPU_EL2 = 2,
	CPU_EL3 = 3,
} CPU_EL_LEVEL;

//#define CONFIG_STARTUP_EL_MODE  STARTUP_EL1

/*
 * __xx is ok: it doesn't pollute the POSIX namespace. Use these in the
 * header files exported to user space
 */

typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

#if defined(__GNUC__)
__extension__ typedef __signed__ long long __s64;
__extension__ typedef unsigned long long __u64;
#endif


/*
 * These aren't exported outside the kernel to avoid name space clashes
 */
//#ifdef __KERNEL__

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

#define BITS_PER_LONG 64

/* Dma addresses are 64-bits wide.  */

typedef u64 phys_addr_t;
typedef u64 phys_size_t;

//#endif /* __KERNEL__ */

#endif
