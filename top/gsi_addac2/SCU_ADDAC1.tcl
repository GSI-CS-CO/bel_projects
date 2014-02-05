set project_name FG900160_SCU_ADDAC1;
set esf FG900160_SCU_ADDAC1 ;#from name.esf
set csf FG900160_SCU_ADDAC1 ;#from name.csf

set_global_assignment -name FAMILY "Arria II GX"
set_global_assignment -name DEVICE EP2AGX125DF25C5

set_global_assignment -name CYCLONEII_RESERVE_NCEO_AFTER_CONFIGURATION "USE AS REGULAR IO"
set_global_assignment -name STRATIX_DEVICE_IO_STANDARD "3.3-V LVTTL"

set_global_assignment -name IOBANK_VCCIO 3.3V -section_id 3A
set_global_assignment -name IOBANK_VCCIO 3.3V -section_id 6A

set_global_assignment -name IOBANK_VCCIO 2.5V -section_id 4A
set_global_assignment -name IOBANK_VCCIO 3.3V -section_id 5A
set_global_assignment -name IOBANK_VCCIO 3.3V -section_id 7A
set_global_assignment -name IOBANK_VCCIO 3.3V -section_id 8A

set_location_assignment PIN_E4			 -to ADC_BUSY               		
set_location_assignment PIN_A4          -to ADC_CONVST_A           	
set_location_assignment PIN_C4          -to ADC_CONVST_B           	
set_location_assignment PIN_N2          -to ADC_DB[0]              	
set_location_assignment PIN_L3          -to ADC_DB[1]              	
set_location_assignment PIN_L4          -to ADC_DB[2]              	
set_location_assignment PIN_K2          -to ADC_DB[3]              	
set_location_assignment PIN_K3          -to ADC_DB[4]              	
set_location_assignment PIN_J3          -to ADC_DB[5]              	
set_location_assignment PIN_J4          -to ADC_DB[6]              	
set_location_assignment PIN_H4          -to ADC_DB[7]              	
set_location_assignment PIN_H3          -to ADC_DB[8]              	
set_location_assignment PIN_G2          -to ADC_DB[9]              	
set_location_assignment PIN_G1          -to ADC_DB[10]             	
set_location_assignment PIN_G3          -to ADC_DB[11]             	
set_location_assignment PIN_F3          -to ADC_DB[12]             	
set_location_assignment PIN_E3          -to ADC_DB[13]             	
set_location_assignment PIN_D3          -to ADC_DB14             	
set_location_assignment PIN_G4          -to ADC_DB15               	
set_location_assignment PIN_F4          -to ADC_FRSTDATA           	
set_location_assignment PIN_B1          -to ADC_OS[0]              	
set_location_assignment PIN_C1          -to ADC_OS[1]              	
set_location_assignment PIN_D4          -to ADC_OS[2]              	
set_location_assignment PIN_B3          -to ADC_RANGE              	
set_location_assignment PIN_B4          -to ADC_RESET              	
set_location_assignment PIN_F20         -to AS_CLK                 	
set_location_assignment PIN_J20         -to AS_DI                  	
set_location_assignment PIN_J19         -to AS_DO                  	
set_location_assignment PIN_AB16    -to     A_A[0]                 	
set_location_assignment PIN_V15     -to     A_A[1]                 	
set_location_assignment PIN_AC21    -to     A_A[2]                 	
set_location_assignment PIN_AB14    -to     A_A[3]                 	
set_location_assignment PIN_V13     -to     A_A[4]                 	
set_location_assignment PIN_AC18    -to     A_A[5]                 	
set_location_assignment PIN_AA14    -to     A_A[6]                 	
set_location_assignment PIN_AB17    -to     A_A[7]                 	
set_location_assignment PIN_AB21    -to     A_A[8]                 	
set_location_assignment PIN_AA16    -to     A_A[9]                 	
set_location_assignment PIN_W13     -to     A_A[10]                	
set_location_assignment PIN_W15     -to     A_A[11]                	
set_location_assignment PIN_W14     -to     A_A[12]                	
set_location_assignment PIN_V16     -to     A_A[13]                	
set_location_assignment PIN_U16     -to     A_A[14]                	
set_location_assignment PIN_V14     -to     A_A[15]                	
set_location_assignment PIN_D14         -to A_ADCDAC_SEL[0]       	
set_location_assignment PIN_D1          -to A_ADCDAC_SEL[1]       	
set_location_assignment PIN_D2          -to A_ADCDAC_SEL[2]       	
set_location_assignment PIN_C2          -to A_ADCDAC_SEL[3]       	
set_location_assignment PIN_A5      -to     A_D[0]                 	
set_location_assignment PIN_B6      -to     A_D[1]                 	
set_location_assignment PIN_C7      -to     A_D[2]                 	
set_location_assignment PIN_D8      -to     A_D[3]                 	
set_location_assignment PIN_C9      -to     A_D[4]                 	
set_location_assignment PIN_F10     -to     A_D[5]                 	
set_location_assignment PIN_E10     -to     A_D[6]                 	
set_location_assignment PIN_A10     -to     A_D[7]                 	
set_location_assignment PIN_A6      -to     A_D[8]                 	
set_location_assignment PIN_B7      -to     A_D[9]                 	
set_location_assignment PIN_D9      -to     A_D[10]                	
set_location_assignment PIN_F9      -to     A_D[11]                	
set_location_assignment PIN_G10     -to     A_D[12]                	
set_location_assignment PIN_G11     -to     A_D[13]                	
set_location_assignment PIN_E9      -to     A_D[14]                	
set_location_assignment PIN_B10     -to     A_D[15]                	
set_location_assignment PIN_R6          -to A_Ext_Data_RD          	
set_location_assignment PIN_B13         -to A_MODE_SEL[0]          	
set_location_assignment PIN_C13         -to A_MODE_SEL[1]          	
set_location_assignment PIN_A7      -to     A_nADR_EN              	
set_location_assignment PIN_D15     -to     A_nADR_FROM_SCUB       	
set_location_assignment PIN_AB1         -to A_nBoardSel            	
set_location_assignment PIN_D12         -to A_nDS                  	
set_location_assignment PIN_AA1         -to A_nDtack               	
set_location_assignment PIN_B16         -to A_nEvent_Str           	

