/* Bunch-to-bucket transfer */
/* Author: Jiaoni Bai */
/* Date: 04.11.2015 */

/* Code for source B2B SCU */
/* ===================================================================================
Soure B2B SCU
      Slot 1-2>> Phase advance prediction module
      Slot 3-4>> Phase correction module
      Slot 5-6>> Phase shift module
      Slot 7-8>> Reference Group DDS
===================================================================================*/

/* C Standard Includes*/
/*====================================================================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>

/* GSI LM32 Includes */
/*====================================================================================*/
#include "mprintf.h"
#include "mini_sdb.h"
#include "ebm.h"
#include "aux.h"
#include "hw-tlu.h"
#include "time_counter.h"
#include "eca_queue_regs.h"
#include "B2B_event.h"

/* Vender ID and device ID */
#define venID 0x651
#define devID_SCUBM 0x9602eb6f
#define devID_EBM   0x00000815
#define devID_ECA   0xd5a3faea
#define devID_TLU   0x10051981
#define devID_TIME  0x53bee0e2
#define LM32_CB_CLUSTER       0x10041000
#define LM32_IRQ_EP           0x10050083

/* #define devID_ECA   0x9602eb6f for timestamp */

/* SCU Slave address */
/*====================================================================================
  E.g. the slave is at slot 5
  Base Address for the slave is 5 * 2^17 + 0x400000 = 0x4a0000
  The Phase Advance Prediction module is in slot 1
  The Phase Correction module is in slot 2
  The Phase Shift module is in slot 3
====================================================================================
  EVT_B2B_START timestamp must be aligned with a T0 edge, tm + 100us corresponds to
  the PAP predicted phase.

====================================================================================*/
/* SCU slave Phase Advance Prediction address offset: read */
#define OS_PAP 0x6
/* SCU slave Phase Correction address offset: write */
#define OS_PC 0x10
/* SCU slave Phase Shift address offset: write */
#define OS_PS 0x10

/* Timing message*/
#define Ebm_WRITE 0x400000
#define WB_Addr   0x7ffffff0
#define FID_GID   0x33333333
#define Param     0x4444444455555555
#define Timestamp 0x1234567812345678

#define Source_B2B_SCU 1
#define Target_B2B_SCU 0

#define SCALE 1000000000000 //ps

#define EVT_START_B2B_tag   0x0B2B0001
#define TGM_PHASE_TIME_tag  0x0B2B0001
#define TGM_SYNCH_WIN_tag   0x0B2B0003


#define Phase_Shift 0
#define Frequency_Beating 0

#define Phase_Shift_Time 9000000000 % unit ps

/* Timining message structure */
#define Param_os      0x4
#define TEF_os        (Param_os + 0x4)
#define Reserved_os   (TEF_os + 0x2)
#define Timestamp_os  (Reserved_os + 0x2)


/* variable to the base address */
volatile unsigned short* pSCUbm = 0;
volatile unsigned short* pEBm   = 0;
volatile unsigned int* pECA_Q = 0;
volatile unsigned      * pTLU   = 0;
volatile unsigned      * pTIME  = 0;


/* variables for h1 phase read from the PAP module */
static uint16_t predicted_phase_h1_src = 0;
static uint16_t predicted_phase_ts = 0;
static uint16_t predicted_phase_h1_trg = 0;
/* variables for calculated high harmonic phase */
static uint16_t phase_high_harmonic_src = 0;
static uint16_t phase_high_harmonic_trg = 0;
/* variables for calculated phase correction and phase shift for the source machine */
static uint16_t phase_correction_h1_src = 0;
static uint16_t phase_shift_high_src = 0;
/* variables for the parameters for source and target machines */
// source and target machine high harmonic rf frequencies
static uint32_t freq_high_harmonic_src = 1572200;
static uint32_t freq_high_harmonic_trg = 1572000;
//source and target machines rf harmonic number
static uint16_t harmonic_src = 2;
static uint16_t harmonic_trg = 10;
//source and target machine rf period with cavity harmonics: uint ps
static uint64_t period_high_harmonic_src = 0;
static uint64_t period_high_harmonic_trg = 0;
//source and target machine rf requency with h=1 : uint ps
static uint64_t period_h1_src = 0;
static uint64_t period_h1_trg = 0;
//source and target machine tm for the zero-crossing of high harmonics
static uint64_t tm_high_zero_trg = 0;
//timestamp for the predicted phase of PAP
static uint64_t predict_phase_ts = 0;
//the start time of the B2B transfer system
static uint64_t EVT_B2B_START_ts = 0;
// the TOF
static uint64_t tof_time = 0;
// the predicted phase uncertainty
static uint16_t predicted_phase_uncertainty_time = 0;
static uint64_t syn_win_start_tm = 0;

