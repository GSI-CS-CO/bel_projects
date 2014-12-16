#ifndef NAU8811_AUDIO_DRIVER
#define NAU8811_AUDIO_DRIVER

/* Compile Options */
/* ==================================================================================================== */
#ifndef NAU8811_TARGET_EMBEDDED
#define NAU8811_TARGET_EMBEDDED 1 /* 1=Compile for embedded device; 0=Compile for Host (x86, ...) */
#endif

/* C Standard Includes */
/* ==================================================================================================== */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* Macros */
/* ==================================================================================================== */
#define NAU8811_ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))

/* NAU8811 Registers */
/* ==================================================================================================== */
#define NAU8811_REG_SOFTWARE_RESET      0x00 /* Reset Area */
#define NAU8811_REG_POWER_MANAGEMENT_1  0x01 /* Power Management Area */
#define NAU8811_REG_POWER_MANAGEMENT_2  0x02
#define NAU8811_REG_POWER_MANAGEMENT_3  0x03
#define NAU8811_REG_AUDIO_INTERFACE     0x04 /* Audio Control Area */
#define NAU8811_REG_COMPANDING          0x05
#define NAU8811_REG_CLOCK_CONTROL_1     0x06
#define NAU8811_REG_CLOCK_CONTROL_2     0x07 /* Also known as: Audio Sample Rate Control Register */
#define NAU8811_REG_DAC_CTRL            0x0a
#define NAU8811_REG_DAC_VOLUME          0x0b /* Also known as: DAC Gain Control Register */
#define NAU8811_REG_ADC_CTRL            0x0e
#define NAU8811_REG_ADC_VOLUME          0x0f /* Also known as: ADC Gain Control Register */
#define NAU8811_REG_DAC_LIMITER_1       0x18 /* Digital to Analog Limiter Area */
#define NAU8811_REG_DAC_LIMITER_2       0x19
#define NAU8811_REG_NOTCH_FILTER_1      0x1b /* Notch Filter Area */
#define NAU8811_REG_NOTCH_FILTER_2      0x1c
#define NAU8811_REG_NOTCH_FILTER_3      0x1d
#define NAU8811_REG_NOTCH_FILTER_4      0x1e
#define NAU8811_REG_ALC_CTRL_1          0x20 /* ALC Control Area */
#define NAU8811_REG_ALC_CTRL_2          0x21
#define NAU8811_REG_ALC_CTRL_3          0x22
#define NAU8811_REG_NOISE_GATE          0x23
#define NAU8811_REG_ATTENUATION_CTRL    0x28 /* Input, Output and Mixer Control Area */
#define NAU8811_REG_INPUT_CTRL          0x2c
#define NAU8811_REG_PGA_GAIN            0x2d
#define NAU8811_REG_ADC_BOOST           0x2f
#define NAU8811_REG_OUTPUT_CTRL         0x31
#define NAU8811_REG_MIXER_CTRL          0x32
#define NAU8811_REG_SPKOUT_VOLUME       0x36
#define NAU8811_REG_MONO_MIXER_CONTROL  0x38
#define NAU8811_REG_POWER_MANAGEMENT_4  0x3a /* Low Power Bits Area */
#define NAU8811_REG_TIME_SLOT           0x3b /* PCM Time Slot Control & ADCOUT Impedance Option Control Area */
#define NAU8811_REG_ADCOUT_DRIVE        0x3c
#define NAU8811_REG_RESERVED            0x41 /* Register ID Area */
#define NAU8811_REG_HIGH_VOLTAGE_CTRL   0x45
#define NAU8811_REG_ALC_ENHANCEMENTS_1  0x46
#define NAU8811_REG_ALC_ENHANCEMENTS_2  0x47
#define NAU8811_REG_ADDITIONAL_IF_CTRL  0x49
#define NAU8811_REG_POWER_TIE_OFF_CTRL  0x4b
#define NAU8811_REG_OUTPUT_TIE_OFF_CTRL 0x4f

