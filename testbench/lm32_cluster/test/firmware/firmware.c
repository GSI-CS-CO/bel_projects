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

void irq_handler(int id) { 
  msi m;
  mprintf("irq_handler %d\n",id);
  // send msi threadsafe to main loop
  m.msg = global_msi.msg;
  m.adr = global_msi.adr;
  // mprintf("irq: host_slot is %x\n",m.msg);
  pCpuMsiBox[(m.msg>>16)*2] = m.msg&0x0000ffff; // send a msg
}

void init_irq_table() {
  isr_table_clr();
  isr_ptr_table[0] = &irq_handler;
  irq_set_mask(0x01);
  //msg_buf[IRQ].ring_head = msg_buf[IRQ].ring_tail; // clear msg buffer
  irq_enable();
  // mprintf("IRQ table configured.\n");
}

void discover() {
  sdb_location found_sdb[20];
  uint32_t idx = 0;
  pCpuMsiBox   = 0;
  pMyMsi       = 0; 

  uart_out = (volatile char*) find_device_adr(GSI, SDB_UART_SIM);
  find_device_multi(&found_sdb[0], &idx, 1, GSI, MSI_MSG_BOX);   
  if(idx) {
    pCpuMsiBox    = (uint32_t*)getSdbAdr(&found_sdb[0]); 
    pMyMsi        = (uint32_t*)getMsiAdr(&found_sdb[0]); 
  } 

  pCpuIrqSlave    = find_device_adr(GSI, CPU_MSI_CTRL_IF);
}

int main() {
	discover();
	init_irq_table();
	puts("start loop\n");
	mprintf("pCpuMsiBox = 0x%x\n", pCpuMsiBox);
	mprintf("pMyMsi     = 0x%x\n", pMyMsi);
	for(int i = 0;;++i) {
		irq_disable();
		irq_enable();
		mprintf("hello world %d\n", i);
	}
	irq_disable();
	return 0;
}