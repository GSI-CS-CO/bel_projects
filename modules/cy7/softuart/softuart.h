#include "fx2.h"

// uart states
#define UART_STATE_IDLE (0x00)
#define UART_STATE_RX_START (0x01)
#define UART_STATE_RX_GET_DATA (0x02)
#define UART_STATE_RX_STOP (0x03)
#define UART_STATE_TX_START (0x04)
#define UART_STATE_TX_TRANSMIT_DATA (0x05)
#define UART_STATE_DELAY (0x06)


// CLKOUT / 4 = 12 MHz
#define tmr0_preload_value 0x2F	// 57600 baud
#define tmr0_preload_start_bit 0x98	// 57600 baud


// function prototypes
void tmr0_init(void);
void int0_init(void);
void uart_transmit_byte(BYTE tx_byte_from_usb);
void software_uart_init(void);
