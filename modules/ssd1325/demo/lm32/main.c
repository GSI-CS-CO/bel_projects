/* C Standard Includes */
/* ==================================================================================================== */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* GSI LM32 Includes */
/* ==================================================================================================== */
#include "irq.h"
#include "uart.h"
#include "timer.h"
#include "mini_sdb.h"
#include "aux.h"
#include "ssd1325_serial_driver.h"

/* Timer defines */
/* ==================================================================================================== */
#define TIMER_MODE_PERIODIC 1
#define TIMER_MODE_ONE_TIME 0
#define TIMER_SRC_NO_TIMER  0
#define TIMER_SRC_TIMER0    1
#define TIMER_SRC_TIMER1    2
#define TIMER_SRC_TIMER2    3
#define TIMER_SRC_TIMER3    4
#define TIMER_CAS_NO_CAS    0
#define TIMER_TICK          125000000 /* 1000ms */
#define __DELAY__           250000
#define EB_REG_IP           0x00000018
#define EP_REG_MACH         0x00000024
#define EP_REG_MACL         0x00000028
#define USE_TIMER           0

/* Externals (avoid warnings) */
/* ==================================================================================================== */
extern int mprintf(char const *format, ...);

/* Globals */
/* ==================================================================================================== */
volatile uint32_t uCounter = 0;
int8_t i_bitmap[] = { 0xff, 0xff, 0xff, 0x11, 0x55, 0x11, 0x77, 0x77, 0x77 };
int8_t i_bitmap_o[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff };
  
/* Prototypes */
/* ==================================================================================================== */
void vConfigTimer(void);
void vISR_timer(void);

/* Function vConfigTimer(...) */
/* ==================================================================================================== */
void vConfigTimer(void)
{

	/* Configuration structure */
  s_timer timer0;
	
	/* Setup */
	timer0.mode      = TIMER_MODE_PERIODIC;
	timer0.src       = TIMER_SRC_TIMER0;  
	timer0.cascade   = TIMER_CAS_NO_CAS;
	timer0.deadline  = TIMER_TICK;
	timer0.msi_dst   = 0;
	timer0.msi_msg   = 0;

	/* Write configuration and enable arm timer */
  irq_tm_write_config(0, &timer0);
  irq_tm_set_arm(1<<0);
	     
}

