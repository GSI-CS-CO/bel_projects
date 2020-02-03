/*!
 *  @file scu_main.c
 *  @brief Main module of SCU function generators in LM32.
 *
 *  @date 10.07.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *  Origin Stefan Rauch
 *  @todo File is going too huge, split it in several files.
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

#ifndef __DOCFSM__ /* Headers will not need for FSM analysator "docfsm" */
 #include <stack.h>
 #include "scu_main.h"
 #include "scu_eca_handler.h"
 #include "scu_command_handler.h"
 #include "scu_mil_fg_handler.h"
 #include "scu_temperature.h"
 #ifdef CONFIG_SCU_DAQ_INTEGRATION
  #include "daq_main.h"
 #endif
#endif // ifndef __DOCFSM__

#define INTERVAL_1000MS 1000000000ULL
#define INTERVAL_2000MS 2000000000ULL
#define INTERVAL_100MS  100000000ULL
#define INTERVAL_84MS   84000000ULL
#define INTERVAL_10MS   10000000ULL
#define INTERVAL_200US  200000ULL
#define INTERVAL_150US  150000ULL
#define INTERVAL_100US  100000ULL
#define INTERVAL_10US   10000ULL
#define INTERVAL_5MS    5000000ULL
#define ALWAYS          0ULL

typedef enum
{
   MIL_INL = 0x00,
   MIL_DRY = 0x01,
   MIL_DRQ = 0x02
} MIL_T;

extern ONE_WIRE_T g_oneWireBase;

/*====================== Begin of shared memory area ========================*/
/*!
 * @brief Memory space of shared memory for communication with Linux-host
 *        and initializing of them.
 */
SCU_SHARED_DATA_T SHARED g_shared = SCU_SHARED_DATA_INITIALIZER;
/*====================== End of shared memory area ==========================*/



/*!
 * @brief Base pointer of SCU bus.
 * @see initializeGlobalPointers
 */
volatile uint16_t*     g_pScub_base       = NULL;

/*!
 * @brief Base pointer of irq controller for SCU bus
 * @see initializeGlobalPointers
 */
volatile uint32_t*     g_pScub_irq_base   = NULL;

/*!
 * @brief Base pointer of MIL extension macro
 * @see initializeGlobalPointers
 */
volatile unsigned int* g_pScu_mil_base    = NULL;

/*!
 * @brief Base pointer of IRQ controller for dev bus extension
 * @see initializeGlobalPointers
 */
volatile uint32_t*     g_pMil_irq_base    = NULL;

/*!
 * @brief  Memory space of message queue.
 */
volatile FG_MESSAGE_BUFFER_T g_aMsg_buf[QUEUE_CNT] = {{0, 0}};

/*!
 * @brief Memory space of sent function generator data.
 */
FG_CHANNEL_T g_aFgChannels[MAX_FG_CHANNELS] = {{0,0}};//,0}};

/*===========================================================================*/
//#define CONFIG_DEBUG_FG_SIGNAL
/*! ---------------------------------------------------------------------------
 * @brief Send a signal back to the Linux-host (SAFTLIB)
 * @param sig Signal
 * @param channel Concerning channel number.
 */
STATIC inline void sendSignal( const SIGNAL_T sig, const unsigned int channel )
{
   *(volatile uint32_t*)(char*)
   (pCpuMsiBox + g_shared.fg_regs[channel].mbx_slot * sizeof(uint16_t)) = sig;
   hist_addx( HISTORY_XYZ_MODULE, signal2String( sig ), channel );
#ifdef CONFIG_DEBUG_FG_SIGNAL
   #warning CONFIG_DEBUG_FG_SIGNAL is defined this will destroy the timing!
   mprintf( ESC_DEBUG"Signal: %s, channel: %d sent\n"ESC_NORMAL,
            signal2String( sig ), channel );
#endif
}

/*! ---------------------------------------------------------------------------
 * @brief Initializing of all global pointers accessing the hardware.
 */
STATIC inline void initializeGlobalPointers( void )
{
   discoverPeriphery();
   uart_init_hw();
   initOneWire();
   /* additional periphery needed for scu */
   g_pScub_base       = (volatile uint16_t*)find_device_adr(GSI, SCU_BUS_MASTER);
   g_pScub_irq_base   = (volatile uint32_t*)find_device_adr(GSI, SCU_IRQ_CTRL);
   g_pScu_mil_base    = (unsigned int*)find_device_adr(GSI,SCU_MIL);
   g_pMil_irq_base    = (volatile uint32_t*)find_device_adr(GSI, MIL_IRQ_CTRL);
}

/*! ---------------------------------------------------------------------------
 * @brief Prints a error message happened in the device-bus respectively
 *        MIL bus.
 * @param status return status of the MIL-driver module.
 * @param slot Slot-number in the case the mil connection is established via
 *             SCU-Bus
 * @param msg String containing additional message text.
 */
STATIC void dev_failure( const int status, const int slot, const char* msg )
{
  static const char* pText = ESC_ERROR"dev bus access in slot ";
  char* pMessage;
  #define __MSG_ITEM( status ) case status: pMessage = #status; break
  switch( status )
  {
     __MSG_ITEM( OKAY );
     __MSG_ITEM( TRM_NOT_FREE );
     __MSG_ITEM( RCV_ERROR );
     __MSG_ITEM( RCV_TIMEOUT );
     __MSG_ITEM( RCV_TASK_ERR );
     default:
     {
        mprintf("%s%d failed with code %d"ESC_NORMAL"\n", pText, slot, status);
        return;
     }
  }
  #undef __MSG_ITEM
  mprintf("%s%d failed with message %s, %s"ESC_NORMAL"\n", pText, slot, pMessage, msg);
}

/*! ---------------------------------------------------------------------------
 */
STATIC void mil_failure( const int status, const int slave_nr )
{
   switch( status )
   {
      case RCV_PARITY:
      {
         mprintf(ESC_ERROR"parity error when reading task %d"ESC_NORMAL"\n", slave_nr );
         break;
      }
      case RCV_TIMEOUT:
      {
         mprintf(ESC_ERROR"timeout error when reading task %d"ESC_NORMAL"\n", slave_nr );
         break;
      }
      case RCV_ERROR:
      {
         mprintf(ESC_ERROR"unknown error when reading task %d"ESC_NORMAL"\n", slave_nr );
         break;
      }
   }
}

/*! ---------------------------------------------------------------------------
 * @brief enables msi generation for the specified channel. \n
 * Messages from the scu bus are send to the msi queue of this cpu with the offset 0x0. \n
 * Messages from the MIL extension are send to the msi queue of this cpu with the offset 0x20. \n
 * A hardware macro is used, which generates msis from legacy interrupts. \n
 * @param channel number of the channel between 0 and MAX_FG_CHANNELS-1
 * @see disable_slave_irq
 */
void enable_scub_msis( const unsigned int channel )
{
   if( channel >= MAX_FG_CHANNELS )
      return;

   //FG_ASSERT( pMyMsi != NULL );

   const uint8_t socket = getSocket( channel );

   if( isNonMilFg( socket ) || isMilScuBusFg( socket ) )
   {
      //SCU Bus Master
      FG_ASSERT( getFgSlotNumber( socket ) > 0 );
      const uint16_t slot = getFgSlotNumber( socket ) - 1;
      g_pScub_base[GLOBAL_IRQ_ENA] = 0x20;
      g_pScub_irq_base[8]  = slot;            // channel select
      g_pScub_irq_base[9]  = slot;            // msg: socket number
      g_pScub_irq_base[10] = (uint32_t)pMyMsi + 0x0;    // msi queue destination address of this cpu
      g_pScub_irq_base[2]  = (1 << slot); // enable slave
      //mprintf("IRQs for slave %d enabled.\n", (socket & SCU_BUS_SLOT_MASK));
      return;
   }

   if( !isMilExtentionFg( socket ) )
      return;

   g_pMil_irq_base[8]   = MIL_DRQ;
   g_pMil_irq_base[9]   = MIL_DRQ;
   g_pMil_irq_base[10]  = (uint32_t)pMyMsi + 0x20; //TODO Who the fuck is 0x20?!?
   g_pMil_irq_base[2]   = (1 << MIL_DRQ);
}

/*! ---------------------------------------------------------------------------
 * @brief disables the generation of irqs for the specified channel
 *  SIO and MIL extension stop generating irqs
 *  @param channel number of the channel from 0 to MAX_FG_CHANNELS-1
 * @see enable_scub_msis
 */
