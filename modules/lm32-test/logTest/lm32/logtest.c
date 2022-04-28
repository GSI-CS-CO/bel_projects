// TODO!
#include <mprintf.h>
#include <eb_console_helper.h>
#include <stdbool.h>
#include <lm32_syslog.h>


void main( void )
{
   MMU_STATUS_T status;

   mprintf( "MMU present?: %s\n", mmuIsPresent()? "yes": "no"  );
   status = syslogInit( 20 );
   mprintf( "\n%s\n", mmuStatus2String( status ) );
   if( !mmuIsOkay( status ) )
   {
      mprintf( ESC_ERROR "ERROR Unable to get DDR3- RAM!\n" ESC_NORMAL );
      return;
   }

   const char* text = "Das ist ein Text im LM32.";
   syslog( 0, "A: Text in syslog: \"%s\", %.32b", text, 4711 );
   syslog( 0, "B: Noch ein Text in syslog!, C = %c", 'u' );
   syslog( 0, "C: Noch ein anderer Text in syslog!, 0x%08X, %d", 4711, 4711 );

   mprintf( "\nText: \"%s\"\nAddress: 0x%p\n", text, text );

   while( true );
}

/*================================== EOF ====================================*/
