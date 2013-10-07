set_location_assignment PIN_G19		-to ddr3_clk[0]
set_location_assignment PIN_G18		-to ddr3_clk_n[0]
set_location_assignment PIN_J17		-to ddr3_ba[2]
set_location_assignment PIN_D20		-to ddr3_ba[1]
set_location_assignment PIN_K18		-to ddr3_ba[0]
set_location_assignment PIN_F24		-to ddr3_cas_n
set_location_assignment PIN_G24		-to ddr3_ras_n
set_location_assignment PIN_K16		-to ddr3_res_n
set_location_assignment PIN_J21		-to ddr3_dq[15]
	set_instance_assignment -name BOARD_MODEL_NEAR_TLINE_LENGTH 2.7008 -to ddr3_dq[15]
set_location_assignment PIN_C18		-to ddr3_dq[14]
	set_instance_assignment -name BOARD_MODEL_NEAR_TLINE_LENGTH 2.6496 -to ddr3_dq[14]
set_location_assignment PIN_K15		-to ddr3_dq[13]
	set_instance_assignment -name BOARD_MODEL_NEAR_TLINE_LENGTH 2.6772 -to ddr3_dq[13]
set_location_assignment PIN_D19		-to ddr3_dq[12]
	set_instance_assignment -name BOARD_MODEL_NEAR_TLINE_LENGTH 2.6181 -to ddr3_dq[12]
set_location_assignment PIN_G17		-to ddr3_dq[11]
	set_instance_assignment -name BOARD_MODEL_NEAR_TLINE_LENGTH 2.6378 -to ddr3_dq[11]
set_location_assignment PIN_D17		-to ddr3_dq[10]
	set_instance_assignment -name BOARD_MODEL_NEAR_TLINE_LENGTH 2.6103 -to ddr3_dq[10]
set_location_assignment PIN_K20		-to ddr3_dq[9]
	set_instance_assignment -name BOARD_MODEL_NEAR_TLINE_LENGTH 2.6063 -to ddr3_dq[9]
set_location_assignment PIN_C17		-to ddr3_dq[8]
	set_instance_assignment -name BOARD_MODEL_NEAR_TLINE_LENGTH 2.6024 -to ddr3_dq[8]
set_location_assignment PIN_B15		-to ddr3_dq[7]
	set_instance_assignment -name BOARD_MODEL_NEAR_TLINE_LENGTH 2.5984 -to ddr3_dq[7]
set_location_assignment PIN_G15		-to ddr3_dq[6]
	set_instance_assignment -name BOARD_MODEL_NEAR_TLINE_LENGTH 2.5984 -to ddr3_dq[6]
set_location_assignment PIN_C16		-to ddr3_dq[5]
	set_instance_assignment -name BOARD_MODEL_NEAR_TLINE_LENGTH 2.6457 -to ddr3_dq[5]
set_location_assignment PIN_H15		-to ddr3_dq[4]
	set_instance_assignment -name BOARD_MODEL_NEAR_TLINE_LENGTH 2.4921 -to ddr3_dq[4]
set_location_assignment PIN_A19		-to ddr3_dq[3]
	set_instance_assignment -name BOARD_MODEL_NEAR_TLINE_LENGTH 2.6496 -to ddr3_dq[3]
set_location_assignment PIN_F15		-to ddr3_dq[2]
	set_instance_assignment -name BOARD_MODEL_NEAR_TLINE_LENGTH 2.6417 -to ddr3_dq[2]
set_location_assignment PIN_A17		-to ddr3_dq[1]
	set_instance_assignment -name BOARD_MODEL_NEAR_TLINE_LENGTH 2.4371 -to ddr3_dq[1]