/* Function vISR_timer(...) */
/* ==================================================================================================== */  
void vISR_timer(void)
{

  /* Helper */
  static int8_t iChar = ' ';
  static uint32_t uPosX = 0;
  static uint32_t uPosY = 0;
  static uint32_t uDemoState = 1;  
  uint32_t uDisplayCounter;
  uint32_t uDisplayIPShift;
  uint32_t *p_uEtherBoneBase = (uint32_t*) 0x80060700;    //p_uEtherBoneBase = find_device(0x68202b22);
  uint32_t *p_ep_base = (uint32_t*) 0x80060100;           //p_ep_base        = find_device(0x650c2d4f);
  static uint8_t myIP[4];
  static uint8_t myMAC[6];
  char cFormatLine[21];
    
  /* Show counter and state */
  mprintf("vISR_timer: Working Counter:    0x%08x\n", uCounter);
  
  /* Display demo */
  switch(uDemoState)
  {
  
    /* Display all printable characters */
    /* ================================================================================ */
    case 0:
    {
      for(uPosY=5; uPosY<6; uPosY++)
      {
        for (uPosX=0; uPosX<17; uPosX++)
        {
          iSSD1325_PrintChar(iChar, uPosX, uPosY);
        }
      }
      if (iChar == 0x7f) {iChar = ' '; uDemoState = 1;}
      else               {iChar++;}
      break;
    }
    
    /* Display IP and MAC */
    case 1:
    {
            
      /* Get MAC and IP */   
      /* ================================================================================ */
      myMAC[5] = (uint8_t) ((*(p_ep_base+(EP_REG_MACL>>2))&0xff));
      myMAC[4] = (uint8_t) (((*(p_ep_base+(EP_REG_MACL>>2))&0xff00))>>8);
      myMAC[3] = (uint8_t) (((*(p_ep_base+(EP_REG_MACL>>2))&0xff0000))>>16);
      myMAC[2] = (uint8_t) (((*(p_ep_base+(EP_REG_MACL>>2))&0xff000000))>>24);
      myMAC[1] = (uint8_t) ((*(p_ep_base+(EP_REG_MACH>>2))&0xff));
      myMAC[0] = (uint8_t) ((*(p_ep_base+(EP_REG_MACH>>2))&0xff00)>>8);
      myIP[3] = (uint8_t) ((*(p_uEtherBoneBase+(EB_REG_IP>>2))&0xff));
      myIP[2] = (uint8_t) (((*(p_uEtherBoneBase+(EB_REG_IP>>2))&0xff00))>>8);
      myIP[1] = (uint8_t) (((*(p_uEtherBoneBase+(EB_REG_IP>>2))&0xff0000))>>16);
      myIP[0] = (uint8_t) (((*(p_uEtherBoneBase+(EB_REG_IP>>2))&0xff000000))>>24); 
      
      /* IP */
      iSSD1325_ClearLine(0,0x00);
      iSSD1325_PrintString("IP-Address:", 0, 0);
      iSSD1325_ClearLine(1,0x00);
      uDisplayIPShift = 0;
      for(uDisplayCounter=0; uDisplayCounter<4; uDisplayCounter++)
      {
        if(uDisplayCounter<3)
        {
          sprintf(cFormatLine, "%u.", myIP[uDisplayCounter] );
        }
        else
        {
          sprintf(cFormatLine, "%u", myIP[uDisplayCounter] );
        }
        iSSD1325_PrintString(cFormatLine, uDisplayIPShift, 1);
        if(myIP[uDisplayCounter]<10)       {uDisplayIPShift+=2;}
        else if(myIP[uDisplayCounter]<100) {uDisplayIPShift+=3;}
        else                               {uDisplayIPShift+=4;}
      }
      
      /* MAC */
      iSSD1325_ClearLine(2,0x00);
      iSSD1325_PrintString("MAC-Address:", 0, 2);
      iSSD1325_ClearLine(3,0x00);
      for(uDisplayCounter=0; uDisplayCounter<6; uDisplayCounter++)
      {
        if(uDisplayCounter<5)
        {
          sprintf(cFormatLine, "%02x:", myMAC[uDisplayCounter] );
        }
        else
        {
          sprintf(cFormatLine, "%02x", myMAC[uDisplayCounter] );
        } 
        iSSD1325_PrintString(cFormatLine, uDisplayCounter*3, 3);
      }
      
      /* Characters */
      iSSD1325_ClearLine(4,0x00); 
      iSSD1325_PrintString("Characters:", 0, 4);
      iSSD1325_ClearLine(5,0x00);   
      
      /* Colors */
      iSSD1325_ClearLine(6,0x00);
      iSSD1325_PrintString("Colors: (15..0)", 0, 6);
      iSSD1325_ClearLine(7,0x00); 
      for(uDisplayCounter=0; uDisplayCounter<16; uDisplayCounter++)
      {
        iSSD1325_ClearChar(uDisplayCounter, 7, ~(uDisplayCounter<<4|uDisplayCounter));
      }
                
      /* Done */          
      uDemoState = 2;
      break;
    }
    
    /* Draw rectangles */
    /* ================================================================================ */
    case 2:
    {
      for(uDisplayCounter=0; uDisplayCounter<8; uDisplayCounter++)
      {
        /* Xs, Ys, Xe, Ye, Pa */
        iSSD1325_DrawRectangle(63,  uDisplayCounter,  63,  uDisplayCounter+1,  0xff); 
        iSSD1325_DrawRectangle(62,  uDisplayCounter,  62,  uDisplayCounter+1,  0x77); 
        iSSD1325_DrawRectangle(61,  uDisplayCounter,  61,  uDisplayCounter+1,  0x11); 
      }
      
      for(uDisplayCounter=0; uDisplayCounter<8; uDisplayCounter++)
      {
        /* Xs, Ys, Xe, Ye, Pa */
        iSSD1325_DrawRectangle(63,  uDisplayCounter+55,  63,  uDisplayCounter+56,  0xff); 
        iSSD1325_DrawRectangle(62,  uDisplayCounter+55,  62,  uDisplayCounter+56,  0x77); 
        iSSD1325_DrawRectangle(61,  uDisplayCounter+55,  61,  uDisplayCounter+56,  0x11); 
      }
      
      /* Xs, Ys, Xe, Ye, Pa */
      iSSD1325_DrawRectangle(57,  0,  60,  63,  0xff); 
      
      /* Xs, Ys, Xe, Ye, Pa */
      iSSD1325_DrawRectangle(61,  8,  63,  55,  0x33); 
      
      /* Xs, Ys, Xe, Ye, Pa */
      iSSD1325_DrawRectangle(57, 16,  60,  47,  0x0f); 
      iSSD1325_DrawRectangle(61, 16,  63,  47,  0x70); 
      
      /* Done */          
      uDemoState = 3;
      break;
    }
    
    /* Draw Bitmap demos */
    /* ================================================================================ */
    case 3:
    {
    
      /* Draw bitmaps from array */
      iSSD1325_DrawBitmap(50, 0, 52, 2, i_bitmap); 
      iSSD1325_DrawBitmap(50, 5, 52, 7, i_bitmap);
      iSSD1325_DrawBitmap(50, 10, 53, 13, i_bitmap_o);
      iSSD1325_DrawBitmap(50, 16, 53, 19, i_bitmap_o);
      
      /* Done */
      uDemoState = 0;
      break;
    }
    
    /* Further demos */
    /* ================================================================================ */
    default:
    {
      break;
    }
    
  }
}

