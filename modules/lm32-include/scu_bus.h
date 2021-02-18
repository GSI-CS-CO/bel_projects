/*!
 * @file scu_bus.h
 * @brief Administration of SCU-Bus for LM32 applications.
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * @see
 * <a href="https://www-acc.gsi.de/wiki/Hardware/Intern/StdRegScuBusSlave">
 *    Registersatz SCU-Bus-Slaves</a>
 *
 *******************************************************************************
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */
#ifndef _SCU_BUS_H
#define _SCU_BUS_H
#ifndef __lm32__
  #error Module is for target Lattice Micro 32 (LM32) only!
#endif

#include "scu_lm32_macros.h"
#include "scu_bus_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @ingroup PATCH
 * @brief Base Macro for accessing SCU-bus slaves via members of device
 *        objects
 * @see __WB_ACCESS
 * @param TO Object type.
 * @param p Pointer to the concerning object.
 * @param m Name of member variable.
 */
#define __SCU_BUS_ACCESS( TO, p, m ) __WB_ACCESS( TO, uint16_t, p, m )

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Calculates the absolute address of a SCU bus slave from the
 *        given slot number.
 * @see MAX_SCU_SLAVES
 * @see find_device_adr
 * @param pScuBusBase Base address of SCU bus.
 *                    Obtained by find_device_adr(GSI, SCU_BUS_MASTER);
 * @param slot Slot number, valid range 1 .. MAX_SCU_SLAVES (12)
 * @return Absolute SCU bus slave address
 */
