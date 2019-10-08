/*!
 * @file scu_main.c
 * @brief Revision from main.c
 */

#ifndef __DOCFSM__

#include <stdint.h>
#include <stack.h>

#include "syscon.h"
//#include "hw/memlayout.h"
#include "eb_console_helper.h"
//#include "mprintf.h"
#include "display.h"
#include "irq.h"
#include "scu_bus.h"
#include "aux.h"
#include "mini_sdb.h"
#include "board.h"
#include "uart.h"
#include "w1.h"
//#include "fg.h"
//#include "cb.h"
#include "scu_shared_mem.h"
#include "scu_mil.h"
#include "dow_crc.h"
#include "eca_queue_regs.h"
#include "eca_flags.h"
#include "history.h"

#ifdef CONFIG_SCU_DAQ_INTEGRATION
#include <daq_main.h>
#endif

#endif // ifndef __DOCFSM__

#define MSI_SLAVE 0
#define MSI_WB_FG 2

#define SIG_REFILL      0
#define SIG_START       1
#define SIG_STOP_EMPTY  2
#define SIG_STOP_NEMPTY 3
#define SIG_ARMED       4
#define SIG_DISARMED    5

#define QUEUE_CNT 5
#define IRQ       0
#define SCUBUS    1
#define DEVBUS    2
#define DEVSIO    3
#define SWI       4

#define DEVB_MSI      0xdeb50000
#define SCUB_MSI      0x5cb50000

#define MIL_DRQ       0x2
#define MIL_DRY       0x1
#define MIL_INL       0x0


#define CLK_PERIOD (1000000 / USRCPUCLK) // USRCPUCLK in KHz
#define OFFS(SLOT) ((SLOT) * (1 << 16))

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

#define MY_ECA_TAG      0xdeadbeef //just define a tag for ECA actions we want to receive


#define SHARED __attribute__((section(".shared")))
/*!!!!!!!!!!!!!!!!!!!!!! Begin of shared memory area !!!!!!!!!!!!!!!!!!!!!!!!*/
SCU_SHARED_DATA_T SHARED g_shared = SCU_SHARED_DATA_INITIALIZER;
/*!!!!!!!!!!!!!!!!!!!!!! End of shared memory area !!!!!!!!!!!!!!!!!!!!!!!!!!*/

typedef uint32_t ECA_T;

volatile uint16_t* g_pScub_base        = NULL;
volatile uint32_t* g_pScub_irq_base    = NULL;
volatile unsigned int* g_pScu_mil_base = NULL;
volatile uint32_t* g_pMil_irq_base     = NULL;
volatile uint32_t* g_pWr_1wire_base    = NULL;
volatile uint32_t* g_pUser_1wire_base  = NULL;
volatile ECA_T*    g_pECAQ             = NULL; // WB address of ECA queue

int g_aClear_is_active[MAX_SCU_SLAVES + 1] = {0};
volatile struct message_buffer g_aMsg_buf[QUEUE_CNT] = {0};


typedef struct
{
   uint64_t timeout;
   uint32_t param_sent;
   int32_t  last_c_coeff;
} FG_CHANNEL_T;

FG_CHANNEL_T g_aFgChannels[MAX_FG_CHANNELS] = {{0,0,0}};

#define SEND_SIG(SIG) \
   *(volatile uint32_t*)(char*) \
   (pCpuMsiBox + g_shared.fg_regs[channel].mbx_slot * sizeof(uint16_t)) = SIG

static void clear_handler_state( unsigned int );

static inline uint32_t _get_macro_number( unsigned int channel )
{
   return g_shared.fg_macros[g_shared.fg_regs[channel].macro_number];
}

static inline unsigned int _get_socket( unsigned int channel )
{
   return getMilDaqLocationByChannel( _get_macro_number( channel ) );
}

static inline unsigned int _get_dev( unsigned int channel )
{
   return getMilDaqAdressByChannel(_get_macro_number( channel ));
}

static inline void initializeGlobalPointers( void )
{
   discoverPeriphery();
   uart_init_hw();
   /* additional periphery needed for scu */
   g_pScub_base     = (volatile uint16_t*)find_device_adr(GSI, SCU_BUS_MASTER);
   g_pScub_irq_base = (volatile uint32_t*)find_device_adr(GSI, SCU_IRQ_CTRL); // irq controller for scu bus
   g_pScu_mil_base  = (unsigned int*)find_device_adr(GSI,SCU_MIL);            // mil extension macro
   g_pMil_irq_base  = (volatile uint32_t*)find_device_adr(GSI, MIL_IRQ_CTRL); // irq controller for dev bus extension
   g_pWr_1wire_base = (volatile uint32_t*)find_device_adr(CERN, WR_1Wire);    // 1Wire controller in the WRC
   g_pUser_1wire_base = (volatile uint32_t*)find_device_adr(GSI, User_1Wire); // 1Wire controller on dev crossbar
}


static void dev_failure(const int status, const int slot, const char* msg)
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
        mprintf("%s%d failed with code %d\n"ESC_NORMAL, pText, slot, status);
        return;
  }
  #undef __MSG_ITEM
  mprintf("%s%d failed with message %s, %s\n"ESC_NORMAL, pText, slot, pMessage, msg);
}

static void mil_failure( const int status, const int slave_nr )
{
   switch( status )
   {
      case RCV_PARITY:
      {
         mprintf(ESC_ERROR"parity error when reading task %d\n"ESC_NORMAL, slave_nr );
         break;
      }
      case RCV_TIMEOUT:
      {
         mprintf(ESC_ERROR"timeout error when reading task %d\n"ESC_NORMAL, slave_nr );
         break;
      }
      case RCV_ERROR:
      {
         mprintf(ESC_ERROR"unknown error when reading task %d\n"ESC_NORMAL, slave_nr );
         break;
      }
   }
}

#if 0
/** debug method
 * prints the last received message signaled interrupt to the UART
 */
static void show_msi( void )
{
  mprintf(" Msg:\t%08x\nAdr:\t%08x\nSel:\t%01x\n", global_msi.msg, global_msi.adr, global_msi.sel);

}

static void isr0( void )
{
   mprintf("ISR0\n");
   show_msi();
}
#endif
/** @brief enables msi generation for the specified channel.
 *  Messages from the scu bus are send to the msi queue of this cpu with the offset 0x0.
 *  Messages from the MIL extension are send to the msi queue of this cpu with the offset 0x20.
 *  A hardware macro is used, which generates msis from legacy interrupts.
 *  @param channel number of the channel between 0 and MAX_FG_CHANNELS-1
 */
static void enable_scub_msis( unsigned int channel )
{
   int socket;

   if( channel >= MAX_FG_CHANNELS )
      return;

   socket = _get_socket( channel );
   if (((socket & 0xf0) == 0) || (socket & DEV_SIO))
   {
      //SCU Bus Master
      g_pScub_base[GLOBAL_IRQ_ENA] = 0x20;              // enable slave irqs in scu bus master
      g_pScub_irq_base[8]  = (socket & SCU_BUS_SLOT_MASK)-1;            // channel select
      g_pScub_irq_base[9]  = (socket & SCU_BUS_SLOT_MASK)-1;            // msg: socket number
      g_pScub_irq_base[10] = (uint32_t)pMyMsi + 0x0;    // msi queue destination address of this cpu
      g_pScub_irq_base[2]  = (1 << ((socket & SCU_BUS_SLOT_MASK) - 1)); // enable slave
      //mprintf("IRQs for slave %d enabled.\n", (socket & SCU_BUS_SLOT_MASK));
      return;
   }

   if( (socket & DEV_MIL_EXT) == 0 )
      return;

   g_pMil_irq_base[8]   = MIL_DRQ;
   g_pMil_irq_base[9]   = MIL_DRQ;
   g_pMil_irq_base[10]  = (uint32_t)pMyMsi + 0x20;
   g_pMil_irq_base[2]   = (1 << MIL_DRQ);
}

/** @brief disables the generation of irqs for the specified channel
 *  SIO and MIL extension stop generating irqs
 *  @param channel number of the channel from 0 to MAX_FG_CHANNELS-1
 */
