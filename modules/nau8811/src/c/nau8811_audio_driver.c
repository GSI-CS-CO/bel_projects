/* Includes */
/* ==================================================================================================== */
#include "nau8811_audio_driver.h" /* Include NAU8811 Header */

/* Device configuration table */
/* ==================================================================================================== */
#if 1
/* PC to MIC; MOUT to PC */
uint32_t a_uDevConfig[][2] =
{ 
  /* Address */                    /* Configuration */
  /* Reset */
  {NAU8811_REG_SOFTWARE_RESET,     NAU8811_REG_SOFTWARE_RESET_ANY_VALUE_TO_RESET},
  /* Power Management */
  {NAU8811_REG_POWER_MANAGEMENT_1, NAU8811_REG_POWER_MANAGEMENT_1_IOBUFEN | NAU8811_REG_POWER_MANAGEMENT_1_ABIASEN | NAU8811_REG_POWER_MANAGEMENT_1_DCBUFEN | NAU8811_REG_POWER_MANAGEMENT_1_REFIMP_R80K},
  {NAU8811_REG_POWER_MANAGEMENT_2, NAU8811_REG_POWER_MANAGEMENT_2_BSTEN | NAU8811_REG_POWER_MANAGEMENT_2_PGAEN | NAU8811_REG_POWER_MANAGEMENT_2_ADCEN},
  {NAU8811_REG_POWER_MANAGEMENT_3, NAU8811_REG_POWER_MANAGEMENT_3_MOUTEN | NAU8811_REG_POWER_MANAGEMENT_3_NSPKEN | NAU8811_REG_POWER_MANAGEMENT_3_PSPKEN | NAU8811_REG_POWER_MANAGEMENT_3_MOUTMXEN | NAU8811_REG_POWER_MANAGEMENT_3_SPKMXEN | NAU8811_REG_POWER_MANAGEMENT_3_DACEN},
  /* Audio Control */
  {NAU8811_REG_AUDIO_INTERFACE,    NAU8811_REG_AUDIO_INTERFACE_WEN_32_BITS | NAU8811_REG_AUDIO_INTERFACE_AIFMT_IIS},
  {NAU8811_REG_COMPANDING,         NAU8811_REG_GENERIC_DEFAULT_VALUE},
  {NAU8811_REG_CLOCK_CONTROL_1,    NAU8811_REG_CLOCK_CONTROL_1_DIVIDER_1},
  {NAU8811_REG_CLOCK_CONTROL_2,    NAU8811_REG_CLOCK_CONTROL_2_SMPLR_48K},
  {NAU8811_REG_DAC_CTRL,           NAU8811_REG_DAC_CTRL_DACOS},
  {NAU8811_REG_DAC_VOLUME,         NAU8811_REG_DAC_VOLUME_MAGIC_BIT | NAU8811_REG_DAC_VOLUME_DACGAIN_MASK },
  {NAU8811_REG_ADC_CTRL,           NAU8811_REG_ADC_CTRL_ADCOS},
  {NAU8811_REG_ADC_VOLUME,         NAU8811_REG_ADC_VOLUME_MAGIC_BIT | NAU8811_REG_ADC_VOLUME_ADCGAIN_MASK },
  /* Limiter */
  {NAU8811_REG_DAC_LIMITER_1,      0x032}, /* Default */
  {NAU8811_REG_DAC_LIMITER_2,      0x000}, /* Default */
  /* Notch filters */
  {NAU8811_REG_NOTCH_FILTER_1,     0x000}, /* Default */
  {NAU8811_REG_NOTCH_FILTER_2,     0x000}, /* Default */
  {NAU8811_REG_NOTCH_FILTER_3,     0x000}, /* Default */
  {NAU8811_REG_NOTCH_FILTER_4,     0x000}, /* Default */
  /* ALC control */
  {NAU8811_REG_ALC_CTRL_1,         0x038}, /* Default */
  {NAU8811_REG_ALC_CTRL_2,         0x00b}, /* Default */
  {NAU8811_REG_ALC_CTRL_3,         0x032}, /* Default */
  /* Input, Output, Mixers, and Noise */
  {NAU8811_REG_NOISE_GATE,         0x000}, /* Default */
  /* [8811_I2S_ADCOUT/BYP Control] */
  {NAU8811_REG_ATTENUATION_CTRL,   0x000}, /* Default */
  {NAU8811_REG_INPUT_CTRL,         0x00c}, /* AUXM, AUXPGA */
  {NAU8811_REG_PGA_GAIN,           0x010}, /* Default */
  {NAU8811_REG_ADC_BOOST,          0x170}, /* PGABST, PMICBSTGAIN */
  {NAU8811_REG_OUTPUT_CTRL,        0x002}, /* Default */
  {NAU8811_REG_MIXER_CTRL,         NAU8811_REG_MIXER_CTRL_DACSPK},
  {NAU8811_REG_SPKOUT_VOLUME,      0x039}, /* Default */
  {NAU8811_REG_MONO_MIXER_CONTROL, NAU8811_REG_MONO_MIXER_CONTROL_DACMOUT},
};
#endif

