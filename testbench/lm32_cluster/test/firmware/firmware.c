#include "mini_sdb.h"
#include "pp-printf.h"
#include "irq.h"
#include "mprintf.h"

void* memset(void* s, int c, int n) {
	void *result = s;
	for (int i = 0; i < n/4; ++i) {
		((int*)s)[i] = c;
	}
	return result;
}
void *memcpy(void *dest, const void *src, int n)
{
	void *result = dest;
	for (int i = 0; i < n/4; ++i) {
		((int*)dest)[i] = ((int*)src)[i];
	}
	return result;
}


volatile char *uart_out;
int puts(const char *str) {
	while(*str) {
		*uart_out = *str++;
	}
	return 1;
}
void uart_write_byte(const char c) {
	*uart_out = c;
}

void my_isr(void) { 
	//<evaluate global_msi and do something useful> 
	puts("my_isr\n");
}

int main() {
	uart_out = (volatile char*) find_device_adr(GSI, SDB_UART_SIM);
	isr_table_clr();
	isr_ptr_table[0] = my_isr;
	irq_set_mask(0x01); //Enable used IRQs ...
	irq_enable(); 
	for(;;) {
		puts("hello, world!\n");
		mprintf("0x%x\n",uart_out);
		// pp_printf("at_on!\n");
		// atomic_on();
		// pp_printf("at_off!\n");
		// atomic_off();
	}
	irq_disable();
	return 0;
}