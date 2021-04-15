
#include <mprintf.h>
#include <eb_console_helper.h>
#include <stdbool.h>
#include <lm32Interrupts.h>

extern uint32_t _ebss;
extern uint32_t _fbss;
extern uint32_t _edata;
extern uint32_t _fdata;

volatile int global0 = 0;
volatile int global1 = -1;

extern volatile uint32_t __reset_count;

void main( void )
{
   mprintf( ESC_CLR_SCR ESC_XY( "1", "1" ) "%d, %d, IRQ-nesting: %d\n", global0, global1, irqGetAtomicNestingCount() );
   mprintf( "__reset_count: 0x%X, %d\n", __reset_count, __reset_count );
   global0++;
   global1++;
   mprintf( "%d, %d\n", global0, global1 );
   mprintf( "_edata:   0x%p\n", _edata );
   mprintf( "_fdata:   0x%p\n", _fdata );
   mprintf( "_ebss:   0x%p\n", _ebss );
   mprintf( "_fbss:   0x%p\n", _fbss );
   mprintf( "&global0: 0x%p\n", &global0 );
   mprintf( "&global1: 0x%p\n", &global1 );
   while( true );
}