set_location_assignment PIN_D16		-to ddr3_dq[0]
	set_instance_assignment -name BOARD_MODEL_NEAR_TLINE_LENGTH 2.7047 -to ddr3_dq[0]
	set_instance_assignment -name IO_STANDARD "SSTL-15 CLASS I" -to ddr3_dq
	set_instance_assignment -name MEM_INTERFACE_DELAY_CHAIN_CONFIG FLEXIBLE_TIMING -to ddr3_dq
	set_instance_assignment -name OUTPUT_TERMINATION "SERIES 50 OHM WITHOUT CALIBRATION" -to ddr3_dq
	set_instance_assignment -name BOARD_MODEL_FAR_PULLUP_R 51 -to ddr3_dq
	set_instance_assignment -name BOARD_MODEL_NEAR_SERIES_R SHORT -to ddr3_dq
	set_instance_assignment -name BOARD_MODEL_NEAR_TLINE_L_PER_LENGTH 8.66N -to ddr3_dq
	set_instance_assignment -name BOARD_MODEL_NEAR_TLINE_C_PER_LENGTH 3.47P -to ddr3_dq
#
set_location_assignment PIN_F20		-to ddr3_we_n
set_location_assignment PIN_F17		-to ddr3_dm[1]
set_location_assignment PIN_C19		-to ddr3_dm[0]
set_location_assignment PIN_D21		-to ddr3_dqs_p[1]
set_location_assignment PIN_B16		-to ddr3_dqs_p[0]
set_location_assignment PIN_C21		-to ddr3_dqs_n[1]
set_location_assignment PIN_A16		-to ddr3_dqs_n[0]
set_location_assignment PIN_F23		-to ddr3_addr[12]
set_location_assignment PIN_F21		-to ddr3_addr[11]
set_location_assignment PIN_H19		-to ddr3_addr[10]
set_location_assignment PIN_J16		-to ddr3_addr[9]
set_location_assignment PIN_B19		-to ddr3_addr[8]
set_location_assignment PIN_J18		-to ddr3_addr[7]
set_location_assignment PIN_D18		-to ddr3_addr[6]
set_location_assignment PIN_L21		-to ddr3_addr[5]
set_location_assignment PIN_E24		-to ddr3_addr[4]
set_location_assignment PIN_K21		-to ddr3_addr[3]
set_location_assignment PIN_K19		-to ddr3_addr[2]
set_location_assignment PIN_F22		-to ddr3_addr[1]
set_location_assignment PIN_J19		-to ddr3_addr[0]
	set_instance_assignment -name IO_STANDARD "SSTL-15 CLASS I" -to ddr3_addr
	set_instance_assignment -name CURRENT_STRENGTH_NEW "MAXIMUM CURRENT" -to ddr3_addr
	set_instance_assignment -name BOARD_MODEL_FAR_PULLUP_R 51 -to ddr3_addr
	set_instance_assignment -name BOARD_MODEL_NEAR_SERIES_R SHORT -to ddr3_addr
	set_instance_assignment -name BOARD_MODEL_NEAR_TLINE_L_PER_LENGTH 8.66N -to ddr3_addr
	set_instance_assignment -name BOARD_MODEL_NEAR_TLINE_C_PER_LENGTH 3.47P -to ddr3_addr
set_location_assignment PIN_E21		-to ddr3_cs_n
set_location_assignment PIN_E22		-to ddr3_odt
set_location_assignment PIN_E15		-to ddr3_cke
set_location_assignment PIN_H18		-to vref_ddr3
set_location_assignment PIN_D15		-to clk_125m_local_i
	set_instance_assignment -name IO_STANDARD LVDS -to clk_125m_local_i
set_location_assignment PIN_E27		-to sfp2_ref_clk_i
	set_instance_assignment -name IO_STANDARD LVDS -to sfp2_ref_clk_i
set_location_assignment PIN_D25		-to sfp1_ref_clk_i
	set_instance_assignment -name IO_STANDARD LVDS -to sfp1_ref_clk_i
set_location_assignment PIN_P25		-to F_PLL_3
	set_instance_assignment -name IO_STANDARD LVDS -to F_PLL_3
