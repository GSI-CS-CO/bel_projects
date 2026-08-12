#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

#include <neorv32.h>
#include <neorv32_uart.h>
//#include <sdb_add_neorv32.h>
//#include <mini_sdb.h>

#define BAUD_RATE 19200
#define CLOCK_HZ 62500000
#define DPRAM_BASE_ADDRESS ((volatile int *)0x71000000)
#define DPRAM_SIZE 32000

#define IMEM_BASE_ADDRESS ((volatile int *)0x90000000)

__attribute__ ((__noinline__))
void * get_pc () { return __builtin_return_address(0); }


int main(void)
{ 
  neorv32_uart0_puts("Program Counter at:");
  neorv32_uart0_printf("%p\n\n", get_pc());

  int test = 30;
  int readout = 0;   //wishbone data width is 4 byte
  /* Set up UART */
  //neorv32_rte_setup();
  neorv32_uart0_setup(BAUD_RATE, 0);

  
  /* Start endless loop */
    //neorv32_aux_print_logo();
    //neorv32_aux_print_hw_config();
    neorv32_uart0_puts("NEORV32: Hello world from Yulien!\n");
    
    neorv32_uart0_puts("Program Counter at:");
    neorv32_uart0_printf("%p\n\n", get_pc());

    for(int i=0 ; i<test ; i++){      //int: 4 byte, wishbone data width is 4 byte(32 bits)
        readout = *(DPRAM_BASE_ADDRESS + i);
        neorv32_uart0_printf("DPRAM Addr 0x%x: 0x%x\n", (DPRAM_BASE_ADDRESS+i), readout);
    }

    neorv32_uart0_puts("Program Counter at:");
    neorv32_uart0_printf("%p\n\n", get_pc());

    for(int j=0 ; j<test ; j++){
        readout = *(IMEM_BASE_ADDRESS + j);
        neorv32_uart0_printf("IMEM Addr 0x%x: 0x%x\n", (IMEM_BASE_ADDRESS+j), readout); 
    }

    neorv32_uart0_puts("Program Counter at:");
    neorv32_uart0_printf("%p\n\n", get_pc());
  /* Test return to start.s */
  return 0;
}
