/*!
 * @file event_measurement.c
 * @brief Module for time measure of events on LM32
 * @author Ulrich Becker <u.becker@gsi.de>
 * @copyright   2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @date 18.01.2021
 */
#include <scu_wr_time.h>
#include <event_measurement.h>

/*!----------------------------------------------------------------------------
 */
void timeMeasure( MEASUREMENT_T* pMeasureTime )
{
   volatile const uint64_t currentTime = getWrSysTime();
   pMeasureTime->firstTime = pMeasureTime->secondTime;
   pMeasureTime->secondTime = currentTime;
}

/*!---------------------------------------------------------------------------
 */
uint64_t timeMeasureGetLast( const MEASUREMENT_T* pMeasureTime )
{
   return pMeasureTime->secondTime - pMeasureTime->firstTime;
}

/*!---------------------------------------------------------------------------
 */
bool timeMeasureIsValid( const MEASUREMENT_T* pMeasureTime )
{
   return pMeasureTime->firstTime != 0L;
}

/*================================== EOF ====================================*/
