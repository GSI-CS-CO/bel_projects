/*
 * DSI Shield
 *
 * Copyright (C) 2013-2014 twl
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* uart.c - simple UART driver */

#include <stdint.h>

#include "uart.h"
#include "board.h"

#include <hw/wb_uart.h>

PACKED struct UART_WB {
	/* [0x0]: REG Status Register */
	uint32_t SR;
	/* [0x4]: REG Baudrate control register */
	uint32_t BCR;
	/* [0x8]: REG Transmit data regsiter */
	uint32_t TDR;
	/* [0xc]: REG Receive data regsiter */
	uint32_t RDR;
};

#define CALC_BAUD(baudrate) \
    ( ((( (unsigned int)baudrate << 12)) + \
      (BASE_CLOCK >> 8)) / (BASE_CLOCK >> 7) )

volatile struct UART_WB *uart;

void uart_init_hw()
{
	//uart = (volatile struct UART_WB *)BASE_UART;
	uart = (volatile struct UART_WB *)0x80040500;

#ifndef SIMULATION
	//uart->BCR = CALC_BAUD(00020000);
  //uart->BCR = 0x00060000;
//#warning "No Simulation"
#else
	uart->BCR = CALC_BAUD((CPU_CLOCK/10));
//#warning "Simulation"
#endif

}

void uart_write_byte(int b)
{
	if (b == '\n')
		uart_write_byte('\r');
	while (uart->SR & UART_SR_TX_BUSY)
		;
	uart->TDR = b;
}

int uart_poll()
{
	return uart->SR & UART_SR_RX_RDY;
}

int uart_read_byte()
{
	if (!uart_poll())
		return -1;

	return uart->RDR & 0xff;
}
int puts(const char *s)
{
  char c;
  while(c=*s++)
    uart_write_byte(c);
}
