#include "fx2.h"

// uart states
#define UART_STATE_IDLE 0x00
#define UART_STATE_RX_START 0x01
#define UART_STATE_RX_GET_DATA 0x02
#define UART_STATE_RX_STOP 0x03
#define UART_STATE_TX_START 0x04
#define UART_STATE_TX_TRANSMIT_DATA 0x05
#define UART_STATE_DELAY 0x06

// define uart buffer length
#define UART_BUFFER_LENGTH 32

// CLKOUT / 12 = 4 MHz tmr increment frequency
// by now maximum baudrate is 38400

// 19200 baud 52,1 us bit duration
//#define tmr_preload_value 0x30
//#define tmr_preload_start_bit 0x98	// half bit duration

// 38400 baud 26 us bit duration
#define tmr_preload_value 0x98
#define tmr_preload_start_bit 0xCC

// 57600 baud 17,4 us bit duration
//#define tmr_preload_value 0xBA
//#define tmr_preload_start_bit 0xDD

// declare variables
extern volatile unsigned char uart_rx_bitcount;
extern volatile unsigned char uart_rx_byte;
extern volatile unsigned char uart_byte_received;

extern volatile unsigned char uart_state;
extern volatile unsigned char uart_next_state;
extern volatile unsigned char uart_tx_byte;
extern volatile unsigned char uart_bit_count;
extern volatile unsigned char uart_transmit_buffer[UART_BUFFER_LENGTH];
extern volatile unsigned char uart_buffer_filled_to;
extern volatile unsigned char uart_buffer_transmitted_to;
extern volatile unsigned char uart_tx_busy;

// function prototypes
void tmr1_init(void);
void tmr2_init(void);
void int0_init(void);
void uart_transmit_byte(BYTE tx_byte_from_usb);
void software_uart_init(void);
void int0_isr(void) __interrupt INT0_VECT;
void tmr1_isr(void) __interrupt TMR1_VECT;
void tmr2_isr(void) __interrupt TMR2_VECT;
