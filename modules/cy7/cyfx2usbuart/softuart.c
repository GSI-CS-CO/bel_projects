#include "softuart.h"
#include "fx2.h"
#include "fx2regs.h"

volatile unsigned char uart_rx_byte = 0;
volatile unsigned char uart_byte_received = 0;
volatile unsigned char uart_state = 0;
volatile unsigned char uart_next_state = 0;
volatile unsigned char uart_tx_byte = 0;
volatile unsigned char uart_bit_count = 0;
volatile unsigned char uart_rx_bit_count = 0;
volatile unsigned char uart_transmit_buffer[UART_BUFFER_LENGTH];
volatile unsigned char uart_buffer_filled_to = 0;
volatile unsigned char uart_buffer_transmitted_to = 0;
volatile unsigned char uart_tx_busy = 0;


static void software_uart_init(void)
{
	OEA = 0xFE;	// 0 input, 1 output -> PA1 = TX, PA0 = RX
	IOA |= 0x02;	// tx pin (PA1) high

	OED = 0xFF;	// for debugging on cypress development board
	IOD = 0xFF;	// pins are not connected on exploder

	//tmr2_init();	// init tmr2 for uart time base (rx), tmr2 doesn't work by now
	tmr1_init();	// init tmr1 for uart  time base (tx)
	int0_init();	// init falling edge interrupt on PA0
}


static void int0_init(void)
{
	PORTACFG = bmBIT0;	// enable int0 on pa0
	TCON |= bmBIT0;		// IT0 = 1; falling edge
	IE |= bmBIT0;		// EX0 = 1; enable int0 interrupt
}

// tmr1 for uart
static void tmr1_init(void)
{
	TCON &= ~bmBIT6;		// Stop tmr1
	TMOD |= bmBIT5;			// Timer 1 mode 2 -> 8 bit counter with autoreload
	TH1 = tmr_preload_value;	// see softuart.h for definition
	CKCON &= ~bmBIT4;		// tmr1 runs with clkout / 12
	IE |= bmBIT3;			// ET1 = 1; // Enable timer 1 interrupts

// for determining baud rate on development board
//	TCON |= bmBIT6; // Timer 1 run enable
}

static void uart_transmit_byte(BYTE tx_byte_from_usb)
{
	if(uart_buffer_filled_to < UART_BUFFER_LENGTH)
	{
		TCON |= bmBIT6; // tmr1 run
		uart_transmit_buffer[uart_buffer_filled_to] = tx_byte_from_usb;
		uart_buffer_filled_to++;
	}
}

static void int0_isr(void) __interrupt INT0_VECT
{
	if(uart_state == UART_STATE_IDLE) {
		TL1 = tmr_preload_start_bit;
		TCON |= bmBIT6;		// Timer run enable
		uart_state = UART_STATE_RX_START;
		PORTACFG = 0x00;	// normal pin operation of pa0
	}
}

// uart_tx
static void tmr1_isr(void) __interrupt TMR1_VECT
{
	if((uart_tx_busy) == 0 && (uart_buffer_filled_to > uart_buffer_transmitted_to))
	{
		uart_state = UART_STATE_TX_START;
		uart_tx_busy = 1;
	}

	switch(uart_state) {
		case UART_STATE_RX_START:
			if((IOA & 0x01) == 0x00)
			{
				PORTACFG = 0x00;	// normal pin operation of pa0
				uart_next_state = UART_STATE_RX_GET_DATA;
				uart_bit_count = 0;	// reset bit count
				uart_rx_byte = 0x00;	// reset rx byte
			}
			else
			{
				uart_next_state = UART_STATE_IDLE;
			}
			break;

		case UART_STATE_RX_GET_DATA:
			if(uart_bit_count < 8)
			{
				uart_rx_byte |= ((IOA & 0x01) << uart_bit_count);
				uart_bit_count++;
			}
			if(uart_bit_count == 8)		// assume STOP bit is coming now.... 
			{
				PORTACFG = bmBIT0;
				uart_next_state = UART_STATE_IDLE;
				uart_byte_received = 1;
				TCON &= ~bmBIT6;	// stop tmr 1
			}
			break;

		case UART_STATE_IDLE:
			if(uart_tx_busy == 1)
			{
				uart_next_state = UART_STATE_TX_START;
			}
			break;

		case UART_STATE_TX_START:
			IOA &= ~0x02;	// PA1 = 0, generate start condition
			uart_bit_count = 0;
			uart_next_state = UART_STATE_TX_TRANSMIT_DATA;
			break;

		case UART_STATE_TX_TRANSMIT_DATA:
			if( uart_bit_count < 8 )
			{
				if(0x01 == (0x01 & (uart_transmit_buffer[uart_buffer_transmitted_to] >> uart_bit_count)))
				{
					IOA |= 0x02;
				}
				else
				{
					IOA &= ~0x02;
				}
			}
			if( uart_bit_count == 8 ) // STOP
			{
				IOA |= 0x02;

				if(uart_buffer_transmitted_to < UART_BUFFER_LENGTH)
				{
					uart_buffer_transmitted_to++;
				}

				if(uart_buffer_filled_to > uart_buffer_transmitted_to)
				{
					uart_next_state = UART_STATE_IDLE;
				}
				else
				{
					uart_next_state = UART_STATE_IDLE;
					uart_buffer_transmitted_to = 0;
					uart_buffer_filled_to = 0;
					uart_tx_busy = 0;
					TCON &= ~bmBIT6;
				}

			}
			uart_bit_count++;
			break;
		default:
			break;
	}

	uart_state = uart_next_state;
}

// tmr2 for uart_rx, doesn't work by now
static void tmr2_init(void)
{
        CKCON |= bmBIT5;	// clkout/12 -> tmr2
        RCAP2L = 0x00;		// tmr1_preload_value;
        RCAP2H = 0xFF;		// by now overflow frequency of tmr2 does not change for different values loaded in RCAP... but it should Oo
        IE |= bmBIT5;		// enable tmr2 interrupt
        T2CON |= bmBIT2;	// tmr2 run
}

// uart_rx
static void tmr2_isr(void) __interrupt TMR2_VECT
{
        //PORTACFG = 0x00;	// normal pin operation of pa0
        //uart_rx_bit_count = 0;
        //uart_rx_byte = 0;
        IOD ^= 0x01;
}
