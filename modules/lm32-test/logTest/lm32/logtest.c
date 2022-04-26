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
   syslog( 0, "Text in syslog: \"%s\", %d", text, 4711 );
   syslog( 0, "Noch ein Text in syslog!" );


   mprintf( "Text: \"%s\"\nAddress: 0x%p\n", text, text );

   while( true );
}

/*================================== EOF ====================================*/
