/*!
 * @file scu_fg_macros.c
 * @brief Module for handling MIL and non MIL
 *        function generator macros
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 04.02.2020
 * Outsourced from scu_main.c
 */
#include "scu_fg_macros.h"

extern volatile uint16_t*     g_pScub_base;
#ifdef CONFIG_MIL_FG
extern volatile unsigned int* g_pScu_mil_base;
#endif

/*!
 * @brief Memory space of sent function generator data.
 *        Non shared memory part for each function generator channel.
 */
FG_CHANNEL_T g_aFgChannels[MAX_FG_CHANNELS] = {{0,0}};//,0}};

/*! ---------------------------------------------------------------------------
 * @brief Prints a error message happened in the device-bus respectively
 *        MIL bus.
 * @param status return status of the MIL-driver module.
 * @param slot Slot-number in the case the mil connection is established via
 *             SCU-Bus
 * @param msg String containing additional message text.
 */
void printDeviceError( const int status, const int slot, const char* msg )
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
 * @see scu_fg_macros.h
 * @todo Split this in two separate functions: MIL and non-MIL.
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

   const uint8_t socket = getSocket( channel );
   /* actions per slave card */
#ifdef CONFIG_MIL_FG
   uint16_t dreq_status = 0;
   #define _MIL_BIT_MASK()  (1 << MAX_SCU_SLAVES)

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
#endif /* CONFIG_MIL_FG */

   const uint8_t dev  = getDevice( channel );
    /* enable irqs */
   if( isNonMilFg( socket ) )
   {                                      //scu bus slave
      g_pScub_base[SRQ_ENA] |= (1 << (socket-1));           // enable irqs for the slave
      g_pScub_base[OFFS(socket) + SLAVE_INT_ACT] =  (FG1_IRQ | FG2_IRQ); // clear all irqs
      g_pScub_base[OFFS(socket) + SLAVE_INT_ENA] |= (FG1_IRQ | FG2_IRQ); // enable fg1 and fg2 irq
   }
#ifdef CONFIG_MIL_FG
   else if( isMilExtentionFg( socket ) )
   {
      if( (status = write_mil(g_pScu_mil_base, 1 << 13, FC_IRQ_MSK | dev)) != OKAY)
         printDeviceError( status, 0, "enable dreq" ); //enable Data-Request
   }
   else if( isMilScuBusFg( socket ) )
   {
      FG_ASSERT( getFgSlotNumber( socket ) > 0 );
      g_pScub_base[SRQ_ENA] |= _SLOT_BIT_MASK();        // enable irqs for the slave
      g_pScub_base[OFFS(getFgSlotNumber( socket )) + SLAVE_INT_ENA] = DREQ; // enable receiving of drq
      if( (status = scub_write_mil(g_pScub_base, getFgSlotNumber( socket ), 1 << 13, FC_IRQ_MSK | dev)) != OKAY)
         printDeviceError( status, getFgSlotNumber( socket ), "enable dreq"); //enable sending of drq
   }
#endif /* CONFIG_MIL_FG */
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
#ifdef CONFIG_MIL_FG
   else if( isMilExtentionFg( socket ) )
   {
      if( (status = write_mil(g_pScu_mil_base, 0x1, FC_IFAMODE_WR | dev)) != OKAY)
         printDeviceError( status, 0, "set FG mode"); // set FG mode
   }
   else if( isMilScuBusFg( socket ) )
   {
      if( (status = scub_write_mil(g_pScub_base, getFgSlotNumber( socket ), 0x1, FC_IFAMODE_WR | dev)) != OKAY)
         printDeviceError( status, getFgSlotNumber( socket ), "set FG mode"); // set FG mode
   }
