/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

/*
 * uart_simple.h
 *
 *  Created on: 2018. 1. 5.
 *      Author: sanguk7.park
 */

#ifndef UART_UART_SIMPLE_H_
#define UART_UART_SIMPLE_H_

extern unsigned int uart_log_mode;

void uart_console_init(void);
void uart_char_in(char *cData);
void uart_char_out(char cData);

#define uart_test_function(void) uart_console_init(void)
#define uart_simple_char_in(void) uart_char_in(void)
#define uart_simple_char_out(void) uart_char_out(void)

#endif /* UART_UART_SIMPLE_H_ */
