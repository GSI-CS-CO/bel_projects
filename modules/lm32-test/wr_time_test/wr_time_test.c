
#include <stdbool.h>
#include "scu_wr_time.h"
#include "eb_console_helper.h"
#include "helper_macros.h"

/*! ---------------------------------------------------------------------------
 */
STATIC inline void init( void )
{
   discoverPeriphery(); // mini-sdb: get info on important Wishbone infrastructure
   uart_init_hw();      // init UART, required for printf...
}

void main( void )
{
   init();

   mprintf( ESC_XY( "1", "1" ) ESC_CLR_SCR ESC_NORMAL  __FILE__ "\n"
            "Compiler:  " COMPILER_VERSION_STRING "\n"
          );

   while( true )
   {
      const uint64_t wrt = getWrSysTime();
      mprintf( ESC_XY( "1", "3" ) ESC_CLR_LINE );
      for( int i = 0; i < sizeof( wrt ); i++ )
      {
         mprintf( "0x%02x ", ((uint8_t*)&wrt)[i] );
      }

      for( unsigned int i = 0; i < (USRCPUCLK * 1000) / 16; i++ )
         NOP();
   }
}