set_location_assignment PIN_AD25	-to F_PLL_2
	set_instance_assignment -name IO_STANDARD LVDS -to F_PLL_2
set_location_assignment PIN_R27		-to F_PLL_0
	set_instance_assignment -name IO_STANDARD LVDS -to F_PLL_0
set_location_assignment PIN_AE27	-to pcie_refclk_i
	set_instance_assignment -name IO_STANDARD HCSL -to pcie_refclk_i
set_location_assignment PIN_L27		-to a_master_con_rx[4]
set_location_assignment PIN_N27		-to a_master_con_rx[3]
set_location_assignment PIN_U27		-to a_master_con_rx[2]
set_location_assignment PIN_W27		-to a_master_con_rx[1]
set_location_assignment PIN_K25		-to mc_tx[4]
set_location_assignment PIN_M25		-to mc_tx[3]
set_location_assignment PIN_T25		-to mc_tx[2]
set_location_assignment PIN_V25		-to mc_tx[1]
set_location_assignment PIN_C27		-to a_ext_con_tx[2]
set_location_assignment PIN_J27		-to a_ext_con_tx[1]
set_location_assignment PIN_B26		-to ec_rx[2]
set_location_assignment PIN_H25		-to ec_rx[1]
# 
set_location_assignment PIN_E3		-to scu_cb_version[3]
set_location_assignment PIN_D3		-to scu_cb_version[2]
set_location_assignment PIN_G5		-to scu_cb_version[1]
set_location_assignment PIN_G6		-to scu_cb_version[0]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to scu_cb_version
# auxiliary sfp
set_location_assignment PIN_B24		-to sfp1_rxp_i
set_location_assignment PIN_A24		-to "sfp1_rxp_i(n)"
	set_instance_assignment -name IO_STANDARD "1.5-V PCML" -to sfp1_rxp_i
set_location_assignment PIN_D23		-to sfp1_txp_o
set_location_assignment PIN_C23		-to "sfp1_txp_o(n)"
	set_instance_assignment -name IO_STANDARD "1.5-V PCML" -to sfp1_txp_i
# timing sfp
set_location_assignment PIN_G27		-to sfp2_rxp_i
set_location_assignment PIN_G28		-to "sfp2_rxp_i(n)"
	set_instance_assignment -name IO_STANDARD "1.5-V PCML" -to sfp2_rxp_i
set_location_assignment PIN_F25		-to sfp2_txp_o
set_location_assignment PIN_F26		-to "sfp2_txp_o(n)"
	set_instance_assignment -name IO_STANDARD "1.5-V PCML" -to sfp2_txp_i
#
set_location_assignment PIN_AE14	-to clk_20m_vcxo_i
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to clk_20m_vcxo_i
set_location_assignment PIN_AC10	-to nkbd_rst
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to nkbd_rst
set_location_assignment PIN_AB10	-to a20gate
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to a20gate
set_location_assignment PIN_AC8		-to a_rear_out[1]
set_location_assignment PIN_AB8		-to a_rear_out[0]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to a_rear_out
set_location_assignment PIN_AH12	-to a_rear_in[1]
set_location_assignment PIN_AG12	-to a_rear_in[0]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to a_rear_in
# scub address
set_location_assignment PIN_Y13		-to a_a[15]
set_location_assignment PIN_AH10	-to a_a[14] 
set_location_assignment PIN_AH8		-to a_a[13]
set_location_assignment PIN_AH9		-to a_a[12]
set_location_assignment PIN_AH7		-to a_a[11]
set_location_assignment PIN_AG9		-to a_a[10]
set_location_assignment PIN_AC14	-to a_a[9]
set_location_assignment PIN_AH6		-to a_a[8]
set_location_assignment PIN_AB14	-to a_a[7]
set_location_assignment PIN_AG6		-to a_a[6]
set_location_assignment PIN_AH5		-to a_a[5]
set_location_assignment PIN_AF13	-to a_a[4]
set_location_assignment PIN_AH4		-to a_a[3]
set_location_assignment PIN_AE13	-to a_a[2]
set_location_assignment PIN_AC13	-to a_a[1]
set_location_assignment PIN_AF12	-to a_a[0]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to a_a
	set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to a_a
