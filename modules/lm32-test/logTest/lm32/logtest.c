// TODO!
#include <mprintf.h>
#include <eb_console_helper.h>
#include <stdbool.h>
#include <lm32_syslog.h>
#include <scu_wr_time.h>
#include <scu_lm32Timer.h>
#include <lm32Interrupts.h>





#define configCPU_CLOCK_HZ   (USRCPUCLK * 1000)


static void onTimerInterrupt( const unsigned int intNum, const void* pContext )
{
   static unsigned int i = 0;

   lm32Log( 6, "Timer interrupt: %u -> %s\n", i, (i%2 == 0)? "even": "odd");
   i++;
}

void main( void )
{
   MMU_STATUS_T status;

   mprintf( "MMU present?: %s\n", mmuIsPresent()? "yes": "no"  );
   status = lm32LogInit( 20 );
   mprintf( "\n%s\n", mmuStatus2String( status ) );
   if( !mmuIsOkay( status ) )
   {
      mprintf( ESC_ERROR "ERROR Unable to get DDR3- RAM!\n" ESC_NORMAL );
      return;
   }

   SCU_LM32_TIMER_T* pTimer = lm32TimerGetWbAddress();
   if( (unsigned int)pTimer == ERROR_NOT_FOUND )
   {
      mprintf( ESC_ERROR "ERROR: Timer not found!\n" ESC_NORMAL );
      while( true );
   }

   lm32TimerSetPeriod( pTimer, configCPU_CLOCK_HZ );
   lm32TimerEnable( pTimer );
   irqRegisterISR( TIMER_IRQ, (void*)pTimer, onTimerInterrupt );

   const char* text = ESC_FG_CYAN ESC_BOLD"Das ist ein Text im LM32."ESC_NORMAL;

   lm32Log( 0, "A: Text in lm32Log: \"%s\", %.32b %%", text, 4711 );
   lm32Log( 1, "B: Noch ein Text in lm32Log!, C = %c", 'u' );
   lm32Log( 2, "C: Noch ein anderer Text in lm32Log!, 0x%08X, %d", 0xAFFE, 4711 );
   lm32Log( 3, "D: Und noch was in lm32Log!, %u, %_9d, %d, %d, %d", -4711, -4711, 100, 200, 300 );
   lm32Log( 4, "E: noch was..." );
   mprintf( "\nText: \"%s\"\nAddress: 0x%p\n", text, text );

   irqEnable();

   uint64_t triggerTime = 0;
   unsigned int c = 0;
   while( true )
   {
      uint64_t time = getWrSysTime();
      if( time < triggerTime )
         continue;
      triggerTime = time + 1001100000ll;

      lm32Log( 5, "Count = %u -> %s", c, (c%2 == 0)? "even": "odd" );
      c++;
   }
}

/*================================== EOF ====================================*/