static void disable_slave_irq( unsigned int channel )
{
   int socket, dev;
   int status;

   if( channel >= MAX_FG_CHANNELS )
      return;

   socket = _get_socket( channel );
   dev  = _get_dev( channel );
   if ((socket & 0xf0) == 0)
   {
      if (dev == 0)
        g_pScub_base[OFFS(socket) + SLAVE_INT_ENA] &= ~(0x8000);       //disable fg1 irq
      else if (dev == 1)
        g_pScub_base[OFFS(socket) + SLAVE_INT_ENA] &= ~(0x4000);       //disable fg2 irq
   }
   else if (socket & DEV_MIL_EXT)
   {
      //write_mil(g_pScu_mil_base, 0x0, FC_COEFF_A_WR | dev);            //ack drq
      if ((status = write_mil(g_pScu_mil_base, 0x0, FC_IRQ_MSK | dev)) != OKAY)
         dev_failure(status, socket & SCU_BUS_SLOT_MASK, __func__);  //mask drq
   }
   else if (socket & DEV_SIO)
   {
      if ((status = scub_write_mil(g_pScub_base, socket & SCU_BUS_SLOT_MASK, 0x0, FC_IRQ_MSK | dev)) != OKAY)
         dev_failure(status, socket & SCU_BUS_SLOT_MASK, __func__);  //mask drq
   }

   //mprintf("IRQs for slave %d disabled.\n", socket);
}

/** @brief delay in multiples of one millisecond
 *  uses the system timer
 *  @param ms delay value in milliseconds
 */
static void msDelayBig(uint64_t ms)
{
   uint64_t later = getSysTime() + ms * 1000000ULL / 8;
   while(getSysTime() < later) {asm("# noop");}
}

#if 0
static void msDelay(uint32_t msecs)
{
  usleep(1000 * msecs);
}
#endif

static inline unsigned int getFgNumberFromRegister( const uint16_t reg )
{
   return (reg >> 4) & 0x3F; // virtual fg number Bits 9..4
}

/** @brief sends the parameters for the next interpolation interval
 *  @param socket number of the slot, including the high bits with the information SIO or MIL_EXT
 *  @param fg_base base address of the function generator macro
 *  @param cntrl_reg state of the control register. saves one read access.
 */
static inline void send_fg_param( const  unsigned int socket,
                                  const unsigned int fg_base,
                                  const uint16_t cntrl_reg,
                                  signed int* pSetvalue )
{
   struct param_set pset;
   unsigned int fg_num;
   uint16_t cntrl_reg_wr;
   int status;
   int16_t blk_data[6];

   //fg_num = (cntrl_reg & 0x3f0) >> 4; // virtual fg number Bits 9..4
   fg_num = getFgNumberFromRegister( cntrl_reg );
   if( fg_num >= ARRAY_SIZE( g_aFgChannels ) )
   {
      mprintf( ESC_ERROR"FG-number %d out of range!\n"ESC_NORMAL, fg_num );
      return;
   }

   if( cbRead(&g_shared.fg_buffer[0], &g_shared.fg_regs[0], fg_num, &pset) == 0 )
   {
      hist_addx(HISTORY_XYZ_MODULE, "buffer empty, no parameter sent", socket);
      return;
   }

   //TODO remove naked mask numbers by well named constants or inline geter functions.
   cntrl_reg_wr = cntrl_reg & ~(0xfc07); // clear freq, step select, fg_running and fg_enabled
   cntrl_reg_wr |= ((pset.control & 0x38) << 10) | ((pset.control & 0x7) << 10);
   blk_data[0] = cntrl_reg_wr;
   blk_data[1] = pset.coeff_a;
   blk_data[2] = pset.coeff_b;
   blk_data[3] = (pset.control & 0x3ffc0) >> 6;     // shift a 17..12 shift b 11..6
   blk_data[4] = pset.coeff_c & 0xffff;
   blk_data[5] = (pset.coeff_c & 0xffff0000) >> 16; // data written with high word

   if ((socket & 0xf0) == 0)
   {
      g_pScub_base[OFFS(socket) + fg_base + FG_CNTRL]  = blk_data[0];
      g_pScub_base[OFFS(socket) + fg_base + FG_A]      = blk_data[1];
      g_pScub_base[OFFS(socket) + fg_base + FG_B]      = blk_data[2];
      g_pScub_base[OFFS(socket) + fg_base + FG_SHIFT]  = blk_data[3];
      g_pScub_base[OFFS(socket) + fg_base + FG_STARTL] = blk_data[4];
      g_pScub_base[OFFS(socket) + fg_base + FG_STARTH] = blk_data[5];
      // no setvalue for scu bus daq 
      *pSetvalue = 0;
   }
   else if (socket & DEV_MIL_EXT)
   {
      // save coeff_c as setvalue
      *pSetvalue = pset.coeff_c;
      // transmit in one block transfer over the dev bus
      if((status = write_mil_blk(g_pScu_mil_base, &blk_data[0], FC_BLK_WR | fg_base)) != OKAY)
         dev_failure(status, socket & SCU_BUS_SLOT_MASK, __func__);
      // still in block mode !
   }
   else if (socket & DEV_SIO)
   {  // save coeff_c as setvalue
      *pSetvalue = pset.coeff_c;
      // transmit in one block transfer over the dev bus
      if((status = scub_write_mil_blk( g_pScub_base,
                                       socket & SCU_BUS_SLOT_MASK,
                                       &blk_data[0],
                                       FC_BLK_WR | fg_base)) != OKAY)
      {
         dev_failure(status, socket & SCU_BUS_SLOT_MASK, __func__);
      }
      // still in block mode !
   }
   g_aFgChannels[fg_num].param_sent++;
}

/** @brief decide how to react to the interrupt request from the function generator macro
 *  @param socket encoded slot number with the high bits for SIO / MIL_EXT distinction
 *  @param fg_base base address of the function generator macro
 *  @param irq_act_reg state of the irq act register, saves a read access
 */