STATIC void disable_slave_irq( const unsigned int channel )
{
   if( channel >= MAX_FG_CHANNELS )
      return;

   int status;
   const uint8_t socket = getSocket( channel );
   const uint8_t dev    = getDevice( channel );

   if( isNonMilFg( socket ) )
   {
      if (dev == 0)
        g_pScub_base[OFFS(socket) + SLAVE_INT_ENA] &= ~(0x8000); //disable fg1 irq
      else if (dev == 1)
        g_pScub_base[OFFS(socket) + SLAVE_INT_ENA] &= ~(0x4000); //disable fg2 irq
   }
   else if( isMilExtentionFg( socket ) )
   {
      //write_mil(g_pScu_mil_base, 0x0, FC_COEFF_A_WR | dev);            //ack drq
      if( (status = write_mil(g_pScu_mil_base, 0x0, FC_IRQ_MSK | dev) ) != OKAY)
         dev_failure( status, getFgSlotNumber( socket ), __func__);  //mask drq
   }
   else if( isMilScuBusFg( socket ) )
   {
      if( (status = scub_write_mil(g_pScub_base, getFgSlotNumber( socket ),
                                   0x0, FC_IRQ_MSK | dev)) != OKAY)
         dev_failure( status, getFgSlotNumber( socket ), __func__);  //mask drq
   }

   //mprintf("IRQs for slave %d disabled.\n", socket);
}

/*! ---------------------------------------------------------------------------
 * @brief delay in multiples of one millisecond
 *  uses the system timer
 *  @param ms delay value in milliseconds
 */
STATIC void msDelayBig( const uint64_t ms )
{
   uint64_t later = getSysTime() + ms * 1000000ULL / 8;
   while(getSysTime() < later) {asm("# noop");}
}

#if 0
STATIC void msDelay(uint32_t msecs)
{
  usleep(1000 * msecs);
}
#endif

/*! ---------------------------------------------------------------------------
 */
STATIC inline unsigned int getFgNumberFromRegister( const uint16_t reg )
{
   return (reg >> 4) & 0x3F; // virtual fg number Bits 9..4
}

/*
 * Mil-library uses "short" rather than "uint16_t"! :-(
 */
STATIC_ASSERT( sizeof( short ) == sizeof( int16_t ) );

/*! ---------------------------------------------------------------------------
 * @brief Sends the parameters for the next interpolation interval.
 * @param socket number of the slot, including the high bits with the information SIO or MIL_EXT
 * @param fg_base base address of the function generator macro
 * @param cntrl_reg state of the control register. saves one read access.
 * @param pSetvalue Pointer in which shall the set-value copied used for MIL-daq.
 * @todo Remove naked mask numbers by well named constants or inline get()-functions.
 * @todo In the case of a periodical signal, check whether its really necessary to use
 *       a circular buffer respectively a FiFo in which the wishbone bus becomes to much traffic! \n
 *       May be its possible to store a full period in the DDR3 RAM.
 */
STATIC inline void send_fg_param( const unsigned int socket,
                                  const unsigned int fg_base,
                                  const uint16_t cntrl_reg,
                                  signed int* pSetvalue )
{
   FG_PARAM_SET_T pset;
   uint16_t       cntrl_reg_wr;
   int            status;
   int16_t        blk_data[MIL_BLOCK_SIZE];

   const unsigned int fg_num = getFgNumberFromRegister( cntrl_reg );
   if( fg_num >= ARRAY_SIZE( g_aFgChannels ) )
   {
      mprintf( ESC_ERROR"FG-number %d out of range!"ESC_NORMAL"\n", fg_num );
      return;
   }

   if( !cbRead( &g_shared.fg_buffer[0], &g_shared.fg_regs[0], fg_num, &pset ) )
   {
      hist_addx(HISTORY_XYZ_MODULE, "buffer empty, no parameter sent", socket);
      return;
   }

   cntrl_reg_wr = cntrl_reg & ~(0xfc07); // clear freq, step select, fg_running and fg_enabled
   cntrl_reg_wr |= ((pset.control & 0x38) << 10) | ((pset.control & 0x7) << 10);
   blk_data[0] = cntrl_reg_wr;
   blk_data[1] = pset.coeff_a;
   blk_data[2] = pset.coeff_b;
   blk_data[3] = (pset.control & 0x3ffc0) >> 6;     // shift a 17..12 shift b 11..6
   blk_data[4] = pset.coeff_c & 0xffff;
   blk_data[5] = (pset.coeff_c & 0xffff0000) >> BIT_SIZEOF(int16_t); // data written with high word

   if( isNonMilFg( socket ) )
   { /*
      * In this case the socket value is equal to the scu-bus slot number.
      */
      g_pScub_base[OFFS(socket) + fg_base + FG_CNTRL]  = blk_data[0];
      g_pScub_base[OFFS(socket) + fg_base + FG_A]      = blk_data[1];
      g_pScub_base[OFFS(socket) + fg_base + FG_B]      = blk_data[2];
      g_pScub_base[OFFS(socket) + fg_base + FG_SHIFT]  = blk_data[3];
      g_pScub_base[OFFS(socket) + fg_base + FG_STARTL] = blk_data[4];
      g_pScub_base[OFFS(socket) + fg_base + FG_STARTH] = blk_data[5];
      // no setvalue for scu bus daq 
      *pSetvalue = 0;
   }
   else if( isMilExtentionFg( socket ) )
   {
      // save coeff_c as setvalue
      *pSetvalue = pset.coeff_c;
      // transmit in one block transfer over the dev bus
      if((status = write_mil_blk(g_pScu_mil_base, &blk_data[0], FC_BLK_WR | fg_base)) != OKAY)
         dev_failure(status, 0, __func__);
      // still in block mode !
   }
   else if( isMilScuBusFg( socket ) )
   {  // save coeff_c as setvalue
      *pSetvalue = pset.coeff_c;
      // transmit in one block transfer over the dev bus
      if((status = scub_write_mil_blk( g_pScub_base,
                                       getFgSlotNumber( socket ),
                                       &blk_data[0],
                                       FC_BLK_WR | fg_base)) != OKAY)
      {
         dev_failure(status, getFgSlotNumber( socket ), __func__);
      }
      // still in block mode !
   }
   g_aFgChannels[fg_num].param_sent++;
}

/*! ---------------------------------------------------------------------------
 * @brief Send signal REFILL to the SAFTLIB when the fifo level has
 *        the threshold reached. Helper function of function handleMacros().
 * @see handleMacros
 * @param channel Channel of concerning function generator.
 */
