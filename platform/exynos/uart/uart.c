/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <lk/debug.h>
#include <lk/err.h>
#include <lk/compiler.h>
#include <sys/types.h>
#include <platform/sfr.h>
#include <platform/uart.h>

#define Outp32(addr, data) (*(volatile unsigned int *)((unsigned long) addr) = (data))
#define Set32(addr, data) (*(volatile unsigned int *)((unsigned long) addr) = (data))
#define Get32(addr) (*(volatile unsigned int *)((unsigned long)addr))

#define SetBits(uAddr, uBaseBit, uMaskValue, uSetValue) \
	Set32(uAddr, (Get32(uAddr) & ~((uMaskValue)<<(uBaseBit))) | (((uMaskValue)&(uSetValue))<<(uBaseBit)))

#define rUART_BASE				EXYNOS_UART_BASE

#define rUART_ULCONN               0x00
#define rUART_UCONN                0x04
#define rUART_UFCONN               0x08
#define rUART_UMCONN               0x0C
#define rUART_UTRSTATN             0x10
#define rUART_UERSTATN             0x14
#define rUART_UFSTATN              0x18
#define rUART_UMSTATN              0x1C
#define rUART_UTXHN                0x20
#define rUART_URXHN                0x24
#define rUART_UBRDIVN              0x28
#define rUART_UFRACVAL             0x2C
#define rUART_UINTPN               0x30
#define rUART_UINTSPN              0x34
#define rUART_UINTMN               0x38
#define rUART_USI_CON              0xc4

#ifndef UART_SRCCLK
#define UART_SRCCLK	133250000
#endif
#ifndef UART_BAUDRATE
#define UART_BAUDRATE 115200
#endif

unsigned int globalUartBase;
unsigned int uart_log_mode = 0;

static void uart_GPIOInit(void);
static void uart_UartInit(unsigned int uUartBase, unsigned int clk, unsigned int nBaudrate);
static void itoa_base_custom(unsigned int number, unsigned int uBaseUnit, unsigned int uUnitWidth, char *Converted);
static void UART_WaitForRxReady(void);
static void UART_WaitForTxEmpty(void);
static void uart_string_out(const char * string);

/* This is how to use sample */
void uart_console_init(void)
{
	char array[9]={0};
	uart_GPIOInit();
	uart_UartInit(rUART_BASE, UART_SRCCLK, UART_BAUDRATE);

	uart_string_out("Done\nfor\nsure.\n");

	itoa_base_custom(rUART_BASE, 0x10, 8, array);
	uart_string_out(array);
	uart_string_out("\n");

	itoa_base_custom(0x1234, 0x10, 4, array);
	uart_string_out(array);
	uart_string_out("\n");
}

/* GPIO configure */
static void uart_GPIOInit(void)
{
	SetBits(UARTGPIO_CON, UARTGPIO_CON_BASE_BIT, UARTGPIO_CON_MASK, UARTGPIO_CON_SET);
}

/* UART USI initialize code */
static void uart_UartInit(unsigned int uUartBase, unsigned int clk, unsigned int nBaudrate)
{
	return;
}

static void itoa_base_custom(unsigned int number, unsigned int uBaseUnit, unsigned int uUnitWidth, char *Converted)
{
    unsigned int n;

    Converted += uUnitWidth;
    *Converted = '\0';

    for (n = uUnitWidth; n != 0; --n) {
        *--Converted = "0123456789ABCDEF"[number % uBaseUnit];
        number /= uBaseUnit;
    }
}

static void UART_WaitForRxReady(void)
{
#define RX_BUF_READY (1 << 0)	/* Rx buffer data ready */

	unsigned int uTxRxStatus;

	do {
		uTxRxStatus = Get32(globalUartBase + rUART_UTRSTATN) & 0x7;
	} while (!(uTxRxStatus & RX_BUF_READY));
}

static void UART_WaitForTxEmpty(void)
{
#define TX_BUF_EMPTY (1 << 1)	/* Tx buffer register empty */

	unsigned int uTxRxStatus;

	do {
		uTxRxStatus = Get32(globalUartBase + rUART_UTRSTATN) & 0x7;
	} while (!(uTxRxStatus & TX_BUF_EMPTY));
}

void uart_char_in(char *cData)
{
	return;
}

void uart_char_out(char cData)
{
     UART_WaitForTxEmpty();
     Outp32(globalUartBase+rUART_UTXHN, cData);
	 if(cData == '\n')
		 platform_dputc('\r');
}

static void uart_string_out(const char * string)
{
	while(*string)
	{
		if(*string=='\n')
		{
			string++;
			platform_dputc('\r');
			platform_dputc('\n');
		}
		else
			platform_dputc(*string++);
	}
}