set_location_assignment PIN_AC13        -to A_nLED[0] 
set_instance_assignment -name IO_MAXIMUM_TOGGLE_RATE "0 MHz" -to A_nLED[0]             	

set_location_assignment PIN_AB11        -to A_nLED[1]              	
set_location_assignment PIN_AB10        -to A_nLED[2]              	
set_location_assignment PIN_AA7         -to A_nLED[3]              	
set_location_assignment PIN_AB6         -to A_nLED[4]              	
set_location_assignment PIN_W11         -to A_nLED[5]              	
set_location_assignment PIN_W9          -to A_nLED[6]              	
set_location_assignment PIN_V12         -to A_nLED[7]              	
set_location_assignment PIN_W12         -to A_nLED[8]
              	
set_location_assignment PIN_AB13        -to A_nLED[9]              	
set_instance_assignment -name IO_MAXIMUM_TOGGLE_RATE "0 MHz" -to A_nLED[9]

set_location_assignment PIN_AA11        -to A_nLED[10]             	
set_location_assignment PIN_AA10        -to A_nLED[11]             	
set_location_assignment PIN_AB7         -to A_nLED[12]             	
set_location_assignment PIN_AA6         -to A_nLED[13]             	
set_location_assignment PIN_W10         -to A_nLED[14]             	
set_location_assignment PIN_V11         -to A_nLED[15]
set_instance_assignment -name  IO_STANDARD "2.5-V" -to A_nLED[15..0]
           	
set_location_assignment PIN_U9          -to A_NLED_TRIG_ADC        	
set_location_assignment PIN_V9          -to A_NLED_TRIG_DAC        	
set_location_assignment PIN_C12          -to A_nReset
               	
set_location_assignment PIN_E12         -to A_nSel_Ext_Data_Drv    	
set_location_assignment PIN_C16         -to A_nSRQ

set_instance_assignment -name IO_STANDARD "2.5-V" -to A_nState_LED[2..0]
set_location_assignment PIN_AA8     -to      A_nState_LED[0]        	
set_location_assignment PIN_AD6     -to      A_nState_LED[1]        	
set_location_assignment PIN_AC6     -to      A_nState_LED[2]

#set_location_assignment PIN_A18         -to A_NUSER_IO_DATA0-7_EN   	
#set_location_assignment PIN_C19         -to A_NUSER_IO_DATA8-15_EN   	
#set_location_assignment PIN_P7       	 -to A_NUSER_IO_DATA16-23_EN   
#set_location_assignment PIN_N7       	 -to A_NUSER_IO_DATA24-31_EN   
set_location_assignment PIN_A18     -to      a_ext_io_7_0_dis   	
set_location_assignment PIN_C19     -to      a_ext_io_15_8_dis
set_location_assignment PIN_P7      -to      a_ext_io_23_16_dis
set_location_assignment PIN_N7      -to      a_ext_io_31_24_dis