STATIC void sendRefillSignalIfThreshold( const unsigned int channel )
{
   if( cbgetCount( &g_shared.fg_regs[0], channel ) == THRESHOLD )
   {
      //mprintf( "*" );
      sendSignal( IRQ_DAT_REFILL, channel );
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Helper function of function handleMacros().
 * @see handleMacros
 */
STATIC inline void makeStop( const unsigned int channel )
{
   sendSignal( cbisEmpty( &g_shared.fg_regs[0], channel )?
                                                      IRQ_DAT_STOP_EMPTY :
                                                      IRQ_DAT_STOP_NOT_EMPTY,
               channel );
   disable_slave_irq( channel );
   g_shared.fg_regs[channel].state = STATE_STOPPED;
}

/*! ---------------------------------------------------------------------------
 * @brief Helper function of function handleMacros().
 * @see handleMacros
 */
STATIC inline void makeStart( const unsigned int channel )
{
   g_shared.fg_regs[channel].state = STATE_ACTIVE;
   sendSignal( IRQ_DAT_START, channel ); // fg has received the tag or brc message
}

/*! ---------------------------------------------------------------------------
 * @see scu_main.h
 */
void handleMacros( const unsigned int socket,
                   const unsigned int fg_base,
                   const uint16_t irq_act_reg,
                   signed int* pSetvalue )
{
   uint16_t cntrl_reg = 0;
   unsigned int channel;

   if( isNonMilFg( socket ) )
   {
      cntrl_reg = g_pScub_base[OFFS(socket) + fg_base + FG_CNTRL];
      channel = getFgNumberFromRegister( cntrl_reg );
   }
   else
   {
      channel = getFgNumberFromRegister( irq_act_reg );
   }

   if( channel >= ARRAY_SIZE( g_shared.fg_regs ) )
   {
      mprintf( ESC_ERROR"%s: Channel out of range: %d\n"ESC_NORMAL, __func__, channel );
      return;
   }

   if( isNonMilFg( socket ) )
   {
      /* last cnt from from fg macro, read from LO address copies hardware counter to shadow reg */
      g_shared.fg_regs[channel].ramp_count = g_pScub_base[OFFS(socket) + fg_base + FG_RAMP_CNT_LO];
      g_shared.fg_regs[channel].ramp_count |= g_pScub_base[OFFS(socket) + fg_base + FG_RAMP_CNT_HI] << BIT_SIZEOF( uint16_t );

      if( (cntrl_reg & FG_RUNNING) == 0 )
      { // fg stopped
         makeStop( channel );
      }
      else
      {
         if( (cntrl_reg & FG_DREQ) == 0 )
         {
            makeStart( channel );
         }
         sendRefillSignalIfThreshold( channel );
         send_fg_param( socket, fg_base, cntrl_reg, pSetvalue );
      }
   }
   else /* isNonMilFg( socket ) */
   {
      /* count in software only */
      g_shared.fg_regs[channel].ramp_count++;
      if( (irq_act_reg  & FG_RUNNING) == 0 )
      {     // fg stopped
         g_shared.fg_regs[channel].ramp_count--;
         makeStop( channel );
      }
      else
      {
         if( (irq_act_reg & DEV_STATE_IRQ) != 0 )
         {
            makeStart( channel );
         }
         if( (irq_act_reg & (DEV_DRQ | DEV_STATE_IRQ)) != 0 )
         {
            sendRefillSignalIfThreshold( channel );
            send_fg_param( socket, fg_base, irq_act_reg, pSetvalue );
         }
      }
   } /* else of if isNonMilFg( socket ) */
}

/*! ---------------------------------------------------------------------------
 * @brief as short as possible, just pop the msi queue of the cpu and
 *         push it to the message queue of the main loop
 * @see init_irq_table
 * @see _irq_entry
 * @see irq_pop_msi
 * @see dispatch
 */
void irq_handler( void )
{
   MSI_T m;

  // send msi threadsafe to main loop
   m.msg = global_msi.msg;
   m.adr = global_msi.adr;
   add_msg( &g_aMsg_buf[0], IRQ, m );
}

/*! ---------------------------------------------------------------------------
 * @brief Move messages to the correct queue, depending on source
 * @see irq_handler
 * @see schedule
 */
STATIC inline void dispatch( void )
{
   const MSI_T m = remove_msg( &g_aMsg_buf[0], IRQ );
   switch( m.adr & 0xFF )
   { //TODO remove these naked numbers asap!
      case 0x00: add_msg( &g_aMsg_buf[0], SCUBUS, m ); return; // message from scu bus
      case 0x10: add_msg( &g_aMsg_buf[0], SWI,    m ); return; // software message from saftlib
      case 0x20: add_msg( &g_aMsg_buf[0], DEVBUS, m ); return; // message from dev bus
   }
}

/*! ---------------------------------------------------------------------------
 * @see scu_main.h
 */
void clear_handler_state( const uint8_t socket )
{
   MSI_T m;

   if( isMilScuBusFg( socket ) )
   {
      // create swi
      FG_ASSERT( getFgSlotNumber( socket ) > 0 );
      m.msg = getFgSlotNumber( socket ) - 1;
      m.adr = 0;
      irq_disable();
      add_msg( &g_aMsg_buf[0], DEVSIO, m );
      irq_enable();
      return;
   }

   if( isMilExtentionFg( socket ) )
   {
      m.msg = 0;
      m.adr = 0;
      irq_disable();
      add_msg(&g_aMsg_buf[0], DEVBUS, m );
      irq_enable();
   }
}

/*! ---------------------------------------------------------------------------
 * @see scu_main.h
 */
int configure_fg_macro( const unsigned int channel )
{
   if( channel >= MAX_FG_CHANNELS )
      return -1;

   #if !defined( CONFIG_GSI ) && !defined( __DOCFSM__ )
    #warning Maybe old Makefile is used, this could be erroneous in using local static variables!
   #endif
   static uint16_t s_clearIsActive = 0;
   STATIC_ASSERT( BIT_SIZEOF( s_clearIsActive ) >= (MAX_SCU_SLAVES + 1) );

   #define _SLOT_BIT_MASK() (1 << (getFgSlotNumber( socket ) - 1))
   #define _MIL_BIT_MASK()  (1 << MAX_SCU_SLAVES)

   uint16_t dreq_status = 0;
   const uint8_t socket = getSocket( channel );
   /* actions per slave card */
   if( isMilScuBusFg( socket ) )
   {
      scub_status_mil( g_pScub_base, getFgSlotNumber( socket ), &dreq_status );
   }
   else if( isMilExtentionFg( socket ) )
   {
      status_mil( g_pScu_mil_base, &dreq_status );
   }

   // if dreq is active
   if( (dreq_status & MIL_DATA_REQ_INTR) != 0 )
   {
      if( isMilScuBusFg( socket ) )
      {
         FG_ASSERT( getFgSlotNumber( socket ) > 0 );
         if( (s_clearIsActive & _SLOT_BIT_MASK()) == 0 )
         {
            s_clearIsActive |= _SLOT_BIT_MASK();
            clear_handler_state( socket );
            hist_addx( HISTORY_XYZ_MODULE, "clear_handler_state", socket );
         }
         return -1;
      }

      if( isMilExtentionFg( socket ) )
      {
         if( (s_clearIsActive & _MIL_BIT_MASK()) == 0 )
         {
            s_clearIsActive |= _MIL_BIT_MASK();
            clear_handler_state( socket );
            hist_addx( HISTORY_XYZ_MODULE, "clear_handler_state", socket );
         }
         return -1;
      }
   }
   else
   {  // reset clear
      if( isMilScuBusFg( socket ) )
      {
         FG_ASSERT( getFgSlotNumber( socket ) > 0 );
         s_clearIsActive &= ~_SLOT_BIT_MASK();
      }
      else if( isMilExtentionFg( socket ) )
      {
         s_clearIsActive &= ~_MIL_BIT_MASK();
      }
   }

   #undef _MIL_BIT_MASK

   int status;
   const uint8_t dev  = getDevice( channel );
    /* enable irqs */
   if( isNonMilFg( socket ) )
   {                                      //scu bus slave
      g_pScub_base[SRQ_ENA] |= (1 << (socket-1));           // enable irqs for the slave
      g_pScub_base[OFFS(socket) + SLAVE_INT_ACT] =  (FG1_IRQ | FG2_IRQ); // clear all irqs
      g_pScub_base[OFFS(socket) + SLAVE_INT_ENA] |= (FG1_IRQ | FG2_IRQ); // enable fg1 and fg2 irq
   }
   else if( isMilExtentionFg( socket ) )
   {
      if( (status = write_mil(g_pScu_mil_base, 1 << 13, FC_IRQ_MSK | dev)) != OKAY)
         dev_failure( status, 0, "enable dreq" ); //enable Data-Request
   }
   else if( isMilScuBusFg( socket ) )
   {
      FG_ASSERT( getFgSlotNumber( socket ) > 0 );
      g_pScub_base[SRQ_ENA] |= _SLOT_BIT_MASK();        // enable irqs for the slave
      g_pScub_base[OFFS(getFgSlotNumber( socket )) + SLAVE_INT_ENA] = DREQ; // enable receiving of drq
      if( (status = scub_write_mil(g_pScub_base, getFgSlotNumber( socket ), 1 << 13, FC_IRQ_MSK | dev)) != OKAY)
         dev_failure( status, getFgSlotNumber( socket ), "enable dreq"); //enable sending of drq
   }
   #undef _SLOT_BIT_MASK

   unsigned int fg_base = 0;
   /* fg mode and reset */
   if( isNonMilFg( socket ) )
   {   //scu bus slave
      unsigned int dac_base;
      switch( dev )
      {
         case 0:
         {
            fg_base = FG1_BASE;
            dac_base = DAC1_BASE;
            break;
         }
         case 1:
         {
            fg_base = FG2_BASE;
            dac_base = DAC2_BASE;
            break;
         }
         default: return -1;
      }
      g_pScub_base[OFFS(socket) + dac_base + DAC_CNTRL] = 0x10;   // set FG mode
      g_pScub_base[OFFS(socket) + fg_base + FG_RAMP_CNT_LO] = 0;  // reset ramp counter
   }
   else if( isMilExtentionFg( socket ) )
   {
      if( (status = write_mil(g_pScu_mil_base, 0x1, FC_IFAMODE_WR | dev)) != OKAY)
         dev_failure( status, 0, "set FG mode"); // set FG mode
   }
   else if( isMilScuBusFg( socket ) )
   {
      if( (status = scub_write_mil(g_pScub_base, getFgSlotNumber( socket ), 0x1, FC_IFAMODE_WR | dev)) != OKAY)
         dev_failure( status, getFgSlotNumber( socket ), "set FG mode"); // set FG mode
   }

   uint16_t cntrl_reg_wr;
   int16_t blk_data[MIL_BLOCK_SIZE];
   FG_PARAM_SET_T pset;
    //fetch first parameter set from buffer
   if( cbRead(&g_shared.fg_buffer[0], &g_shared.fg_regs[0], channel, &pset) != 0 )
   {
      cntrl_reg_wr = ((pset.control & 0x38) << 10) | ((pset.control & 0x7) << 10) | channel << 4;
      blk_data[0] = cntrl_reg_wr;
      blk_data[1] = pset.coeff_a;
      blk_data[2] = pset.coeff_b;
      blk_data[3] = (pset.control & 0x3ffc0) >> 6;     // shift a 17..12 shift b 11..6
      blk_data[4] = pset.coeff_c & 0xffff;
      blk_data[5] = (pset.coeff_c & 0xffff0000) >> BIT_SIZEOF(uint16_t);; // data written with high word

      if( isNonMilFg( socket ) )
      {
        //set virtual fg number Bit 9..4
         FG_ASSERT( fg_base != 0 );
         g_pScub_base[OFFS(socket) + fg_base + FG_CNTRL]  = blk_data[0];
         g_pScub_base[OFFS(socket) + fg_base + FG_A]      = blk_data[1];
         g_pScub_base[OFFS(socket) + fg_base + FG_B]      = blk_data[2];
         g_pScub_base[OFFS(socket) + fg_base + FG_SHIFT]  = blk_data[3];
         g_pScub_base[OFFS(socket) + fg_base + FG_STARTL] = blk_data[4];
         g_pScub_base[OFFS(socket) + fg_base + FG_STARTH] = blk_data[5];
      }
      else if( isMilExtentionFg( socket ) )
      {
        // save the coeff_c for mil daq
         g_aFgChannels[channel].last_c_coeff = pset.coeff_c;
        // transmit in one block transfer over the dev bus
         if((status = write_mil_blk(g_pScu_mil_base, &blk_data[0], FC_BLK_WR | dev)) != OKAY)
            dev_failure( status, 0, "blk trm");
        // still in block mode !
         if((status = write_mil(g_pScu_mil_base, cntrl_reg_wr, FC_CNTRL_WR | dev)) != OKAY)
            dev_failure( status, 0, "end blk trm");
      }
      else if( isMilScuBusFg( socket ) )
      {
         // save the coeff_c for mil daq
         g_aFgChannels[channel].last_c_coeff = pset.coeff_c;
         // transmit in one block transfer over the dev bus
         if( (status = scub_write_mil_blk(g_pScub_base, getFgSlotNumber( socket ), &blk_data[0], FC_BLK_WR | dev)) != OKAY)
            dev_failure( status, getFgSlotNumber( socket ), "blk trm");
         // still in block mode !
         if( (status = scub_write_mil(g_pScub_base, getFgSlotNumber( socket ), cntrl_reg_wr, FC_CNTRL_WR | dev)) != OKAY)
            dev_failure( status, getFgSlotNumber( socket ), "end blk trm");
      }
      g_aFgChannels[0].param_sent++;
  //!! }CONFIG_GOTO_STWAIT_WHEN_TIMEOUT

   /* configure and enable macro */
   if( isNonMilFg( socket ) )
   {
      g_pScub_base[OFFS(socket) + fg_base + FG_TAG_LOW] = g_shared.fg_regs[channel].tag & 0xffff;
      g_pScub_base[OFFS(socket) + fg_base + FG_TAG_HIGH] = g_shared.fg_regs[channel].tag >> BIT_SIZEOF(uint16_t);
      g_pScub_base[OFFS(socket) + fg_base + FG_CNTRL] |= FG_ENABLED;
   }
   else if( isMilExtentionFg( socket ) )
   { // enable and end block mode
      if( (status = write_mil(g_pScu_mil_base, cntrl_reg_wr | FG_ENABLED, FC_CNTRL_WR | dev)) != OKAY)
         dev_failure( status, 0, "end blk mode");
   }
   else if( isMilScuBusFg( socket ) )
   { // enable and end block mode
      if( (status = scub_write_mil( g_pScub_base, getFgSlotNumber( socket ),
           cntrl_reg_wr | FG_ENABLED, FC_CNTRL_WR | dev ) ) != OKAY )
         dev_failure( status, getFgSlotNumber( socket ), "end blk mode");
   }
   } //!!
   // reset watchdog
 //  g_aFgChannels[channel].timeout = 0;
   g_shared.fg_regs[channel].state = STATE_ARMED;
   sendSignal( IRQ_DAT_ARMED, channel );
   return 0;
}

/*! ---------------------------------------------------------------------------
 * @brief Prints all found function generators.
 */
STATIC inline void printFgs( void )
{
   for( unsigned int i = 0; i < ARRAY_SIZE( g_shared.fg_macros ); i++ )
   {
      if( g_shared.fg_macros[i].outputBits == 0 )
         break;
      mprintf( "fg-%d-%d\tver: %d output-bits: %d\n",
               g_shared.fg_macros[i].socket,
               g_shared.fg_macros[i].device,
               g_shared.fg_macros[i].version,
               g_shared.fg_macros[i].outputBits
             );
   }
}


/*! ---------------------------------------------------------------------------
 * @brief Print the values and states of all channel registers.
 */
inline STATIC void print_regs( void)
{
   for( unsigned int i = 0; i < ARRAY_SIZE( g_shared.fg_regs ); i++ )
   {
      mprintf("channel[%d].wr_ptr %d\n", i, g_shared.fg_regs[i].wr_ptr);
      mprintf("channel[%d].rd_ptr %d\n", i, g_shared.fg_regs[i].rd_ptr);
      mprintf("channel[%d].mbx_slot 0x%x\n", i, g_shared.fg_regs[i].mbx_slot);
      mprintf("channel[%d].macro_number %d\n", i, g_shared.fg_regs[i].macro_number);
      mprintf("channel[%d].ramp_count %d\n", i, g_shared.fg_regs[i].ramp_count);
      mprintf("channel[%d].tag 0x%x\n", i, g_shared.fg_regs[i].tag);
      mprintf("channel[%d].state %d\n", i, g_shared.fg_regs[i].state);
      mprintf("\n");
   }
}

/*! ---------------------------------------------------------------------------
 * @see scu_main.h
 */
void disable_channel( const unsigned int channel )
{
   FG_CHANNEL_REG_T* pFgRegs = &g_shared.fg_regs[channel];

   if( pFgRegs->macro_number == SCU_INVALID_VALUE )
      return;

   int status;
   int16_t data;
   const uint8_t socket = getSocket( channel );
   const uint16_t dev   = getDevice( channel );
   //mprintf("disarmed socket %d dev %d in channel[%d] state %d\n", socket, dev, channel, pFgRegs->state); //ONLY FOR TESTING
   if( isNonMilFg( socket ) )
   {
      unsigned int fg_base, dac_base;
      /* which macro are we? */
      switch( dev )
      {
         case 0:
         {
            fg_base = FG1_BASE;
            dac_base = DAC1_BASE;
            break;
         }
         case 1:
         {
            fg_base = FG2_BASE;
            dac_base = DAC2_BASE;
            break;
         }
         default: return;
      }

     // disarm hardware
      g_pScub_base[OFFS(socket) + fg_base + FG_CNTRL] &= ~(0x2);
      g_pScub_base[OFFS(socket) + dac_base + DAC_CNTRL] &= ~(0x10); // unset FG mode
   }
   else if( isMilExtentionFg( socket ) )
   {  // disarm hardware
      if( (status = read_mil( g_pScu_mil_base, &data, FC_CNTRL_RD | dev)) != OKAY )
         dev_failure( status, 0, "disarm hw 1" );

      if( (status = write_mil( g_pScu_mil_base, data & ~(0x2), FC_CNTRL_WR | dev)) != OKAY )
         dev_failure( status, 0, "disarm hw 2" );
   }
   else if( isMilScuBusFg( socket ) )
   {  // disarm hardware
      if( (status = scub_read_mil( g_pScub_base, getFgSlotNumber( socket ),
           &data, FC_CNTRL_RD | dev)) != OKAY )
         dev_failure( status, getFgSlotNumber( socket ), "disarm hw 3" );

      if( (status = scub_write_mil( g_pScub_base, getFgSlotNumber( socket ),
           data & ~(0x2), FC_CNTRL_WR | dev)) != OKAY )
         dev_failure( status, getFgSlotNumber( socket ), "disarm hw 4" );
   }

   if( pFgRegs->state == STATE_ACTIVE )
   {    // hw is running
      hist_addx( HISTORY_XYZ_MODULE, "flush circular buffer", channel );
      pFgRegs->rd_ptr =  pFgRegs->wr_ptr;
   }
   else
   {
      pFgRegs->state = STATE_STOPPED;
      sendSignal( IRQ_DAT_DISARMED, channel );
   }
}

/*! ---------------------------------------------------------------------------
 * @brief initialize the irq table and set the irq mask
 */
STATIC void init_irq_table( void )
{
   isr_table_clr();
   isr_ptr_table[0] = &irq_handler;
   irq_set_mask(0x01);
   g_aMsg_buf[IRQ].ring_head = g_aMsg_buf[IRQ].ring_tail; // clear msg buffer
   irq_enable();
   mprintf("IRQ table configured. 0x%x\n", irq_get_mask());
}

/*! ---------------------------------------------------------------------------
 * @brief initialize procedure at startup
 */
STATIC void init( void )
{
   hist_init(HISTORY_XYZ_MODULE);
   for( int i = 0; i < ARRAY_SIZE(g_shared.fg_regs); i++ )
      g_shared.fg_regs[i].macro_number = SCU_INVALID_VALUE;     //no macros assigned to channels at startup
   updateTemperature();                       //update 1Wire ID and temperatures
   scanFgs();                        //scans for slave cards and fgs
}

/*! ---------------------------------------------------------------------------
 * @brief segfault handler, not used at the moment
 */
void _segfault( void )
{
   mprintf(ESC_ERROR"KABOOM!"ESC_NORMAL"\n");
  //while (1) {}
}


/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Mecro declares a state of a Finite-State-Machine. \n
 *        Helper macro for documenting the FSM by the FSM-visualizer DOCFSM.
 * @see FG_STATE_T
 * @see https://github.com/UlrichBecker/DocFsm
 */
#define FSM_DECLARE_STATE( state, attr... ) state

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Macro performs a FSM transition. \n
 *        Helper macro for documenting the FSM by the FSM-visualizer DOCFSM.
 * @see milDeviceHandler
 * @see https://github.com/UlrichBecker/DocFsm
 */
#define FSM_TRANSITION( newState, attr... ) pMilData->state = newState

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Initializer for Finite-State-Machines. \n
 *        Helper macro for documenting the FSM by the FSM-visualizer DOCFSM.
 * @see milDeviceHandler
 * @see https://github.com/UlrichBecker/DocFsm
 */
#define FSM_INIT_FSM( init, attr... )       pMilData->state = init

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Declaration of the states of the task- FSM
 * @see https://github.com/UlrichBecker/DocFsm
 */
typedef enum
{
   FSM_DECLARE_STATE( ST_WAIT,            label='Wait for message', color=blue ),
   FSM_DECLARE_STATE( ST_PREPARE,         label='Request MIL-IRQ-flags\nclear old IRQ-flags', color=cyan ),
   FSM_DECLARE_STATE( ST_FETCH_STATUS,    label='Read MIL-IRQ-flags', color=green ),
   FSM_DECLARE_STATE( ST_HANDLE_IRQS,     label='Send data to\nfunction\nif IRQ-flag set', color=red ),
   FSM_DECLARE_STATE( ST_DATA_AQUISITION, label='Request MIL-DAQ data\nif IRQ-flag set', color=cyan ),
   FSM_DECLARE_STATE( ST_FETCH_DATA,      label='Read MIL-DAQ data\nif IRQ-flag set',color=green )
} FG_STATE_T;

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Converts the states of the FSM in strings.
 * @note For debug purposes only!
 */
STATIC const char* state2string( const FG_STATE_T state )
{
   #define __CASE_RETURN( s ) case s: return #s
   switch( state )
   {
      __CASE_RETURN( ST_WAIT );
      __CASE_RETURN( ST_PREPARE );
      __CASE_RETURN( ST_FETCH_STATUS );
      __CASE_RETURN( ST_HANDLE_IRQS );
      __CASE_RETURN( ST_DATA_AQUISITION );
      __CASE_RETURN( ST_FETCH_DATA );
   }
   return "unknown";
   #undef __CASE_RETURN
}

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief
 */
typedef struct
{
   int16_t   irq_data;      /*!<@brief saved irq state */
   int       setvalue;      /*!<@brief setvalue from the tuple sent */
   uint64_t  daq_timestamp; /*!<@brief timestamp of daq sampling */
} FG_CHANNEL_TASK_T;

/*! --------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Task data type for MIL-FGs and MIL-DAQs
 * @see TASK_T
 * @see milDeviceHandler
 */
typedef struct
{
   FG_STATE_T        state;            /*!<@brief current FSM state */
   unsigned int      slave_nr;         /*!<@brief slave nr of the controlling sio card
                                                  its the SCU-slot number when > 0,
                                                  in the case of zero the MIL-extention */
   unsigned int      lastChannel;      /*!<@brief loop index for channel */
   unsigned int      task_timeout_cnt; /*!<@brief timeout counter */
   uint64_t          timestamp1;       /*!<@brief timestamp */
#ifdef CONFIG_READ_MIL_TIME_GAP
   uint64_t          gapReadingTime; // Workaround!!! Move this in FG_CHANNEL_T resp. g_aFgChannels!!!
#endif
   FG_CHANNEL_TASK_T aFgChannels[ARRAY_SIZE(g_aFgChannels)]; /*!<@see FG_CHANNEL_TASK_T */
} MIL_TASK_DATA_T;

/*!
 * @brief Slot-value when no slave selected yet.
 */
#define INVALID_SLAVE_NR ((unsigned int)~0)

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Initializer of a single MIL task
 */
#ifdef CONFIG_READ_MIL_TIME_GAP
  #define MIL_TASK_DATA_ITEM_INITIALIZER { ST_WAIT, INVALID_SLAVE_NR, \
                                           0, 0, 0, 0, {{0, 0, 0}} }
#else
  #define MIL_TASK_DATA_ITEM_INITIALIZER { ST_WAIT, INVALID_SLAVE_NR, \
                                           0, 0, 0, {{0, 0, 0}} }
