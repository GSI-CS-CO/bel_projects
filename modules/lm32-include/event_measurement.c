/*!
 * @file event_measurement.c
 * @brief Module for time measure of events on LM32
 * @author Ulrich Becker <u.becker@gsi.de>
 * @copyright   2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @date 18.01.2021
 */
#include <scu_wr_time.h>
#include <lm32Interrupts.h>
#include <mprintf.h>
#include <event_measurement.h>


#ifdef CONFIG_TIME_MEASUREMENT_OVERFLOW_CHECK
   /* CAUTION:
    * Assert-macros could be expensive in memory consuming and the
    * latency time can increase as well!
    * Especially in embedded systems with small resources.
    * Therefore use them for bug-fixing or developing purposes only!
    */
   #include <scu_assert.h>
   #define OVERFLOW_ASSERT SCU_ASSERT
#else
   #define OVERFLOW_ASSERT(__e) ((void)0)
#endif

/*!----------------------------------------------------------------------------
 */
void timeMeasure( TIME_MEASUREMENT_T* pMeasureTime )
{
   volatile const uint64_t currentTime = getWrSysTime();
   pMeasureTime->firstTime = pMeasureTime->secondTime;
   pMeasureTime->secondTime = currentTime;
}

/*!----------------------------------------------------------------------------
 */
void timeMeasureSave( TIME_MEASUREMENT_T* pMeasureTime )
{
   ATOMIC_SECTION() timeMeasure( pMeasureTime );
}


/*!---------------------------------------------------------------------------
 */
uint64_t timeMeasureGetLast( const TIME_MEASUREMENT_T* pMeasureTime )
{
   return pMeasureTime->secondTime - pMeasureTime->firstTime;
}

/*!---------------------------------------------------------------------------
 */
uint64_t timeMeasureGetLastSafe( const TIME_MEASUREMENT_T* pMeasureTime )
{
   uint64_t ret;
   ATOMIC_SECTION() ret = timeMeasureGetLast( pMeasureTime );
   return ret;
}

/*!---------------------------------------------------------------------------
 */
uint32_t timeMeasureGetLast32( const TIME_MEASUREMENT_T* pMeasureTime )
{
   uint64_t ret = timeMeasureGetLast( pMeasureTime );
   OVERFLOW_ASSERT( GET_UPPER_HALF( ret ) == 0L );
   return (uint32_t)GET_LOWER_HALF( ret );
}

/*!---------------------------------------------------------------------------
 */
uint32_t timeMeasureGetLast32Safe( const TIME_MEASUREMENT_T* pMeasureTime )
{
   uint64_t ret = timeMeasureGetLastSafe( pMeasureTime );
   OVERFLOW_ASSERT( GET_UPPER_HALF( ret ) == 0L );
   return (uint32_t)GET_LOWER_HALF( ret );
}

/*!---------------------------------------------------------------------------
 */
bool timeMeasureIsValid( const TIME_MEASUREMENT_T* pMeasureTime )
{
   return pMeasureTime->firstTime != 0L;
}

/*!---------------------------------------------------------------------------
 */
bool timeMeasureIsValidSafe( const TIME_MEASUREMENT_T* pMeasureTime )
{
   bool ret;
   ATOMIC_SECTION() ret = timeMeasureIsValid( pMeasureTime );
   return ret;
}

/*!---------------------------------------------------------------------------
 */
void timeMeasureReset( TIME_MEASUREMENT_T* pMeasureTime )
{
   pMeasureTime->firstTime  = 0L;
   pMeasureTime->secondTime = 0L;
}

/*!---------------------------------------------------------------------------
 */
void timeMeasureResetSafe( TIME_MEASUREMENT_T* pMeasureTime )
{
   ATOMIC_SECTION() timeMeasureReset( pMeasureTime );
}

/*!---------------------------------------------------------------------------
 */
int timeMeasurePrintSeconds( const TIME_MEASUREMENT_T* pMeasureTime )
{
   const uint32_t time = timeMeasureGetLast32( pMeasureTime );
   return mprintf( "%d.%08d", time / 1000000000, time % 1000000000 );
}

/*!---------------------------------------------------------------------------
 */
int timeMeasurePrintSecondsSafe( const TIME_MEASUREMENT_T* pMeasureTime )
{
   const uint32_t time = timeMeasureGetLast32Safe( pMeasureTime );
   return mprintf( "%d.%08d", time / 1000000000, time % 1000000000 );
}

/*!---------------------------------------------------------------------------
 */
int timeMeasurePrintMilliseconds( const TIME_MEASUREMENT_T* pMeasureTime )
{
   const uint32_t time = timeMeasureGetLast32( pMeasureTime );
   return mprintf( "%d.%05d", time / 1000000, time % 1000000 );
}

/*!---------------------------------------------------------------------------
 */
int timeMeasurePrintMillisecondsSafe( const TIME_MEASUREMENT_T* pMeasureTime )
{
   const uint32_t time = timeMeasureGetLast32Safe( pMeasureTime );
   return mprintf( "%d.%05d", time / 1000000, time % 1000000 );
}

/*================================== EOF ====================================*/