#if 0
/* PC to AUX; MOUT to PC */
uint32_t a_uDevConfig[][2] =
{ 
  /* Address */                    /* Configuration */
  /* Reset */
  {NAU8811_REG_SOFTWARE_RESET,     NAU8811_REG_SOFTWARE_RESET_ANY_VALUE_TO_RESET},
  /* Power Management */
  {NAU8811_REG_POWER_MANAGEMENT_1, NAU8811_REG_POWER_MANAGEMENT_1_IOBUFEN | NAU8811_REG_POWER_MANAGEMENT_1_ABIASEN | NAU8811_REG_POWER_MANAGEMENT_1_DCBUFEN | NAU8811_REG_POWER_MANAGEMENT_1_REFIMP_R80K},
  {NAU8811_REG_POWER_MANAGEMENT_2, NAU8811_REG_POWER_MANAGEMENT_2_BSTEN | NAU8811_REG_POWER_MANAGEMENT_2_PGAEN | NAU8811_REG_POWER_MANAGEMENT_2_ADCEN},
  {NAU8811_REG_POWER_MANAGEMENT_3, NAU8811_REG_POWER_MANAGEMENT_3_MOUTEN | NAU8811_REG_POWER_MANAGEMENT_3_NSPKEN | NAU8811_REG_POWER_MANAGEMENT_3_PSPKEN | NAU8811_REG_POWER_MANAGEMENT_3_MOUTMXEN | NAU8811_REG_POWER_MANAGEMENT_3_SPKMXEN | NAU8811_REG_POWER_MANAGEMENT_3_DACEN},
  /* Audio Control */
  {NAU8811_REG_AUDIO_INTERFACE,    NAU8811_REG_AUDIO_INTERFACE_WEN_32_BITS | NAU8811_REG_AUDIO_INTERFACE_AIFMT_IIS},
  {NAU8811_REG_COMPANDING,         NAU8811_REG_GENERIC_DEFAULT_VALUE},
  {NAU8811_REG_CLOCK_CONTROL_1,    NAU8811_REG_CLOCK_CONTROL_1_DIVIDER_1},
  {NAU8811_REG_CLOCK_CONTROL_2,    NAU8811_REG_CLOCK_CONTROL_2_SMPLR_48K},
  {NAU8811_REG_DAC_CTRL,           NAU8811_REG_DAC_CTRL_DACOS},
  {NAU8811_REG_DAC_VOLUME,         NAU8811_REG_DAC_VOLUME_MAGIC_BIT | NAU8811_REG_DAC_VOLUME_DACGAIN_MASK },
  {NAU8811_REG_ADC_CTRL,           NAU8811_REG_ADC_CTRL_ADCOS},
  {NAU8811_REG_ADC_VOLUME,         NAU8811_REG_ADC_VOLUME_MAGIC_BIT | NAU8811_REG_ADC_VOLUME_ADCGAIN_MASK },
  /* Limiter */
  {NAU8811_REG_DAC_LIMITER_1,      0x032}, /* Default */
  {NAU8811_REG_DAC_LIMITER_2,      0x000}, /* Default */
  /* Notch filters */
  {NAU8811_REG_NOTCH_FILTER_1,     0x000}, /* Default */
  {NAU8811_REG_NOTCH_FILTER_2,     0x000}, /* Default */
  {NAU8811_REG_NOTCH_FILTER_3,     0x000}, /* Default */
  {NAU8811_REG_NOTCH_FILTER_4,     0x000}, /* Default */
  /* ALC control */
  {NAU8811_REG_ALC_CTRL_1,         0x038}, /* Default */
  {NAU8811_REG_ALC_CTRL_2,         0x00b}, /* Default */
  {NAU8811_REG_ALC_CTRL_3,         0x032}, /* Default */
  /* Input, Output, Mixers, and Noise */
  {NAU8811_REG_NOISE_GATE,         0x000}, /* Default */
  /* [8811_I2S_ADCOUT/BYP Control] */
  {NAU8811_REG_ATTENUATION_CTRL,   0x000}, /* Default */
  {NAU8811_REG_INPUT_CTRL,         0x00c}, /* AUXM, AUXPGA */
  {NAU8811_REG_PGA_GAIN,           0x010}, /* Default */
  {NAU8811_REG_ADC_BOOST,          0x010}, /* PMICBSTGAIN  */
  {NAU8811_REG_OUTPUT_CTRL,        0x002}, /* Default */
  {NAU8811_REG_MIXER_CTRL,         NAU8811_REG_MIXER_CTRL_DACSPK},
  {NAU8811_REG_SPKOUT_VOLUME,      0x039}, /* Default */
  {NAU8811_REG_MONO_MIXER_CONTROL, NAU8811_REG_MONO_MIXER_CONTROL_DACMOUT},
};
#endif

