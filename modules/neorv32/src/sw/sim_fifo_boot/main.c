#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

#include <neorv32.h>
#include <neorv32_uart.h>
//#include <sdb_add_neorv32.h>
//#include <mini_sdb.h>

#define BAUD_RATE 19200
#define CLOCK_HZ 62500000

#define WR_BASE_ADDRESS ((volatile int *)0x04000800)
#define WR_REG_STATUS ((volatile int *)0x04000804)
#define WR_BOOT_STATUS ((volatile int *)0x04000808)
#define RD_BASE_ADDRESS ((volatile int *)0x00000000)
#define RD_REG_STATUS ((volatile int *)0x00000004)
#define RD_BOOT_STATUS ((volatile int *)0x00000008)
//#define DPRAM_ADDRESS ((volatile int *)0x71000000)
//#define IMEM_ADDRESS ((volatile int *)0x90000000)
//#define IDLE_MS 1

__attribute__ ((__noinline__))
void * get_pc () { return __builtin_return_address(0); }

int read_reg, read_data;

int main(void)
{ 
  neorv32_uart0_setup(BAUD_RATE, 0);
  neorv32_uart0_puts("PC at:");
  neorv32_uart0_printf("%p\n", get_pc());
  neorv32_uart0_puts("\n Here is main.c, success!");
  neorv32_uart0_puts("PC at:");
  neorv32_uart0_printf("%p\n", get_pc());

  /*
  read_reg = *RD_REG_STATUS;
  neorv32_uart0_printf("%p\n", read_reg);
  *WR_BASE_ADDRESS = read_reg;
  read_reg = *RD_REG_STATUS;
  neorv32_uart0_printf("%p\n", read_reg);
  *WR_BASE_ADDRESS = 0x11556611;
  read_reg = *RD_REG_STATUS;
  neorv32_uart0_printf("%p\n", read_reg);
  read_data = *RD_BASE_ADDRESS;
  //neorv32_uart0_printf("%p\n", read_data);
  read_data = *RD_BASE_ADDRESS;
  //neorv32_uart0_printf("%p\n", read_data);
*/

  //neorv32_uart0_printf("%p\n\n", get_pc());

  /*
  neorv32_uart0_setup(BAUD_RATE, 0);

  int test = 5;
  int mem_readout, fifo_readout = 0;   //wishbone data width is 4 byte
  //neorv32_rte_setup();
  //int writein[5] = {255, 248511265, 123, 55667788, 3333};   //ff,ECFFB21,12,56,ec
  neorv32_uart0_printf("main");
  //neorv32_aux_delay_ms(CLOCK_HZ, IDLE_MS);
  
  for(int i=0 ; i<test ; i++){      //int: 4 byte, wishbone data width is 4 byte(32 bits)
        mem_readout = *(DPRAM_ADDRESS+i); 
        *(FIFO_WR_BASE_ADDRESS) = mem_readout;
        fifo_readout = *(FIFO_RD_BASE_ADDRESS);
        *(IMEM_ADDRESS+i) = fifo_readout;
    }
  neorv32_uart0_printf("end");
  */
  
  /* Test return to start.s */
  return 0;
  
}