/* NAU8811 Bit Fields */
/* ==================================================================================================== */
#define NAU8811_REG_GENERIC_DEFAULT_VALUE             0x000
#define NAU8811_REG_SOFTWARE_RESET_ANY_VALUE_TO_RESET 0x000
#define NAU8811_REG_POWER_MANAGEMENT_1_REFIMP_MASK    0x003
#define NAU8811_REG_POWER_MANAGEMENT_1_REFIMP_DISABLE 0x000 /* VREF Reference impedance selection */
#define NAU8811_REG_POWER_MANAGEMENT_1_REFIMP_R80K    0x001 /* 80K Ohm */
#define NAU8811_REG_POWER_MANAGEMENT_1_REFIMP_R300K   0x002 /* 300K Ohm */
#define NAU8811_REG_POWER_MANAGEMENT_1_REFIMP_R3K     0x003 /* 3K Ohm */
#define NAU8811_REG_POWER_MANAGEMENT_1_IOBUFEN        0x004 /* Unused input/output tie off buffer enable */
#define NAU8811_REG_POWER_MANAGEMENT_1_ABIASEN        0x008 /* Analogue amplifier bias control */
#define NAU8811_REG_POWER_MANAGEMENT_1_AUXEN          0x040 /* AUX input buffer enable */
#define NAU8811_REG_POWER_MANAGEMENT_1_DCBUFEN        0x100 /* Buffer for DC level shifting Enable */
#define NAU8811_REG_POWER_MANAGEMENT_2_ADCEN          0x001 /* ADC Enable */
#define NAU8811_REG_POWER_MANAGEMENT_2_PGAEN          0x004 /* MIC (+/-) PGA Enable */
#define NAU8811_REG_POWER_MANAGEMENT_2_BSTEN          0x010 /* Input Boost Enable */
#define NAU8811_REG_POWER_MANAGEMENT_3_MOUTEN         0x080 /* MOUT Enable */
#define NAU8811_REG_POWER_MANAGEMENT_3_NSPKEN         0x040 /* SPKOUT- Enable */
#define NAU8811_REG_POWER_MANAGEMENT_3_PSPKEN         0x020 /* SPKOUT+ Enable */
#define NAU8811_REG_POWER_MANAGEMENT_3_MOUTMXEN       0x008 /* Headphone Mixer Enable */
#define NAU8811_REG_POWER_MANAGEMENT_3_SPKMXEN        0x004 /* Speaker Mixer Enable */
#define NAU8811_REG_POWER_MANAGEMENT_3_DACEN          0x001 /* DAC Enable */
#define NAU8811_REG_AUDIO_INTERFACE_BCKP              0x100 /* BCLK Polarity */
#define NAU8811_REG_AUDIO_INTERFACE_FSP               0x080 /* Frame Clock Polarity */
#define NAU8811_REG_AUDIO_INTERFACE_WEN_MASK          0x060
#define NAU8811_REG_AUDIO_INTERFACE_WEN_16_BITS       0x000 /* Word Length = 16 Bits */
#define NAU8811_REG_AUDIO_INTERFACE_WEN_20_BITS       0x020 /* Word Length = 20 Bits */
#define NAU8811_REG_AUDIO_INTERFACE_WEN_24_BITS       0x040 /* Word Length = 24 Bits */
#define NAU8811_REG_AUDIO_INTERFACE_WEN_32_BITS       0x060 /* Word Length = 32 Bits */
#define NAU8811_REG_AUDIO_INTERFACE_AIFMT_MASK        0x018
#define NAU8811_REG_AUDIO_INTERFACE_AIFMT_RIGHT_JUST  0x000 /* Audio Data Format Select = Right Justified */
#define NAU8811_REG_AUDIO_INTERFACE_AIFMT_LEFT_JUST   0x008 /* Audio Data Format Select = Left Justified */
#define NAU8811_REG_AUDIO_INTERFACE_AIFMT_IIS         0x010 /* Audio Data Format Select = I2S/IIS */
#define NAU8811_REG_AUDIO_INTERFACE_AIFMT_PCMA        0x018 /* Audio Data Format Select = PCMA */
#define NAU8811_REG_AUDIO_INTERFACE_DACPHS            0x004 /* DAC Data „right‟ or „left‟ phases of FRAME clock */
#define NAU8811_REG_AUDIO_INTERFACE_ADCPHS            0x002 /* ADC Data „right‟ or „left‟ phases of FRAME clock */
#define NAU8811_REG_COMPANDING_DACCM_MASK             0x018
#define NAU8811_REG_COMPANDING_DACCM_DISABLED         0x000 /* DAC Companding Selection = Disabled */
#define NAU8811_REG_COMPANDING_DACCM_RESERVED         0x008 /* DAC Companding Selection = Reserved */
#define NAU8811_REG_COMPANDING_DACCM_U_LAW            0x010 /* DAC Companding Selection = u-Law */
#define NAU8811_REG_COMPANDING_DACCM_A_LAW            0x018 /* DAC Companding Selection = A-Law */
#define NAU8811_REG_COMPANDING_ADCCM_MASK             0x006
#define NAU8811_REG_COMPANDING_ADCCM_DISABLED         0x000 /* ADC Companding Selection = Disabled */
#define NAU8811_REG_COMPANDING_ADCCM_RESERVED         0x002 /* ADC Companding Selection = Reserved */
#define NAU8811_REG_COMPANDING_ADCCM_U_LAW            0x004 /* ADC Companding Selection = u-Law */
#define NAU8811_REG_COMPANDING_ADCCM_A_LAW            0x006 /* ADC Companding Selection = A-Law */
#define NAU8811_REG_COMPANDING_ADDAP                  0x001 /* Connect ADC output to DAC input internally. In this mode, the DAC data can not be seen at the DACIN pin of the device */
#define NAU8811_REG_CLOCK_CONTROL_1_BCLKSEL_MASK      0x0e0
#define NAU8811_REG_CLOCK_CONTROL_1_DIVIDER_1         0x000
#define NAU8811_REG_CLOCK_CONTROL_1_DIVIDER_1_5       0x020
#define NAU8811_REG_CLOCK_CONTROL_1_DIVIDER_2         0x040
#define NAU8811_REG_CLOCK_CONTROL_1_DIVIDER_3         0x060
#define NAU8811_REG_CLOCK_CONTROL_1_DIVIDER_4         0x080
#define NAU8811_REG_CLOCK_CONTROL_1_DIVIDER_6         0x0a0
#define NAU8811_REG_CLOCK_CONTROL_1_DIVIDER_8         0x0c0
#define NAU8811_REG_CLOCK_CONTROL_1_DIVIDER_12        0x0e0
#define NAU8811_REG_CLOCK_CONTROL_2_SMPLR_MASK        0x006
#define NAU8811_REG_CLOCK_CONTROL_2_SMPLR_48K         0x000 /* Sample Rate Selection = 48k */
#define NAU8811_REG_CLOCK_CONTROL_2_SMPLR_32K         0x002 /* Sample Rate Selection = 32k */
#define NAU8811_REG_CLOCK_CONTROL_2_SMPLR_24K         0x004 /* Sample Rate Selection = 24k */
#define NAU8811_REG_CLOCK_CONTROL_2_SMPLR_16K         0x006 /* Sample Rate Selection = 16k */
#define NAU8811_REG_CLOCK_CONTROL_2_SMPLR_12K         0x008 /* Sample Rate Selection = 12k */
#define NAU8811_REG_CLOCK_CONTROL_2_SMPLR_8K          0x00a /* Sample Rate Selection = 8k */
#define NAU8811_REG_CLOCK_CONTROL_2_SMPLR_RES1        0x00c /* Sample Rate Selection = Reserved#1 */
#define NAU8811_REG_CLOCK_CONTROL_2_SMPLR_RES2        0x00e /* Sample Rate Selection = Reserved#2 */
#define NAU8811_REG_DAC_CTRL_DACMT                    0x040 /* Soft Mute Enable */
#define NAU8811_REG_DAC_CTRL_DEEMP_MASK               0x030
#define NAU8811_REG_DAC_CTRL_DEEMP_NO                 0x000 /* De-emphasis: No */
#define NAU8811_REG_DAC_CTRL_DEEMP_32KHZ              0x010 /* De-emphasis: 32kHz sample rate */
#define NAU8811_REG_DAC_CTRL_DEEMP_44_1KHZ            0x020 /* De-emphasis: 44.1kHz sample rate */
#define NAU8811_REG_DAC_CTRL_DEEMP_48_KHZ             0x030 /* De-emphasis: 48kHz sample rate */
#define NAU8811_REG_DAC_CTRL_DACOS                    0x008 /* Over Sample Rate */
#define NAU8811_REG_DAC_CTRL_AUTOMT                   0x004 /* Auto Mute enable */
#define NAU8811_REG_DAC_CTRL_DACPL                    0x001 /* Polarity Invert */
#define NAU8811_REG_DAC_VOLUME_DACGAIN_MASK           0x0ff /* DAC Gain Mask */
#define NAU8811_REG_DAC_VOLUME_MAGIC_BIT              0x100 /* Undocumented */
#define NAU8811_REG_ADC_CTRL_HPFEN                    0x100 /* High Pass Filter Enable */
#define NAU8811_REG_ADC_CTRL_HPFAM                    0x080 /* Audio or Application Mode */
#define NAU8811_REG_ADC_CTRL_HPF_MASK                 0x070 /* High Pass Filter Mask */
#define NAU8811_REG_ADC_CTRL_ADCOS                    0x004 /* Over Sample Rate */
#define NAU8811_REG_ADC_CTRL_ADCPL                    0x001 /* ADC Polarity */
#define NAU8811_REG_ADC_VOLUME_ADCGAIN_MASK           0x0ff /* ADC Gain Mask */
#define NAU8811_REG_ADC_VOLUME_MAGIC_BIT              0x100 /* Undocumented */
#define NAU8811_REG_MIXER_CTRL_DACSPK                 0x001 /* DAC to Speaker Mixer */
#define NAU8811_REG_MIXER_CTRL_BYPSPK                 0x002 /* Bypass path (output of Boost stage) to Speaker Mixer */
#define NAU8811_REG_MIXER_CTRL_AUXSPK                 0x020 /* Auxiliary to Speaker Mixer */
#define NAU8811_REG_MONO_MIXER_CONTROL_DACMOUT        0x001 /* DAC to Headphone Mixer */
#define NAU8811_REG_MONO_MIXER_CONTROL_BYPMOUT        0x002 /* Bypass path (output of Boost Stage) to Headphone Mixer */
#define NAU8811_REG_MONO_MIXER_CONTROL_AUXMOUT        0x004 /* Auxiliary to Headphone Mixer */

