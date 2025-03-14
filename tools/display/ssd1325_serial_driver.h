#ifndef SSD1325_SERIAL_DRIVER
#define SSD1325_SERIAL_DRIVER

/* Compile Options */
/* ==================================================================================================== */
#ifndef SSD1325_USE_LOWER_CHARS
#define SSD1325_USE_LOWER_CHARS                                1 /* 1=Use all characters; 0=Use uppercase characters only (and save memory) */
#endif
#ifndef SSD1325_TARGET_EMBEDDED
#define SSD1325_TARGET_EMBEDDED                                1 /* 1=Compile for embedded device; 0=Compile for Host (x86, ...) */
#endif
#ifndef SSD1325_DEMO_CONFIGURATION
#define SSD1325_DEMO_CONFIGURATION                             1 /* 1=Display demo configuration; 0=Display minimal configuration */
#endif

/* Defines (SSD1325 Manual) */
/* ==================================================================================================== */
#define SSD1325_CMD_SET_DISPLAY_OFFSET                         0xA2
#define SSD1325_CMD_SET_DISPLAY_OFFSET_0                       0x00
#define SSD1325_CMD_SET_DISPLAY_START_LINE                     0xA1
#define SSD1325_CMD_SET_DISPLAY_START_LINE_0                   0x00
#define SSD1325_CMD_SET_DISPLAY_RE_MAP                         0xA0
#define SSD1325_CMD_SET_DISPLAY_RE_MAP_DIS                     0x00
#define SSD1325_CMD_SET_DISPLAY_RE_MAP_DEFAULT                 0x56
#define SSD1325_CMD_SET_DISPLAY_CURRENT_RNG_QR                 0x84
#define SSD1325_CMD_SET_DISPLAY_CURRENT_RNG_HR                 0x85
#define SSD1325_CMD_SET_DISPLAY_CURRENT_RNG_FR                 0x86
#define SSD1325_CMD_SET_CONTRAST_CONTROL                       0x81
#define SSD1325_CMD_SET_CONTRAST_CONTROL_DEFAULT               0x7F
#define SSD1325_CMD_SET_ROW_PERIOD                             0xB2
#define SSD1325_CMD_SET_ROW_PERIOD_DEFAULT                     0x25
#define SSD1325_CMD_SET_PHASE_LENGHT                           0xB1
#define SSD1325_CMD_SET_PHASE_LENGHT_DEFAULT                   0x55
#define SSD1325_CMD_SET_PRE_CHARGE_VOLTAGE                     0xBC
#define SSD1325_CMD_SET_PRE_CHARGE_VOLTAGE_DEFAULT             0x10
#define SSD1325_CMD_SET_PRE_CHARGE_COMPENSATION                0xB4
#define SSD1325_CMD_SET_PRE_CHARGE_COMPENSATION_DEFAULT        0x02
#define SSD1325_CMD_SET_PRE_CHARGE_COMPENSATION_ENABLE         0xB0
#define SSD1325_CMD_SET_PRE_CHARGE_COMPENSATION_ENABLE_DEFAULT 0x28
#define SSD1325_CMD_SET_VCOMH_VOLTAGE                          0xBE
#define SSD1325_CMD_SET_VCOMH_VOLTAGE_DEFAULT                  0x1C
#define SSD1325_CMD_SET_VSL                                    0xBF
#define SSD1325_CMD_SET_VSL_DEFAULT                            0x0E
#define SSD1325_CMD_SET_DISPLAY_NORMAL_MODE                    0xA4
#define SSD1325_CMD_SET_DISPLAY_INVERSE_MODE                   0xA7
#define SSD1325_CMD_SET_COLUMN_ADDRESS                         0x15
#define SSD1325_CMD_SET_ROW_ADDRESS                            0x75
#define SSD1325_CMD_DISPLAY_ON                                 0xAF
#define SSD1325_CMD_DISPLAY_OFF                                0xAE
#define SSD1325_CMD_DRAW_RECTANGLE                             0x24

/* Function Return Codes */
/* ==================================================================================================== */
#define SSD1325_RETURN_SUCCESS_CODE                            0
#define SSD1325_RETURN_FAILURE_CODE                            1

/* Display Attributes */
/* ==================================================================================================== */
#define SSD1325_COL_COUNT                                      128
#define SSD1325_ROW_COUNT                                      64
#define SSD1325_COL_CHAR_COUNT                                 21
#define SSD1325_ROW_CHAR_COUNT                                 8
#define SSD1325_PIXELS_PER_CHAR_Y                              8
#define SSD1325_PIXELS_PER_CHAR_X                              6
#define SSD1325_COLUMNS_PER_CHAR                               3
#define SSD1325_ROWS_PER_CHAR                                  8
#define SSD1325_MAX_WAIT_CC_UNTIL_TX_DONE                      1000
#define SSD1335_ROW_BEGIN_OFFSET                               0x04
#define SSD1335_ROW_END_OFFSET                                 0x0b
#define SSD1335_COL_BEGIN_OFFSET                               0x00
#define SSD1335_COL_END_OFFSET                                 0x7f