sdb_location lm32_irq_endp[10];


unsigned int cpuId;

enum B2B_WAIT_machine {
  IDLE,
  WAIT_b2b_start,
  WAIT_phase_to_src,
  WAIT_sync_window,
  WAIT_phase_corr,
  WAIT_phase_jump,
  WAIT_trigger_ts_s,
  WAIT_trigger_ts_t,
  WAIT_kicker_ts_s,
  WAIT_kicker_ts_t
};
int procedure_error (enum B2B_WAIT_machine B2B_example)
{
  mprintf("Procedure Error: %s\n", B2B_example);
  return -1;
}
/* Etherbone master send Eb telegram */
void ebmInit()
{
  ebm_init();
  //config the source and destination MAC, ip, port
  ebm_config_if(LOCAL, "hw/00:26:7b:00:03:d7/udp/192.168.0.1/port/60368");
  ebm_config_if(REMOTE, "hw/ff:ff:ff:ff:ff:ff/udp/192.168.0.2/port/60369");
  ebm_config_meta(80, 0x11, 16, 0x00000000 );
}

void init()
{
  /* Get uart unit address */
  discoverPeriphery();
  uart_init_hw();
  ebmInit();
  cpuId = getCpuIdx();
  //init_irq_handler();

}

void event_get_from_queue (uint32_t event_param_hi, uint32_t event_param_lo, uint32_t event_ex_hi, uint32_t event_ex_lo)
{
  mprintf("/************************TIMING EVENT************************/\n");
  event_param_hi   = *(pECA_Q +(ECA_QUEUE_PARAM_HI_GET >> 2));
  event_param_lo   = *(pECA_Q +(ECA_QUEUE_PARAM_LO_GET >> 2));
  event_ex_hi      = *(pECA_Q +(ECA_QUEUE_EXECUTED_HI_GET >> 2));
  event_ex_lo      = *(pECA_Q +(ECA_QUEUE_EXECUTED_LO_GET >> 2));
  //Pop the ECA queue
  *(pECA_Q +(ECA_QUEUE_POP_OWR >> 2)) = 0x1;

  mprintf(" EVENT_PARAM_HI:%x\n EVENT_PARAM_LO:%x\n EVENT_EX_HI:%x\n EVENT_EX_LO:%x \n",event_param_hi,event_param_lo,event_ex_hi,event_ex_lo);
}

/* TLU simulates the next zero cross timestamp of two machines */
// B2 is used for the src B2B SCU, channel 1 for TLU
uint64_t get_rising_edge_tm_from_FG ()
{
  mprintf("/************************TLU************************/\n");
  uint32_t trg_ts_hi;
  uint32_t trg_ts_lo;
  //uint32_t B2_timestamp_hi;
  //uint32_t B2_timestamp_lo;
  uint32_t count;

  /* TLU configure */
  /*Configure the TLU to record rising edge timestamps at the same time*/
  *(pTLU + (TLU_ACTIVE_SET >> 2)) = 0x3;
  *(pTLU + (TLU_CH_SELECT >> 2)) = 0x1;
  count = *(pTLU + (TLU_CH_FILL_COUNT >> 2));
  mprintf("TLU active %x \n",count);
  //clear channel 0 (B1) 0x1, channel 1 (B2) 0x2
  *(pTLU + (TLU_CLEAR >> 2)) = 0x2;
  count = *(pTLU + (TLU_CH_FILL_COUNT >> 2));
  mprintf("TLU clear activate %x \n",count);
  //usleep (1000000);
  //for the 1Hz signal, must give delay before to get the ts
  *(pTLU + (TLU_CH_POP >> 2)) = 0x1;
  trg_ts_hi = *(pTLU + (TLU_CH_TIME1 >> 2));
  trg_ts_lo = *(pTLU + (TLU_CH_TIME0 >> 2));
  tm_high_zero_trg = ((uint64_t)trg_ts_hi << 32) | trg_ts_lo;
  mprintf("SIS18 src B2B SCU rf simulation timestamp (B2) %x %x\n",trg_ts_hi,trg_ts_lo);

  return tm_high_zero_trg;
}

