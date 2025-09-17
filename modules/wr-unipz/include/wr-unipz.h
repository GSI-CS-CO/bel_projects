#ifndef _WR_UNIPZ_H_
#define _WR_UNIPZ_H_

#include <common-defs.h>

// !!!!!
// experimental: let's try the same header and DP RAM layout for ALL firmwares....
// !!!!!

// this file is structured in two parts
// 1st: definitions of general things like error messages
// 2nd: definitions for data exchange via DP RAM

// ****************************************************************************************
// general things
// ****************************************************************************************
#define WRUNIPZ_QUERYTIMEOUT             1    // timeout for querying virt acc from MIL Piggy FIFO [ms] 
                                              // Ludwig: we have 10ms time; here: use 5 ms to be on the safe side
//#define WRUNIPZ_MILTIMEOUT             100    // timeout for querying MIL event [ms]
#define WRUNIPZ_MATCHWINDOW         200000    // used for comparing timestamps: 1 TS from TLU->ECA matches event from MIL FIFO, 2: synch EVT_MB_TRIGGER, ... [ns]
#define WRUNIPZ_UNILACFREQ              50    // frequency of UNILAC operation [Hz]
#define WRUNIPZ_UNILACPERIOD      20000000    // length of one UNILAC cylce [ns]
#define WRUNIPZ_UNILACPERIODMAX   20200000    // max length of one UNILAC cylce [ns]
#define WRUNIPZ_UNILACPERIODMIN   19800000    // min length of one UNILAC cylce [ns]

#define WRUNIPZ_MAXPREPOFFSET         2000    // max offset of a prep event within UNILAC cycle [us]; all with events a smaller offset will be predicted from previous cycles
#define WRUNIPZ_MILCALIBOFFSET       27000    // calibration offset to MIL event bus [ns]; MIL events are always 'late' due its protocol; this offset must be added to WR deadlines
#define WRUNIPZ_QQOFFSET               250    // offset for sending special service event for QQ [us] /* chk: QQ is breaking the concept of WR */
#define WRUNIPZ_A4OFFSET               250    // offset for sending special service event for A4 [us] /* chk: A4 is breaking the concept of WR */
#define WRUNIPZ_NOBEAMOFFSET          1320    // offset for sending special service event for EVT_BEAM_OFF [us] /* chk: EVT_NO_BEAM is breaking the concept of WR */
#define WRUNIPZ_TDIFFMIL                45    // minimmum time difference [us] between sending telegrams to the MIL bus; 25us is enough, but UNIPZ sends service events 45us after last event

// numbers for UNIPZ
#define WRUNIPZ_NEVT                    20    // # of events per virt acc
#define WRUNIPZ_NVACC                   16    // # vAcc
#define WRUNIPZ_NCHN                     2    // # of channels (each virtacc may have multiple channels, example: "verkuerzte" operation)
#define WRUNIPZ_NPZ                      7    // # of Pulszentralen
#define WRUNIPZ_NFLAG                    4    // # flags per virt acc 

// (error) status, each status will be represented by one bit, bits will be ORed into a 32bit word
#define WRUNIPZ_STATUS_LATE             16    // a timing messages is not dispatched in time
#define WRUNIPZ_STATUS_EARLY            17    // a timing messages is dispatched unreasonably early (dt > UNILACPERIOD)
#define WRUNIPZ_STATUS_TRANSACTION      18    // transaction failed
#define WRUNIPZ_STATUS_MIL              19    // an error on MIL hardware occured (MIL piggy etc...)
#define WRUNIPZ_STATUS_NOMILEVENTS      20    // no MIL events from UNIPZ
#define WRUNIPZ_STATUS_WRONGVIRTACC     21    // received EVT_READY_TO_SIS with wrong virt acc number
#define WRUNIPZ_STATUS_SAFETYMARGIN     22    // violation of safety margin for data master and timing network
#define WRUNIPZ_STATUS_NOTIMESTAMP      23    // received EVT_READY_TO_SIS in MIL FIFO but not via TLU -> ECA
#define WRUNIPZ_STATUS_BADTIMESTAMP     24    // TS from TLU->ECA does not coincide with MIL Event from FIFO
#define WRUNIPZ_STATUS_ORDERTIMESTAMP   25    // TS from TLU->ECA and MIL Events are out of order
                              
// commands from the outside
/*#define WRUNIPZ_CMD_CONFINIT            11    // init transaction of table data*/
#define WRUNIPZ_CMD_CONFSUBMIT          12    // submit data written to DP RAM
/*#define WRUNIPZ_CMD_CONFKILL            13    // this will kill an ongoing transaction*/
#define WRUNIPZ_CMD_CONFCLEAR           14    // this will clear all event tables