#if 0
/* DAC and ADC test */
uint32_t a_uDevConfig[][2] =
{ 
  /* Address */                    /* Configuration */
  /* Reset */
  {NAU8811_REG_SOFTWARE_RESET,     NAU8811_REG_SOFTWARE_RESET_ANY_VALUE_TO_RESET},
  /* Power Management */
  {NAU8811_REG_POWER_MANAGEMENT_1, NAU8811_REG_POWER_MANAGEMENT_1_IOBUFEN | NAU8811_REG_POWER_MANAGEMENT_1_ABIASEN | NAU8811_REG_POWER_MANAGEMENT_1_DCBUFEN | NAU8811_REG_POWER_MANAGEMENT_1_AUXEN | NAU8811_REG_POWER_MANAGEMENT_1_REFIMP_R80K},
  {NAU8811_REG_POWER_MANAGEMENT_2, NAU8811_REG_POWER_MANAGEMENT_2_BSTEN | NAU8811_REG_POWER_MANAGEMENT_2_PGAEN | NAU8811_REG_POWER_MANAGEMENT_2_ADCEN},
  {NAU8811_REG_POWER_MANAGEMENT_3, NAU8811_REG_POWER_MANAGEMENT_3_MOUTEN | NAU8811_REG_POWER_MANAGEMENT_3_NSPKEN | NAU8811_REG_POWER_MANAGEMENT_3_PSPKEN | NAU8811_REG_POWER_MANAGEMENT_3_MOUTMXEN | NAU8811_REG_POWER_MANAGEMENT_3_SPKMXEN | NAU8811_REG_POWER_MANAGEMENT_3_DACEN},
  /* Audio Control */
  {NAU8811_REG_AUDIO_INTERFACE,    NAU8811_REG_AUDIO_INTERFACE_WEN_32_BITS | NAU8811_REG_AUDIO_INTERFACE_AIFMT_IIS},
  {NAU8811_REG_COMPANDING,         NAU8811_REG_GENERIC_DEFAULT_VALUE},
  {NAU8811_REG_CLOCK_CONTROL_1,    NAU8811_REG_CLOCK_CONTROL_1_DIVIDER_1},
  {NAU8811_REG_CLOCK_CONTROL_2,    NAU8811_REG_CLOCK_CONTROL_2_SMPLR_48K},
  {NAU8811_REG_CLOCK_CONTROL_2,    NAU8811_REG_CLOCK_CONTROL_2_SMPLR_48K},
  {NAU8811_REG_DAC_CTRL,           NAU8811_REG_DAC_CTRL_DACOS},
  {NAU8811_REG_DAC_VOLUME,         NAU8811_REG_DAC_VOLUME_MAGIC_BIT | NAU8811_REG_DAC_VOLUME_DACGAIN_MASK },
  {NAU8811_REG_ADC_CTRL,           NAU8811_REG_ADC_CTRL_ADCOS},
  {NAU8811_REG_ADC_VOLUME,         NAU8811_REG_ADC_VOLUME_MAGIC_BIT | NAU8811_REG_ADC_VOLUME_ADCGAIN_MASK },
  /* Limiter */
  {NAU8811_REG_DAC_LIMITER_1,      0x032}, /* Default */
  {NAU8811_REG_DAC_LIMITER_2,      0x000}, /* Default */
  /* Notch filters */
  {NAU8811_REG_NOTCH_FILTER_1,     0x000}, /* Default */
  {NAU8811_REG_NOTCH_FILTER_2,     0x000}, /* Default */
  {NAU8811_REG_NOTCH_FILTER_3,     0x000}, /* Default */
  {NAU8811_REG_NOTCH_FILTER_4,     0x000}, /* Default */
  /* ALC control */
  {NAU8811_REG_ALC_CTRL_1,         0x038}, /* Default */
  {NAU8811_REG_ALC_CTRL_2,         0x00b}, /* Default */
  {NAU8811_REG_ALC_CTRL_3,         0x032}, /* Default */
  /* Input, Output, Mixers, and Noise */
  {NAU8811_REG_NOISE_GATE,         0x000}, /* Default */
  /* [8811_I2S_ADCOUT/BYP Control] */
  {NAU8811_REG_ATTENUATION_CTRL,   0x000}, /* Default */
  {NAU8811_REG_INPUT_CTRL,         0x003}, /* Default */
  {NAU8811_REG_PGA_GAIN,           0x010}, /* Default */
  {NAU8811_REG_ADC_BOOST,          0x177}, /* PGABST, PMICBSTGAIN, AUXBSTGAIN */
  {NAU8811_REG_OUTPUT_CTRL,        0x002}, /* Default */
  {NAU8811_REG_MIXER_CTRL,         NAU8811_REG_MIXER_CTRL_DACSPK},
  {NAU8811_REG_SPKOUT_VOLUME,      0x039}, /* Default */
  {NAU8811_REG_MONO_MIXER_CONTROL, NAU8811_REG_MONO_MIXER_CONTROL_DACMOUT},
};
#endif

