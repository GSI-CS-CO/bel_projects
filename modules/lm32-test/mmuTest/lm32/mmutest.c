// TODO!
#include <mprintf.h>
#include <eb_console_helper.h>
#include <scu_mmu_lm32.h>


MMU_OBJ_T g_mmuObj;

extern const uint32_t MMU_MAGIC;
extern const MMU_ADDR_T MMU_LIST_START;

void main( void )
{
   mprintf( ESC_XY( "1", "1" ) ESC_CLR_SCR "MMU-Test\n" );
   if( mmuInit( &g_mmuObj ) != OK )
   {
      mprintf( ESC_ERROR "ERROR Unable to get DDR3- RAM!\n" ESC_NORMAL );
      return;
   }

   mmuDelete();

   mprintf( "MMU present?: %s\n", mmuIsPresent()? "yes": "no"  );
   
  // RAM_PAYLOAD_T b = { .ad32[0] = MMU_MAGIC, .ad32[1] = 0 };
  // mmuWrite( MMU_LIST_START, &b, 1 );
   MMU_ADDR_T a;
   size_t len = 10;
   mmuAlloc( 4711, &a, &len );

   mprintf( "MMU present?: %s\n", mmuIsPresent()? "yes": "no"  );



   RAM_PAYLOAD_T x;
   mmuRead( MMU_LIST_START, &x, 1 );
   mprintf( "ad32[0] = 0x%08X, ad32[1] = 0x%08X\n", x.ad32[0], x.ad32[1] );


   mprintf( "Listitem length = %u\n", sizeof( MMU_ITEM_T ) / sizeof( RAM_PAYLOAD_T ) );
   mprintf( "Number of blocks = %u\n", mmuGetNumberOfBlocks() );

}

/*================================== EOF ====================================*/
