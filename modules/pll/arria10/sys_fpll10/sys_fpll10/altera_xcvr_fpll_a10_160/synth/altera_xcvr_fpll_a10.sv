// (C) 2001-2016 Altera Corporation. All rights reserved.
// Your use of Altera Corporation's design tools, logic functions and other 
// software and tools, and its AMPP partner logic functions, and any output 
// files any of the foregoing (including device programming or simulation 
// files), and any associated documentation or information are expressly subject 
// to the terms and conditions of the Altera Program License Subscription 
// Agreement, Altera MegaCore Function License Agreement, or other applicable 
// license agreement, including, without limitation, that your use is for the 
// sole purpose of programming logic devices manufactured by Altera and sold by 
// Altera or its authorized distributors.  Please refer to the applicable 
// agreement for further details.


 
//------------------------------------------------------------------
// filename: altera_xcvr_fpll_a10.sv
//
// Description : instantiates avmm and cmu_fpll
//
// Limitation  : Intended for NightFury
//
// Copyright (c) Altera Corporation 1997-2012
// All rights reserved
//-------------------------------------------------------------------
`timescale 1ps/1ps

module altera_xcvr_fpll_a10
#( 
// This has been modified

        parameter [4:0] cmu_fpll_pll_vco_freq_band_0_fix = 5'b00000,
        parameter [2:0] cmu_fpll_pll_vco_freq_band_0_dyn_low_bits = 3'b011,
        parameter [1:0] cmu_fpll_pll_vco_freq_band_0_dyn_high_bits = 2'b00,
        parameter cmu_fpll_pll_vco_freq_band_0_fix_high = "pll_vco_freq_band_0_fix_high_0", 
        parameter [4:0] cmu_fpll_pll_vco_freq_band_1_fix = 5'b00000,
        parameter [4:0] cmu_fpll_pll_vco_freq_band_1_dyn_low_bits = 3'b011,
        parameter [1:0] cmu_fpll_pll_vco_freq_band_1_dyn_high_bits = 2'b00,
        parameter cmu_fpll_pll_vco_freq_band_1_fix_high = "pll_vco_freq_band_1_fix_high_0",
        parameter cmu_fpll_xpm_cmu_fpll_core_cal_vco_count_length = "sel_8b_count",
        parameter cmu_fpll_xpm_cmu_fpll_core_pfd_pulse_width = "pulse_width_setting0",
        parameter cmu_fpll_xpm_cmu_fpll_core_pfd_delay_compensation = "normal_delay",
        parameter cmu_fpll_xpm_cmu_fpll_core_xpm_cpvco_fpll_xpm_chgpmplf_fpll_cp_current_boost = "normal_setting",

	// Virtual parameters
	parameter enable_debug_info = "true",
	parameter cmu_fpll_bw_sel = "low",
	parameter cmu_fpll_compensation_mode = "direct",
	parameter cmu_fpll_datarate = "0 ps",
	parameter cmu_fpll_pll_c0_pllcout_enable = "disable",
	parameter cmu_fpll_pll_c1_pllcout_enable = "disable",
	parameter cmu_fpll_pll_c2_pllcout_enable = "disable",
	parameter cmu_fpll_pll_c3_pllcout_enable = "disable",
	parameter cmu_fpll_fpll_cas_out_enable = "fpll_cas_out_disable",
	parameter cmu_fpll_fpll_hclk_out_enable = "disable",
	parameter cmu_fpll_fpll_iqtxrxclk_out_enable = "disable",
	parameter cmu_fpll_duty_cycle_0 = 50,
	parameter cmu_fpll_duty_cycle_1 = 50,
	parameter cmu_fpll_duty_cycle_2 = 50,
	parameter cmu_fpll_duty_cycle_3 = 50,
	parameter cmu_fpll_hssi_output_clock_frequency = "0 ps",
	parameter cmu_fpll_is_cascaded_pll = "false",
	parameter cmu_fpll_is_otn = "false",
	parameter cmu_fpll_is_sdi = "false",
	parameter cmu_fpll_output_clock_frequency_0 = "0 ps",
	parameter cmu_fpll_output_clock_frequency_1 = "0 ps",
	parameter cmu_fpll_output_clock_frequency_2 = "0 ps",
	parameter cmu_fpll_output_clock_frequency_3 = "0 ps",
	parameter cmu_fpll_phase_shift_0 = "0 ps",
	parameter cmu_fpll_phase_shift_1 = "0 ps",
	parameter cmu_fpll_phase_shift_2 = "0 ps",
	parameter cmu_fpll_phase_shift_3 = "0 ps",
	parameter cmu_fpll_prot_mode = "basic",
	parameter cmu_fpll_reference_clock_frequency = "100.0 MHz",
    parameter cmu_fpll_reference_clock_frequency_scratch = "0 Hz",
	parameter cmu_fpll_vco_frequency = "100.0 MHz",
	parameter cmu_fpll_reconfig_en = "0",
	parameter cmu_fpll_dps_en = "false",
	parameter cmu_fpll_is_pa_core = "false",

	// Added parameters
	parameter cmu_fpll_primary_use = "tx",
	parameter cmu_fpll_feedback = "normal",
	
	// Additional parameters for Rev D
	parameter cmu_fpll_vco_freq_hz = "0",
	parameter cmu_fpll_out_freq_hz = "0",
	parameter cmu_fpll_f_out_c0_hz = "0",
	parameter cmu_fpll_f_out_c1_hz = "0",
	parameter cmu_fpll_f_out_c2_hz = "0",
	parameter cmu_fpll_f_out_c3_hz = "0",

	parameter cmu_fpll_f_min_vco = "0",
	parameter cmu_fpll_f_max_vco = "0",

	// Bitvec parameters
	parameter cmu_fpll_refclk_freq = "000000000000000000000000000000000001",
	parameter cmu_fpll_vco_freq = "000000000000000000000000000000000001",
	parameter cmu_fpll_pfd_freq = "000000000000000000000000000000000001",
	parameter cmu_fpll_out_freq = "000000000000000000000000000000000001",
	parameter cmu_fpll_f_out_c0 = "000000000000000000000000000000000001",
	parameter cmu_fpll_f_out_c1 = "000000000000000000000000000000000001",
	parameter cmu_fpll_f_out_c2 = "000000000000000000000000000000000001",
	parameter cmu_fpll_f_out_c3 = "000000000000000000000000000000000001",
	parameter [4:0] cmu_fpll_l_counter = 5'b00001,
	parameter [7:0] cmu_fpll_m_counter = 8'b00000001,
	parameter [5:0] cmu_fpll_n_counter = 6'b000001,
	parameter [8:0] cmu_fpll_m_counter_c0 = 9'b000000001,
	parameter [8:0] cmu_fpll_m_counter_c1 = 9'b000000001,
	parameter [8:0] cmu_fpll_m_counter_c2 = 9'b000000001,
	parameter [8:0] cmu_fpll_m_counter_c3 = 9'b000000001,
        parameter [7:0] cmu_fpll_set_fpll_input_freq_range = 8'b11111111,
	parameter cmu_fpll_pma_width = 8,
	parameter cmu_fpll_cgb_div = 1,

	
	// twentynm_cmu_fpll_refclk_select parameters
	parameter cmu_fpll_refclk_select_mux_pll_clkin_0_src = "pll_clkin_0_src_vss",
	parameter cmu_fpll_refclk_select_mux_pll_clkin_1_src = "pll_clkin_1_src_vss",
	parameter cmu_fpll_refclk_select_mux_pll_auto_clk_sw_en = "false",
	parameter cmu_fpll_refclk_select_mux_pll_clk_loss_edge = "pll_clk_loss_both_edges",
	parameter cmu_fpll_refclk_select_mux_pll_clk_loss_sw_en = "false",
	parameter cmu_fpll_refclk_select_mux_pll_clk_sw_dly = 0,
	parameter cmu_fpll_refclk_select_mux_pll_manu_clk_sw_en = "false",
	parameter cmu_fpll_refclk_select_mux_pll_sw_refclk_src = "pll_sw_refclk_src_clk_0",
	parameter cmu_fpll_refclk_select_mux_mux0_inclk0_logical_to_physical_mapping = "power_down",
	parameter cmu_fpll_refclk_select_mux_mux0_inclk1_logical_to_physical_mapping = "power_down",
	parameter cmu_fpll_refclk_select_mux_mux0_inclk2_logical_to_physical_mapping = "power_down",
	parameter cmu_fpll_refclk_select_mux_mux0_inclk3_logical_to_physical_mapping = "power_down",
	parameter cmu_fpll_refclk_select_mux_mux0_inclk4_logical_to_physical_mapping = "power_down",
	parameter cmu_fpll_refclk_select_mux_mux1_inclk0_logical_to_physical_mapping = "power_down",
	parameter cmu_fpll_refclk_select_mux_mux1_inclk1_logical_to_physical_mapping = "power_down",
	parameter cmu_fpll_refclk_select_mux_mux1_inclk2_logical_to_physical_mapping = "power_down",
	parameter cmu_fpll_refclk_select_mux_mux1_inclk3_logical_to_physical_mapping = "power_down",
	parameter cmu_fpll_refclk_select_mux_mux1_inclk4_logical_to_physical_mapping = "power_down",
	parameter cmu_fpll_refclk_select_mux_refclk_select0 = "power_down",
	parameter cmu_fpll_refclk_select_mux_refclk_select1 = "power_down",
	parameter cmu_fpll_refclk_select_mux_pll_clk_sel_override = "normal",
	parameter cmu_fpll_refclk_select_mux_pll_clk_sel_override_value = "select_clk0",
    parameter cmu_fpll_refclk_select_mux_silicon_rev = "20nm5es",

	
	// twentynm_cmu_fpll parameters
    parameter cmu_fpll_xpm_cmu_fpll_core_fpll_refclk_source = "normal_refclk",//Valid values: normal_refclk, lc_dedicated_refclk
	parameter cmu_fpll_pll_a_csr_bufin_posedge = "zero", 
	parameter cmu_fpll_pll_atb = "atb_selectdisable",
	parameter cmu_fpll_pll_bw_mode = "low_bw",
	parameter cmu_fpll_pll_c_counter_0 = 1,	
	parameter cmu_fpll_pll_c_counter_0_coarse_dly = "0 ps",
	parameter cmu_fpll_pll_c_counter_0_fine_dly = "0 ps",
	parameter cmu_fpll_pll_c_counter_0_in_src = "c_cnt_in_src_test_clk",
	parameter cmu_fpll_pll_c_counter_0_min_tco_enable = "true",
	parameter cmu_fpll_pll_c_counter_0_ph_mux_prst = 0,
	parameter cmu_fpll_pll_c_counter_0_prst = 1,
	parameter cmu_fpll_pll_c_counter_1 = 1,
	parameter cmu_fpll_pll_c_counter_1_coarse_dly = "0 ps",
	parameter cmu_fpll_pll_c_counter_1_fine_dly = "0 ps",
	parameter cmu_fpll_pll_c_counter_1_in_src = "c_cnt_in_src_test_clk",
	parameter cmu_fpll_pll_c_counter_1_min_tco_enable = "true",
	parameter cmu_fpll_pll_c_counter_1_ph_mux_prst = 0,
	parameter cmu_fpll_pll_c_counter_1_prst = 1,
	parameter cmu_fpll_pll_c_counter_2 = 1,
	parameter cmu_fpll_pll_c_counter_2_coarse_dly = "0 ps",
	parameter cmu_fpll_pll_c_counter_2_fine_dly = "0 ps",
	parameter cmu_fpll_pll_c_counter_2_in_src = "c_cnt_in_src_test_clk",
	parameter cmu_fpll_pll_c_counter_2_min_tco_enable = "true",
	parameter cmu_fpll_pll_c_counter_2_ph_mux_prst = 0,
	parameter cmu_fpll_pll_c_counter_2_prst = 1,
	parameter cmu_fpll_pll_c_counter_3 = 1,
	parameter cmu_fpll_pll_c_counter_3_coarse_dly = "0 ps",
	parameter cmu_fpll_pll_c_counter_3_fine_dly = "0 ps",
	parameter cmu_fpll_pll_c_counter_3_in_src = "c_cnt_in_src_test_clk",
	parameter cmu_fpll_pll_c_counter_3_min_tco_enable = "true",
	parameter cmu_fpll_pll_c_counter_3_ph_mux_prst = 0,
	parameter cmu_fpll_pll_c_counter_3_prst = 1,
	parameter cmu_fpll_pll_cal_status = "false", 	
	parameter cmu_fpll_pll_calibration = "true",
	parameter cmu_fpll_pll_cmp_buf_dly = "0 ps",
	parameter cmu_fpll_pll_cp_compensation = "true",
	parameter cmu_fpll_pll_cp_current_setting = "cp_current_setting0",
	parameter cmu_fpll_pll_cp_testmode = "cp_normal",
	parameter cmu_fpll_pll_cp_lf_3rd_pole_freq = "lf_3rd_pole_setting0",
	parameter cmu_fpll_pll_cp_lf_4th_pole_freq = "lf_4th_pole_setting0",
	parameter cmu_fpll_pll_cp_lf_order = "lf_2nd_order",
	parameter cmu_fpll_pll_ctrl_override_setting = "true",
	parameter cmu_fpll_pll_ctrl_plniotri_override = "false",   
	parameter cmu_fpll_pll_device_variant = "device1",
	parameter cmu_fpll_pll_dprio_base_addr = 256,
	parameter cmu_fpll_pll_dprio_broadcast_en = "true",
	parameter cmu_fpll_pll_dprio_clk_vreg_boost = "clk_fpll_vreg_no_voltage_boost",
	parameter cmu_fpll_pll_dprio_cvp_inter_sel = "true",
	parameter cmu_fpll_pll_dprio_force_inter_sel = "true",
	parameter cmu_fpll_pll_dprio_fpll_vreg_boost = "fpll_vreg_no_voltage_boost",
	parameter cmu_fpll_pll_dprio_fpll_vreg1_boost = "fpll_vreg1_no_voltage_boost",
	parameter cmu_fpll_pll_dprio_power_iso_en = "true",
	parameter cmu_fpll_pll_dprio_status_select = "dprio_normal_status",
	parameter cmu_fpll_pll_dsm_ecn_bypass = "false",
	parameter cmu_fpll_pll_dsm_ecn_test_en = "false",
	parameter cmu_fpll_pll_dsm_fractional_division = "1",
	parameter cmu_fpll_pll_dsm_fractional_value_ready = "pll_k_ready",
	parameter cmu_fpll_pll_dsm_mode = "dsm_mode_integer",
	parameter cmu_fpll_pll_dsm_out_sel = "pll_dsm_disable",
	parameter cmu_fpll_pll_enable = "true",
	parameter cmu_fpll_pll_extra_csr = 0,
	parameter cmu_fpll_pll_fbclk_mux_1 = "pll_fbclk_mux_1_glb",
	parameter cmu_fpll_pll_fbclk_mux_2 = "pll_fbclk_mux_2_m_cnt",
	parameter cmu_fpll_initial_settings = "true",
	parameter cmu_fpll_pll_iqclk_mux_sel	= "power_down", 
	parameter cmu_fpll_pll_l_counter	= 1,
	parameter cmu_fpll_pll_l_counter_bypass = "false",
	parameter cmu_fpll_pll_l_counter_enable = "true",
	parameter cmu_fpll_pll_lf_resistance = "lf_res_setting0",
	parameter cmu_fpll_pll_lf_cbig = "lf_cbig_setting4",
	parameter cmu_fpll_pll_lf_ripplecap = "lf_ripple_enabled_0",
	parameter cmu_fpll_pll_lock_fltr_cfg = 1,
	parameter cmu_fpll_pll_lock_fltr_test = "pll_lock_fltr_nrm",
	parameter cmu_fpll_pll_lpf_rstn_value = "lpf_normal",
	parameter cmu_fpll_pll_m_counter	= 1, 
	parameter cmu_fpll_pll_m_counter_coarse_dly = "0 ps",
	parameter cmu_fpll_pll_m_counter_fine_dly = "0 ps",
	parameter cmu_fpll_pll_m_counter_in_src = "m_cnt_in_src_test_clk",
	parameter cmu_fpll_pll_m_counter_min_tco_enable = "true",
	parameter cmu_fpll_pll_m_counter_ph_mux_prst	= 0,
	parameter cmu_fpll_pll_m_counter_prst = 1,
	parameter cmu_fpll_pll_n_counter	= 1,
	parameter cmu_fpll_pll_n_counter_coarse_dly = "0 ps",
	parameter cmu_fpll_pll_n_counter_fine_dly = "0 ps",
	parameter cmu_fpll_pll_nreset_invert = "false", 
	parameter cmu_fpll_pll_op_mode = "false",
	parameter cmu_fpll_pll_optimal = "true",
	parameter cmu_fpll_pll_overrange_underrange_voltage_coarse_sel = "vreg_setting_coarse1",
	parameter cmu_fpll_pll_overrange_voltage = "over_setting3",
	parameter cmu_fpll_power_mode = "low_power",
	parameter cmu_fpll_pll_powerdown_mode = "false",
	parameter cmu_fpll_pll_ppm_clk0_src = "ppm_clk0_vss",
	parameter cmu_fpll_pll_ppm_clk1_src = "ppm_clk1_vss",
	parameter cmu_fpll_pll_ref_buf_dly = "0 ps",
	parameter cmu_fpll_pll_rstn_override = "false",
	parameter cmu_fpll_pll_self_reset = "false",
	parameter cmu_fpll_pll_cmu_rstn_value = "true",
	parameter cmu_fpll_silicon_rev = "20nm5es",
	parameter cmu_fpll_pm_speed_grade = "e2",
	parameter cmu_fpll_pll_sup_mode = "user_mode",
	parameter cmu_fpll_pll_tclk_mux_en = "false",
	parameter cmu_fpll_pll_tclk_sel = "pll_tclk_m_src",
	parameter cmu_fpll_pll_test_enable = "false",
	parameter cmu_fpll_pll_underrange_voltage = "under_setting3",
	parameter cmu_fpll_pll_unlock_fltr_cfg = 0,
	parameter cmu_fpll_pll_vccr_pd_en = "true",
	parameter cmu_fpll_pll_vco_freq_band_0 = "pll_freq_band0",
	parameter cmu_fpll_pll_vco_freq_band_1 = "pll_freq_band0_1",
	parameter cmu_fpll_pll_vco_freq_band = "pll_freq_band0",
	parameter cmu_fpll_pll_vco_ph0_en = "false",
	parameter cmu_fpll_pll_vco_ph0_value = "pll_vco_ph0_vss",
	parameter cmu_fpll_pll_vco_ph1_en = "false",
	parameter cmu_fpll_pll_vco_ph1_value = "pll_vco_ph1_vss",
	parameter cmu_fpll_pll_vco_ph2_en = "false",
	parameter cmu_fpll_pll_vco_ph2_value = "pll_vco_ph2_vss",
	parameter cmu_fpll_pll_vco_ph3_en = "false",
	parameter cmu_fpll_pll_vco_ph3_value = "pll_vco_ph3_vss",
	parameter cmu_fpll_pll_vreg0_output = "vccdreg_nominal",
	parameter cmu_fpll_pll_vreg1_output = "vccdreg_nominal",
	
	parameter enable_mcgb = 0,                                                   // \RANGE (0) 1
    parameter enable_mcgb_debug_ports_parameters = 0,                            // \RANGE (0) 1
    parameter enable_analog_resets        = 0,      // (0,1)
                                                    // 0 - Disable pll_powerdown and mcgb_rst reset input connections. Still allows soft register override
                                                    // 1 - Enable pll_powerdown and mcgb_rst reset input connections
	parameter hip_cal_en = "disable",                                            // Indicates whether HIP is enabled or not. Valid values: disable, enable
    parameter hssi_pma_cgb_master_silicon_rev = "20nm5es",                         		// \RANGE (20nm5es) "revb" "revc"
    parameter hssi_pma_cgb_master_prot_mode  = "basic_tx",                      		// \RANGE "unused" (basic_tx) "basic_kr_tx" "pcie_gen1_tx" "pcie_gen2_tx" "pcie_gen3_tx" "pcie_gen4_tx" "cei_tx" "qpi_tx" "cpri_tx" "fc_tx" "srio_tx" "gpon_tx" "sdi_tx" "sata_tx" "xaui_tx" "obsai_tx" "gige_tx" "higig_tx" "sonet_tx" "sfp_tx" "xfp_tx" "sfi_tx"
    parameter hssi_pma_cgb_master_cgb_enable_iqtxrxclk = "disable_iqtxrxclk",   		// \OPEN in atom default is enable in _hw.tcl default is disable // \RANGE disable_iqtxrxclk (enable_iqtxrxclk) 
    parameter hssi_pma_cgb_master_x1_div_m_sel = "divbypass",                   		// \RANGE (divbypass) divby2 divby4 divby8
    parameter hssi_pma_cgb_master_ser_mode = "eight_bit",                       		// \RANGE (eight_bit) ten_bit sixteen_bit twenty_bit thirty_two_bit forty_bit sixty_four_bit
    parameter hssi_pma_cgb_master_datarate = "0 Mbps",
    parameter hssi_pma_cgb_master_cgb_power_down                     = "normal_cgb",                  // \RANGE normal_cgb (power_down_cgb)                           
    parameter hssi_pma_cgb_master_master_cgb_clock_control           = "master_cgb_no_dft_control",   // \RANGE (master_cgb_no_dft_control) master_cgb_dft_control_high master_cgb_dft_control_low  
    parameter hssi_pma_cgb_master_observe_cgb_clocks                 = "observe_nothing",             // \RANGE (observe_nothing) observe_x1mux_out   
    parameter hssi_pma_cgb_master_op_mode                            = "enabled",                     // \RANGE (enabled) pwr_down             
    parameter hssi_pma_cgb_master_tx_ucontrol_reset_pcie             = "pcscorehip_controls_mcgb",    // \RANGE (pcscorehip_controls_mcgb) cgb_reset tx_pcie_gen1 tx_pcie_gen2 tx_pcie_gen3 tx_pcie_gen4  
    parameter hssi_pma_cgb_master_vccdreg_output                     = "vccdreg_nominal",             // \RANGE (vccdreg_nominal) vccdreg_pos_setting0 vccdreg_pos_setting1 vccdreg_pos_setting2 vccdreg_pos_setting3 vccdreg_pos_setting4 vccdreg_pos_setting5 vccdreg_pos_setting6 vccdreg_pos_setting7 vccdreg_pos_setting8 vccdreg_pos_setting9 vccdreg_pos_setting10 vccdreg_pos_setting11 vccdreg_pos_setting12 vccdreg_pos_setting13 vccdreg_pos_setting14 vccdreg_pos_setting15 reserved1 reserved2 vccdreg_neg_setting0 vccdreg_neg_setting1 vccdreg_neg_setting2 vccdreg_neg_setting3 reserved3 reserved4 reserved5 reserved6 reserved7 reserved8 reserved9 reserved10 reserved11   
    parameter hssi_pma_cgb_master_input_select                       = "lcpll_top",                   // \RANGE lcpll_bot lcpll_top fpll_bot fpll_top (unused)      
    parameter hssi_pma_cgb_master_input_select_gen3                  = "unused",                      // \RANGE lcpll_bot lcpll_top fpll_bot fpll_top (unused) 
    parameter hssi_pma_cgb_master_inclk0_logical_to_physical_mapping = "unused",                      // \RANGE lcpll_bot lcpll_top fpll_bot fpll_top (unused) 
    parameter hssi_pma_cgb_master_inclk1_logical_to_physical_mapping = "unused",                      // \RANGE lcpll_bot lcpll_top fpll_bot fpll_top (unused) 
    parameter hssi_pma_cgb_master_inclk2_logical_to_physical_mapping = "unused",                      // \RANGE lcpll_bot lcpll_top fpll_bot fpll_top (unused) 
    parameter hssi_pma_cgb_master_inclk3_logical_to_physical_mapping = "unused",                      // \RANGE lcpll_bot lcpll_top fpll_bot fpll_top (unused) 
    parameter hssi_pma_cgb_master_bonding_reset_enable               = "allow_bonding_reset",         // \RANGE disallow_bonding_reset (allow_bonding_reset) 
    parameter hssi_pma_cgb_master_optimal                            = "true",                        // \RANGE (true) false   
 	parameter hssi_pma_cgb_master_pcie_gen3_bitwidth                 = "pciegen3_wide" ,		      // \RANGE (pciegen3_wide) pciegen3_narrow parameter	powerdown_mode = "powerup" ,		//Valid values: powerup , powerdown 
	parameter hssi_pma_cgb_master_powerdown_mode                     = "powerup" ,		              // \RANGE (powerup) powerdown 
	parameter hssi_pma_cgb_master_sup_mode                           = "user_mode" ,		          // \RANGE (user_mode) engineering_mode 
	parameter hssi_pma_cgb_master_initial_settings                   = "true", 		              // \RANGE (false) true 
	
	

    // instantiate paramters for embedded debug
    parameter enable_pll_reconfig         = 0,
    parameter rcfg_jtag_enable            = 0,
    parameter dbg_embedded_debug_enable   = 0,
    parameter dbg_capability_reg_enable   = 0,
    parameter dbg_user_identifier         = 0,
    parameter dbg_stat_soft_logic_enable  = 0,
    parameter dbg_ctrl_soft_logic_enable  = 0,
    parameter cmu_fpll_calibration_en = "disable",
    parameter rcfg_separate_avmm_busy = 0,          // (0,1)
                                                    // 0 - AVMM busy is reflected on the waitrequest
                                                    // 1 - AVMM busy must be read from a soft CSR


	parameter SIZE_AVMM_RDDATA_BUS = 32,
	parameter SIZE_AVMM_WRDATA_BUS = 32
)
(
	// general usage for core and HSSI
	input pll_refclk0,
    input pll_refclk1,          
    input pll_refclk2,          
    input pll_refclk3,          
    input pll_refclk4,          
	input pll_powerdown,
	output tx_serial_clk,
	output outclk0,
	output outclk1,
	output outclk2,
	output outclk3,
	output pll_pcie_clk,
	output pll_locked,

	// dynamic phase shift
	input phase_reset,
	input phase_en,
	input updn,
	input [3:0] cntsel,
	output phase_done,

	// clock switchover
	input extswitch,
	output [1:0] clkbad,
	output activeclk,

	// reconfig for PLL
    input reconfig_clk0,
    input reconfig_write0,
    input reconfig_read0,
	input reconfig_reset0,
    input [9:0] reconfig_address0,                     
    input [SIZE_AVMM_WRDATA_BUS-1:0] reconfig_writedata0,
    output [SIZE_AVMM_RDDATA_BUS-1:0] reconfig_readdata0,
    output reconfig_waitrequest0,
    output avmm_busy0,
    output pll_cal_busy,
    output hip_cal_done,

    // reconfig for Master CGB
    input reconfig_clk1,
    input reconfig_write1,
    input reconfig_read1,
	input reconfig_reset1,
    input [9:0] reconfig_address1,              
    input  [SIZE_AVMM_WRDATA_BUS-1:0] reconfig_writedata1,
    output [SIZE_AVMM_RDDATA_BUS-1:0] reconfig_readdata1,
    output reconfig_waitrequest1,    
    output avmm_busy1,
    output mcgb_cal_busy,   
    output mcgb_hip_cal_done,  

	// Central Clock Divider
    input mcgb_aux_clk0,                      
    input mcgb_aux_clk1,
    input mcgb_aux_clk2,
    input mcgb_rst,
	input [1:0] pcie_sw,
    output [5:0] tx_bonding_clocks,               
    output mcgb_serial_clk,                      
    output [1:0] pcie_sw_done, 	
	
	// cascading
	output fpll_to_fpll_cascade_clk,
	output hssi_pll_cascade_clk,
    input atx_to_fpll_cascade_clk,

	// debug ports
	output clk_src,
	output clklow,
	output fref
);

    localparam  MAX_CONVERSION_SIZE_ALT_XCVR_FPLL_A10 = 128;
    localparam  MAX_STRING_CHARS_ALT_XCVR_FPLL_A10  = 64;
    localparam  lcl_enable_analog_resets = 
    `ifdef ALTERA_RESERVED_QIS
      `ifdef ALTERA_XCVR_A10_ENABLE_ANALOG_RESETS
        1;  // MACRO override for quartus synthesis. Connect resets
      `else
        enable_analog_resets; // parameter option for synthesis
      `endif // ALTERA_XCVR_A10_ENABLE_ANALOG_RESETS
    `else
      1; // not synthesis. Connect resets
    `endif  // (NOT ALTERA_RESERVED_QIS)

   localparam  lcl_disable_reset_connected_to_cal_busy = 
    `ifdef ALTERA_RESERVED_QIS
      `ifdef ALTERA_XCVR_A10_DISABLE_RESET_CONNECTED_TO_CAL_BUSY
        1;  // MACRO override for quartus synthesis. Disconnect pll_cal_busy connection to reset.
      `else
        0; // Default. Connect pll_cal_busy to reset.
      `endif // ALTERA_XCVR_A10_DISABLE_RESET_CONNECTED_TO_CAL_BUSY
    `else
      0; // not synthesis. Connect cal_busy and user reset to PLL reset
    `endif  // (NOT ALTERA_RESERVED_QIS)


    function automatic [MAX_CONVERSION_SIZE_ALT_XCVR_FPLL_A10-1:0] str_2_bin_altera_xcvr_fpll_pll_a10;
      input [MAX_STRING_CHARS_ALT_XCVR_FPLL_A10*8-1:0] instring;

      integer this_char;
      integer i;
      begin
        // Initialize accumulator
        str_2_bin_altera_xcvr_fpll_pll_a10 = {MAX_CONVERSION_SIZE_ALT_XCVR_FPLL_A10{1'b0}};
        for(i=MAX_STRING_CHARS_ALT_XCVR_FPLL_A10-1;i>=0;i=i-1) begin
          this_char = instring[i*8+:8];
          // Add value of this digit
          if(this_char >= 48 && this_char <= 57)
            str_2_bin_altera_xcvr_fpll_pll_a10 = (str_2_bin_altera_xcvr_fpll_pll_a10 * 10) + (this_char - 48);
        end
      end
    endfunction

    // String to binary conversions
    localparam  [127:0] temp_cmu_fpll_pll_dsm_fractional_division  = str_2_bin_altera_xcvr_fpll_pll_a10(cmu_fpll_pll_dsm_fractional_division);
    localparam  [31:0] cmu_fpll_pll_dsm_fractional_division_bin = temp_cmu_fpll_pll_dsm_fractional_division[31:0];

    localparam SIZE_CGB_BONDING_CLK = 6;
    localparam avmm_interfaces = ((enable_mcgb==1) && (enable_mcgb_debug_ports_parameters==1)) ? 2 : 1;
    localparam RCFG_ADDR_BITS = 10;
    localparam refiqclk_size = 12;
    localparam refclk_cnt = 4;
	
	localparam  lcl_adme_assgn_map = {" assignments {device_revision ",cmu_fpll_silicon_rev,"}"};

    // upper 24 bits are not used, but should not be left at X
    // assign reconfig_readdata0[SIZE_AVMM_RDDATA_BUS-1:8] =  0;
    assign reconfig_readdata1[SIZE_AVMM_RDDATA_BUS-1:8] =  0;

    //-----------------------------------
    // reconfig AVMM to pll internal wires  
    // interface #0 to PLL, interface #1 to CGB 
    wire  [avmm_interfaces-1    :0] pll_avmm_clk;
    wire  [avmm_interfaces-1    :0] pll_avmm_rstn;
    wire  [avmm_interfaces*8-1  :0] pll_avmm_writedata;
    wire  [avmm_interfaces*9-1  :0] pll_avmm_address;
    wire  [avmm_interfaces-1    :0] pll_avmm_write;
    wire  [avmm_interfaces-1    :0] pll_avmm_read;
    wire  [avmm_interfaces*8-1  :0] pll_avmmreaddata_cmu_fpll;                 
    wire  [avmm_interfaces*8-1  :0] pll_avmmreaddata_cmu_fpll_refclk_select;       
    wire  [avmm_interfaces*8-1  :0] pll_avmmreaddata_cgb_master;          
    wire  [avmm_interfaces-1    :0] pll_blockselect_cmu_fpll;               
    wire  [avmm_interfaces-1    :0] pll_blockselect_cmu_fpll_refclk_select;  
    wire  [avmm_interfaces-1    :0] pll_blockselect_cgb_master; 

    //-----------------------------------
    // reconfigAVMM to top wrapper wires  
    // interface #0 to PLL, interface #1 to CGB 
    wire  [avmm_interfaces-1    :0] reconfig_clk;
    wire  [avmm_interfaces-1    :0] reconfig_reset;
    wire  [avmm_interfaces*8-1  :0] reconfig_writedata;
    wire  [avmm_interfaces*10-1 :0] reconfig_address;
    wire  [avmm_interfaces-1    :0] reconfig_write;
    wire  [avmm_interfaces-1    :0] reconfig_read;
    wire  [avmm_interfaces*8-1  :0] reconfig_readdata;
    wire  [avmm_interfaces-1    :0] reconfig_waitrequest;
    wire  [avmm_interfaces-1    :0] avmm_busy;
    wire  [avmm_interfaces-1    :0] pld_cal_done;
    wire  [avmm_interfaces-1    :0] hip_cal_done_w;

   // AVMM reconfiguration signals for the hardware
   wire [avmm_interfaces-1:0]     avmm_write;
   wire [avmm_interfaces-1:0]     avmm_read;
   wire [avmm_interfaces-1:0]     avmm_waitrequest;
   wire [avmm_interfaces*8-1:0]   avmm_readdata;

   // AVMM reconfiguration signals for embedded debug
   wire [avmm_interfaces*8-1:0]   debug_writedata;
   wire [avmm_interfaces-1:0]     debug_clk;
   wire [avmm_interfaces-1:0]     debug_reset;
   wire [avmm_interfaces*10-1:0]  debug_address; 
   wire [avmm_interfaces-1:0]     debug_write;
   wire [avmm_interfaces-1:0]     debug_read;
   wire [avmm_interfaces-1:0]     debug_busy;
   wire [avmm_interfaces-1:0]     debug_waitrequest;
   wire [avmm_interfaces*8-1:0]   debug_readdata;

   // Wires for control signals from the embedded debug
   wire                           pll_powerdown_int;
   // wire for gating pll_powerdown with pll_cal_busy before connecting to the atom input
   wire                           pll_powerdown_int_gated;
   // Wire for FPLL DPS
   wire final_phase_en;
   wire final_phase_reset;
   wire pre_phase_done;

    //-----------------------------------
    // reconfigAVMM to cgb atom 
    wire         avmm_clk_mcgb;
    wire         avmm_rstn_mcgb;
    wire   [7:0] avmm_writedata_mcgb;
    wire   [8:0] avmm_address_mcgb;
    wire         avmm_write_mcgb;
    wire         avmm_read_mcgb;
    wire   [7:0] avmmreaddata_mcgb;
    wire         blockselect_mcgb;

    // Wires for disconnecting pll_powerdown and mcgb_rst
    // When the parameter "lcl_enable_analog_resets" is set to 0, these wires will be driven to 0.
    // When the parameter "lcl_enable_analog_resets" is set to 1, these wires will be connected
    // to the pll_powerdown and mcgb_rst inputs.
    wire pll_powerdown_input;
    wire mcgb_rst_input;

     // Pulling in the rule for calibration attributes from PCS channel RBC
    localparam  arbiter_ctrl      = (cmu_fpll_calibration_en == "enable") ? "uc"                : "pld";
    localparam  cal_done          = (cmu_fpll_calibration_en == "enable") ? "cal_done_deassert" : "cal_done_assert";
    localparam  hip_cal_en_fnl    = (cmu_fpll_calibration_en == "enable") ? hip_cal_en          : "disable";
    localparam  avmm_busy_en      = rcfg_separate_avmm_busy ? "enable" : "disable";

    // Analog reset masking. We always connect analog resets for simulation. 
    // For synthesis it is parameter controlled or MACRO overridden
    generate
    if(lcl_enable_analog_resets == 1) begin
      assign pll_powerdown_input = pll_powerdown;
      assign mcgb_rst_input      = mcgb_rst;
    end else begin
      assign pll_powerdown_input = 1'b0;
      assign mcgb_rst_input      = 1'b0;
    end
    endgenerate

    // By default, pll_cal_busy needs to be connected back to pll_powerdown in HSSI and Core mode. 
    //    -> OR pll_cal_busy (active high) from hardware with pll_powerdown_int (active high) from user or embedded CSR.
    // If the MACRO is enabled, then do not connect pll_cal_busy to pll_powerdown for a fallback option.
    // For synthesis it is MACRO overridden
    generate
    if((lcl_disable_reset_connected_to_cal_busy == 1) || (hip_cal_en_fnl == "enable")) begin
      assign pll_powerdown_int_gated = pll_powerdown_int;
    end else begin
      assign pll_powerdown_int_gated = pll_powerdown_int | pll_cal_busy;
    end
    endgenerate

    assign pll_avmmreaddata_cgb_master[7:0] = { 8 {1'b0} };                           // \NOTE only [15:8] is used, hence [7:0] is tied-off to '0'
    assign pll_blockselect_cgb_master[0:0] = {1'b0};  

    assign reconfig_clk[0] = debug_clk;
    assign reconfig_reset[0] = debug_reset;
    assign reconfig_writedata[7:0] = debug_writedata[7:0];
    assign reconfig_address[9:0] = debug_address;
    assign reconfig_write[0] = avmm_write;
    assign reconfig_read[0] = avmm_read;
    assign avmm_readdata[7:0]=reconfig_readdata[7:0];
    assign avmm_waitrequest = reconfig_waitrequest[0];
    assign avmm_busy0 = avmm_busy[0];
    assign hip_cal_done = hip_cal_done_w[0];
	assign mcgb_serial_clk = tx_bonding_clocks[SIZE_CGB_BONDING_CLK-1];

    generate
       if (avmm_interfaces==2) begin
          assign pll_avmmreaddata_cmu_fpll[avmm_interfaces*8-1:8] = { 8 {1'b0} };             // \NOTE only [7:0] is used, hence [15:8] is tied-off to '0'
          assign pll_avmmreaddata_cmu_fpll_refclk_select[avmm_interfaces*8-1:8] = { 8 {1'b0} };   // \NOTE only [7:0] is used, hence [15:8] is tied-off to '0'

          assign pll_blockselect_cmu_fpll[avmm_interfaces-1:1] = {1'b0};                      // \NOTE only [0:0] is used, hence [1:1] is tied-off to '0'
          assign pll_blockselect_cmu_fpll_refclk_select[avmm_interfaces-1:1] = {1'b0};            // \NOTE only [0:0] is used, hence [1:1] is tied-off to '0'

          assign avmm_clk_mcgb                     = pll_avmm_clk[1];
          assign avmm_rstn_mcgb                    = pll_avmm_rstn[1];
          assign avmm_writedata_mcgb               = pll_avmm_writedata[15:8];
          assign avmm_address_mcgb                 = pll_avmm_address[17:9];
          assign avmm_write_mcgb                   = pll_avmm_write[1];
          assign avmm_read_mcgb                    = pll_avmm_read[1];
          assign pll_avmmreaddata_cgb_master[15:8] = avmmreaddata_mcgb;
          assign pll_blockselect_cgb_master[1]     = blockselect_mcgb;
       end
    endgenerate

   //---
   assign mcgb_cal_busy = 1'b0;
   assign pll_cal_busy = ~pld_cal_done[0];

   generate
      if (avmm_interfaces==2) begin
         assign reconfig_clk[1] = reconfig_clk1;
         assign reconfig_reset[1] = reconfig_reset1;
         assign reconfig_writedata[15:8] = reconfig_writedata1[7:0];
         assign reconfig_address[19:10] = reconfig_address1;
         assign reconfig_write[1] = reconfig_write1;
         assign reconfig_read[1] = reconfig_read1;
         assign reconfig_readdata1[7:0]=reconfig_readdata[15:8];
         assign reconfig_waitrequest1 = reconfig_waitrequest[1];
         assign avmm_busy1 = avmm_busy[1];
         //assign mcgb_cal_busy = ~pld_cal_done[1];
         assign mcgb_hip_cal_done = hip_cal_done_w[1];
      end
      else begin
         assign reconfig_readdata1[7:0] = 0;
         assign reconfig_waitrequest1 = 0;
         assign avmm_busy1 = 1'b0;
         assign mcgb_hip_cal_done = 0;
      end
   endgenerate

	// feedback wire
	wire refclk_sel_outclk_wire;
	wire fbclk_wire;
	wire iqtxrxclk_wire;
	wire pll_extfb_wire;
	wire pll_pa_iqtxrxclk_wire;
	
	// clock switchover wire
	wire clk0_bad_wire;
	wire clk1_bad_wire;
	
	// cascade out on IQTXRXCLK
	assign hssi_pll_cascade_clk = iqtxrxclk_wire;
	
	//-----------------------------------

    alt_xcvr_native_avmm_nf #(
        .CHANNELS         	(1),
        .ADDR_BITS        	(RCFG_ADDR_BITS),
        .ADME_SLAVE_MAP   	("altera_xcvr_fpll_a10"),
		.ADME_ASSGN_MAP 	(lcl_adme_assgn_map),
        .RECONFIG_SHARED  	(0),
        .JTAG_ENABLED     	(enable_pll_reconfig && rcfg_jtag_enable)
      ) altera_xcvr_pll_avmm_nf_inst (
      // Reconfig interface ports
      .reconfig_clk         (reconfig_clk0        ),
      .reconfig_reset       (reconfig_reset0      ),
      .reconfig_write       (reconfig_write0      ),
      .reconfig_read        (reconfig_read0       ),
      .reconfig_address     (reconfig_address0    ),
      .reconfig_writedata   (reconfig_writedata0  ),
      .reconfig_readdata    (reconfig_readdata0   ),
      .reconfig_waitrequest (reconfig_waitrequest0),

      // AVMM ports to transceiver
      .avmm_clk             (debug_clk            ),
      .avmm_reset           (debug_reset          ),
      .avmm_write           (debug_write          ),
      .avmm_read            (debug_read           ),
      .avmm_address         (debug_address        ),
      .avmm_writedata       (debug_writedata      ),
      .avmm_readdata        (debug_readdata       ),
      .avmm_waitrequest     (debug_waitrequest    )
    );

    //-----------------------------------
    // Instantiate the embedded debug
    generate if(dbg_embedded_debug_enable == 1) begin: en_embedded_debug

       // AVMM reconfiguration signals for embedded debug
       wire [avmm_interfaces-1:0]     csr_write;
       wire [avmm_interfaces-1:0]     csr_read;
       wire [avmm_interfaces-1:0]     csr_waitrequest;
       wire [avmm_interfaces*8-1:0]   csr_readdata;

        // avmm arbitration for soft csr and pll
        assign csr_read           = (debug_address[RCFG_ADDR_BITS-1]) ? debug_read         : 1'b0;
        assign csr_write          = (debug_address[RCFG_ADDR_BITS-1]) ? debug_write        : 1'b0;
        assign avmm_read          = (debug_address[RCFG_ADDR_BITS-1]) ? 1'b0               : debug_read        ;
        assign avmm_write         = (debug_address[RCFG_ADDR_BITS-1]) ? 1'b0               : debug_write       ;
        assign debug_waitrequest  = (debug_address[RCFG_ADDR_BITS-1]) ? csr_waitrequest    : avmm_waitrequest  ;
        assign debug_readdata     = (debug_address[RCFG_ADDR_BITS-1]) ? csr_readdata       : avmm_readdata     ;

        alt_xcvr_pll_embedded_debug #(
          .dbg_capability_reg_enable      ( dbg_capability_reg_enable ),
          .dbg_user_identifier            ( dbg_user_identifier ),
          .dbg_stat_soft_logic_enable     ( dbg_stat_soft_logic_enable ),
          .dbg_ctrl_soft_logic_enable     ( dbg_ctrl_soft_logic_enable ),
          .en_master_cgb                  ( enable_mcgb )
        ) pll_embedded_debug (
          // avmm signals
        /*input         */  .avmm_clk                              (debug_clk),
        /*input         */  .avmm_reset                            (debug_reset),
        /*input  [8:0]  */  .avmm_address                          (debug_address[8:0]),
        /*input  [7:0]  */  .avmm_writedata                        (debug_writedata),
        /*input         */  .avmm_write                            (csr_write),
        /*input         */  .avmm_read                             (csr_read),
        /*output [7:0]  */  .avmm_readdata                         (csr_readdata),
        /*output        */  .avmm_waitrequest                      (csr_waitrequest),

          // input signals from the core
        /*input         */  .in_pll_powerdown                      (pll_powerdown_input),
        /*input         */  .in_pll_locked                         (pll_locked),
        /*input         */  .in_pll_cal_busy                       (pll_cal_busy),
                            .in_avmm_busy                          (avmm_busy0),

          // output signals to the ip
        /*output        */  .out_pll_powerdown                     (pll_powerdown_int)
        );
      end else begin: dis_embedded_debug
        assign pll_powerdown_int  = pll_powerdown_input;
        assign avmm_write         = debug_write;
        assign avmm_read          = debug_read;
        assign debug_waitrequest  = avmm_waitrequest;
        assign debug_readdata     = avmm_readdata;
      end
    endgenerate
    //----------------------------------

 
    // instantiation of twentynm_xcvr_avmm 
    twentynm_xcvr_avmm
    #(
     .rcfg_enable(enable_pll_reconfig),              
     .avmm_interfaces(avmm_interfaces),
	   .arbiter_ctrl(arbiter_ctrl),
	   .calibration_en(cmu_fpll_calibration_en),
           .avmm_busy_en(avmm_busy_en),
     .cal_done(cal_done),
	   .hip_cal_en(hip_cal_en_fnl)
    ) xcvr_avmm_inst (
       .avmm_clk(        {reconfig_clk           } ),
       .avmm_reset(      {reconfig_reset         } ),
       .avmm_writedata(  {reconfig_writedata     } ),
       .avmm_address(    {reconfig_address[8:0]  } ),
       .avmm_write(      {reconfig_write         } ),
       .avmm_read(       {reconfig_read          } ),
       .avmm_readdata(   {reconfig_readdata      } ),
       .avmm_waitrequest({reconfig_waitrequest   } ),
       .avmm_busy(       {avmm_busy              } ),
       .pld_cal_done(    {pld_cal_done           } ),
       .hip_cal_done(    {hip_cal_done_w         } ),

       .chnl_pll_avmm_clk(pll_avmm_clk),
       .chnl_pll_avmm_rstn(pll_avmm_rstn),
       .chnl_pll_avmm_writedata(pll_avmm_writedata),
       .chnl_pll_avmm_address(pll_avmm_address),
       .chnl_pll_avmm_write(pll_avmm_write),
       .chnl_pll_avmm_read(pll_avmm_read),

       .pll_avmmreaddata_cmu_fpll(pll_avmmreaddata_cmu_fpll),                          
       .pll_avmmreaddata_cmu_fpll_refclk_select(pll_avmmreaddata_cmu_fpll_refclk_select), 
       .pll_avmmreaddata_cgb_master(pll_avmmreaddata_cgb_master),                                        
       .pll_blockselect_cmu_fpll(pll_blockselect_cmu_fpll),            
       .pll_blockselect_cmu_fpll_refclk_select(pll_blockselect_cmu_fpll_refclk_select),  
       .pll_blockselect_cgb_master(pll_blockselect_cgb_master),  

       //-----------------------------------
       // IRRELEVANT PORTS
       .pma_avmmreaddata_tx_ser                   ( {avmm_interfaces{8'b0}} ),
       .pma_avmmreaddata_tx_cgb                   ( {avmm_interfaces{8'b0}} ),
       .pma_avmmreaddata_tx_buf                   ( {avmm_interfaces{8'b0}} ),
       .pma_avmmreaddata_rx_deser                 ( {avmm_interfaces{8'b0}} ),
       .pma_avmmreaddata_rx_buf                   ( {avmm_interfaces{8'b0}} ),
       .pma_avmmreaddata_rx_sd                    ( {avmm_interfaces{8'b0}} ),
       .pma_avmmreaddata_rx_odi                   ( {avmm_interfaces{8'b0}} ),
       .pma_avmmreaddata_rx_dfe                   ( {avmm_interfaces{8'b0}} ),
       .pma_avmmreaddata_cdr_pll                  ( {avmm_interfaces{8'b0}} ),
       .pma_avmmreaddata_cdr_refclk_select        ( {avmm_interfaces{8'b0}} ),
       .pma_avmmreaddata_pma_adapt                ( {avmm_interfaces{8'b0}} ),
       .pma_blockselect_tx_ser                    ( {avmm_interfaces{1'b0}} ),
       .pma_blockselect_tx_cgb                    ( {avmm_interfaces{1'b0}} ),
       .pma_blockselect_tx_buf                    ( {avmm_interfaces{1'b0}} ),
       .pma_blockselect_rx_deser                  ( {avmm_interfaces{1'b0}} ),
       .pma_blockselect_rx_buf                    ( {avmm_interfaces{1'b0}} ),
       .pma_blockselect_rx_sd                     ( {avmm_interfaces{1'b0}} ),
       .pma_blockselect_rx_odi                    ( {avmm_interfaces{1'b0}} ),
       .pma_blockselect_rx_dfe                    ( {avmm_interfaces{1'b0}} ),
       .pma_blockselect_cdr_pll                   ( {avmm_interfaces{1'b0}} ),
       .pma_blockselect_cdr_refclk_select         ( {avmm_interfaces{1'b0}} ),
       .pma_blockselect_pma_adapt                 ( {avmm_interfaces{1'b0}} ),
       .pcs_avmmreaddata_8g_rx_pcs                ( {avmm_interfaces{8'b0}} ),
       .pcs_avmmreaddata_pipe_gen1_2              ( {avmm_interfaces{8'b0}} ),
       .pcs_avmmreaddata_8g_tx_pcs                ( {avmm_interfaces{8'b0}} ),
       .pcs_avmmreaddata_10g_rx_pcs               ( {avmm_interfaces{8'b0}} ),
       .pcs_avmmreaddata_10g_tx_pcs               ( {avmm_interfaces{8'b0}} ),
       .pcs_avmmreaddata_gen3_rx_pcs              ( {avmm_interfaces{8'b0}} ),
       .pcs_avmmreaddata_pipe_gen3                ( {avmm_interfaces{8'b0}} ),
       .pcs_avmmreaddata_gen3_tx_pcs              ( {avmm_interfaces{8'b0}} ),
       .pcs_avmmreaddata_krfec_rx_pcs             ( {avmm_interfaces{8'b0}} ),
       .pcs_avmmreaddata_krfec_tx_pcs             ( {avmm_interfaces{8'b0}} ),
       .pcs_avmmreaddata_fifo_rx_pcs              ( {avmm_interfaces{8'b0}} ),
       .pcs_avmmreaddata_fifo_tx_pcs              ( {avmm_interfaces{8'b0}} ),
       .pcs_avmmreaddata_rx_pcs_pld_if            ( {avmm_interfaces{8'b0}} ),
       .pcs_avmmreaddata_com_pcs_pld_if           ( {avmm_interfaces{8'b0}} ),
       .pcs_avmmreaddata_tx_pcs_pld_if            ( {avmm_interfaces{8'b0}} ),
       .pcs_avmmreaddata_rx_pcs_pma_if            ( {avmm_interfaces{8'b0}} ),
       .pcs_avmmreaddata_com_pcs_pma_if           ( {avmm_interfaces{8'b0}} ),
       .pcs_avmmreaddata_tx_pcs_pma_if            ( {avmm_interfaces{8'b0}} ),
       .pcs_blockselect_8g_rx_pcs                 ( {avmm_interfaces{1'b0}} ),
       .pcs_blockselect_pipe_gen1_2               ( {avmm_interfaces{1'b0}} ),
       .pcs_blockselect_8g_tx_pcs                 ( {avmm_interfaces{1'b0}} ),
       .pcs_blockselect_10g_rx_pcs                ( {avmm_interfaces{1'b0}} ),
       .pcs_blockselect_10g_tx_pcs                ( {avmm_interfaces{1'b0}} ),
       .pcs_blockselect_gen3_rx_pcs               ( {avmm_interfaces{1'b0}} ),
       .pcs_blockselect_pipe_gen3                 ( {avmm_interfaces{1'b0}} ),
       .pcs_blockselect_gen3_tx_pcs               ( {avmm_interfaces{1'b0}} ),
       .pcs_blockselect_krfec_rx_pcs              ( {avmm_interfaces{1'b0}} ),
       .pcs_blockselect_krfec_tx_pcs              ( {avmm_interfaces{1'b0}} ),
       .pcs_blockselect_fifo_rx_pcs               ( {avmm_interfaces{1'b0}} ),
       .pcs_blockselect_fifo_tx_pcs               ( {avmm_interfaces{1'b0}} ),
       .pcs_blockselect_rx_pcs_pld_if             ( {avmm_interfaces{1'b0}} ),
       .pcs_blockselect_com_pcs_pld_if            ( {avmm_interfaces{1'b0}} ),
       .pcs_blockselect_tx_pcs_pld_if             ( {avmm_interfaces{1'b0}} ),
       .pcs_blockselect_rx_pcs_pma_if             ( {avmm_interfaces{1'b0}} ),
       .pcs_blockselect_com_pcs_pma_if            ( {avmm_interfaces{1'b0}} ),
       .pcs_blockselect_tx_pcs_pma_if             ( {avmm_interfaces{1'b0}} ),
       .pll_avmmreaddata_lc_pll                   ( {avmm_interfaces{8'b0}} ),
       .pll_avmmreaddata_lc_refclk_select         ( {avmm_interfaces{8'b0}} ),
       .pll_blockselect_lc_pll                    ( {avmm_interfaces{1'b0}} ),
       .pll_blockselect_lc_refclk_select          ( {avmm_interfaces{1'b0}} )
    );   
	
	//-----------------------------------
    // instantiation of twentynm_cmu_fpll_refclk_select 
	twentynm_cmu_fpll_refclk_select
	#(
		.pll_clkin_0_src(cmu_fpll_refclk_select_mux_pll_clkin_0_src),
		.pll_clkin_1_src(cmu_fpll_refclk_select_mux_pll_clkin_1_src),
		.pll_auto_clk_sw_en(cmu_fpll_refclk_select_mux_pll_auto_clk_sw_en),
		.pll_clk_loss_edge(cmu_fpll_refclk_select_mux_pll_clk_loss_edge),
		.pll_clk_loss_sw_en(cmu_fpll_refclk_select_mux_pll_clk_loss_sw_en),
		.pll_clk_sw_dly(cmu_fpll_refclk_select_mux_pll_clk_sw_dly),
		.pll_manu_clk_sw_en(cmu_fpll_refclk_select_mux_pll_manu_clk_sw_en),
		.pll_sw_refclk_src(cmu_fpll_refclk_select_mux_pll_sw_refclk_src),
		.mux0_inclk0_logical_to_physical_mapping(cmu_fpll_refclk_select_mux_mux0_inclk0_logical_to_physical_mapping),
		.mux0_inclk1_logical_to_physical_mapping(cmu_fpll_refclk_select_mux_mux0_inclk1_logical_to_physical_mapping),
		.mux0_inclk2_logical_to_physical_mapping(cmu_fpll_refclk_select_mux_mux0_inclk2_logical_to_physical_mapping),
		.mux0_inclk3_logical_to_physical_mapping(cmu_fpll_refclk_select_mux_mux0_inclk3_logical_to_physical_mapping),
		.mux0_inclk4_logical_to_physical_mapping(cmu_fpll_refclk_select_mux_mux0_inclk4_logical_to_physical_mapping),
		.mux1_inclk0_logical_to_physical_mapping(cmu_fpll_refclk_select_mux_mux1_inclk0_logical_to_physical_mapping),
		.mux1_inclk1_logical_to_physical_mapping(cmu_fpll_refclk_select_mux_mux1_inclk1_logical_to_physical_mapping),
		.mux1_inclk2_logical_to_physical_mapping(cmu_fpll_refclk_select_mux_mux1_inclk2_logical_to_physical_mapping),
		.mux1_inclk3_logical_to_physical_mapping(cmu_fpll_refclk_select_mux_mux1_inclk3_logical_to_physical_mapping),
		.mux1_inclk4_logical_to_physical_mapping(cmu_fpll_refclk_select_mux_mux1_inclk4_logical_to_physical_mapping),
		.refclk_select0(cmu_fpll_refclk_select_mux_refclk_select0),
		.refclk_select1(cmu_fpll_refclk_select_mux_refclk_select1),
		.pll_clk_sel_override(cmu_fpll_refclk_select_mux_pll_clk_sel_override),
		.pll_clk_sel_override_value(cmu_fpll_refclk_select_mux_pll_clk_sel_override_value),
		.silicon_rev(cmu_fpll_silicon_rev)
	) fpll_refclk_select_inst
	(
		// Input Ports
		.avmmaddress(pll_avmm_address[8:0]),
		// .avmmclk((cmu_fpll_reconfig_en == "0" && cmu_fpll_dps_en == "false") ? 1'b0 : pll_avmm_clk[0]),
		.avmmclk(pll_avmm_clk[0]),
		.avmmread(pll_avmm_read[0]),
		.avmmrstn(pll_avmm_rstn[0]),
		.avmmwrite(pll_avmm_write[0]),
		.avmmwritedata(pll_avmm_writedata[7:0]),
		.core_refclk(),
		.extswitch(extswitch),
		.iqtxrxclk(),
		.pll_cascade_in(),
		.ref_iqclk({{(refiqclk_size-refclk_cnt){1'b0}}, {pll_refclk4, pll_refclk3, pll_refclk2, pll_refclk1}}),
		.refclk(pll_refclk0),
		.tx_rx_core_refclk(),
		
		// Output Ports
		.avmmreaddata(pll_avmmreaddata_cmu_fpll_refclk_select[7:0]),
		.blockselect(pll_blockselect_cmu_fpll_refclk_select[0]),
		.clk_src(clk_src),
		.clk0bad(clk0_bad_wire),
		.clk1bad(clk1_bad_wire),
		.extswitch_buf(),
		.outclk(refclk_sel_outclk_wire),
		.pllclksel(activeclk)
	);

	//-----------------------------------
	// Instantiate pulse-width control module
	dps_pulse_ctrl phase_en_pulse(
		.clk(reconfig_clk),  // input clock to the HSSI AVMM
		.rst(phase_reset),
		.user_phase_en(phase_en), 
		.phase_en(final_phase_en)      
	);

        //-----------------------------------
        // Instantiate dps reset generation
        dps_reset_gen dps_reset_gen_1 (
		.clk(reconfig_clk),                  // input clock to the HSSI AVMM
                .phase_reset_in(phase_reset),        // active high reset input
                .phase_en_in(final_phase_en),        // phase_en input
                .phase_done_in(~pre_phase_done),     // phase_done input - this thing is actually active LOW		       
                .phase_reset_out(final_phase_reset), // active high reset output
                .phase_done_out(phase_done)          // phase_done output                 
	);

    wire fbclk_in;
    wire iqtxrxclk;
    assign fbclk_in = (cmu_fpll_compensation_mode == "normal") ? fbclk_wire : 1'b0;
    assign iqtxrxclk = (cmu_fpll_compensation_mode == "iqtxrxclk") ? iqtxrxclk_wire : (cmu_fpll_compensation_mode == "fpll_bonding") ? pll_extfb_wire : (cmu_fpll_is_pa_core == "true") ? pll_pa_iqtxrxclk_wire : 1'b0;
    assign pll_pa_iqtxrxclk_wire = (cmu_fpll_is_pa_core == "true") ? outclk1 : 1'b0; // Connect clk1 to this hardwired feedback connection if we are in phase alignment mode

	//-----------------------------------
	// instantiate of twentynm_cmu_fpll 
	twentynm_cmu_fpll
	#(
                .pll_vco_freq_band_0_fix          (cmu_fpll_pll_vco_freq_band_0_fix),
                .pll_vco_freq_band_0_dyn_low_bits          (cmu_fpll_pll_vco_freq_band_0_dyn_low_bits),
                .pll_vco_freq_band_0_dyn_high_bits          (cmu_fpll_pll_vco_freq_band_0_dyn_high_bits),
                .pll_vco_freq_band_0_fix_high          (cmu_fpll_pll_vco_freq_band_0_fix_high),
                .pll_vco_freq_band_1_fix          (cmu_fpll_pll_vco_freq_band_1_fix),
                .pll_vco_freq_band_1_dyn_low_bits          (cmu_fpll_pll_vco_freq_band_1_dyn_low_bits),
                .pll_vco_freq_band_1_dyn_high_bits          (cmu_fpll_pll_vco_freq_band_1_dyn_high_bits),
                .pll_vco_freq_band_1_fix_high          (cmu_fpll_pll_vco_freq_band_1_fix_high),
		.xpm_cmu_fpll_core_cal_vco_count_length  (cmu_fpll_xpm_cmu_fpll_core_cal_vco_count_length),
                .xpm_cmu_fpll_core_pfd_pulse_width          (cmu_fpll_xpm_cmu_fpll_core_pfd_pulse_width),
                .xpm_cmu_fpll_core_pfd_delay_compensation   (cmu_fpll_xpm_cmu_fpll_core_pfd_delay_compensation),
                .set_fpll_input_freq_range          (cmu_fpll_set_fpll_input_freq_range),
                .xpm_cmu_fpll_core_xpm_cpvco_fpll_xpm_chgpmplf_fpll_cp_current_boost          (cmu_fpll_xpm_cmu_fpll_core_xpm_cpvco_fpll_xpm_chgpmplf_fpll_cp_current_boost),

		// virtual parameters
		.is_pa_core(cmu_fpll_is_pa_core),
		.bw_sel(cmu_fpll_bw_sel),
		.compensation_mode(cmu_fpll_compensation_mode),
		.datarate(cmu_fpll_datarate),
		.duty_cycle_0(cmu_fpll_duty_cycle_0),
		.duty_cycle_1(cmu_fpll_duty_cycle_1),
		.duty_cycle_2(cmu_fpll_duty_cycle_2),
		.duty_cycle_3(cmu_fpll_duty_cycle_3),
		.feedback(cmu_fpll_feedback),  		
		.hssi_output_clock_frequency(cmu_fpll_hssi_output_clock_frequency),
		.initial_settings(cmu_fpll_initial_settings),
		.is_cascaded_pll(cmu_fpll_is_cascaded_pll),
		.is_otn(cmu_fpll_is_otn),
		.is_sdi(cmu_fpll_is_sdi),
		.output_clock_frequency_0(cmu_fpll_output_clock_frequency_0),
		.output_clock_frequency_1(cmu_fpll_output_clock_frequency_1),
		.output_clock_frequency_2(cmu_fpll_output_clock_frequency_2),
		.output_clock_frequency_3(cmu_fpll_output_clock_frequency_3),
		.phase_shift_0(cmu_fpll_phase_shift_0),
		.phase_shift_1(cmu_fpll_phase_shift_1),
		.phase_shift_2(cmu_fpll_phase_shift_2),
		.phase_shift_3(cmu_fpll_phase_shift_3),
		.primary_use(cmu_fpll_primary_use),		
		.prot_mode(cmu_fpll_prot_mode),
		.reference_clock_frequency(cmu_fpll_reference_clock_frequency),
		.vco_frequency(cmu_fpll_vco_frequency),
		
		// Bitvec parameters
		.reference_clock_frequency_scratch(cmu_fpll_reference_clock_frequency),
		.vco_freq(cmu_fpll_vco_freq),
		.pfd_freq(cmu_fpll_pfd_freq),
		.out_freq(cmu_fpll_out_freq),
		.f_out_c0(cmu_fpll_f_out_c0),
		.f_out_c1(cmu_fpll_f_out_c1),
		.f_out_c2(cmu_fpll_f_out_c2),
		.f_out_c3(cmu_fpll_f_out_c3),
		.vco_freq_hz(cmu_fpll_vco_freq_hz),
		.out_freq_hz(cmu_fpll_out_freq_hz),
		.f_out_c0_hz(cmu_fpll_f_out_c0_hz),
		.f_out_c1_hz(cmu_fpll_f_out_c1_hz),
		.f_out_c2_hz(cmu_fpll_f_out_c2_hz),
		.f_out_c3_hz(cmu_fpll_f_out_c3_hz),
//		.f_min_vco(cmu_fpll_f_min_vco),
//		.f_max_vco(cmu_fpll_f_max_vco),
		.l_counter(cmu_fpll_l_counter),
		.n_counter(cmu_fpll_n_counter),
		.m_counter(cmu_fpll_m_counter), 
		.m_counter_c0(cmu_fpll_m_counter_c0),
		.m_counter_c1(cmu_fpll_m_counter_c1),
		.m_counter_c2(cmu_fpll_m_counter_c2),
		.m_counter_c3(cmu_fpll_m_counter_c3),
		.cgb_div(cmu_fpll_cgb_div),
		.pma_width(cmu_fpll_pma_width),
	
		// twentynm_cmu_fpll parameters	
		.pll_atb(cmu_fpll_pll_atb),
		.pll_bw_mode(cmu_fpll_pll_bw_mode),
		.pll_c0_pllcout_enable(cmu_fpll_pll_c0_pllcout_enable),
		.pll_c1_pllcout_enable(cmu_fpll_pll_c1_pllcout_enable),
		.pll_c2_pllcout_enable(cmu_fpll_pll_c2_pllcout_enable),
		.pll_c3_pllcout_enable(cmu_fpll_pll_c3_pllcout_enable),
		.fpll_cas_out_enable(cmu_fpll_fpll_cas_out_enable),
		.fpll_hclk_out_enable(cmu_fpll_fpll_hclk_out_enable),
		.fpll_iqtxrxclk_out_enable(cmu_fpll_fpll_iqtxrxclk_out_enable),
		.pll_c_counter_0(cmu_fpll_pll_c_counter_0),
		.pll_c_counter_0_coarse_dly(cmu_fpll_pll_c_counter_0_coarse_dly),
		.pll_c_counter_0_fine_dly(cmu_fpll_pll_c_counter_0_fine_dly),
		.pll_c_counter_0_in_src(cmu_fpll_pll_c_counter_0_in_src),
		.pll_c_counter_0_min_tco_enable(cmu_fpll_pll_c_counter_0_min_tco_enable),
		.pll_c_counter_0_ph_mux_prst(cmu_fpll_pll_c_counter_0_ph_mux_prst),
		.pll_c_counter_0_prst(cmu_fpll_pll_c_counter_0_prst),
		.pll_c_counter_1(cmu_fpll_pll_c_counter_1),
		.pll_c_counter_1_coarse_dly(cmu_fpll_pll_c_counter_1_coarse_dly),
		.pll_c_counter_1_fine_dly(cmu_fpll_pll_c_counter_1_fine_dly),
		.pll_c_counter_1_in_src(cmu_fpll_pll_c_counter_1_in_src),
		.pll_c_counter_1_min_tco_enable(cmu_fpll_pll_c_counter_1_min_tco_enable),
		.pll_c_counter_1_ph_mux_prst(cmu_fpll_pll_c_counter_1_ph_mux_prst),
		.pll_c_counter_1_prst(cmu_fpll_pll_c_counter_1_prst),
		.pll_c_counter_2(cmu_fpll_pll_c_counter_2),
		.pll_c_counter_2_coarse_dly(cmu_fpll_pll_c_counter_2_coarse_dly),
		.pll_c_counter_2_fine_dly(cmu_fpll_pll_c_counter_2_fine_dly),
		.pll_c_counter_2_in_src(cmu_fpll_pll_c_counter_2_in_src),
		.pll_c_counter_2_min_tco_enable(cmu_fpll_pll_c_counter_2_min_tco_enable),		
		.pll_c_counter_2_ph_mux_prst(cmu_fpll_pll_c_counter_2_ph_mux_prst),
		.pll_c_counter_2_prst(cmu_fpll_pll_c_counter_2_prst),
		.pll_c_counter_3(cmu_fpll_pll_c_counter_3),
		.pll_c_counter_3_coarse_dly(cmu_fpll_pll_c_counter_3_coarse_dly),
		.pll_c_counter_3_fine_dly(cmu_fpll_pll_c_counter_3_fine_dly),
		.pll_c_counter_3_in_src(cmu_fpll_pll_c_counter_3_in_src),
		.pll_c_counter_3_min_tco_enable(cmu_fpll_pll_c_counter_3_min_tco_enable),		
		.pll_c_counter_3_ph_mux_prst(cmu_fpll_pll_c_counter_3_ph_mux_prst),
		.pll_c_counter_3_prst(cmu_fpll_pll_c_counter_3_prst),
		.pll_cal_status(cmu_fpll_pll_cal_status),
		.pll_calibration(cmu_fpll_pll_calibration),
		.pll_cmp_buf_dly(cmu_fpll_pll_cmp_buf_dly),
		.pll_cp_compensation(cmu_fpll_pll_cp_compensation),
		.pll_cp_current_setting(cmu_fpll_pll_cp_current_setting),
		.pll_cp_testmode(cmu_fpll_pll_cp_testmode),
		.pll_cp_lf_3rd_pole_freq(cmu_fpll_pll_cp_lf_3rd_pole_freq),
		.pll_cp_lf_order(cmu_fpll_pll_cp_lf_order),
		.pll_ctrl_override_setting(cmu_fpll_pll_ctrl_override_setting),
		.pll_ctrl_plniotri_override(cmu_fpll_pll_ctrl_plniotri_override),
		.pll_device_variant(cmu_fpll_pll_device_variant),
		.pll_dprio_base_addr(cmu_fpll_pll_dprio_base_addr),
		.pll_dprio_broadcast_en(cmu_fpll_pll_dprio_broadcast_en),
		.pll_dprio_clk_vreg_boost(cmu_fpll_pll_dprio_clk_vreg_boost),
		.pll_dprio_cvp_inter_sel(cmu_fpll_pll_dprio_cvp_inter_sel),
		.pll_dprio_force_inter_sel(cmu_fpll_pll_dprio_force_inter_sel),
		.pll_dprio_fpll_vreg_boost(cmu_fpll_pll_dprio_fpll_vreg_boost),
		.pll_dprio_fpll_vreg1_boost(cmu_fpll_pll_dprio_fpll_vreg1_boost),
		.pll_dprio_power_iso_en(cmu_fpll_pll_dprio_power_iso_en),
		.pll_dprio_status_select(cmu_fpll_pll_dprio_status_select),
		.pll_dsm_ecn_bypass(cmu_fpll_pll_dsm_ecn_bypass),
		.pll_dsm_ecn_test_en(cmu_fpll_pll_dsm_ecn_test_en),
		.pll_dsm_fractional_division(cmu_fpll_pll_dsm_fractional_division_bin),
		.pll_dsm_fractional_value_ready(cmu_fpll_pll_dsm_fractional_value_ready),
		.pll_dsm_mode(cmu_fpll_pll_dsm_mode),
		.pll_dsm_out_sel(cmu_fpll_pll_dsm_out_sel),
		.pll_enable(cmu_fpll_pll_enable),
		.pll_extra_csr(cmu_fpll_pll_extra_csr),
		.pll_fbclk_mux_1(cmu_fpll_pll_fbclk_mux_1),
		.pll_fbclk_mux_2(cmu_fpll_pll_fbclk_mux_2),
		.pll_iqclk_mux_sel(cmu_fpll_pll_iqclk_mux_sel),
		.pll_l_counter(cmu_fpll_pll_l_counter),
		.pll_l_counter_bypass(cmu_fpll_pll_l_counter_bypass),
		.pll_l_counter_enable(cmu_fpll_pll_l_counter_enable),
		.pll_lf_resistance(cmu_fpll_pll_lf_resistance),
		.pll_lf_cbig(cmu_fpll_pll_lf_cbig),
		.pll_lf_ripplecap(cmu_fpll_pll_lf_ripplecap),
		.pll_lock_fltr_cfg(cmu_fpll_pll_lock_fltr_cfg),
		.pll_lock_fltr_test(cmu_fpll_pll_lock_fltr_test),
		.pll_lpf_rstn_value(cmu_fpll_pll_lpf_rstn_value),
		.pll_m_counter(cmu_fpll_pll_m_counter),
		.pll_m_counter_coarse_dly(cmu_fpll_pll_m_counter_coarse_dly),
		.pll_m_counter_fine_dly(cmu_fpll_pll_m_counter_fine_dly),
		.pll_m_counter_in_src(cmu_fpll_pll_m_counter_in_src),
		.pll_m_counter_min_tco_enable(cmu_fpll_pll_m_counter_min_tco_enable),
		.pll_m_counter_ph_mux_prst(cmu_fpll_pll_m_counter_ph_mux_prst),
		.pll_m_counter_prst(cmu_fpll_pll_m_counter_prst),
		.pll_n_counter(cmu_fpll_pll_n_counter),
		.pll_n_counter_coarse_dly(cmu_fpll_pll_n_counter_coarse_dly),
		.pll_n_counter_fine_dly(cmu_fpll_pll_n_counter_fine_dly),
		.pll_nreset_invert(cmu_fpll_pll_nreset_invert),
		.pll_op_mode(cmu_fpll_pll_op_mode),
		.pll_optimal(cmu_fpll_pll_optimal),
		.power_mode(cmu_fpll_power_mode),
		.pll_powerdown_mode(cmu_fpll_pll_powerdown_mode),
		.pll_ppm_clk0_src(cmu_fpll_pll_ppm_clk0_src),
		.pll_ppm_clk1_src(cmu_fpll_pll_ppm_clk1_src),
		.pll_ref_buf_dly(cmu_fpll_pll_ref_buf_dly),
		.pll_rstn_override(cmu_fpll_pll_rstn_override),
		.pll_self_reset(cmu_fpll_pll_self_reset),
		.pll_cmu_rstn_value(cmu_fpll_pll_cmu_rstn_value),
		.silicon_rev(cmu_fpll_silicon_rev),
		.pm_speed_grade(cmu_fpll_pm_speed_grade),
		.pll_vco_freq_band_0(cmu_fpll_pll_vco_freq_band_0),
		.pll_vco_freq_band_1(cmu_fpll_pll_vco_freq_band_1),
		.pll_sup_mode(cmu_fpll_pll_sup_mode),
		.pll_tclk_mux_en(cmu_fpll_pll_tclk_mux_en),
		.pll_tclk_sel(cmu_fpll_pll_tclk_sel),
		.pll_test_enable(cmu_fpll_pll_test_enable),
		.pll_unlock_fltr_cfg(cmu_fpll_pll_unlock_fltr_cfg),
		.pll_vccr_pd_en(cmu_fpll_pll_vccr_pd_en),
		.pll_vco_ph0_en(cmu_fpll_pll_vco_ph0_en),
		.pll_vco_ph0_value(cmu_fpll_pll_vco_ph0_value),
		.pll_vco_ph1_en(cmu_fpll_pll_vco_ph1_en),
		.pll_vco_ph1_value(cmu_fpll_pll_vco_ph1_value),
		.pll_vco_ph2_en(cmu_fpll_pll_vco_ph2_en),
		.pll_vco_ph2_value(cmu_fpll_pll_vco_ph2_value),
		.pll_vco_ph3_en(cmu_fpll_pll_vco_ph3_en),
		.pll_vco_ph3_value(cmu_fpll_pll_vco_ph3_value),
        .xpm_cmu_fpll_core_fpll_refclk_source(cmu_fpll_xpm_cmu_fpll_core_fpll_refclk_source)
	) fpll_inst
	(
		// Input Ports
		.avmmaddress(pll_avmm_address[8:0]),
		// .avmmclk((cmu_fpll_reconfig_en == "0" && cmu_fpll_dps_en == "false") ? 1'b0 : pll_avmm_clk[0]),
		.avmmclk(pll_avmm_clk[0]),
		.avmmread(pll_avmm_read[0]),
		.avmmrstn(pll_avmm_rstn[0]),
		.avmmwrite(pll_avmm_write[0]),
		.avmmwritedata(pll_avmm_writedata[7:0]),

		.cnt_sel(cntsel),
		.csr_bufin(),
		.csr_clk(),
		.csr_en(),
		.csr_in(),
		.csr_en_dly(),
		.dps_rst_n(~final_phase_reset),
		.fbclk_in(fbclk_in),	
		.iqtxrxclk(iqtxrxclk),
		.mdio_dis(),
                .num_phase_shifts(3'd1),
		.pfden(),
		.phase_en(final_phase_en),
		.pma_csr_test_dis(),
		.refclk(refclk_sel_outclk_wire),
		.rst_n(~pll_powerdown_int_gated), //Active low
		.scan_mode_n(),
		.scan_shift_n(),
		.up_dn(updn),
		.clk0bad_in(clk0_bad_wire),
		.clk1bad_in(clk1_bad_wire),
        .lc_to_fpll_refclk(atx_to_fpll_cascade_clk),
			
		// Output Ports
		.clk0bad(clkbad[0]),
		.clk1bad(clkbad[1]),
		.avmmreaddata(pll_avmmreaddata_cmu_fpll[7:0]),
		.block_select(pll_blockselect_cmu_fpll[0]),
		.clk0(tx_serial_clk),
		.clk180(),
		.clklow(clklow),
		.csr_bufout(),
		.csr_out(),
		.fbclk_out(fbclk_wire),
		.fref(fref),
		.hclk_out(pll_pcie_clk),
		.iqtxrxclk_out(iqtxrxclk_wire),
		.lock(pll_locked),
		.outclk({outclk3, outclk2, outclk1, outclk0}),
		.phase_done(pre_phase_done),
		.pll_cascade_out(fpll_to_fpll_cascade_clk)
	);
	
	//-----------------------------------
	// instantiate of twentynm_hssi_pma_cgb_master 
	generate
	if (enable_mcgb == 1) begin  
		twentynm_hssi_pma_cgb_master
		#(
		   .enable_debug_info					(	enable_debug_info									),	   
		   .silicon_rev			    (	hssi_pma_cgb_master_silicon_rev							),
		   .datarate				(	hssi_pma_cgb_master_datarate							),
		   .x1_div_m_sel			(	hssi_pma_cgb_master_x1_div_m_sel							),
		   .prot_mode				(	hssi_pma_cgb_master_prot_mode							),
		   .ser_mode				(	hssi_pma_cgb_master_ser_mode								),
		   .cgb_enable_iqtxrxclk		(	hssi_pma_cgb_master_cgb_enable_iqtxrxclk					), 
		   .cgb_power_down                     	(	hssi_pma_cgb_master_cgb_power_down                    	),
		   //.master_cgb_clock_control          (	hssi_pma_cgb_master_master_cgb_clock_control          	),
		   .observe_cgb_clocks                 	(	hssi_pma_cgb_master_observe_cgb_clocks                	),
		   //.op_mode                           (	hssi_pma_cgb_master_op_mode                           	),
		   //.tx_ucontrol_reset_pcie            (	hssi_pma_cgb_master_tx_ucontrol_reset_pcie            	),
		   .vccdreg_output                     	(	hssi_pma_cgb_master_vccdreg_output                    	),
		   .input_select                       	(	hssi_pma_cgb_master_input_select                      	),
		   .input_select_gen3                  	(	hssi_pma_cgb_master_input_select_gen3                 	),		   
		   .bonding_reset_enable                (   hssi_pma_cgb_master_bonding_reset_enable         		),  
		   .optimal                             (   hssi_pma_cgb_master_optimal                             ), 
		   .pcie_gen3_bitwidth                  (   hssi_pma_cgb_master_pcie_gen3_bitwidth                  ),
		   .powerdown_mode                      (   hssi_pma_cgb_master_powerdown_mode                      ),
		   .sup_mode                            (   hssi_pma_cgb_master_sup_mode                            ),
		   .initial_settings                    (   hssi_pma_cgb_master_initial_settings                    )
		) twentynm_hssi_pma_cgb_master_inst (
		   .cgb_rstb(~mcgb_rst_input),

		   //-----------------------------------           // \OPEN [FITTER]?      
		   .clk_fpll_b(mcgb_aux_clk2),
		   .clk_fpll_t(mcgb_aux_clk0),
		   .clk_lc_b(mcgb_aux_clk1),
		   .clk_lc_t(tx_serial_clk),

		   .clkb_fpll_b( /*unused*/ ),
		   .clkb_fpll_t( /*unused*/ ),
		   .clkb_lc_b( /*unused*/ ),
		   .clkb_lc_t( /*unused*/ ),
		   //-----------------------------------

		   .cpulse_out_bus(tx_bonding_clocks),              

		   .tx_iqtxrxclk_out(pll_extfb_wire),

		   .pcie_sw_done(pcie_sw_done),
		   .pcie_sw(pcie_sw),

		   //-----------------------------------                           
		   .tx_bonding_rstb(1'b1),                        // \NOTE carried over from slave cgb
		   //-----------------------------------  

		   .avmmaddress(avmm_address_mcgb),
		   .avmmclk(avmm_clk_mcgb),
		   .avmmread(avmm_read_mcgb),
		   .avmmrstn(avmm_rstn_mcgb),
		   .avmmwrite(avmm_write_mcgb),
		   .avmmwritedata(avmm_writedata_mcgb),
		   .avmmreaddata(avmmreaddata_mcgb),
		   .blockselect(blockselect_mcgb)        
		); 
	end
	else begin
	   assign tx_bonding_clocks = 0;
	   assign pcie_sw_done = 0;
           assign pll_extfb_wire = 0;
	end
	endgenerate

endmodule

module dps_pulse_ctrl (
    input  wire clk,            // the DPS clock
    input  wire rst,            // active high reset
    input  wire user_phase_en,  // the user's phase_en signal
    output reg  phase_en        // the phase_en signal for the IOPLL atom 
 );
 
    //-------------------------------------------------------------------------
    // States
    localparam IDLE        = 0,  // Idle state: user_phase_en = 0, phase_en = 0
               PULSE       = 1,  // Activate state: phase_en = 1
               WAIT        = 2;  // Wait for user_phase_en to go low

    //-------------------------------------------------------------------------
    // FSM current and next states
    reg [1:0] state, next;     
    
    // State update
    always @(posedge clk) begin
        if (rst)    state <= IDLE;
        else        state <= next; 
    end  

    //-------------------------------------------------------------------------    
    // Next-state and output logic
    always @(*) begin
        next     = IDLE;  // Default next state 
        phase_en = 1'b0;  // Default output
        
        case (state)
            IDLE :  begin
                        if (user_phase_en)  next = PULSE;
                        else                next = IDLE;
                    end     
                         
            PULSE : begin
                        phase_en = 1'b1;
                        next     = WAIT;
                    end
                         
            WAIT :  begin         
                        if (~user_phase_en) next = IDLE;
                        else                next = WAIT;                  
                    end  
        endcase
    end
     
 endmodule

/* This module is used to correctly reset the DPS logic.
   It snoops the phase_en_in pulse and waits for the phase_done_in
   to be asserted by the fpll core. It then generates a phase_reset_out
   pulse that resets the DPS logic in the fpll core and finally
   generates a phase_done_out to send back to the user. phase_reset_in
   generated by the user is OR'ed in with the phase_reset_out generated
   by this module.
 */
module dps_reset_gen (
    input wire clk,               // DPS clock input
    input wire phase_reset_in,    // active high reset input
    input wire phase_en_in,       // phase_en input
    input wire phase_done_in,     // phase_done input		       
    output wire phase_reset_out,  // active high reset output
    output wire phase_done_out    // phase_done output 
 );
 
    //-------------------------------------------------------------------------
    // States
    localparam WAIT_PE_PULSE    = 4'd0,  // Wait for phase_en_in pulse 
               WAIT_CORE_DONE   = 4'd1,  // Wait for phase_done_in pulse
               GEN_RESET_DELAY  = 4'd2,  // Wait for N cycle before asserting phase_reset_out
               GEN_RESET_1      = 4'd3,  // First cycle of phase_reset_out assertion
               GEN_RESET_2      = 4'd4,  // Second cycle of phase_reset_out assertion
               GEN_RESET_DELAY_1 = 4'd5, // Wait for phase_done_in deassertion
               GEN_RESET_DELAY_2 = 4'd6, // Wait before deasserting phase_reset_out 1
               GEN_RESET_DELAY_3 = 4'd7, // Wait before deasserting phase_reset_out 2
               GEN_RESET_DELAY_4 = 4'd8, // Wait before deasserting phase_reset_out 3
               GEN_DONE_DELAY   = 4'd9,  // Wait cycle before asserting phase_done_out 1
               GEN_DONE_DELAY_1 = 4'd10,  // Wait cycle before asserting phase_done_out 2
               GEN_DONE_1       = 4'd11,  // First cycle of phase_done_out
               GEN_DONE_2       = 4'd12;  // Second cycle of phase_done_out
    localparam GEN_RESET_DELAY_COUNT = 6'd52; // Number of cycles after DONE assertions to wait before asserting reset
   
    //-------------------------------------------------------------------------
    // FSM current and next states
    reg [3:0] state, next;
    reg [5:0] clock_counter;
    wire      clock_counter_done;

    // Reset Delay Counter
    assign clock_counter_done = (clock_counter == GEN_RESET_DELAY_COUNT);
    always @(posedge clk) begin
       if (phase_reset_in)    clock_counter <= 6'd0;
       else if (state != GEN_RESET_DELAY) clock_counter <= 6'd0;
       else clock_counter <= clock_counter + 6'd1;
    end
   
    // State update
    always @(posedge clk) begin
        if (phase_reset_in)    state <= WAIT_PE_PULSE;
        else                   state <= next; 
    end  

    //-------------------------------------------------------------------------    
    // Next-state and output logic
    always @(*) begin
        next     = WAIT_PE_PULSE;  // Default next state
       
        case (state)
            WAIT_PE_PULSE : begin
                              if (phase_en_in)    next = WAIT_CORE_DONE;
                              else                next = WAIT_PE_PULSE;
                            end     
                         
            WAIT_CORE_DONE : begin
	                       if (phase_done_in) next = GEN_RESET_DELAY;
	                       else               next = WAIT_CORE_DONE;
                             end
                         
            GEN_RESET_DELAY : begin
                               if (clock_counter_done) next = GEN_RESET_1;
                               else                    next = GEN_RESET_DELAY;
                              end  

            GEN_RESET_1     : begin         
                                next = GEN_RESET_2;
                              end  

	    GEN_RESET_2     : begin         
                                next = GEN_RESET_DELAY_1;
                              end  

	    GEN_RESET_DELAY_1 : begin
	                          if (!phase_done_in) next = GEN_RESET_DELAY_2;
	                          else next = GEN_RESET_DELAY_1;
	                        end
	  
	    GEN_RESET_DELAY_2 : begin
	                          next = GEN_RESET_DELAY_3;
	                        end

	    GEN_RESET_DELAY_3 : begin
	                          next = GEN_RESET_DELAY_4;
	                        end

	    GEN_RESET_DELAY_4 : begin
	                          next = GEN_DONE_DELAY;
	                        end
	  
	    GEN_DONE_DELAY  : begin         
                                next = GEN_DONE_DELAY_1;
                              end  

	    GEN_DONE_DELAY_1: begin         
                                next = GEN_DONE_1;
                              end  

	    GEN_DONE_1      : begin         
	                        next = GEN_DONE_2;
                              end  

	    GEN_DONE_2      : begin         
	                        next = WAIT_PE_PULSE;
                              end  
        endcase
    end

    assign phase_reset_out = (state == GEN_RESET_1) ||
			     (state == GEN_RESET_2) ||
			     (state == GEN_RESET_DELAY_1) ||
			     (state == GEN_RESET_DELAY_2) ||
			     (state == GEN_RESET_DELAY_3) ||
			     (state == GEN_RESET_DELAY_4) ||
			     phase_reset_in;
    assign phase_done_out = (state == GEN_DONE_1) ||
			    (state == GEN_DONE_2);
   
 endmodule