#if 0
static void handle( const unsigned int socket,
                    const unsigned int fg_base,
                    const uint16_t irq_act_reg,
                    signed int* setvalue )
{
   uint16_t cntrl_reg = 0;
   unsigned int channel = 0;

   if( (socket & 0xf0) == 0 )
   {
      cntrl_reg = g_pScub_base[OFFS(socket) + fg_base + FG_CNTRL];
      channel = getFgNumberFromRegister( cntrl_reg );
   }
   //else if( (socket & DEV_MIL_EXT) || (socket & DEV_SIO) )
   else if( (socket & (DEV_MIL_EXT | DEV_SIO)) != 0 )
   {
      channel = getFgNumberFromRegister( irq_act_reg );
   }

   if( channel >= ARRAY_SIZE( g_shared.fg_regs ) )
   {
      mprintf( ESC_ERROR"%s: Channel out of range: %d\n"ESC_NORMAL, __func__, channel );
      return;
   }

   if( (socket & 0xf0) == 0 )
   {  /* last cnt from from fg macro, read from LO address copies hardware counter to shadow reg */
      g_shared.fg_regs[channel].ramp_count = g_pScub_base[OFFS(socket) + fg_base + FG_RAMP_CNT_LO];
      g_shared.fg_regs[channel].ramp_count |= g_pScub_base[OFFS(socket) + fg_base + FG_RAMP_CNT_HI] << 16;
   }
   //else if((socket & DEV_MIL_EXT) || (socket & DEV_SIO))
   else if( (socket & (DEV_MIL_EXT | DEV_SIO)) != 0 )
   {  /* count in software only */
      g_shared.fg_regs[channel].ramp_count++;
   }

   if( (socket & 0xf0) == 0 )
   {
      if( !(cntrl_reg  & FG_RUNNING))
      {       // fg stopped
         if( cbisEmpty(&g_shared.fg_regs[0], channel))
         {
            SEND_SIG(SIG_STOP_EMPTY);           // normal stop
            hist_addx(HISTORY_XYZ_MODULE, "sig_stop_empty", channel);
         }
         else
         {
            SEND_SIG(SIG_STOP_NEMPTY);          // something went wrong
            hist_addx(HISTORY_XYZ_MODULE, "sig_stop_nempty", channel);
         }
         disable_slave_irq(channel);
         g_shared.fg_regs[channel].state = STATE_STOPPED;
      }
   }
   else
   {
      if( !(irq_act_reg  & FG_RUNNING) )
      {     // fg stopped
         g_shared.fg_regs[channel].ramp_count--;
         if( cbisEmpty(&g_shared.fg_regs[0], channel) )
         {
            SEND_SIG(SIG_STOP_EMPTY);           // normal stop
            hist_addx(HISTORY_XYZ_MODULE, "sig_stop_empty", channel);
         }
         else
         {
            SEND_SIG(SIG_STOP_NEMPTY);          // something went wrong
            hist_addx(HISTORY_XYZ_MODULE, "sig_stop_nempty", channel);
         }
         disable_slave_irq(channel);
         g_shared.fg_regs[channel].state = STATE_STOPPED;
      }
   }

   if( (socket & 0xf0) == 0)
   {
      if ((cntrl_reg & FG_RUNNING) && !(cntrl_reg & FG_DREQ))
      {
         g_shared.fg_regs[channel].state = STATE_ACTIVE;
         SEND_SIG(SIG_START); // fg has received the tag or brc message
         hist_addx(HISTORY_XYZ_MODULE, "sig_start", channel);
         if( cbgetCount(&g_shared.fg_regs[0], channel) == THRESHOLD )
            SEND_SIG(SIG_REFILL);
         send_fg_param( socket, fg_base, cntrl_reg, setvalue );
      }
      else if( (cntrl_reg & FG_RUNNING) && (cntrl_reg & FG_DREQ) )
      {
         if( cbgetCount(&g_shared.fg_regs[0], channel) == THRESHOLD )
            SEND_SIG(SIG_REFILL);
        send_fg_param(socket, fg_base, cntrl_reg, setvalue);
      }
   }
   else
   {
      if ((irq_act_reg & FG_RUNNING) && (irq_act_reg & DEV_STATE_IRQ))
      {
         g_shared.fg_regs[channel].state = STATE_ACTIVE;
         SEND_SIG(SIG_START); // fg has received the tag or brc message
         hist_addx(HISTORY_XYZ_MODULE, "sig_start", channel);
         if( cbgetCount(&g_shared.fg_regs[0], channel) == THRESHOLD )
          SEND_SIG(SIG_REFILL);
         send_fg_param( socket, fg_base, irq_act_reg, setvalue );
      }
      else if( irq_act_reg & (FG_RUNNING | DEV_DRQ) )
      {
         if (cbgetCount(&g_shared.fg_regs[0], channel) == THRESHOLD)
            SEND_SIG(SIG_REFILL);
         send_fg_param(socket, fg_base, irq_act_reg, setvalue);
      }
   }
}
#else
static void handle( const unsigned int socket,
                    const unsigned int fg_base,
                    const uint16_t irq_act_reg,
                    signed int* setvalue )
{
   uint16_t cntrl_reg = 0;
   unsigned int channel;

   if( (socket & (DEV_MIL_EXT | DEV_SIO)) == 0 )
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

   if( (socket & (DEV_MIL_EXT | DEV_SIO)) == 0 )
   {
      /* last cnt from from fg macro, read from LO address copies hardware counter to shadow reg */
      g_shared.fg_regs[channel].ramp_count = g_pScub_base[OFFS(socket) + fg_base + FG_RAMP_CNT_LO];
      g_shared.fg_regs[channel].ramp_count |= g_pScub_base[OFFS(socket) + fg_base + FG_RAMP_CNT_HI] << 16;

      if( (cntrl_reg  & FG_RUNNING) == 0 )
      {       // fg stopped
         if( cbisEmpty(&g_shared.fg_regs[0], channel))
         {
            SEND_SIG(SIG_STOP_EMPTY);           // normal stop
            hist_addx(HISTORY_XYZ_MODULE, "sig_stop_empty", channel);
         }
         else
         {
            SEND_SIG(SIG_STOP_NEMPTY);          // something went wrong
            hist_addx(HISTORY_XYZ_MODULE, "sig_stop_nempty", channel);
         }
         disable_slave_irq(channel);
         g_shared.fg_regs[channel].state = STATE_STOPPED;
      }
      else
      {
         if( (cntrl_reg & FG_DREQ) == 0 )
         {
            g_shared.fg_regs[channel].state = STATE_ACTIVE;
            SEND_SIG(SIG_START); // fg has received the tag or brc message
            hist_addx(HISTORY_XYZ_MODULE, "sig_start", channel);
            if( cbgetCount(&g_shared.fg_regs[0], channel) == THRESHOLD )
               SEND_SIG(SIG_REFILL);
            send_fg_param( socket, fg_base, cntrl_reg, setvalue );
         }
         else
         {
            if( cbgetCount(&g_shared.fg_regs[0], channel) == THRESHOLD )
               SEND_SIG(SIG_REFILL);
           send_fg_param(socket, fg_base, cntrl_reg, setvalue);
         }
      }
   }
   else /* (socket & (DEV_MIL_EXT | DEV_SIO)) == 0 */
   {
      /* count in software only */
      g_shared.fg_regs[channel].ramp_count++;
      if( (irq_act_reg  & FG_RUNNING) == 0 )
      {     // fg stopped
         g_shared.fg_regs[channel].ramp_count--;
         if( cbisEmpty(&g_shared.fg_regs[0], channel) )
         {
            SEND_SIG(SIG_STOP_EMPTY);           // normal stop
            hist_addx(HISTORY_XYZ_MODULE, "sig_stop_empty", channel);
         }
         else
         {
            SEND_SIG(SIG_STOP_NEMPTY);          // something went wrong
            hist_addx(HISTORY_XYZ_MODULE, "sig_stop_nempty", channel);
         }
         disable_slave_irq(channel);
         g_shared.fg_regs[channel].state = STATE_STOPPED;
      }
      else
      {
         if( (irq_act_reg & DEV_STATE_IRQ) != 0 )
         {
            g_shared.fg_regs[channel].state = STATE_ACTIVE;
            SEND_SIG(SIG_START); // fg has received the tag or brc message
            hist_addx(HISTORY_XYZ_MODULE, "sig_start", channel);
            if( cbgetCount(&g_shared.fg_regs[0], channel) == THRESHOLD )
               SEND_SIG(SIG_REFILL);
            send_fg_param( socket, fg_base, irq_act_reg, setvalue );
         }
         else if( (irq_act_reg & DEV_DRQ) != 0 )
         {
            if (cbgetCount(&g_shared.fg_regs[0], channel) == THRESHOLD)
               SEND_SIG(SIG_REFILL);
            send_fg_param(socket, fg_base, irq_act_reg, setvalue);
         }
      }
   } /* else of if (socket & (DEV_MIL_EXT | DEV_SIO)) == 0 */
}

#endif
/** @brief as short as possible, just pop the msi queue of the cpu and
 *         push it to the message queue of the main loop
 */
void irq_handler( void ) {
  struct msi m;

  // send msi threadsafe to main loop
  m.msg = global_msi.msg;
  m.adr = global_msi.adr;
  add_msg(&g_aMsg_buf[0], IRQ, m);
}


/** @brief configures each function generator channel.
 *  checks first, if the drq line is inactive, if not the line is cleared
 *  then activate irqs and send the first tuple of data to the function generator
 *  @param channel number of the specified function generator channel from 0 to MAX_FG_CHANNELS-1
 */
