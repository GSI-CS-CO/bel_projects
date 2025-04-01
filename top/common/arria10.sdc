# Timing constraints
derive_pll_clocks -create_base_clocks
derive_clock_uncertainty

# Named clocks
set clk_ref0_125m_ref_clk              [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk2}]
set clk_ref1_200m_butis_clk            [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk3}]
set clk_ref2_25m_phase_butis_clk       [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk4}]
set clk_ref3_1000m_clk_lvds            [get_clocks {main|\ref_a10:ref_inst|iopll_0|altera_iopll_i|twentynm_pll|lvds_clk[0]}]
set clk_ref4_125m_clk_lvds_enable_18dc [get_clocks {main|\ref_a10:ref_inst|iopll_0|altera_iopll_i|twentynm_pll|iopll_inst|loaden[0]}]
set clk_aux_ref0_125m_ref_clk          [get_clocks {main|\dual_port_wr_core_ref_a10_aux:ref_a10_aux:ref_inst_aux|iopll_0|outclk2}]
set clk_sys0_62_5_sys_clk              [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk0}]
set clk_sys2_20_generic_clk            [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk2}]
set clk_sys4_10_flash_clk              [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk4}]
set clk_dmtd_62_5_clk                  [get_clocks {main|\dmtd_a10:dmtd_inst|iopll_0|outclk0}]
set clk_aux_dmtd_62_5_clk              [get_clocks {main|\dual_port_wr_core_dmtd_a10_aux:dmtd_a10_aux:dmtd_inst_aux|iopll_0|outclk0}]

# Special device input clocks
create_clock -period 125Mhz -name clk_125m_tcb_local_in  [get_ports {clk_125m_tcb_local_i}]
create_clock -period 125Mhz -name clk_125m_tcb_pllref_in [get_ports {clk_125m_tcb_pllref_i}]
create_clock -period 125Mhz -name clk_125m_tcb_sfpref_in [get_ports {clk_125m_tcb_sfpref_i}]
create_clock -period 20Mhz  -name clk_20m_vcxo_in        [get_ports {clk_20m_vcxo_i}]
create_clock -period 125Mhz -name clk_125m_tcb_local_in  [get_ports {clk_125m_local_i}]
create_clock -period 125Mhz -name clk_125m_tcb_pllref_in [get_ports {clk_125m_pllref_i}]
create_clock -period 125Mhz -name clk_125m_tcb_sfpref_in [get_ports {clk_125m_sfpref_i}]

# Cut temperature and voltage sensors
set_false_path -from {monster:main|a10ts:\a10ts_y:a10ts_inst|a10ts_ip:\ext_trigger_n:core_a10ts_ip|altera_temp_sense:temp_sense_0|*} -to {monster:main|a10ts:\a10ts_y:a10ts_inst|*}
set_false_path -from {monster:main|a10vs:\a10vs_y:a10vs_0|a10vs_ip:a10vs_ip_0|a10vs_ip_altera_voltage_sensor_231_dihpyaa:voltage_sensor_0|*} -to {monster:main|a10vs:\a10vs_y:a10vs_0|*}

# PSRAM
#create_clock -name psram_clk -period 16.000 -waveform {0 8} [get_ports {psram_clk}]