# outputs: scub slave selects, low active
set_location_assignment PIN_AB13	-to a_nsel[12]
set_location_assignment PIN_AE12	-to a_nsel[11]
set_location_assignment PIN_AF10	-to a_nsel[10]
set_location_assignment PIN_AF11	-to a_nsel[9]
set_location_assignment PIN_AE10	-to a_nsel[8]
set_location_assignment PIN_AE11	-to a_nsel[7]
set_location_assignment PIN_W13		-to a_nsel[6]
set_location_assignment PIN_AF9		-to a_nsel[5]
set_location_assignment PIN_W12		-to a_nsel[4]
set_location_assignment PIN_AE9		-to a_nsel[3]
set_location_assignment PIN_AF8		-to a_nsel[2]
set_location_assignment PIN_AF7		-to a_nsel[1]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to a_nsel
	set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to a_nsel
# output: scub data acknowlege, low active
set_location_assignment PIN_AF5		-to a_ndtack
# inputs: scub slave service requests, low active
set_location_assignment PIN_AE7		-to a_nsrq[12]
set_location_assignment PIN_W11		-to a_nsrq[11]
set_location_assignment PIN_AC12	-to a_nsrq[10]
set_location_assignment PIN_AE5		-to a_nsrq[9]
set_location_assignment PIN_AC11	-to a_nsrq[8]
set_location_assignment PIN_AG4		-to a_nsrq[7]
set_location_assignment PIN_AB11	-to a_nsrq[6]
set_location_assignment PIN_AG3		-to a_nsrq[5]
set_location_assignment PIN_AH3		-to a_nsrq[4]
set_location_assignment PIN_AF4		-to a_nsrq[3]
set_location_assignment PIN_AH2		-to a_nsrq[2]
set_location_assignment PIN_AE4		-to a_nsrq[1]
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to a_nsrq
#
set_location_assignment PIN_AD7		-to nextcd0_perst
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to nextcd0_perst 
set_location_assignment PIN_AF3		-to nfpga_res_out
set_location_assignment PIN_AF1		-to onewire_cb
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to onewire_cb 
set_location_assignment PIN_AD9		-to nadr_en
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to nadr_en 
set_location_assignment PIN_AC9		-to nthrmtrip
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to nthrmtrip
set_location_assignment PIN_AC7		-to a_nconfig
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to a_nconfig
set_location_assignment PIN_U24		-to sfp1_tx_fault
	set_instance_assignment -name IO_STANDARD "2.5 V" -to sfp1_tx_fault
set_location_assignment PIN_AH17	-to sfp1_tx_disable_o
	set_instance_assignment -name IO_STANDARD "2.5 V" -to sfp1_tx_disable_o
set_location_assignment PIN_V24		-to sfp1_los
	set_instance_assignment -name IO_STANDARD "2.5 V" -to sfp1_los
set_location_assignment PIN_Y1		-to sfp1_mod2
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to sfp1_mod2
set_location_assignment PIN_T4		-to sfp1_mod1
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to sfp1_mod1
set_location_assignment PIN_AH14	-to sfp1_mod0
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to sfp1_mod0
set_location_assignment PIN_AB19	-to sfp2_tx_fault
	set_instance_assignment -name IO_STANDARD "2.5 V" -to sfp2_tx_fault
set_location_assignment PIN_AH18	-to sfp2_tx_disable_o
	set_instance_assignment -name IO_STANDARD "2.5 V" -to sfp2_tx_disable_o
set_location_assignment PIN_AC22	-to sfp2_los
	set_instance_assignment -name IO_STANDARD "2.5 V" -to sfp2_los