#endif
/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Memory space and pre-initializing of MIL-task data.
 */
STATIC MIL_TASK_DATA_T g_aMilTaskData[5] =
{
   MIL_TASK_DATA_ITEM_INITIALIZER,
   MIL_TASK_DATA_ITEM_INITIALIZER,
   MIL_TASK_DATA_ITEM_INITIALIZER,
   MIL_TASK_DATA_ITEM_INITIALIZER,
   MIL_TASK_DATA_ITEM_INITIALIZER
};

STATIC_ASSERT( TASKMAX >= (ARRAY_SIZE( g_aMilTaskData ) + MAX_FG_CHANNELS-1 + TASKMIN));

#if defined( CONFIG_READ_MIL_TIME_GAP ) && !defined(__DOCFSM__)
/*! ---------------------------------------------------------------------------
 * @see scu_main.h
 */
bool isMilFsmInST_WAIT( void )
{
   for( unsigned int i = 0; i < ARRAY_SIZE( g_aMilTaskData ); i++ )
   {
      if( g_aMilTaskData[i].state != ST_WAIT )
         return false;
   }
   return true;
}

/*! ---------------------------------------------------------------------------
 * @see scu_main.h
 */
void suspendGapReading( void )
{
   for( unsigned int i = 0; i < ARRAY_SIZE( g_aMilTaskData ); i++ )
      g_aMilTaskData[i].slave_nr = INVALID_SLAVE_NR;
}
#endif // if defined( CONFIG_READ_MIL_TIME_GAP ) && !defined(__DOCFSM__)

