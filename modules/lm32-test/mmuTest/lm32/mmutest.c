// TODO!
#include <mprintf.h>
#include <eb_console_helper.h>
#include <scu_mmu.h>

typedef DDR3_T         MMU_OBJ_T;

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





/////////////////////////
#ifdef __lm32__
MMU_STATUS_T mmuInit( void )
{
   return (ddr3init( &mg_mmuObj ) == 0)? OK : MEM_NOT_PRESENT;
}

void mmuRead( const MMU_ADDR_T index, RAM_PAYLOAD_T* pItem, size_t len )
{
   for( size_t i = 0; i < len; i++ )
      ddr3read64( &mg_mmuObj, &pItem[i], index );
}

void mmuWrite( const MMU_ADDR_T index, const RAM_PAYLOAD_T* pItem, size_t len )
{
   for( size_t i = 0; i < len; i++ )
      ddr3write64( &mg_mmuObj, index, &pItem[i] );
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