# Cut the clock domains from each other
set_clock_groups -asynchronous \
-group [get_clocks {altera_reserved_tck}] \
-group [get_clocks {clk_20m_vcxo_in}] \
-group [get_clocks {clk_125m_tcb_local_i}] \
-group [get_clocks {clk_125m_tcb_pllref_i}] \
-group [get_clocks {clk_125m_tcb_sfpref_i}] \
-group [get_clocks {clk_125m_local_i}] \
-group [get_clocks {clk_125m_pllref_i}] \
-group [get_clocks {clk_125m_sfpref_i}] \
-group [get_clocks {clk_gxbl1c_u24}] \
-group [get_clocks {clk_gxbl1c_w24}] \
-group [get_clocks {~ALTERA_CLKUSR~}] \
-group [get_clocks {pcie_refclk_i}] \
-group [get_clocks {altera_ts_clk}] \
-group [get_clocks {main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|coreclkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|pma_hclk_by2 \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|rx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|rx_clkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|rx_fref \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|rx_pma_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|tx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|pma_hclk_by2 \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|rx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|rx_clkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|rx_fref \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|rx_pma_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|tx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|pma_hclk_by2 \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|rx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|rx_clkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|rx_fref \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|rx_pma_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|tx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|pma_hclk_by2 \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|rx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|rx_clkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|rx_fref \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|rx_pma_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|tx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|hip_cmn_clk[0] \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|pld_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|pll_pcie_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|tx_bonding_clocks[0] \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_scu4:hip|pcie_a10_hip_0|tx_clkout}] \
-group [get_clocks {main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|coreclkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|pma_hclk_by2 \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|rx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|rx_clkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|rx_fref \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|rx_pma_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|tx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|pma_hclk_by2 \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|rx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|rx_clkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|rx_fref \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|rx_pma_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|tx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|pma_hclk_by2 \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|rx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|rx_clkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|rx_fref \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|rx_pma_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|tx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|pma_hclk_by2 \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|rx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|rx_clkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|rx_fref \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|rx_pma_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|tx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|hip_cmn_clk[0] \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|pld_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|pll_pcie_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|tx_bonding_clocks[0] \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm4:hip|pcie_a10_hip_0|tx_clkout}] \
-group [get_clocks {main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|coreclkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|pma_hclk_by2 \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|rx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|rx_clkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|rx_fref \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|rx_pma_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|tx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|pma_hclk_by2 \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|rx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|rx_clkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|rx_fref \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|rx_pma_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|tx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|pma_hclk_by2 \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|rx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|rx_clkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|rx_fref \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|rx_pma_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|tx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|pma_hclk_by2 \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|rx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|rx_clkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|rx_fref \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|rx_pma_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|tx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|hip_cmn_clk[0] \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|pld_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|pll_pcie_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|tx_clkout}] \
-group [get_clocks {main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|coreclkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|pma_hclk_by2 \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|rx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|rx_clkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|rx_fref \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|rx_pma_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|tx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|pma_hclk_by2 \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|rx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|rx_clkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|rx_fref \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|rx_pma_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|tx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|pma_hclk_by2 \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|rx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|rx_clkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|rx_fref \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|rx_pma_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|tx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|pma_hclk_by2 \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|rx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|rx_clkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|rx_fref \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|rx_pma_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|tx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|hip_cmn_clk[0] \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|pld_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|pll_pcie_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_pex10:hip|pcie_a10_hip_0|tx_clkout}] \
-group [get_clocks {main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|coreclkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|pma_hclk_by2 \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|rx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|rx_clkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|rx_fref \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|rx_pma_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|g_xcvr_native_insts[0]|tx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|pma_hclk_by2 \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|rx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|rx_clkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|rx_fref \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|rx_pma_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|g_xcvr_native_insts[1]|tx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|pma_hclk_by2 \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|rx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|rx_clkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|rx_fref \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|rx_pma_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|g_xcvr_native_insts[2]|tx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|pma_hclk_by2 \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|rx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|rx_clkout \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|rx_fref \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|rx_pma_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|g_xcvr_native_insts[3]|tx_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|hip_cmn_clk[0] \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|pld_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|pll_pcie_clk \
                    main|\pcie_y:pcie|pcie_phy|\arria10gx_ftm10:hip|pcie_a10_hip_0|tx_clkout}] \
-group [get_clocks {main|\phy_a10:phy|\det_phy:scu4_phy:inst_phy|xcvr_native_a10_0|avmmclk \
                    main|\phy_a10:phy|\det_phy:scu4_phy:inst_phy|xcvr_native_a10_0|rx_clkout \
                    main|\phy_a10:phy|\det_phy:scu4_phy:inst_phy|xcvr_native_a10_0|rx_pma_clk \
                    main|\phy_a10:phy|\det_phy:scu4_phy:inst_phy|xcvr_native_a10_0|tx_clkout \
                    main|\phy_a10:phy|\det_phy:scu4_phy:inst_phy|xcvr_native_a10_0|tx_pma_clk}] \
-group [get_clocks {main|\phy_a10:phy|\det_phy:ftm4_phy:inst_phy|xcvr_native_a10_0|avmmclk \
                    main|\phy_a10:phy|\det_phy:ftm4_phy:inst_phy|xcvr_native_a10_0|rx_clkout \
                    main|\phy_a10:phy|\det_phy:ftm4_phy:inst_phy|xcvr_native_a10_0|rx_pma_clk \
                    main|\phy_a10:phy|\det_phy:ftm4_phy:inst_phy|xcvr_native_a10_0|tx_clkout \
                    main|\phy_a10:phy|\det_phy:ftm4_phy:inst_phy|xcvr_native_a10_0|tx_pma_clk}] \
