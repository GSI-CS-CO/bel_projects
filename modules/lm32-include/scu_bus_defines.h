/*!
 *  @file scu_bus_defines.h
 *  @brief Some definitions and inline functions of the SCU-Bus for
 *         LM32 and Linux.
 * @note Header only!
 *
 * Outsourced from scu_bus.h
 *
 *  @see scu_bus.h
 *  @see scu_bus.c
 *  @date 04.03.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
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
#ifndef _SCU_BUS_DEFINES_H
#define _SCU_BUS_DEFINES_H

#include <stdint.h>
#include <stdbool.h>
#include <helper_macros.h>

#ifdef CONFIG_SCU_BUS_PEDANTIC_CHECK
   /* Paranoia mode is enabled... ;-) */
   #include <scu_assert.h>
   #define SCUBUS_ASSERT SCU_ASSERT
#else
   #define SCUBUS_ASSERT(__e) ((void)0)
#endif

/*!
 * @defgroup SCU_BUS
 * @brief Register offset addresses and functions for the SCU bus-macro.
 */

#ifdef __cplusplus
extern "C" {
namespace Scu
{
namespace Bus
{
#endif

/*!
 * @ingroup SCU_BUS
 * @brief Basic form-factor constants of the SCU-bus
 */
typedef enum
{ /*!
   * @ingroup SCU_BUS
   * @brief Physical maximum number of SCU-Bus slots
   */
   MAX_SCU_SLAVES           = 12,

   /*!
    * @ingroup SCU_BUS
    * @brief First slot of SCU-bus
    */
   SCUBUS_START_SLOT        = 1,

   /*!
    * @ingroup SCU_BUS
    * @brief Invalid register content.
    *        Preinitialized value when no hardware connected.
    * @todo What shall we do when 0xdead is a real register content?!? \n
    *       That isn't 100% impossible! >:-/
    * @note In any case it's not usable to probing all registers with that.
    */
   SCUBUS_INVALID_VALUE     = 0xDEAD,

   /*!
    * @ingroup SCU_BUS
    * @brief Address space in bytes for each SCU bus slave 128k
    */
   SCUBUS_SLAVE_ADDR_SPACE  = (1 << 17)
} BASIC_T;

/*!
 * @ingroup SCU_BUS
 * @brief Flag field for slaves connected in the SCU bus.
 *
 * Each bit reflects a slot in the SCU bus.
 * If a bit equal one so a SCU device is connected at this place. \n
 * E.g.: \n
 * 000000010101 means: Slot 1, 3 and 5 are used.
 * @see MAX_SCU_SLAVES
 */
typedef uint16_t SCUBUS_SLAVE_FLAGS_T;
#ifndef __DOXYGEN__
STATIC_ASSERT( BIT_SIZEOF( SCUBUS_SLAVE_FLAGS_T ) >= MAX_SCU_SLAVES );
#endif

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

/*!
 * @ingroup SCU_BUS
 * @brief Offset base addresses of devices in SCU-bus slaves
 */
typedef enum
{
   DAC1_BASE = 0x200,
   DAC2_BASE = 0x210,
   FG1_BASE  = 0x300,
   FG2_BASE  = 0x340,
   TMR_BASE  = 0x330,
   ADC_BASE  = 0x230
} BUS_BASE_T;


/*!
 * @ingroup SCU_BUS
 * @ingroup INTERRUPT
 * @brief Relative register addresses and interrupt flags of specific SCU-bus slaves.
 */
typedef enum
{
   POWER_UP_IRQ    = 0x0001,

   DAC_CNTRL       = 0x0000,
   DAC_DATA        = 0x0001,

   IO4x8           = 0x0220,
   ADC_CNTRL       = 0x0000,
   ADC_CHN1        = 0x0001,
   ADC_CHN2        = 0x0002,
   ADC_CHN3        = 0x0003,
   ADC_CHN4        = 0x0004,
   ADC_CHN5        = 0x0005,
   ADC_CHN6        = 0x0006,
   ADC_CHN7        = 0x0007,
   ADC_CHN8        = 0x0008,

   /*!
    * @see https://www-acc.gsi.de/wiki/Hardware/Intern/AdcDac2Scu
    */
   FG_CNTRL        = 0x0000,
   FG_A            = 0x0001,
   FG_B            = 0x0002,
   FG_BROAD        = 0x0003,
   FG_SHIFT        = 0x0004,
   FG_STARTH       = 0x0005,
   FG_STARTL       = 0x0006,
   FG_RAMP_CNT_LO  = 0x0007,
   FG_RAMP_CNT_HI  = 0x0008,
   FG_TAG_LOW      = 0x0009,
   FG_TAG_HIGH     = 0x000A,
   FG_VER          = 0x000B,

   FG1_IRQ         = (1<<15),
   FG2_IRQ         = (1<<14),
   DREQ            = (1<<4),

   WB_FG_CNTRL     = 0x0000,
   WB_FG_A         = 0x0001,
   WB_FG_B         = 0x0002,
   WB_FG_BROAD     = 0x0003,
   WB_FG_SHIFTA    = 0x0004,
   WB_FG_SHIFTB    = 0x0005,
   WB_FG_START     = 0x0006,
   WB_RAMP_CNT     = 0x0007,
   WB_FG_SW_DST    = 0x0008,

   TMR_CNTRL       = 0x0000,
   TMR_IRQ_CNT     = 0x0001,
   TMR_VALUEL      = 0x0002,
   TMR_VALUEH      = 0x0003,
   TMR_REPEAT      = 0x0004,

   GLOBAL_IRQ_ENA  = 0x0002,
   SRQ_ENA         = 0x0006,
   SRQ_ACT         = 0x0008,
   MULTI_SLAVE_SEL = 0x000C,
   MULTICAST_ACC   = 0x0008
} BUS_DEVICES_T;


/*!
 * @ingroup SCU_BUS
 * @brief Definition of SCU-bus slave system IDs.
 */
typedef enum
{
   SYS_LOEP =   3,
   SYS_CSCO =  55,
   SYS_PBRF =  42
} SLAVE_SYSTEM_T;

/*!
 * @ingroup SCU_BUS
 * @brief Definition of SCU-bus slave group IDs.
 */
typedef enum
{
   GRP_ADDAC1  =   3,  /*!<@brief Group ID ADDAC 1 */
   GRP_ADDAC2  =  38,  /*!<@brief Group ID ADDAC 2 */
   GRP_DIOB    =  26,
   GRP_FIB_DDS =   1,
   GRP_MFU     =   2,  /*!<@brief Group ID: ACU/MFU */
   GRP_SIO3    =  69,  /*!<@brief Group ID: SIO 3 */
   GRP_SIO2    =  23   /*!<@brief Group ID: SIO 2 */
} SLAVE_GROUP_T;

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Loop for scanning all scu-bus slots.
 * @param slot Variable of the current slot number of each loop iteration.
 *             The type is "unsigned int".
 */
#define SCU_BUS_FOR_EACH_SLOT( slot ) \
   for( unsigned int slot = SCUBUS_START_SLOT; slot <= MAX_SCU_SLAVES; slot++ )

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Loop- macro scans all SCU-bus slots and skips the iteration step
 *        if in the current slot not a slave which has defined in slotFlags.
 * @see   SCU_BUS_FOR_EACH_SLOT
 * @see   scuBusIsSlavePresent
 * @param slave Slave-number respectively slot number in which resides a slave.
 * @param slotFlags Flag-field of used slots. @see SCUBUS_SLAVE_FLAGS_T
 */      
#define SCU_BUS_FOR_EACH_SLAVE( slave, slotFlags )  \
   SCU_BUS_FOR_EACH_SLOT( slave )                   \
      if( scuBusIsSlavePresent( slotFlags, slave ) )
      
/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Calculates the relative address offset in bytes of a SCU bus slave
 *        from the given slot number.
 * @see MAX_SCU_SLAVES
 * @param slot Slot number, valid range 1 .. MAX_SCU_SLAVES (12)
 * @return Relative slot address
 */
STATIC inline unsigned int scuBusGetSlotOffset( const unsigned int slot )
{
   SCUBUS_ASSERT( slot >= SCUBUS_START_SLOT );
   SCUBUS_ASSERT( slot <= MAX_SCU_SLAVES );
   return slot * SCUBUS_SLAVE_ADDR_SPACE;
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Returns the slot-flag of the given SCU-bus slot number.
 * @param slot Slot number
 * @return Packed slave present bit belonging to the slot number.
 */
STATIC inline ALWAYS_INLINE
SCUBUS_SLAVE_FLAGS_T scuBusGetSlaveFlag( const unsigned int slot )
{
   SCUBUS_ASSERT( slot >= SCUBUS_START_SLOT );
   SCUBUS_ASSERT( slot <= MAX_SCU_SLAVES );

   return (1 << (slot-SCUBUS_START_SLOT));
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Extract a single slave-present-flag from the SCU-slave-flag-present
 *        field
 * @see scuBusFindSpecificSlaves
 * @see scuBusFindAllSlaves
 * @param flags packed slave present flags of all SCU bus slots
 * @param slot Slot number, valid range 1 .. MAX_SCU_SLAVES (12)
 * @retval true slave present
 * @retval false slave not present
 */
STATIC inline
bool scuBusIsSlavePresent( const SCUBUS_SLAVE_FLAGS_T flags,
                                                     const unsigned int slot )
{
   return ((flags & scuBusGetSlaveFlag( slot )) != 0);
}

#ifdef __cplusplus
} /* namespace Bus */
} /* namespace Scu */
} /* extern "C"    */
#endif

#endif /* ifndef _SCU_BUS_DEFINES_H */
/*================================= EOF =====================================*/