set_location_assignment PIN_AC6		-to sfp2_mod2
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to sfp2_mod2
set_location_assignment PIN_T3		-to sfp2_mod1
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to sfp2_mod1
set_location_assignment PIN_AH13	-to sfp2_mod0
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to sfp2_mod0
set_location_assignment PIN_AH11	-to a_spare[1]
set_location_assignment PIN_Y14		-to a_spare[0]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to a_spare
set_location_assignment PIN_AE8		-to wdt
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to wdt
set_location_assignment PIN_AF2		-to serial_to_cb_o
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to serial_to_cb_o
set_location_assignment PIN_Y12		-to uart_txd_o[1]
set_location_assignment PIN_AF6		-to uart_txd_o[0]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to uart_txd_o
set_location_assignment PIN_AD12	-to uart_rxd_i[1]
set_location_assignment PIN_AE6		-to uart_rxd_i[0]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to uart_rxd_i
set_location_assignment PIN_Y11		-to onewire_ext
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to onewire_ext
set_location_assignment PIN_W10		-to a_sysclock
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to a_sysclock
set_location_assignment PIN_P1		-to dac_din
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to dac_din
set_location_assignment PIN_R4		-to dac_sclk
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to dac_sclk
set_location_assignment PIN_AD6		-to ndac_cs[2]
set_location_assignment PIN_AG1		-to ndac_cs[1]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to ndac_cs
set_location_assignment PIN_AA10	-to adr_to_scub
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to adr_to_scub
set_location_assignment PIN_Y10		-to nres
set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to nres
set_location_assignment PIN_AE15	-to clk_125m_pllref_i
	set_instance_assignment -name IO_STANDARD LVDS -to clk_125m_pllref_i
set_location_assignment PIN_AF18	-to hpla_ch[15]
set_location_assignment PIN_AF19	-to hpla_ch[14]
set_location_assignment PIN_AE18	-to hpla_ch[13]
set_location_assignment PIN_AE19	-to hpla_ch[12]
set_location_assignment PIN_AA16	-to hpla_ch[11]
set_location_assignment PIN_AF17	-to hpla_ch[10]
set_location_assignment PIN_Y16		-to hpla_ch[9]
set_location_assignment PIN_AE17	-to hpla_ch[8]
set_location_assignment PIN_AD15	-to hpla_ch[7]
set_location_assignment PIN_AF16	-to hpla_ch[6]
set_location_assignment PIN_AC15	-to hpla_ch[5]
set_location_assignment PIN_AE16	-to hpla_ch[4]
set_location_assignment PIN_AA19	-to hpla_ch[3]
set_location_assignment PIN_AH19	-to hpla_ch[2]
set_location_assignment PIN_Y18		-to hpla_ch[1]
set_location_assignment PIN_AG19	-to hpla_ch[0]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to hpla_ch
set_location_assignment PIN_AH16	-to hpla_clk
	set_instance_assignment -name IO_STANDARD "2.5 V" -to hpla_clk
set_location_assignment PIN_AG15	-to nuser_pb
	set_instance_assignment -name IO_STANDARD "2.5 V" -to nuser_pb
# EXT_CONN2: 2.5 Volt IOs
set_location_assignment PIN_AC19	-to io_2_5v[15]
set_location_assignment PIN_AB16	-to io_2_5v[14]
set_location_assignment PIN_AC16	-to io_2_5v[13]
set_location_assignment PIN_AB17	-to io_2_5v[12]
set_location_assignment PIN_AC17	-to io_2_5v[11]
set_location_assignment PIN_AC18	-to io_2_5v[10]
set_location_assignment PIN_AD18	-to io_2_5v[9]
set_location_assignment PIN_Y19		-to io_2_5v[8]
set_location_assignment PIN_Y20		-to io_2_5v[7]
set_location_assignment PIN_AC21	-to io_2_5v[6]
set_location_assignment PIN_AD21	-to io_2_5v[5]
set_location_assignment PIN_W21		-to io_2_5v[4]
set_location_assignment PIN_Y22		-to io_2_5v[3]
set_location_assignment PIN_AD22	-to io_2_5v[2]
set_location_assignment PIN_AD23	-to io_2_5v[1]
set_location_assignment PIN_AC23	-to io_2_5v[0]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to io_2_5v
	set_instance_assignment -name CURRENT_STRENGTH_NEW 4MA -to io_2_5v
