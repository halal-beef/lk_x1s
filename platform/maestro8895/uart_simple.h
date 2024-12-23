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


void uart_simple_GPIOInit(void);
void uart_simple_UartInit(unsigned int uUartBase, unsigned int clk, unsigned int nBaudrate);
void uart_simple_string_out(const char * string);
void itoa_base_custom(unsigned int number, unsigned int uBaseUnit, unsigned int uUnitWidth, char *Converted);
void uart_test_function(void);
void uart_simple_char_in(char *cData);
void uart_simple_char_out(char cData);

#endif /* UART_UART_SIMPLE_H_ */