// activity requested by ECA Handler, the relevant codes are also used as "tags".
#define WRUNIPZ_ECADO_TIMEOUT            0    // timeout: no activity requested
#define WRUNIPZ_ECADO_UNKOWN             1    // unnkown activity requested (unexpected action by ECA)
#define WRUNIPZ_ECADO_MIL                3    // a MIL event was received

/*// data transaction
#define  WRUNIPZ_CONFSTAT_IDLE           0    // no transaction in progress
#define  WRUNIPZ_CONFSTAT_INIT           1    // transaction of config data has been initialized
#define  WRUNIPZ_CONFSTAT_SUBMIT         2    // config data for transaction has been submitted, waiting for commit event
*/

// event codes from Super PZ received via internal bus (bits 0..7)
#define WRUNIPZ_EVT_PZ1                0x1    // next cycle PZ 1
#define WRUNIPZ_EVT_PZ2                0x2    // next cycle PZ 2
#define WRUNIPZ_EVT_PZ3                0x3    // next cycle PZ 3
#define WRUNIPZ_EVT_PZ4                0x4    // next cycle PZ 4
#define WRUNIPZ_EVT_PZ5                0x5    // next cycle PZ 5
#define WRUNIPZ_EVT_PZ6                0x6    // next cycle PZ 6
#define WRUNIPZ_EVT_PZ7                0x7    // next cycle PZ 7
#define WRUNIPZ_EVT_SYNCH_DATA        0x20    // commit event for transaction
#define WRUNIPZ_EVT_50HZ_SYNCH        0x21    // 50 Hz trigger, cycle start
#define WRUNIPZ_EVT_NO_BEAM           0x89    // 'there is no beam ...', speciality by UNIPZ (prov. comm. P. Kainberger)

// event data from Super PZ received via internal bus (bits 12..15)
#define WRUNIPZ_EVTDATA_CHANNEL        0x1    // bit 12 - channel number: there are only two channels -> channel number coded in one bit
#define WRUNIPZ_EVTDATA_NOCHOP         0x2    // bit 13 (bit 15 not set) - flag: execute virt acc without chopper
#define WRUNIPZ_EVTDATA_SHORTCHOP      0x4    // bit 14 (bit 15 not set) - flag: execute virt acc with short chopper;
                                              // informative: not used by UNIPZ. UNIPZ implements 'short chopper' by using a different 'Kanal'
#define WRUNIPZ_EVTDATA_SERVICE        0x8    // if bit 15 is set, a service event must be sent after all other events have been sent
#define WRUNIPZ_EVTDATA_UNLOCKA4       0xc    // service event: send unlock command to A4
#define WRUNIPZ_EVTDATA_PREPACCNOW     0xd    // service event: execute preparation event for virt acc: VERY SPECIAL! event sent immediately 
#define WRUNIPZ_EVTDATA_PREPACC        0xe    // service event: execute preparation event for virt acc 
#define WRUNIPZ_EVTDATA_ZEROACC        0xf    // set all magnets to zero value

// event codes for events (sent by wr-unipz)
#define EVT_AUX_PRP_NXT_ACC           0x11    // set values in magnet prep. cycles
#define EVT_UNLOCK_ALVAREZ            0x15    // unlock A4 for next pulse 
#define EVT_MAGN_DOWN                 0x19    // set magnets to zero current
#define EVT_NO_BEAM                   0x89    // signal 'no beam'
#define EVT_COMMAND                   0xff    // event command


typedef struct dataTable {                    // table with _one_ virtAcc for _one_ Pulszentrale
  uint32_t validFlags;                        // if bit 'n' is set, data[n] is valid
  uint32_t prepFlags;                         // if bit 'n' is set, data[n] is prep data
  uint32_t evtFlags;                          // if bit 'n' is set, data[n] is event data  
  uint32_t data[WRUNIPZ_NEVT];                // bits 0..7 'event code', bits 8..11 reserved, bits 12..15 'event data', bits 16..31 offset [us]
                                              // 'event data':
                                              // WRUNIPZ_EVT_PZ1..PZ7: bit 15 (n/a), bit 14 ('short chopper'/ unused (?)), bit 13 ('no chopper'), bit 12 ('channel')
                                              // EVT_Prep_Next_Acc   : https://www-acc.gsi.de/data/documentation/eq-models/pzus/gm-pzus.pdf (page 72)
                                              //                       bit 15 ('high current'), bit 14 (unused)      , bit 13 (short chopper), bit 12 (no chopper)
                                              // Beam Status (??)    : bit 15 ('high current'), bit 14 ('no chopper'), bit 13 ('high brho')  , bit 12 (reserved)
                                              // EVT_COMMAND         : PZ-Kennung: 1(SIS), 2(ESR), 9(QR), 10(QL), 11(QN), 12(UN), 13(UH), 14(UA), 15(TK)
                                              // Commands, 0d200..208: ??
                                              // UTC (0xe0..e4)      : UTC time in special format, see code or documentation
                                              // all anderen Evts    : 0x0
} dataTable;

