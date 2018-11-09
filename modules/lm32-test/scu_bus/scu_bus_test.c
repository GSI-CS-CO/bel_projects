

#include <stdbool.h>
#include "mprintf.h"
#include "mini_sdb.h"
#include "../../top/gsi_scu/scu_bus.h"
#include "helper_macros.h"
#include "lm32_assert.h"

#define SCUBUS_INVALID_VALUE 0xdead

typedef uint16_t SCU_BUS_SLAVE_FLAGS_T;
STATIC_ASSERT( sizeof( SCU_BUS_SLAVE_FLAGS_T ) * 8 >= MAX_SCU_SLAVES );


void showPeripheryPointer( void )
{
   mprintf( "CPU_INFO_ROM:    0x%x\n", pCpuId );
   mprintf( "CPU_ATOM_ACC:    0x%x\n", pCpuAtomic );
   mprintf( "CPU_SYSTEM_TIME: 0x%x\n", pCpuSysTime );
   mprintf( "CPU_MSI_CTRL_IF: 0x%x\n", pCpuIrqSlave );
}

static inline uint32_t getSlotOffset( unsigned int slot )
{
   LM32_ASSERT( slot > 0 );
   LM32_ASSERT( slot <= MAX_SCU_SLAVES );
   return slot << 16;
}

static inline unsigned short* getScuBusSlaveAddr( const unsigned short* pScuBusBase,
                                                  unsigned int slot )
{
   return (unsigned short*)&pScuBusBase[getSlotOffset(slot)];
}

static inline
unsigned short getScuBusSlaveValue16( const unsigned short* pSlaveAddr, unsigned int index )
{
   LM32_ASSERT( index >= 0 );
   LM32_ASSERT( index < getSlotOffset(1) );
   return pSlaveAddr[index];
}

static inline
void setScuBusSlaveValue16( unsigned short* pSlaveAddr, unsigned int index, uint16_t value )
{
   LM32_ASSERT( index >= 0 );
   LM32_ASSERT( index < getSlotOffset(1) );
   pSlaveAddr[index] = value;
}


static inline bool scuBusIsSlavePresent( SCU_BUS_SLAVE_FLAGS_T flags, int slotNo )
{
   LM32_ASSERT( slotNo > 0 );
   LM32_ASSERT( slotNo <= MAX_SCU_SLAVES );
   return ((flags & (1 << (slotNo-1))) != 0);
}

SCU_BUS_SLAVE_FLAGS_T scuBusFindSpecificSlaves( const unsigned short* pScuBusBase,
                                                const unsigned short systemAddr,
                                                const unsigned short grupAddr )
{
   SCU_BUS_SLAVE_FLAGS_T slaveFlags = 0;

   for( int slot = 1; slot <= MAX_SCU_SLAVES; slot++ )
   {
      const unsigned short* pSlaveAddr = getScuBusSlaveAddr( pScuBusBase, slot );
      if( getScuBusSlaveValue16( pSlaveAddr, CID_SYS )   == systemAddr &&
          getScuBusSlaveValue16( pSlaveAddr, CID_GROUP ) == grupAddr )
         slaveFlags |= (1 << (slot-1));
   }

   return slaveFlags;
}


SCU_BUS_SLAVE_FLAGS_T scuFindAllSlaves( const unsigned short* pScuBusBase )
{
   SCU_BUS_SLAVE_FLAGS_T slaveFlags = 0;

   for( int slot = 1; slot <= MAX_SCU_SLAVES; slot++ )
   {
      const unsigned short* pSlaveAddr = getScuBusSlaveAddr( pScuBusBase, slot );
      if( getScuBusSlaveValue16( pSlaveAddr, CID_SYS )   != SCUBUS_INVALID_VALUE ||
          getScuBusSlaveValue16( pSlaveAddr, CID_GROUP ) != SCUBUS_INVALID_VALUE )
         slaveFlags |= (1 << (slot-1));
   }

   return slaveFlags;
}

#define ESC_RED    "\e[31m"
#define ESC_NORMAL "\e[0m"

void main( void )
{
   SCU_BUS_SLAVE_FLAGS_T slavePersentFlags;

   discoverPeriphery();
   uart_init_hw();
   mprintf("\nTest...\n");

   //showPeripheryPointer();
   unsigned short* pScuBusBase = (unsigned short*)find_device_adr(GSI, SCU_BUS_MASTER);
   LM32_ASSERT( pScuBusBase != (void*)ERROR_NOT_FOUND );
   mprintf( "SCU base address: 0x%x\n", pScuBusBase );

   slavePersentFlags = scuFindAllSlaves( pScuBusBase );
   if( slavePersentFlags != 0 )
   {
      for( int i = 1; i <= MAX_SCU_SLAVES; i++ )
         mprintf( "Slot %02d: %s\n", i,
                  scuBusIsSlavePresent( slavePersentFlags, i )? ESC_RED"used"ESC_NORMAL : "free" );
   }
   else
      mprintf( "No slaves found!\n" );
}

/*================================== EOF ====================================*/
