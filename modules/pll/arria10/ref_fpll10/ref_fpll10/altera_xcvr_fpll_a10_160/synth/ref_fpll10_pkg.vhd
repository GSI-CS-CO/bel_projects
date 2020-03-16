library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

package ref_fpll10_pkg is
	component altera_xcvr_fpll_a10 is
		generic (
			enable_pll_reconfig                                                          : integer := 0;
			rcfg_jtag_enable                                                             : integer := 0;
			rcfg_separate_avmm_busy                                                      : integer := 0;
			dbg_embedded_debug_enable                                                    : integer := 0;
			dbg_capability_reg_enable                                                    : integer := 0;
			dbg_user_identifier                                                          : integer := 0;
			dbg_stat_soft_logic_enable                                                   : integer := 0;
			dbg_ctrl_soft_logic_enable                                                   : integer := 0;
			cmu_fpll_silicon_rev                                                         : string  := "20nm5es";
			cmu_fpll_is_otn                                                              : string  := "false";
			cmu_fpll_is_sdi                                                              : string  := "false";
			cmu_fpll_f_max_vco                                                           : string  := "1";
			cmu_fpll_f_min_vco                                                           : string  := "1";
			cmu_fpll_feedback                                                            : string  := "normal";
			cmu_fpll_fpll_cas_out_enable                                                 : string  := "fpll_cas_out_disable";
			cmu_fpll_fpll_hclk_out_enable                                                : string  := "fpll_hclk_out_disable";
			cmu_fpll_fpll_iqtxrxclk_out_enable                                           : string  := "fpll_iqtxrxclk_out_disable";
			cmu_fpll_l_counter                                                           : integer := 1;
			cmu_fpll_m_counter                                                           : integer := 1;
			cmu_fpll_n_counter                                                           : integer := 1;
			cmu_fpll_out_freq_hz                                                         : string  := "0 hz";
			cmu_fpll_out_freq                                                            : string  := "1";
			cmu_fpll_pll_vco_freq_band_0                                                 : string  := "pll_freq_band0";
			cmu_fpll_pll_vco_freq_band_1                                                 : string  := "pll_freq_band0_1";
			cmu_fpll_primary_use                                                         : string  := "tx";
			cmu_fpll_prot_mode                                                           : string  := "basic_tx";
			cmu_fpll_reference_clock_frequency_scratch                                   : string  := "0 hz";
			cmu_fpll_vco_freq_hz                                                         : string  := "0 hz";
			cmu_fpll_vco_freq                                                            : string  := "1";
			cmu_fpll_pll_bw_mode                                                         : string  := "high";
			cmu_fpll_datarate                                                            : string  := "0 bps";
			cmu_fpll_pll_cmu_rstn_value                                                  : string  := "true";
			cmu_fpll_pll_lpf_rstn_value                                                  : string  := "lpf_normal";
			cmu_fpll_pll_ppm_clk0_src                                                    : string  := "ppm_clk0_vss";
			cmu_fpll_pll_ppm_clk1_src                                                    : string  := "ppm_clk1_vss";
			cmu_fpll_pll_rstn_override                                                   : string  := "false";
			cmu_fpll_pll_op_mode                                                         : string  := "false";
			cmu_fpll_pll_optimal                                                         : string  := "true";
			cmu_fpll_is_pa_core                                                          : string  := "false";
			cmu_fpll_pll_powerdown_mode                                                  : string  := "false";
			cmu_fpll_pll_sup_mode                                                        : string  := "user_mode";
			cmu_fpll_pll_c0_pllcout_enable                                               : string  := "false";
			cmu_fpll_pll_c_counter_0                                                     : integer := 1;
			cmu_fpll_pll_c_counter_0_min_tco_enable                                      : string  := "true";
			cmu_fpll_pll_c_counter_0_in_src                                              : string  := "m_cnt_in_src_test_clk";
			cmu_fpll_pll_c_counter_0_ph_mux_prst                                         : integer := 0;
			cmu_fpll_pll_c_counter_0_prst                                                : integer := 1;
			cmu_fpll_pll_c_counter_0_coarse_dly                                          : string  := "0 ps";
			cmu_fpll_pll_c_counter_0_fine_dly                                            : string  := "0 ps";
			cmu_fpll_pll_c1_pllcout_enable                                               : string  := "false";
			cmu_fpll_pll_c_counter_1                                                     : integer := 1;
			cmu_fpll_pll_c_counter_1_min_tco_enable                                      : string  := "true";
			cmu_fpll_pll_c_counter_1_in_src                                              : string  := "m_cnt_in_src_test_clk";
			cmu_fpll_pll_c_counter_1_ph_mux_prst                                         : integer := 0;
			cmu_fpll_pll_c_counter_1_prst                                                : integer := 1;
			cmu_fpll_pll_c_counter_1_coarse_dly                                          : string  := "0 ps";
			cmu_fpll_pll_c_counter_1_fine_dly                                            : string  := "0 ps";
			cmu_fpll_pll_c2_pllcout_enable                                               : string  := "false";
			cmu_fpll_pll_c_counter_2                                                     : integer := 1;
			cmu_fpll_pll_c_counter_2_min_tco_enable                                      : string  := "true";
			cmu_fpll_pll_c_counter_2_in_src                                              : string  := "m_cnt_in_src_test_clk";
			cmu_fpll_pll_c_counter_2_ph_mux_prst                                         : integer := 0;
			cmu_fpll_pll_c_counter_2_prst                                                : integer := 1;
			cmu_fpll_pll_c_counter_2_coarse_dly                                          : string  := "0 ps";
			cmu_fpll_pll_c_counter_2_fine_dly                                            : string  := "0 ps";
			cmu_fpll_pll_c3_pllcout_enable                                               : string  := "false";
			cmu_fpll_pll_c_counter_3                                                     : integer := 1;
			cmu_fpll_pll_c_counter_3_min_tco_enable                                      : string  := "true";
			cmu_fpll_pll_c_counter_3_in_src                                              : string  := "m_cnt_in_src_test_clk";
			cmu_fpll_pll_c_counter_3_ph_mux_prst                                         : integer := 0;
			cmu_fpll_pll_c_counter_3_prst                                                : integer := 1;
			cmu_fpll_pll_c_counter_3_coarse_dly                                          : string  := "0 ps";
			cmu_fpll_pll_c_counter_3_fine_dly                                            : string  := "0 ps";
			cmu_fpll_pll_atb                                                             : string  := "atb_selectdisable";
			cmu_fpll_pll_fbclk_mux_1                                                     : string  := "pll_fbclk_mux_1_glb";
			cmu_fpll_pll_fbclk_mux_2                                                     : string  := "pll_fbclk_mux_2_fb_1";
			cmu_fpll_pll_iqclk_mux_sel                                                   : string  := "power_down";
			cmu_fpll_pll_cp_compensation                                                 : string  := "true";
			cmu_fpll_pll_cp_current_setting                                              : string  := "cp_current_setting0";
			cmu_fpll_pll_cp_testmode                                                     : string  := "cp_normal";
			cmu_fpll_pll_cp_lf_3rd_pole_freq                                             : string  := "lf_3rd_pole_setting0";
			cmu_fpll_pll_lf_cbig                                                         : string  := "lf_cbig_setting0";
			cmu_fpll_pll_cp_lf_order                                                     : string  := "lf_2nd_order";
			cmu_fpll_pll_lf_resistance                                                   : string  := "lf_res_setting0";
			cmu_fpll_pll_lf_ripplecap                                                    : string  := "lf_ripple_enabled_0";
			cmu_fpll_pll_vco_ph0_en                                                      : string  := "false";
			cmu_fpll_pll_vco_ph0_value                                                   : string  := "pll_vco_ph0_vss";
			cmu_fpll_pll_vco_ph1_en                                                      : string  := "false";
			cmu_fpll_pll_vco_ph1_value                                                   : string  := "pll_vco_ph1_vss";
			cmu_fpll_pll_vco_ph2_en                                                      : string  := "false";
			cmu_fpll_pll_vco_ph2_value                                                   : string  := "pll_vco_ph2_vss";
			cmu_fpll_pll_vco_ph3_en                                                      : string  := "false";
			cmu_fpll_pll_vco_ph3_value                                                   : string  := "pll_vco_ph3_vss";
			cmu_fpll_pll_dsm_mode                                                        : string  := "dsm_mode_integer";
			cmu_fpll_pll_dsm_out_sel                                                     : string  := "pll_dsm_disable";
			cmu_fpll_pll_dsm_ecn_bypass                                                  : string  := "false";
			cmu_fpll_pll_dsm_ecn_test_en                                                 : string  := "false";
			cmu_fpll_pll_dsm_fractional_division                                         : string  := "0";
			cmu_fpll_pll_dsm_fractional_value_ready                                      : string  := "pll_k_ready";
			cmu_fpll_pll_l_counter_bypass                                                : string  := "false";
			cmu_fpll_pll_l_counter                                                       : integer := 1;
			cmu_fpll_pll_l_counter_enable                                                : string  := "true";
			cmu_fpll_pll_lock_fltr_cfg                                                   : integer := 1;
			cmu_fpll_pll_lock_fltr_test                                                  : string  := "pll_lock_fltr_nrm";
			cmu_fpll_pll_unlock_fltr_cfg                                                 : integer := 0;
			cmu_fpll_pll_m_counter                                                       : integer := 1;
			cmu_fpll_pll_m_counter_min_tco_enable                                        : string  := "true";
			cmu_fpll_pll_m_counter_in_src                                                : string  := "m_cnt_in_src_test_clk";
			cmu_fpll_pll_n_counter                                                       : integer := 1;
			cmu_fpll_pll_tclk_mux_en                                                     : string  := "false";
			cmu_fpll_pll_tclk_sel                                                        : string  := "pll_tclk_m_src";
			cmu_fpll_pll_dprio_base_addr                                                 : integer := 256;
			cmu_fpll_pll_dprio_broadcast_en                                              : string  := "false";
			cmu_fpll_pll_dprio_cvp_inter_sel                                             : string  := "true";
			cmu_fpll_pll_dprio_force_inter_sel                                           : string  := "false";
			cmu_fpll_pll_dprio_power_iso_en                                              : string  := "true";
			cmu_fpll_pll_dprio_status_select                                             : string  := "dprio_normal_status";
			cmu_fpll_pll_extra_csr                                                       : integer := 0;
			cmu_fpll_pll_nreset_invert                                                   : string  := "false";
			cmu_fpll_pll_ctrl_override_setting                                           : string  := "true";
			cmu_fpll_pll_enable                                                          : string  := "false";
			cmu_fpll_pll_test_enable                                                     : string  := "false";
			cmu_fpll_pll_ctrl_plniotri_override                                          : string  := "false";
			cmu_fpll_pll_vccr_pd_en                                                      : string  := "false";
			cmu_fpll_bw_sel                                                              : string  := "auto";
			cmu_fpll_compensation_mode                                                   : string  := "direct";
			cmu_fpll_duty_cycle_0                                                        : integer := 50;
			cmu_fpll_duty_cycle_1                                                        : integer := 50;
			cmu_fpll_duty_cycle_2                                                        : integer := 50;
			cmu_fpll_duty_cycle_3                                                        : integer := 50;
			cmu_fpll_hssi_output_clock_frequency                                         : string  := "0 ps";
			cmu_fpll_is_cascaded_pll                                                     : string  := "false";
			cmu_fpll_output_clock_frequency_0                                            : string  := "0 ps";
			cmu_fpll_output_clock_frequency_1                                            : string  := "0 ps";
			cmu_fpll_output_clock_frequency_2                                            : string  := "0 ps";
			cmu_fpll_output_clock_frequency_3                                            : string  := "0 ps";
			cmu_fpll_phase_shift_0                                                       : string  := "0 ps";
			cmu_fpll_phase_shift_1                                                       : string  := "0 ps";
			cmu_fpll_phase_shift_2                                                       : string  := "0 ps";
			cmu_fpll_phase_shift_3                                                       : string  := "0 ps";
			cmu_fpll_reference_clock_frequency                                           : string  := "0 ps";
			cmu_fpll_vco_frequency                                                       : string  := "0 ps";
			cmu_fpll_cgb_div                                                             : integer := 1;
			cmu_fpll_pma_width                                                           : integer := 8;
			cmu_fpll_f_out_c3_hz                                                         : string  := "0 hz";
			cmu_fpll_f_out_c1_hz                                                         : string  := "0 hz";
			cmu_fpll_f_out_c0_hz                                                         : string  := "0 hz";
			cmu_fpll_f_out_c2_hz                                                         : string  := "0 hz";
			cmu_fpll_f_out_c3                                                            : string  := "1";
			cmu_fpll_f_out_c1                                                            : string  := "1";
			cmu_fpll_f_out_c0                                                            : string  := "1";
			cmu_fpll_f_out_c2                                                            : string  := "1";
			cmu_fpll_initial_settings                                                    : string  := "true";
			cmu_fpll_m_counter_c2                                                        : integer := 0;
			cmu_fpll_m_counter_c3                                                        : integer := 0;
			cmu_fpll_m_counter_c0                                                        : integer := 0;
			cmu_fpll_m_counter_c1                                                        : integer := 0;
			cmu_fpll_pfd_freq                                                            : string  := "1";
			cmu_fpll_pll_vco_freq_band_0_fix_high                                        : string  := "pll_vco_freq_band_0_fix_high_0";
			cmu_fpll_pll_vco_freq_band_1_fix_high                                        : string  := "pll_vco_freq_band_1_fix_high_0";
			cmu_fpll_xpm_cmu_fpll_core_cal_vco_count_length                              : string  := "sel_8b_count";
			cmu_fpll_xpm_cmu_fpll_core_pfd_pulse_width                                   : string  := "pulse_width_setting0";
			cmu_fpll_pll_vco_freq_band_1_dyn_high_bits                                   : integer := 0;
			cmu_fpll_set_fpll_input_freq_range                                           : integer := 0;
			cmu_fpll_pll_vco_freq_band_0_fix                                             : integer := 1;
			cmu_fpll_pll_vco_freq_band_0_dyn_high_bits                                   : integer := 0;
			cmu_fpll_pll_vco_freq_band_1_fix                                             : integer := 1;
			cmu_fpll_xpm_cmu_fpll_core_xpm_cpvco_fpll_xpm_chgpmplf_fpll_cp_current_boost : string  := "normal_setting";
			cmu_fpll_xpm_cmu_fpll_core_fpll_refclk_source                                : string  := "normal_refclk";
			cmu_fpll_pll_vco_freq_band_0_dyn_low_bits                                    : integer := 0;
			cmu_fpll_xpm_cmu_fpll_core_pfd_delay_compensation                            : string  := "normal_delay";
			cmu_fpll_pll_vco_freq_band_1_dyn_low_bits                                    : integer := 0;
			cmu_fpll_refclk_select_mux_pll_clk_sel_override                              : string  := "normal";
			cmu_fpll_refclk_select_mux_pll_clk_sel_override_value                        : string  := "select_clk0";
			cmu_fpll_refclk_select_mux_pll_auto_clk_sw_en                                : string  := "false";
			cmu_fpll_refclk_select_mux_pll_clk_loss_edge                                 : string  := "pll_clk_loss_both_edges";
			cmu_fpll_refclk_select_mux_pll_clk_loss_sw_en                                : string  := "false";
			cmu_fpll_refclk_select_mux_pll_clk_sw_dly                                    : integer := 0;
			cmu_fpll_refclk_select_mux_pll_manu_clk_sw_en                                : string  := "false";
			cmu_fpll_refclk_select_mux_pll_sw_refclk_src                                 : string  := "pll_sw_refclk_src_clk_0";
			cmu_fpll_refclk_select_mux_silicon_rev                                       : string  := "20nm5es";
			cmu_fpll_refclk_select_mux_refclk_select0                                    : string  := "ref_iqclk0";
			cmu_fpll_refclk_select_mux_refclk_select1                                    : string  := "ref_iqclk0";
			cmu_fpll_refclk_select_mux_mux0_inclk0_logical_to_physical_mapping           : string  := "ref_iqclk0";
			cmu_fpll_refclk_select_mux_mux0_inclk1_logical_to_physical_mapping           : string  := "ref_iqclk0";
			cmu_fpll_refclk_select_mux_mux0_inclk2_logical_to_physical_mapping           : string  := "ref_iqclk0";
			cmu_fpll_refclk_select_mux_mux0_inclk3_logical_to_physical_mapping           : string  := "ref_iqclk0";
			cmu_fpll_refclk_select_mux_mux0_inclk4_logical_to_physical_mapping           : string  := "ref_iqclk0";
			cmu_fpll_refclk_select_mux_mux1_inclk0_logical_to_physical_mapping           : string  := "ref_iqclk0";
			cmu_fpll_refclk_select_mux_mux1_inclk1_logical_to_physical_mapping           : string  := "ref_iqclk0";
			cmu_fpll_refclk_select_mux_mux1_inclk2_logical_to_physical_mapping           : string  := "ref_iqclk0";
			cmu_fpll_refclk_select_mux_mux1_inclk3_logical_to_physical_mapping           : string  := "ref_iqclk0";
			cmu_fpll_refclk_select_mux_mux1_inclk4_logical_to_physical_mapping           : string  := "ref_iqclk0";
			enable_analog_resets                                                         : integer := 0;
			hip_cal_en                                                                   : string  := "disable";
			cmu_fpll_reconfig_en                                                         : string  := "";
			cmu_fpll_dps_en                                                              : string  := "";
			cmu_fpll_calibration_en                                                      : string  := "";
			cmu_fpll_refclk_freq                                                         : string  := "";
			enable_mcgb                                                                  : integer := 0;
			enable_mcgb_debug_ports_parameters                                           : integer := 0;
			hssi_pma_cgb_master_prot_mode                                                : string  := "";
			hssi_pma_cgb_master_silicon_rev                                              : string  := "";
			hssi_pma_cgb_master_x1_div_m_sel                                             : string  := "";
			hssi_pma_cgb_master_cgb_enable_iqtxrxclk                                     : string  := "";
			hssi_pma_cgb_master_ser_mode                                                 : string  := "";
			hssi_pma_cgb_master_datarate                                                 : string  := "";
			hssi_pma_cgb_master_cgb_power_down                                           : string  := "normal_cgb";
			hssi_pma_cgb_master_observe_cgb_clocks                                       : string  := "observe_nothing";
			hssi_pma_cgb_master_op_mode                                                  : string  := "enabled";
			hssi_pma_cgb_master_tx_ucontrol_reset_pcie                                   : string  := "pcscorehip_controls_mcgb";
			hssi_pma_cgb_master_vccdreg_output                                           : string  := "vccdreg_nominal";
			hssi_pma_cgb_master_input_select                                             : string  := "lcpll_top";
			hssi_pma_cgb_master_input_select_gen3                                        : string  := "unused"
		);
		port (
			pll_refclk0              : in  std_logic                     := 'X';             -- clk
			pll_powerdown            : in  std_logic                     := 'X';             -- pll_powerdown
			pll_locked               : out std_logic;                                        -- pll_locked
			outclk0                  : out std_logic;                                        -- clk
			outclk1                  : out std_logic;                                        -- clk
			outclk2                  : out std_logic;                                        -- clk
			outclk3                  : out std_logic;                                        -- clk
			pll_cal_busy             : out std_logic;                                        -- pll_cal_busy
			pll_refclk1              : in  std_logic                     := 'X';             -- clk
			pll_refclk2              : in  std_logic                     := 'X';             -- clk
			pll_refclk3              : in  std_logic                     := 'X';             -- clk
			pll_refclk4              : in  std_logic                     := 'X';             -- clk
			tx_serial_clk            : out std_logic;                                        -- clk
			pll_pcie_clk             : out std_logic;                                        -- pll_pcie_clk
			fpll_to_fpll_cascade_clk : out std_logic;                                        -- clk
			hssi_pll_cascade_clk     : out std_logic;                                        -- clk
			atx_to_fpll_cascade_clk  : in  std_logic                     := 'X';             -- clk
			reconfig_clk0            : in  std_logic                     := 'X';             -- clk
			reconfig_reset0          : in  std_logic                     := 'X';             -- reset
			reconfig_write0          : in  std_logic                     := 'X';             -- write
			reconfig_read0           : in  std_logic                     := 'X';             -- read
			reconfig_address0        : in  std_logic_vector(9 downto 0)  := (others => 'X'); -- address
			reconfig_writedata0      : in  std_logic_vector(31 downto 0) := (others => 'X'); -- writedata
			reconfig_readdata0       : out std_logic_vector(31 downto 0);                    -- readdata
			reconfig_waitrequest0    : out std_logic;                                        -- waitrequest
			avmm_busy0               : out std_logic;                                        -- avmm_busy0
			hip_cal_done             : out std_logic;                                        -- hip_cal_done
			phase_reset              : in  std_logic                     := 'X';             -- phase_reset
			phase_en                 : in  std_logic                     := 'X';             -- phase_en
			updn                     : in  std_logic                     := 'X';             -- updn
			cntsel                   : in  std_logic_vector(3 downto 0)  := (others => 'X'); -- cntsel
			phase_done               : out std_logic;                                        -- phase_done
			extswitch                : in  std_logic                     := 'X';             -- extswitch
			activeclk                : out std_logic;                                        -- activeclk
			clkbad                   : out std_logic_vector(1 downto 0);                     -- clkbad
			clklow                   : out std_logic;                                        -- clk
			fref                     : out std_logic;                                        -- clk
			mcgb_rst                 : in  std_logic                     := 'X';             -- mcgb_rst
			mcgb_aux_clk0            : in  std_logic                     := 'X';             -- tx_serial_clk
			mcgb_aux_clk1            : in  std_logic                     := 'X';             -- tx_serial_clk
			mcgb_aux_clk2            : in  std_logic                     := 'X';             -- tx_serial_clk
			tx_bonding_clocks        : out std_logic_vector(5 downto 0);                     -- clk
			mcgb_serial_clk          : out std_logic;                                        -- clk
			pcie_sw                  : in  std_logic_vector(1 downto 0)  := (others => 'X'); -- pcie_sw
			pcie_sw_done             : out std_logic_vector(1 downto 0);                     -- pcie_sw_done
			reconfig_clk1            : in  std_logic                     := 'X';             -- clk
			reconfig_reset1          : in  std_logic                     := 'X';             -- reset
			reconfig_write1          : in  std_logic                     := 'X';             -- write
			reconfig_read1           : in  std_logic                     := 'X';             -- read
			reconfig_address1        : in  std_logic_vector(9 downto 0)  := (others => 'X'); -- address
			reconfig_writedata1      : in  std_logic_vector(31 downto 0) := (others => 'X'); -- writedata
			reconfig_readdata1       : out std_logic_vector(31 downto 0);                    -- readdata
			reconfig_waitrequest1    : out std_logic;                                        -- waitrequest
			mcgb_cal_busy            : out std_logic;                                        -- mcgb_cal_busy
			mcgb_hip_cal_done        : out std_logic                                         -- hip_cal_done
		);
	end component altera_xcvr_fpll_a10;

end ref_fpll10_pkg;
