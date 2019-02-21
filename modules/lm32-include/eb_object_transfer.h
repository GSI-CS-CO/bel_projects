/*!
 * @brief     Some helper functions and macros for simplifying the
 *            data transfer of flat objects of types struct, union or class
 *            via wishbone/etherbone bus.
 *
 * @note
 * Flat objects means: the object doesn't contain members of type pointer or
 * reverence.
 *
 * Based on etherbone.h
 *
 * Required library: etherbone
 *
 * @file      eb_object_transfer.h
 * @see       eb_object_transfer.c
 * @see       etherbone.h
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      21.02.2019
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
#ifndef _EB_OBJECT_TRANSFER_H
#define _EB_OBJECT_TRANSFER_H

#ifndef __linux__
 #error This module is only for Linux targets!
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <etherbone.h>
#include <stdbool.h>
#include <scu_assert.h>
#include <eb_console_helper.h>
#include <helper_macros.h>

/*!
 * @defgroup EB_HELPER
 * @brief Some helper functions and macros simplifying the
 *        data transfer of flat objects of types struct, union or class
 *        via wishbone/etherbone bus.
 * @{
 */

#define SHARED_BASE_ADDRESS (0x100A0000 + INT_BASE_ADR + SHARED_OFFS)

#define GET_ADDR_OF_MEMBER( type, member ) \
   (SHARED_BASE_ADDRESS + offsetof( type, member ))

#define EB_FOR_MEMBER( type, member ) \
   GET_ADDR_OF_MEMBER( type, member ), GET_SIZE_OF_MEMBER( type, member ) | EB_BIG_ENDIAN


#define EB_INIT_INFO_ITEM( name, memberAcess, index )  \
{                                                      \
   typeof( index ) _index = index;                     \
   name[_index].pData = (uint8_t*)&(memberAcess);      \
   name[_index].size  = sizeof( memberAcess );         \
}

#define EB_INIT_CB_ARG( arg, infoArray )               \
{                                                      \
   (arg).aInfo   = infoArray;                          \
   (arg).infoLen = ARRAY_SIZE( infoArray );            \
   (arg).exit    = false;                              \
}

#define EB_OJECT_MEMBER_READ( pThis, type, member )                   \
   eb_cycle_read( pThis->cycle, EB_FOR_MEMBER( type, member ), NULL )


#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
   eb_socket_t socket;
   eb_device_t device;
   eb_cycle_t  cycle;
} EB_HANDLE_T;

typedef struct
{
   uint8_t* pData;
   size_t   size;
} EB_MEMBER_INFO_T;

typedef struct
{
   bool              exit;
   eb_status_t       status;
   size_t            infoLen;
   EB_MEMBER_INFO_T* aInfo;
} EB_CYCLE_CB_ARG_T;

eb_status_t ebOpen( EB_HANDLE_T* pThis, char* name );

eb_status_t ebClose( EB_HANDLE_T* pThis );

void ebCycleReadIoObjectCb( eb_user_data_t user, eb_device_t dev,
                             eb_operation_t op, eb_status_t status );

static inline
eb_status_t ebObjectCycleOpen( EB_HANDLE_T* pThis, EB_CYCLE_CB_ARG_T* pCArg )
{
   return eb_cycle_open( pThis->device, pCArg, ebCycleReadIoObjectCb,
                         &pThis->cycle );
}

static inline
eb_status_t ebCycleClose( EB_HANDLE_T* pThis )
{
   return eb_cycle_close( pThis->cycle );
}

static inline
eb_status_t ebSocketRun( EB_HANDLE_T* pThis )
{
   return eb_socket_run( pThis->socket, 10000 );
}

#ifdef __cplusplus
}
#endif
/*!@}*/
#endif /* ifndef _EB_OBJECT_TRANSFER_H */
/*================================== EOF ====================================*/