set_location_assignment PIN_K11		-to clk_fsh
set_location_assignment PIN_G14		-to noe_fsh
set_location_assignment PIN_F14		-to nwe_fsh
# flash address bus
set_location_assignment PIN_C9		-to ad[25]
set_location_assignment PIN_K12		-to ad[24]
set_location_assignment PIN_E13		-to ad[23]
set_location_assignment PIN_G12		-to ad[22]
set_location_assignment PIN_J14		-to ad[21]
set_location_assignment PIN_F10		-to ad[20]
set_location_assignment PIN_G13		-to ad[19]
set_location_assignment PIN_F12		-to ad[18]
set_location_assignment PIN_K13		-to ad[17]
set_location_assignment PIN_K14		-to ad[16]
set_location_assignment PIN_B6		-to ad[15]
set_location_assignment PIN_B9		-to ad[14]
set_location_assignment PIN_A6		-to ad[13]
set_location_assignment PIN_A7		-to ad[12]
set_location_assignment PIN_E12		-to ad[11]
set_location_assignment PIN_A8		-to ad[10]
set_location_assignment PIN_A9		-to ad[9]
set_location_assignment PIN_A10		-to ad[8]
set_location_assignment PIN_C12		-to ad[7]
set_location_assignment PIN_A11		-to ad[6]
set_location_assignment PIN_C13		-to ad[5]
set_location_assignment PIN_D13		-to ad[4]
set_location_assignment PIN_D12		-to ad[3]
set_location_assignment PIN_D11		-to ad[2]
set_location_assignment PIN_C11		-to ad[1]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to ad
# flash address valid, low active
set_location_assignment PIN_A3		-to adv_fsh
	set_instance_assignment -name IO_STANDARD "2.5 V" -to adv_fsh
# flash data bus
set_location_assignment PIN_D6		-to df[15]
set_location_assignment PIN_D10		-to df[14]
set_location_assignment PIN_J12		-to df[13]
set_location_assignment PIN_A4		-to df[12]
set_location_assignment PIN_J11		-to df[11]
set_location_assignment PIN_D9		-to df[10]
set_location_assignment PIN_C7		-to df[9]
set_location_assignment PIN_D8		-to df[8]
set_location_assignment PIN_E7		-to df[7]
set_location_assignment PIN_C5		-to df[6]
set_location_assignment PIN_G11		-to df[5]
set_location_assignment PIN_C6		-to df[4]
set_location_assignment PIN_D7		-to df[3]
set_location_assignment PIN_B12		-to df[2]
set_location_assignment PIN_C8		-to df[1]
set_location_assignment PIN_C10		-to df[0]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to df
# flash chip select, low active
set_location_assignment PIN_F13		-to nce_fsh
	set_instance_assignment -name IO_STANDARD "2.5 V" -to nce_fsh
# flash reset, low active
set_location_assignment PIN_F7		-to nrst_fsh
	set_instance_assignment -name IO_STANDARD "2.5 V" -to nrst_fsh
# input: flash wait
set_location_assignment PIN_G7		-to wait_fsh
	set_instance_assignment -name IO_STANDARD "2.5 V" -to wait_fsh
set_location_assignment PIN_D14		-to a_ext_lvds_clkin
	set_instance_assignment -name IO_STANDARD LVDS -to a_ext_lvds_clkin
