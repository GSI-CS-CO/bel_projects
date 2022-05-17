/*!
 * @file scu_command_handler.c
 * @brief  Module for receiving of commands from SAFT-LIB
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 03.02.2020
 * Outsourced from scu_main.c
 */
#include <scu_fg_macros.h>
#include <scu_fg_list.h>
#ifdef CONFIG_MIL_FG
  #include <scu_mil_fg_handler.h>
#endif
#include <scu_command_handler.h>
#ifdef CONFIG_SCU_DAQ_INTEGRATION
  #include <daq_main.h>
  #include <daq_command_interface_uc.h>
#endif

//extern FG_MESSAGE_BUFFER_T    g_aMsg_buf[QUEUE_CNT];
extern FG_CHANNEL_T           g_aFgChannels[MAX_FG_CHANNELS];
extern volatile uint16_t*     g_pScub_base;
#ifdef CONFIG_MIL_FG
extern volatile unsigned int* g_pScu_mil_base;
#endif
#ifdef DEBUG_SAFTLIB
  #warning "DEBUG_SAFTLIB is defined! This could lead to timing problems!"
#endif

//#define CONFIG_DEBUG_SWI

/*
 * Creating a message queue for by the interrupt received messages from SAFT-LIB
 */
QUEUE_CREATE_STATIC( g_queueSaftCmd, MAX_FG_CHANNELS, SAFT_CMD_T );

#ifdef CONFIG_DEBUG_SWI
#warning Function printSwIrqCode() is activated! In this mode the software will not work!
/*! ---------------------------------------------------------------------------
 * @brief For debug purposes only!
 */
STATIC inline
void printSwIrqCode( const unsigned int code, const unsigned int value )
{
   mprintf( ESC_DEBUG "SW-IRQ: %s\tValue: %2d" ESC_NORMAL "\n",
            fgCommand2String( code ), value );
}
#else
#define printSwIrqCode( code, value )
#endif


/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @brief Handles so called software interrupts (SWI) coming from SAFTLIB.
 */
