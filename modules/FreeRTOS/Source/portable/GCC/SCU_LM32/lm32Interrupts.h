#ifndef INTERRUPTS_HEADER_FILE
#define INTERRUPTS_HEADER_FILE

/******************************************************************************
 * Manifest constants                                                         *
 ******************************************************************************
 * If not using higher-levels, reduces the data-allocation (8 bytes/level)    *
 ******************************************************************************/
#define MAX_MICO32_ISR_LEVEL (31)                 /* max. isr level 0 thru 31 */

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Data Structures                                                            *
 ******************************************************************************/
/* isr-callback typedef */
typedef void(*ISRCallback)( const unsigned int, const void* );


/*!
 * @brief Registers and de-registers interrupt-handler routine.
 *
 * To register, pass a valid function pointer to the Callback parameter.
 * To deregister, pass 0 as the callback parameter.
 *
 * Arguments:
 *  unsigned int intNum: interrupt line number that your component is
 *      connected to (0 to 31).
 *  void *Context: pointer provided by user that will be passed to the
 *      interrupt-handler callback.
 *  ISRCallback Callback:   User-provided interrupt-handler routine.
 *
 * Return values:
 *      MICO_STATUS_OK if successful.
 */ 
unsigned int registerISR( const unsigned int intNum, void* pContext, ISRCallback Callback );

/*!
 * @brief Disables a specific interrupt
 * 
 * Arguments:
 *  unsigned int Intlevel: interrupt 0 through 31 that needs to
 *      be disabled.
 *
 * Return values:
 *      MICO_STATUS_OK if successful.
 */
unsigned int disableInterrupt( const unsigned int intNum );


/*!
 * @brief Enables a specific interrupt
 * 
 * Arguments:
 *  unsigned int Intlevel: interrupt 0 through 31 that needs to
 *      be enabled.
 *
 * Return values:
 *      MICO_STATUS_OK if successful.
 */ 
unsigned int enableInterrupt( const unsigned int intNum );


#ifdef __cplusplus
}
#endif

#endif