/*! ---------------------------------------------------------------------------
 * @brief Scans for fgs on mil extension and scu bus.
 */
void scanFgs( void )
{
#ifdef CONFIG_USE_RESCAN_FLAG
   g_shared.fg_rescan_busy = 1; //signal busy to saftlib
#endif
#ifdef CONFIG_READ_MIL_TIME_GAP
   suspendGapReading();
#endif
#if __GNUC__ >= 9
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif
   scan_all_fgs( g_pScub_base,
                 g_pScu_mil_base,
                 &g_shared.fg_macros[0],
                 &g_shared.ext_id );
#if __GNUC__ >= 9
  #pragma GCC diagnostic pop
#endif
#ifdef CONFIG_USE_RESCAN_FLAG
   g_shared.fg_rescan_busy = 0; //signal done to saftlib
#endif
   printFgs();
}



/* task prototypes */
#ifndef __DOXYGEN__
static void dev_sio_handler( register TASK_T* );
static void dev_bus_handler( register TASK_T* );
static void scu_bus_handler( register TASK_T* FG_UNUSED );

#ifdef CONFIG_SCU_DAQ_INTEGRATION
static void scuBusDaqTask( register TASK_T* FG_UNUSED );
#endif
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @brief Task configuration table.
 * @see schedule
 */
STATIC TASK_T g_aTasks[] =
{
#ifdef CONFIG_SCU_DAQ_INTEGRATION
   { NULL,               ALWAYS, 0, scuBusDaqTask   },
#endif
   { &g_aMilTaskData[0], ALWAYS, 0, dev_sio_handler }, // sio task 1
 //!!  { &g_aMilTaskData[1], ALWAYS, 0, dev_sio_handler }, // sio task 2
 //!!  { &g_aMilTaskData[2], ALWAYS, 0, dev_sio_handler }, // sio task 3
 //!!  { &g_aMilTaskData[3], ALWAYS, 0, dev_sio_handler }, // sio task 4
   { &g_aMilTaskData[4], ALWAYS, 0, dev_bus_handler },
   { NULL,               ALWAYS, 0, scu_bus_handler },
   { NULL,               ALWAYS, 0, ecaHandler      },
   { NULL,               ALWAYS, 0, sw_irq_handler  }
};

/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @brief Scheduler for all SCU-tasks defined in g_aTasks. \n
 *        Performing of a cooperative multitasking.
 * @see TASK_T
 * @see g_aTasks
 * @see dev_sio_handler
 * @see dev_bus_handler
 * @see scu_bus_handler
 * @see ecaHandler
 * @see sw_irq_handler
 * @see scuBusDaqTask
 */
STATIC inline void schedule( void )
{
   const uint64_t tick = getSysTime();

   for( unsigned int i = 0; i < ARRAY_SIZE( g_aTasks ); i++ )
   {
      // call the dispatch task before every other task
      dispatch();
      TASK_T* pCurrent = &g_aTasks[i];
      if( (tick - pCurrent->lasttick) < pCurrent->interval )
      {
         continue;
      }
      pCurrent->func( pCurrent );
      pCurrent->lasttick = tick;
   }
}

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Returns the task-id-number of the given MIL-task data object.
 * @param pMilTaskData Pointer to the currently MIL-task-data object.
 */
inline STATIC unsigned int getMilTaskId( const MIL_TASK_DATA_T* pMilTaskData )
{
   return (((unsigned int)pMilTaskData) - ((unsigned int)g_aMilTaskData))
                                             / sizeof( MIL_TASK_DATA_T );
}

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Returns the task number of a mil device.
 * @param pMilTaskData Pointer to the currently running system task.
 * @param channel Channel number
 */
