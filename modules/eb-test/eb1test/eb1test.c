
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <etherbone.h>

char* g_pProgramName;

static void die(const char* msg, eb_status_t status)
{
   fprintf(stderr, "%s: %s: %s\n", g_pProgramName, msg, eb_status(status));
   exit(1);
}

int main( int argc, char** ppArgv )
{
   eb_socket_t socket;
   eb_status_t status;
   eb_device_t device;
   struct sdb_device sdb[25];

   g_pProgramName = ppArgv[0];

   printf( "arg: %s\n", ppArgv[1] );
   status = eb_socket_open( EB_ABI_CODE, 0, EB_DATAX | EB_ADDRX, &socket );
   if( status != EB_OK )
      die( "eb_soccntet_open", status );
   status = eb_device_open( socket, ppArgv[1], EB_DATAX | EB_ADDRX, 3, &device );
   if( status != EB_OK )
      die( ppArgv[1], status );

   fprintf(stdout, "end..\n" );
   return 0;
}

/*================================== EOF ====================================*/
