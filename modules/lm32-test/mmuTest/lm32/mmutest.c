// TODO!
#include <mprintf.h>
#include <eb_console_helper.h>
#include <helper_macros.h>
#include <scu_ddr3.h>
#include <stdint.h>


typedef uint64_t       MMU_TAG_T;
typedef unsigned int   MMU_ADDR_T;
typedef DDR3_T         MMU_OBJ_T;
typedef DDR3_PAYLOAD_T RAM_PAYLOAD_T;



/*!
 * @brief Return values of the memory management unit. 
 */
typedef enum
{
   /*!
    * @brief Action was successful.
    */
   OK              =  0,

   /*!
    * @brief Wishbone device of RAM not found. 
    */
   MEM_NOT_PRESENT = -1,

   /*!
    * @brief Memory block not found.
    */
   TAG_NOT_FOUND   = -2,

   /*!
    * @brief Requested memory block already present,
    *        but length doesn't match. 
    */
   NO_MATCH        = -3,

   /*!
    * @brief Requested memory block doesn't fit in physical memory.
    */
   OUT_OF_MEM      = -4
} MMU_STATUS_T;


const uint32_t MMU_MAGIC = 0xAAFF0055;
const MMU_ADDR_T MMU_LIST_START = 0;
const MMU_ADDR_T MMU_MAX_INDEX = DDR3_MAX_INDEX64;

/*!
 * @brief Type of list item of memory allocation list
 */
typedef struct PACKED_SIZE
{
   /*!
    * @brief Tag respectively identification (ID) of memory block.
    */
   uint16_t  tag;

   /*!
    * @brief Access flags of memory block. (rfu).
    */
   uint16_t  flags;

   /*!
    * @brief Index of next item.
    * @note In the case of the last item then it has to be zero.
    */
   uint32_t  iNext;

   /*!
    * @brief Start index of memory block.
    */
   uint32_t  iStart;

   /*!
    * @brief Data size in RAM_PAYLOAD_T units of memory block.
    */
   uint32_t  length;
} MMU_ITEM_T;

STATIC_ASSERT( sizeof( MMU_ITEM_T ) == 2 * sizeof( RAM_PAYLOAD_T ) );

typedef union
{
   MMU_ITEM_T     mmu;
   RAM_PAYLOAD_T  item[sizeof(MMU_ITEM_T)/sizeof(RAM_PAYLOAD_T)];
} MMU_ACCESS_T;

STATIC_ASSERT( sizeof( MMU_ACCESS_T ) == sizeof( MMU_ITEM_T ) );

#ifdef __lm32__
 MMU_OBJ_T mg_mmuObj;

 MMU_STATUS_T mmuInit( void );
 void mmuSetObject( MMU_OBJ_T obj );

#else
 typedef void (*MMU_READ_F)( const MMU_ADDR_T index, RAM_PAYLOAD_T* pItem );
 typedef void (*MMU_WRITE_F)( const MMU_ADDR_T index, const RAM_PAYLOAD_T* pItem );

 MMU_READ_F mmuRead;
 MMU_WRITE_F mmuWrite;

 void mmuSetWriteReadFunction( MMU_WRITE_F _mmuWrite, MMU_READ_F _mmuRead );
#endif

bool mmuIsPresent( void );
unsigned int mmuGetNumberOfBlocks( void );
MMU_STATUS_T mmuAlloc( MMU_TAG_T tag, MMU_ADDR_T* pStartAddr, size_t len );
MMU_STATUS_T mmuGet(  MMU_TAG_T tag, MMU_ADDR_T* pStartAddr, size_t* pLen );




/////////////////////////
#ifdef __lm32__
MMU_STATUS_T mmuInit( void )
{
   return (ddr3init( &mg_mmuObj ) == 0)? OK : MEM_NOT_PRESENT;
}

void mmuRead( const MMU_ADDR_T index, RAM_PAYLOAD_T* pItem )
{
   ddr3read64( &mg_mmuObj, pItem, index );
}

void mmuWrite( const MMU_ADDR_T index, const RAM_PAYLOAD_T* pItem )
{
   ddr3write64( &mg_mmuObj, index, pItem );
}
#else
void mmuSetWriteReadFunction( MMU_WRITE_F _mmuWrite, MMU_READ_F _mmuRead )
{
   mmuWrite = _mmuWrite;
   mmuRead  = _mmuRead;
}
#endif






void main( void )
{
   mprintf( ESC_XY( "1", "1" ) ESC_CLR_SCR "MMU-Test\n" );
   if( mmuInit() != OK )
   {
      mprintf( ESC_ERROR "ERROR Unable to get DDR3- RAM!\n" ESC_NORMAL );
      return;
   }
   
   
   
}

/*================================== EOF ====================================*/