inline STATIC unsigned char getMilTaskNumber( const MIL_TASK_DATA_T* pMilTaskData,
                                              const unsigned int channel )
{
   //!!return TASKMIN + getFgMacroIndexFromFgRegister( channel );
   return TASKMIN + channel + getMilTaskId( pMilTaskData );
}

#ifdef CONFIG_SCU_DAQ_INTEGRATION
/*! ---------------------------------------------------------------------------
 * @ingroup DAQ
 * @ingroup TASK
 * @brief Handles all detected non-MIL DAQs
 * @see schedule
 */
STATIC void scuBusDaqTask( register TASK_T* pThis FG_UNUSED )
{
   FG_ASSERT( pThis->pTaskData == NULL );
   forEachScuDaqDevice();
}
#endif /* ifdef CONFIG_SCU_DAQ_INTEGRATION */

/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @brief task definition of scu_bus_handler
 * called by the scheduler in the main loop
 * decides which action for a scu bus interrupt is suitable
 * @param pThis pointer to the current task object (not used)
 * @see schedule
 */
STATIC void scu_bus_handler( register TASK_T* pThis FG_UNUSED )
{
   FG_ASSERT( pThis->pTaskData == NULL );

   if( !has_msg(&g_aMsg_buf[0], SCUBUS) )
      return;

   const MSI_T m = remove_msg(&g_aMsg_buf[0], SCUBUS);
   if( m.adr != 0x0 )
      return;

   const uint32_t slave_nr = m.msg + 1;
   if( slave_nr > MAX_SCU_SLAVES )
   {
      mprintf( ESC_ERROR"slave nr unknown.\n"ESC_NORMAL );
      return;
   }

   const uint16_t slv_int_act_reg = g_pScub_base[OFFS(slave_nr) + SLAVE_INT_ACT];
   uint16_t       slave_acks      = 0;

   if( (slv_int_act_reg & 0x1) != 0 )
   {// powerup interrupt
      slave_acks |= 0x1;
   }

   int dummy;
   if( (slv_int_act_reg & FG1_IRQ) != 0 )
   { //FG irq?
      handleMacros( slave_nr, FG1_BASE, 0, &dummy );
      slave_acks |= FG1_IRQ;
   }

   if( (slv_int_act_reg & FG2_IRQ) != 0 )
   { //FG irq?
      handleMacros( slave_nr, FG2_BASE, 0, &dummy );
      slave_acks |= FG2_IRQ;
   }

   if( (slv_int_act_reg & DREQ) != 0 )
   { //DRQ irq?
      add_msg(&g_aMsg_buf[0], DEVSIO, m);
      slave_acks |= DREQ;
   }

   g_pScub_base[OFFS(slave_nr) + SLAVE_INT_ACT] = slave_acks; // ack all pending irqs
}

#define CONFIG_LAGE_TIME_DETECT

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @ingroup TASK
 * @brief Writes the data set coming from one of the MIL-DAQs in the
 *        ring-buffer.
 * @param channel DAQ-channel where the data come from.
 * @param timestamp White-Rabbit time-stamp.
 * @param actValue Actual value.
 * @param setValue Set-value.
 * @param setValueInvalid If true, the set value is invalid.
 * @todo Storing the MIL-DAQ data in the DDR3-RAM instead wasting of
 *       shared memory.
 */
STATIC void pushDaqData( const FG_MACRO_T fgMacro, const uint64_t timestamp,
                         const uint16_t actValue, const uint32_t setValue
                      #ifdef CONFIG_READ_MIL_TIME_GAP
                         , const bool setValueInvalid
                      #endif
                       )
{
#ifdef CONFIG_LAGE_TIME_DETECT
   static uint64_t lastTime = 0;
   static unsigned int count = 0;
   if( lastTime > 0 )
   {
      if( (timestamp - lastTime) > 100000000ULL )
         mprintf( ESC_WARNING"Time-gap: %d"ESC_NORMAL"\n", count++ );
   }
   lastTime = timestamp;
#endif

#ifdef CONFIG_MIL_DAQ_USE_RAM
#error Extern RAM for MIL-DAQ not implemented yet!
   MIL_DAQ_RAM_ITEM_PAYLOAD_T pl;
   pl.item.timestamp = timestamp;
   pl.item.setValue = setValue >> (BIT_SIZEOF(uint32_t)/sizeof(MIL_DAQ_T));
   pl.item.actValue = actValue;
   pl.item.fgMacro = fgMacro;
 #ifdef CONFIG_READ_MIL_TIME_GAP
   pl.item.fgMacro.outputBits |= SET_VALUE_NOT_VALID_MASK;
 #endif
#else
   MIL_DAQ_OBJ_T d;

   d.actvalue = actValue;
   d.tmstmp_l = timestamp & 0xffffffff;
   d.tmstmp_h = timestamp >> BIT_SIZEOF(uint32_t);
   d.fgMacro  = fgMacro;
 #ifdef CONFIG_READ_MIL_TIME_GAP
   if( setValueInvalid )
      d.fgMacro.outputBits |= SET_VALUE_NOT_VALID_MASK;
 #endif
   d.setvalue = setValue;
#if 0
   #ifdef CONFIG_READ_MIL_TIME_GAP
   #warning This will make a race-condition in the currently MIL-DAQ-circular buffer!
   #endif
   if( isMilDaqBufferFull( &g_shared.daq_buf ) )
      removeOldestItem( &g_shared.daq_buf );
#endif
   add_daq_msg( &g_shared.daq_buf, d );
#endif
   hist_addx(HISTORY_XYZ_MODULE, "daq_high", actValue >> BIT_SIZEOF(uint8_t));
   hist_addx(HISTORY_XYZ_MODULE, "daq_low", actValue & 0xff);
}

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Helper function printing a timeout message.
 */
STATIC void printTimeoutMessage( register MIL_TASK_DATA_T* pMilTaskData,
                                 const bool isScuBus )
{
   mprintf( ESC_WARNING"timeout %s: state %s, taskid %d index %d"ESC_NORMAL"\n",
            isScuBus? "dev_bus_handler" : "dev_sio_handler",
            state2string( pMilTaskData->state ),
            getMilTaskId( pMilTaskData ),
            pMilTaskData->lastChannel );
}

/*
 * A little bit of paranoia doesn't hurt too much. ;-)
 */
STATIC_ASSERT( MAX_FG_CHANNELS == ARRAY_SIZE( g_aMilTaskData[0].aFgChannels ) );
STATIC_ASSERT( MAX_FG_CHANNELS == ARRAY_SIZE( g_aFgChannels ));

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief function returns true when no interrupt of the given channel
 *        is pending.
 *
 * Helper-function for function milDeviceHandler
 * @param pMilTaskData pointer to the current MIL-task-data object
 * @param channel channel number of function generator
 * @retval true No interrupt pending
 * @retval false Any interrupt pending
 * @see milDeviceHandler
 * @see milReqestStatus milGetStatus
 */
ALWAYS_INLINE STATIC inline
bool isNoIrqPending( register const MIL_TASK_DATA_T* pMilTaskData,
                                                 const unsigned int channel )
{
   return
   (pMilTaskData->aFgChannels[channel].irq_data & (DEV_STATE_IRQ | DEV_DRQ)) == 0;
}

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Requests the current status of the MIL device.
 * @param pMilTaskData pointer to the current MIL-task-data object
 * @param isScuBus if true via SCU bus MIL adapter
 * @param channel channel number of function generator
 * @return MIL-device status
 * @see milDeviceHandler
 */
STATIC inline
int milReqestStatus( register MIL_TASK_DATA_T* pMilTaskData, const bool isScuBus,
                                                  const unsigned int channel )
{
   FG_ASSERT( pMilTaskData->slave_nr != INVALID_SLAVE_NR );
   const uint8_t      socket     = getSocket( channel );
   const unsigned int devAndMode = getDevice( channel ) | FC_IRQ_ACT_RD;
   const unsigned char milTaskNo = getMilTaskNumber( pMilTaskData, channel );
   pMilTaskData->aFgChannels[channel].irq_data = 0; // clear old irq data
   /* test only if as connected to sio */
   if( isScuBus )
   {
      if( getFgSlotNumber( socket ) != pMilTaskData->slave_nr )
         return OKAY;

      if( !isMilScuBusFg( socket ) )
         return OKAY;

      return scub_set_task_mil( g_pScub_base, pMilTaskData->slave_nr,
                                                    milTaskNo, devAndMode );
   }

   if( !isMilExtentionFg( socket ) )
       return OKAY;

   return set_task_mil( g_pScu_mil_base, milTaskNo, devAndMode );
}

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Reads the currently status of the MIL device back.
 * @param pMilTaskData pointer to the current MIL-task-data object
 * @param isScuBus if true via SCU bus MIL adapter
 * @param channel channel number of function generator
 * @return MIL-device status
 * @see milDeviceHandler
 */
