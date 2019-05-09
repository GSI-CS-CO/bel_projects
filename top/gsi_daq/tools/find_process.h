/*****************************************************************************/
/*                                                                           */
/*! @brief Module for finding process-IDs of running LINUX processes         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*! @file     find_process.h lib: libFindProcess.so resp. -lFindProcess      */
/*! @see      find_process.c                                                 */
/*! @author   Ulrich Becker                                                  */
/*! @date     16.12.2015                                                     */
/*  Revision:                                                                */
/*****************************************************************************/
/*!
 * @example test.c
 * @example mypidof.c
 * @example mywich.c
 */

#ifndef FIND_PROCESS_H_
#define FIND_PROCESS_H_

#include <sys/types.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Helper-macro for OFP_LAMBDA.
 * Makes anonymous functions for C possible if the compiler
 * supported nested functions.
 * @see OPT_LAMBDA
 */
#if !defined(_LAMBDA_) || defined(__DOXYGEN__)
   #define _LAMBDA_(L_) ({ L_ _;})
#endif

/*!
 * @brief Data type of the process search mode.
 */
typedef enum
{
   /*!
    * @brief Resolves symbolic links.
    */
   FPROC_RLINK    = (1 << 0),

   /*!
    * @brief Discards the path-name.
    */
   FPROC_BASENAME = (1 << 1)
} FPROC_MODE_T;


/*!
 * @brief Data type of the command-line of the found processes.
 */
typedef struct
{
   /*!
    * @brief Start-pointer of the data-buffer of the entire
    *        command-line.
    */
   uint8_t* buffer;

   /*!
    * @brief Number of valid bytes in the data-buffer.
    */
   size_t   len;

   /*!
    * @brief Number of arguments inclusive the process-name.
    */
   size_t   argc;
} CMD_LINE_T;

/*!
 * @brief Argument-type for the callback-function
 *        ON_FOUND_PROCESS_F
 * @note The pointers of this structure are only valid within the
 *       callback-function! \n
 *       If you'll export one or more of them for using outside, so
 *       you have to make deep-copy of them within the callback-function.
 * @see ON_FOUND_PROCESS_F
 */
typedef struct
{
  /*!
   * @brief Pointer to the process-name to search.
   */
  const char*  name;

  /*!
   * @brief Pointer to process-directory of the currently found
   *        process.
   */
  const char*  procDir;

  /*!
   * @brief Command line data of the currently found
   *        process.
   */
  CMD_LINE_T   commandLine;

  /*!
   * @brief Current process-ID of the found process.
   */
  pid_t        pid;

  /*!
   * @brief Counter of the found processes.
   */
  int          count;

  /*!
   * @brief Tunnel for private user-data.
   *        Third argument (pUser) of the function findProcesses()
   * @see findProcesses()
   */
  void*        pUser;

  /*!
   * @brief Adjusted process search-mode.
   *        Fourth argument (mode) of function
   */
  FPROC_MODE_T mode;
} OFP_ARG_T;

/*!
 * @brief Signature of the callback-function for function findProcesses()
 *
 *        Second argument of function findProcesses()
 * @param pArg Pointer to the found process-data.
 * @see OFP_ARG_T
 * @retval ==0 Process searching will continue.
 * @retval <0  Process searching will immediately terminated,
 *             return value of findProcesses() will be this value.
 * @retval >0  Process searching will immediately terminated,
 *             return value of findProcesses() will be the number
 *             of found processes.
 */
typedef int (*ON_FOUND_PROCESS_F)( OFP_ARG_T* pArg );

/*!
 * @brief Macro for implementing the callback-function as "lambda-function".
 *
 * This macro makes it possible to implement the callback-function
 * directly in the argument of the function findProcesses() instead
 * the using of a function-pointer. \n
 * That is only meaningful when the callback-function is very small.
 *
 * @note Keep in mind that lambda-functions are a matter of nested functions.
 *       This macro can only be used if your compiler supported them.
 *
 * @param argName Name of the argument for using within the function-body. 
 * @param body Body of callback-function.
 * 
 * @see mypidof.c
 */
#define OFP_LAMBDA( argName, body ) \
   _LAMBDA_( int _( OFP_ARG_T* argName ) body )

/*!
 * @brief Argument-object-type for the callback-function ON_PID_F
 *        of function forAllProcesses
 * @see forAllProcesses
 * @see ON_PID_F
 */
typedef struct
{
   /*! @brief Current found process-id. */
   pid_t pid;

   /*! @brief Tunnel for private user-data.
    *
    *  Second argument (pUser) of the function forAllProcesses()
    */
   void* pUser;
} ON_PID_ARG_T;

/*!
 * @brief Signature of the callback-function for function forAllProcesses()
 *
 * First argument of the function forAllProcesses()
 *
 * @param pArg Pointer to the argument-object of type ON_PID_ARG_T.
 * @retval <0: Process-search-loop of forAllProcesses() becomes
 *             terminated immediately, the return-value of
 *             forAllProcesses() will be this value.
 * @retval ==0 Process-search-loop will continued.
 * @retval >0  Process-search-loop of forAllProcesses() becomes
 *             terminated immediately, the return-value
 *             of forAllProcesses() will be the number of
 *             found processes so far.
 */
typedef int (*ON_PID_F)( ON_PID_ARG_T* pArg );

