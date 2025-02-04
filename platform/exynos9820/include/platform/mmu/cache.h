/*
 * ==============================================================================
 *
 *       Filename:  cache.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2014-05-25
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Kim, Young-san (ys0225.kim@samsung.com)
 *   Organization:  AP Development, System LSI Business
 *
 * ==============================================================================
 */

#ifndef CACHE_H_
#define CACHE_H_

#include <stdio.h>
#include "types.h"

extern bool is_enable_dcache(void);

#ifdef CONFIG_SYS_CACHELINE_SIZE
#define ARCH_DMA_MINALIGN	CONFIG_SYS_CACHELINE_SIZE
#else
#define ARCH_DMA_MINALIGN	64
#endif

extern void enable_icache(void);
extern void disable_icache(void);
extern void enable_dcache(void);
extern void disable_dcache(void);

/*
 * I-CACHE operations
 */
extern void invalidate_icache_all_is(void);
extern void invalidate_icache_all(void);
extern void invalidate_icache_va(void *);

/*
 * D-CACHE operations
 */
extern void invalidate_dcache_all(void);
extern void invalidate_dcache_level(u64 level);
extern void invalidate_dcache_range(void *start, void *end);

extern void clean_dcache_all(void);
extern void clean_dcache_level(u64 level);
extern void clean_dcache_range(void *start, void *end);

extern void clean_invalidate_dcache_all(void);
extern void clean_invalidate_dcache_level(u64 level);
extern void clean_invalidate_dcache_range(void *start, void *end);

#endif