/* Function main(...) */
/* ==================================================================================================== */  
int main (void)
{
      
	/* Get uart unit address */
	discoverPeriphery();

	/* Initialize uart unit */
	uart_init_hw();

  /* Initialize ssd1325 driver address */
  if(iSSD1325_AutoInitialize())
  {
    mprintf("main: iSSD1325_AutoInitialize() failed!\n");
    while(true) {};
  }
  else
  {
    mprintf("main: iSSD1325_AutoInitialize() succeeded!\n");	
    iSSD1325_ResetDevice();
    for(uCounter=0; uCounter<__DELAY__; uCounter++) {};
    mprintf("main: iSSD1325_ResetDevice() succeeded!\n");	
    
    iSSD1325_ConfigureScreen();
    for(uCounter=0; uCounter<__DELAY__; uCounter++) {};
    mprintf("main: iSSD1325_ConfigureScreen() succeeded!\n");	
    
    iSSD1325_ClearLine(0,0xff);     for(uCounter=0; uCounter<__DELAY__; uCounter++) {};
    iSSD1325_ClearLine(1,0x77);     for(uCounter=0; uCounter<__DELAY__; uCounter++) {};
    iSSD1325_ClearLine(2,0x11);     for(uCounter=0; uCounter<__DELAY__; uCounter++) {};  
    iSSD1325_ClearLine(3,0x00);     for(uCounter=0; uCounter<__DELAY__; uCounter++) {};
    iSSD1325_ClearLine(4,0x00);     for(uCounter=0; uCounter<__DELAY__; uCounter++) {};
    iSSD1325_ClearLine(5,0x11);     for(uCounter=0; uCounter<__DELAY__; uCounter++) {};
    iSSD1325_ClearLine(6,0x77);     for(uCounter=0; uCounter<__DELAY__; uCounter++) {};
    iSSD1325_ClearLine(7,0xff);     for(uCounter=0; uCounter<__DELAY__; uCounter++) {};
    mprintf("main: iSSD1325_ClearLine() succeeded!\n");
    
    for(uCounter=0; uCounter<__DELAY__; uCounter++) {};
    iSSD1325_ClearScreen();
    mprintf("main: iSSD1325_ClearScreen() succeeded!\n");

    /* Initialize interrupt handling */
#if USE_TIMER    
    irq_disable();
    isr_table_clr();
    isr_ptr_table[0]= vISR_timer;
    irq_set_mask(0x01);
    irq_enable();
#endif    	
  }
  
	/* Display welcome message */
	mprintf("main: Application started ...\n");	
	
	/* Initialize timer unit */
	vConfigTimer();
	
	/* Wait fo interrupt */
	mprintf("main: Waiting for timer interrupt now...\n");	
	uCounter = 0;
	while(true)
	{
		asm volatile ("nop");
		uCounter++;
#if USE_TIMER==0
  if (uCounter>__DELAY__/2)
  {
    vISR_timer();
    uCounter = 0;
  }
#endif    
	}
	
	/* Should never get here! */
	mprintf("main: Application finished!\n");	
	return (0);
	
}