STATIC inline
int milGetStatus( register MIL_TASK_DATA_T* pMilTaskData, const bool isScuBus,
                                                  const unsigned int channel )
{
   FG_ASSERT( pMilTaskData->slave_nr != INVALID_SLAVE_NR );
   const uint8_t socket = getSocket( channel );
   const unsigned char milTaskNo = getMilTaskNumber( pMilTaskData, channel );
    /* test only if as connected to sio */
   if( isScuBus )
   {
      if( getFgSlotNumber( socket ) != pMilTaskData->slave_nr )
         return OKAY;
      if( !isMilScuBusFg( socket ) )
         return OKAY;

      return scub_get_task_mil( g_pScub_base, pMilTaskData->slave_nr,  milTaskNo,
                                &pMilTaskData->aFgChannels[channel].irq_data );
   }

   if( !isMilExtentionFg( socket ) )
      return OKAY;

   return get_task_mil( g_pScu_mil_base, milTaskNo,
                                &pMilTaskData->aFgChannels[channel].irq_data );
}

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Writes data to the MIL function generator
 * @param pMilTaskData pointer to the current MIL-task-data object
 * @param isScuBus if true via SCU bus MIL adapter
 * @param channel channel number of function generator
 * @return MIL-device status
 * @see milDeviceHandler
 */
STATIC inline
int milHandleAndWrite( register MIL_TASK_DATA_T* pMilTaskData, const bool isScuBus,
                                                  const unsigned int channel )
{
   FG_ASSERT( pMilTaskData->slave_nr != INVALID_SLAVE_NR );
   const unsigned int dev = getDevice( channel );
   handleMacros( getSocket( channel ), dev, pMilTaskData->aFgChannels[channel].irq_data,
                                   &(pMilTaskData->aFgChannels[channel].setvalue) );

   //clear irq pending and end block transfer
   if( isScuBus )
   {
      return scub_write_mil( g_pScub_base, pMilTaskData->slave_nr,
                                                    0,  dev | FC_IRQ_ACT_WR );
   }
   return write_mil( g_pScu_mil_base, 0, dev | FC_IRQ_ACT_WR );
}

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Set the read task of the MIL device
 * @param pMilTaskData pointer to the current MIL-task-data object
 * @param isScuBus if true via SCU bus MIL adapter
 * @param channel channel number of function generator
 * @return MIL-device status
 * @see milDeviceHandler
 */
STATIC inline
int milSetTask( register MIL_TASK_DATA_T* pMilTaskData, const bool isScuBus,
                const unsigned int channel )
{
   FG_ASSERT( pMilTaskData->slave_nr != INVALID_SLAVE_NR );
   const unsigned int  devAndMode = getDevice( channel ) | FC_ACT_RD;
   const unsigned char milTaskNo  = getMilTaskNumber( pMilTaskData, channel );
   if( isScuBus )
   {
      return scub_set_task_mil( g_pScub_base, pMilTaskData->slave_nr,
                                                      milTaskNo, devAndMode );
   }
   return set_task_mil( g_pScu_mil_base, milTaskNo, devAndMode );
}

STATIC_ASSERT( sizeof(short) == sizeof(int16_t) );

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Reads the actual ADC value from MIL-device
 * @param pMilTaskData pointer to the current MIL-task-data object
 * @param isScuBus if true via SCU bus MIL adapter
 * @param channel channel number of function generator
 * @param pActAdcValue Pointer in which shall the ADC-value copied
 * @return MIL-device status
 * @see milDeviceHandler
 */
STATIC inline
int milGetTask( register MIL_TASK_DATA_T* pMilTaskData, const bool isScuBus,
                const unsigned int channel, int16_t* pActAdcValue )
{
   FG_ASSERT( pMilTaskData->slave_nr != INVALID_SLAVE_NR );
   const unsigned char milTaskNo = getMilTaskNumber( pMilTaskData, channel );
   if( isScuBus )
   {
      return scub_get_task_mil( g_pScub_base, pMilTaskData->slave_nr,
                                                   milTaskNo, pActAdcValue );
   }
   return get_task_mil( g_pScu_mil_base, milTaskNo, pActAdcValue );
}

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Loop control macro for all present function generator channels
 *        started form given channel in parameter "start"
 *
 * Helper-macro for function milDeviceHandler
 *
 * It works off the FG-list initialized by function scan_all_fgs() in file
 * scu_function_generator.c from the in parameter start given channel number.
 *
 * @see isFgPresent
 * @see milDeviceHandler
 * @param channel current channel number, shall be of type unsigned int.
 * @param start Channel number to start.
 */
#define FOR_EACH_FG_CONTINUING( channel, start ) \
   for( channel = start; isFgPresent( channel ); channel++ )

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Loop control macro for all present function generator channels.
 *
 * It works off the entire FG-list initialized by function scan_all_fgs() in
 * file scu_function_generator.c
 *
 * Helper-macro for function milDeviceHandler
 *
 * @see isFgPresent
 * @see milDeviceHandler
 * @param channel current channel number, shall be of type unsigned int.
 */
#define FOR_EACH_FG( channel ) FOR_EACH_FG_CONTINUING( channel, 0 )

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Task-function for handling all MIL-FGs and MIL-DAQs via FSM.
 * @param pThis pointer to the current task object
 * @param isScuBus if true via SCU bus MIL adapter
 *
 * @dotfile scu_main.gv State graph for this function
 * @see https://github.com/UlrichBecker/DocFsm
 *
 * @todo When gap-reading is activated (compiler switch CONFIG_READ_MIL_TIME_GAP
 *       is defined) so the danger of jittering could be exist! \n
 *       Solution proposal: Linux-host resp. SAFTLIB shall send a
 *       "function-generator-announcement-signal", before the function generator
 *       issued a new analog signal.
 */
