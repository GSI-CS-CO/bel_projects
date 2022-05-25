// TODO!
#include <mprintf.h>
#include <eb_console_helper.h>
#include <lm32_hexdump.h>
#include <scu_lm32Timer.h>
#include <lm32Interrupts.h>
#include <string.h>
#include <scu_mmu_lm32.h>


MMU_OBJ_T g_mmuObj;

extern const uint32_t MMU_MAGIC;
extern const MMU_ADDR_T MMU_LIST_START;


STATIC_ASSERT( sizeof(unsigned int) == sizeof(uint32_t) );
STATIC_ASSERT( sizeof(unsigned short) == sizeof(uint16_t) );

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

void mmuReadItem( const MMU_ADDR_T index, MMU_ITEM_T* pItem );
void mmuWriteItem( const MMU_ADDR_T index, const MMU_ITEM_T* pItem );

void main( void )
{
   mprintf( ESC_XY( "1", "1" ) ESC_CLR_SCR "MMU-Test\n" );
   if( mmuInit( &g_mmuObj ) != OK )
   {
      mprintf( ESC_ERROR "ERROR Unable to get DDR3- RAM!\n" ESC_NORMAL );
      return;
   }

   MMU_ITEM_T item;

   item.flags   = 0x1111;
   item.tag     = 0x2222;
   item.iNext   = 0x33333333;
   item.iStart  = 0x44444444;
   item.length  = 0x55555555;

   mprintf( "write:\n" );
   hexdump( &item, sizeof( item ) );
   mmuWriteItem( 2000, &item );

   MMU_ITEM_T item1;
   memset( &item1, 0, sizeof( item1 ) );
   mmuReadItem( 2000, &item1 );
   mprintf( "read:\n" );
   hexdump( &item1, sizeof( item1 ) );

#if 1
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
#endif

}

/*================================== EOF ====================================*/
