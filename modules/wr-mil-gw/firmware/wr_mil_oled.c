#include "wr_mil_oled.h"

uint32_t oled_loop_counter = 0;
uint32_t oled_char_counter = 0;

uint64_t oled_num_events = 0;
uint32_t oled_num_delayed_events = 0;

int oled_loop(volatile WrMilConfig *config, volatile uint32_t *oled)
{
	if (oled_loop_counter ==  1000000) {
		oled[0] = 0;
	}
	if (oled_loop_counter++ > 1000000)
	{
		oled_num_events         = config->num_events.value;
		oled_num_delayed_events = config->late_events;
		oled_write_one_char(config, oled, oled_char_counter++);
		if (oled_char_counter == 6*11) oled_char_counter = 0, oled_loop_counter = 0;
		return 1;
	}
	return 0;
}

static char line0[]  = "WR-MIL-GW  ";
static char line1a[] = "SIS18      ";
static char line1b[] = "ESR        ";
static char line1c[] = "NOT CONFIG.";
static char line2[]  = "# events:  ";
static char line4[]  = "# delayed: ";
static int row, col;
void oled_write_one_char(volatile WrMilConfig *config, volatile uint32_t *oled, int ch)
{
	if (ch == 0) {
		row = -1;
		col = 10;
	} else {
		++col;
		if (col == 11) {
			col = 0;
			++row;
		}
	}

  if (config->state==WR_MIL_GW_STATE_CONFIGURED)
  {
	  if (row == 3 || row == 5)
	  {
	    if (col == 0) { oled[2] = '0'; return; }
	    if (col == 1) { oled[2] = 'x'; return; }
	    if (col == 10) { oled[2] = ' '; return; }
	    int digit;
	    if (row == 3) digit = ((oled_num_events)         >> (((9-col)*4))&0xf);
	    if (row == 5) digit = ((oled_num_delayed_events) >> (((9-col)*4))&0xf);
	    if (digit < 10) oled[2] = '0' + digit;
	    else            oled[2] = 'a' + (digit-10);
	    return;
	  }
	  if (row == 1 && config->event_source==WR_MIL_GW_EVENT_SOURCE_SIS) { oled[2] = line1a[col]; return; }
	  if (row == 1 && config->event_source==WR_MIL_GW_EVENT_SOURCE_ESR) { oled[2] = line1b[col]; return; }
	  if (row == 2) { oled[2] = line2[col]; return; }
	  if (row == 4) { oled[2] = line4[col]; return; }
  }
  if (ch == 0) { oled[2] = '\r'; return; }
  if (row == 0) { oled[2] = line0[col]; return; }
  if (row == 1 && config->event_source==WR_MIL_GW_EVENT_SOURCE_UNKNOWN) { oled[2] = line1c[col]; return; }
  oled[2] = ' ';
  return;
}
