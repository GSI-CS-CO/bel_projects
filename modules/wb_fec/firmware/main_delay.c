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
#define REF_CLOCK_PERIOD_PS 8000
#define REF_CLOCK_FREQ_HZ 125000000

volatile uint8_t *BASE_FRAME_RAM = (uint8_t *) 0x1000000;
volatile uint8_t *BASE_FR2WB     = (uint8_t *) 0x2000000;
volatile uint16_t *BASE_WB2FR    = (uint8_t *) 0x8000000;

int frame = 0;

void init()
{
   enable_irq();
   discoverPeriphery();
   uart_init_hw();
   uart_write_string("\nDebug Port\n");
}

uint64_t get_utc(void)
{

  uint64_t out;
  uint32_t low, high;

  low  = *(volatile uint32_t *)(pPps+2);
  high = *(volatile uint32_t *)(pPps+3);

  high &= 0xFF;           /* CNTR_UTCHI has only 8 bits defined -- rest are HDL don't care */

  out = (uint64_t) low | (uint64_t) high << 32;
  return out;
}


void get_time(uint64_t * seconds, uint32_t * nanoseconds)
{
        uint32_t ns_cnt;
        uint64_t sec1, sec2;

        do {
                sec1 = get_utc();
                ns_cnt = *(volatile uint32_t *)(pPps+1) & 0xFFFFFFFUL;    /* 28-bit wide register */
                sec2 = get_utc();
        } while (sec2 != sec1);

        if (seconds)
                *seconds = sec2;
        if (nanoseconds)
                *nanoseconds = (uint32_t) ((int64_t) ns_cnt *
                                (int64_t) REF_CLOCK_PERIOD_PS / 1000LL);
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
  uint32_t cnt_ts;
  uint32_t cnt_ts_r;
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

  uint32_t counter_ppsg;
  int32_t delta;
  int32_t pg_delta;
  uint64_t sec;

  unsigned int pg_tx_seconds;
  unsigned int pg_tx_nano;

  unsigned int pg_rx_seconds;
  unsigned int pg_rx_nano;

  unsigned int tx_seconds;
  unsigned int tx_nano;
  unsigned int tx_nano_f;
  unsigned int rx_seconds;
  unsigned int rx_nano;
  unsigned int rx_nano_f;
  unsigned int tx_cnt;
  unsigned int tx_cnt_prev;
  unsigned int payload_l;
  unsigned int p_miss=0;

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
        //for(cnt = 0; cnt < words_in_frame; cnt++)
        for(cnt = 0; cnt < 15; cnt++)
        {
          frame_chunk = *(volatile uint32_t *)(BASE_FRAME_RAM + (frame_cnt*words_frame) + cnt);
          //pp_printf("%08x ",frame_chunk);
          //
          *(volatile uint32_t *)(BASE_WB2FR) = 0xffff & frame_chunk;
          *(volatile uint32_t *)(BASE_WB2FR) = 16 >> frame_chunk;

          if (cnt <= ETH_HEADER) {
            header[cnt] = frame_chunk;
            //pp_printf("%08x ",header[cnt]);
          } else if ((cnt > 3) && (cnt < 14)) {
	  	if (cnt == 4)
		{
			if (frame_chunk == 0xbeefbeef)
			{
            			//pp_printf("\n");
            			//pp_printf("DELAY\n");
			}
			else
				break;
		} else if (cnt == 5) {
	        	tx_nano   = (((frame_chunk&MASK_SWAP)<<16) | (frame_chunk>>16));
	        	tx_nano_f = (tx_nano >> 28) & 0xf;
			tx_nano   = tx_nano & 0xfffffff;
			tx_nano   = tx_nano * 8;
		} else if (cnt == 6) {
          		//pp_printf("%08x ",frame_chunk);
			tx_seconds = frame_chunk;
		} else if (cnt == 7) {
          		//pp_printf("%08x ",frame_chunk);
			tx_seconds = tx_seconds | (frame_chunk&0xffff);
		} else if (cnt == 8) {
			pg_tx_nano = (((frame_chunk&MASK_SWAP)<<16) | (frame_chunk>>16));
			pg_tx_nano   = pg_tx_nano & 0xfffffff;
			pg_tx_nano   = pg_tx_nano * 8;
		} else if (cnt == 9) {
			pg_tx_seconds = frame_chunk;
		} else if (cnt == 10) {
          		//pp_printf("%08x ",frame_chunk);
			pg_tx_seconds = pg_tx_seconds | (frame_chunk&0xffff);
		} else if (cnt == 12) {
	        	tx_cnt   = (((frame_chunk&MASK_SWAP)<<16) | (frame_chunk>>16));
                        if( tx_cnt_prev != (tx_cnt-1)) {
                	  	pp_printf("-----Missing Packet: %d  \n", p_miss);
                	  	pp_printf("Counter: %d  \n", tx_cnt);
                                p_miss += 1;
                        }
                        tx_cnt_prev = tx_cnt;
                } else if (cnt == 13) {
	        	payload_l   = (((frame_chunk&MASK_SWAP)<<16) | (frame_chunk>>16));
                }


	  } else {
        	//pp_printf("\n");

		frame_chunk = *(volatile uint32_t *)(BASE_FRAME_RAM + (frame_cnt*words_frame) + 254);
	        rx_nano = (((frame_chunk&MASK_SWAP)<<16) | (frame_chunk>>16));
          	//pp_printf("TS R&F: %08x\n", rx_nano);
	        rx_nano_f = (rx_nano >> 28) & 0xf;
	        rx_nano = rx_nano &  0xfffffff;
          	//pp_printf("TS R: %08x\n", rx_nano);
	        //rx_nano = rx_nano * 8;
          	//pp_printf("%08u\n", rx_nano);

	  	//rx_second = *(volatile uint32_t *)(pPps+2);
		get_time(&sec, &counter_ppsg);

		if (rx_nano > 3 * REF_CLOCK_FREQ_HZ / 4
                            && counter_ppsg < 250000000)
                                sec--;

                rx_seconds = sec & 0x7fffffff;
                rx_nano = rx_nano * (REF_CLOCK_PERIOD_PS / 1000);

//-------------------------------------
		if (rx_nano > tx_nano)
			delta = rx_nano - tx_nano;
		else
			delta = tx_nano - rx_nano;
//-------------------------------------
		if (pg_rx_nano > pg_tx_nano)
			pg_delta = pg_rx_nano - pg_tx_nano;
		else
			pg_delta = pg_tx_nano - pg_rx_nano;
//-------------------------------------
//          	pp_printf("DELAY PHY TO PHY\n");
//          	pp_printf("WRC TS Tx  \t %08u \t %08u \n", tx_seconds, tx_nano);
//	  	pp_printf("WRC TS Rx: \t %08u \t %08u \n", rx_seconds, rx_nano);
//	  	pp_printf("\nDelay From PHY two PHY:\t \t %08d  ns \n\n", delta);
////-------------------------------------
//          	pp_printf("DELAY PACKET GENERATOR TO PACKET GENERATOR\n");
//          	pp_printf("PACKET GEN TS Tx: %08u \t %08u \n", pg_tx_seconds, pg_tx_nano);
//	  	pp_printf("PACKET GEN TS Rx: %08u \t %08u \n", pg_rx_seconds, pg_rx_nano);
//	  	pp_printf("\nDelay From END two END:\t \t %08d  ns \n\n", pg_delta);
////-------------------------------------
//          	pp_printf("DELAY IN GATEWARE\n");
//	  	pp_printf("Delay Gateware Tx: %08u  ns \n\n", tx_nano - pg_tx_nano);
//	  	pp_printf("Delay Gateware Rx: %08u  ns \n\n", rx_nano - pg_rx_nano);
////-------------------------------------
//	  	pp_printf("Counter: %d  \n", tx_cnt);
//	  	pp_printf("Missing Packet: %d  \n", p_miss);
//	  	pp_printf("Payload: %d  \n", payload_l);
//                pp_printf("----------------------- \n");
	   }

        }
        if (frame_cnt == 3)
          frame_cnt = 0;
        else
          frame_cnt++;

        frames_read++;
      }

      *(volatile uint32_t *)(BASE_FR2WB + OTHERS_WB);

    }
  }

  return 0;

}