static int configure_fg_macro( const unsigned int channel )
{
   unsigned int socket;
   int dev;
   unsigned int fg_base, dac_base;
   uint16_t cntrl_reg_wr;
   uint16_t dreq_status = 0;
   struct param_set pset;
   int16_t blk_data[6];
   int status;

   if( channel >= MAX_FG_CHANNELS )
      return -1;

   /* actions per slave card */
   socket = _get_socket( channel );
   dev  = _get_dev( channel );
   if (socket & DEV_SIO)
   {
      scub_status_mil(g_pScub_base, socket & SCU_BUS_SLOT_MASK, &dreq_status);
   }
   else if (socket & DEV_MIL_EXT)
   {
      status_mil(g_pScu_mil_base, &dreq_status);
   }

   // if dreq is active
   if( (dreq_status & MIL_DATA_REQ_INTR) != 0 )
   {
      if( (socket & DEV_SIO) != 0 )
      {
         if( g_aClear_is_active[socket & SCU_BUS_SLOT_MASK] == 0 )
         {
            clear_handler_state(socket);
            hist_addx(HISTORY_XYZ_MODULE, "clear_handler_state", socket);
            g_aClear_is_active[socket & SCU_BUS_SLOT_MASK] = 1;
         }
         // yield
         return -1;
      }

      if( (socket & DEV_MIL_EXT) != 0 )
      {
         if( g_aClear_is_active[MAX_SCU_SLAVES] == 0 )
         {
           clear_handler_state(socket);
           hist_addx(HISTORY_XYZ_MODULE, "clear_handler_state", socket);
           g_aClear_is_active[MAX_SCU_SLAVES] = 1;
         }
         // yield
         return -1;
      }
   }
   else
   {  // reset clear flag
      if( (socket & DEV_SIO) != 0)
      {
         g_aClear_is_active[socket & SCU_BUS_SLOT_MASK] = 0;
      }
      else if( (socket & DEV_MIL_EXT) != 0 )
      {
         g_aClear_is_active[MAX_SCU_SLAVES] = 0;
      }
   }

    /* enable irqs */
   if( (socket & (DEV_MIL_EXT | DEV_SIO)) == 0 )
   {                                      //scu bus slave
      g_pScub_base[SRQ_ENA] |= (1 << (socket-1));           // enable irqs for the slave
      g_pScub_base[OFFS(socket) + SLAVE_INT_ACT] = 0xc000;  // clear all irqs
      g_pScub_base[OFFS(socket) + SLAVE_INT_ENA] |= 0xc000; // enable fg1 and fg2 irq
   }
   else if( (socket & DEV_MIL_EXT) != 0 )
   {
      if( (status = write_mil(g_pScu_mil_base, 1 << 13, FC_IRQ_MSK | dev)) != OKAY)
         dev_failure(status, socket & SCU_BUS_SLOT_MASK, "enable dreq"); //enable Data-Request
   }
   else  if( (socket & DEV_SIO) != 0)
   {
      g_pScub_base[SRQ_ENA] |= (1 << ((socket & SCU_BUS_SLOT_MASK)-1));        // enable irqs for the slave
      g_pScub_base[OFFS(socket & SCU_BUS_SLOT_MASK) + SLAVE_INT_ENA] = 0x0010; // enable receiving of drq
      if ((status = scub_write_mil(g_pScub_base, socket & SCU_BUS_SLOT_MASK, 1 << 13, FC_IRQ_MSK | dev)) != OKAY)
         dev_failure(status, socket & SCU_BUS_SLOT_MASK, "enable dreq"); //enable sending of drq
   }

    /* which macro are we? */
   if ((socket & (DEV_MIL_EXT | DEV_SIO)) == 0)
   {   //scu bus slave
      if (dev == 0)
      {
         fg_base = FG1_BASE;
         dac_base = DAC1_BASE;
      }
      else if (dev == 1)
      {
         fg_base = FG2_BASE;
         dac_base = DAC2_BASE;
      }
      else
         return -1;
   }

   /* fg mode and reset */
   if( (socket & (DEV_MIL_EXT | DEV_SIO)) == 0 )
   {   //scu bus slave
      g_pScub_base[OFFS(socket) + dac_base + DAC_CNTRL] = 0x10;        // set FG mode
      g_pScub_base[OFFS(socket) + fg_base + FG_RAMP_CNT_LO] = 0;       // reset ramp counter
   }
   else if( (socket & DEV_MIL_EXT) != 0 )
   {
      if ((status = write_mil(g_pScu_mil_base, 0x1, FC_IFAMODE_WR | dev)) != OKAY)
         dev_failure (status, 0, "set FG mode"); // set FG mode
   }
   else if( (socket & DEV_SIO) != 0 )
   {
      if ((status = scub_write_mil(g_pScub_base, socket & 0xf, 0x1, FC_IFAMODE_WR | dev)) != OKAY)
         dev_failure (status, socket & 0xf, "set FG mode"); // set FG mode
   }

    //fetch first parameter set from buffer
   if( cbRead(&g_shared.fg_buffer[0], &g_shared.fg_regs[0], channel, &pset) != 0 )
   {
      cntrl_reg_wr = ((pset.control & 0x38) << 10) | ((pset.control & 0x7) << 10) | channel << 4;
      blk_data[0] = cntrl_reg_wr;
      blk_data[1] = pset.coeff_a;
      blk_data[2] = pset.coeff_b;
      blk_data[3] = (pset.control & 0x3ffc0) >> 6;     // shift a 17..12 shift b 11..6
      blk_data[4] = pset.coeff_c & 0xffff;
      blk_data[5] = (pset.coeff_c & 0xffff0000) >> 16; // data written with high word

      if( (socket & (DEV_MIL_EXT | DEV_SIO)) == 0 )
      {
        //set virtual fg number Bit 9..4
         g_pScub_base[OFFS(socket) + fg_base + FG_CNTRL]  = blk_data[0];
         g_pScub_base[OFFS(socket) + fg_base + FG_A]      = blk_data[1];
         g_pScub_base[OFFS(socket) + fg_base + FG_B]      = blk_data[2];
         g_pScub_base[OFFS(socket) + fg_base + FG_SHIFT]  = blk_data[3];
         g_pScub_base[OFFS(socket) + fg_base + FG_STARTL] = blk_data[4];
         g_pScub_base[OFFS(socket) + fg_base + FG_STARTH] = blk_data[5];
      }
      else if( (socket & DEV_MIL_EXT) != 0 )
      {
        // save the coeff_c for mil daq
         g_aFgChannels[channel].last_c_coeff = pset.coeff_c;
        // transmit in one block transfer over the dev bus
         if((status = write_mil_blk(g_pScu_mil_base, &blk_data[0], FC_BLK_WR | dev)) != OKAY)
            dev_failure (status, 0, "blk trm");
        // still in block mode !
         if((status = write_mil(g_pScu_mil_base, cntrl_reg_wr, FC_CNTRL_WR | dev)) != OKAY)
            dev_failure (status, 0, "end blk trm");
      }
      else if( (socket & DEV_SIO) != 0 )
      {
         // save the coeff_c for mil daq
         g_aFgChannels[channel].last_c_coeff = pset.coeff_c;
         // transmit in one block transfer over the dev bus
         if((status = scub_write_mil_blk(g_pScub_base, socket & SCU_BUS_SLOT_MASK, &blk_data[0], FC_BLK_WR | dev))  != OKAY)
            dev_failure (status, socket & SCU_BUS_SLOT_MASK, "blk trm");
         // still in block mode !
         if((status = scub_write_mil(g_pScub_base, socket & SCU_BUS_SLOT_MASK, cntrl_reg_wr, FC_CNTRL_WR | dev))  != OKAY)
            dev_failure (status, socket & SCU_BUS_SLOT_MASK, "end blk trm");
      }
      g_aFgChannels[0].param_sent++;
   }

   /* configure and enable macro */
   if( (socket & (DEV_MIL_EXT | DEV_SIO)) == 0 )
   {
      g_pScub_base[OFFS(socket) + fg_base + FG_TAG_LOW] = g_shared.fg_regs[channel].tag & 0xffff;
      g_pScub_base[OFFS(socket) + fg_base + FG_TAG_HIGH] = g_shared.fg_regs[channel].tag >> 16;
      g_pScub_base[OFFS(socket) + fg_base + FG_CNTRL] |= FG_ENABLED;
   }
   else if( (socket & DEV_MIL_EXT) != 0 )
   { // enable and end block mode
      if ((status = write_mil(g_pScu_mil_base, cntrl_reg_wr | FG_ENABLED, FC_CNTRL_WR | dev)) != OKAY)
         dev_failure (status, 0, "end blk mode");
   }
   else if( (socket & DEV_SIO) != 0 )
   { // enable and end block mode
      if ((status = scub_write_mil(g_pScub_base, socket & SCU_BUS_SLOT_MASK, cntrl_reg_wr | FG_ENABLED, FC_CNTRL_WR | dev)) != OKAY)
         dev_failure (status, socket & SCU_BUS_SLOT_MASK, "end blk mode");
   }

   // reset watchdog
   g_aFgChannels[channel].timeout = 0;
   g_shared.fg_regs[channel].state = STATE_ARMED;
   SEND_SIG(SIG_ARMED);
   hist_addx(HISTORY_XYZ_MODULE, "sig_armed", channel);
   return 0;
}