/* Function iNAU8811_GetParameter(...) */
/* ==================================================================================================== */
int32_t iNAU8811_GetParameter(e_NAU8811_RegisterArea eParameter, uint32_t *p_uValue)
{
  
#if NAU8811_TARGET_EMBEDDED
  /* Switch depending on parameter */
  switch(eParameter)
  {
    case eStatusRegister:    { *p_uValue = p_sNAU8811_Area->uStatusRegister;    break; }
    case eControlRegister:   { *p_uValue = p_sNAU8811_Area->uControlRegister;   break; }
    case eTxSpiDataRegister: { *p_uValue = p_sNAU8811_Area->uTxSpiDataRegister; break; }
    case eTxIisDataRegister: { *p_uValue = p_sNAU8811_Area->uTxIisDataRegister; break; }
    case eRxIisDataRegister: { *p_uValue = p_sNAU8811_Area->uRxIisDataRegister; break; }
    default:                 { return(NAU8811_RETURN_FAILURE_CODE);             break; }
  }
#else
  /* Helpers */
  eb_format_t s_EBFormat = EB_ADDR32|EB_DATA32;
  eb_status_t s_EBStatus;
  eb_cycle_t  s_EBCycle;
  eb_data_t   s_EBData;
  uint32_t    uRegisterOffset = 0;
  
  /* Switch depending on parameter */
  switch(eParameter)
  {
    case eStatusRegister:    { uRegisterOffset = 0;                 break; }
    case eControlRegister:   { uRegisterOffset = 1;                 break; }
    case eTxSpiDataRegister: { uRegisterOffset = 2;                 break; }
    case eTxIisDataRegister: { uRegisterOffset = 3;                 break; }
    case eRxIisDataRegister: { uRegisterOffset = 4;                 break; }
    default:                 { return(NAU8811_RETURN_FAILURE_CODE); break; }
  }
  
  /* Get data */
  if ((s_EBStatus = eb_cycle_open(s_DeviceName, 0, eb_block, &s_EBCycle)) != EB_OK)
  {
    printf("Error: Failed to create cycle: %s\n", eb_status(s_EBStatus));
    return(NAU8811_RETURN_FAILURE_CODE);
  }
  eb_cycle_read(s_EBCycle, (eb_address_t)(p_uNAU8811_Area+uRegisterOffset), s_EBFormat, &s_EBData); 
  eb_cycle_close(s_EBCycle);
  
  /* Convert eb_data_t to uint32_t */
  *p_uValue = (uint32_t) s_EBData;
#endif
  
  /* Operation done */
  return(NAU8811_RETURN_SUCCESS_CODE);
  
}

