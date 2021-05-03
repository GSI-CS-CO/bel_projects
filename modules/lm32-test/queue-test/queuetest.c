
#include <mprintf.h>
#include <eb_console_helper.h>
#include <stdbool.h>
#include <sw_queue.h>

typedef struct
{
   int a;
   int b;
   int c;
} ITEM_T;

QUEUE_CREATE_STATIC( g_queue1, 5, ITEM_T );

void printLevel( SW_QUEUE_T* pQueue )
{
   mprintf( ESC_FG_CYAN "Current level:      %d items.\n", queueGetSize( pQueue )  );
   mprintf( "Remaining capacity: %d items.\n" ESC_NORMAL, queueGetRemainingCapacity( pQueue ) );
}

void fillQueue( SW_QUEUE_T* pQueue )
{
   ITEM_T item;
   int i = 0;
   do
   {
      printLevel( pQueue );
      i++;
      item.a = i;
      item.b = i * 10;
      item.c = i * 100;
      if( !queueIsFull( pQueue ) )
      {
         mprintf( "Push item %d\n", i );
      }
   }
   while( queuePush( pQueue, &item ) );
}

void emptyQueue( SW_QUEUE_T* pQueue )
{
   ITEM_T item;
   int i = 0;
   while( queuePop( pQueue, &item ) )
   {
      printLevel( pQueue );
      mprintf( "Item %d:\n", ++i );
      mprintf( "\titem.a = %d\n", item.a );
      mprintf( "\titem.b = %d\n", item.b );
      mprintf( "\titem.c = %d\n", item.c );
   }
}



void main( void )
{
   mprintf( ESC_XY( "1", "1" ) ESC_CLR_SCR "Queue test\n" );

   // queueCreate( &g_queue1, g_Buffer1, sizeof(ITEM_T), CAPACITY1 );
  // QEUE_CREATE( &g_queue1, g_Buffer1, ITEM_T );
   queueReset( &g_queue1 );
   mprintf( "Queue created\n" );   

   fillQueue( &g_queue1 );
   emptyQueue( &g_queue1 );
   
   ITEM_T item = { .a = 4711, .b = 42, .c = 1010 };
   
   if( queuePush( &g_queue1, &item ) )
      mprintf( "queuePush\n" );

   emptyQueue( &g_queue1 );
   
   fillQueue( &g_queue1 );
   emptyQueue( &g_queue1 );
   while( true );
}
