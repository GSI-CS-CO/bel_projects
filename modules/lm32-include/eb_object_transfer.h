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

/*! ---------------------------------------------------------------------------
 */
#ifndef EB_LM32_BASE
   #define EB_LM32_BASE 0x100A0000
#endif

/*! ---------------------------------------------------------------------------
 * @brief Base address of the Linux perspective of the shared memory for
 *        the communication between LM32 and Linux.
 * @note The macros INT_BASE_ADR and SHARED_OFFS are project dependent and
 *       will be defined in the automatically generated header file
 *       "generated/shared_mmap.h". Therefore this file has to be also
 *       registered in the header file include path of the associated Linux
 *       project. For this reason the Linux module depends on the
 *       LM32 module, whereby the LM32-module has to be compiled first.
 */
#define EB_LM32_SHARED_BASE_ADDRESS (EB_LM32_BASE + INT_BASE_ADR + SHARED_OFFS)

/*! ---------------------------------------------------------------------------
 * @brief Macro calculates the eb/wb address of a member variable of a
 *        transfer object for the communication between LM32 and Linux.
 * @param type Name of the data type including the concerning member.
 * @param member Name of the member variable.
 */
#define EB_LM32_GET_ADDR_OF_MEMBER( type, member )                            \
   (EB_LM32_SHARED_BASE_ADDRESS + offsetof( type, member ))

/*! ---------------------------------------------------------------------------
 */
#define EB_LM32_FOR_MEMBER( type, member )                                    \
   EB_LM32_GET_ADDR_OF_MEMBER( type, member ),                                \
   GET_SIZE_OF_MEMBER( type, member ) | EB_BIG_ENDIAN

/*! ---------------------------------------------------------------------------
 */
#define __EB_INIT_INFO_ITEM( name, index, memberAcess )                       \
   name[index].pData = (uint8_t*)&(memberAcess);                              \
   name[index].size  = sizeof( memberAcess )                                  \

/*! ---------------------------------------------------------------------------
 */
#define EB_INIT_INFO_ITEM_STATIC( name, index, memberAcess )                  \
{                                                                             \
   STATIC_ASSERT( index >= 0  );                                              \
   STATIC_ASSERT( index < ARRAY_SIZE( name ) );                               \
   __EB_INIT_INFO_ITEM( name, index, memberAcess );                           \
}

/*! ---------------------------------------------------------------------------
 */
#define EB_INIT_INFO_ITEM( name, index, memberAcess )                         \
{                                                                             \
   size_t _index = index;                                                     \
   SCU_ASSERT( _index < ARRAY_SIZE( name ) );                                 \
   __EB_INIT_INFO_ITEM( name, _index, memberAcess );                          \
}

/*! ---------------------------------------------------------------------------
 */
#define EB_INIT_CB_OR_ARG( arg, infoArray )                                   \
{                                                                             \
   (arg).aInfo   = infoArray;                                                 \
   (arg).infoLen = ARRAY_SIZE( infoArray );                                   \
   (arg).exit    = false;                                                     \
}

/*! ---------------------------------------------------------------------------
 */
#define EB_MAKE_CB_OR_ARG( arg, infoArray )                                   \
   EB_CYCLE_OR_CB_ARG_T arg;                                                  \
   EB_INIT_CB_OR_ARG( arg, infoArray )

/*! ---------------------------------------------------------------------------
 */
#define EB_INIT_CB_OW_ARG( arg )                                              \
{                                                                             \
   (arg).exit    = false;                                                     \
}

/*! ---------------------------------------------------------------------------
 */
#define EB_MAKE_CB_OW_ARG( arg )                                              \
   EB_CYCLE_OW_CB_ARG_T arg;                                                  \
   EB_INIT_CB_OW_ARG( arg )

/*! ---------------------------------------------------------------------------
 */
#define EB_OJECT_MEMBER_READ( pThis, type, member )                           \
   eb_cycle_read( pThis->cycle, EB_LM32_FOR_MEMBER( type, member ), NULL )

/*! ---------------------------------------------------------------------------
 * @brief Macro builds the argument list of the last three arguments for
 *        the function eb_cycle_write() to simplify the communication
 *        with LM32.
 * @param ptr Pointer to the concerning object.
 * @param member Name of the member variable.
 */
#define EB_LM32_OJECT_MEMBER_WRITE_ARG( ptr, member )                         \
      EB_LM32_GET_ADDR_OF_MEMBER( TYPEOF( *(ptr) ), member ),                 \
                   sizeof( (ptr)->member ) | EB_BIG_ENDIAN,                   \
                   (ptr)->member

