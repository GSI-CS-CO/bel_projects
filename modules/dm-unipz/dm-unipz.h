#ifndef __DM_UNIPZ_H_
#define __DM_UNIPZ_H_

#define  DMUNIPZ_ECA_TAG          0x4         // tag for ECA actions we want to receive
#define  DMUNIPZ_US_ASMNOP        31          // # of asm("nop") operations per microsecond
#define  DMUNIPZ_MS_ASMNOP        31 * 1000   // # of asm("nop") operations per microsecond
#define  DMUNIPZ_WB_ECA_IN        0x7ffffff0  // Wishbone address of ECA input

// (error) status
#define  DMUNIPZ_STATUS_UNKNOWN   0           // unknown status
#define  DMUNIPZ_STATUS_OK        1           // status OK
#define  DMUNIPZ_STATUS_ERROR     2           // an error occured
#define  DMUNIPZ_STATUS_TIMEDOUT  3           // a timeout occured

// commands from the outside
#define  DMUNIPZ_CMD_NOCMD        0           // no command ...
#define  DMUNIPZ_CMD_CONFIGURE    1           // configures the gateway
#define  DMUNIPZ_CMD_STARTOP      2           // starts operation
#define  DMUNIPZ_CMD_STOPOP       3           // stops operation
#define  DMUNIPZ_CMD_IDLE         4           // requests gateway to enter idle state
#define  DMUNIPZ_CMD_RECOVER      5           // recovery from error state

//states; implicitely, all states may transit to the ERROR state
#define  DMUNIPZ_STATE_UNKNOWN    0           // initial state -> IDLE (automatic)
#define  DMUNIPZ_STATE_S0         1           // initial state -> IDLE (automatic)
#define  DMUNIPZ_STATE_IDLE       2           // idle state -> CONFIGURED (by command "configure")
#define  DMUNIPZ_STATE_CONFIGURED 3           // configured state -> IDLE ("idle"), CONFIGURED ("configure"), OPERATION ("startop")
#define  DMUNIPZ_STATE_OPERATION  4           // gateway in operation -> STOPPING ("stopop")
#define  DMUNIPZ_STATE_STOPPING   5           // gateway in operation -> CONFIGURED (automatic)
#define  DMUNIPZ_STATE_ERROR      6           // gateway in error -> IDLE ("recover")
#define  DMUNIPZ_STATE_FATAL      7           // gateway in fatal error; RIP


// part below provide by Ludwig Hechler 
#define IFB_ADDRESS_SIS     0x20        /* Adresse der Interfacekarte               */
#define IFB_ADDRESS_UNI     0x10        /* Adresse der Interfacekarte               */

#define IO_MODULE_1            1        /* Adresse I/O-Modul 1                      */
#define IO_MODULE_2            2        /* Adresse I/O-Modul 2                      */
#define IO_MODULE_3            3        /* Adresse I/O-Modul 3                      */

#define IFB_ADR_BUS_W       0x11        /* Funktionscode Adressbus schreiben        */
#define IFB_DATA_BUS_W      0x10        /* Funktionscode Datenbus schreiben         */
#define IFB_DATA_BUS_R      0x90        /* Funktionscode Datenbus lesen             */

#define C_IO32_KANAL_0      0x00        /* Subadresse I/O-Modul für I/O-Bits 0..15  */
#define C_IO32_KANAL_1      0x02        /* Subadresse I/O-Modul für I/O-Bits 16..31 */


// part below provided by Ludwig Hechler
typedef struct TPZSInfo1 {
  /* Bit 11..15 */  uint16_t free1         : 5;
  /* Bit 10     */  uint16_t Med_MachID_ok : 1;   /* MachineID is ok */
  /* Bit  9     */  uint16_t DTAVal_Ack    : 1;   /* Data valid acknowledge */
  /* Bit  8     */  uint16_t Med_Lock_Sts  : 1;   /* Lock operation status */
  /* Bit  7     */  uint16_t Med_Req_enabl : 1;   /* therapy requests enabled for Unilac */ 
  /* Bit  6     */  uint16_t Req_not_ok    : 1;   /* Unilac PZ signals error for beam request */
  /* Bit  5     */  uint16_t SIS_Req_Ack   : 1;   /* Acknowledge for SIS beam request */
  /* Bit  4     */  uint16_t TK_Req_Ack    : 1;   /* Acknowledge for TK preparation */ 
  /* Bit  0..3  */  uint16_t free2         : 4;
} TPZSInfo1;

typedef struct TPZSInfo5 {
  /* Bit  8..15 */  uint16_t freeIn1       : 8; 
  /* Bit  7     */  uint16_t ReqNoBeam     : 1;   /* Request accelerator but without beam */
  /* Bit  6     */  uint16_t Req_not_ok_Ack: 1;   /* Acknowledge for beam request error */
  /* Bit  5     */  uint16_t SIS_Request   : 1;   /* Request beam for SIS */
  /* Bit  4     */  uint16_t TK_Request    : 1;   /* Request TK preparation */
  /* Bit  0..3  */  uint16_t SIS_Acc_Select: 4;   /* Requested accelerator */ 
} TPZSInfo5;


// part below provided by Ludwig Hechler
typedef union {
  TPZSInfo5 bits;
  uint16_t uword;
} WriteToPZU_Type;

typedef union {
  TPZSInfo1 bits;
  uint16_t uword;
} ReadFromPZU_Type;


//#define INFO1_SIS_REQ_ACK   0x10        /* Acknowledge for TK preparation           */
//#define INFO1_TK_REQ_ACK   0x20         /* Acknowledge for SIS beam request         */

#endif