/* Device Specific Includes */
/* ==================================================================================================== */
#if NAU8811_TARGET_EMBEDDED
#include "mini_sdb.h"  /* GSI LM32 Includes (find_device_adr) */
#else
#include <etherbone.h> /* Include Etherbone */
#endif 

/* Function Return Codes */
/* ==================================================================================================== */
#define NAU8811_RETURN_SUCCESS_CODE 0
#define NAU8811_RETURN_FAILURE_CODE 1

/* Device Specific Includes and Defines */
/* ==================================================================================================== */
#define NAU8811_GSI_VENDOR_ID 0x0000000000000651LL
#define NAU8811_GSI_DEVICE_ID 0x7AE8811D
#ifndef NAU8811_AUD_DRIVER
#define NAU8811_AUD_DRIVER 0x7AE8811D
#endif

/* Registers and Bits */
/* ==================================================================================================== */
#define NAU8811_REG_STATUS_OFFSET             0x000
#define NAU8811_REG_CONTROL_OFFSET            0x004
#define NAU8811_REG_TX_SPI_DATA_OFFSET        0x008
#define NAU8811_REG_TX_IIS_DATA_OFFSET        0x00C
#define NAU8811_REG_RX_IIS_DATA_OFFSET        0x010
#define NAU8811_REG_TX_IIS_STREAM_OFFSET      0x014
#define NAU8811_REG_RX_IIS_STREAM_OFFSET      0x018
#define NAU8811_REG_FIFO_FILL_LEVEL_OFFSET    0x01C
#define NAU8811_REG_STATUS_SPI_TX_FIFO_FULL   0x001
#define NAU8811_REG_STATUS_SPI_TX_FIFO_EMPTY  0x002
#define NAU8811_REG_STATUS_IIS_TX_FIFO_FULL   0x004
#define NAU8811_REG_STATUS_IIS_TX_FIFO_EMPTY  0x008
#define NAU8811_REG_STATUS_IIS_RX_FIFO_FULL   0x010
#define NAU8811_REG_STATUS_IIS_RX_FIFO_EMPTY  0x020
#define NAU8811_REG_CONTROL_SS_CTRL_CONFIG    0x001
#define NAU8811_REG_CONTROL_SS_STATE          0x002
#define NAU8811_REG_CONTROL_CLOCK_ENABLE      0x004
#define NAU8811_REG_CONTROL_TRANSMISSION_MODE 0x008
#define NAU8811_REG_CONTROL_MONO_OUTPUT       0x010
#define NAU8811_REG_CONTROL_INVERT_FS         0x020
#define NAU8811_REG_CONTROL_PADDING           0x040
#define NAU8811_REG_CONTROL_INTERNAL_LOOP     0x080
#define NAU8811_REG_CONTROL_HEARTBEAT_MODE    0x100
#define NAU8811_REG_CONTROL_HEARTBEAT_POL     0x200
#define NAU8811_REG_FIFO_FILL_LEVEL_TX_MASK   0x0000ffff
#define NAU8811_REG_FIFO_FILL_LEVEL_RX_MASK   0xffff0000

