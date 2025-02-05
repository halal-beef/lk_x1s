/*
 * Copyright (c) 2008-2014 Travis Geiselbrecht
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */
#ifndef __APP_TESTS_H
#define __APP_TESTS_H

#include <lib/console.h>

int cbuf_tests(int argc, const cmd_args *argv);
int fibo(int argc, const cmd_args *argv);
int port_tests(void);
int spinner(int argc, const cmd_args *argv);
int thread_tests(void);
void benchmarks(void);
void clock_tests(void);
void printf_tests(void);
void printf_tests_float(void);

#endif