/*! ---------------------------------------------------------------------------
 * @brief Macro for sending a member variable via eb/wb bus to LM32.
 * @param pThis Pointer to the EB_HANDLE_T
 * @param ptr Pointer to the concerning object.
 * @param member Name of the member variable.
 */
#define EB_LM32_OJECT_MEMBER_WRITE( pThis, ptr, member )                      \
   eb_cycle_write( pThis->cycle,                                              \
                            EB_LM32_OJECT_MEMBER_WRITE_ARG( ptr, member ) )

/*! ---------------------------------------------------------------------------
 * @brief Macro builds the argument list of the last three arguments for
 *        the function eb_cycle_write() to simplify the communication
 *        with LM32. Especially for bit fields.
 * @note <b>CAUTION:</b> This macro includes a "bad cast" preventing a
 *       compiler error! \n
 *       Use this macro for bit-fields only and if you exactly know
 *       what you do.
 *       The size of these bit fields shall not exceed the size of
 *       "eb_data_t"! \n
 *       When the concerning bit field is the last element of the whole
 *       transfer object and its size is smaller than "eb_data_t", so a
 *       following padding field becomes necessary, \n
 *       otherwise a segmentation error could occur!
 * @note Keep in mind: The order of bit field elements depends on the
 *       endianness convention!
 * @param ptr Pointer to the concerning object.
 * @param bfMember Name of the member variable of type bit field.
 */
#define EB_LM32_OJECT_MEMBER_WRITE_BF_ARG( ptr, bfMember )                    \
   EB_LM32_GET_ADDR_OF_MEMBER( TYPEOF( *(ptr) ), bfMember ),                  \
   sizeof( (ptr)->bfMember ) | EB_BIG_ENDIAN,                                 \
   *((eb_data_t*)&((ptr)->bfMember)) &                                        \
   ~(((eb_data_t)~0) << BIT_SIZEOF( (ptr)->bfMember )) )

/*! ---------------------------------------------------------------------------
 * @brief Macro for sending bit fields via eb/wb bus to LM32.
 * @note <b>CAUTION:</b> This macro includes a "bad cast" preventing a
 *       compiler error! \n
 *       Use this macro for bit-fields only and if you exactly know
 *       what you do.
 *       The size of these bit fields shall not exceed the size of
 *       "eb_data_t"! \n
 *       When the concerning bit field is the last element of the whole
 *       transfer object and its size is smaller than "eb_data_t", so a
 *       following padding field becomes necessary, \n
 *       otherwise a segmentation error could occur!
 * @note Keep in mind: The order of bit field elements depends on the
 *       endianness convention!
 * @param pThis Pointer to the EB_HANDLE_T
 * @param ptr Pointer to the concerning object.
 * @param bfMember Name of the member variable of type bit field.
 */
#define EB_LM32_OJECT_MEMBER_WRITE_BF( pThis, ptr, bfMember )                 \
   eb_cycle_write( pThis->cycle,                                              \
                   EB_LM32_OJECT_MEMBER_WRITE_BF_ARG( ptr, bfMember ))

