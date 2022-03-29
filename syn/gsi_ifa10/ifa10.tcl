# Copyright (C) 2018  Intel Corporation. All rights reserved.
# Your use of Intel Corporation's design tools, logic functions 
# and other software and tools, and its AMPP partner logic 
# functions, and any output files from any of the foregoing 
# (including device programming or simulation files), and any 
# associated documentation or information are expressly subject 
# to the terms and conditions of the Intel Program License 
# Subscription Agreement, the Intel Quartus Prime License Agreement,
# the Intel FPGA IP License Agreement, or other applicable license
# agreement, including, without limitation, that your use is for
# the sole purpose of programming logic devices manufactured by
# Intel and sold by Intel or its authorized distributors.  Please
# refer to the applicable agreement for further details.

# Quartus Prime: Generate Tcl File for Project
# File: ifa10.tcl
# Generated on: Mon Sep 21 17:44:23 2020

# Load Quartus Prime Tcl Project package
package require ::quartus::project

set need_to_close_project 0
set make_assignments 1

# Check that the right project is open
if {[is_project_open]} {
	if {[string compare $quartus(project) "ifa10"]} {
		puts "Project ifa10 is not open"
		set make_assignments 0
	}
} else {
	# Only open if not already open
	if {[project_exists ifa10]} {
		project_open -revision ifa10 ifa10
	} else {
		project_new -revision ifa10 ifa10
	}
	set need_to_close_project 1
}