/* Miscellaneous Defines */
/* ==================================================================================================== */
#define NAU8811_MAX_WAIT_CC_UNTIL_TX_DONE 1000 /* Timeout until FIFO must be empty */
#define NAU8811_MAX_WAIT_CC_UNTIL_RX_DONE 1000 /* Timeout until FIFO must contain new data */
#define NAU8811_ADDRESS_SHIFT                9 /* SPI Frame contains: [7xaddress bits][9xdata bits] */
#define NAU8811_SPI_MODE                    16 /* 16 Bit SPI mode */

/* Structures */
/* ==================================================================================================== */
#if NAU8811_TARGET_EMBEDDED
typedef struct
{
  uint32_t uStatusRegister;
  uint32_t uControlRegister;
  uint32_t uTxSpiDataRegister;
  uint32_t uTxIisDataRegister;
  uint32_t uRxIisDataRegister;
  uint32_t uTxIisStreamRegister;
  uint32_t uRxIisStreamRegister;
  uint32_t uFillLevelIisRegister;
} s_NAU8811_RegisterArea;
#endif

/* Enumerations */
/* ==================================================================================================== */
typedef enum
{
  eStatusRegister,
  eControlRegister,
  eTxSpiDataRegister, 
  eTxIisDataRegister,
  eRxIisDataRegister,
  eTxIisStreamRegister,
  eRxIisStreamRegister,
  eFillLevelIisRegister
} e_NAU8811_RegisterArea;