/* Function iNAU8811_SetParameter(...) */
/* ==================================================================================================== */
int32_t iNAU8811_SetParameter(e_NAU8811_RegisterArea eParameter, uint32_t uValue)
{
  
#if NAU8811_TARGET_EMBEDDED
  /* Switch depending on parameter */
  switch(eParameter)
  {
    case eStatusRegister:    { p_sNAU8811_Area->uStatusRegister    = uValue; break; }
    case eControlRegister:   { p_sNAU8811_Area->uControlRegister   = uValue; break; }
    case eTxSpiDataRegister: { p_sNAU8811_Area->uTxSpiDataRegister = uValue; break; }
    case eTxIisDataRegister: { p_sNAU8811_Area->uTxIisDataRegister = uValue; break; }
    case eRxIisDataRegister: { p_sNAU8811_Area->uRxIisDataRegister = uValue; break; }
    default:                 { return(NAU8811_RETURN_FAILURE_CODE);          break; }
  }
#else
  /* Helpers */
  eb_format_t s_EBFormat = EB_ADDR32|EB_DATA32;
  eb_status_t s_EBStatus;
  eb_cycle_t  s_EBCycle;
  uint32_t    uRegisterOffset = 0;
  
  /* Switch depending on parameter */
  switch(eParameter)
  {
    case eStatusRegister:    { uRegisterOffset = 0;                 break; }
    case eControlRegister:   { uRegisterOffset = 1;                 break; }
    case eTxSpiDataRegister: { uRegisterOffset = 2;                 break; }
    case eTxIisDataRegister: { uRegisterOffset = 3;                 break; }
    case eRxIisDataRegister: { uRegisterOffset = 4;                 break; }
    default:                 { return(NAU8811_RETURN_FAILURE_CODE); break; }
  }
  
  /* Write data */
  if ((s_EBStatus = eb_cycle_open(s_DeviceName, 0, eb_block, &s_EBCycle)) != EB_OK)
  {
    printf("Error: Failed to create cycle: %s\n", eb_status(s_EBStatus));
    return(NAU8811_RETURN_FAILURE_CODE);
  }
  eb_cycle_write(s_EBCycle, (eb_address_t)(p_uNAU8811_Area+uRegisterOffset), s_EBFormat, uValue);      
  eb_cycle_close(s_EBCycle);
#endif
  
  /* Operation done */
  return(NAU8811_RETURN_SUCCESS_CODE);
  
}

/* Function iNAU8811_AutoInitialize(...) */
/* ==================================================================================================== */
int32_t iNAU8811_AutoInitialize(void)
{
  
#if NAU8811_TARGET_EMBEDDED
  /* Find device via common functions */
  p_sNAU8811_Area = (s_NAU8811_RegisterArea*) find_device_adr(GSI, NAU8811_AUD_DRIVER);
  
  /* Check if device was found */
  if(p_sNAU8811_Area == NULL)
  {
    /* Device was not found */
    return(NAU8811_RETURN_FAILURE_CODE);
  }
  else
  {
    /* Set control register zo zero */
    iNAU8811_SetParameter(eControlRegister, 0);
    
    /* Device was found and resetted */
    return(NAU8811_RETURN_SUCCESS_CODE);
  }
#else
  /* Helper */
  struct sdb_device s_NAU8811Sound;
  int32_t iDevicesFound = 1;
  
  /* Get device */
  eb_sdb_find_by_identity(s_DeviceName, NAU8811_GSI_VENDOR_ID, NAU8811_GSI_DEVICE_ID, &s_NAU8811Sound, &iDevicesFound);
  
  /* Check if device was found */
  if (!iDevicesFound)
  { 
    /* Device was not found */
    return(NAU8811_RETURN_FAILURE_CODE);
  }
  else
  {
    /* Device was found */
    p_uNAU8811_Area = (uint32_t*)(s_NAU8811Sound.sdb_component.addr_first);
    return(NAU8811_RETURN_SUCCESS_CODE);
  }
#endif
  
}

