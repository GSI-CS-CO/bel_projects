#include "softuart.h"
#include "fx2.h"
#include "fx2regs.h"

BYTE uart_state;
BYTE uart_rx_byte;
BYTE uart_tx_byte;
BYTE uart_bit_count;
BYTE uart_byte_to_receive;
BYTE uart_byte_received;
BYTE uart_byte_to_transmit;


void software_uart_init(void)
{
	OEA = 0xFE;		// 0 input, 1 output -> PA1 = TX, PA0 = RX
	IOA |= 0x02;	// tx pin (PA1) high setzen

	OED = 0xFF;		// for debugging on cypress development board
	IOD = 0xFF;		// pins not connected on exploder

	uart_rx_byte = 0;
	uart_tx_byte = 0;
	uart_bit_count = 0;
	uart_byte_received = 0;
	uart_byte_to_transmit = 0;
	uart_state = UART_STATE_IDLE;

	tmr0_init();	// init tmr0 for time base
	int0_init();	// init falling edge interrupt on PA0
}


void int0_init(void)
{
	PORTACFG = 0x01;	// enable int0 on pa0
	IT0 = 1;			// falling edge
	EX0 = 1;			// enable int0 interrupt
}

void tmr0_init(void)
{
	TCON = TCON & 0xEF;			// Stop tmr0
	TMOD = TMOD | 0x02;			// Timer 0 mode 2 -> 8 bit counter with autoreload
	TH0 = tmr0_preload_value;	// see softuart.h for definition
	CKCON |= 0x08;				// tmr0 runs with CLKOUT / 4

	ET0 = 1;					// Enable timer 0 interrupts

// for determining baud rate on development board
//	TCON = TCON | 0x10; // Timer run enable
}


void uart_transmit_byte(BYTE tx_byte_from_usb)
{
	uart_tx_byte = tx_byte_from_usb;
	uart_state = UART_STATE_TX_START;
	//uart_byte_to_transmit = 1;	// in next version uart_state wont be updated outside statemachine

	TL0 = 0xF0;	// set timer nearly to overflow for fast interrupt

	TCON = TCON | 0x10; // Timer run enable
}

void int0_isr(void) interrupt INT0_VECT
{
	if(uart_state == UART_STATE_IDLE) {
	// falling edge of start bit
		TL0 = tmr0_preload_start_bit;	// set timer to 0x98 = 26 us sample rate for reaching mid of start bit
		TCON = TCON | 0x10; // Timer run enable

		uart_state = UART_STATE_RX_START;
		PORTACFG = 0x00; // normal pin operation of pa0
	}
}


void tmr0_isr(void) interrupt TMR0_VECT
{
//	IOD--; can be measured on cypress development board


	switch(uart_state) {
		case UART_STATE_RX_START:
			if((IOA & 0x01) == 0x00)
			{
				PORTACFG = 0x00; // normal pin operation of pa0
				uart_state = UART_STATE_RX_GET_DATA;
				uart_bit_count = 0;	// reset bit count
				uart_rx_byte = 0x00; // reset rx byte
			}
			else
			{
				uart_state = UART_STATE_IDLE;
			}
			break;

		case UART_STATE_RX_GET_DATA:
			if(uart_bit_count < 8)
			{
				uart_rx_byte |= ((IOA & 0x01) << uart_bit_count);
				uart_bit_count++;
			}
			if(uart_bit_count == 8) // STOP 
			{
				PORTACFG = 0x01; // interrupt pin int0 on pa0 again
				uart_state = UART_STATE_IDLE;
				uart_byte_received = 1;
				TCON = TCON & 0xEF; // Timer run disable
			}
			break;

		case UART_STATE_RX_STOP:
			break;
		case UART_STATE_IDLE:
			break;
		
		case UART_STATE_TX_START:
			PA1 = 0 ;	// generate start condition
			uart_bit_count = 0;
			uart_state = UART_STATE_TX_TRANSMIT_DATA;
			break;
		case UART_STATE_TX_TRANSMIT_DATA:
			if(uart_bit_count <= 8)
			{
				if(0x01 == (0x01 & (uart_tx_byte >> uart_bit_count)))
				{
					IOA |= 0x02;
				}
				else
				{
					IOA &= 0xFD;
				}

				uart_bit_count++;
			}
			if(uart_bit_count == 9) // STOP
			{
				IOA |= 0x02;
				uart_state = UART_STATE_IDLE;
				TCON = TCON & 0xEF; // Timer run disable
			}
			break;
	}
}						  