/* Registers */
/* ==================================================================================================== */
#define SSD1325_REG_TX_FIFO_DATA_OFFSET                        0x00
#define SSD1325_REG_TX_FIFO_STATUS_OFFSET                      0x04
#define SSD1325_REG_TX_FIFO_STATUS_BIT_FIFO_FULL               0x01
#define SSD1325_REG_TX_FIFO_STATUS_BIT_FIFO_EMPTY              0x02
#define SSD1325_REG_TX_FIFO_STATUS_BIT_TX_DONE                 0x04
#define SSD1325_REG_TX_FIFO_FILL_LEVEL_OFFSET                  0x08
#define SSD1325_REG_CONTROL_OFFSET                             0x0C
#define SSD1325_REG_CONTROL_BIT_RESET                          0x01
#define SSD1325_REG_CONTROL_BIT_DATA_COMMAND                   0x02
#define SSD1325_REG_CONTROL_BIT_SS_CTRL_CONFIG                 0x04
#define SSD1325_REG_CONTROL_BIT_SS_CTRL_STATE                  0x08
#define SSD1325_REG_CONTROL_BIT_IRQ_ENABLE                     0x10
#define SSD1325_REG_CONTROL_BIT_IRQ_CLEAR                      0x20

/* C Standard Includes */
/* ==================================================================================================== */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* Device Specific Includes */
/* ==================================================================================================== */
#if SSD1325_TARGET_EMBEDDED
#include "mini_sdb.h"   /* GSI LM32 Includes (find_device_adr) */
#else
#include <etherbone.h>  /* Include Etherbone */
#define SSD1325_GSI_VENDOR_ID 0x0000000000000651LL
#define SSD1325_GSI_DEVICE_ID 0x55d1325d
#endif 

/* Structures */
/* ==================================================================================================== */
#if SSD1325_TARGET_EMBEDDED
typedef struct
{
  uint32_t uTxFifoDataRegister;
  uint32_t uTxFifoStatusRegister;
  uint32_t uTxFifoFillLevelRegister;
  uint32_t uControlRegister;
} s_SSD1325_RegisterArea;
#endif

/* Enumerations */
/* ==================================================================================================== */
typedef enum
{
  eTxFifoDataRegister,
  eTxFifoStatusRegister,
  eTxFifoFillLevelRegister, 
  eControlRegister
} e_SSD1325_RegisterArea;

/* Globals */
/* ==================================================================================================== */
#if SSD1325_TARGET_EMBEDDED
extern volatile s_SSD1325_RegisterArea* p_sSSD1325_Area;
#endif

/* Prototypes */
/* ==================================================================================================== */
int32_t iSSD1325_GetParameter       (e_SSD1325_RegisterArea eParameter, uint32_t *p_uValue);
int32_t iSSD1325_SetParameter       (e_SSD1325_RegisterArea eParameter, uint32_t uValue);
int32_t iSSD1325_AutoInitialize     (void);
int32_t iSSD1325_ManualInitialize   (uint32_t uAreaAddress);
int32_t iSSD1325_ResetDevice        (void);
int32_t iSSD1325_ConfigureScreen    (void);
int32_t iSSD1325_ClearLine          (uint32_t uLine, uint32_t uPattern);
int32_t iSSD1325_ClearChar          (uint32_t uPositionX, uint32_t uPositionY, uint32_t uPattern);
int32_t iSSD1325_ClearScreen        (void);
int32_t iSSD1325_PrintChar          (char cChar, uint32_t uPositionX, uint32_t uPositionY);
int32_t iSSD1325_PrintString        (const char* p_cChar, uint32_t uStartPositionX, uint32_t uStartPositionY);
int32_t iSSD1325_PrepareSendData    (void);
int32_t iSSD1325_PrepareSendCommand (void);
int32_t iSSD1325_WaitUntilTxDone    (void);
int32_t iSSD1325_EnableInterrupt    (bool fConfig);
int32_t iSSD1325_ClearInterrupt     (void);
int32_t iSSD1325_DrawRectangle      (uint32_t uStartPositionX, uint32_t uStartPositionY, 
                                     uint32_t uEndPositionX, uint32_t uEndPositionY, 
                                     uint32_t uPattern);
int32_t iSSD1325_DrawBitmap         (uint32_t uStartPositionX, uint32_t uStartPositionY, 
                                     uint32_t uEndPositionX, uint32_t uEndPositionY, 
                                     int8_t a_iBitmap[]);

#if SSD1325_TARGET_EMBEDDED==0  
/* Wrappers (for host display functions -> see https://www-acc.gsi.de/svn/bel/timing/.../development) */
/* ==================================================================================================== */
extern eb_device_t s_DeviceName;
extern volatile uint32_t* p_uSSD1325_Area;
void vSSD1325_InitDisplay (eb_device_t device);
void vSSD1325_HostPutLocC (eb_device_t device, char ascii, unsigned char row, unsigned char col);
void vSSD1325_HostPutC    (eb_device_t device, char ascii);
void vSSD1325_HostPutS    (eb_device_t device, const char* str);
void vSSD1325_HostPutLine (eb_device_t device, const char *sPtr, unsigned char row);
#endif

#endif /* SSD1325_SERIAL_DRIVER */