set_location_assignment PIN_C6          -to A_OneWire              	
set_location_assignment PIN_D6          -to A_OneWire_EEPROM       	
set_location_assignment PIN_A13         -to A_RnW                  	
set_location_assignment PIN_B9          -to A_SEL[0]               	
set_location_assignment PIN_A9          -to A_SEL[1]               	
set_location_assignment PIN_D10         -to A_SEL[2]               	
set_location_assignment PIN_C10         -to A_SEL[3]               	
set_location_assignment PIN_G9          -to A_Spare[0]             	
set_location_assignment PIN_H9          -to A_Spare[1]             	
set_location_assignment PIN_B12         -to A_SysClock             	
set_location_assignment PIN_U6          -to A_TA[0]                	
set_location_assignment PIN_Y1          -to A_TA[1]                	
set_location_assignment PIN_AD4         -to A_TA[2]                	
set_location_assignment PIN_W1          -to A_TA[3]                	
set_location_assignment PIN_AB4         -to A_TA[4]                	
set_location_assignment PIN_T2          -to A_TA[5]                	
set_location_assignment PIN_T3          -to A_TA[6]                	
set_location_assignment PIN_R3          -to A_TA[7]                	
set_location_assignment PIN_V7          -to A_TA[8]                	
set_location_assignment PIN_AA4         -to A_TA[9]                	
set_location_assignment PIN_V6          -to A_TA[10]               	
set_location_assignment PIN_R4          -to A_TA[11]               	
set_location_assignment PIN_T6          -to A_TA[12]               	
set_location_assignment PIN_T1          -to A_TA[13]               	
set_location_assignment PIN_AD5         -to A_TA[14]               	
set_location_assignment PIN_R1          -to A_TA[15]               	
set_location_assignment PIN_V5          -to A_TB[0]                	
set_location_assignment PIN_AA5         -to A_TB[1]                	
set_location_assignment PIN_AC4         -to A_TB[2]                	
set_location_assignment PIN_AA3         -to A_TB[3]                	
set_location_assignment PIN_AB2         -to A_TB[4]                	
set_location_assignment PIN_Y4          -to A_TB[5]                	
set_location_assignment PIN_AB3         -to A_TB[6]                	
set_location_assignment PIN_AC1         -to A_TB[7]                	
set_location_assignment PIN_Y3          -to A_TB[8]                	
set_location_assignment PIN_W4          -to A_TB[9]                	
set_location_assignment PIN_W3          -to A_TB[10]               	
set_location_assignment PIN_U4          -to A_TB[11]               	
set_location_assignment PIN_V4          -to A_TB[12]               	
set_location_assignment PIN_AB5         -to A_TB[13]               	
set_location_assignment PIN_U3          -to A_TB[14]               	
set_location_assignment PIN_AC3         -to A_TB[15]               	
set_location_assignment PIN_G19         -to A_TDI                  	
set_location_assignment PIN_G20         -to A_TDO                  	
set_location_assignment PIN_Y22         -to A_TX0N                 	
set_location_assignment PIN_Y21         -to A_TX0P                 	
set_location_assignment PIN_K21         -to A_TX4P                 	
set_location_assignment PIN_K22         -to A_TX4N
                 	
#set_location_assignment PIN_A17         -to A_USER_IO_DATA0-7_TX   	
#set_location_assignment PIN_B18         -to A_USER_IO_DATA8-15_TX   
#set_location_assignment PIN_N6          -to A_USER_IO_DATA24-31_TX   	
#set_location_assignment PIN_R7          -to A_USER_IO_DATA16-23_TX   	
set_location_assignment PIN_A17     -to a_io_7_0_tx   	
set_location_assignment PIN_B18     -to a_io_15_8_tx	
set_location_assignment PIN_N6      -to a_io_23_16_tx   	
set_location_assignment PIN_R7      -to a_io_31_24_tx   	

