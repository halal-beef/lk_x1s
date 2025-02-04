#ifndef __ASM_BARRIER_H__
#define __ASM_BARRIER_H__

#ifndef __ASSEMBLY__

#define sev()       asm volatile("sev" : : : "memory")
#define wfe()       asm volatile("wfe" : : : "memory")
#define wfi()       asm volatile("wfi" : : : "memory")

#define isb()       asm volatile("isb" : : : "memory")
#define dsb()       asm volatile("dsb sy" : : : "memory")
#define dmb()       asm volatile("dmb sy" : : : "memory")

#define mb()        dsb()
#define rmb()       asm volatile("dsb ld" : : : "memory")
#define wmb()       asm volatile("dsb st" : : : "memory")

#ifndef CONFIG_SMP
#define smp_mb()    asm volatile("dmb sy" : : : "memory")
#define smp_rmb()   asm volatile("dmb sy" : : : "memory")
#define smp_wmb()   asm volatile("dmb sy" : : : "memory")
#else
#define smp_mb()    asm volatile("dmb ish" : : : "memory")
#define smp_rmb()   asm volatile("dmb ishld" : : : "memory")
#define smp_wmb()   asm volatile("dmb ishst" : : : "memory")
#endif

#define nop()       asm volatile("nop")

#endif /* __ASSEMBLY __ */
#endif /* __ASM_BARRIER_H__ */