int ebm_send_msg (uint32_t WB_Addr_t, uint64_t eventID, uint64_t Param_t, uint32_t TEF, uint32_t Reserved, uint64_t Timestamp_t)
{
  mprintf("/************************EBm************************/\n");
  uint32_t EventID_H = (uint32_t) ((eventID & 0xffffffff00000000 ) >> 32);
  uint32_t EventID_L = (uint32_t) (eventID & 0x00000000ffffffff );
  mprintf("eventID = %x, eventID = %x",EventID_H, EventID_L);
  uint32_t Param_H   = (uint32_t)((Param_t & 0xffffffff00000000 ) >> 32);
  uint32_t Param_L   = (uint32_t)(Param_t & 0x00000000ffffffff );
  //uint32_t TEF       = 0x11111111;
  //uint32_t Reserved  = 0x22222222;
  uint32_t Timestamp_H = (uint32_t)((Timestamp_t & 0xffffffff00000000 ) >> 32);
  uint32_t Timestamp_L = (uint32_t)(Timestamp_t & 0x00000000ffffffff );
  //creat WB cy (uint32_t)(Param & 0x00000000ffffffff );  cle : WRITE
  //Format of Timing Message : 32 bits WB Addr + 256 bits Payload
  //32 bits WB Addr
  ebm_hi(WB_Addr);
  atomic_on();
  //256 bits Payload
  ebm_op(WB_Addr_t, EventID_H  , Ebm_WRITE);
  ebm_op(WB_Addr_t, EventID_L  , Ebm_WRITE);
  ebm_op(WB_Addr_t, Param_H    , Ebm_WRITE);
  ebm_op(WB_Addr_t, Param_L    , Ebm_WRITE);
  ebm_op(WB_Addr_t, TEF        , Ebm_WRITE);
  ebm_op(WB_Addr_t, Reserved   , Ebm_WRITE);
  ebm_op(WB_Addr_t, Timestamp_H, Ebm_WRITE);
  ebm_op(WB_Addr_t, Timestamp_L, Ebm_WRITE);
  atomic_off();
  ebm_flush();

}