// ****************************************************************************************
// DP RAM
// ****************************************************************************************

// sizes
#define WRUNIPZ_NEVTDATA              (WRUNIPZ_NEVT  * WRUNIPZ_NCHN * WRUNIPZ_NPZ * WRUNIPZ_NVACC)   // # of config data words
#define WRUNIPZ_NEVTFLAG              (WRUNIPZ_NFLAG * WRUNIPZ_NCHN * WRUNIPZ_NPZ * WRUNIPZ_NVACC)   // # of config flag words

// offsets
// simple values
#define WRUNIPZ_SHARED_TCYCLEAVG      (COMMON_SHARED_END         + _32b_SIZE_)          // period of UNILAC cycle [ns] (average over one second)
#define WRUNIPZ_SHARED_NCYCLE         (WRUNIPZ_SHARED_TCYCLEAVG  + _32b_SIZE_)          // # of UNILAC cycles
#define WRUNIPZ_SHARED_NMESSAGEHI     (WRUNIPZ_SHARED_NCYCLE     + _32b_SIZE_)          // # of messsages, high bits
#define WRUNIPZ_SHARED_NMESSAGELO     (WRUNIPZ_SHARED_NMESSAGEHI + _32b_SIZE_)          // # of messsages, low bits
#define WRUNIPZ_SHARED_MSGFREQAVG     (WRUNIPZ_SHARED_NMESSAGELO + _32b_SIZE_)          // message rate (average over one second)
#define WRUNIPZ_SHARED_DTMAX          (WRUNIPZ_SHARED_MSGFREQAVG + _32b_SIZE_)          // delta T max (actTime - deadline)
#define WRUNIPZ_SHARED_DTMIN          (WRUNIPZ_SHARED_DTMAX      + _32b_SIZE_)          // delta T min (actTime - deadline)
#define WRUNIPZ_SHARED_CYCJMPMAX      (WRUNIPZ_SHARED_DTMIN      + _32b_SIZE_)          // delta T max (expected and actual start of UNILAC cycle)
#define WRUNIPZ_SHARED_CYCJMPMIN      (WRUNIPZ_SHARED_CYCJMPMAX  + _32b_SIZE_)          // delta T min (expected and actual start of UNILAC cycle)
#define WRUNIPZ_SHARED_VACCAVG        (WRUNIPZ_SHARED_CYCJMPMIN  + _32b_SIZE_)          // virt accs used (past second) bits 0..15 (normal), 16-31 (verkuerzt)
#define WRUNIPZ_SHARED_PZAVG          (WRUNIPZ_SHARED_VACCAVG    + _32b_SIZE_)          // PZ used (past second) bits 0..6

// shared memory for submitting new 'event tables'                                      
// event data layout
// ==================
// start with 1st channel of PZ0 of vacc0, then 2nd channel of same PZ, then other PZs, then the same for the other vaccs 
// [[[[data0 of PZ0-chn0-vacc0]..[data20 of PZ0-chan0-vacc0][data0 of PZ0-chn1-vacc0]..[data20] of PZ0-chn1-vacc0]]...PZ6]]]...vacc15]
// data bits 0..15: event data; 16..31: time offset within cycle [us]
#define WRUNIPZ_SHARED_EVT_DATA       (WRUNIPZ_SHARED_PZAVG      + _32b_SIZE_) 
// config flag layout
// ==================
// (there are 4 words per virtual accelerator for each channel
// [[[flags of PZ0-chn0-vacc0][flags of PZ0-chn1-vacc0]...PZ6]...vacc15]
// flags[0] = 0x1: new data available; flags[1]: data is valid if bit is set; flags[2]: data is prep if bit is set; flags[3]: data is evt if bit is set 
#define WRUNIPZ_SHARED_EVT_FLAGS      (WRUNIPZ_SHARED_EVT_DATA  + (WRUNIPZ_NEVTDATA << 2))  

// diagnosis: end of used shared memory
#define WRUNIPZ_SHARED_END            (WRUNIPZ_SHARED_EVT_FLAGS  + (WRUNIPZ_NEVTFLAG << 2)) // end of shared memory

#endif
