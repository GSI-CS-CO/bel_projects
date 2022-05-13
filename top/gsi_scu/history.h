/*!
 *  @file history.h
 *  @brief Administration of history buffer
 *
 *  @date ?
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *  Origin Stefan Rauch (maybe)
 ******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
#ifndef _HISTORY_H
#define _HISTORY_H

/*!
 * @defgroup HISTORY Administration of history buffer.
 */

#include <stdint.h>
#include <stdbool.h>
#include <helper_macros.h>

/*!
 * @ingroup HISTORY
 * @brief  Number of HistItem entries in history buffer
 */
#ifndef HISTSIZE
   #define HISTSIZE 256
#endif

/*!
 * @ingroup HISTORY
 * @brief
 */
#ifdef CONFIG_SMALL_HISTORY_VALUE
  typedef uint8_t HIST_VALUE_T;
#else
  typedef uint32_t HIST_VALUE_T;
#endif

/*!
 * @ingroup HISTORY
 * @brief Item type of history buffer
 */
typedef struct PACKED_SIZE
{
   /*!
    * @brief White Rabbit time stamp
    */
   uint64_t     timeStamp;

   /*!
    * @brief Pointer to a constant message string.
    */
   const char*  message;

   /*!
    * @brief Associated integer value (optional).
    */
   HIST_VALUE_T associatedData;
   /* add more fields here Š */
} HistItem;

/*
 * Define bits to enable/disable circular history debugging
 * for certain subsystems
 */
#define HISTORY_BOOT            0x00000001
#define HISTORY_XYZ_MODULE      0x00000002
/* add more subsystem bit defines here  */


/*!
 * @ingroup HISTORY
 * @brief Variable value used in history events with no associated data
 */
#define HIST_NOVAL ((HIST_VALUE_T)~0)


#if defined( CONFIG_USE_HISTORY ) || defined(__DOXYGEN__)

#ifdef __cplusplus
extern "C" {
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup HISTORY
 * @brief Initializing of the history-buffer.
 */
void hist_init( const uint32_t subsystemsEnabled );


void hist_enableSubsystem( const uint32_t bit );
void hist_disableSubsystem( const uint32_t bit);

/*! ---------------------------------------------------------------------------
 * @ingroup HISTORY
 * @brief Adds a history item to the history buffer with associated
 *        integer value.
 * @param msg Pointer to the constant message string.
 * @param data
 */
GSI_DEPRECATED
void hist_addx( const uint32_t subsystem, const char *msg, const HIST_VALUE_T data);

/*! ---------------------------------------------------------------------------
 * @ingroup HISTORY
 * @brief Adds a history item to the history buffer
 * @param msg Pointer to the constant message string.
 */
GSI_DEPRECATED
STATIC inline void hist_add( const uint32_t subsystem, const char *msg )
{
   hist_addx( subsystem, msg, HIST_NOVAL );
}

/*! ---------------------------------------------------------------------------
 * @ingroup HISTORY
 * @brief Printing of the entire history-buffer via UART to the console.
 */
void hist_print( const bool doReturn );

#ifdef __cplusplus
}
#endif

/* MACRO's for XYZ module */
#define HIST_XYZ(s)     hist_add(HISTORY_XYZ_MODULE,s)
#define HIST_XYZx(s,x)  hist_add(HISTORY_XYZ_MODULE,s,(uint32_t)x);

#else /* CONFIG_USE_HISTORY */

#define hist_init(subsystemsEnabled)
#define hist_enableSubsystem(bit)
#define hist_disableSubsystem(bit)
#define hist_add(subsystem, msg)
#define hist_addx(subsystem, msg, data)
#define hist_print(doReturn)

   /* MACRO¹s for XYZ module */
   #define HIST_XYZ(s)
   #define HIST_XYZx(s,x)

#endif  /* /CONFIG_USE_HISTORY */


#endif /* _HISTORY_H */
/*================================== EOF ====================================*/
