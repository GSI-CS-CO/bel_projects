#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

#include <neorv32.h>
#include <neorv32_uart.h>
//#include <sdb_add_neorv32.h>
//#include <mini_sdb.h>

#define BAUD_RATE 921600
#define CLOCK_HZ 62500000
#define FIFO_WR_BASE_ADDRESS ((volatile int *)0x04060000)
#define FIFO_RD_BASE_ADDRESS ((volatile int *)0x05060000)
#define DPRAM_ADDRESS ((volatile int *)0x71000000)
#define IMEM_ADDRESS ((volatile int *)0x90000000)
//#define IDLE_MS 1


int main(void)
{ 
  neorv32_uart0_setup(BAUD_RATE, 0);

  int test = 5;
  int mem_readout, fifo_readout = 0;   //wishbone data width is 4 byte
  /* Set up UART */
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
  /* Test return to start.s */
  return 0;
}