/*!
 * @brief Macro for implementing the callback-function as "lambda-function".
 *
 * This macro makes it possible to implement the callback-function
 * directly in the argument of the function forAllProcesses() instead
 * the using of a function-pointer. \n
 * That is only meaningful when the callback-function is very small.
 *
 * @note Keep in mind that lambda-functions are a matter of nested functions.
 *       This macro can only be used if your compiler supported them.
 *
 * @param argName Name of the argument for using within the function-body. 
 * @param body Body of callback-function.
 */
#define ON_PID_LAMBDA( argName, body ) \
   _LAMBDA_( int _( ON_PID_ARG_T* argName ) body )

/*!
 * @brief The function calls for each currently running process 
 *        its callback function onPid() as long as it returns 0.
 * @param onPid Pointer to the callback-function.
 * @param pUser Optional pointer to user-data which will be forwarded to
 *               the callback function.
 * @retval <0 Error
 * @retval >0 Number of found processes.
 */
int forAllProcesses( ON_PID_F onPid, void* pUser );


/*!
 * @brief  Function search the whole process directory for running processes
 *         witch match with the given name of the argument pProcessName.
 *
 * For each found process a optional callback function (if not NULL)
 * "onFound" will invoked with the PID in the first argument of the
 * current found process. \n
 * When onFound() returns with 0: findProcesses will continue for eventually
 * further processes else the function will terminated.
 *
 * @param  pProcessName String of the process.name to search.
 * @param  onFound Pointer to the callback function if not used it shall be NULL.
 * @param  pUser Optional pointer to user-data which will be forwarded to
 *               the second parameter of the callback function.
 * @param mode   Search-mode @see FPROC_MODE_T
 * @retval <0 Error occurred.
 * @retval >=0 Number of found processes.
 *
 * Following example lists the process-id's of all running processes of "bash":\n
 * E.g.:
 * @code
 * int callBack( OFP_ARG_T* pArg )
 * {
 *    printf( "found PID: %d\n", pArg->pid )
 *    return 0;
 * }
 *
 * int main( void )
 * {
 *    int numberOfRunningProcesses;
 * 
 *    printf( "PID's of of the current running process(es) \"bash\"\n" );
 *    numberOfRunningProcesses = findProcesses( "bash",
 *                                              callBack,
 *                                              NULL,
 *                                              FPROC_RLINK | FPROC_BASENAME );
 *    if( numberOfRunningProcesses < 0 )
 *    {
 *       fprintf( stderr, "Error %d by findProcesses!\n", errno );
 *       return 1;
 *    }
 *    printf( "%d running processes found.\n", numberOfRunningProcesses );
 *    return 0;
 * }
 * @endcode
 */
int findProcesses( const char* pProcessName,
                   ON_FOUND_PROCESS_F onFound,
                   void* pUser,
                   FPROC_MODE_T mode
                 );

/*!
 * @brief  Function returns the process-ID of the given program name if
 *         the same program is already running.
 *
 * This process ID is the ID of
 * first found process in the process file system.
 * If the given process name is the name of the own process and no further
 * instances of this process was started, will 0 returned. \n
 * This function is suitable to check whether a instance of its self is
 * already running. \n
 * e.g.:
 * @code
 * int main( char** ppArgv, int argc )
 * {
 *    pid_t pid = checkProcess( ppArgv[0], FPROC_RLINK | FPROC_BASENAME );
 *    if( pid < 0 )
 *    {
 *       fprintf( stderr, "Error %d by checking process-instances!\n", errno );
 *       return -1;
 *    }
 *    if( pid > 0 )
 *    {
 *       printf( "A instance of this program was already started by the PID: %d\n", pid );
 *       return 1;
 *    }
 * 
 *    printf( "This program is a single instance!\n" );
 * 
 * //  ...
 * 
 *    return 0;
 * }
 * @endcode
 * @param  String of the program-name.
 * @param mode   Search-mode @see FPROC_MODE_T
 * @retval Process-ID (PID) of the first found process-instance. Zero if the
 *         program name the own process and no further instances was started.
 *         -1 when a error occurred.
 */
extern pid_t checkProcess( const char* pProcessName, FPROC_MODE_T mode );

/*!
 * @brief  Function checks whether a clone of its own program is already running.
 *
 * If this the case, so the function try to terminate its clone by
 * sending the signal SIGTERM.
 * @param  ppArgv Usually the second argument of the function "main".
 * @param mode Search-mode  @see FPROC_MODE_T
 * @retval == 0: No clone was running.
 *         >  0  PID of the terminated clone.
 *         <  0  Error.
 */
extern pid_t terminateClone( char* const* ppArgv, FPROC_MODE_T mode );

/*!
 * @brief Function appends the full directory-path to the executable
 *        process-name and copy it in the argument "buffer"...
 *
 *  ... if the process-file was found in one of the directories
 *  named in the environment variable "PATH". \n
 *  Similar to the shell-command "which". \n
 *
 *  If the process-file was not found, then only the process-name
 *  will copied in "buffer".
 *
 * @param buffer Target-buffer.
 * @param size   Capacity in bytes of the target-buffer.
 * @param name   Name of the executable process-file.
 * @retval ==-1: System-error the system-variable errno contains
 *               the exact error-number.
 * @retval ==-2: Capacity of buffer is to small.
 * @retval ==0:  Success.
 * @retval ==1:  Process-file was not found in the named directories
 *               of the environment-variable "PATH".
 *               The argument "buffer" contains the copy of argument
 *               "name" only.
 * @see mywich.c
 */
int addPath( char* buffer, size_t size, const char* name );

#ifdef __cplusplus
}
#endif

#endif /* FIND_PROCESS_H_ */
/*================================== EOF ====================================*/
