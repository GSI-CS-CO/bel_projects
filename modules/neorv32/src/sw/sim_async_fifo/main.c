#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

#include <neorv32.h>
#include <neorv32_uart.h>
//#include <sdb_add_neorv32.h>
//#include <mini_sdb.h>

#define BAUD_RATE 921600
#define CLOCK_HZ 62500000
#define FIFO_WR_BASE_ADDRESS ((volatile int *)0x60000000)
#define FIFO_RD_BASE_ADDRESS ((volatile int *)0x65000000)
//#define IDLE_MS 1


int main(void)
{ 
  neorv32_uart0_setup(BAUD_RATE, 0);

  int test = 5;
  int readout, status = 0;   //wishbone data width is 4 byte
  /* Set up UART */
  //neorv32_rte_setup();
  int writein[5] = {255, 248511265, 123, 55667788, 3333};   //ff,ECFFB21,12,56,ec
  neorv32_uart0_printf("asy");
  //neorv32_aux_delay_ms(CLOCK_HZ, IDLE_MS);
    for(int i=0 ; i<test ; i++){      //int: 4 byte, wishbone data width is 4 byte(32 bits)
        *(FIFO_WR_BASE_ADDRESS) = writein[i];
        status = *(FIFO_RD_BASE_ADDRESS+1);
    }

    for(int j=0 ; j<test+1; j++){
        readout = *(FIFO_RD_BASE_ADDRESS);
        status = *(FIFO_RD_BASE_ADDRESS+1);
        //neorv32_uart0_printf("%d,%x;\n", j, readout); 
    }

    for(int k=0 ; k<test ; k++){
        status = *(FIFO_RD_BASE_ADDRESS+1);
        //neorv32_uart0_printf("%d %x\n", k, status); 
    }
  /* Test return to start.s */
  return 0;
}