#endif
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
   #ifdef CONFIG_MIL_FG
      else if( isMilExtentionFg( socket ) )
      {
        // save the coeff_c for mil daq
         g_aFgChannels[channel].last_c_coeff = pset.coeff_c;
        // transmit in one block transfer over the dev bus
         if((status = write_mil_blk(g_pScu_mil_base, &blk_data[0], FC_BLK_WR | dev)) != OKAY)
            printDeviceError( status, 0, "blk trm");
        // still in block mode !
         if((status = write_mil(g_pScu_mil_base, cntrl_reg_wr, FC_CNTRL_WR | dev)) != OKAY)
            printDeviceError( status, 0, "end blk trm");
      }
      else if( isMilScuBusFg( socket ) )
      {
         // save the coeff_c for mil daq
         g_aFgChannels[channel].last_c_coeff = pset.coeff_c;
         // transmit in one block transfer over the dev bus
         if( (status = scub_write_mil_blk(g_pScub_base, getFgSlotNumber( socket ), &blk_data[0], FC_BLK_WR | dev)) != OKAY)
            printDeviceError( status, getFgSlotNumber( socket ), "blk trm");
         // still in block mode !
         if( (status = scub_write_mil(g_pScub_base, getFgSlotNumber( socket ), cntrl_reg_wr, FC_CNTRL_WR | dev)) != OKAY)
            printDeviceError( status, getFgSlotNumber( socket ), "end blk trm");
      }
   #endif /* CONFIG_MIL_FG */
      g_aFgChannels[0].param_sent++;
  //!! }CONFIG_GOTO_STWAIT_WHEN_TIMEOUT

   /* configure and enable macro */
   if( isNonMilFg( socket ) )
   {
      g_pScub_base[OFFS(socket) + fg_base + FG_TAG_LOW] = g_shared.fg_regs[channel].tag & 0xffff;
      g_pScub_base[OFFS(socket) + fg_base + FG_TAG_HIGH] = g_shared.fg_regs[channel].tag >> BIT_SIZEOF(uint16_t);
      g_pScub_base[OFFS(socket) + fg_base + FG_CNTRL] |= FG_ENABLED;
   }
#ifdef CONFIG_MIL_FG
   else if( isMilExtentionFg( socket ) )
   { // enable and end block mode
      if( (status = write_mil(g_pScu_mil_base, cntrl_reg_wr | FG_ENABLED, FC_CNTRL_WR | dev)) != OKAY)
         printDeviceError( status, 0, "end blk mode");
   }
   else if( isMilScuBusFg( socket ) )
   { // enable and end block mode
      if( (status = scub_write_mil( g_pScub_base, getFgSlotNumber( socket ),
           cntrl_reg_wr | FG_ENABLED, FC_CNTRL_WR | dev ) ) != OKAY )
         printDeviceError( status, getFgSlotNumber( socket ), "end blk mode");
   }
#endif /* CONFIG_MIL_FG */
   } //!!
   // reset watchdog
 //  g_aFgChannels[channel].timeout = 0;
   g_shared.fg_regs[channel].state = STATE_ARMED;
   sendSignal( IRQ_DAT_ARMED, channel );
   return 0;
}

/*! ---------------------------------------------------------------------------
 * @see scu_fg_macros.h
 */
