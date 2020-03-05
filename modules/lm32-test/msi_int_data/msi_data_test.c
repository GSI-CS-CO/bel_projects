/*!
 * @file msi_data_test.c
 * @brief Test program verifying the data type IRQ_MSI_T
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      04.03.2020
 * @see scu_msi.h
 */
#include <stdbool.h>
#include <mini_sdb.h>
#include "eb_console_helper.h"
#include "scu_msi.h"

static inline void init( void )
{
   discoverPeriphery(); // mini-sdb: get info on important Wishbone infrastructure
   uart_init_hw();      // init UART, required for printf...
}

typedef struct
{
   uint32_t  msg;
   uint32_t  adr;
   uint32_t  sel;
} msi;
STATIC_ASSERT( sizeof( msi ) == sizeof( MSI_ITEM_T ) );

volatile msi global_msi;

static inline void irq_pop_msi( uint32_t irq_no )
{
   uint32_t  offset    = (IRQ_OFFS_QUE + (irq_no<<4));    //queue is at 32 + irq_no * 16
   uint32_t* msg_queue = (uint32_t*)(pCpuIrqSlave + (offset >>2));
    global_msi.msg =  *(msg_queue+((uint32_t)IRQ_OFFS_MSG>>2));
    global_msi.adr =  *(msg_queue+((uint32_t)IRQ_OFFS_ADR>>2));
    global_msi.sel =  *(msg_queue+((uint32_t)IRQ_OFFS_SEL>>2));
    *(pCpuIrqSlave + (IRQ_REG_POP>>2)) = 1<<irq_no;
}

void printGlobalMsi( void )
{
   mprintf( "msg: 0x%08x\n", global_msi.msg );
   mprintf( "adr: 0x%08x\n", global_msi.adr );
   mprintf( "sel: 0x%08x\n", global_msi.sel );
   mprintf( "pop: 0x%08x\n\n", IRQ_MSI_CONTROL_ACCESS( pop ) );
}

IRQ_MSI_T g_msiList =
{
   .control =
   {
      .reset  = 0x11111111,
      .status = 0x22222222,
      .pop    = 0x33333333
   },
   .queue =
   {
      {
         .item.msg = 0x0101010A,
         .item.adr = 0x0101010B,
         .item.sel = 0x0101010C
      },
      {
         .item.msg = 0x0202020A,
         .item.adr = 0x0202020B,
         .item.sel = 0x0202020C
      },
      {
         .item.msg = 0x0303030A,
         .item.adr = 0x0303030B,
         .item.sel = 0x0303030C
      },
      {
         .item.msg = 0x0404040A,
         .item.adr = 0x0404040B,
         .item.sel = 0x0404040C
      }
   }
};

void printItem( const unsigned int i )
{
   MSI_ITEM_T Item;
   irqMsiCopyObjectAndRemove( &Item, i );

   mprintf( "%d: msg: 0x%08x\n", i, Item.msg );
   mprintf( "%d: adr: 0x%08x\n", i, Item.adr );
   mprintf( "%d: sel: 0x%08x\n", i, Item.sel );
   mprintf( "reset:  0x%08x\n",   IRQ_MSI_CONTROL_ACCESS( reset ) );
   mprintf( "status: 0x%08x\n",   IRQ_MSI_CONTROL_ACCESS( status ) );
   mprintf( "pop:    0x%08x\n\n", IRQ_MSI_CONTROL_ACCESS( pop ) );

}


void main( void )
{
   init();
   #if __GNUC__ >= 9
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
   #endif

   pCpuIrqSlave = (uint32_t*)&g_msiList;

   #if __GNUC__ >= 9
    #pragma GCC diagnostic pop
   #endif

   mprintf( "MSI-IRQ data test\nCompiler: " COMPILER_VERSION_STRING "\n" );

   irq_pop_msi( 0 );
   printGlobalMsi();
   irq_pop_msi( 1 );
   printGlobalMsi();
   irq_pop_msi( 2 );
   printGlobalMsi();
   irq_pop_msi( 3 );
   printGlobalMsi();

   printItem( 0 );
   printItem( 1 );
   printItem( 2 );
   printItem( 3 );
   while( true );
}

/*================================== EOF ====================================*/
