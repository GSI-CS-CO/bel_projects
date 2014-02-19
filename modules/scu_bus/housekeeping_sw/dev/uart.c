#include <inttypes.h>

#include "board.h"
#include "uart.h"

#include <hw/wb_vuart.h>

#define CALC_BAUD(baudrate) \
    ( ((( (unsigned long long)baudrate * 8ULL) << (16 - 7)) + \
      (CPU_CLOCK*125/32)) / (CPU_CLOCK*125/16) )

volatile struct UART_WB *uart = (volatile struct UART_WB *)BASE_UART;

void uart_init()
{
	uart->BCR = CALC_BAUD(UART_BAUDRATE);
}

void uart_write_byte(int b)
{
	if(b == '\n')
		uart_write_byte('\r');
	while(uart->SR & UART_SR_TX_BUSY)
		;
	uart->TDR = b;
}

void uart_write_string(char *s)
{
	while (*s)
		uart_write_byte(*(s++));
}

int uart_poll()
{
 	return uart->SR & UART_SR_RX_RDY;
}

int uart_read_byte()
{
 	return uart ->RDR & 0xff;
}
