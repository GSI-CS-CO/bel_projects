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

#ifdef __cplusplus
extern "C" {
namespace Scu {
#endif

typedef struct
{
   uint64_t firstTime;
   uint64_t secondTime;
} MEASUREMENT_T;

/*!----------------------------------------------------------------------------
 */
void timeMeasure( MEASUREMENT_T* pMeasureTime );

/*!----------------------------------------------------------------------------
 */
uint64_t timeMeasureGetLast( const MEASUREMENT_T* pMeasureTime );

/*!----------------------------------------------------------------------------
 */
bool timeMeasureIsValid( const MEASUREMENT_T* pMeasureTime );


#ifdef __cplusplus
} /* namespace Scu */
} /* extern "C" */
#endif
#endif /* ifndef _EVENT_MEASUREMENT_H */
/* ================================= EOF ====================================*/