/* Function iNAU8811_ManualInitialize(...) */
/* ==================================================================================================== */
int32_t iNAU8811_ManualInitialize(uint32_t uAreaAddress)
{
  
#if NAU8811_TARGET_EMBEDDED
  /* Set address directly */
  p_sNAU8811_Area = (s_NAU8811_RegisterArea*) uAreaAddress;
  
  /* Set control register zo zero */
  iNAU8811_SetParameter(eControlRegister, 0);
  
  /* No plausibility check here */
  return(NAU8811_RETURN_SUCCESS_CODE);  
#else
  /* Set address directly */
  p_uNAU8811_Area = (uint32_t*) uAreaAddress;
  
  /* No plausibility check here */
  return(NAU8811_RETURN_SUCCESS_CODE);
#endif
  
}

/* Function iNAU8811_ConfigureDevice(...) */
/* ==================================================================================================== */
int32_t iNAU8811_ConfigureDevice (void)
{
  
  /* Helper */
  uint32_t uPacketCounter = 0;
  uint32_t uTransmitCtrlData = 0;
  volatile uint32_t uSendDelay = 0;
  
  /* Send complete configuration table */
  for (uPacketCounter=0; uPacketCounter<NAU8811_ARRAY_LENGTH(a_uDevConfig); uPacketCounter++)
  {
    /* Build packet from address and payload */
    uTransmitCtrlData = (uint32_t) (a_uDevConfig[uPacketCounter][0] << NAU8811_ADDRESS_SHIFT) | a_uDevConfig[uPacketCounter][1];
    /* Send packet */
    if(iNAU8811_TransmitCtrlData(uTransmitCtrlData)) { return(NAU8811_RETURN_FAILURE_CODE); }
  }

  /* Wait until SPI unit has send all data */
  while (uSendDelay<NAU8811_MAX_WAIT_CC_UNTIL_TX_DONE) { uSendDelay++; }
  
  /* Turn on clock */
  if(iNAU8811_SetParameter(eControlRegister, NAU8811_REG_CONTROL_CLOCK_ENABLE|NAU8811_REG_CONTROL_MONO_OUTPUT)) { return(NAU8811_RETURN_FAILURE_CODE); }

  /* Done */
  return(NAU8811_RETURN_SUCCESS_CODE);
  
}

/* Function iNAU8811_TransmitCtrlData(...) */
/* ==================================================================================================== */
int32_t iNAU8811_TransmitCtrlData (uint32_t uValue)
{
  
  /* Helper */
  uint32_t uGetStatusRegister = 0;
  volatile uint32_t uTimeout = 0;
  volatile uint32_t uSendDelay = 0;
  
  /* Wait until SPI unit has send all data */
  while (uSendDelay<5) { uSendDelay++; }
  
  /* Wait for empty flag or timeout (we must wait here for the empty flag to prevent timing violations) */
  do
  {
    /* Get flags */
    if(iNAU8811_GetParameter(eStatusRegister, &uGetStatusRegister)) { return(NAU8811_RETURN_FAILURE_CODE); }
    
    /* Check for timeout */
    if (uTimeout>=NAU8811_MAX_WAIT_CC_UNTIL_TX_DONE) { return(NAU8811_RETURN_FAILURE_CODE); }
    uTimeout++;
  } while (!(uGetStatusRegister&NAU8811_REG_STATUS_SPI_TX_FIFO_EMPTY));
  
  /* Send data */
  if(iNAU8811_SetParameter(eTxSpiDataRegister, uValue)) { return(NAU8811_RETURN_FAILURE_CODE); }
  
  /* Wait until SPI unit has send all data */
  while (uSendDelay<5) { uSendDelay++; }
  
  /* Done */
  return(NAU8811_RETURN_SUCCESS_CODE);
  
}

