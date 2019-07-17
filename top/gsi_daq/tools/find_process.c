/*!
 * @see
 * <a href="https://github.com/UlrichBecker/get_linux_process_id">
 * GitHub Repository</a>
 */
/*****************************************************************************/
/*                                                                           */
/*! @brief Module for finding process-IDs of running LINUX processes         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*! @file     find_process.c                                                 */
/*! @see      find_process.h                                                 */
/*! @author   Ulrich Becker                                                  */
/*! @date     16.12.2015                                                     */
/*  Revision:                                                                */
/*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <assert.h>
#include <signal.h>
#include <limits.h>
#include <libgen.h>
#include <assert.h>

#include <find_process.h>

#ifndef FIND_PROCESS_PATH_LEN
   #define FIND_PROCESS_PATH_LEN (PATH_MAX + 1)
#endif
#ifndef CMD_LINE_BUFFER_LEN
   #define CMD_LINE_BUFFER_LEN (PATH_MAX + 1)
#endif

#ifndef IS_IN_RANGE
  #define IS_IN_RANGE( x, min, max ) ( ((x) >= (min)) && ((x) <= (max)) )
#endif
#define PROC_DIR_NAME "/proc"

/*-----------------------------------------------------------------------------
*/
static const char* climeToBasename( const char* name )
{
   const char* ret = name;

   while( *name != '\0' )
   {
      if( *(name++) == '/' )
         ret = name;
   }

   return ret;
}

/*-----------------------------------------------------------------------------
*/
static int hafeNoPath( const char* name )
{
   while( *name != '\0' )
   {
      if( *name == '/' )
         return 1;
      name++;
   }
   return 0;
}

/*!----------------------------------------------------------------------------
 * @see find_process.h
*/
int addPath( char* buffer, size_t size, const char* name )
{
   char* path;
   char* pos;
   size_t len;
   size_t namelen;

   assert( !IS_IN_RANGE( name, buffer, buffer + size ));

   path = getenv( "PATH" );
   if( path == NULL )
      return -1;

   namelen = strlen( name );
   assert( !IS_IN_RANGE( name + namelen + 1, buffer, buffer + size ));

   pos = path;
   while( 1 )
   {
      if( *path == ':' || *path == '\0' )
      {
         len = path - pos;
         if( (len + namelen + 2) > size )
            return -2;
         strncpy( buffer, pos, len );
         buffer[len++] = '/';
         strcpy( buffer + len, name );
         if( access( buffer, X_OK ) == 0 )
            return 0;
         if( *path == '\0' )
         {
            strcpy( buffer, name );
            break;
         }
         pos = path + 1;
      }
      path++;
   };
   return 1;
}

/*!----------------------------------------------------------------------------
 * @brief Argument structure for internal using only.
*/
typedef struct
{
   OFP_ARG_T          cbfArg;
   const char*        originProcessNameBase;
   ON_FOUND_PROCESS_F onFound;
} OFP_INTERN_ARG_T;

/*!----------------------------------------------------------------------------
 */
static inline void initCmdLineObj( CMD_LINE_T* pCmdLine, const size_t maxSize )
{
   pCmdLine->argc = 0;
   while( pCmdLine->len < (maxSize-1) )
   {
      if( pCmdLine->buffer[pCmdLine->len++] == 0 )
      {
         pCmdLine->argc++;
         if( pCmdLine->buffer[pCmdLine->len] == 0 )
            break;
      }
   }
}

/*!----------------------------------------------------------------------------
 * @brief internal callback function used for function findProcesses.
 *        Invoked within function "forAllProcesses"
 * @see findProcesses
 */
static int _onPid( ON_PID_ARG_T* pArg )
{
   int               ret;
   char              commandLineName[256];
   char              cmdLineBuffer[CMD_LINE_BUFFER_LEN];
   char              execName[FIND_PROCESS_PATH_LEN];
   char              qualifiedName[FIND_PROCESS_PATH_LEN];
   FILE*             pFile;
   char*             pFgetRet;
   const char*       token;
   OFP_INTERN_ARG_T* pInternArg = (OFP_INTERN_ARG_T*) pArg->pUser;

   snprintf( commandLineName,
             sizeof( commandLineName ),
             PROC_DIR_NAME "/%ld/cmdline",
             (long int)pArg->pid );

   pFile = fopen( commandLineName, "r" );
   if( pFile == NULL )
      return 0;

   memset( cmdLineBuffer, 0, sizeof( cmdLineBuffer ) );
   pFgetRet = fgets( cmdLineBuffer, sizeof( cmdLineBuffer ), pFile );
   fclose( pFile );
   if( pFgetRet == NULL )
      return 0;

   token = strtok( cmdLineBuffer, " " );
   if( token == NULL )
      return 0;

   if( (pInternArg->cbfArg.mode & FPROC_RLINK) != 0 )
   {
      if( hafeNoPath( token ) == 0 )
      {
         strncpy( execName, token, sizeof( execName ));
         ret = addPath( qualifiedName, sizeof( qualifiedName ), execName );
         if( ret < 0 )
         {
            pInternArg->cbfArg.count = ret;
            return 1;
         }
         if( ret == 0 )
         {
            if( realpath( qualifiedName, execName ) == NULL )
            {
               pInternArg->cbfArg.count = -1;
               return 1;
            }
            token = execName;
         }
         else
            token = qualifiedName;
      }
      else
      {
         if( realpath( token, execName ) == NULL )
         {
            pInternArg->cbfArg.count = -1;
            return 1;
         }
         token = execName;
      }
   } /* End if( (cbfArg.mode & FPROC_RLINK) != 0 ) */

   if( (pInternArg->cbfArg.mode & FPROC_BASENAME) != 0 )
      token = climeToBasename( token );

   if( strcmp( token, pInternArg->originProcessNameBase ) != 0 )
      return 0; /* That means: continue the proc-dir browsing. */

   /* Running process with the given name found. */
   pInternArg->cbfArg.count++;

   if( pInternArg->onFound == NULL ) /* No call-back function defined. */
      return 0; /* That means: continue the proc-dir browsing. */

   commandLineName[climeToBasename( commandLineName ) - commandLineName] = '\0';
   pInternArg->cbfArg.procDir = commandLineName;
   pInternArg->cbfArg.pid  = pArg->pid;
   pInternArg->cbfArg.commandLine.buffer = (uint8_t*)cmdLineBuffer;
   initCmdLineObj( &pInternArg->cbfArg.commandLine, sizeof(cmdLineBuffer) );
   return pInternArg->onFound( &pInternArg->cbfArg );
};