# correct port name from A_USER_IO_DATA[n] -to a_io[n]      	
set_location_assignment PIN_A11     -to    a_io[0]      	
set_location_assignment PIN_A12     -to    a_io[1]      	
set_location_assignment PIN_F13     -to    a_io[2]      	
set_location_assignment PIN_D13     -to    a_io[3]      	
set_location_assignment PIN_A15     -to    a_io[4]      	
set_location_assignment PIN_A16     -to    a_io[5]      	
set_location_assignment PIN_A19     -to    a_io[6]      	
set_location_assignment PIN_A20     -to    a_io[7]      	
set_location_assignment PIN_B15     -to    a_io[8]      	
set_location_assignment PIN_A14     -to    a_io[9]      	
set_location_assignment PIN_E13     -to    a_io[10]     	
set_location_assignment PIN_E15     -to    a_io[11]     	
set_location_assignment PIN_F14     -to    a_io[12]     	
set_location_assignment PIN_F15     -to    a_io[13]     	
set_location_assignment PIN_G14     -to    a_io[14]     	
set_location_assignment PIN_G13     -to    a_io[15]     	
set_location_assignment PIN_AB15    -to    a_io[16]     	
set_location_assignment PIN_Y15     -to    a_io[17]     	
set_location_assignment PIN_AA15    -to    a_io[18]     	
set_location_assignment PIN_AA19    -to    a_io[19]     	
set_location_assignment PIN_AB19    -to    a_io[20]     	
set_location_assignment PIN_AC19    -to    a_io[21]     	
set_location_assignment PIN_AD21    -to    a_io[22]     	
set_location_assignment PIN_AD20    -to    a_io[23]     	
set_location_assignment PIN_AD19    -to    a_io[24]     	
set_location_assignment PIN_AB18    -to    a_io[25]     	
set_location_assignment PIN_AD18    -to    a_io[26]     	
set_location_assignment PIN_AA18    -to    a_io[27]     	
set_location_assignment PIN_AD17    -to    a_io[28]     	
set_location_assignment PIN_AD16    -to    a_io[29]
set_instance_assignment -name IO_MAXIMUM_TOGGLE_RATE "0 MHz" -to a_io[29]     	
set_location_assignment PIN_AC15    -to    a_io[30]     	
set_location_assignment PIN_AD15    -to    a_io[31]
set_instance_assignment -name IO_MAXIMUM_TOGGLE_RATE "0 MHz" -to a_io[31]     	     	

#set_location_assignment PIN_AD14        -to CLK_FPGAN              	
set_location_assignment PIN_AD13        -to clk_fpga              	
set_instance_assignment -name  IO_STANDARD "LVDS" -to clk_fpga

set_location_assignment PIN_H1          -to DAC2_SDI               	
set_location_assignment PIN_L7          -to DAC1_SDI               	
set_location_assignment PIN_J5          -to DAC2_SDO               	
set_location_assignment PIN_L1          -to DAC1_SDO               	
set_location_assignment PIN_U1          -to EXT_TRIG_ADC           	
set_location_assignment PIN_V1          -to EXT_TRIG_DAC           	
set_location_assignment PIN_AD11        -to HW_REV[0]              	
set_location_assignment PIN_AD12        -to HW_REV[1]              	
set_location_assignment PIN_AB12        -to HW_REV[2]              	
set_location_assignment PIN_AC12        -to HW_REV[3]              	
set_location_assignment PIN_Y19         -to MS[1]                  	
set_location_assignment PIN_T19         -to MS[2]                  	
set_location_assignment PIN_W20         -to MS[3]                  	
set_location_assignment PIN_T20         -to MS[4]                  	
set_location_assignment PIN_A2          -to NADC_CS                	
set_location_assignment PIN_C3          -to NADC_PAR_SER_SEL       	
set_location_assignment PIN_A3          -to NADC_RD_SCLK           	
set_location_assignment PIN_H19         -to NAS_CS                 	
set_location_assignment PIN_V20         -to NCONFIG                	
set_location_assignment PIN_P1          -to NDAC1_CLK              	
set_location_assignment PIN_K4          -to NDAC2_CLK              	
set_location_assignment PIN_N1          -to NDAC1_A[0]             	
set_location_assignment PIN_K1          -to NDAC1_CLR              	
set_location_assignment PIN_K5          -to NDAC2_A[0]             	
set_location_assignment PIN_J6          -to NDAC2_CLR              	
set_location_assignment PIN_M7          -to NDAC1_A[1]             	
set_location_assignment PIN_J1          -to NDAC2_A[1]             	
set_location_assignment PIN_F1          -to NDIFF_IN_EN            	
set_location_assignment PIN_R5          -to nExtension_Res_Out     	
                	

#set_location_assignment PIN_F22         -to Q1_CLK5N               	
set_location_assignment PIN_F21         -to Q1_CLK5              	
#set_instance_assignment -name  IO_STANDARD "LVDS" -to Q1_CLK5

#set_location_assignment PIN_U24         -to Q0_CLK0N     
set_location_assignment PIN_U23         -to Q0_CLK0               	
#set_instance_assignment -name  IO_STANDARD "LVDS" -to Q0_CLK0

#set_location_assignment PIN_G24         -to Q1_CLK1N
set_location_assignment PIN_G23         -to Q1_CLK1              	
#set_instance_assignment -name  IO_STANDARD "LVDS" -to Q1_CLK1


set_location_assignment PIN_L23         -to Q1_RX4PB               	
set_location_assignment PIN_AA24        -to Q0_RX0N                	
set_location_assignment PIN_AA23        -to Q0_RX0P             	
set_location_assignment PIN_L24         -to Q1_RX4NB               	
          	
set_location_assignment PIN_D18         -to TCK                    	
set_location_assignment PIN_G17         -to TMS                    	
set_location_assignment PIN_AD2         -to TP[1]                  	
set_location_assignment PIN_AD3         -to TP[2]                                      