set_location_assignment PIN_R5		-to dir_rb3_to_scub
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to dir_rb3_to_scub
set_location_assignment PIN_N3 -to CLK_20_VCXO
set_location_assignment PIN_P5		-to lemo_io[2]
set_location_assignment PIN_V3		-to lemo_io[1]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to lemo_io
set_location_assignment PIN_P4		-to lemo_led[2]
set_location_assignment PIN_L6		-to lemo_led[1]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to lemo_led
set_location_assignment PIN_P3		-to lemo_en_in[2]
set_location_assignment PIN_M5		-to lemo_en_in[1]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to lemo_en_in
set_location_assignment PIN_P2		-to lpc_fpga_clk
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to lpc_fpga_clk
set_location_assignment PIN_V1		-to nlpc_drq0
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to nlpc_drq0
set_location_assignment PIN_P6		-to nlpc_frame
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to nlpc_frame
set_location_assignment PIN_N4		-to lpc_ad[3]
set_location_assignment PIN_V4		-to lpc_ad[2]
set_location_assignment PIN_W1		-to lpc_ad[1]
set_location_assignment PIN_W2		-to lpc_ad[0]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to lpc_ad
	set_instance_assignment -name WEAK_PULL_UP_RESISTOR ON -to lpc_ad
set_location_assignment PIN_W3		-to lpc_serirq
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to lpc_serirq
set_location_assignment PIN_U4		-to npci_pme
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to npci_pme
set_location_assignment PIN_U5		-to a_ntiming_cycle
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to a_ntiming_cycle
set_location_assignment PIN_R3		-to a_nds
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to a_nds
set_location_assignment PIN_U1		-to a_nreset
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to a_nreset
set_location_assignment PIN_R6		-to npci_reset
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to npci_reset
set_location_assignment PIN_U3		-to nsel_ext_data_drv
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to nsel_ext_data_drv
set_location_assignment PIN_N6		-to a_rnw
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to a_rnw
set_location_assignment PIN_M6		-to nsys_reset
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to nsys_reset
set_location_assignment PIN_R1		-to npwrbtn
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to npwrbtn
set_location_assignment PIN_AC2		-to eio[17]
set_location_assignment PIN_AA3		-to eio[16]
set_location_assignment PIN_AD3		-to eio[15]
set_location_assignment PIN_AE3		-to eio[14]
set_location_assignment PIN_AC4		-to eio[13]
set_location_assignment PIN_AD4		-to eio[12]
	set_instance_assignment -name WEAK_PULL_UP_RESISTOR ON -to eio[12]
set_location_assignment PIN_AB5		-to eio[11]
	set_instance_assignment -name WEAK_PULL_UP_RESISTOR ON -to eio[11]
set_location_assignment PIN_AC5		-to eio[10]
set_location_assignment PIN_AA6		-to eio[9]
set_location_assignment PIN_AB6		-to eio[8]
set_location_assignment PIN_AB4		-to eio[7]
set_location_assignment PIN_AB7		-to eio[6]
set_location_assignment PIN_Y6		-to eio[5]
set_location_assignment PIN_Y5		-to eio[4]
set_location_assignment PIN_W6		-to eio[3]
set_location_assignment PIN_W8		-to eio[2]
set_location_assignment PIN_Y9		-to eio[1]
set_location_assignment PIN_V7		-to eio[0]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to eio
# bidir: scub data
set_location_assignment PIN_AC3		-to a_d[15]
set_location_assignment PIN_AA4		-to a_d[14]
set_location_assignment PIN_V6		-to a_d[13]
set_location_assignment PIN_W4		-to a_d[12]
set_location_assignment PIN_U6		-to a_d[11]
set_location_assignment PIN_W5		-to a_d[10]
set_location_assignment PIN_AB2		-to a_d[9]
set_location_assignment PIN_Y3		-to a_d[8]
set_location_assignment PIN_AB3		-to a_d[7]
set_location_assignment PIN_Y4		-to a_d[6]
set_location_assignment PIN_T6		-to a_d[5]
set_location_assignment PIN_AE1		-to a_d[4]
set_location_assignment PIN_T7		-to a_d[3]
set_location_assignment PIN_AD1		-to a_d[2]
set_location_assignment PIN_AC1		-to a_d[1]
set_location_assignment PIN_AA1		-to a_d[0]
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to a_d
set_location_assignment PIN_AB1 	-to a_onewire
	set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to a_onewire