STATIC inline void* scuBusGetAbsSlaveAddr( const void* pScuBusBase,
                                           const unsigned int slot )
{
   return &(((uint8_t*)pScuBusBase)[scuBusGetSlotOffset(slot)]);
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Returns a pointer of a 16 bit slave device register by index.
 * @param pScuBusBase Base address of SCU bus.
 *                    Obtained by find_device_adr(GSI, SCU_BUS_MASTER);
 * @param index Location of relevant register for access, that means offset to
 *              pAbsSlaveAddr
 * @return Pointer to the 16 bit slave register located by index.
 */
STATIC inline uint16_t* scuBusGetSlaveRegisterPtr16( const void* pAbsSlaveAddr,
                                                     const unsigned int index )
{
   SCUBUS_ASSERT( index >= 0 );
   SCUBUS_ASSERT( index < (SCUBUS_SLAVE_ADDR_SPACE / sizeof(uint16_t)) );

   return &((uint16_t* volatile)pAbsSlaveAddr)[index];
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Returns a pointer of a 16 bit slave device register located by slot-
 *        number and 16-bit offset index.
 * @param pScuBusBase Base address of SCU bus.
 * @param slot Slot-number between 1 and 12.
 *                    Obtained by find_device_adr(GSI, SCU_BUS_MASTER);
 * @param index Location of relevant register for access, that means offset
 *              within the slave address room.
 * @return Pointer to the 16 bit slave register located by index.
 */
STATIC inline
uint16_t* scuBusGetSlaveRegisterPtr16BySlot( const void* pScuBusBase,
                                             const unsigned int slot,
                                             const unsigned int index )
{
   return scuBusGetSlaveRegisterPtr16( scuBusGetAbsSlaveAddr( pScuBusBase, slot ), index );
}


/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Reads a 16 bit register value from a SCU bus slave
 * @see scuBusGetAbsSlaveAddr
 * @see scuBusSetSlaveValue16
 * @param pAbsSlaveAddr Absolute SCU bus slave address e.g. obtained by
 *                      scuBusGetAbsSlaveAddr
 * @param index Location of relevant register to read, that means offset to
 *              pAbsSlaveAddr
 * @return Content of the addressed register
 */
STATIC inline volatile
uint16_t scuBusGetSlaveValue16( const void* pAbsSlaveAddr,
                                const unsigned int index )
{
   /*
    * At least 2 bytes alignment assumed!
    */
   SCUBUS_ASSERT( ((unsigned int)pAbsSlaveAddr % sizeof(uint16_t)) == 0 );

   //return ((uint16_t* volatile)pAbsSlaveAddr)[index];
   return *scuBusGetSlaveRegisterPtr16( pAbsSlaveAddr, index );
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Writes a given 16 bit value in the addressed SCU bus slave register.
 * @see scuBusGetAbsSlaveAddr
 * @see scuBusGetSlaveValue16
 * @param pAbsSlaveAddr Absolute SCU bus slave address e.g. obtained by scuBusGetAbsSlaveAddr
 * @param index Location of relevant register to read, that means offset to pAbsSlaveAddr
 * @param value 16 bit value to write
 */
STATIC inline
void scuBusSetSlaveValue16( void* pAbsSlaveAddr, const unsigned int index,
                            const uint16_t value )
{
   SCUBUS_ASSERT( index >= 0 );
   SCUBUS_ASSERT( index < (SCUBUS_SLAVE_ADDR_SPACE / sizeof(uint16_t)) );

   /*
    * At least 2 bytes alignment assumed!
    */
   SCUBUS_ASSERT( ((unsigned int)pAbsSlaveAddr % sizeof(uint16_t)) == 0 );

   *scuBusGetSlaveRegisterPtr16( pAbsSlaveAddr, index ) = value;
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS INTERRUPT
 * @brief Returns the pointer to slave interrupt enable register.
 * @param pScuBusBase Base-address of SCU-bus.
 * @return Pointer to the slave interrupt enable register.
 */
STATIC inline
uint16_t* volatile scuBusGetMasterInterruptEnableRegPtr( const void* pScuBusBase )
{
   return &((uint16_t* volatile)pScuBusBase)[SRQ_ENA];
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS INTERRUPT
 * @brief Enables the SCU-bus slave interrupt of the given slot.
 * @param pScuBusBase Base-address of SCU-bus.
 * @param slot Slot number of the slave.
 */
STATIC inline void scuBusEnableSlaveInterrupt( const void* pScuBusBase,
                                               const unsigned int slot )
{
   SCUBUS_ASSERT( slot > 0 );
   SCUBUS_ASSERT( slot <= MAX_SCU_SLAVES );
   *scuBusGetMasterInterruptEnableRegPtr( pScuBusBase ) |= (1 << (slot-1));
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS INTERRUPT
 * @brief Disables the SCU-bus slave interrupt of the given slot.
 * @param pScuBusBase Base-address of SCU-bus.
 * @param slot Slot number of the slave.
 */
STATIC inline void scuBusDisableSlaveInterrupt( const void* pScuBusBase,
                                               const unsigned int slot )
{
   SCUBUS_ASSERT( slot > 0 );
   SCUBUS_ASSERT( slot <= MAX_SCU_SLAVES );
   *scuBusGetMasterInterruptEnableRegPtr( pScuBusBase ) &= ~(1 << (slot-1));
}


/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS INTERRUPT
 * @brief Returns the pointer of the interrupt active register of the given
 *        SCU-bus slave.
 * @param pScuBusBase Base-address of SU-bus.
 * @param slot Slave- respectively slot- number of slave.
 */
STATIC inline
uint16_t* volatile scuBusGetInterruptActiveFlagRegPtr( const void* pScuBusBase,
                                                     const unsigned int slot )
{
   return &((uint16_t*)scuBusGetAbsSlaveAddr( pScuBusBase, slot ))[Intr_Active];
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS INTERRUPT
 * @brief Saves the current interrupt pending bits from the slaves interrupt
 *        pending register as return value and resets this register.
 * @param pScuBusBase Base-address of SU-bus.
 * @param slot Slave- respectively slot- number of slave.
 * @return Flag field of all pending interrupts of this slave before this
 *         function call.
 */
volatile STATIC inline
uint16_t scuBusGetAndResetIterruptPendingFlags( const void* pScuBusBase,
                                                const unsigned int slot )
{ /*
   * Note: Keep in mind that is a memory mapped i/o handling and the
   *       following lines seems a bit confused, respectively
   *       logical meaningless.
   */

   uint16_t* volatile pIntActive = scuBusGetInterruptActiveFlagRegPtr(
                                                          pScuBusBase, slot );
   volatile const uint16_t intActive = *pIntActive;

   /*
    * The interrupt pending flags becomes deleted by writing a one in
    * the concerning bit position(s).
    */
   *pIntActive = intActive;

   return intActive;
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS INTERRUPT
 * @brief Returns the pointer of the interrupt enable register of the given
 *        SCU-bus slave.
 * @param pScuBusBase Base-address of SU-bus.
 * @param slot Slave- respectively slot- number of slave.
 */
STATIC inline
uint16_t* volatile scuBusGetInterruptEnableFlagRegPtr( const void* pScuBusBase,
                                                     const unsigned int slot )
{
   return &((uint16_t*)scuBusGetAbsSlaveAddr( pScuBusBase, slot ))[Intr_Ena];
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Sets unique bits via OR link in a 16 bit SCU bus register
 * @param pAbsSlaveAddr Absolute SCU bus slave address e.g. obtained by scuBusGetAbsSlaveAddr
 * @param index Location of relevant register to read, that means offset to pAbsSlaveAddr
 * @param flags16 Bit field
 */
STATIC inline
void scuBusSetRegisterFalgs( void* pAbsSlaveAddr, const unsigned int index,
                             const uint16_t flags16 )
{
   SCUBUS_ASSERT( index >= 0 );
   SCUBUS_ASSERT( index < (SCUBUS_SLAVE_ADDR_SPACE / sizeof(uint16_t)) );
   SCUBUS_ASSERT( ((unsigned int)pAbsSlaveAddr % sizeof(uint16_t)) == 0 ); // At least 2 bytes alignment assumed!

   ((uint16_t* volatile)pAbsSlaveAddr)[index] |= flags16;
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Clears unique bits via AND link in a 16 bit SCU bus register
 * @param pAbsSlaveAddr Absolute SCU bus slave address e.g. obtained by scuBusGetAbsSlaveAddr
 * @param index Location of relevant register to read, that means offset to pAbsSlaveAddr
 * @param flags16 Bit field
 */
STATIC inline
void scuBusClearRegisterFalgs( void* pAbsSlaveAddr, const unsigned int index,
                               const uint16_t flags16 )
{
   SCUBUS_ASSERT( index >= 0 );
   SCUBUS_ASSERT( index < (SCUBUS_SLAVE_ADDR_SPACE / sizeof(uint16_t)) );
   SCUBUS_ASSERT( ((unsigned int)pAbsSlaveAddr % sizeof(uint16_t)) == 0 ); // At least 2 bytes alignment assumed!

   ((uint16_t* volatile)pAbsSlaveAddr)[index] &= flags16;
}


/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Reads a 32 bit register value from a SCU bus slave
 * @see scuBusGetAbsSlaveAddr
 * @see scuBusSetSlaveValue16
 * @param pAbsSlaveAddr Absolute SCU bus slave address e.g. obtained by scuBusGetAbsSlaveAddr
 * @param index Location of relevant register to read, that means offset to pAbsSlaveAddr
 * @return Content of the addressed register
 */
STATIC inline volatile
uint32_t scuBusGetSlaveValue32( const void* pAbsSlaveAddr, const unsigned int index )
{
   SCUBUS_ASSERT( index >= 0 );
   SCUBUS_ASSERT( index < (SCUBUS_SLAVE_ADDR_SPACE / sizeof(uint32_t)) );
   SCUBUS_ASSERT( ((unsigned int)pAbsSlaveAddr % sizeof(uint16_t)) == 0 ); // At least 2 bytes alignment assumed!

   return ((uint32_t* volatile)pAbsSlaveAddr)[index];
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Writes a given 32 bit value in the addressed SCU bus slave register.
 * @see scuBusGetAbsSlaveAddr
 * @see scuBusGetSlaveValue16
 * @param pAbsSlaveAddr Absolute SCU bus slave address e.g. obtained by scuBusGetAbsSlaveAddr
 * @param index Location of relevant register to read, that means offset to pAbsSlaveAddr
 * @param value 16 bit value to write
 */
STATIC inline
void scuBusSetSlaveValue32( void* pAbsSlaveAddr, const unsigned int index,
                            const uint32_t value )
{
   SCUBUS_ASSERT( index >= 0 );
   SCUBUS_ASSERT( index < (SCUBUS_SLAVE_ADDR_SPACE / sizeof(uint32_t)) );
   SCUBUS_ASSERT( ((unsigned int)pAbsSlaveAddr % sizeof(uint16_t)) == 0 ); // At least 2 bytes alignment assumed!

   ((uint32_t* volatile)pAbsSlaveAddr)[index] = value;
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Calculates the number of slaves from the slave flag field.
 * @see scuBusFindSlavesByMatchList16
 * @see scuBusFindAllSlaves
 * @param slaveFlags Slave flag field
 * @return Number of SCU-bus slaves
 */
unsigned int scuBusGetNumberOfSlaves( const SCUBUS_SLAVE_FLAGS_T slaveFlags );

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Item type of scu-bus match list.
 *
 * Necessary to find specific scu-bus devices with attributes matching by
 * each item of this list.
 *
 * @see scuBusFindSlaveByMatchList16
 * @see SCUBUS_MATCH_LIST16_TERMINATOR
 * @see SCUBUS_FIND_MODE_T
 * @note The last item of pMatchList has always to be the terminator:
 *       SCUBUS_MATCH_LIST16_TERMINATOR
 */
typedef struct
{
   SCUBUS_ADDR_OFFSET_T index; //!< @brief Relative address resp. index of value to match.
   uint16_t             value; //!< @brief 16 bit value to match
} SCU_BUS_MATCH_ITEM16_T;

/*!
 * @ingroup SCU_BUS
 * @brief Terminator of a scu-bus match-list it has to be always the last item
 *        of the list.
 * @see SCU_BUS_MATCH_ITEM16_T
 * @see SCUBUS_INVALID_INDEX16
 * @note Don't forget it!
 */
#define SCUBUS_MATCH_LIST16_TERMINATOR { .index = SCUBUS_INVALID_INDEX16, .value = 0 }

/*!
 * @ingroup SCU_BUS
 * @brief Data type for the third argument of function scuBusFindSlavesByMatchList16
 *
 * It determines whether the whole items of the match-list has to be match, or
 * only one item of the list.
 * @see SCU_BUS_MATCH_ITEM16_T
 * @see scuBusFindSlavesByMatchList16
 */
typedef enum
{
   ALL, //!< @brief All items of the match list has to be match
   ANY  //!< @brief Only one item of the match list has to be match
} SCUBUS_FIND_MODE_T;

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Finds all scu-bus slaves which match by one or all items of the
 *        given match-list depending on mode.
 * @see SCU_BUS_MATCH_ITEM16_T
 * @see scuBusIsSlavePresent
 * @see scuBusFindAllSlaves
 * @see find_device_adr
 * @see SCUBUS_FIND_MODE_T
 * @param pScuBusBase Base address of SCU bus.
 *                    Obtained by find_device_adr(GSI, SCU_BUS_MASTER);
 * @param pMatchList  Match-list with SCU_BUS_MATCH_LIST16_TERMINATOR as last element.
 * @note The last item of pMatchList has always to be the terminator:
 *       SCU_BUS_MATCH_LIST16_TERMINATOR
 * @param mode Determines how the match-list becomes handled.
 * @return Flag field for SCU present bits e.g.: \n
 *         0000 0000 0010 1000: means: Slot 4 and 6 are used by devices where \n
 *         all or one item of the given match-list match depending on parameter mode.
 */
SCUBUS_SLAVE_FLAGS_T scuBusFindSlavesByMatchList16( const void* pScuBusBase,
                                                 const SCU_BUS_MATCH_ITEM16_T pMatchList[],
                                                 const SCUBUS_FIND_MODE_T mode );

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Scans the whole SCU bus and initialized a slave-flags present field if
 *        the given system address and group address match.
 * @see scuBusIsSlavePresent
 * @see scuBusFindAllSlaves
 * @see find_device_adr
 * @param pScuBusBase Base address of SCU bus.
 *                    Obtained by find_device_adr(GSI, SCU_BUS_MASTER);
 * @param systemAddr System address
 * @param groupAddr  group address
 * @return Flag field for SCU present bits e.g.: \n
 *         0000 0000 0010 1000: means: Slot 4 and 6 are used by devices where \n
 *         system address and group address match.
 */
SCUBUS_SLAVE_FLAGS_T scuBusFindSpecificSlaves( const void* pScuBusBase,
                                               const SLAVE_SYSTEM_T systemAddr,
                                               const SLAVE_GROUP_T grupAddr );

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Scans the whole SCU bus for all slots and initialized a slave-flags
 *        present field for each found device.
 * @see scuBusIsSlavePresent
 * @see scuBusFindSpecificSlaves
 * @param pScuBusBase Base address of SCU bus.
 *                    Obtained by find_device_adr(GSI, SCU_BUS_MASTER);
 * @return Flag field for SCU present bits e.g.: \n
 *         0000 0001 0001 0000: means: Slot 5 and 9 are used all others are free.
 */
SCUBUS_SLAVE_FLAGS_T scuBusFindAllSlaves( const void* pScuBusBase );

#ifdef __cplusplus
}
#endif

#endif /* ifndef _SCU_BUS_H */
/* ================================= EOF ====================================*/
