#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "pp-printf.h"
#include "mini_sdb.h"
#include "sdb_arg.h"
#include "uart.h"
#include "irq.h"
#include "ethernet.h"
#include <unistd.h>

#define WORDS_MEMORY    0
#define FRAMES_MEMORY   4
#define WORDS_FRAME     8
#define OVERFLOW_RAM    12
#define WORDS_FRAME_WB  16
#define OTHERS_WB       20

#define WR_PPS_GEN_CNTR_UTCLO   0x8

volatile uint8_t *BASE_FRAME_RAM = (uint8_t *) 0x100000;
volatile uint8_t *BASE_FR2WB     = (uint8_t *) 0x800000;

int frame = 0;

void init()
{
   enable_irq();
   discoverPeriphery();
   uart_init_hw();
   uart_write_string("\nDebug Port\n");
}

int main(void) {
  uint32_t frames_read = 0;
  uint32_t overflow = 0;
  uint32_t word = 0;
  uint32_t frame_b;
  uint32_t header[ETH_HEADER];
  uint32_t fec_header[ETH_HEADER];
  uint32_t fec_payload[ETH_HEADER];
  uint32_t ram_s = 0;

  uint32_t frames_ram = 0;
  uint32_t words_in_frame = 32;
  uint32_t cnt;
  uint16_t protocol;
  uint32_t frame_chunk;
  uint32_t counter_h;
  uint32_t counter_l;
  uint16_t prot;
  uint32_t words_frame;
  uint32_t length_frame;

  uint32_t delta_seconds[4];
  uint32_t delta_nano[4];
  uint32_t tx_ts_second[4];
  uint32_t tx_ts_nano[4];
  uint32_t rx_ts_second[4];
  uint32_t rx_ts_nano[4];
  uint32_t rx_ts_test;
  uint32_t block_cnt = 0;
  uint32_t frame_cnt = 0;
  uint32_t ts_offset;

  init();

  pp_printf("FEC Unit starting!\n");
  pp_printf("SDB Record %x \n", r_sdb_add());
  pp_printf("FEC Wb %x \n",*(volatile uint32_t *)(BASE_FR2WB+OTHERS_WB));
  words_frame = *(volatile uint32_t *)(BASE_FR2WB + WORDS_FRAME);
  pp_printf("Words per Frame %x \n",words_frame);

  while(1)
  {
    frames_ram = *(volatile uint32_t *)(BASE_FR2WB + FRAMES_MEMORY);

    if(frames_ram > 0)
    {
      frames_read = 0;
      frame_cnt = 0;

      while(frames_read < frames_ram)
      {
        for(cnt = 0; cnt < words_in_frame; cnt++)
        {
          frame_chunk = *(volatile uint32_t *)(BASE_FRAME_RAM + (frame_cnt*words_frame) + cnt);
          //pp_printf("%08x ",frame_chunk);

          if (cnt <= ETH_HEADER) {
            header[cnt] = frame_chunk;
            //pp_printf("%08x ",header[cnt]);
          }
          if (cnt == PROTO_HEADER) {
            length_frame = (((frame_chunk & MASK_SWAP) & MASK_SWAP) + ETH_HEADER_L);
            length_frame >>= 1; // length in 16 bit words
            if (length_frame & 0x1) { // 16 bits odd
              length_frame  = (length_frame >> 1) + 1;
              ts_offset     = 0;
            } else {
              length_frame >>= 1;
              ts_offset     = 1;
            }
            //pp_printf("FRAME LENGTH in WB cycles: %d \n", length_frame);
          } else if (cnt == PROTO_ID) {
            prot = (frame_chunk >> 24) & MASK_PROT;
            if (prot == 0xd) {
              rx_ts_nano[block_cnt]   =  *(volatile uint32_t *)(BASE_FRAME_RAM + (frame_cnt*words_frame) + length_frame + ts_offset);
              rx_ts_nano[block_cnt]   = ((rx_ts_nano[block_cnt] & MASK_SWAP) << 16) | (rx_ts_nano[block_cnt] >> 16);
              rx_ts_second[block_cnt] = *(volatile uint32_t *)(pPps+2);
              pp_printf("PROT: %x FRAME %d: %08u %08u", prot, block_cnt, rx_ts_second[block_cnt], rx_ts_nano[block_cnt]);
              block_cnt++;
           } else
           	pp_printf("PROT: %x", prot);
          } else if (cnt > PROTO_TS_B && PROTO_TS_E < cnt) {
            if (prot == 0xe) {
              if (block_cnt =4 )
                block_cnt = 0;
              //pp_printf("TS %d:", block_cnt);
              //block_cnt++;
            }
          }

//        else if (cnt == PROTO_TS_1) {
//            if (prot == 2) {
//              tx_seconds = ((frame_chunk&MASK_SWAP)<<16) | (frame_chunk>>16);
//            }
//        } else if (cnt ==  PROTO_TS_2) {
//            if (prot == 1) {
//              pp_printf("ID: %04x \n", frame_chunk>>16);
//              pp_printf("TS RX: %08d %08d \n", seconds, time_stamp);
//            } else {
//              tx_nano = ((frame_chunk&MASK_SWAP)<<16) | (frame_chunk>>16);
//              pp_printf("TS TX: %08d %08d \n", tx_seconds, tx_nano);
//            }
//        } else if( cnt ==  PROTO_TS_2+1)  {
//            if (prot == 2) {
//              delta_seconds = seconds - tx_seconds;
//              delta_nano    = (uint32_t)( time_stamp - tx_nano);
//              pp_printf("Delay: %08d %08d \n\n", delta_seconds,delta_nano);
//            }
//          }
        }
        pp_printf("\n");

        if (frame_cnt == 3)
          frame_cnt = 0;
        else
          frame_cnt++;

        frames_read++;
      }
    }
  }

  return 0;

}
