

#include <iostream>
#include <string>
#include <scu_env.hpp>
#include <scu_mmu_fe.hpp>
#include <lm32_hexdump.h>
#include <eb_console_helper.h>
#include <unistd.h>

using namespace std;
using namespace Scu;
using namespace Scu::mmu;


void testAdd( Mmu& mmu, const MMU_TAG_T tag, size_t len, const bool create )
{
   MMU_ADDR_T startAddr = 0;

   cout << ESC_FG_MAGENTA
        << "tag:         0x" << hex << uppercase << tag << dec << "\n"
        << "len:         " << len  << "\n"
        << "create:      " << (create? "true":"false")
        << ESC_NORMAL << endl;
   cout << ESC_BOLD"allocate:    " << mmu.status2String( mmu.allocate( tag, startAddr, len, create )) << ESC_NORMAL << endl;
   cout << ESC_FG_CYAN
        << "tag:         0x" << hex << uppercase << tag << dec << "\n"
        << "startAddr:   " << startAddr << "\n"
        << "len:         " << len  << "\n"
        << "create:      " << (create? "true":"false")
        << ESC_NORMAL"\n" << endl;
}

extern "C"
{
void mmuReadItem( const MMU_ADDR_T index, MMU_ITEM_T* pItem );
void mmuWriteItem( const MMU_ADDR_T index, const MMU_ITEM_T* pItem );
}

int main( int argc, char** ppArgv )
{
   cout << "MMU-Test" << endl;
   cout << sizeof( eb_data_t ) << endl;

   eb_data_t x = 0;
   cout << "x=" << x << endl;
   x++;
   cout << "x=" << x << endl;

   string scuUrl;
   if( isRunningOnScu() )
   {
      scuUrl = "dev/wbm0";
   }
   else
   {
      if( argc < 2 )
      {
         cerr << "ERROR: Missing SCU-url!" << endl;
         return 1;
      }
      scuUrl = ppArgv[1];
      if( scuUrl.find( "tcp/" ) == string::npos )
         scuUrl = "tcp/" + scuUrl;
   }

   cout << scuUrl << endl;

   mmuEb::EtherboneConnection ebc( scuUrl );

#define TESTADDR  6000
   Mmu mmu( &ebc );
#if 0
   MMU_ITEM_T item;

   item.flags   = 0x1111;
   item.tag     = 0x2222;
   item.iNext   = 0x33333333;
   item.iStart  = 0x44444444;
   item.length  = 0x55555555;

   cout << "write:" << endl;
   ::hexdump( &item, sizeof( item ) );

   mmuWriteItem( TESTADDR, &item );
#endif

   mmu.getEb()->setDebug( true );
    uint64_t d[2] = { 0x7766554433221100, 0xFFEEDDCCBBAA9988 };
   ::hexdump( &d, sizeof( d ) );

   mmu.write( TESTADDR+0, &d[0], EB_DATA32 | EB_LITTLE_ENDIAN, 2 );
  // ::sleep( 1);
  // mmu.write( TESTADDR+1, &d[1], EB_DATA32 | EB_LITTLE_ENDIAN, 2 );
   //  mmu.getEb()->doWrite( mmu.getBase() + TESTADDR, &d[0], EB_DATA32 | EB_LITTLE_ENDIAN, 2 );

   MMU_ITEM_T item1;
   ::memset( &item1, 0, sizeof( item1 ) );
   mmuReadItem( TESTADDR, &item1 );

   cout << "read:" << endl;
   ::hexdump( &item1, sizeof( item1 ) );
#if 0
   mmu.getEb()->setDebug( false );
   mmu.clear();
   cout << "MMU present? " << (mmu.isPresent()? "Yes" : "No" ) << endl;
   cout << "Number of blocks: " << mmu.getNumberOfBlocks() << endl;

   testAdd( mmu, 0x4711, 10, true );
   cout << "MMU present? " << (mmu.isPresent()? "Yes" : "No" ) << endl;
   testAdd( mmu, 0x4711, 100, true );
   testAdd( mmu, 0x4712, 10, false );
   cout << "Number of blocks: " << mmu.getNumberOfBlocks() << endl;
   testAdd( mmu, 0x4712, 10, true );
   testAdd( mmu, 0x4711, 100, false );
   cout << "Number of blocks: " << mmu.getNumberOfBlocks() << endl;
   testAdd( mmu, 0x4713, DDR3_MAX_INDEX64 - 30, true );
   cout << "Number of blocks: " << mmu.getNumberOfBlocks() << endl;
#endif
   return 0;
}

//================================== EOF ======================================