/* Globals */
/* ==================================================================================================== */
#if NAU8811_TARGET_EMBEDDED
volatile s_NAU8811_RegisterArea* p_sNAU8811_Area;
#endif

/* Prototypes */
/* ==================================================================================================== */
int32_t iNAU8811_GetParameter     (e_NAU8811_RegisterArea eParameter, uint32_t *p_uValue);
int32_t iNAU8811_SetParameter     (e_NAU8811_RegisterArea eParameter, uint32_t uValue);
int32_t iNAU8811_AutoInitialize   (void);
int32_t iNAU8811_ManualInitialize (uint32_t uAreaAddress);
int32_t iNAU8811_ConfigureDevice  (void);
int32_t iNAU8811_TransmitCtrlData (uint32_t uValue);
int32_t iNAU8811_TransmitData     (uint32_t *p_uData, uint32_t uSize);
int32_t iNAU8811_ReceiveData      (uint32_t *p_uData, uint32_t uSize);
int32_t iNAU8811_CleanRxFifo      (uint32_t uReadCycles);
#if NAU8811_TARGET_EMBEDDED
void    vNAU8811_TransmitStream   (uint32_t uData);
void    vNAU8811_ReceiveStream    (uint32_t *p_uData);
#endif

#if NAU8811_TARGET_EMBEDDED==0  
/* Host Wrappers */
/* ==================================================================================================== */
eb_device_t s_DeviceName;
volatile uint32_t* p_uNAU8811_Area;
#endif

#endif /* NAU8811_AUDIO_DRIVER */
