/* ================================================================================ */
/* NEORV32 Bootloader                                                               */
/* -------------------------------------------------------------------------------- */
/* The NEORV32 RISC-V Processor - https://github.com/stnolting/neorv32              */
/* Copyright (c) NEORV32 contributors.                                              */
/* Copyright (c) 2020 - 2026 Stephan Nolting. All rights reserved.                  */
/* Licensed under the BSD-3-Clause license, see LICENSE for details.                */
/* SPDX-License-Identifier: BSD-3-Clause                                            */
/* ================================================================================ */

// libraries
#include <stdint.h>
#include <neorv32.h>
#include <config.h>
#include <system.h>
#include <uart.h>
//#include <user_mem.h>

#define BIT_1   (1U << 1)  //integel

#define FIFO_RD_BASE_ADDRESS ((volatile int *)0x04000800)   
#define FIFO_RD_STATUS_ADDR ((volatile int *)0x04000804)
#define RD_BOOT_STATUS_ADDR ((volatile int *)0x04000808)
#define REG_RD_EMPTY   (*(volatile uint32_t *)FIFO_RD_STATUS_ADDR)
#define BOOT_STATUS   (*(volatile uint32_t *)RD_BOOT_STATUS_ADDR)
#define DPRAM_ADDRESS 0x71000000
#define IMEM_ADDRESS  0x90000000           //((volatile int *)0x90000000)
#define GPIO0 0
#define GPIO1 1

/**********************************************************************//**
 * Bootloader main. Should never return.
 **************************************************************************/
int main(void) {

  // ------------------------------------------------
  // System setup
  // ------------------------------------------------
  volatile uint32_t *dpram_ptr = (volatile uint32_t *)DPRAM_ADDRESS;   //declare a pointer

  uint32_t g_exe_size =0;
 
  uint32_t rd_cnt=0;
  uint32_t readout=0;
  uart_puts("2\n");
  // hardware setup
  system_setup();
  uart_puts("1\n");

  neorv32_gpio_pin_set(GPIO0,0);
  uart_puts("Hi here is bootloader\n");

  neorv32_gpio_pin_set(GPIO1,1);    // get status and file size from fifo
  while(REG_RD_EMPTY & BIT_1){
    uart_puts("wait for first input\n");
  }
  g_exe_size = *(FIFO_RD_BASE_ADDRESS);
  uart_puts("get file_size");
  uart_puth(g_exe_size);

  if((BOOT_STATUS & 1) == 0){  //DPRAM
    neorv32_gpio_pin_set(GPIO1,0); //DPRAM
    uart_puts("switch to dpram\n");
    uart_puts("start load data in dpram to IMEM\n");
    while (rd_cnt < g_exe_size){
      readout = *(dpram_ptr);
      neorv32_cpu_store_unsigned_word(IMEM_ADDRESS+rd_cnt*4, readout);
      dpram_ptr++;
      rd_cnt++;
    }
    uart_puts("end");
  }
  else if((BOOT_STATUS & 1) == 1){   //FIFO, wait for terminal input
    uart_puts("fifo");
    while (rd_cnt < g_exe_size/4){
      uart_puth(REG_RD_EMPTY);
      uart_puts("\n");
      if(!(REG_RD_EMPTY & BIT_1)){
        readout = *(FIFO_RD_BASE_ADDRESS);
        neorv32_cpu_store_unsigned_word(IMEM_ADDRESS+rd_cnt*4, readout);
        uart_puth(rd_cnt);
        uart_puth(readout);
        uart_puts("read\n");
        rd_cnt++;
      }
    }
  }

  uart_puts("OK\n");
  // sync data cache
  asm volatile ("fence");

  neorv32_cpu_csr_write(CSR_MSTATUS, (1 << CSR_MSTATUS_MPP_H) + (1 << CSR_MSTATUS_MPP_L));
  uart_puts("Booting ...\n\n");
  while (neorv32_uart0_tx_busy());

  asm volatile (
    "fence.i            \n"
    "csrw mepc, %[addr] \n"
    "mret               \n"
    : : [addr] "r" (IMEM_ADDRESS)
  );

  __builtin_unreachable();


/*
  //Test start
  neorv32_gpio_pin_set(1,0); //dpram, get test data
  for (int i=0; i<4 ; i++){
    *(FIFO_WR_BASE_ADDRESS) = *(dpram_ptr);
    dpram_ptr++;
  }

  neorv32_gpio_pin_set(1,1);  // switch to internal rd port
  for (int j=0; j<4 ; j++){
    header[j] = *(FIFO_RD_BASE_ADDRESS);
  }


  if (signature != (uint32_t)BIN_SIGNATURE) {
    uart_puts("\aERROR_SIGNATURE\n");
    return 1;
  }

  neorv32_gpio_pin_set(1,0); //dpram, get test data

  while (rd_cnt < g_exe_size ){
    while( !(REG_WR_FULL & BIT_1) && wr_cnt < g_exe_size){
      *(FIFO_WR_BASE_ADDRESS) = *(dpram_ptr);
      dpram_ptr++;
      wr_cnt++;     
    }
    if ((REG_WR_FULL & BIT_1) && rd_cnt < g_exe_size) {
      neorv32_gpio_pin_set(1,1);
    }   
    while( !(REG_RD_EMPTY & BIT_1) && rd_cnt < g_exe_size){
      fifo_readout = *(FIFO_RD_BASE_ADDRESS);
      checksum += fifo_readout;
      neorv32_cpu_store_unsigned_word(IMEM_ADDRESS+rd_cnt*4, fifo_readout);
      rd_cnt++;
    }
    if ((REG_RD_EMPTY & BIT_1) && wr_cnt < g_exe_size) {
      neorv32_gpio_pin_set(1,0);
    }
  }
  uart_puts("y");
  if ((checksum + 1) != 0) {
    uart_puts("\aERROR_CHECKSUM\n");
    uart_puth(checksum);
    return 1;
  }

  uart_puts("OK\n");
  // sync data cache
  asm volatile ("fence");
  uart_puth(g_exe_size);

  neorv32_cpu_csr_write(CSR_MSTATUS, (1 << CSR_MSTATUS_MPP_H) + (1 << CSR_MSTATUS_MPP_L));
  uart_puts("Booting ...\n\n");
  while (neorv32_uart0_tx_busy());

  asm volatile (
    "fence.i            \n"
    "csrw mepc, %[addr] \n"
    "mret               \n"
    : : [addr] "r" (IMEM_ADDRESS)
  );

  __builtin_unreachable();
*/



}
