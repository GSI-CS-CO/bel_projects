#include "mini_sdb.h"

void* memset(void* s, int c, int n) {
	for (int i = 0; i < n; ++i) {
		*(int*)s = c;
	}
}


volatile char *uart_out;

void print_str(char *str) {
	while(*str) {
		*uart_out = *str++;
	}
}

int main() {
	uart_out = (volatile char*) find_device_adr(GSI, SDB_UART_SIM);
	for(;;) {
		print_str("hello, world!\n");
	}
	return 0;
}