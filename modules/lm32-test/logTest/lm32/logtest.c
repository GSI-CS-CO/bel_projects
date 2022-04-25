// TODO!
#include <mprintf.h>
#include <eb_console_helper.h>
#include <stdbool.h>
#include <lm32_syslog.h>


MMU_OBJ_T g_mmuObj;

void main( void )
{
   if( syslogInit( 100 ) != 0 )
   {
      mprintf( ESC_ERROR "ERROR Unable to get DDR3- RAM!\n" ESC_NORMAL );
      return;
   }

   MMU_ADDR_T startAddr;

  // MMU_STATUS_T status = mmuAlloc( TAG_LM32_LOG, &startAddr, &len, true );
  // mprintf( "Allocate status: \"%s\"\n", mmuStatus2String( status ) );


   const char* text = "Das ist ein Text im LM32.";
   mprintf( "Text: \"%s\"\nAddress: 0x%p\n", text, text );

   while( true );
}

/*================================== EOF ====================================*/
