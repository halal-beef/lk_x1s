/*
 * Copyright (c) 2006-2015 Travis Geiselbrecht
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */
#include <new.h>
#include <lk/debug.h>
#include <lib/heap.h>

void *operator new(size_t s)
{
    return malloc(s);
}

void *operator new[](size_t s)
{
    return malloc(s);
}

void *operator new(size_t , void *p)
{
    return p;
}

void operator delete(void *p)
{
    return free(p);
}

void operator delete[](void *p)
{
    return free(p);
}

