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
#endif

/*!
 * @ingroup SCU_BUS
 * @brief Physical maximum number of SCU-Bus slots
 */
#ifndef MAX_SCU_SLAVES
  #define MAX_SCU_SLAVES    12
#endif

#if (MAX_SCU_SLAVES > 12)
  #error Maximum value of macro MAX_SCU_SLAVES has to be 12 !
#endif
#if (MAX_SCU_SLAVES < 1)
  #error Minimum value of macro MAX_SCU_SLAVES has to be at least 1 !
#endif

/*!
 * @ingroup SCU_BUS
 * @brief First slot of SCU-bus
 */
#define SCUBUS_START_SLOT  1

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

#if 0
/* Deprecated defines */
//#define CID_SYS           0x4
//#define CID_GROUP         0x5
#define SLAVE_VERSION     0x6
#define SLAVE_INT_ENA     0x21
#define SLAVE_INT_PEND    0x22
#define SLAVE_INT_ACT     0x24
#define SLAVE_EXT_CLK     0x30

#define SLAVE_INFO_TEXT   0x01c0 //!< @brief Zero terminated info-string max. 512 byte
#endif

#ifdef __cplusplus
namespace Bus
{
#endif

/*!
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
} BUS_BASE;

//#define DAC1_BASE         0x200
//#define DAC2_BASE         0x210
#define DAC_CNTRL         0x0
#define DAC_DATA          0x1

#define IO4x8             0x220
//#define ADC_BASE          0x230
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
//#define FG1_BASE          0x300
//#define FG2_BASE          0x340
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

//#define TMR_BASE          0x330
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

#define GRP_ADDAC1  3   /*!<@brief Group ID ADDAC 2 */
#define GRP_ADDAC2  38  /*!<@brief Group ID ADDAC 2 */
#define GRP_DIOB    26
#define GRP_FIB_DDS 1
#define GRP_MFU     2   /*!<@brief Group ID: ACU/MFU */
#define GRP_SIO3    69  /*!<@brief Group ID: SIO 2 */
#define GRP_SIO2    23  /*!<@brief Group ID: SIO 2 */

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
   SCUBUS_ASSERT( slot >= SCUBUS_START_SLOT );
   SCUBUS_ASSERT( slot <= MAX_SCU_SLAVES );

   return ((flags & (1 << (slot-SCUBUS_START_SLOT))) != 0);
}

#ifdef __cplusplus
} /* namespace Bus */
} /* namespace Scu */
} /* extern "C"    */
#endif

#endif /* ifndef _SCU_BUS_DEFINES_H */
/*================================= EOF =====================================*/
