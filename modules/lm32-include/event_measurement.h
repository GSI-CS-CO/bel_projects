/*!
 * @file event_measurement.h
 * @brief Module for time measure of events on LM32
 * @author Ulrich Becker <u.becker@gsi.de>
 * @copyright   2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @date 18.01.2021
 */
#ifndef _EVENT_MEASUREMENT_H
#define _EVENT_MEASUREMENT_H
#ifndef __lm32__
  #error This module is for LM32 only:
#endif
#include <stdint.h>
#include <stdbool.h>

/*!
 * @defgroup MEASUREMENT_UTILS
 * @brief Utilities to measurement duration of functions and interrupts based
 *        on white rabbit timer over wishbone bus.
 */


#ifdef __cplusplus
extern "C" {
namespace Scu {
#endif

/*!----------------------------------------------------------------------------
 * @ingroup MEASUREMENT_UTILS
 * @brief General object for time measurements.
 */
typedef struct
{
   uint64_t firstTime;
   uint64_t secondTime;
} TIME_MEASUREMENT_T;

/*!----------------------------------------------------------------------------
 * @ingroup MEASUREMENT_UTILS
 * @brief Static initializer for object of type TIME_MEASUREMENT_T
 */
#define TIME_MEASUREMENT_INITIALIZER { 0L, 0L }

/*!----------------------------------------------------------------------------
 * @ingroup MEASUREMENT_UTILS
 * @brief Hold the current white rabbit time.
 * @note This could be used within a interrupt function.
 * @param pMeasureTime Pointer to the time measurement object.
 */
void timeMeasure( TIME_MEASUREMENT_T* pMeasureTime );

/*!----------------------------------------------------------------------------
 * @ingroup MEASUREMENT_UTILS
 * @brief Returns the time in nanoseconds between the last two calls of
 *        the function timeMeasure.
 * @param pMeasureTime Pointer to the time measurement object.
 * @return Time in nanoseconds.
 */
uint64_t timeMeasureGetLast( const TIME_MEASUREMENT_T* pMeasureTime );

/*!----------------------------------------------------------------------------
 * @ingroup MEASUREMENT_UTILS
 * @brief Returns the time in nanoseconds between the last two calls of
 *        the function timeMeasure.
 * @note This query will made within a atomic section, where the interrupts
 *       are disabled.
 * @param pMeasureTime Pointer to the time measurement object.
 * @return Time in nanoseconds.
 */
uint64_t timeMeasureGetLastSafe( const TIME_MEASUREMENT_T* pMeasureTime );

/*!----------------------------------------------------------------------------
 * @ingroup MEASUREMENT_UTILS
 * @brief Returns the time in nanoseconds between the last two calls of
 *        the function timeMeasure for measurements where the time will be
 *        smaller than 4,294967296 seconds.
 * @note CAUTION: Be sure, that the intended measure time doesn't exceed the
 *                maximum time of 4,294967296 seconds! Otherwise an
 *                integer overflow will happen!
 * @param pMeasureTime Pointer to the time measurement object.
 * @return Time in nanoseconds.
 */
uint32_t timeMeasureGetLast32( const TIME_MEASUREMENT_T* pMeasureTime );

/*!----------------------------------------------------------------------------
 * @ingroup MEASUREMENT_UTILS
 * @brief Returns the time in nanoseconds between the last two calls of
 *        the function timeMeasure for measurements where the time will be
 *        smaller than 4,294967296 seconds.
 * @note This query will made within a atomic section, where the interrupts
 *       are disabled.
 * @note CAUTION: Be sure, that the intended measure time doesn't exceed the
 *                maximum time of 4,294967296 seconds! Otherwise an
 *                integer overflow will happen!
 */
uint32_t timeMeasureGetLast32Safe( const TIME_MEASUREMENT_T* pMeasureTime );

/*!----------------------------------------------------------------------------
 * @ingroup MEASUREMENT_UTILS
 * @brief Checks whether the time measurement object is valid.
 * @param pMeasureTime Pointer to the time measurement object.
 * @retval true Time measurement is valid that means more then two calls of
 *              function timeMeasure() was been made.
 */
bool timeMeasureIsValid( const TIME_MEASUREMENT_T* pMeasureTime );

/*!----------------------------------------------------------------------------
 * @ingroup MEASUREMENT_UTILS
 * @brief Checks whether the time measurement object is valid.
 * @note This query will made within a atomic section, where the interrupts
 *       are disabled.
 * @param pMeasureTime Pointer to the time measurement object.
 * @retval true Time measurement is valid that means more then two calls of
 *              function timeMeasure() was been made.
 */
bool timeMeasureIsValidSafe( const TIME_MEASUREMENT_T* pMeasureTime );

/*!----------------------------------------------------------------------------
 * @ingroup MEASUREMENT_UTILS
 * @brief Resets the time measurement object
 * @param pMeasureTime Pointer to the time measurement object.
 */
void timeMeasureReset( TIME_MEASUREMENT_T* pMeasureTime );

/*!----------------------------------------------------------------------------
 * @ingroup MEASUREMENT_UTILS
 * @brief Resets the time measurement object
 * @note This will performed within a atomic section, where the interrupts
 *       are disabled.
 * @param pMeasureTime Pointer to the time measurement object.
 */
void timeMeasureResetSafe( TIME_MEASUREMENT_T* pMeasureTime );

/*!---------------------------------------------------------------------------
 * @ingroup MEASUREMENT_UTILS
 * @brief Helper function printing the time in seconds by emulating as
 *        floating point value.
 * @note CAUTION: Be sure, that the intended measure time doesn't exceed the
 *                maximum time of 4,294967296 seconds! Otherwise an
 *                integer overflow will happen!
 * @param pMeasureTime Pointer to the time measurement object.
 * @return Number of printed characters.
 */
int timeMeasurePrintSeconds( const TIME_MEASUREMENT_T* pMeasureTime );


/*!---------------------------------------------------------------------------
 * @ingroup MEASUREMENT_UTILS
 * @brief Helper function printing the time in seconds by emulating as
 *        floating point value.
 * @note CAUTION: Be sure, that the intended measure time doesn't exceed the
 *                maximum time of 4,294967296 seconds! Otherwise an
 *                integer overflow will happen!
 * @param pMeasureTime Pointer to the time measurement object.
 * @return Number of printed characters.
 */
int timeMeasurePrintSecondsSafe( const TIME_MEASUREMENT_T* pMeasureTime );

/*!---------------------------------------------------------------------------
 * @ingroup MEASUREMENT_UTILS
 * @brief Helper function printing the time in milliseconds by emulating as
 *        floating point value.
 * @note CAUTION: Be sure, that the intended measure time doesn't exceed the
 *                maximum time of 4,294967296 seconds! Otherwise an
 *                integer overflow will happen!
 * @param pMeasureTime Pointer to the time measurement object.
 * @return Number of printed characters.
 */
int timeMeasurePrintMilliseconds( const TIME_MEASUREMENT_T* pMeasureTime );


/*!---------------------------------------------------------------------------
 * @ingroup MEASUREMENT_UTILS
 * @brief Helper function printing the time in milliseconds by emulating as
 *        floating point value.
 * @note CAUTION: Be sure, that the intended measure time doesn't exceed the
 *                maximum time of 4,294967296 seconds! Otherwise an
 *                integer overflow will happen!
 * @param pMeasureTime Pointer to the time measurement object.
 * @return Number of printed characters.
 */
int timeMeasurePrintMillisecondsSafe( const TIME_MEASUREMENT_T* pMeasureTime );


#ifdef __cplusplus
} /* namespace Scu */
} /* extern "C" */
#endif
#endif /* ifndef _EVENT_MEASUREMENT_H */
/* ================================= EOF ====================================*/
