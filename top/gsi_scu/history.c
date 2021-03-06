/* history.c */

#ifdef HISTORY

#include <stdio.h>
#include <inttypes.h>
#include <history.h>
#include "mprintf.h"


/* Define PRINTF appropriate for the operating system being used */
#define PRINTF	mprintf

extern uint64_t getTick();


/* Allocate space for the circular history buffer */
HistItem histbuf[HISTSIZE];

/* Variables used to maintain the circular history buffer */
UINT32 histidx; 			/* next empty slot */
UINT32 histstart;			/* oldest item */
UINT32 histSubsystemsEnabled;	/* used to mask/unmask module logging */ 

void hist_init(UINT32 subsystemsEnabled)
{
   histstart = 0;
   histidx = 0;
   histSubsystemsEnabled = subsystemsEnabled;
   hist_add(HISTORY_BOOT,"init");
}

void hist_enableSubsystem(UINT32 bit)
{
   histSubsystemsEnabled |= bit;
}

void hist_disableSubsystem(UINT32 bit)
{
   histSubsystemsEnabled &= ~bit;
}

void hist_addx(UINT32 subsystem, char *msg, unsigned char data)
{
   //UINT32 interruptEnabledState = disableInterrupts();
   if ( subsystem & histSubsystemsEnabled ) {
      histbuf[histidx].timeStamp = getTick();  /* whatever is appropriate */
      histbuf[histidx].message = msg;
      histbuf[histidx].associatedData = data;
      if ( ++histidx >= HISTSIZE ) {
         histidx = 0;
      }
      if ( histidx == histstart ) {
         if ( ++histstart >= HISTSIZE ) 
           histstart = 0;
      }
   }
   //enableInterrupts(interruptEnabledState);
}

void hist_add(UINT32 subsystem, char *msg)
{
   hist_addx(subsystem, msg, NOVAL);
}

void hist_print(int doReturn)
{
   UINT32 idx = histstart;

   PRINTF("*********** history *************\n");
   while ( idx != histidx )
   {
      PRINTF("%u :%s",(unsigned int)((histbuf[idx].timeStamp)/1000ULL),histbuf[idx].message);
      if ( histbuf[idx].associatedData != NOVAL ) {
         PRINTF(":0x%02x",histbuf[idx].associatedData);
      }
      PRINTF("\n\r");
      idx++;
      if ( idx >= HISTSIZE ) {
         idx = 0;
      }
   }
   PRINTF("*********** end history *************\n\n");

   while ( ! doReturn ) 
   {
      ;
   }
}

#endif /* HISTORY */