/*!----------------------------------------------------------------------------
 * @see find_process.h
 */
int forAllProcesses( ON_PID_F onPid, void* pUser )
{
   DIR*           pDir;
   struct dirent* poEntry;
   char*          pEnd;
   int            ret;
   int            count = 0;

   ON_PID_ARG_T onPidArg =
   {
      .pid   = 0,
      .pUser = pUser
   };

   pDir = opendir( PROC_DIR_NAME );
   if( pDir == NULL )
      return -1;

   while( (poEntry = readdir( pDir )) != NULL )
   {
      if( poEntry->d_type != DT_DIR )
         continue;

      onPidArg.pid = strtol( poEntry->d_name, &pEnd, 10 );
      /* If pEnd don't point to a null character,
       * the directory-item is not a process-dir so we ignore it. */
      if( *pEnd != '\0' )
         continue;

      count++;

      if( onPid == NULL )
         continue;
      ret = onPid( &onPidArg );
      if( ret < 0 )
      {
         count = ret;
         break;
      }
      if( ret > 0 )
         break;
   }

   closedir( pDir );
   return count;
}

/*!----------------------------------------------------------------------------
 * @see find_process.h
*/
int findProcesses( const char* pProcessName,
                   ON_FOUND_PROCESS_F onFound,
                   void* pUser,
                   FPROC_MODE_T mode
                 )
{
   char           originProcessName[FIND_PROCESS_PATH_LEN];
   char           qualifiedName[FIND_PROCESS_PATH_LEN];
   int ret;
   OFP_INTERN_ARG_T cbfInternArg =
   {
      .cbfArg =
      {
         .name    = pProcessName,
         .pid     = 0,
         .pUser   = pUser,
         .count   = 0,
         .mode    = mode
      },
      .onFound = onFound
   };

   if( (cbfInternArg.cbfArg.mode & FPROC_RLINK) != 0 )
   {
      if( hafeNoPath( cbfInternArg.cbfArg.name ) == 0 )
      {
         ret = addPath( qualifiedName, sizeof( qualifiedName ), cbfInternArg.cbfArg.name );
         if( ret < 0 )
            return ret;
         if( (ret == 0) && (realpath( qualifiedName, originProcessName ) == NULL) )
            return -1;
         if( ret > 0 )
            strcpy( originProcessName, cbfInternArg.cbfArg.name );
      }
      else if( realpath( cbfInternArg.cbfArg.name, originProcessName ) == NULL )
         return -1;

      if( (cbfInternArg.cbfArg.mode & FPROC_BASENAME) != 0 )
         cbfInternArg.originProcessNameBase = climeToBasename( originProcessName );
   }
   else if( (cbfInternArg.cbfArg.mode & FPROC_BASENAME) != 0 )
      cbfInternArg.originProcessNameBase = climeToBasename( cbfInternArg.cbfArg.name );
   else
      cbfInternArg.originProcessNameBase = cbfInternArg.cbfArg.name;

   ret = forAllProcesses( _onPid, &cbfInternArg );
   if( ret <= 0 )
      return ret;
   return cbfInternArg.cbfArg.count;
}

/*!----------------------------------------------------------------------------
@brief Implementation of the callback function for checkProcess().
*/
static int _onFound( OFP_ARG_T* pArg )
{
   if( pArg->pid == getpid() )
   {
      *((pid_t*)pArg->pUser) = 0;
      return 0; /* Continue of the process search loop */
   }

   *((pid_t*)pArg->pUser) = pArg->pid;
   return 1; /* Leave the process search loop */
}

/*!----------------------------------------------------------------------------
*/
pid_t checkProcess( const char* pProcessName, FPROC_MODE_T mode )
{
   pid_t foundPid = 0;
   if( findProcesses( pProcessName, _onFound, &foundPid, mode ) < 0 )
      return -1;
   return foundPid;
}

/*!----------------------------------------------------------------------------
*/
pid_t terminateClone( char* const* ppArgv, FPROC_MODE_T mode  )
{
   pid_t pidOfClone;

   pidOfClone = checkProcess( ppArgv[0], mode );
   if( pidOfClone <= 0 ) /* IF error or no further process running. */
      return pidOfClone;

   /* Terminating the concurrent process. */
   if( kill( pidOfClone, SIGTERM ) < 0 )
      return -1;

   return pidOfClone;
}

/*!================================= EOF ====================================*/