# user leds
set_location_assignment PIN_J4		-to leds_o[4]
set_location_assignment PIN_H3		-to leds_o[3]
set_location_assignment PIN_J5		-to leds_o[2]
set_location_assignment PIN_H4		-to leds_o[1]
	set_instance_assignment -name IO_STANDARD "2.5 V" -to leds_o
set_location_assignment PIN_L7		-to a_ext_lvds_tx[3]
	set_instance_assignment -name IO_STANDARD LVDS -to a_ext_lvds_tx[3]
set_location_assignment PIN_J3		-to a_ext_lvds_tx[2]
	set_instance_assignment -name IO_STANDARD LVDS -to a_ext_lvds_tx[2]
set_location_assignment PIN_J7		-to a_ext_lvds_tx[1]
	set_instance_assignment -name IO_STANDARD LVDS -to a_ext_lvds_tx[1]
set_location_assignment PIN_F1		-to a_ext_lvds_tx[0]
	set_instance_assignment -name IO_STANDARD LVDS -to a_ext_lvds_tx[0]
set_location_assignment PIN_K1		-to a_ext_lvds_clkout
	set_instance_assignment -name IO_STANDARD LVDS -to a_ext_lvds_clkout
set_location_assignment PIN_L3		-to a_ext_lvds_rx[3]
set_instance_assignment -name IO_STANDARD LVDS -to a_ext_lvds_rx[3]
set_location_assignment PIN_M4		-to a_ext_lvds_rx[2]
set_instance_assignment -name IO_STANDARD LVDS -to a_ext_lvds_rx[2]
set_location_assignment PIN_H1		-to a_ext_lvds_rx[1]
set_instance_assignment -name IO_STANDARD LVDS -to a_ext_lvds_rx[1]
set_location_assignment PIN_L4		-to a_ext_lvds_rx[0]
set_instance_assignment -name IO_STANDARD LVDS -to a_ext_lvds_rx[0]
#
set_location_assignment PIN_H6		-to ntiming_sfp_red
	set_instance_assignment -name IO_STANDARD "2.5 V" -to ntiming_sfp_red
set_location_assignment PIN_A2		-to ntiming_sfp_grn
	set_instance_assignment -name IO_STANDARD "2.5 V" -to ntiming_sfp_grn
set_location_assignment PIN_K9		-to naux_sfp_grn
	set_instance_assignment -name IO_STANDARD "2.5 V" -to naux_sfp_grn
set_location_assignment PIN_B1		-to naux_sfp_red
	set_instance_assignment -name IO_STANDARD "2.5 V" -to naux_sfp_red
#
set_location_assignment PIN_AA27		-to pcie_rx_i[3]
set_location_assignment PIN_AC27		-to pcie_rx_i[2]
set_location_assignment PIN_AG25		-to pcie_rx_i[1]
set_location_assignment PIN_AG23		-to pcie_rx_i[0]
	set_instance_assignment -name IO_STANDARD "1.5-V PCML" -to pcie_rx_i
set_location_assignment PIN_Y25			-to pcie_tx_o[3]
set_location_assignment PIN_AB25		-to pcie_tx_o[2]
set_location_assignment PIN_AG27		-to pcie_tx_o[1]
set_location_assignment PIN_AE24		-to pcie_tx_o[0]
	set_instance_assignment -name IO_STANDARD "1.5-V PCML" -to pcie_tx_i