#ifdef __cplusplus
extern "C" {
namespace Scu
{
#endif

/*! ---------------------------------------------------------------------------
 * @brief Wishbone/etherbone handle type.
 */
typedef struct
{
   eb_socket_t socket;
   eb_device_t device;
   eb_cycle_t  cycle;
   eb_status_t status;
} EB_HANDLE_T;

/*! ---------------------------------------------------------------------------
 * @brief Information type for each single data target.
 * @see EB_CYCLE_OR_CB_ARG_T
 */
typedef struct
{
   uint8_t* pData; /*!<@brief Target address */
   size_t   size;  /*!<@brief Length in bytes of the target data type */
} EB_MEMBER_INFO_T;

/*! ---------------------------------------------------------------------------
 * @brief Argument data type for object read callback functions.
 */
typedef struct
{
   /*! @brief Exit flag */
   bool              exit;

   /*! @brief Storing of the status of the callback function.*/
   eb_status_t       status;

   /*! @brief Array size of "aInfo" */
   size_t            infoLen;

   /*! @brief Start address of info array with "infoLen elements. */
   EB_MEMBER_INFO_T* aInfo;
} EB_CYCLE_OR_CB_ARG_T;

/*! ---------------------------------------------------------------------------
 * @brief Argument data type for object write callback functions.
 */
typedef struct
{
   /*! @brief Exit flag */
   bool        exit;

   /*! @brief Storing of the status of the callback function.*/
   eb_status_t status;
} EB_CYCLE_OW_CB_ARG_T;

/*! ---------------------------------------------------------------------------
 */
static inline
eb_status_t ebGetStatus( EB_HANDLE_T* pThis )
{
   return pThis->status;
}

/*! ---------------------------------------------------------------------------
 */
static inline
const char* ebGetStatusString( EB_HANDLE_T* pThis )
{
   return eb_status( pThis->status );
}

/*! ---------------------------------------------------------------------------
 */
eb_status_t ebOpen( EB_HANDLE_T* pThis, const char* name );

/*! ---------------------------------------------------------------------------
 */
eb_status_t ebClose( EB_HANDLE_T* pThis );

/*! ---------------------------------------------------------------------------
 */
eb_status_t ebFindFirstDeviceAddrById( EB_HANDLE_T* pThis,
                                   uint64_t vendorId, uint32_t deviceId,
                                   uint32_t* pDevAddr );

/*! ---------------------------------------------------------------------------
 */
void __ebCycleReadIoObjectCb( eb_user_data_t user, eb_device_t dev,
                              eb_operation_t op, eb_status_t status );

/*! ---------------------------------------------------------------------------
 */
static inline
eb_status_t ebObjectReadCycleOpen( EB_HANDLE_T* pThis,
                                   EB_CYCLE_OR_CB_ARG_T* pCArg )
{
   SCU_ASSERT( pThis != NULL );
   SCU_ASSERT( pCArg != NULL );

   pCArg->exit = false;
   pThis->status = eb_cycle_open( pThis->device, pCArg,
                                  __ebCycleReadIoObjectCb,
                                  &pThis->cycle );
   if( pThis->status != EB_OK )
      pThis->cycle = 0;
   return pThis->status;
}

/*! ---------------------------------------------------------------------------
 */
static inline
void ebCycleRead( EB_HANDLE_T* pThis, eb_address_t addr,
                                         eb_format_t format, eb_data_t* pData )
{
   SCU_ASSERT( pThis->cycle != 0 );
   eb_cycle_read( pThis->cycle, addr, format, pData );
}

/*! ---------------------------------------------------------------------------
 */
void __ebCycleWriteIoObjectCb( eb_user_data_t user, eb_device_t dev,
                              eb_operation_t op, eb_status_t status );

/*! ---------------------------------------------------------------------------
 */
eb_status_t ebReadData32( EB_HANDLE_T* pThis, uint32_t addr, uint32_t* pData,
                          size_t len );

/*! ---------------------------------------------------------------------------
 */
eb_status_t ebWriteData32( EB_HANDLE_T* pThis, uint32_t addr, uint32_t* pData,
                           size_t len );

/*! ---------------------------------------------------------------------------
 */
static inline
eb_status_t ebObjectWriteCycleOpen( EB_HANDLE_T* pThis,
                                    EB_CYCLE_OW_CB_ARG_T* pCArg )
{
   SCU_ASSERT( pThis != NULL );
   SCU_ASSERT( pCArg != NULL );

   pCArg->exit = false;
   pThis->status = eb_cycle_open( pThis->device, pCArg,
                                  __ebCycleWriteIoObjectCb,
                                  &pThis->cycle );
   return pThis->status;
}


/*! ---------------------------------------------------------------------------
 */
static inline
eb_status_t ebCycleClose( EB_HANDLE_T* pThis )
{
   pThis->status = eb_cycle_close( pThis->cycle );
   if( pThis->status == EB_OK )
      pThis->cycle = 0;
   return pThis->status;
}

/*! ---------------------------------------------------------------------------
 */
static inline
void _ebSetOrStatus( EB_HANDLE_T* pThis, EB_CYCLE_OR_CB_ARG_T* pArg )
{
   pThis->status = pArg->status;
}

/*! ---------------------------------------------------------------------------
 */
static inline
eb_status_t ebSocketRun( EB_HANDLE_T* pThis )
{
   pThis->status = eb_socket_run( pThis->socket, 10000 );
   return pThis->status;
}

/*! ---------------------------------------------------------------------------
 */
static inline
eb_status_t ebDeviceRead( EB_HANDLE_T* pThis,
                          eb_address_t   address,
                          eb_format_t    format,
                          eb_data_t*     pData )
{
   pThis->status = eb_device_read( pThis->device, address, format, pData,
                                                              NULL, eb_block );
   return pThis->status;
}

#ifdef __cplusplus
} /* namespace Scu */
} /* extern "C" */
#endif
/*!@}*/
#endif /* ifndef _EB_OBJECT_TRANSFER_H */
/*================================== EOF ====================================*/