/* Function iNAU8811_TransmitData(...) */
/* ==================================================================================================== */
int32_t iNAU8811_TransmitData (uint32_t *p_uData, uint32_t uSize)
{
  
  /* Helper */
  volatile uint32_t uTimeout = 0;
  uint32_t uDataCounter = 0;
  uint32_t uGetStatusRegister = 0;
    
  /* Loop for each item */
  for (uDataCounter = 0; uDataCounter<uSize; uDataCounter++)
  {
    /* Wait for finish flag or timeout */
    do
    {
      /* Get flags */
      if(iNAU8811_GetParameter(eStatusRegister, &uGetStatusRegister)) { return(NAU8811_RETURN_FAILURE_CODE); }
      
      /* Check for timeout */
      if (uTimeout>=NAU8811_MAX_WAIT_CC_UNTIL_TX_DONE) { return(NAU8811_RETURN_FAILURE_CODE); }
      uTimeout++;
    } while (uGetStatusRegister&NAU8811_REG_STATUS_IIS_TX_FIFO_FULL);

    /* Send data */
    if(iNAU8811_SetParameter(eTxIisDataRegister, p_uData[uDataCounter])) { return(NAU8811_RETURN_FAILURE_CODE); }
  }
  
  /* Done */
  return(NAU8811_RETURN_SUCCESS_CODE);
  
}

/* Function iNAU8811_ReceiveData(...) */
/* ==================================================================================================== */
int32_t iNAU8811_ReceiveData (uint32_t *p_uData, uint32_t uSize)
{
  
  /* Helper */
  volatile uint32_t uTimeout = 0;
  uint32_t uDataCounter = 0;
  uint32_t uGetStatusRegister = 0;
  
  /* Loop for each item */
  for (uDataCounter = 0; uDataCounter<uSize; uDataCounter++)
  {
    /* Wait for not empty flag or timeout */
    do
    {
      /* Get flags */
      if(iNAU8811_GetParameter(eStatusRegister, &uGetStatusRegister)) { return(NAU8811_RETURN_FAILURE_CODE); }
      
      /* Check for timeout */
      if (uTimeout>=NAU8811_MAX_WAIT_CC_UNTIL_RX_DONE) { return(NAU8811_RETURN_FAILURE_CODE); }
      uTimeout++;
    } while (uGetStatusRegister&NAU8811_REG_STATUS_IIS_RX_FIFO_EMPTY);
    
    /* Get data */
    if(iNAU8811_GetParameter(eRxIisDataRegister, &p_uData[uDataCounter])) { return(NAU8811_RETURN_FAILURE_CODE); }
  }
      
  /* Done */
  return(NAU8811_RETURN_SUCCESS_CODE);
  
}

/* Function iNAU8811_CleanRxFifo(...) */
/* ==================================================================================================== */
int32_t iNAU8811_CleanRxFifo (uint32_t uReadCycles)
{
  /* Helper */
  uint32_t uDataCounter = 0;
  uint32_t uGetRxValue = 0;

  /* Loop for each item */
  for (uDataCounter = 0; uDataCounter<uReadCycles; uDataCounter++)
  {
    /* Get flags */
    if(iNAU8811_GetParameter(eRxIisDataRegister, &uGetRxValue)) { return(NAU8811_RETURN_FAILURE_CODE); }
  }
  
  /* Done */
  return(NAU8811_RETURN_SUCCESS_CODE);
}

#if NAU8811_TARGET_EMBEDDED
/* Function vNAU8811_TransmitStream(...) */
/* ==================================================================================================== */
void vNAU8811_TransmitStream (uint32_t uData)
{
  p_sNAU8811_Area->uTxIisStreamRegister = uData;
}
#endif

/* Function vNAU8811_ReceiveStream(...) */
/* ==================================================================================================== */
#if NAU8811_TARGET_EMBEDDED
void vNAU8811_ReceiveStream (uint32_t *p_uData)
{
  *p_uData = p_sNAU8811_Area->uRxIisStreamRegister;
}
#endif
