/*!
 *  @file history.c
 *  @brief Administration of history buffer
 *
 *  @date ?
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *  Origin Stefan Rauch (maybe)
 ******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
#ifndef CONFIG_USE_HISTORY
 #error Macro CONFIG_USE_HISTORY has to be defined in Makefile when this modlle schall be compiled!
#endif

#include <history.h>
#include <scu_wr_time.h>
#include <lm32Interrupts.h>
#include <mprintf.h>

#if defined( CONFIG_USE_INTERRUPT_TIMESTAMP ) && defined( CONFIG_MIL_FG )
   #include <scu_mil_fg_handler.h>
#endif

/* Define PRINTF appropriate for the operating system being used */
#define PRINTF  mprintf

/*!
 * @ingroup HISTORY
 * @brief  Allocated space for the circular history buffer
 */
HistItem mg_aHistbuf[HISTSIZE];

/* Variables used to maintain the circular history buffer */
unsigned int mg_histidx;                 /*!<@brief next empty slot */
unsigned int mg_histstart;               /*!<@brief oldest item */
unsigned int mg_histSubsystemsEnabled = 0;   /*!<@brief used to mask/unmask module logging */

/*! ---------------------------------------------------------------------------
 * @see history.h
 */
void hist_init( const uint32_t subsystemsEnabled )
{
   ATOMIC_SECTION()
   {
      mg_histstart = 0;
      mg_histidx = 0;
      mg_histSubsystemsEnabled = subsystemsEnabled;
      hist_add( HISTORY_BOOT, "init" );
   }
}

/*! ---------------------------------------------------------------------------
 */
void hist_enableSubsystem( const uint32_t bit )
{
   mg_histSubsystemsEnabled |= bit;
}

/*! ---------------------------------------------------------------------------
 */
void hist_disableSubsystem( const uint32_t bit )
{
   mg_histSubsystemsEnabled &= ~bit;
}

/*! ---------------------------------------------------------------------------
 * @see history.h
 */
void hist_addx( const uint32_t subsystem, const char *msg, const HIST_VALUE_T data )
{
   if( (subsystem & mg_histSubsystemsEnabled) == 0 )
      return;

   ATOMIC_SECTION()
   {
      mg_aHistbuf[mg_histidx].timeStamp = getWrSysTime();
      mg_aHistbuf[mg_histidx].message = msg;
      mg_aHistbuf[mg_histidx].associatedData = data;
      mg_histidx++;
      mg_histidx %= ARRAY_SIZE( mg_aHistbuf );
      if( mg_histidx == mg_histstart )
      { /*
         * Removing of the oldest item.
         */
         mg_histstart++;
         mg_histstart %= ARRAY_SIZE( mg_aHistbuf );
      }
   }
}

#define CONFIG_HIST_NO_REPEAT_OLD_LOGS

/*! ---------------------------------------------------------------------------
 * @see history.h
 */
void hist_print( const bool doReturn )
{
#ifdef CONFIG_SMALL_HISTORY_VALUE
  #define _FORMAT_ ": 0x%02X"
#else
  #define _FORMAT_ ": 0x%08X"
#endif
   unsigned int idx = mg_histstart;

   PRINTF("\n*********** history *************\n");
   while( idx != mg_histidx )
   {
      PRINTF( "%u: \"%s\"",
              (unsigned int)((mg_aHistbuf[idx].timeStamp)/1000ULL),
              mg_aHistbuf[idx].message
            );
      if( mg_aHistbuf[idx].associatedData != HIST_NOVAL )
      {
         PRINTF( _FORMAT_, mg_aHistbuf[idx].associatedData );
      }
      PRINTF("\n");
      idx++;
      idx %= ARRAY_SIZE( mg_aHistbuf );
   }
#if defined( CONFIG_USE_INTERRUPT_TIMESTAMP ) && defined( CONFIG_MIL_FG )
   PRINTF( "\nMIL-interrupt duration:\n" );
   for( unsigned int i = 0; i < ARRAY_SIZE( g_aMilTaskData ); i++ )
   {
      unsigned int time = (unsigned int)g_aMilTaskData[i].irqDurationTime;
      g_aMilTaskData[i].irqDurationTime = 0LL;
      PRINTF( "Task %u: %u.%06u ms\n", i, time / 1000000, time % 1000000 );
   }
#endif
#ifdef CONFIG_HIST_NO_REPEAT_OLD_LOGS
   mg_histstart = idx;
#endif
   if( !doReturn )
      PRINTF( "+++ System stopped! +++\n" );
   PRINTF("*********** end history *************\n\n");

   while( !doReturn )
   {
      ;
   }
}

//#endif /* HISTORY */
/*================================== EOF ====================================*/
