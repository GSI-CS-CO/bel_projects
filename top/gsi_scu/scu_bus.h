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

#include <stdbool.h>
#include "scu_lm32_macros.h"
#include "scu_bus_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @defgroup SCU_BUS
 * @brief Register offset addresses and functions for the SCU bus-macro.
 */

/*!
 * @ingroup SCU_BUS
 * @brief Invalid register content.
 *        Preinitialized value when no hardware connected.
 * @todo What shall we do when 0xdead is a real register content?!? \n
 *       That isn't 100% impossible! >:-/
 * @note In any case it's not usable to probing all registers with that.
 */
#define SCUBUS_INVALID_VALUE     (uint16_t)0xdead

/*!
 * @ingroup SCU_BUS
 * @brief Address space in bytes for each SCU bus slave 128k
 */
#define SCUBUS_SLAVE_ADDR_SPACE  (1 << 17)

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


/*!
 * @ingroup SCU_BUS
 * @brief Definitions of SCU-bus slave (offset) addresses.
 * @see
 * <a href="https://www-acc.gsi.de/wiki/Hardware/Intern/StdRegScuBusSlave">
 *    Registersatz SCU-Bus-Slaves</a>
 */
typedef enum
{
   ID                   = 0x0001, /*!< @brief Slave-ID */
   FW_VERSION           = 0x0002, /*!< @brief FW Version of FPGA */
   FW_RELEASE           = 0x0003, /*!< @brief FW Release of FPGA */
   CID_SYSTEM           = 0x0004, /*!< @brief System Part of SCU Slave Component-ID */
   CID_GROUP            = 0x0005, /*!< @brief Group Part of SCU Slave Component-ID */
   VR_SCUBSL_Macro      = 0x0006, /*!< @brief Version/Revision of SCU Slave Macro */
   Extension_CID_System = 0x0007, /*!< @brief System Part of Extension Component-ID */
   Extension_CID_Group  = 0x0008, /*!< @brief Group Part of Extension Component-ID */
   CLK_10kHz            = 0x0009, /*!< @brief Clock frequency (in 10Hz steps) */
   __RFU__              = 0x000A, /*!< @brief Reserved for future use */
   Echo_Register        = 0x0010, /*!< @brief Read echoes the word written before */
   Status_Register      = 0x0011, /*!< @brief Bit 0= Power up done
                                   *          Bit 1=User Ready
                                   *          Bit 2..15=Reserved */
   Intr_In              = 0x0020, /*!< @brief Interrupt Input Register (Bit
                                   *          15:1=Actual Status, Bit 0 = 0)
                                   *          Synchronized status as seen on
                                   *          inputs of SCU Bus Slave macro */
   Intr_Ena             = 0x0021, /*!< @brief Interrupt Enable Register
                                   *          High-active Enable for Interrupt 15:1
                                   *          Bit0 = Powerup IRQ = always enabled
                                   *          Reset-value: 0x0001 */
   Intr_pending         = 0x0022, /*!< @brief Interrupt pending. */
   Intr_Active          = 0x0024, /*!< @brief Bit 15:1=interrupt, as seen after
                                   *          passing interrupt mask.
                                   *          Set to active when bits are
                                   *          enabled and input shows an interrupt
                                   *          “Active” bits are cleared when
                                   *          interrupts are disabled.
                                   *          “Active” bits are cleared \n
                                   *          *) on writing a 1 to bit position. \n
                                   *          *) when interrupt is still pending \n
                                   *             and not masked, bit is set again. */
   extern_clock         = 0x0030, /*!< @brief Extern clock */
   Info_text            = 0x01C0, /*!< @brief Zero terminated info-string max. 512 byte */
   mil_rd_wr_data       = 0x0400, /*!< @brief Reads received data from device bus.
                                   *          writes data to dev. bus (do only when TX is not
                                   *          busy,check mil_trm_rdy register bit) */
   mil_wr_cmd           = 0x0401, /*!< @brief read a received command word from dev.bus
                                   *          writes command to dev. bus ( do only when TX is not
                                   *          busy,check mil_trm_rdy register bit) */
   mil_wr_rd_status     = 0x0402, /*!< @brief read MIL Status bits 0...7 (for details see Table 14)
                                   *          write MIL Control Register bits 8..15 */
   rd_clr_no_vw_cnt     = 0x0403, /*!< @brief reads "no valid word" counters \n
                                   *          writes (clears) "no valid word" counters */
   rd_wr_not_eq_cnt     = 0x0404, /*!< @brief reads "not equal data/cmd" counters \n
                                   *          writes (clears) "not equal data/cmd" counters */
   rd_wr_ev_fifo        = 0x0405, /*!< @brief reads_event fifo only allowed when fifo
                                   *          isn't empty writes (sw-clear) event fifo */
   rd_clr_ev_timer      = 0x0406, /*!< @brief reads event timer upper word and stores LW in latch
                                   *          writes (sw-clear) complete event timer */
   rd_clr_dly_timer     = 0x0407, /*!< @brief reads delay timer upper word and stores LW in latch
                                   *          write is used for handle preload and start see Ch5.4 */
   rd_clr_wait_timer    = 0x0408, /*!< @brief reads wait timer upper word and store LW in latch \n
                                   *          writes (sw-clear) complete wait timer */
   mil_wr_rd_lemo_conf  = 0x0409, /*!< @brief Reads lemo config register (for details see Table 18) \n
                                   *          Writes the lemo config register */
   mil_wr_rd_lemo_dat   = 0x040A, /*!< @brief Reads lemo output data reg (details see Table 16) \n
                                   *          Writes the lemo output data register */
   mil_rd_lemo_inp      = 0x040B, /*!< @brief Reads lemo pin status (for details seeTable 17) \n
                                   *          Write has no effect */
   rd_ev_timer_lw       = 0x040C, /*!< @brief Reads LW ( bit 15..0) of event timer \n
                                   *          Write has no effect */
   rd_wait_timer_lw     = 0x040E, /*!< @brief Reads LW (bit 15..0) of wait timer \n
                                   *          Write has no effect */
   rd_wr_dly_timer_lw   = 0x0410, /*!< @brief Reads bit 15..0 of delay timer \n
                                   *          Write preloads the LW buffer (see Delay Timer) */
   rd_wr_dly_timer_hw   = 0x0411, /*!< @brief Reads bit 31..16 of delay timer \n
                                              Write preloads the HW buffer (see Delay Timer) */
   /*! @brief will used as list-terminator value */
   SCUBUS_INVALID_INDEX16 = (SCUBUS_SLAVE_ADDR_SPACE / sizeof(uint16_t))
} SCUBUS_ADDR_OFFSET_T;