/* Function main */
int main (void)
{
  mprintf ("----------------------Target B2B SCU--------------------------\n ");
  init();
  enum B2B_WAIT_machine trg_b2b_scu=IDLE;

  *(pECA_Q +(ECA_QUEUE_POP_OWR >> 2)) = 0x1;
  uint32_t eca_queue_flag;
  uint32_t event_id_hi;
  uint32_t event_id_lo;
  uint32_t event_tag;
  uint32_t event_tef;
  uint32_t param_hi;
  uint32_t param_lo;
  uint32_t ts_ex_hi;
  uint32_t ts_ex_lo;
  uint64_t ts_now;
  //timiestamp of the zero crossing point from the target function generator
  uint32_t trg_tm_hi;
  uint32_t trg_tm_lo;

  //timestamp of the trigger pulse from the src Trigger SCU
  uint32_t trigger_ts_s_hi;
  uint32_t trigger_ts_s_lo;
  //timestamp of the beam extraction from the src Kikcer SCU
  uint32_t kicker_ts_s_hi;
  uint32_t kicker_ts_s_lo;

  //timestamp of the trigger pulse from the trg Trigger SCU
  uint32_t trigger_ts_t_hi;
  uint32_t trigger_ts_t_lo;
  //timestamp of the beam extraction from the trg Kikcer SCU
  uint32_t kicker_ts_t_hi;
  uint32_t kicker_ts_t_lo;

  int chose_frequency_beating_method = 1;

  uint32_t fre_src;
  uint32_t fre_trg;
  uint32_t phase_shift_time;
  sdb_location found_sdb[20];
  uint32_t lm32_endp_idx = 0;
  uint32_t clu_cb_idx = 0;
  uint64_t phase_h1_tof;
  uint64_t phase_high_harmonic_tof;
  uint64_t * phase_high_h_st;
  uint64_t * tm_high_0_st;
  uint32_t synchronization_window_uncertainty;
  uint64_t frequency_beating_time;

  /* Function time measurement */
  uint32_t start_tai;
  uint32_t start_cycle;
  uint32_t stop_tai;
  uint32_t stop_cycle;

  int idx_four_ts = 0;
  uint32_t  Restriction1_from_SM;
  uint32_t  Restriction2_from_SM;

  bool phase_to_src_en   = false;
  bool trigger_kicker_en = false;

  uint32_t trg_ts_hi;
  uint32_t trg_ts_lo;

  int i = 0;

  const char*b2b_procedure;
  //source and target machine rf period with cavity harmonics
  period_high_harmonic_src = SCALE/freq_high_harmonic_src;
  period_high_harmonic_trg = SCALE/freq_high_harmonic_trg;
  //source and target machine rf requency with h=1
  period_h1_src = SCALE/freq_high_harmonic_src * harmonic_src;
  period_h1_trg = SCALE/freq_high_harmonic_trg * harmonic_trg;

  phase_h1_tof = tof_time % period_h1_src * 0xFFFF / period_h1_src;
  phase_high_harmonic_tof = tof_time % period_high_harmonic_src * 0xFFFF / period_high_harmonic_src;;

  mprintf ("SIS18  h=1 period  =  %d(ps)\n",(uint32_t)period_h1_src);
  mprintf ("SIS100 h=1 period  =  %d(ps)\n",(uint32_t)period_h1_trg);
  mprintf ("SIS18  high harmonic period = %d(ps)\n",(uint32_t)period_high_harmonic_src);
  mprintf ("SIS100 high harmonic period = %d(ps)\n",(uint32_t)period_high_harmonic_trg);
  init();
  //SM_param_load(fre_src,fre_trg,har_src,har_trg,phase_shift_time, tof_time, kicker_delay, rst_fmax1, rst_fmax2);

  /* Base address of the related ep cores */
  pSCUbm =(unsigned short*)find_device_adr(venID,devID_SCUBM);
  pEBm   = (unsigned int*)find_device_adr(venID,devID_EBM);
  pECA_Q   = 0xa00000c0;
  //pECA_Q = (unsigned int*)find_device_adr(venID,devID_ECA);
  pTLU   = (unsigned int*)find_device_adr(venID,devID_TLU);
  pTIME  = (unsigned int*)find_device_adr(venID,devID_TIME);

  mprintf("base address %x\n",pTLU);
 // find_device_multi(&found_sdb[0], &clu_cb_idx, 20, GSI, LM32_CB_CLUSTER); // find location of cluster crossbar
  //find_device_multi_in_subtree(&found_sdb[0], lm32_irq_endp, &lm32_endp_idx, 10, GSI, LM32_IRQ_EP); // list irq endpoints in cluster crossbar
  //for (int i=0; i < lm32_endp_idx; i++)
  //{
  //  mprintf("irq_endp[%d] is: 0x%x\n",i, getSdbAdr(&lm32_irq_endp[i]));
  //}


  while (1)
  {
    eca_queue_flag = *(pECA_Q +(ECA_QUEUE_FLAGS_GET >> 2));
    event_id_hi    = *(pECA_Q +(ECA_QUEUE_EVENT_ID_HI_GET >> 2));
    event_id_lo    = *(pECA_Q +(ECA_QUEUE_EVENT_ID_LO_GET >> 2));
    event_tag      = *(pECA_Q +(ECA_QUEUE_TAG_GET >> 2));
    event_tef      = *(pECA_Q +(ECA_QUEUE_TEF_GET >> 2));

    //select event based on Event ID, maybe later tag is also needed.
    if (eca_queue_flag && 0x00000010 == 0x10 && event_id_hi == ((EVT_START_B2B >> 32) & 0xffffffff) && event_id_lo == (EVT_START_B2B & 0xffffffff))
    {
      if (phase_to_src_en == false)
      {
        trg_b2b_scu = WAIT_b2b_start;
        phase_to_src_en = true;
      }
      else
      {
        trg_b2b_scu = IDLE;
      }
    }

    switch(trg_b2b_scu)
    {
      case WAIT_b2b_start:
        //Pop the EVT_START_B2B from queue
        *(pECA_Q +(ECA_QUEUE_POP_OWR >> 2)) = 0x1;
        /* Function Generator simulates the zero crossing point of high harmonic. TLU gets the timestamp of source TTL signal */
        get_rising_edge_tm_from_FG ();
        //event_get_from_queue(param_hi,param_lo,ts_ex_hi,ts_ex_lo);
        ts_now = ((uint64_t)ts_ex_hi << 32) | ts_ex_lo;
        mprintf("^^^^^^^^^time HI: %x LO:%x\n ts_ex_hi,ts_ex_lo");

        ebm_send_msg (WB_Addr, TGM_PHASE_TIME, tm_high_zero_trg, 0,0,0);
        //ebm_send_msg (WB_Addr, 0xdeadbeef22222222, 0x1234567811223344, 0x87654321,0xaabbccdd,0x9988776655443322);
        break;

      default: break;
    }

  }
}



