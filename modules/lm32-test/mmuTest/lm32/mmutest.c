// TODO!
#include <mprintf.h>
#include <eb_console_helper.h>
#include <scu_mmu_lm32.h>


MMU_OBJ_T g_mmuObj;

extern const uint32_t MMU_MAGIC;
extern const MMU_ADDR_T MMU_LIST_START;


void testAdd( const MMU_TAG_T tag, size_t len, const bool create )
{
   MMU_ADDR_T startAddr;
   mprintf( ESC_FG_MAGENTA
            "tag:       0x%X\n"
            "len:       %u\n"
            "create:    %s\n"
            ESC_NORMAL,
            tag,
            len,
            create? "true":"false" );
   mprintf( ESC_BOLD
            "mmuAlloc:  %s\n"
            ESC_NORMAL,
            mmuStatus2String( mmuAlloc( tag, &startAddr, &len, create ) ));
   mprintf( ESC_FG_CYAN
            "tag:       0x%X\n"
            "startAddr: %u\n"
            "len:       %u\n"
            "create:    %s\n\n"
            ESC_NORMAL,
            tag,
            startAddr,
            len,
            create? "true":"false" );
}


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

   testAdd( 0x4711, 10, true );

   mprintf( "MMU present?: %s\n", mmuIsPresent()? "yes": "no"  );
   mprintf( "Number of blocks = %u\n", mmuGetNumberOfBlocks() );


   testAdd( 0x4711, 100, true );

   testAdd( 0x4712, 10, false );

   mprintf( "Number of blocks = %u\n", mmuGetNumberOfBlocks() );

   testAdd( 0x4712, 10, true );

   testAdd( 0x4711, 100, false );
   mprintf( "Number of blocks = %u\n", mmuGetNumberOfBlocks() );

   testAdd( 0x4713, DDR3_MAX_INDEX64 - 30, true );
   mprintf( "Number of blocks = %u\n", mmuGetNumberOfBlocks() );

}

/*================================== EOF ====================================*/