ONE_TIME_CALL void saftLibCommandHandler( void )
{
#if defined( CONFIG_MIL_FG ) && defined( CONFIG_READ_MIL_TIME_GAP )
   if( !isMilFsmInST_WAIT() )
   { /*
      * Wait a round...
      */
      return;
   }
#endif
#ifdef _CONFIG_MEASURE_COMMAND_HANDLER
   #warning _CONFIG_MEASURE_COMMAND_HANDLER is activated!
   TIME_MEASUREMENT_T tm = TIME_MEASUREMENT_INITIALIZER;
#endif

   SAFT_CMD_T cmd;
  /*
   * Is a message from SATF-LIB for FG present?
   */
   if( !queuePopSave( &g_queueSaftCmd, &cmd ) )
   { /*
      * No, leave this function.
      */
      return;
   }

   /*
    * Signal busy to saftlib.
    */
   g_shared.oSaftLib.oFg.busy = 1;

   const unsigned int code  = GET_UPPER_HALF( cmd );
   const unsigned int value = GET_LOWER_HALF( cmd );

  /*
   * When debug mode active only.
   */
   printSwIrqCode( code, value );
   lm32Log( LM32_LOG_CMD, "MSI command: %s( %u )\n", fgCommand2String( code ), value );
#ifdef CONFIG_USE_HISTORY
   if( code != FG_OP_PRINT_HISTORY )
      hist_addx( HISTORY_XYZ_MODULE, fgCommand2String( code ), value );
#endif

  /*
   * Verifying the command parameter for all commands with a
   * array index as parameter.
   */
   switch( code )
   {
      case FG_OP_RESET_CHANNEL:       /* Go immediately to next case. */
      case FG_OP_ENABLE_CHANNEL:      /* Go immediately to next case. */
      case FG_OP_DISABLE_CHANNEL:     /* Go immediately to next case. */
      case FG_OP_CLEAR_HANDLER_STATE:
      {
         if( value < ARRAY_SIZE( g_aFgChannels ) )
            break;

        /*
         * In the case of a detected parameter error this function
         * becomes terminated.
         */
      #ifdef CONFIG_USE_LM32LOG
         lm32Log( LM32_LOG_ERROR,"Value %d out of range!\n", value );
      #else
         mprintf( ESC_ERROR "Value %d out of range!\n" ESC_NORMAL, value );
      #endif
      #ifdef CONFIG_USE_HISTORY
         hist_addx( HISTORY_XYZ_MODULE, "ERROR: Value out of range!", value );
      #endif
         /*
          * signal done to saftlib
          */
         g_shared.oSaftLib.oFg.busy = 0;
         return;
      }
      default: break;
   }

   /*
    * Executing the SAFT-LIB command if known.
    */
   switch( code )
   {
      case FG_OP_RESET_CHANNEL:
      {
         fgResetAndInit( g_shared.oSaftLib.oFg.aRegs,
                         value,
                         g_shared.oSaftLib.oFg.aMacros,
                         (void*)g_pScub_base
                        #ifdef CONFIG_MIL_FG
                         ,(void*)g_pScu_mil_base
                        #endif
                       );
       #ifdef CONFIG_USE_SENT_COUNTER
         g_aFgChannels[value].param_sent = 0;
      #endif
         break;
      }

      case FG_OP_MIL_GAP_INTERVAL:
      {
      #ifdef _CONFIG_VARIABLE_MIL_GAP_READING
         g_gapReadingTime = value;
      #else
       #ifdef CONFIG_USE_LM32LOG
         lm32Log( LM32_LOG_ERROR, "No variable MIL gap reading support!" );
       #else
         mprintf( ESC_ERROR "No variable MIL gap reading support!\n" ESC_NORMAL );
       #endif
      #endif
         break;
      }

      case FG_OP_ENABLE_CHANNEL:
      { /*
         * Start of a function generator.
         */
         scuBusEnableMeassageSignaledInterrupts( value ); //duration: 0.03 ms
         fgEnableChannel( value ); //duration: 0.12 ms
         break;
      }

      case FG_OP_DISABLE_CHANNEL:
      { /*
         * Stop of a function generator.
         */
         fgDisableChannel( value );
      #ifdef DEBUG_SAFTLIB
         mprintf( "-%d ", value );
      #endif
         break;
      }

      case FG_OP_RESCAN:
      { /*
         * Rescaning of all function generators.
         */
         scanFgs();
         break;
      }

      case FG_OP_CLEAR_HANDLER_STATE:
      {
       #ifdef CONFIG_MIL_FG
         fgMilClearHandlerState( value );
       #else
        #ifdef CONFIG_USE_LM32LOG
          lm32Log( LM32_LOG_ERROR, "No MIL support!\n" );
        #else
          mprintf( ESC_ERROR "No MIL support!\n" ESC_NORMAL );
        #endif
       #endif
         break;
      }

      case FG_OP_PRINT_HISTORY:
      {
       #ifdef CONFIG_DBG_MEASURE_IRQ_TIME
         mprintf( "\n\nLast IRQ time: ");
         timeMeasurePrintMillisecondsSafe( &g_irqTimeMeasurement );
         mprintf( " ms\n\n" );
       #endif
       #ifdef CONFIG_USE_HISTORY
         hist_print( true );
       #else
        #ifdef CONFIG_USE_LM32LOG
         lm32Log( LM32_LOG_WARNING, "No history support!\n" );
        #else
         mprintf( ESC_ERROR "No history!\n" ESC_NORMAL );
        #endif
       #endif
         break;
      }

      default:
      {
      #ifdef CONFIG_USE_LM32LOG
         lm32Log( LM32_LOG_ERROR, ESC_ERROR
                  "Error: Unknown MSI-command! op-code: 0x04%X, value: 0x%04X\n"
                  ESC_NORMAL, code, value );
      #else
         mprintf( ESC_ERROR
                  "Error: Unknown MSI-command! op-code: 0x%X, value: 0x%X\n"
                  ESC_NORMAL, code, value );
      #endif
         break;
      }
   }

   /*
    * signal done to saftlib
    */
   g_shared.oSaftLib.oFg.busy = 0;
}

/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @brief Software irq handler
 *
 * dispatch the calls from linux to the helper functions
 * called via scheduler in main loop
 * @see schedule
 */
void commandHandler( void )
{
   saftLibCommandHandler();

#ifdef CONFIG_SCU_DAQ_INTEGRATION
   /*!
    * Executing a possible ADDAC-DAQ command if requested...
    */
   executeIfRequested( &g_scuDaqAdmin );
#endif
}

/*================================== EOF ====================================*/
