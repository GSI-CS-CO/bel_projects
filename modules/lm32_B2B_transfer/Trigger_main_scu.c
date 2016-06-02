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
static uint64_t tm_high_zero_src = 0;
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

void event_get_from_queue (uint32_t event_param_hi, uint32_t event_param_lo)
{
  mprintf("/************************TIMING EVENT************************/\n");
  event_param_hi   = *(pECA_Q +(ECA_QUEUE_PARAM_HI_GET >> 2));
  event_param_lo   = *(pECA_Q +(ECA_QUEUE_PARAM_LO_GET >> 2));
  //Pop the ECA queue
  *(pECA_Q +(ECA_QUEUE_POP_OWR >> 2)) = 0x1;

  mprintf("EVENT_ID_HI : %x\n EVENT_ID_LO : %x\n EVENT_PARAM_HI:%x\n EVENT_PARAM_LO:%x\n EVENT_TAG:%x\n EVENT_TEF:%x \n",*(pECA_Q +(ECA_QUEUE_EVENT_ID_HI_GET >> 2)), *(pECA_Q +(ECA_QUEUE_EVENT_ID_LO_GET >> 2)),event_param_hi,event_param_lo);
}

/* Function main */
int main (void)
{
  mprintf ("----------------------Trigger SCU--------------------------\n ");
  init();
  enum B2B_WAIT_machine trigger_scu=IDLE;

  *(pECA_Q +(ECA_QUEUE_POP_OWR >> 2)) = 0x1;
  uint32_t eca_queue_flag;
  uint32_t event_id_hi;
  uint32_t event_id_lo;
  uint32_t event_tag;
  uint32_t event_tef;
  //timiestamp of the zero crossing point from the target function generator
  uint32_t trg_tm_hi;
  uint32_t trg_tm_lo;

  //timestamp of the trigger pulse from the src Trigger SCU
  uint32_t trigger_ts_s_hi;
  uint32_t trigger_ts_s_lo;

  /* Function time measurement */
  uint32_t start_tai;
  uint32_t start_cycle;
  uint32_t stop_tai;
  uint32_t stop_cycle;

  int idx_four_ts = 0;

  int i = 0;

  const char*b2b_procedure;

  init();

  /* Base address of the related ep cores */
  pSCUbm =(unsigned short*)find_device_adr(venID,devID_SCUBM);
  pEBm   = (unsigned int*)find_device_adr(venID,devID_EBM);
  pECA_Q   = 0xa00000c0;
  //pECA_Q = (unsigned int*)find_device_adr(venID,devID_ECA);
  pTLU   = (unsigned int*)find_device_adr(venID,devID_TLU);
  pTIME  = (unsigned int*)find_device_adr(venID,devID_TIME);

  //mprintf("base address %x\n",pTLU);

  while (1)
  {
    eca_queue_flag = *(pECA_Q +(ECA_QUEUE_FLAGS_GET >> 2));
    event_id_hi    = *(pECA_Q +(ECA_QUEUE_EVENT_ID_HI_GET >> 2));
    event_id_lo    = *(pECA_Q +(ECA_QUEUE_EVENT_ID_LO_GET >> 2));
    event_tag      = *(pECA_Q +(ECA_QUEUE_TAG_GET >> 2));
    event_tef      = *(pECA_Q +(ECA_QUEUE_TEF_GET >> 2));
    //select event based on Event ID, maybe later tag is also needed.
    if (eca_queue_flag && 0x00000010 == 0x10 && event_id_hi == ((TGM_SYNCH_WIN >> 32) & 0xffffffff) && event_id_lo == (TGM_SYNCH_WIN & 0xffffffff))
    {
      trigger_scu = WAIT_sync_window;
    }
    else
    {
      trigger_scu = IDLE;
    }

    switch(trigger_scu)
    {
      case WAIT_sync_window:
        //ECA io produces the TTL pulse
        break;

        default: break;
    }

  }
}



