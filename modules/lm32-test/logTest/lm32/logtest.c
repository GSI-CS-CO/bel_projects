// TODO!
#include <mprintf.h>
#include <eb_console_helper.h>
#include <stdbool.h>
#include <scu_mmu_lm32.h>
#include <scu_mmu_tag.h>

MMU_OBJ_T g_mmuObj;

void main( void )
{
   if( mmuInit( &g_mmuObj ) != OK )
   {
      mprintf( ESC_ERROR "ERROR Unable to get DDR3- RAM!\n" ESC_NORMAL );
      return;
   }

   MMU_ADDR_T startAddr;
   size_t     len = 100;

   MMU_STATUS_T status = mmuAlloc( TAG_LM32_LOG, &startAddr, &len, true );
   mprintf( "Allocate status: \"%s\"\n", mmuStatus2String( status ) );


   const char* text = "Das ist ein Text.";
   mprintf( "Text: \"%s\"\nAddress: 0x%p\n", text, text );

   while( true );
}

/*================================== EOF ====================================*/