#define POWER_UP_IRQ      0x0001

/* Deprecated defines */
#define CID_SYS           0x4
#define CID_GROUP         0x5
#define SLAVE_VERSION     0x6
#define SLAVE_INT_ENA     0x21
#define SLAVE_INT_PEND    0x22
#define SLAVE_INT_ACT     0x24
#define SLAVE_EXT_CLK     0x30

#define SLAVE_INFO_TEXT   0x01c0 //!< @brief Zero terminated info-string max. 512 byte

#define DAC1_BASE         0x200
#define DAC2_BASE         0x210
#define DAC_CNTRL         0x0
#define DAC_DATA          0x1

#define IO4x8             0x220
#define ADC_BASE          0x230
#define ADC_CNTRL         0x0
#define ADC_CHN1          0x1
#define ADC_CHN2          0x2
#define ADC_CHN3          0x3
#define ADC_CHN4          0x4
#define ADC_CHN5          0x5
#define ADC_CHN6          0x6
#define ADC_CHN7          0x7
#define ADC_CHN8          0x8

/*!
 * @see https://www-acc.gsi.de/wiki/Hardware/Intern/AdcDac2Scu
 */
#define FG1_BASE          0x300
#define FG2_BASE          0x340
#define FG_CNTRL          0x0
#define FG_A              0x1
#define FG_B              0x2
#define FG_BROAD          0x3
#define FG_SHIFT          0x4
#define FG_STARTH         0x5
#define FG_STARTL         0x6
#define FG_RAMP_CNT_LO    0x7
#define FG_RAMP_CNT_HI    0x8
#define FG_TAG_LOW        0x9
#define FG_TAG_HIGH       0xa
#define FG_VER            0xb

#define FG1_IRQ           (1<<15)
#define FG2_IRQ           (1<<14)
#define DREQ              (1<<4)
#define WB_FG_CNTRL       0x0
#define WB_FG_A           0x1
#define WB_FG_B           0x2
#define WB_FG_BROAD       0x3
#define WB_FG_SHIFTA      0x4
#define WB_FG_SHIFTB      0x5
#define WB_FG_START       0x6
#define WB_RAMP_CNT       0x7
#define WB_FG_SW_DST      0x8

