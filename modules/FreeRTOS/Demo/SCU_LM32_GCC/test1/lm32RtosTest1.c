
#include "eb_console_helper.h"
#include "mini_sdb.h"
#include "helper_macros.h"

void init( void )
{
   discoverPeriphery();   // mini-sdb: get info on important Wishbone infrastructure
   uart_init_hw();        // init UART, required for printf...
}


int main( void )
{
   init();
   mprintf( "freeRTOS-test\nCompiler: " COMPILER_VERSION_STRING "\n" );
   return 0;
}
