#include "irq.h"
#include "pp-printf.h"
#include <stdint.h>
#include <stdlib.h>


void _irq_entry(void)
{
//  uint8_t cnt;
//  uint8_t *BASE_FRAME_RAM = (uint8_t *) 0x100000;
//  pp_printf("Frame!!! \n");

  //pp_printf("IRQ bit %d \n", irq_get_enable());
  //pp_printf("IRQ mask %d \n", irq_get_mask());
  //pp_printf("IRQ bit %d \n", irq_get_enable());

  frame = 1;
//  cnt = 0;
//  if (frame_cnt == 3)
//    frame_cnt = 0;
//  else
//    frame_cnt++;
//
//      while(cnt < 40)
//      {
//        //pp_printf("%x",*(volatile int*)(BASE_FRAME_RAM + (frame_cnt*381) +base));
//        pp_printf("%x ",*(uint32_t *)(BASE_FRAME_RAM + (frame_cnt*381) + cnt));
//        //pp_printf("%p ",(BASE_FRAME_RAM + cnt));
//        //pp_printf("%x ",*(uint32_t*)(BASE_FRAME_RAM + cnt));
//        //pp_printf("%x ",*(volatile uint32_t*)(BASE_FRAME_RAM + cnt));
//        cnt++;
//      }
//      pp_printf("\n");

  clear_irq();
}
