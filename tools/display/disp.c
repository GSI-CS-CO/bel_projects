#include "disp.h"

volatile unsigned int* display;
volatile unsigned int desiredDisplay;

const unsigned int devID_disp_lcd     = 0xb77a5045;
const unsigned int devID_disp_ssd1325 = 0x55d1325d;
const unsigned long vendID_GSI        = 0x0000000000000651;

void (*disp_put_loc_c)(eb_device_t device, char ascii, unsigned char row, unsigned char col) = NULL;
void (*disp_put_c)(eb_device_t device, char ascii)                                           = NULL;
void (*disp_put_s)(eb_device_t device, const char* str)                                      = NULL;
void (*disp_put_line)(eb_device_t device, const char *sPtr, unsigned char row)               = NULL;



t_disp_type init_disp(eb_device_t device)
{

  // Helpers
  struct sdb_device myOLEDisplay;
  struct sdb_device myLCDisplay;
  struct sdb_device mySSD1325Display;
  bool fFoundOLED    = false;
  bool fFoundLCD     = false;
  bool fFoundSSD1325 = false;
  t_disp_type ret    = NONE;
  int dispsFound     = 0;

  // Check for available displays

  // OLED
  dispsFound = 1;
  eb_sdb_find_by_identity(device, OLED_SDB_VENDOR_ID, OLED_SDB_DEVICE_ID, &myOLEDisplay, &dispsFound);
  if (dispsFound)
  {
    fFoundOLED=true;
    printf("%s: Found OLEDisplay... use -d %d\n", program, OLED);

  }

  // LCD
  dispsFound = 1;
  eb_sdb_find_by_identity(device, vendID_GSI, devID_disp_lcd, &myLCDisplay, &dispsFound);
  if (dispsFound)
  {
    fFoundLCD=true;
    printf("%s: Found LCDisplay... use -d %d\n", program, LCD);
  }

  // SSD1325
  dispsFound = 1;
  eb_sdb_find_by_identity(device, vendID_GSI, devID_disp_ssd1325, &mySSD1325Display, &dispsFound);
  if (dispsFound)
  {
    fFoundSSD1325=true;
    printf("%s: Found SSD1325 Display... use -d %d\n", program, SSD1325);
  }

  // Select Display
  if (fFoundOLED && (desiredDisplay==OLED))
  {
    display = (unsigned int*)(myOLEDisplay.sdb_component.addr_first);
    disp_put_loc_c = &oled_disp_put_loc_c;
    disp_put_c     = &oled_disp_put_c;
    disp_put_s     = &oled_disp_put_s;
    disp_put_line  = &oled_disp_put_line;
    printf("%s: Using OLEDisplay @ %p\n", program, display);
    ret = OLED;
  }
  else if (fFoundLCD && (desiredDisplay==LCD))
  {
    display = (unsigned int*)(myLCDisplay.sdb_component.addr_first);
    disp_put_loc_c = &lcd_disp_put_loc_c;
    disp_put_c     = &lcd_disp_put_c;
    disp_put_s     = &lcd_disp_put_s;
    disp_put_line  = &lcd_disp_put_line;
    printf("%s: Using LCDisplay @ %p\n", program, display);
    ret = LCD;
  }
  else if (fFoundSSD1325 && (desiredDisplay==SSD1325))
  {
    display = (unsigned int*)(mySSD1325Display.sdb_component.addr_first);
    disp_put_loc_c = &vSSD1325_HostPutLocC;
    disp_put_c     = &vSSD1325_HostPutC;
    disp_put_s     = &vSSD1325_HostPutS;
    disp_put_line  = &vSSD1325_HostPutLine;
    printf("%s: Using SSD1325 Display @ %p\n", program, display);
    ret = SSD1325;
  }
  else if (desiredDisplay==NONE)
  {
    // Missing desired display
    printf("%s: Sorry, no display selected (use -d to specify a device)!\n", program);
    ret = NONE;
  }
  else
  {
    // No display was found
    printf("%s: Sorry, can't find any known/selected display(s)!\n", program);
    ret = NONE;
  }

  // Done
  return ret;

}