STATIC void milDeviceHandler( register TASK_T* pThis, const bool isScuBus )
{
#ifdef CONFIG_READ_MIL_TIME_GAP
  // static uint64_t gapReadingTime = 0;
#endif
   unsigned int channel;
   int status = OKAY;

   /*
    * Checking integrity of pointer when macro FG_ASSERT is activated, that means
    * CONFIG_FG_PEDANTIC_CHECK is defined.
    */
   FG_ASSERT( pThis->pTaskData != NULL );
   FG_ASSERT( (unsigned int)pThis->pTaskData >= (unsigned int)&g_aMilTaskData[0] );
   FG_ASSERT( (unsigned int)pThis->pTaskData <= (unsigned int)&g_aMilTaskData[ARRAY_SIZE(g_aMilTaskData)-1] );

   MIL_TASK_DATA_T* pMilData = (MIL_TASK_DATA_T*) pThis->pTaskData;

   const FG_STATE_T lastState = pMilData->state;

   /*
    * Performing the FSM state-do activities.
    */
   switch( lastState )
   {
      case ST_WAIT:
      {
         if( has_msg( &g_aMsg_buf[0], isScuBus? DEVSIO : DEVBUS ) )
         {
            FSM_TRANSITION( ST_PREPARE, label='Massage received', color=green );
            break;
         }
      #ifdef CONFIG_READ_MIL_TIME_GAP
         /*
          * Only a task which has already served a function generator
          * can read a time-gap. That means its slave number has to be valid.
          */
         if( (pMilData->slave_nr != INVALID_SLAVE_NR) && (getSysTime() >= pMilData->gapReadingTime) )
         {
            FSM_TRANSITION( ST_DATA_AQUISITION, label='Gap reading time\nexpired',
                                                color=magenta );
            break;
         }
      #endif
      #ifdef __DOCFSM__
         FSM_TRANSITION( ST_WAIT, label='No message', color=blue );
      #endif
         break;
      } // end case ST_WAIT

      case ST_PREPARE:
      {
         // wait for 200 us
         if( getSysTime() < pMilData->timestamp1 )
         {
         #ifdef __DOCFSM__
            FSM_TRANSITION( ST_PREPARE, label='200 us not expired', color=blue );
         #endif
            break;
         }
         /* poll all pending regs on the dev bus; non blocking read operation */
         FOR_EACH_FG( channel )
         {
            status = milReqestStatus( pMilData, isScuBus, channel );
            if( status != OKAY )
               dev_failure( status, 20, "dev_sio set task" );
         }
         FSM_TRANSITION( ST_FETCH_STATUS, color=green );
         break;
      }

      case ST_FETCH_STATUS:
      {
         /* if timeout reached, proceed with next task */
         if( pMilData->task_timeout_cnt > TASK_TIMEOUT )
         {
            printTimeoutMessage( pMilData, isScuBus );
         #ifdef CONFIG_GOTO_STWAIT_WHEN_TIMEOUT
            FSM_TRANSITION( ST_WAIT, label='maximum timeout-count\nreached'
                                     color=red );
            break;
         #else
            /*
             * skipping the faulty channel
             */
            pMilData->lastChannel++;
            pMilData->task_timeout_cnt = 0;
         #endif
         }
         /* fetch status from dev bus controller; */
         FOR_EACH_FG_CONTINUING( channel, pMilData->lastChannel )
         {
            status = milGetStatus( pMilData, isScuBus, channel );
            if( status == RCV_TASK_BSY )
               break; // break from FOR_EACH_FG_CONTINUING loop
            if( status != OKAY )
               mil_failure( status, pMilData->slave_nr );
         }
         if( status == RCV_TASK_BSY )
         {
            pMilData->lastChannel = channel; // start next time from channel
            pMilData->task_timeout_cnt++;
         #ifdef __DOCFSM__
            FSM_TRANSITION( ST_FETCH_STATUS, label='Receiving busy', color=blue );
         #endif
            break;
         }
         FSM_TRANSITION( ST_HANDLE_IRQS, color=green );
         break;
      } // end case ST_FETCH_STATUS

      case ST_HANDLE_IRQS:
      {  /*
          * handle irqs for ifas with active pending regs; non blocking write
          */
         FOR_EACH_FG( channel )
         {
            if( isNoIrqPending( pMilData, channel ) )
               continue; // Handle next channel...

            status = milHandleAndWrite( pMilData, isScuBus, channel );
            if( status != OKAY )
               dev_failure(status, 22, "dev_sio end handle");
         }
         FSM_TRANSITION( ST_DATA_AQUISITION, color=green );
         break;
      } // end case ST_HANDLE_IRQS

      case ST_DATA_AQUISITION:
      {  /* data aquisition */
         FOR_EACH_FG( channel )
         {
            if( isNoIrqPending( pMilData, channel ) )
               continue; // Handle next channel...

            pMilData->aFgChannels[channel].daq_timestamp = getSysTime(); // store the sample timestamp of daq
            status = milSetTask( pMilData, isScuBus, channel );
            if( status != OKAY )
               dev_failure( status, 23, "dev_sio read daq" );
         }
         FSM_TRANSITION( ST_FETCH_DATA, color=green );
         break;
      } // end case ST_DATA_AQUISITION

      case ST_FETCH_DATA:
      {
         /* if timeout reached, proceed with next task */
         if( pMilData->task_timeout_cnt > TASK_TIMEOUT )
         {
            printTimeoutMessage( pMilData, isScuBus );
         #ifdef CONFIG_GOTO_STWAIT_WHEN_TIMEOUT
            FSM_TRANSITION( ST_WAIT, label='maximum timeout-count\nreached'
                                     color=blue );
            break;
         #else
            /*
             * skipping the faulty channel
             */
            pMilData->lastChannel++;
            pMilData->task_timeout_cnt = 0;
         #endif
         }
         /* fetch daq data */
         FOR_EACH_FG_CONTINUING( channel, pMilData->lastChannel )
         {
            if( isNoIrqPending( pMilData, channel ) )
               continue; // Handle next channel...

            int16_t actAdcValue;
            status = milGetTask( pMilData, isScuBus, channel, &actAdcValue );
            if( status == RCV_TASK_BSY )
               break; // break from FOR_EACH_FG_CONTINUING loop

            if( status != OKAY )
            {
               mil_failure( status, pMilData->slave_nr );
               continue; // Handle next channel...
            }
            pushDaqData( getFgMacroViaFgRegister( channel ),
                         pMilData->aFgChannels[channel].daq_timestamp,
                         actAdcValue,
                         g_aFgChannels[channel].last_c_coeff
                      #ifdef CONFIG_READ_MIL_TIME_GAP
                         , pMilData->gapReadingTime != 0
                      #endif
                       );
            // save the setvalue from the tuple sent for the next drq handling
            g_aFgChannels[channel].last_c_coeff = pMilData->aFgChannels[channel].setvalue;
         } // end FOR_EACH_FG_CONTINUING

         if( status == RCV_TASK_BSY )
         {
            pMilData->lastChannel = channel; // start next time from channel
            pMilData->task_timeout_cnt++;
         #ifdef __DOCFSM__
            FSM_TRANSITION( ST_FETCH_DATA, label='Receiving busy', color=blue );
         #endif
            break;
         }
         FSM_TRANSITION( ST_WAIT, color=green );
         break;
      } // end case ST_FETCH_DATA

      default: /* Should never be reached! */
      {
         mprintf( ESC_ERROR"Unknown FSM-state of %s(): %d !"ESC_NORMAL"\n",
                  __func__, pMilData->state );
         FSM_INIT_FSM( ST_WAIT, label='Initializing', color=blue );
         break;
      }
   } /* End of state-do activities */

   /*
    * Has the FSM-state changed?
    */
   if( lastState == pMilData->state )
      return; /* No, there is nothing more to do. */

   /*
    *    *** The FSM-state has changed! ***
    *
    * Performing FSM-state-transition activities if necessary,
    * respectively here the state-entry activities.
    */
   switch( pMilData->state )
   {
   #ifdef CONFIG_READ_MIL_TIME_GAP
      case ST_WAIT:
      {
         pMilData->gapReadingTime = getSysTime() + INTERVAL_10MS;
         break;
      }
   #endif
      case ST_PREPARE:
      {
      #ifdef CONFIG_READ_MIL_TIME_GAP
         pMilData->gapReadingTime = 0;
      #endif
         const MSI_T m = remove_msg( &g_aMsg_buf[0], isScuBus? DEVSIO : DEVBUS );
         pMilData->slave_nr = isScuBus? (m.msg + 1) : 0;
         pMilData->timestamp1 = getSysTime() + INTERVAL_200US;
         break;
      }
      case ST_FETCH_STATUS: /* Go immediately to next case. */
      case ST_FETCH_DATA:
      {
         pMilData->lastChannel = 0; // start next time from channel 0
         pMilData->task_timeout_cnt = 0;
         break;
      }
      default: break;
   } /* End of state entry activities */
} // end function milDeviceHandler

/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @ingroup MIL_FSM
 * @brief can have multiple instances, one for each active sio card controlling
 * a dev bus persistent data, like the state or the sio slave_nr, is stored in
 * a global structure
 * @param pThis pointer to the current task object
 * @see schedule
 */
STATIC void dev_sio_handler( register TASK_T* pThis )
{
   milDeviceHandler( pThis, true ); //false );
}

/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @ingroup MIL_FSM
 * @brief has only one instance
 * persistent data is stored in a global structure
 * @param pThis pointer to the current task object
 * @see schedule
 */
STATIC void dev_bus_handler( register TASK_T* pThis )
{
   milDeviceHandler( pThis, false ); //true );
}

/*! ---------------------------------------------------------------------------
 * @brief Helper function for printing the CPU-ID and the number of
 *        MSI endpoints.
 */
STATIC inline void printCpuId( void )
{
   unsigned int* cpu_info_base;
   cpu_info_base = (unsigned int*)find_device_adr(GSI, CPU_INFO_ROM);
   if((int)cpu_info_base == ERROR_NOT_FOUND)
   {
      mprintf(ESC_ERROR"no CPU INFO ROM found!"ESC_NORMAL"\n");
      return;
   }
   mprintf("CPU ID: 0x%x\n", cpu_info_base[0]);
   mprintf("number MSI endpoints: %d\n", cpu_info_base[1]);
}

/*! ---------------------------------------------------------------------------
 * @brief Tells saftlib the mailbox slot for sw irqs
 */
STATIC inline void tellMailboxSlot( void )
{
   const int slot = getMsiBoxSlot(0x10); //TODO Where does 0x10 come from?
   if( slot == -1 )
      mprintf( ESC_ERROR"No free slots in MsgBox left!"ESC_NORMAL"\n" );
   else
      mprintf( "Configured slot %d in MsgBox\n", slot );
   g_shared.fg_mb_slot = slot;
}

/*================================ MAIN =====================================*/
int main( void )
{
   initializeGlobalPointers();
   mprintf("Compiler: "COMPILER_VERSION_STRING"\n"
           "Git revision: "TO_STRING(GIT_REVISION)"\n"
           "Found MsgBox at 0x%08x. MSI Path is 0x%08x\n",
           (uint32_t)pCpuMsiBox, (uint32_t)pMyMsi);
#ifdef CONFIG_READ_MIL_TIME_GAP
   mprintf( ESC_WARNING"CAUTION! Time gap reading activated!"ESC_NORMAL"\n" );
#endif
   tellMailboxSlot();
   init_irq_table();

   msDelayBig(1500); //wait for wr deamon to read sdbfs

   if( (int)BASE_SYSCON == ERROR_NOT_FOUND )
      mprintf(ESC_ERROR"no SYS_CON found!"ESC_NORMAL"\n");
   else
      mprintf("SYS_CON found on adr: 0x%x\n", BASE_SYSCON);

   timer_init(1); //needed by usleep_init()
   usleep_init();

   printCpuId();
   mprintf("g_oneWireBase.pWr is:   0x%08x\n", g_oneWireBase.pWr);
   mprintf("g_oneWireBase.pUser is: 0x%08x\n", g_oneWireBase.pUser);
   mprintf("g_pScub_irq_base is:    0x%08x\n", g_pScub_irq_base);
   mprintf("g_pMil_irq_base is:     0x%08x\n", g_pMil_irq_base);
   initEcaQueue();

   init(); // init and scan for fgs

#ifdef CONFIG_SCU_DAQ_INTEGRATION
   scuDaqInitialize( &g_scuDaqAdmin ); // Init and scan for DAQs
   mprintf( "SCU-DAQ initialized\n" );
#endif
   //print_regs();
   while( true )
   {
      check_stack();
      schedule();
   }
   return 0;
}

/*================================== EOF ====================================*/
