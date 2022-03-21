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

MMU_OBJ_T mg_mmuObj;

typedef enum
{
   OK          =  0,
   NOT_PRESENT = -1,
   NOT_FOUND   = -2,
   NO_MATCH    = -3,
   OUT_OF_MEM  = -4
} MMU_STATUS_T;


MMU_STATUS_T mmuInit( void );
MMU_STATUS_T mmuAlloc( MMU_TAG_T tag, MMU_ADDR_T startAddr, size_t len );
MMU_STATUS_T mmuGet(  MMU_TAG_T tag, MMU_ADDR_T* pStartAddr, size_t* pLen );


MMU_STATUS_T mmuInit( void )
{
   return (ddr3init( &mg_mmuObj ) == 0)? OK : NOT_PRESENT;
}


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