static inline void printFgs( void )
{
   for( unsigned int  i = 0; i < ARRAY_SIZE( g_shared.fg_macros ); i++ )
   { // hi..lo bytes: socket, device, version, output-bits
      if( g_shared.fg_macros[i] == 0 )
         continue;
      mprintf( "fg-%d-%d ver: %d output-bits: %d\n",
               getMilDaqLocationByChannel( g_shared.fg_macros[i] ),
               getMilDaqAdressByChannel( g_shared.fg_macros[i] ),
               getFgMacroVersion( g_shared.fg_macros[i] ),
               getFgOutputBits( g_shared.fg_macros[i] ));
   }
}

/** @brief scans for fgs on mil extension and scu bus
 */
static void scanFgs( void )
{
   for( unsigned int i = 0; i < ARRAY_SIZE( g_shared.fg_macros ); i++ )
      g_shared.fg_macros[i] = 0;

#if __GNUC__ >= 9
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif
   scan_scu_bus( g_pScub_base,
                 g_pScu_mil_base,
                 &g_shared.fg_macros[0],
                 &g_shared.ext_id);
#if __GNUC__ >= 9
  #pragma GCC diagnostic pop
#endif
   printFgs();
}

/** @brief print the values and states of all channel registers
 */
inline static void print_regs( void)
{
   for( unsigned int i = 0; i < ARRAY_SIZE(g_shared.fg_regs); i++ )
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

/** @brief disable function generator channel
 *  @param channel number of the function generator channel from 0 to MAX_FG_CHANNELS-1
 */
static void disable_channel( unsigned int channel )
{
   struct channel_regs* pFgRegs = &g_shared.fg_regs[channel];

   if( pFgRegs->macro_number == SCU_INVALID_VALUE )
      return;

   int status;
   int16_t data;
   unsigned int socket = _get_socket( channel );
   unsigned int dev  = _get_dev( channel );
   //mprintf("disarmed socket %d dev %d in channel[%d] state %d\n", socket, dev, channel, pFgRegs->state); //ONLY FOR TESTING
   if( (socket & (DEV_MIL_EXT | DEV_SIO)) == 0 )
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
   else if( (socket & DEV_MIL_EXT) != 0 )
   {  // disarm hardware
      if( (status = read_mil(g_pScu_mil_base, &data, FC_CNTRL_RD | dev)) != OKAY )
         dev_failure(status, 0, "disarm hw");

      if( (status = write_mil(g_pScu_mil_base, data & ~(0x2), FC_CNTRL_WR | dev)) != OKAY )
         dev_failure(status, 0, "disarm hw");
   }
   else if( (socket & DEV_SIO) != 0 )
   {  // disarm hardware
      if((status = scub_read_mil(g_pScub_base, socket & SCU_BUS_SLOT_MASK, &data, FC_CNTRL_RD | dev)) != OKAY)
         dev_failure(status, socket & SCU_BUS_SLOT_MASK, "disarm hw");

      if((status = scub_write_mil(g_pScub_base, socket & SCU_BUS_SLOT_MASK, data & ~(0x2), FC_CNTRL_WR | dev)) != OKAY)
         dev_failure(status, socket & SCU_BUS_SLOT_MASK, "disarm hw");
   }

   if( pFgRegs->state == STATE_ACTIVE )
   {    // hw is running
      hist_addx( HISTORY_XYZ_MODULE, "flush circular buffer", channel );
      pFgRegs->rd_ptr =  pFgRegs->wr_ptr;
   }
   else
   {
      pFgRegs->state = STATE_STOPPED;
      SEND_SIG(SIG_DISARMED);
   }
}

/** @brief updates the temperatur information in the shared section
 */
static void updateTemp( void )
{
  BASE_ONEWIRE = (uint8_t*)g_pWr_1wire_base;
  wrpc_w1_init();
#if __GNUC__ >= 9
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif
  ReadTempDevices(0, &g_shared.board_id, &g_shared.board_temp);
  BASE_ONEWIRE = (uint8_t*)g_pUser_1wire_base;
  wrpc_w1_init();
  ReadTempDevices(0, &g_shared.ext_id, &g_shared.ext_temp);
  ReadTempDevices(1, &g_shared.backplane_id, &g_shared.backplane_temp);
#if __GNUC__ >= 9
  #pragma GCC diagnostic pop
#endif
  BASE_ONEWIRE = (uint8_t*)g_pWr_1wire_base; // important for PTP deamon
  wrpc_w1_init();
}

/** @brief initialize the irq table and set the irq mask
 */
static void init_irq_table( void )
{
  isr_table_clr();
  isr_ptr_table[0] = &irq_handler;
  irq_set_mask(0x01);
  g_aMsg_buf[IRQ].ring_head = g_aMsg_buf[IRQ].ring_tail; // clear msg buffer
  irq_enable();
  mprintf("IRQ table configured. 0x%x\n", irq_get_mask());
}

/** @brief initialize procedure at startup
 */
static void init( void )
{

  hist_init(HISTORY_XYZ_MODULE);
  for( int i = 0; i < ARRAY_SIZE(g_shared.fg_regs); i++ )
     g_shared.fg_regs[i].macro_number = SCU_INVALID_VALUE;     //no macros assigned to channels at startup
  updateTemp();                       //update 1Wire ID and temperatures
  scanFgs();                        //scans for slave cards and fgs
}

/** @brief segfault handler, not used at the moment
 */
//void _segfault(int sig)
void _segfault( void )
{
  mprintf(ESC_ERROR"KABOOM!\n"ESC_NORMAL);
  //while (1) {}
}

/** @brief helper function which clears the state of a dev bus after malfunction
 */
static void clear_handler_state( unsigned int socket )
{
   struct msi m;

   if( socket & DEV_SIO )
   {
      // create swi
      m.msg = (socket & SCU_BUS_SLOT_MASK) - 1;
      m.adr = 0;
      irq_disable();
      add_msg(&g_aMsg_buf[0], DEVSIO, m);
      irq_enable();
      return;
   }

   if( socket & DEV_MIL_EXT )
   {
      m.msg = 0;
      m.adr = 0;
      irq_disable();
      add_msg(&g_aMsg_buf[0], DEVBUS, m);
      irq_enable();
   }
}


/*************************************************************
* @brief
* demonstrate how to poll actions ("events") from ECA
* HERE: get WB address of relevant ECA queue
* code written by D.Beck, example.c
*
**************************************************************/
static void findECAQ( void )
{
#define ECAQMAX           4         //  max number of ECA queues
#define ECACHANNELFORLM32 2         //  this is a hack! suggest to implement proper sdb-records with info for queues

  // stuff below needed to get WB address of ECA queue 
  sdb_location ECAQ_base[ECAQMAX]; // base addresses of ECA queues
  uint32_t ECAQidx = 0;            // max number of ECA queues in the SoC

  g_pECAQ = NULL; // Pre-nitialize Wishbone address for LM32 ECA queue

  // get Wishbone addresses of all ECA Queues
  find_device_multi(ECAQ_base, &ECAQidx, ARRAY_SIZE(ECAQ_base), ECA_QUEUE_SDB_VENDOR_ID, ECA_QUEUE_SDB_DEVICE_ID);

  // walk through all ECA Queues and find the one for the LM32
  for( uint32_t i = 0; i < ECAQidx; i++ )
  {
     ECA_T* tmp = (ECA_T*)(getSdbAdr(&ECAQ_base[i]));
     if( (tmp != NULL) && (tmp[ECA_QUEUE_QUEUE_ID_GET / sizeof(ECA_T)] == ECACHANNELFORLM32) )
        g_pECAQ = tmp;
  }

  if( g_pECAQ == NULL )
  {
     mprintf(ESC_ERROR"\nFATAL: can't find ECA queue for lm32, good bye!\n"ESC_NORMAL);
     while(true) asm("nop");
  }
  mprintf("\nECA queue found at: 0x%08x. Waiting for actions with tag 0x%08x ...\n\n", g_pECAQ, MY_ECA_TAG);
} // findECAQ


typedef struct
{
   short     irq_data;      /* saved irq state */
   int       setvalue;      /* setvalue from the tuple sent */
   uint64_t  daq_timestamp; /* timestamp of daq sampling */
} FG_CHANNEL_TASK_T;

#define FSM_DECLARE_STATE( state, attr... ) state
#define FSM_TRANSITION( newState, attr... ) pThis->state = newState
#define FSM_INIT_FSM( init, attr... )      pThis->state = init

typedef enum
{
   FSM_DECLARE_STATE( ST_WAIT,            label='Wait for message' ),
   FSM_DECLARE_STATE( ST_PREPARE,         label='Set MIL task' ),
   FSM_DECLARE_STATE( ST_FETCH_STATUS,    label='Fetch status' ),
   FSM_DECLARE_STATE( ST_HANDLE_IRQS,     label='Handle irq' ),
   FSM_DECLARE_STATE( ST_DATA_AQUISITION, label='Data aquisition' ),
   FSM_DECLARE_STATE( ST_FETCH_DATA,      label='Fetch MIL-DAQ data' )
} FG_STATE_T;

static const char* state2string( const FG_STATE_T state )
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

/* declaration for the scheduler */
typedef struct _TaskType
{
  FG_STATE_T state;
  int slave_nr;                             /* slave nr of the controlling sio card */
  int i;                                    /* loop index for channel */
  int task_timeout_cnt;                     /* timeout counter */
  uint64_t interval;                        /* interval of the task */
  uint64_t lasttick;                        /* when was the task ran last */
  uint64_t timestamp1;                      /* timestamp */
  FG_CHANNEL_TASK_T  aFgChannels[ARRAY_SIZE(g_aFgChannels)];
  void (*func)(struct _TaskType*);          /* pointer to the function of the task */
} TaskType;

/* task prototypes */
static void dev_sio_handler( register TaskType* );
static void dev_bus_handler( register TaskType* );
static void scu_bus_handler( register TaskType* );
static void sw_irq_handler( register TaskType* );
static void ecaHandler( register TaskType* );

/* task configuration table */
static TaskType g_aTasks[] =
{
  { ST_WAIT, 0, 0, 0, ALWAYS, 0, 0, {{0, 0, 0}}, dev_sio_handler }, // sio task 1
  { ST_WAIT, 0, 0, 0, ALWAYS, 0, 0, {{0, 0, 0}}, dev_sio_handler }, // sio task 2
  { ST_WAIT, 0, 0, 0, ALWAYS, 0, 0, {{0, 0, 0}}, dev_sio_handler }, // sio task 3
  { ST_WAIT, 0, 0, 0, ALWAYS, 0, 0, {{0, 0, 0}}, dev_sio_handler }, // sio task 4
  { ST_WAIT, 0, 0, 0, ALWAYS, 0, 0, {{0, 0, 0}}, dev_bus_handler },
  { ST_WAIT, 0, 0, 0, ALWAYS, 0, 0, {{0, 0, 0}}, scu_bus_handler },
  { ST_WAIT, 0, 0, 0, ALWAYS, 0, 0, {{0, 0, 0}}, ecaHandler      },
  { ST_WAIT, 0, 0, 0, ALWAYS, 0, 0, {{0, 0, 0}}, sw_irq_handler  }
};

static unsigned int getId( register TaskType* pThis )
{
   return (((uint8_t*)pThis) - ((uint8_t*)g_aTasks)) / sizeof( TaskType );
}

/*!************************************************************
* @brief
* demonstrate how to poll actions ("events") from ECA
* HERE: poll ECA, get data of action and do something
*
* This example assumes that
* - action for this lm32 are configured by using saft-ecpu-ctl
*   from the host system
* - a TAG with value 0x4 has been configure (see saft-ecpu-ctl -h
*   for help
*
**************************************************************/
static void ecaHandler( register TaskType* pThis UNUSED )
{
   // read flag for the next action and check if there was an action
   if( (g_pECAQ[ECA_QUEUE_FLAGS_GET / sizeof(ECA_T)] & (1 << ECA_VALID)) == 0 )
      return;

   bool dev_mil_armed = false;
   bool dev_sio_armed = false;
   uint32_t active_sios = 0;     // bitmap with active sios

   /* check if there are armed fgs */
   for( unsigned int i = 0; i < ARRAY_SIZE(g_shared.fg_regs); i++ )
   { // only armed fgs
      if( g_shared.fg_regs[i].state != STATE_ARMED )
         continue;
      unsigned int socket = _get_socket( i );
      if( socket & DEV_MIL_EXT )
      {
         dev_mil_armed = true;
         continue;
      }
      if( socket & DEV_SIO )
      {
         active_sios |= (1 << ((socket & SCU_BUS_SLOT_MASK) - 1));
         dev_sio_armed = true;
      }
   }

   // read data tag of action
   ECA_T actTag = g_pECAQ[ECA_QUEUE_TAG_GET / sizeof(ECA_T)];

   // pop action from channel
   g_pECAQ[ECA_QUEUE_POP_OWR / sizeof(ECA_T)] = 0x1;

   // here: do s.th. according to action
   switch( actTag )
   {
      case MY_ECA_TAG:
      {
         // send broadcast start to mil extension
         if (dev_mil_armed)
            g_pScu_mil_base[MIL_SIO3_TX_CMD] = 0x20ff;
         // send broadcast start to active sio slaves
         if( dev_sio_armed )
         {  // select active sio slaves
            g_pScub_base[OFFS(0) + MULTI_SLAVE_SEL] = active_sios;
            // send broadcast
            g_pScub_base[OFFS(13) + MIL_SIO3_TX_CMD] = 0x20ff;
         }
         break;
      }
      default:
      break;
   } // switch
} // ecaHandler

/** @brief software irq handler
 *  dispatch the calls from linux to the helper functions
 *  called via scheduler in main loop
 *  @param pThis pointer to the current task object (not used)
 * @todo Replace the naked numbers for OP-code, use enums defined in
 *       scu_shared_mem.h instead! (UB)
 */
//#define CONFIG_DEBUG_FG
static void sw_irq_handler( register TaskType* pThis )
{
   unsigned int code, value;
   struct msi m;

   if( !has_msg(&g_aMsg_buf[0], SWI) )
      return; /* Nothing to do.. */

   m = remove_msg(&g_aMsg_buf[0], SWI);
   if( m.adr != 0x10 )
      return;

   code = m.msg >> 16;
   value = m.msg & 0xffff;
   //TODO Verifying range of variable "value"
   switch( code )
   {
      case FG_OP_INITIALIZE:
      {
         hist_addx(HISTORY_XYZ_MODULE, "init_buffers", m.msg);
        #if __GNUC__ >= 9
         #pragma GCC diagnostic push
         #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
        #endif
         init_buffers( &g_shared.fg_regs[0],
                       m.msg,
                       &g_shared.fg_macros[0],
                       g_pScub_base,
                       g_pScu_mil_base );
        #if __GNUC__ >= 9
         #pragma GCC diagnostic pop
        #endif
         if( value >= ARRAY_SIZE( g_aFgChannels ) )
         {
            mprintf( ESC_ERROR"Value %d out of range!\n"ESC_NORMAL, value );
            break;
         }
         g_aFgChannels[value].param_sent = 0;
         break;
      }

      case FG_OP_RFU:
      {
         break;
      }

      case FG_OP_CONFIGURE:
      {
         enable_scub_msis(value);
         configure_fg_macro(value);
         break;
      }

      case FG_OP_DISABLE_CHANNEL:
      {
         disable_channel(value);
         break;
      }

      case FG_OP_RESCAN:
      { //rescan for fg macros
         scanFgs();
         break;
      }

      case FG_OP_CLEAR_HANDLER_STATE:
      {
         clear_handler_state(value);
         break;
      }

      case FG_OP_PRINT_HISTORY:
      {
       #ifdef HISTORY
         hist_print(1);
       #else
         mprintf( "No history!\n" );
       #endif
         break;
      }

      default:
      {
         mprintf("swi: 0x%x\n", m.adr);
         mprintf("     0x%x\n", m.msg);
         break;
      }
   }
#ifdef CONFIG_DEBUG_FG
   #warning When CONFIG_DEBUG_FG defined then the timing will destroy!
   mprintf( ESC_FG_CYAN ESC_BOLD"FG-command: %s: %d\n"ESC_NORMAL,
            fgCommand2String( code ), value );
#endif
}

/**
 * @brief task definition of scu_bus_handler
 * called by the scheduler in the main loop
 * decides which action for a scu bus interrupt is suitable
 * @param pThis pointer to the current task object (not used)
 */
static void scu_bus_handler( register TaskType* pThis UNUSED )
{
  uint16_t slv_int_act_reg;
  unsigned char slave_nr;
  uint16_t slave_acks = 0;
  struct msi m;
  signed int dummy;

  if (!has_msg(&g_aMsg_buf[0], SCUBUS))
     return;

  m = remove_msg(&g_aMsg_buf[0], SCUBUS);
  if (m.adr != 0x0)
     return;

  slave_nr = m.msg + 1;
  if (slave_nr < 0 || slave_nr > MAX_SCU_SLAVES) {
     mprintf(ESC_ERROR"slave nr unknown.\n"ESC_NORMAL);
     return;
  }

  slv_int_act_reg = g_pScub_base[OFFS(slave_nr) + SLAVE_INT_ACT];
  if (slv_int_act_reg & 0x1) {// powerup interrupt
    slave_acks |= 0x1;
  }
  if (slv_int_act_reg & FG1_IRQ) { //FG irq?
    handle(slave_nr, FG1_BASE, 0, &dummy);
    slave_acks |= FG1_IRQ;
  }
  if (slv_int_act_reg & FG2_IRQ) { //FG irq?
    handle(slave_nr, FG2_BASE, 0, &dummy);
    slave_acks |= FG2_IRQ;
  }
  if (slv_int_act_reg & DREQ) { //DRQ irq?
    add_msg(&g_aMsg_buf[0], DEVSIO, m);
    slave_acks |= DREQ;
  }
  g_pScub_base[OFFS(slave_nr) + SLAVE_INT_ACT] = slave_acks; // ack all pending irqs
}

#define CONFIG_LAGE_TIME_DETECT
#ifdef CONFIG_LAGE_TIME_DETECT
   static uint64_t lastTime = 0;
#endif
static void pushDaqData( uint32_t channel, uint64_t timestamp,
                         short actValue, uint32_t setValue )
{
#ifdef CONFIG_LAGE_TIME_DETECT
   if( lastTime > 0 )
   {
      if( (timestamp - lastTime) > 100000000ULL )
         mprintf( ESC_WARNING"Time-gap!\n"ESC_NORMAL );
   }
   lastTime = timestamp;
#endif
#ifdef CONFIG_MIL_DAQ_USE_RAM
#error Extern RAM for MIL-DAQ not implemented yet!
   MIL_DAQ_RAM_ITEM_PAYLOAD_T pl;
   pl.item.timestamp = timestamp;
   pl.item.setValue = setValue >> (BIT_SIZEOF(uint32_t)/sizeof(MIL_DAQ_T));
   pl.item.actValue = actValue;
   pl.item.channel = channel;
#else
   struct daq d;

   d.actvalue = actValue;
   d.tmstmp_l = timestamp & 0xffffffff;
   d.tmstmp_h = timestamp >> 32;
   d.channel  = channel;
   d.setvalue = setValue;
   add_daq_msg(&g_shared.daq_buf, d);
#endif
   hist_addx(HISTORY_XYZ_MODULE, "daq_high", actValue >> 8);
   hist_addx(HISTORY_XYZ_MODULE, "daq_low", actValue & 0xff);
}

static void printTimeoutMessage( register TaskType* pThis, const bool isScuBus )
{
   mprintf( ESC_WARNING"timeout %s: state %s, taskid %d index %d\n"ESC_NORMAL,
            isScuBus? "dev_bus_handler" : "dev_sio_handler",
            state2string( pThis->state ),
            getId( pThis ),
            pThis->i );
}

static void dev_sio_bus_handler( register TaskType* pThis, const bool isScuBus )
{
   int i;
   unsigned int socket, dev;
   int status = OKAY;
   short data_aquisition;
   struct msi m;

   switch( pThis->state )
   {
      case ST_WAIT:
      {  // we have nothing to do
         if( !has_msg(&g_aMsg_buf[0], isScuBus? DEVSIO : DEVBUS) )
         {
         #ifdef __DOCFSM__
            FSM_TRANSITION( ST_WAIT, label='No message' );
         #endif
            break;
         }

         m = remove_msg( &g_aMsg_buf[0], isScuBus? DEVSIO : DEVBUS );
         if( isScuBus )
            pThis->slave_nr = m.msg + 1;
         pThis->timestamp1 = getSysTime();
         FSM_TRANSITION( ST_PREPARE, label='Massage received' );
         break; //yield
      } // end case ST_WAIT

      case ST_PREPARE:
      {
         // wait for 200 us
         if( getSysTime() < (pThis->timestamp1 + INTERVAL_200US) )
         {
         #ifdef __DOCFSM__
            FSM_TRANSITION( ST_PREPARE, label='Interval not expired' );
         #endif
            break; //yield
         }
         /* poll all pending regs on the dev bus; non blocking read operation */
         for( i = 0; i < MAX_FG_CHANNELS; i++ )
         {
            socket = _get_socket( i );
            dev  = _get_dev( i );
            pThis->aFgChannels[i].irq_data = 0; // clear old irq data
            /* test only ifas connected to sio */
            status = OKAY;
            if( isScuBus )
            {
               if( ((socket & SCU_BUS_SLOT_MASK) != pThis->slave_nr ) || ((socket & DEV_SIO) == 0) )
                  continue;
               status = scub_set_task_mil( g_pScub_base,
                                           pThis->slave_nr,
                                           getId( pThis ) + i + 1,
                                           FC_IRQ_ACT_RD | dev );
            }
            else
            {
               if( (socket & DEV_MIL_EXT) == 0 )
                  continue;
               status = set_task_mil( g_pScu_mil_base,
                                      getId( pThis ) + i + 1,
                                      FC_IRQ_ACT_RD | dev);
            }
            if( status != OKAY )
               dev_failure( status, 20, "dev_sio set task" );
         }  // end for
         pThis->i = 0;
         FSM_TRANSITION( ST_FETCH_STATUS );
         break;
      }

      case ST_FETCH_STATUS:
      {
         /* if timeout reached, proceed with next task */
         if( pThis->task_timeout_cnt > TASK_TIMEOUT )
         {
            printTimeoutMessage( pThis, isScuBus );
            pThis->i++;
            pThis->task_timeout_cnt = 0;
         }
         /* fetch status from dev bus controller; */
         status = OKAY;
         for( i = pThis->i; i < MAX_FG_CHANNELS; i++ )
         {
            socket = _get_socket( i );
            /* test only ifas connected to sio */
            if( isScuBus )
            {
               if( ((socket & SCU_BUS_SLOT_MASK) != pThis->slave_nr ) || ((socket & DEV_SIO) == 0) )
                  continue;
               status = scub_get_task_mil( g_pScub_base, pThis->slave_nr,
                                           getId( pThis ) + i + 1,
                                           &pThis->aFgChannels[i].irq_data );
            }
            else
            {
               if( (socket & DEV_MIL_EXT) == 0 )
                  continue;
               status = get_task_mil( g_pScu_mil_base, getId( pThis ) + i + 1,
                                      &pThis->aFgChannels[i].irq_data );
            }
            if( status == RCV_TASK_BSY )
               break; // break from for loop
            if( status != OKAY )
               mil_failure( status, pThis->slave_nr );
         } // end for
         if( status == RCV_TASK_BSY )
         {
            pThis->i = i; // start next time from i
            pThis->task_timeout_cnt++;
         #ifdef __DOCFSM__
            FSM_TRANSITION( ST_FETCH_STATUS, label='receive busy' );
         #endif
            break; //yield
         }

         pThis->i = 0; // start next time from 0
         pThis->task_timeout_cnt = 0;
         FSM_TRANSITION( ST_HANDLE_IRQS );
         break;
      } // end case ST_FETCH_STATUS

      case ST_HANDLE_IRQS:
      {  /* handle irqs for ifas with active pending regs; non blocking write */
         status = OKAY;
         for( i = 0; i < MAX_FG_CHANNELS; i++ )
         {  // any irq pending?
            if( (pThis->aFgChannels[i].irq_data & (DEV_STATE_IRQ | DEV_DRQ)) == 0 )
               continue; // No

            socket = _get_socket( i );
            dev  = _get_dev( i );
            handle( socket, dev, pThis->aFgChannels[i].irq_data,
                    &(pThis->aFgChannels[i].setvalue));
            //clear irq pending and end block transfer
            if( isScuBus )
            {
               status = scub_write_mil( g_pScub_base, pThis->slave_nr,
                                        0, FC_IRQ_ACT_WR | dev);
            }
            else
            {
               status = write_mil( g_pScu_mil_base, 0, FC_IRQ_ACT_WR | dev);
            }
            if( status != OKAY )
               dev_failure(status, 22, "dev_sio end handle");
         } // end for
         FSM_TRANSITION( ST_DATA_AQUISITION );
         break;
      } // end case ST_HANDLE_IRQS

      case ST_DATA_AQUISITION:
      {  /* data aquisition */
         for( i = 0; i < MAX_FG_CHANNELS; i++ )
         {  // any irq pending?
            if( (pThis->aFgChannels[i].irq_data & (DEV_STATE_IRQ | DEV_DRQ)) == 0 )
               continue; // No

            pThis->aFgChannels[i].daq_timestamp = getSysTime(); // store the sample timestamp of daq
            dev = _get_dev( i );
            // non blocking read for DAQ
            if( isScuBus )
            {
               status = scub_set_task_mil( g_pScub_base, pThis->slave_nr,
                                          getId( pThis ) + i + 1,
                                          FC_ACT_RD | dev);
            }
            else
            {
               status = set_task_mil( g_pScu_mil_base, getId( pThis ) + i + 1,
                                     FC_ACT_RD | dev);
            }
            if( status != OKAY )
               dev_failure(status, 23, "dev_sio read daq");
         } // end for
         FSM_TRANSITION( ST_FETCH_DATA );
         break;
      } // end case ST_DATA_AQUISITION

      case ST_FETCH_DATA:
      {
         /* if timeout reached, proceed with next task */
         if( pThis->task_timeout_cnt > TASK_TIMEOUT )
         {
            printTimeoutMessage( pThis, isScuBus );
            pThis->i++;
            pThis->task_timeout_cnt = 0;
         }
         /* fetch daq data */
         for( i = pThis->i; i < MAX_FG_CHANNELS; i++ )
         {  // any irq pending?
            if( (pThis->aFgChannels[i].irq_data & (DEV_STATE_IRQ | DEV_DRQ)) == 0 )
               continue; // No

            if( isScuBus )
            {
               status = scub_get_task_mil( g_pScub_base, pThis->slave_nr,
                                           getId( pThis ) + i + 1, &data_aquisition);
            }
            else
            {
               status = get_task_mil( g_pScu_mil_base, getId( pThis ) + i + 1,
                                      &data_aquisition);
            }

            if( status == RCV_TASK_BSY )
               break; // break from for loop

            if( status != OKAY )
            {
               mil_failure( status, pThis->slave_nr );
               /* TODO Why not break from loop? */
            }

            pushDaqData( _get_macro_number( i ),
                         pThis->aFgChannels[i].daq_timestamp,
                         data_aquisition,
                         g_aFgChannels[i].last_c_coeff );
            // save the setvalue from the tuple sent for the next drq handling
            g_aFgChannels[i].last_c_coeff = pThis->aFgChannels[i].setvalue;
         } // end for

         if( status == RCV_TASK_BSY )
         {
            pThis->i = i; // start next time from i
            pThis->task_timeout_cnt++;
         #ifdef __DOCFSM__
            FSM_TRANSITION( ST_FETCH_DATA, label='Receiving busy' );
         #endif
            break; //yield
         }
         pThis->i = 0; // start next time from 0
         pThis->task_timeout_cnt = 0;
         FSM_TRANSITION( ST_WAIT );
         break;
      } // end case ST_FETCH_DATA

      default:
      {
         mprintf(ESC_ERROR"unknown state of dev bus handler!\n"ESC_NORMAL);
         FSM_INIT_FSM( ST_WAIT );
         break;
      }
   } // end switch
}


/**
 * @brief can have multiple instances, one for each active sio card controlling a dev bus
 * persistent data, like the state or the sio slave_nr, is stored in a global structure
 * @param pThis pointer to the current task object
 */
static void dev_sio_handler( register TaskType* pThis )
{
   dev_sio_bus_handler( pThis, false );
}
/**
 * @brief has only one instance
 * persistent data is stored in a global structure
 * @param pThis pointer to the current task object
 */
static void dev_bus_handler( register TaskType* pThis )
{
   dev_sio_bus_handler( pThis, true );
}


#if 0
static uint64_t tick = 0;               // system tick
uint64_t getTick( void ) {
  return tick;
}
#endif
/* definition of task dispatch */
/* move messages to the correct queue, depending on source */
static inline void dispatch( void )
{
   struct msi m;
   m = remove_msg(&g_aMsg_buf[0], IRQ);
   switch( m.adr & 0xff )
   {
      case 0x00: add_msg(&g_aMsg_buf[0], SCUBUS, m); return; // message from scu bus
      case 0x10: add_msg(&g_aMsg_buf[0], SWI,    m); return; // software message from saftlib
      case 0x20: add_msg(&g_aMsg_buf[0], DEVBUS, m); return; // message from dev bus
   }
}

static inline void printCpuId( void )
{
   unsigned int* cpu_info_base;
   cpu_info_base = (unsigned int*)find_device_adr(GSI, CPU_INFO_ROM);
   if((int)cpu_info_base == ERROR_NOT_FOUND)
   {
      mprintf(ESC_ERROR"no CPU INFO ROM found!\n"ESC_NORMAL);
      return;
   }
   mprintf("CPU ID: 0x%x\n", cpu_info_base[0]);
   mprintf("number MSI endpoints: %d\n", cpu_info_base[1]);
}

/**
 * @brief after the init phase at startup, the scheduler loop runs forever
 */
int main( void )
{
  int mb_slot;

 // unsigned int* cpu_info_base;
 // sdb_location found_sdb[20];
 // uint32_t clu_cb_idx = 0;
//  cpu_info_base = (unsigned int*)find_device_adr(GSI, CPU_INFO_ROM);
  initializeGlobalPointers();
 // find_device_multi(&found_sdb[0], &clu_cb_idx, ARRAY_SIZE(found_sdb), GSI, LM32_CB_CLUSTER); // find location of cluster crossbar

  mprintf("Compiler: "COMPILER_VERSION_STRING"\n"
          "Found MsgBox at 0x%08x. MSI Path is 0x%08x\n", (uint32_t)pCpuMsiBox, (uint32_t)pMyMsi);

  mb_slot = getMsiBoxSlot(0x10);
  if (mb_slot == -1)
    mprintf(ESC_ERROR"No free slots in MsgBox left!\n"ESC_NORMAL);
  else
    mprintf("Configured slot %d in MsgBox\n", mb_slot);
  g_shared.fg_mb_slot = mb_slot; //tell saftlib the mailbox slot for sw irqs

  init_irq_table();

  msDelayBig(1500); //wait for wr deamon to read sdbfs

  if ((int)BASE_SYSCON == ERROR_NOT_FOUND)
    mprintf(ESC_ERROR"no SYS_CON found!\n"ESC_NORMAL);
  else
    mprintf("SYS_CON found on adr: 0x%x\n", BASE_SYSCON);

  timer_init(1); //needed by usleep_init() 
  usleep_init();

  printCpuId();
  mprintf("g_pWr_1wire_base is: 0x%x\n", g_pWr_1wire_base);
  mprintf("g_pUser_1wire_base is: 0x%x\n", g_pUser_1wire_base);
  mprintf("g_pScub_irq_base is: 0x%x\n", g_pScub_irq_base);
  mprintf("g_pMil_irq_base is: 0x%x\n", g_pMil_irq_base);
  findECAQ();

  init(); // init and scan for fgs

#ifdef CONFIG_SCU_DAQ_INTEGRATION
  scuDaqInitialize( &g_scuDaqAdmin ); // Init and scan for DAQs
  mprintf( "SCU-DAQ initialized\n" );
#endif

  while( true )
  {
    check_stack();
    uint64_t tick = getSysTime(); /* FIXME get the current system tick */

    // loop through all task: if interval is 0, run every time, otherwise obey interval
    for( unsigned int i = 0; i < ARRAY_SIZE( g_aTasks ); i++ )
    {
      // call the dispatch task before every other task
      dispatch();
      TaskType* pCurrent = &g_aTasks[i];
      if( (tick - pCurrent->lasttick) < pCurrent->interval )
      {
         continue;
      }
      pCurrent->func( pCurrent );
      pCurrent->lasttick = tick;
    }
  #ifdef CONFIG_SCU_DAQ_INTEGRATION
    forEachScuDaqDevice();
  #endif
  }
  return 0;
}

/*================================== EOF ====================================*/