-group [get_clocks {main|\dual_port_wr:phy_aux_a10:phy_aux|\det_phy:ftm4_phy:inst_phy|xcvr_native_a10_0|avmmclk \
                    main|\dual_port_wr:phy_aux_a10:phy_aux|\det_phy:ftm4_phy:inst_phy|xcvr_native_a10_0|rx_clkout \
                    main|\dual_port_wr:phy_aux_a10:phy_aux|\det_phy:ftm4_phy:inst_phy|xcvr_native_a10_0|rx_pma_clk \
                    main|\dual_port_wr:phy_aux_a10:phy_aux|\det_phy:ftm4_phy:inst_phy|xcvr_native_a10_0|tx_clkout \
                    main|\dual_port_wr:phy_aux_a10:phy_aux|\det_phy:ftm4_phy:inst_phy|xcvr_native_a10_0|tx_pma_clk}] \
-group [get_clocks {main|\dual_port_wr:phy_aux_a10:phy_aux|\det_phy:ftm10_phy:inst_phy|xcvr_native_a10_0|avmmclk \
                    main|\dual_port_wr:phy_aux_a10:phy_aux|\det_phy:ftm10_phy:inst_phy|xcvr_native_a10_0|rx_clkout \
                    main|\dual_port_wr:phy_aux_a10:phy_aux|\det_phy:ftm10_phy:inst_phy|xcvr_native_a10_0|rx_pma_clk \
                    main|\dual_port_wr:phy_aux_a10:phy_aux|\det_phy:ftm10_phy:inst_phy|xcvr_native_a10_0|tx_clkout \
                    main|\dual_port_wr:phy_aux_a10:phy_aux|\det_phy:ftm10_phy:inst_phy|xcvr_native_a10_0|tx_pma_clk}] \
-group [get_clocks {main|\phy_a10:phy|\det_phy:e3p1_phy:inst_phy|xcvr_native_a10_0|avmmclk \
                    main|\phy_a10:phy|\det_phy:e3p1_phy:inst_phy|xcvr_native_a10_0|rx_clkout \
                    main|\phy_a10:phy|\det_phy:e3p1_phy:inst_phy|xcvr_native_a10_0|rx_pma_clk \
                    main|\phy_a10:phy|\det_phy:e3p1_phy:inst_phy|xcvr_native_a10_0|tx_clkout \
                    main|\phy_a10:phy|\det_phy:e3p1_phy:inst_phy|xcvr_native_a10_0|tx_pma_clk}] \
-group [get_clocks {main|\phy_a10:phy|\det_phy:pex10_phy:inst_phy|xcvr_native_a10_0|avmmclk \
                    main|\phy_a10:phy|\det_phy:pex10_phy:inst_phy|xcvr_native_a10_0|rx_clkout \
                    main|\phy_a10:phy|\det_phy:pex10_phy:inst_phy|xcvr_native_a10_0|rx_pma_clk \
                    main|\phy_a10:phy|\det_phy:pex10_phy:inst_phy|xcvr_native_a10_0|tx_clkout \
                    main|\phy_a10:phy|\det_phy:pex10_phy:inst_phy|xcvr_native_a10_0|tx_pma_clk}] \
-group [get_clocks {main|\phy_a10:phy|\det_phy:ftm10_phy:inst_phy|xcvr_native_a10_0|avmmclk \
                    main|\phy_a10:phy|\det_phy:ftm10_phy:inst_phy|xcvr_native_a10_0|rx_clkout \
                    main|\phy_a10:phy|\det_phy:ftm10_phy:inst_phy|xcvr_native_a10_0|rx_pma_clk \
                    main|\phy_a10:phy|\det_phy:ftm10_phy:inst_phy|xcvr_native_a10_0|tx_clkout \
                    main|\phy_a10:phy|\det_phy:ftm10_phy:inst_phy|xcvr_native_a10_0|tx_pma_clk}] \
-group [get_clocks {main|\dmtd_a10:dmtd_inst|iopll_0|outclk0}] \
-group [get_clocks {main|\dual_port_wr_core_dmtd_a10_aux:dmtd_a10_aux:dmtd_inst_aux|iopll_0|outclk0}] \
-group [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk0}] \
-group [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk2}] \
-group [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk3}] \
-group [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk4}] \
-group [get_clocks {main|\dual_port_wr_core_ref_a10_aux:ref_a10_aux:ref_inst_aux|iopll_0|outclk2}] \
-group [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk2 \
                    main|\ref_a10:ref_inst|iopll_0|altera_iopll_i|twentynm_pll|lvds_clk[0] \
                    main|\ref_a10:ref_inst|iopll_0|altera_iopll_i|twentynm_pll|iopll_inst|loaden[0]}] \
-group [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk3 \
                    main|\ref_a10:ref_inst|iopll_0|outclk4}] \
