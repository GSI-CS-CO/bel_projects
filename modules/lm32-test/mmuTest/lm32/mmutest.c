// TODO!
#include <mprintf.h>
#include <eb_console_helper.h>
#include <scu_mmu_lm32.h>


MMU_OBJ_T g_mmuObj;

void main( void )
{
   mprintf( ESC_XY( "1", "1" ) ESC_CLR_SCR "MMU-Test\n" );
   if( mmuInit( &g_mmuObj ) != OK )
   {
      mprintf( ESC_ERROR "ERROR Unable to get DDR3- RAM!\n" ESC_NORMAL );
      return;
   }
   
   
   
}

/*================================== EOF ====================================*/
