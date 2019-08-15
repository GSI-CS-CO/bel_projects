#include "wr_mil_oled.h"

static uint32_t oled_loop_counter = 0;
static uint32_t oled_char_counter = 0;

static uint64_t oled_num_events = 0;
static uint32_t oled_num_delayed_events = 0;

void oled_numbers(uint32_t *six_numbers, volatile uint32_t *oled) {
	for (int row = 0; row < 6; ++row) {
		for (int col = 0; col < 11; ++col) {
			char ch = ' ';
			if (col == 0) {
				ch = '0';
			} else if (col == 1) {
				ch = 'x';
			} else if (col < 10) {
				int digit = ((six_numbers[row]) >> (((9-col)*4))&0xf);
				if (digit < 10) ch = digit + '0'; else ch = digit-10+'a';
			}
			oled[3] = (row<<12) | (col<<8) | ch;
		}
	}
}

void oled_array(volatile WrMilConfig *config, volatile uint32_t *oled) {
	static char text[6][14] = {
		          "WR-MIL-GW   ",
                  "            ", 
                  "# events    ",
                  "0x          ",
                  "# delayed   ",
                  "0x          "
              };
	static char line1a[] = "SIS18        ";
	static char line1b[] = "ESR          ";
	static char line1c[] = "NOT CONFIG.  ";
	oled[0] = 0;
	for (int row = 0; row < 6; ++row) {
		for (int col = 0; col < 11; ++col) {
			char ch = text[row][col];
			if (row == 1) {
				     if (config->event_source==WR_MIL_GW_EVENT_SOURCE_SIS) ch = line1a[col]; 
				else if (config->event_source==WR_MIL_GW_EVENT_SOURCE_ESR) ch = line1b[col]; 
				else                                                       ch = line1c[col]; 
			} else if (row == 3 && col > 1 && col < 10) {
				int digit = ((config->num_events.value) >> (((9-col)*4))&0xf);
				if (digit < 10) ch = digit + '0'; else ch = digit-10+'a';
			} else if (row == 5 && col > 1 && col < 10) {
				int digit = ((config->late_events) >> (((9-col)*4))&0xf);
				if (digit < 10) ch = digit + '0'; else ch = digit-10+'a';
			}
			oled[3] = (row<<12) | (col<<8) | ch;
		}
	}
}