#define TMR_BASE          0x330
#define TMR_CNTRL         0x0
#define TMR_IRQ_CNT       0x1
#define TMR_VALUEL        0x2
#define TMR_VALUEH        0x3
#define TMR_REPEAT        0x4

#define GLOBAL_IRQ_ENA    0x2
#define SRQ_ENA           0x6
#define SRQ_ACT           0x8
#define MULTI_SLAVE_SEL   0xc
#define MULTICAST_ACC     0x8
//#define MAX_SCU_SLAVES    12    /*!< @brief Maximum number of slots */

#define SYS_LOEP    3
#define SYS_CSCO    55
#define SYS_PBRF    42

#define GRP_ADDAC1  3
#define GRP_ADDAC2  38
#define GRP_DIOB    26
#define GRP_FIB_DDS 1
#define GRP_MFU     2
#define GRP_SIO3    69
#define GRP_SIO2    23

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Calculates the relative address offset in bytes of a SCU bus slave
 *        from the given slot number.
 * @see MAX_SCU_SLAVES
 * @param slot Slot number, valid range 1 .. MAX_SCU_SLAVES (12)
 * @return Relative slot address
 */
static inline uint32_t scuBusGetSlotOffset( const unsigned int slot )
{
   SCUBUS_ASSERT( slot >= SCUBUS_START_SLOT );
   SCUBUS_ASSERT( slot <= MAX_SCU_SLAVES );
   return slot * SCUBUS_SLAVE_ADDR_SPACE;
}

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
static inline void* scuBusGetAbsSlaveAddr( const void* pScuBusBase,
                                           const unsigned int slot )
{
   return &(((uint8_t*)pScuBusBase)[scuBusGetSlotOffset(slot)]);
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Reads a 16 bit register value from a SCU bus slave
 * @see scuBusGetAbsSlaveAddr
 * @see scuBusSetSlaveValue16
 * @param pAbsSlaveAddr Absolute SCU bus slave address e.g. obtained by scuBusGetAbsSlaveAddr
 * @param index Location of relevant register to read, that means offset to pAbsSlaveAddr
 * @return Content of the addressed register
 */
static inline volatile
uint16_t scuBusGetSlaveValue16( const void* pAbsSlaveAddr,
                                const unsigned int index )
{
   SCUBUS_ASSERT( index >= 0 );
   SCUBUS_ASSERT( index < (SCUBUS_SLAVE_ADDR_SPACE / sizeof(uint16_t)) );
   SCUBUS_ASSERT( ((unsigned int)pAbsSlaveAddr % sizeof(uint16_t)) == 0 ); // At least 2 bytes alignment assumed!

   return ((uint16_t* volatile)pAbsSlaveAddr)[index];
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
static inline
void scuBusSetSlaveValue16( void* pAbsSlaveAddr, const unsigned int index,
                            const uint16_t value )
{
   SCUBUS_ASSERT( index >= 0 );
   SCUBUS_ASSERT( index < (SCUBUS_SLAVE_ADDR_SPACE / sizeof(uint16_t)) );
   SCUBUS_ASSERT( ((unsigned int)pAbsSlaveAddr % sizeof(uint16_t)) == 0 ); // At least 2 bytes alignment assumed!

   ((uint16_t* volatile)pAbsSlaveAddr)[index] = value;
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS INTERRUPT
 * @brief Returns the pointer to slave interrupt enable register.
 * @param pScuBusBase Base-address of SCU-bus.
 * @return Pointer to the slave interrupt enable register.
 */
static inline
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
static inline void scuBusEnableSlaveInterrupt( const void* pScuBusBase,
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
static inline void scuBusDisableSlaveInterrupt( const void* pScuBusBase,
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
static inline
uint16_t* volatile scuBusGetInterruptActiveFlagRegPtr( const void* pScuBusBase,
                                                     const unsigned int slot )
{
   return &((uint16_t*)scuBusGetAbsSlaveAddr( pScuBusBase, slot ))[Intr_Active];
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS INTERRUPT
 * @brief Returns the pointer of the interrupt enable register of the given
 *        SCU-bus slave.
 * @param pScuBusBase Base-address of SU-bus.
 * @param slot Slave- respectively slot- number of slave.
 */
static inline
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
static inline
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
static inline
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
static inline volatile
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
static inline
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
                                               const uint16_t systemAddr,
                                               const uint16_t grupAddr );

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
