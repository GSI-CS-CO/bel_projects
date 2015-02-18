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
#define SLOT_RW         4
//#define WORDS_FRAME     8
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
uint32_t header[ETH_HEADER];
uint32_t written_slot = 0;
uint32_t words_in_frame = 32;
uint32_t cnt;
uint32_t frame_chunk;
uint32_t words_frame;
uint32_t frame_cnt = 0;
uint32_t counter_ppsg;
int32_t delta;
int32_t pg_delta;
int32_t pg_rx_seconds;
uint64_t sec;

unsigned int pg_tx_seconds;
unsigned int pg_tx_nano;
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
unsigned int slot = 0;
unsigned int slot_max = 0;
unsigned int prev_cnt = 0;

init();

pp_printf("FEC Unit starting!\n");
pp_printf("SDB Record %x \n", r_sdb_add());
pp_printf("FEC Wb %x \n",*(volatile uint32_t *)(BASE_FR2WB+OTHERS_WB));
words_frame = *(volatile uint32_t *)(BASE_FR2WB + WORDS_FRAME_WB);
pp_printf("Words per Frame %d \n",words_frame);
slot_max = *(volatile uint32_t *)(BASE_FR2WB + WORDS_MEMORY);
pp_printf("Max Slot %x \n",slot_max);

while(1)
{
        written_slot = *(volatile uint32_t *)(BASE_FR2WB + SLOT_RW);

        if( written_slot & (0x1<<slot))
        {
                //pp_printf("\n ADD %08x Writen slot %d Read slot %d \n",(slot*words_frame) + cnt, written_slot, 0x1<<slot);
                //frame_chunk = *(volatile uint32_t *)(BASE_FRAME_RAM + (slot*words_frame) + 6) & 0xffff;
                frame_chunk = *(volatile uint32_t *)(BASE_FRAME_RAM + (slot*words_frame) + 12);
                //pp_printf("%u ADD %08x Writen slot %d Read slot %d \n",frame_chunk, (slot*words_frame) + cnt, written_slot, 0x1<<slot);
                //pp_printf("%u ADD %d SLOT \n",frame_chunk, 0x1<<slot);
                //pp_printf("%u  \n",frame_chunk);

                if (prev_cnt != (frame_chunk -1)) {
                        pp_printf("---Missing Prev %d Cnt %d diff %d slot %d \n", prev_cnt, frame_chunk, frame_chunk - prev_cnt, 0x1<<slot);
                }
                prev_cnt = frame_chunk;

                *(volatile uint32_t *)(BASE_FR2WB + SLOT_RW) = slot;
        }

        if(slot < slot_max-1)
            slot += 1;
        else
            slot = 0;
}

return 0;

}
