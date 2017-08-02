#include <stdint.h>

#include <inttypes.h>
#include <stdarg.h>

#include "syscon.h"
#include "hw/memlayout.h"
#include "mprintf.h"
#include "board.h" //from WR
#include "hk_board.h"
#include "uart.h"
#include "w1.h"
#include "mini_sdb.h"
#include <hw/sockit_owm_regs.h>
#include <hw/wb_uart.h>


extern struct w1_bus wrpc_w1_bus;
volatile struct UART_WB *uart;
volatile unsigned char* scu_reg;
volatile unsigned int* aru_base;
volatile unsigned char* asmi_base;

#define CPU_CLOCK 125000000ULL
#define FW_VER 0x1

#define CALC_BAUD(baudrate) \
    ( ((( (unsigned long long)baudrate * 8ULL) << (16 - 7)) + \
      (CPU_CLOCK >> 8)) / (CPU_CLOCK >> 7) )

void uart_hw_init()
{
  uart = (volatile struct UART_WB *)BASE_UART;
  uart->BCR = CALC_BAUD(UART_BAUDRATE);
}

/* Init from sockitowm code */
#define CLK_DIV_NOR (CPU_CLOCK / 200000 - 1)  /* normal mode */
#define CLK_DIV_OVD (CPU_CLOCK / 1000000 - 1) /* overdrive mode (not used) */
void w1_init(void)
{
  IOWR_SOCKIT_OWM_CDR(BASE_ONEWIRE,
          ((CLK_DIV_NOR & SOCKIT_OWM_CDR_N_MSK) |
           ((CLK_DIV_OVD << SOCKIT_OWM_CDR_O_OFST) &
            SOCKIT_OWM_CDR_O_MSK)));
}



void msDelay(uint32_t msecs) {
  usleep(1000 * msecs);
}

void ReadOwDevices(int bus) {
  struct w1_dev *d;
  int i;
  int tvalue;
  wrpc_w1_bus.detail = bus; // set the portnumber of the onewire controller
  if (w1_scan_bus(&wrpc_w1_bus) > 0) {
    for (i = 0; i < W1_MAX_DEVICES; i++) {
      d = wrpc_w1_bus.devs + i;
        if (d->rom) {
          mprintf("bus,device (%d,%d): 0x%08x%08x ", wrpc_w1_bus.detail, i, (int)(d->rom >> 32), (int)d->rom);
          *(int*)(scu_reg + 0) = (d->rom >> 32);
          *(int*)(scu_reg + 4) = d->rom;
          if ((char)d->rom == 0x42) {
            tvalue = w1_read_temp(d, 0);
            *(int*)(scu_reg + 8) = (tvalue >> 12);
            mprintf("temp: %dC", tvalue >> 16); //show only integer part for debug
          }
          mprintf("\n");
        }
    }
  } else {
    mprintf("no devices found on bus %d\n", wrpc_w1_bus.detail);
  }
}

int main(void)
{
  int i, j, addr, magicword;
  unsigned char flash_word;
  discoverPeriphery();
  aru_base      = (unsigned int *)find_device_adr(GSI, WB_REMOTE_UPDATE);
  scu_reg       = (unsigned char *)find_device_adr(GSI, WB_SCU_REG);
  asmi_base     = (unsigned char *)find_device_adr(GSI, WB_ASMI);
  BASE_ONEWIRE  = (unsigned char *)find_device_adr(CERN, WR_1Wire);

  if (!BASE_UART) {
    while (1) {};
  }
  uart_hw_init();
  uart_write_string("\nDebug Port\n");

  mprintf("aru_base: 0x%x\n", aru_base);
  mprintf("scu_reg: 0x%x\n", scu_reg);
  mprintf("asmi_base: 0x%x\n", asmi_base);
  mprintf("BASE_UART: 0x%x\n", BASE_UART);
  mprintf("BASE_ONEWIRE: 0x%x\n", BASE_ONEWIRE);

  if ((int)aru_base == ERROR_NOT_FOUND) {
    mprintf("no remote update controller found!\n");
    while (1) {};
  }
  if ((int)scu_reg == ERROR_NOT_FOUND) {
    mprintf("no WB to SCU memory found!\n");
    while (1) {};
  }
  if ((int)asmi_base == ERROR_NOT_FOUND) {
    mprintf("no ASMI controller found!\n");
    while (1) {};
  }
  if ((int)BASE_ONEWIRE == ERROR_NOT_FOUND) {
    mprintf("no 1Wire controller found!\n");
    while (1) {};
  }
  
  *(volatile int *)(scu_reg + FW_VERSION) = FW_VER;

  w1_init();
  //read id and temp at least once
  ReadOwDevices(0);
 
  wrpc_w1_init();
  while(1) {
    switch(*(volatile int *)(scu_reg + ASMI_CMD)) {
      case READ_ASMI_STATUS:
        *(volatile int *)(scu_reg + ASMI_STATUS) = *(asmi_base + 0x4);
        *(volatile int *)(scu_reg + ASMI_CMD) = DONE;
        mprintf("status read!\n"); 
      break;

      case READ_ASMI_ID:
        *(volatile int*)(scu_reg + ASMI_RDID) = *(asmi_base + 0x8);
        *(volatile int*)(scu_reg + ASMI_CMD) = DONE;
      break;

      case SECTOR_ERASE:
        // param has to be in 0 to 63
        *(volatile int*)(asmi_base + 0xc) = *(volatile int*)(scu_reg + ASMI_PARAM);
        *(volatile int*)(scu_reg + ASMI_CMD) = DONE;
      break;

      case PAGE_WRITE:
        j = 0;
        // fill page buffer
        for(i = 0; i < 256; i++)
          *(char *)(asmi_base + (i << 4)) = *(char*)(scu_reg + ASMI_BUFFER + i);
        // start address of page
        addr = *(volatile int*)(scu_reg + ASMI_PARAM);
        //write to flash
        *(unsigned int*)(asmi_base + 0xf) = addr;
        // signal end of operation 
        *(volatile int*)(scu_reg + ASMI_CMD) = DONE;
      break;

      case PAGE_READ:
        // start address of page
        addr = *(volatile int*)(scu_reg + ASMI_PARAM);
        // read 256 bytes from flash
        for(i = 0; i < 256; i++) {
          flash_word = *(char*)(asmi_base + ((i + addr) << 4));
          *(char*)(scu_reg + ASMI_BUFFER + i) = flash_word;
        }
        // signal end of operation 
        *(volatile int*)(scu_reg + ASMI_CMD) = DONE;
      break;

      case RECONFIG:
        magicword = *(volatile int*)(scu_reg + ASMI_PARAM);
        
        if (magicword == 0xdeadbeef)
          aru_base[CONFIG] = 0x1;

        // signal end of operation 
        *(volatile int*)(scu_reg + ASMI_CMD) = DONE;
      break;  
      
      case READ_TEMP:
        ReadOwDevices(0);
        // signal end of operation 
        *(volatile int*)(scu_reg + ASMI_CMD) = DONE;
      break;  
      default: break;
    }
    
  }
  //if (aru_base[CONFIG_SRC] == 0) {  //PowerUp
  //  aru_base[PAGE_SEL] = 0x90000; // start address for Application image
  //  aru_base[CONFIG_MODE] = 0x1;  // set to Application mode
    //aru_base[CONFIG] = 0x1; // trigger reconfiguration
  //}
}