# Make assignments
if {$make_assignments} {
	set_global_assignment -name ORIGINAL_QUARTUS_VERSION 16.1.0
	set_global_assignment -name PROJECT_CREATION_TIME_DATE "12:02:14  APRIL 10, 2019"
	set_global_assignment -name LAST_QUARTUS_VERSION "18.1.0 Lite Edition"
	set_global_assignment -name PROJECT_OUTPUT_DIRECTORY output_files
	set_global_assignment -name NUM_PARALLEL_PROCESSORS ALL
	set_global_assignment -name FLOW_ENABLE_IO_ASSIGNMENT_ANALYSIS ON
	set_global_assignment -name MIN_CORE_JUNCTION_TEMP 0
	set_global_assignment -name MAX_CORE_JUNCTION_TEMP 85
	set_global_assignment -name TIMING_ANALYZER_MULTICORNER_ANALYSIS ON
	set_global_assignment -name TIMING_ANALYZER_DO_REPORT_TIMING ON
	set_global_assignment -name TIMING_ANALYZER_REPORT_SCRIPT ifa10.tcl
	set_global_assignment -name FAMILY "MAX 10"
	set_global_assignment -name TOP_LEVEL_ENTITY Ifa10
	set_global_assignment -name EDA_DESIGN_ENTRY_SYNTHESIS_TOOL Synplify
	set_global_assignment -name PROJECT_IP_REGENERATION_POLICY ALWAYS_REGENERATE_IP
	set_global_assignment -name DEVICE_FILTER_PIN_COUNT 672
	set_global_assignment -name DEVICE_FILTER_SPEED_GRADE 7
	set_global_assignment -name DEVICE 10M50DAF672C7G
	set_global_assignment -name ERROR_CHECK_FREQUENCY_DIVISOR 256
	set_global_assignment -name STRATIXV_CONFIGURATION_SCHEME "PASSIVE SERIAL"
	set_global_assignment -name ENABLE_CRC_ERROR_PIN ON
	set_global_assignment -name CRC_ERROR_OPEN_DRAIN ON
	set_global_assignment -name STRATIX_DEVICE_IO_STANDARD "3.3-V LVCMOS"
	set_global_assignment -name INTERNAL_FLASH_UPDATE_MODE "SINGLE COMP IMAGE WITH ERAM"
	set_global_assignment -name FORCE_CONFIGURATION_VCCIO ON
	set_global_assignment -name EDA_SIMULATION_TOOL "ModelSim (VHDL)"
	set_global_assignment -name EDA_BOARD_DESIGN_TIMING_TOOL "Stamp (Timing)"
	set_global_assignment -name EDA_BOARD_DESIGN_SYMBOL_TOOL "ViewDraw (Symbol)"
	set_global_assignment -name EDA_BOARD_DESIGN_SIGNAL_INTEGRITY_TOOL "IBIS (Signal Integrity)"
	set_global_assignment -name ENABLE_OCT_DONE OFF
	set_global_assignment -name USE_CONFIGURATION_DEVICE OFF
	set_global_assignment -name EXTERNAL_FLASH_FALLBACK_ADDRESS 00000000
	set_global_assignment -name ON_CHIP_BITSTREAM_DECOMPRESSION OFF
	set_global_assignment -name ENABLE_SIGNALTAP OFF
	set_global_assignment -name USE_SIGNALTAP_FILE stp2.stp
	set_global_assignment -name POWER_PRESET_COOLING_SOLUTION "23 MM HEAT SINK WITH 200 LFPM AIRFLOW"
	set_global_assignment -name POWER_BOARD_THERMAL_MODEL "NONE (CONSERVATIVE)"
	set_global_assignment -name POWER_DEFAULT_INPUT_IO_TOGGLE_RATE "12.5 %"
	set_global_assignment -name OUTPUT_IO_TIMING_NEAR_END_VMEAS "HALF VCCIO" -rise
	set_global_assignment -name OUTPUT_IO_TIMING_NEAR_END_VMEAS "HALF VCCIO" -fall
	set_global_assignment -name OUTPUT_IO_TIMING_FAR_END_VMEAS "HALF SIGNAL SWING" -rise
	set_global_assignment -name OUTPUT_IO_TIMING_FAR_END_VMEAS "HALF SIGNAL SWING" -fall
	set_global_assignment -name EDA_TIME_SCALE "1 ps" -section_id eda_simulation
	set_global_assignment -name EDA_OUTPUT_DATA_FORMAT VHDL -section_id eda_simulation
	set_global_assignment -name EDA_LMF_FILE synplcty.lmf -section_id eda_design_synthesis
	set_global_assignment -name EDA_INPUT_DATA_FORMAT VQM -section_id eda_design_synthesis
	set_global_assignment -name EDA_OUTPUT_DATA_FORMAT STAMP -section_id eda_board_design_timing
	set_global_assignment -name EDA_OUTPUT_DATA_FORMAT VIEWDRAW -section_id eda_board_design_symbol
	set_global_assignment -name EDA_OUTPUT_DATA_FORMAT IBIS -section_id eda_board_design_signal_integrity
	set_global_assignment -name PARTITION_NETLIST_TYPE SOURCE -section_id Top
	set_global_assignment -name PARTITION_FITTER_PRESERVATION_LEVEL PLACEMENT_AND_ROUTING -section_id Top
	set_global_assignment -name PARTITION_COLOR 16764057 -section_id Top
	set_global_assignment -name OPTIMIZATION_MODE "HIGH PERFORMANCE EFFORT"
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/Ifa10.vhd
	set_global_assignment -name BDF_FILE ../../top/gsi_ifa10/IFA8_X.bdf
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/ifk10_pkg.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/IO_BUFF.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/IF_Mode_a.vhd
	set_global_assignment -name TCL_SCRIPT_FILE ../../top/gsi_ifa10/ifa10.tcl
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/LAN_MILIO.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/io_dec.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/ifa/irq_mask.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/ifa/ifa.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/LedIO/LED_out.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/LedIO/led_stretch.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/ifa/ifactl.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/FG_Quad/fg_quad_datapath.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/FG_Quad/fg_quad_ifa.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/FG_Quad/fg_quad_ifa_pkg.vhd
	set_global_assignment -name SDC_FILE ifa10.sdc
	set_global_assignment -name SDC_FILE ifa10.out.sdc
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/ifa/build_id_ram.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/dds_out.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/IFA_SWoutSel.vhd
	set_global_assignment -name AHDL_FILE ../../top/gsi_ifa10/FBCtrl_c.tdf
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/MildecTimeEdge/Mil_dec_edge_timed_vhd.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/MIL_IO/A6408_decoder.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/MIL_IO/A6408_encoder.vhd
	set_global_assignment -name BDF_FILE ../../top/gsi_ifa10/MIL_IO/mil_en_decoder_ifa.bdf
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/MIL_IO/Mil_bipol_dec.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/MIL_IO/Mil_dec_edge_timed.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/MIL_IO/mil_serpar.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/MIL_IO/mil_enc_ifa.vhd
	set_global_assignment -name AHDL_FILE ../../top/gsi_ifa10/Debounce/debounce.tdf
	set_global_assignment -name QIP_FILE ../../top/gsi_ifa10/Clock_Timing/PLL_1.qip
	set_global_assignment -name AHDL_FILE ../../top/gsi_ifa10/Clock_Timing/Timebase.tdf
	set_global_assignment -name AHDL_FILE ../../top/gsi_ifa10/Clock_Timing/Freq_Teiler.tdf
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/Clock_Timing/Clock_Timing.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/Clock_Timing/PLL_1.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/IFK_FairBus/Vers_1_Rev_5/ifk_fairbus.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/I2C_SPI/f_divider.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/I2C_SPI/I2C_Cntrl.vhd
	set_global_assignment -name AHDL_FILE ../../top/gsi_ifa10/I2C_SPI/i2c.tdf
	set_global_assignment -name BDF_FILE ../../top/gsi_ifa10/I2C_SPI/I2C_Buff.bdf
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/SWEEP/sweep.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/SWEEP/sweep_cntrl.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/mux/sd_mux_b.vhd
	set_global_assignment -name VHDL_FILE ../../top/gsi_ifa10/mux/EN_mux_test.vhd
	set_global_assignment -name CDF_FILE output_files/Chain1.cdf
	set_global_assignment -name AHDL_FILE ../../top/gsi_ifa10/MBCtrl_f.tdf
	set_location_assignment PIN_G12 -to A_Test[0]
	set_location_assignment PIN_D12 -to A_Test[1]
	set_location_assignment PIN_AF22 -to SPARE[5]
	set_location_assignment PIN_AB20 -to SPARE[8]
	set_location_assignment PIN_AF23 -to SPARE[7]
	set_location_assignment PIN_AD20 -to SPARE[1]
	set_location_assignment PIN_AC16 -to SPARE[2]
	set_location_assignment PIN_AD21 -to SPARE[3]
	set_location_assignment PIN_Y17 -to SPARE[4]
	set_location_assignment PIN_Y16 -to SPARE[6]
	set_location_assignment PIN_A12 -to SHOW_CONFIG
	set_location_assignment PIN_AA19 -to RUP
	set_location_assignment PIN_AA20 -to RDN
	set_location_assignment PIN_B6 -to OE_Test
	set_location_assignment PIN_D8 -to nUSER_LD_LED
	set_location_assignment PIN_E10 -to nFAILSAVE_LD_LED
	set_location_assignment PIN_AF8 -to MCU_RESET_FILTER
	set_location_assignment PIN_AD10 -to MCU_NMI
	set_location_assignment PIN_N1 -to A_ME_12MHZ
	set_location_assignment PIN_U12 -to SPARE[11]
	set_location_assignment PIN_R11 -to A_CLK_50MHz
	set_location_assignment PIN_P7 -to A_CLK_24MHz
	set_location_assignment PIN_AD4 -to EPIOS[15]
	set_location_assignment PIN_AA10 -to EPIOS[14]
	set_location_assignment PIN_AC4 -to EPIOS[13]
	set_location_assignment PIN_Y10 -to EPIOS[12]
	set_location_assignment PIN_AF3 -to EPIOS[11]
	set_location_assignment PIN_AB7 -to EPIOS[10]
	set_location_assignment PIN_AE3 -to EPIOS[9]
	set_location_assignment PIN_AC6 -to EPIOS[8]
	set_location_assignment PIN_AD3 -to EPIOS[7]
	set_location_assignment PIN_AA6 -to EPIOS[6]
	set_location_assignment PIN_AE2 -to EPIOS[5]
	set_location_assignment PIN_AA7 -to EPIOS[4]
	set_location_assignment PIN_AC5 -to EPIOS[3]
	set_location_assignment PIN_Y9 -to EPIOS[2]
	set_location_assignment PIN_AB6 -to EPIOS[1]
	set_location_assignment PIN_Y8 -to EPIOS[0]
	set_location_assignment PIN_F15 -to A_nLED[22]
	set_location_assignment PIN_E15 -to A_nLED[21]
	set_location_assignment PIN_G14 -to A_nLED[20]
	set_location_assignment PIN_D15 -to A_nLED[19]
	set_location_assignment PIN_F14 -to A_nLED[18]
	set_location_assignment PIN_D14 -to A_nLED[17]
	set_location_assignment PIN_L14 -to A_nLED[16]
	set_location_assignment PIN_K14 -to A_nLED[15]
	set_location_assignment PIN_B15 -to A_nLED[14]
	set_location_assignment PIN_B16 -to A_nLED[13]
	set_location_assignment PIN_E14 -to A_nLED[12]
	set_location_assignment PIN_A16 -to A_nLED[11]
	set_location_assignment PIN_D13 -to A_nLED[10]
	set_location_assignment PIN_A15 -to A_nLED[9]
	set_location_assignment PIN_G13 -to A_nLED[8]
	set_location_assignment PIN_C15 -to A_nLED[7]
	set_location_assignment PIN_F13 -to A_nLED[6]
	set_location_assignment PIN_C14 -to A_nLED[5]
	set_location_assignment PIN_L13 -to A_nLED[4]
	set_location_assignment PIN_A14 -to A_nLED[3]
	set_location_assignment PIN_K13 -to A_nLED[2]
	set_location_assignment PIN_C16 -to A_nLED[1]
	set_location_assignment PIN_D17 -to A_nLED[0]
	set_location_assignment PIN_C17 -to A_nSWITCH_Ena
	set_location_assignment PIN_B8 -to A_SEL_B[0]
	set_location_assignment PIN_A9 -to A_SEL_B[1]
	set_location_assignment PIN_A8 -to A_SEL_B[2]
	set_location_assignment PIN_A7 -to A_SEL_B[3]
	set_location_assignment PIN_AA12 -to A_EXT_CLK
	set_location_assignment PIN_R6 -to SPARE[10]
	set_location_assignment PIN_M1 -to SPARE[9]
	set_location_assignment PIN_P11 -to A_ME_DSC
	set_location_assignment PIN_T10 -to A_ME_EE
	set_location_assignment PIN_N4 -to A_ME_BOI
	set_location_assignment PIN_U3 -to A_ME_BZI
	set_location_assignment PIN_P4 -to A_ME_CDS
	set_location_assignment PIN_P2 -to A_ME_ESC
	set_location_assignment PIN_P1 -to A_ME_nBOO
	set_location_assignment PIN_R3 -to A_ME_nBZO
	set_location_assignment PIN_N3 -to A_ME_SD
	set_location_assignment PIN_U10 -to A_ME_SDI
	set_location_assignment PIN_R4 -to A_ME_SDO
	set_location_assignment PIN_V2 -to A_ME_SS
	set_location_assignment PIN_M2 -to A_ME_TD
	set_location_assignment PIN_U4 -to A_ME_UDI
	set_location_assignment PIN_P10 -to A_ME_VW
	set_location_assignment PIN_R7 -to A_MIL1_BOI
	set_location_assignment PIN_P3 -to A_MIL1_BZI
	set_location_assignment PIN_U6 -to A_MIL1_nBOO
	set_location_assignment PIN_T5 -to A_MIL1_nBZO
	set_location_assignment PIN_U5 -to A_MIL1_nIN_Ena
	set_location_assignment PIN_T4 -to A_MIL1_OUT_Ena
	set_location_assignment PIN_AA2 -to A_NLED_EVT_INR
	set_location_assignment PIN_AA4 -to A_nLED_EXTCLK
	set_location_assignment PIN_Y2 -to A_nMANUAL_RES
	set_location_assignment PIN_W1 -to A_nOPK_DRDY
	set_location_assignment PIN_V3 -to A_nOPK_DRQ
	set_location_assignment PIN_V1 -to A_nOPK_INL
	set_location_assignment PIN_U2 -to A_nSEL_6408
	set_location_assignment PIN_W3 -to FS_ASDI
	set_location_assignment PIN_V7 -to FS_DATA
	set_location_assignment PIN_V4 -to FS_DCLK
	set_location_assignment PIN_V6 -to FS_nCS
	set_location_assignment PIN_R2 -to A_UMIL5V
	set_location_assignment PIN_N5 -to A_UMIL15V
	set_location_assignment PIN_AE7 -to A_WAKE
	set_location_assignment PIN_C6 -to IN_NDRQ
	set_location_assignment PIN_D6 -to IN_NINL
	set_location_assignment PIN_E8 -to IN_NRDY
	set_location_assignment PIN_B9 -to A_Test[15]
	set_location_assignment PIN_C9 -to A_Test[14]
	set_location_assignment PIN_D9 -to A_Test[13]
	set_location_assignment PIN_D10 -to A_Test[12]
	set_location_assignment PIN_A10 -to A_Test[11]
	set_location_assignment PIN_B10 -to A_Test[10]
	set_location_assignment PIN_A11 -to A_Test[9]
	set_location_assignment PIN_C10 -to A_Test[8]
	set_location_assignment PIN_B11 -to A_Test[7]
	set_location_assignment PIN_K12 -to A_Test[6]
	set_location_assignment PIN_C12 -to A_Test[5]
	set_location_assignment PIN_L12 -to A_Test[4]
	set_location_assignment PIN_E12 -to A_Test[3]
	set_location_assignment PIN_F12 -to A_Test[2]
	set_location_assignment PIN_AB12 -to A_P[0]
	set_location_assignment PIN_AD11 -to A_P[1]
	set_location_assignment PIN_AC12 -to A_P[2]
	set_location_assignment PIN_AC11 -to A_P[3]
	set_location_assignment PIN_U13 -to A_P[4]
	set_location_assignment PIN_AE10 -to A_P[5]
	set_location_assignment PIN_T13 -to A_P[6]
	set_location_assignment PIN_AF10 -to A_P[7]
	set_location_assignment PIN_AD13 -to A_P[8]
	set_location_assignment PIN_AE11 -to A_P[9]
	set_location_assignment PIN_AD12 -to A_P[10]
	set_location_assignment PIN_AF11 -to A_P[11]
	set_location_assignment PIN_AA13 -to A_P[12]
	set_location_assignment PIN_AE12 -to A_P[13]
	set_location_assignment PIN_Y13 -to A_P[14]
	set_location_assignment PIN_AF12 -to A_P[15]
	set_location_assignment PIN_U14 -to A_P[16]
	set_location_assignment PIN_AF13 -to A_P[17]
	set_location_assignment PIN_T14 -to A_P[18]
	set_location_assignment PIN_AF14 -to A_P[19]
	set_location_assignment PIN_AB13 -to A_P[20]
	set_location_assignment PIN_AE14 -to A_P[21]
	set_location_assignment PIN_AC13 -to A_P[22]
	set_location_assignment PIN_AF15 -to A_P[23]
	set_location_assignment PIN_T15 -to A_P[24]
	set_location_assignment PIN_U15 -to A_P[25]
	set_location_assignment PIN_AF16 -to A_P[26]
	set_location_assignment PIN_AD14 -to A_P[27]
	set_location_assignment PIN_AF17 -to A_P[28]
	set_location_assignment PIN_AC14 -to A_P[29]
	set_location_assignment PIN_Y14 -to A_P[30]
	set_location_assignment PIN_AD15 -to A_P[31]
	set_location_assignment PIN_AA14 -to A_P[32]
	set_location_assignment PIN_AC15 -to A_P[33]
	set_location_assignment PIN_AA18 -to A_P[34]
	set_location_assignment PIN_AE18 -to A_P[35]
	set_location_assignment PIN_Y18 -to A_P[36]
	set_location_assignment PIN_AE17 -to A_P[37]
	set_location_assignment PIN_AD17 -to A_P[38]
	set_location_assignment PIN_AF18 -to A_P[39]
	set_location_assignment PIN_AC17 -to A_P[40]
	set_location_assignment PIN_AF19 -to A_P[41]
	set_location_assignment PIN_AA15 -to A_P[42]
	set_location_assignment PIN_AD18 -to A_P[43]
	set_location_assignment PIN_AB15 -to A_P[44]
	set_location_assignment PIN_AD19 -to A_P[45]
	set_location_assignment PIN_Y22 -to A_I_Out
	set_location_assignment PIN_AD23 -to A_K_Ena
	set_location_assignment PIN_AC22 -to A_K_Out
	set_location_assignment PIN_AC24 -to A_L_Ena
	set_location_assignment PIN_Y20 -to A_L_Out
	set_location_assignment PIN_W22 -to A_I_Ena
	set_location_assignment PIN_AB24 -to A_M_Out
	set_location_assignment PIN_AD26 -to A_M_Ena
	set_location_assignment PIN_AC26 -to A_N_Ena
	set_location_assignment PIN_U17 -to A_N_Out
	set_location_assignment PIN_W23 -to A_X1_Out
	set_location_assignment PIN_AA25 -to A_X1_Ena
	set_location_assignment PIN_AB26 -to A_X2_Out
	set_location_assignment PIN_T17 -to A_X2_Ena
	set_location_assignment PIN_T16 -to A_X3_Out
	set_location_assignment PIN_Y26 -to A_X3_Ena
	set_location_assignment PIN_U23 -to A_X4_Out
	set_location_assignment PIN_T22 -to A_X4_Ena
	set_location_assignment PIN_U24 -to A_X5_Out
	set_location_assignment PIN_Y24 -to A_X5_Ena
	set_location_assignment PIN_Y25 -to A_X6_Out
	set_location_assignment PIN_R16 -to A_X6_Ena
	set_location_assignment PIN_R17 -to A_X7_Out
	set_location_assignment PIN_V25 -to A_X7_Ena
	set_location_assignment PIN_R22 -to A_X8_Out
	set_location_assignment PIN_R20 -to A_X8_Ena
	set_location_assignment PIN_T24 -to A_X9_Out
	set_location_assignment PIN_R25 -to A_X9_Ena
	set_location_assignment PIN_U26 -to A_X10_Out
	set_location_assignment PIN_P16 -to A_X10_Ena
	set_location_assignment PIN_P20 -to A_X11_Out
	set_location_assignment PIN_P23 -to A_X12_Out
	set_location_assignment PIN_R24 -to A_X12_Ena
	set_location_assignment PIN_N26 -to A_B18_B24_Out
	set_location_assignment PIN_M23 -to A_AC26_AC27_Out
	set_location_assignment PIN_K26 -to A_nB18_B24_Ena
	set_location_assignment PIN_J25 -to A_nAC22_AC25_Ena
	set_location_assignment PIN_K24 -to A_nAC26_AC27_Ena
	set_location_assignment PIN_L22 -to A_nAC28_AC31_Ena
	set_location_assignment PIN_F24 -to A_nC20_21_Ena
	set_location_assignment PIN_G23 -to A_nAC19_Ena
	set_location_assignment PIN_E24 -to A_nIO1_Ena
	set_location_assignment PIN_L17 -to A_C20_21_Out
	set_location_assignment PIN_F23 -to A_IO1_Out
	set_location_assignment PIN_M22 -to A_AC28_AC31_Out
	set_location_assignment PIN_F26 -to A_AC22_AC25_Out
	set_location_assignment PIN_J22 -to A_AC19_Out
	set_location_assignment PIN_C5 -to LEMO_OUT
	set_location_assignment PIN_D5 -to LEMO_IN
	set_location_assignment PIN_P22 -to A_X11_Ena
	set_location_assignment PIN_C2 -to ~ALTERA_TMS~
	set_location_assignment PIN_D3 -to ~ALTERA_TCK~
	set_location_assignment PIN_N7 -to ~ALTERA_TDI~
	set_location_assignment PIN_N6 -to ~ALTERA_TDO~
	set_location_assignment PIN_F10 -to ~ALTERA_CONFIG_SEL~
	set_location_assignment PIN_G9 -to ~ALTERA_nCONFIG~
	set_location_assignment PIN_E9 -to ~ALTERA_CRC_ERROR~
	set_location_assignment PIN_F9 -to ~ALTERA_nSTATUS~
	set_location_assignment PIN_F8 -to ~ALTERA_CONF_DONE~
	set_location_assignment PIN_U11 -to EPIOS_28
	set_location_assignment PIN_T11 -to EPIOS_29
	set_location_assignment PIN_AB10 -to EPIOS_30
	set_location_assignment PIN_AC8 -to EPIOS_31
	set_location_assignment PIN_AB9 -to EPIOS_32
	set_location_assignment PIN_AC7 -to EPIOS_33
	set_location_assignment PIN_AD9 -to EPIOS_34
	set_location_assignment PIN_AC9 -to EPIOS_35
	set_location_assignment PIN_AA9 -to EPIOS_ADR[0]
	set_location_assignment PIN_AD5 -to EPIOS_ADR[1]
	set_location_assignment PIN_AA8 -to EPIOS_ADR[2]
	set_location_assignment PIN_AE4 -to EPIOS_ADR[3]
	set_location_assignment PIN_AD6 -to EPIOS_ADR[4]
	set_location_assignment PIN_AF4 -to EPIOS_ADR[5]
	set_location_assignment PIN_AE6 -to EPIOS_ADR[6]
	set_location_assignment PIN_AF5 -to EPIOS_ADR[7]
	set_location_assignment PIN_AA11 -to EPIOS_ADR[8]
	set_location_assignment PIN_AD7 -to EPIOS_ADR[9]
	set_location_assignment PIN_Y11 -to EPIOS_ADR[10]
	set_location_assignment PIN_AD8 -to EPIOS_ADR[11]
	set_location_assignment PIN_B22 -to V_A[3]
	set_location_assignment PIN_D22 -to V_A[4]
	set_location_assignment PIN_A22 -to V_A[5]
	set_location_assignment PIN_C22 -to V_A[6]
	set_location_assignment PIN_F20 -to V_A[7]
	set_location_assignment PIN_D20 -to V_A[8]
	set_location_assignment PIN_E20 -to V_A[9]
	set_location_assignment PIN_D21 -to V_A[10]
	set_location_assignment PIN_G19 -to V_B[5]
	set_location_assignment PIN_C21 -to V_B[6]
	set_location_assignment PIN_F19 -to V_B[7]
	set_location_assignment PIN_G17 -to V_B[10]
	set_location_assignment PIN_B21 -to V_B[27]
	set_location_assignment PIN_F17 -to V_B[29]
	set_location_assignment PIN_A21 -to V_B[31]
	set_location_assignment PIN_E17 -to V_C[1]
	set_location_assignment PIN_E18 -to V_C[3]
	set_location_assignment PIN_E16 -to V_C[4]
	set_location_assignment PIN_D18 -to V_C[5]
	set_location_assignment PIN_G18 -to V_C[6]
	set_location_assignment PIN_D19 -to V_C[7]
	set_location_assignment PIN_F18 -to V_C[8]
	set_location_assignment PIN_C19 -to V_C[9]
	set_location_assignment PIN_C18 -to V_C[10]
	set_location_assignment PIN_A20 -to V_C[17]
	set_location_assignment PIN_AE24 -to V_A[12]
	set_location_assignment PIN_AD24 -to V_A[14]
	set_location_assignment PIN_AB22 -to V_A[13]
	set_location_assignment PIN_AE25 -to V_C[12]
	set_location_assignment PIN_W21 -to V_C[11]
	set_location_assignment PIN_Y19 -to V_C[14]
	set_location_assignment PIN_AB23 -to V_C[15]
	set_location_assignment PIN_AC23 -to V_A[15]
	set_location_assignment PIN_W24 -to V_A[11]
	set_location_assignment PIN_AA26 -to V_A[16]
	set_location_assignment PIN_T21 -to V_A[17]
	set_location_assignment PIN_T23 -to V_B[1]
	set_location_assignment PIN_U25 -to V_B[8]
	set_location_assignment PIN_V24 -to V_B[13]
	set_location_assignment PIN_T20 -to V_B[15]
	set_location_assignment PIN_R23 -to V_B[16]
	set_location_assignment PIN_P17 -to V_B[17]
	set_location_assignment PIN_T26 -to V_B[9]
	set_location_assignment PIN_P21 -to V_C[16]
	set_location_assignment PIN_R26 -to V_B[3]
	set_location_assignment PIN_V21 -to V_B[2]
	set_location_assignment PIN_V22 -to V_B[4]
	set_location_assignment PIN_U21 -to V_B[12]
	set_location_assignment PIN_AD25 -to V_B[11]
	set_location_assignment PIN_Y21 -to V_B[28]
	set_location_assignment PIN_AA22 -to V_B[30]
	set_location_assignment PIN_AA24 -to V_B[32]
	set_location_assignment PIN_L26 -to V_B[23]
	set_location_assignment PIN_AC25 -to V_B[25]
	set_location_assignment PIN_M26 -to V_B[18]
	set_location_assignment PIN_J26 -to V_B[19]
	set_location_assignment PIN_H26 -to V_B[20]
	set_location_assignment PIN_E26 -to V_C[22]
	set_location_assignment PIN_N17 -to V_A[22]
	set_location_assignment PIN_H25 -to V_C[23]
	set_location_assignment PIN_N16 -to V_A[23]
	set_location_assignment PIN_J24 -to V_C[24]
	set_location_assignment PIN_N21 -to V_A[24]
	set_location_assignment PIN_N23 -to V_C[25]
	set_location_assignment PIN_N22 -to V_A[25]
	set_location_assignment PIN_K23 -to V_A[26]
	set_location_assignment PIN_D26 -to V_C[26]
	set_location_assignment PIN_C26 -to V_C[27]
	set_location_assignment PIN_F25 -to V_A[27]
	set_location_assignment PIN_M21 -to V_C[28]
	set_location_assignment PIN_L23 -to V_A[28]
	set_location_assignment PIN_D25 -to V_A[29]
	set_location_assignment PIN_G24 -to V_A[31]
	set_location_assignment PIN_J23 -to V_C[29]
	set_location_assignment PIN_H23 -to V_C[30]
	set_location_assignment PIN_L21 -to V_C[18]
	set_location_assignment PIN_D24 -to V_C[19]
	set_location_assignment PIN_K22 -to V_C[20]
	set_location_assignment PIN_K21 -to V_C[21]
	set_location_assignment PIN_U16 -to V_B[14]
	set_location_assignment PIN_K16 -to V_A[18]
	set_location_assignment PIN_C24 -to V_A[19]
	set_location_assignment PIN_K17 -to V_A[20]
	set_location_assignment PIN_B25 -to V_A[21]
	set_location_assignment PIN_C25 -to V_A[30]
	set_location_assignment PIN_L16 -to V_C[31]
	set_location_assignment PIN_AA21 -to V_C[13]
	set_location_assignment PIN_L25 -to V_B[22]
	set_location_assignment PIN_G26 -to V_B[21]
	set_location_assignment PIN_K25 -to V_B[24]
	set_location_assignment PIN_AA23 -to V_B[26]
	set_location_assignment PIN_F16 -to V_C[2]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to SPARE[5]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to SPARE[1]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to SPARE[2]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to SPARE[3]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_X12_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to EPIOS[10]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to SPARE[4]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to SPARE[6]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to SPARE[7]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to SPARE[8]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to SPARE[9]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to SPARE[10]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to SPARE[11]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to SHOW_CONFIG
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to RUP
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to RDN
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to OE_Test
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to nUSER_LD_LED
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to nFAILSAVE_LD_LED
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to MCU_RESET_FILTER
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to MCU_NMI
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to FS_nCS
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_AC19_Out
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_AC22_AC25_Out
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_AC26_AC27_Out
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_AC28_AC31_Out
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_B18_B24_Out
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_C20_21_Out
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to A_CLK_24MHz
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to A_CLK_50MHz
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_EXT_CLK
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_I_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_I_Out
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_IO1_Out
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_K_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_K_Out
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_L_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_L_Out
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_M_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_M_Out
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_ME_12MHZ
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_ME_BOI
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_ME_BZI
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to A_ME_CDS
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to A_ME_DSC
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_ME_EE
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to A_ME_ESC
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to A_ME_nBOO
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to A_ME_nBZO
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to A_ME_SD
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_ME_SDI
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to A_ME_SDO
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_ME_SS
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to A_ME_TD
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_ME_UDI
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to A_ME_VW
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to A_MIL1_BOI
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to A_MIL1_BZI
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_MIL1_nBOO
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_MIL1_nBZO
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_MIL1_nIN_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_MIL1_OUT_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_N_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_N_Out
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_nAC19_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_nAC22_AC25_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_nAC26_AC27_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_nAC28_AC31_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_nB18_B24_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_nC20_21_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_nIO1_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nLED[22]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nLED[21]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nLED[20]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nLED[19]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nLED[18]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nLED[17]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nLED[16]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nLED[15]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nLED[14]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nLED[13]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nLED[12]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nLED[11]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nLED[10]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nLED[9]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nLED[8]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nLED[7]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nLED[6]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nLED[5]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nLED[4]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nLED[3]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nLED[2]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nLED[1]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nLED[0]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_NLED_EVT_INR
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nLED_EXTCLK
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to A_nMANUAL_RES
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nOPK_DRDY
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nOPK_DRQ
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nOPK_INL
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to A_nSEL_6408
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_nSWITCH_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[45]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[44]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[43]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[42]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[41]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[40]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[39]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[38]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[37]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[36]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[35]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[34]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[33]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[32]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[31]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[30]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[29]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[28]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[27]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[26]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[25]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[24]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[23]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[22]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[21]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[20]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[19]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[18]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[17]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[16]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[15]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[14]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[13]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[12]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[11]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[10]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[9]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[8]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[7]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[6]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[5]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[4]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[3]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[2]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[1]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_P[0]
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to A_SEL_B[3]
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to A_SEL_B[2]
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to A_SEL_B[1]
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to A_SEL_B[0]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_Test[15]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_Test[14]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_Test[13]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_Test[12]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_Test[11]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_Test[10]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_Test[9]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_Test[8]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_Test[7]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_Test[6]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_Test[5]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_Test[4]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_Test[3]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_Test[2]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_Test[1]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to A_Test[0]
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to A_UMIL5V
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to A_UMIL15V
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_WAKE
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_X1_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_X1_Out
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_X2_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_X2_Out
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_X3_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_X3_Out
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_X4_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_X4_Out
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_X5_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_X5_Out
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_X6_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_X6_Out
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_X7_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_X7_Out
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_X8_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_X8_Out
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_X9_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_X9_Out
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_X10_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_X10_Out
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_X11_Out
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_X12_Out
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to EPIOS[15]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to EPIOS[14]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to EPIOS[13]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to EPIOS[12]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to EPIOS[11]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to EPIOS[9]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to EPIOS[8]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to EPIOS[7]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to EPIOS[6]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to EPIOS[5]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to EPIOS[4]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to EPIOS[3]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to EPIOS[2]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to EPIOS[1]
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to EPIOS[0]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to FS_ASDI
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to FS_DATA
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to FS_DCLK
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to IN_NDRQ
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to IN_NINL
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to IN_NRDY
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to LEMO_IN
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to LEMO_OUT
	set_instance_assignment -name IO_STANDARD "3.3-V LVCMOS" -to A_X11_Ena
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to ~ALTERA_CRC_ERROR~
	set_instance_assignment -name IO_STANDARD "3.3 V SCHMITT TRIGGER" -to ~ALTERA_CONFIG_SEL~
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to ~ALTERA_nSTATUS~
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to ~ALTERA_nCONFIG~
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to ~ALTERA_CONF_DONE~
	set_instance_assignment -name WEAK_PULL_UP_RESISTOR ON -to A_WAKE
	set_instance_assignment -name PARTITION_HIERARCHY root_partition -to | -section_id Top

	# Commit assignments
	export_assignments

	# Close project
	if {$need_to_close_project} {
		project_close
	}
}