void disable_channel( const unsigned int channel )
{
   FG_CHANNEL_REG_T* pFgRegs = &g_shared.fg_regs[channel];

   if( pFgRegs->macro_number == SCU_INVALID_VALUE )
      return;

#ifdef CONFIG_MIL_FG
   int status;
   int16_t data;
#endif
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
#ifdef CONFIG_MIL_FG
   else if( isMilExtentionFg( socket ) )
   {  // disarm hardware
      if( (status = read_mil( g_pScu_mil_base, &data, FC_CNTRL_RD | dev)) != OKAY )
         printDeviceError( status, 0, "disarm hw 1" );

      if( (status = write_mil( g_pScu_mil_base, data & ~(0x2), FC_CNTRL_WR | dev)) != OKAY )
         printDeviceError( status, 0, "disarm hw 2" );
   }
   else if( isMilScuBusFg( socket ) )
   {  // disarm hardware
      if( (status = scub_read_mil( g_pScub_base, getFgSlotNumber( socket ),
           &data, FC_CNTRL_RD | dev)) != OKAY )
         printDeviceError( status, getFgSlotNumber( socket ), "disarm hw 3" );

      if( (status = scub_write_mil( g_pScub_base, getFgSlotNumber( socket ),
           data & ~(0x2), FC_CNTRL_WR | dev)) != OKAY )
         printDeviceError( status, getFgSlotNumber( socket ), "disarm hw 4" );
   }
#endif /* CONFIG_MIL_FG */

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
 * @brief disables the generation of irqs for the specified channel
 *  SIO and MIL extension stop generating irqs
 *  @param channel number of the channel from 0 to MAX_FG_CHANNELS-1
 * @see enable_scub_msis
 */
void disable_slave_irq( const unsigned int channel )
{
   if( channel >= MAX_FG_CHANNELS )
      return;

#ifdef CONFIG_MIL_FG
   int status;
#endif
   const uint8_t socket = getSocket( channel );
   const uint8_t dev    = getDevice( channel );

   if( isNonMilFg( socket ) )
   {
      if( dev == 0 )
        g_pScub_base[OFFS(socket) + SLAVE_INT_ENA] &= ~(0x8000); //disable fg1 irq
      else if( dev == 1 )
        g_pScub_base[OFFS(socket) + SLAVE_INT_ENA] &= ~(0x4000); //disable fg2 irq
   }
#ifdef CONFIG_MIL_FG
   else if( isMilExtentionFg( socket ) )
   {
      //write_mil(g_pScu_mil_base, 0x0, FC_COEFF_A_WR | dev);            //ack drq
      if( (status = write_mil(g_pScu_mil_base, 0x0, FC_IRQ_MSK | dev) ) != OKAY)
         printDeviceError( status, getFgSlotNumber( socket ), __func__);  //mask drq
   }
   else if( isMilScuBusFg( socket ) )
   {
      if( (status = scub_write_mil(g_pScub_base, getFgSlotNumber( socket ),
                                   0x0, FC_IRQ_MSK | dev)) != OKAY)
         printDeviceError( status, getFgSlotNumber( socket ), __func__);  //mask drq
   }
#endif
   //mprintf("IRQs for slave %d disabled.\n", socket);
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
 * @todo Split this in two separate functions: MIL and non-MIL.
 */
 void send_fg_param( const unsigned int socket,
                                  const unsigned int fg_base,
                                  const uint16_t cntrl_reg,
                                  signed int* pSetvalue )
{
   FG_PARAM_SET_T pset;
   uint16_t       cntrl_reg_wr;
#ifdef CONFIG_MIL_FG
   int            status;
#endif
   int16_t        blk_data[MIL_BLOCK_SIZE];

   const unsigned int fg_num = getFgNumberFromRegister( cntrl_reg );
   if( fg_num >= ARRAY_SIZE( g_aFgChannels ) )
   {
      mprintf( ESC_ERROR"FG-number %d out of range!"ESC_NORMAL"\n", fg_num );
      return;
   }

   /*
    * Reading circular buffer with new FG-data.
    */
   if( !cbRead( &g_shared.fg_buffer[0], &g_shared.fg_regs[0], fg_num, &pset ) )
   {
      hist_addx(HISTORY_XYZ_MODULE, "buffer empty, no parameter sent", socket);
      return;
   }

   //!@todo Replace the following hex numbers by meaningful defined bit-masks!!!
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
#ifdef CONFIG_MIL_FG
   else if( isMilExtentionFg( socket ) )
   {
      // save coeff_c as setvalue
      *pSetvalue = pset.coeff_c;
      // transmit in one block transfer over the dev bus
      if((status = write_mil_blk(g_pScu_mil_base, &blk_data[0], FC_BLK_WR | fg_base)) != OKAY)
         printDeviceError( status, 0, __func__);
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
         printDeviceError( status, getFgSlotNumber( socket ), __func__);
      }
      // still in block mode !
   }
#endif /* CONFIG_MIL_FG */
   g_aFgChannels[fg_num].param_sent++;
}

/*! ---------------------------------------------------------------------------
 * @brief Send signal REFILL to the SAFTLIB when the fifo level has
 *        the threshold reached. Helper function of function handleMacros().
 * @see handleMacros
 * @param channel Channel of concerning function generator.
 */
void sendRefillSignalIfThreshold( const unsigned int channel )
{
   if( cbgetCount( &g_shared.fg_regs[0], channel ) == THRESHOLD )
   {
      //mprintf( "*" );
      sendSignal( IRQ_DAT_REFILL, channel );
   }
}

#ifndef _CONFIG_NEW
/*! ---------------------------------------------------------------------------
 * @see scu_main.h
 * @todo Split this in two separate functions: MIL and non-MIL.
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
#ifdef CONFIG_MIL_FG
   else
   {
      channel = getFgNumberFromRegister( irq_act_reg );
   }
#endif
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
#ifdef CONFIG_MIL_FG
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
#endif /* CONFIG_MIL_FG */
}
#endif //_CONFIG_NEW
/*================================== EOF ====================================*/
