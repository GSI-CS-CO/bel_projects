# Timing constraints
derive_pll_clocks -create_base_clocks
derive_clock_uncertainty

# Named clocks
set clk_ref0_125m_ref_clk              [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[0].output_counter|divclk}]
set clk_ref1_200m_butis_clk            [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[1].output_counter|divclk}]
set clk_ref2_25m_phase_butis_clk       [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[2].output_counter|divclk}]
set clk_ref3_1000m_clk_lvds            [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[3].output_counter|divclk}]
set clk_ref4_125m_clk_lvds_enable_18dc [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[4].output_counter|divclk}]
set clk_sys0_62_5_sys_clk              [get_clocks {main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[0].gpll~PLL_OUTPUT_COUNTER|divclk}]
set clk_sys1_100_reconf_clk            [get_clocks {main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[1].gpll~PLL_OUTPUT_COUNTER|divclk}]
set clk_sys2_20_lcd_clk                [get_clocks {main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[2].gpll~PLL_OUTPUT_COUNTER|divclk}]
set clk_sys3_10_update_clk             [get_clocks {main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[3].gpll~PLL_OUTPUT_COUNTER|divclk}]
set clk_sys3_20_flash_ext_clk          [get_clocks {main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[4].gpll~PLL_OUTPUT_COUNTER|divclk}]
set clk_dmtd_62_5_clk                  [get_clocks {main|\dmtd_a5:dmtd_inst|dmtd_pll5_inst|altera_pll_i|general[0].gpll~PLL_OUTPUT_COUNTER|divclk}]

#######################################################################################################################

# Phy clocks
create_clock -name {monster:main|ref_pll5:\ref_a5:ref_inst|ref_pll5_0002:ref_pll5_inst|altera_pll:altera_pll_i|altera_arriav_pll:arriav_pll|altera_arriav_pll_base:fpll_0|PLL_RECONFIG~FMAX_CAP_FF} -period 8.000 [get_pins {main|phase|raw_trap|clk}]

#######################################################################################################################

# Special device input clocks
create_clock -period 10Mhz  -name exploder5_ext_clk_in  [get_ports {lvds_clk_p_i}]
create_clock -period 10Mhz  -name exploder5_sfp_clk_in  [get_ports {clk_sfp_i}]
create_clock -period 125Mhz -name pexarria5_sfp_clk_in  [get_ports {sfp234_ref_clk_i}]
create_clock -period 10Mhz  -name pexp_ext_clk_in       [get_ports {clk_lvtio_i}]
create_clock -period 125Mhz -name pexp_sfp_clk_in       [get_ports {clk_sfp_ref_i}]
create_clock -period 125Mhz -name microtca_sfp_clk_in   [get_ports {clk_sfp_ref_i}]
create_clock -period 10Mhz  -name microtca_ext_clk_p_in [get_ports {clk_lvtio_p_i}]
create_clock -period 10Mhz  -name microtca_ext_clk_n_in [get_ports {clk_lvtio_n_i}]
create_clock -period 125Mhz -name pmc_sfp_clk_in        [get_ports {clk_sfp_ref_i}]
create_clock -period 10Mhz  -name pmc_ext_clk_in        [get_ports {clk_lvtio_i}]
create_clock -period 30Mhz  -name pmc_pci_clk_in        [get_ports {pmc_clk_i}]
create_clock -period 10Mhz  -name clk10                  [get_ports {monster:main|clk_10m}]


#######################################################################################################################

# Cut the clock domains from each other
set_clock_groups -asynchronous \
 -group {exploder5_ext_clk_in} \
 -group {exploder5_sfp_clk_in} \
 -group [get_clocks { main|\nau8811_y:nau8811_audio|\audio_pll_y:x|audio_pll_inst|altera_pll_i|general[0].gpll~PLL_OUTPUT_COUNTER|divclk} ] \
 -group {pexarria5_sfp_clk_in} \
 -group {pexp_ext_clk_in} \
 -group {pexp_sfp_clk_in} \
 -group {microtca_sfp_clk_in} \
 -group {microtca_ext_clk_p_in} \
 -group {microtca_ext_clk_n_in} \
 -group {pmc_sfp_clk_in} \
 -group {pmc_ext_clk_in} \
 -group {pmc_pci_clk_in} \
 -group [get_clocks { pmc_clk_i } ] \
 -group [get_clocks { altera_reserved_tck } ] \
 -group [get_clocks { alt_cal_av_edge_detect_clk } ] \
 -group [get_clocks { clk_20m_vcxo_i } ] \
 -group [get_clocks { clk_125m_local_i } ] \
 -group [get_clocks { clk_125m_pllref_i } ] \
 -group [get_clocks { main|\dmtd_a5:dmtd_inst|dmtd_pll5_inst|altera_pll_i|general[0].gpll~PLL_OUTPUT_COUNTER|divclk } ] \
 -group [get_clocks { main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[1].output_counter|divclk \
                      main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[2].output_counter|divclk } ] \
 -group [get_clocks { main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[0].gpll~PLL_OUTPUT_COUNTER|divclk \
                      main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[2].gpll~PLL_OUTPUT_COUNTER|divclk \
                      main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[3].gpll~PLL_OUTPUT_COUNTER|divclk } ] \
 -group [get_clocks { main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[4].gpll~PLL_OUTPUT_COUNTER|divclk } ] \
 -group [get_clocks { main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[1].gpll~PLL_OUTPUT_COUNTER|divclk } ] \
 -group [get_clocks { pcie_refclk_i \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_cavhip.arriav_hd_altpe2_hip_top|coreclkout \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|av_xcvr_tx_pll_inst|pll[0].pll.cmu_pll.pll_mux.pll_refclk_select_mux|clkout \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|av_xcvr_tx_pll_inst|pll[0].pll.cmu_pll.tx_pll|clkcdr \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pcs|ch[0].inst_av_pcs_ch|inst_av_hssi_8g_rx_pcs|wys|txpmaclk \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pcs|ch[0].inst_av_pcs_ch|inst_av_hssi_8g_tx_pcs|wys|txpmalocalclk \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pcs|ch[1].inst_av_pcs_ch|inst_av_hssi_8g_rx_pcs|wys|txpmaclk \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pcs|ch[1].inst_av_pcs_ch|inst_av_hssi_8g_tx_pcs|wys|txpmalocalclk \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pcs|ch[2].inst_av_pcs_ch|inst_av_hssi_8g_rx_pcs|wys|txpmaclk \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pcs|ch[2].inst_av_pcs_ch|inst_av_hssi_8g_tx_pcs|wys|txpmalocalclk \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pcs|ch[3].inst_av_pcs_ch|inst_av_hssi_8g_rx_pcs|wys|txpmaclk \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pcs|ch[3].inst_av_pcs_ch|inst_av_hssi_8g_tx_pcs|wys|txpmalocalclk \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_rx_pma|rx_pmas[0].rx_pma.cdr_refclk_mux0|clkout \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_rx_pma|rx_pmas[0].rx_pma.rx_cdr|clk90bdes \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_rx_pma|rx_pmas[0].rx_pma.rx_pma_deser|clk90b \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_rx_pma|rx_pmas[1].rx_pma.cdr_refclk_mux0|clkout \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_rx_pma|rx_pmas[1].rx_pma.rx_cdr|clk90bdes \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_rx_pma|rx_pmas[1].rx_pma.rx_pma_deser|clk90b \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_rx_pma|rx_pmas[2].rx_pma.cdr_refclk_mux0|clkout \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_rx_pma|rx_pmas[2].rx_pma.rx_cdr|clk90bdes \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_rx_pma|rx_pmas[2].rx_pma.rx_pma_deser|clk90b \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_rx_pma|rx_pmas[3].rx_pma.cdr_refclk_mux0|clkout \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_rx_pma|rx_pmas[3].rx_pma.rx_cdr|clk90bdes \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_rx_pma|rx_pmas[3].rx_pma.rx_pma_deser|clk90b \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[0].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|cpulse \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[0].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|hfclkp \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[0].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|lfclkp \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[0].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|pclk[0] \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[0].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|pclk[1] \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[0].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|pclk[2] \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[1].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|cpulse \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[1].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|cpulseout \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[1].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|hfclkp \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[1].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|lfclkp \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[1].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|lfclkpout \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[1].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|pclk[0] \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[1].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|pclk[1] \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[1].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|pclk[2] \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[1].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|pclkout \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[2].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|cpulse \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[2].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|hfclkp \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[2].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|lfclkp \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[2].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|pclk[0] \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[2].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|pclk[1] \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[2].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|pclk[2] \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[3].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|cpulse \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[3].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|hfclkp \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[3].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|lfclkp \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[3].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|pclk[0] \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[3].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|pclk[1] \
                      main|\pcie_y:pcie|pcie_phy|\arria5:hip|arria5_pcie_hip_inst|altpcie_av_hip_128bit_atom|g_pcie_xcvr.av_xcvr_pipe_native_hip|inst_av_xcvr_native|inst_av_pma|av_tx_pma|tx_pma_insts[3].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|pclk[2] } ] \
 -group [get_clocks { main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[0].output_counter|divclk \
                      main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[3].output_counter|divclk \
                      main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[4].output_counter|divclk } ] \
 -group [get_clocks { main|\phy_a5:phy|\gen_arria5_phy8:U_The_PHY|arria5_phy8_inst|A5|transceiver_core|gen.av_xcvr_native_insts[0].gen_bonded_group.av_xcvr_native_inst|inst_av_pcs|ch[0].inst_av_pcs_ch|inst_av_hssi_8g_rx_pcs|wys|rcvdclkpma \
                      main|\phy_a5:phy|\gen_arria5_phy8:U_The_PHY|arria5_phy8_inst|A5|transceiver_core|gen.av_xcvr_native_insts[0].gen_bonded_group.av_xcvr_native_inst|inst_av_pcs|ch[0].inst_av_pcs_ch|inst_av_hssi_8g_tx_pcs|wys|txpmalocalclk \
                      main|\phy_a5:phy|\gen_arria5_phy8:U_The_PHY|arria5_phy8_inst|A5|transceiver_core|gen.av_xcvr_native_insts[0].gen_bonded_group.av_xcvr_native_inst|inst_av_pma|av_rx_pma|rx_pmas[0].rx_pma.cdr_refclk_mux0|clkout \
                      main|\phy_a5:phy|\gen_arria5_phy8:U_The_PHY|arria5_phy8_inst|A5|transceiver_core|gen.av_xcvr_native_insts[0].gen_bonded_group.av_xcvr_native_inst|inst_av_pma|av_rx_pma|rx_pmas[0].rx_pma.rx_cdr|clk90bdes \
                      main|\phy_a5:phy|\gen_arria5_phy8:U_The_PHY|arria5_phy8_inst|A5|transceiver_core|gen.av_xcvr_native_insts[0].gen_bonded_group.av_xcvr_native_inst|inst_av_pma|av_rx_pma|rx_pmas[0].rx_pma.rx_pma_deser|clk90b } ] \
 -group [get_clocks { main|\phy_a5:phy|\gen_arria5_phy8:U_The_PHY|arria5_phy8_inst|A5|transceiver_core|gen.av_xcvr_native_insts[0].gen_bonded_group.av_xcvr_native_inst|inst_av_pma|av_tx_pma|tx_pma_insts[0].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|cpulse \
                      main|\phy_a5:phy|\gen_arria5_phy8:U_The_PHY|arria5_phy8_inst|A5|transceiver_core|gen.av_xcvr_native_insts[0].gen_bonded_group.av_xcvr_native_inst|inst_av_pma|av_tx_pma|tx_pma_insts[0].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|hfclkp \
                      main|\phy_a5:phy|\gen_arria5_phy8:U_The_PHY|arria5_phy8_inst|A5|transceiver_core|gen.av_xcvr_native_insts[0].gen_bonded_group.av_xcvr_native_inst|inst_av_pma|av_tx_pma|tx_pma_insts[0].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|lfclkp \
                      main|\phy_a5:phy|\gen_arria5_phy8:U_The_PHY|arria5_phy8_inst|A5|transceiver_core|gen.av_xcvr_native_insts[0].gen_bonded_group.av_xcvr_native_inst|inst_av_pma|av_tx_pma|tx_pma_insts[0].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|pclk[0] \
                      main|\phy_a5:phy|\gen_arria5_phy8:U_The_PHY|arria5_phy8_inst|A5|transceiver_core|gen.av_xcvr_native_insts[0].gen_bonded_group.av_xcvr_native_inst|inst_av_pma|av_tx_pma|tx_pma_insts[0].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|pclk[1] \
                      main|\phy_a5:phy|\gen_arria5_phy8:U_The_PHY|arria5_phy8_inst|A5|transceiver_core|gen.av_xcvr_native_insts[0].gen_bonded_group.av_xcvr_native_inst|inst_av_pma|av_tx_pma|tx_pma_insts[0].av_tx_pma_ch_inst|tx_pma_ch.tx_cgb|pclk[2] \
                      main|\phy_a5:phy|\gen_arria5_phy8:U_The_PHY|arria5_phy8_inst|A5|transceiver_core|gen.av_xcvr_native_insts[0].gen_tx_plls.gen_tx_plls.tx_plls|pll[0].pll.cmu_pll.pll_mux.pll_refclk_select_mux|clkout \
                      main|\phy_a5:phy|\gen_arria5_phy8:U_The_PHY|arria5_phy8_inst|A5|transceiver_core|gen.av_xcvr_native_insts[0].gen_tx_plls.gen_tx_plls.tx_plls|pll[0].pll.cmu_pll.tx_pll|clkcdr } ]
                      

#######################################################################################################################

# PCI Clock Settings
##############################
create_clock -period 30 -name pmc_pci_clk_in [get_ports {pmc_clk_i}]
set_false_path -from [get_ports pmc_rst_i] -to *

# Assigments for node pmc_devsel_io 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_devsel_io]
set_input_delay -clock pmc_pci_clk_in -max 23.0 [get_ports pmc_devsel_io]
set_input_delay -clock pmc_pci_clk_in -min 0.0 [get_ports pmc_devsel_io]

# Assigments for node pmc_frame_io 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_frame_io]
set_input_delay -clock pmc_pci_clk_in -max 23.0 [get_ports pmc_frame_io]
set_input_delay -clock pmc_pci_clk_in -min 0.0 [get_ports pmc_frame_io]

# Assigments for node pmc_irdy_io 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_irdy_io]
set_input_delay -clock pmc_pci_clk_in -max 23.0 [get_ports pmc_irdy_io]
set_input_delay -clock pmc_pci_clk_in -min 0.0 [get_ports pmc_irdy_io]

# Assigments for node pmc_par_io 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_par_io]
set_input_delay -clock pmc_pci_clk_in -max 23.0 [get_ports pmc_par_io]
set_input_delay -clock pmc_pci_clk_in -min 0.0 [get_ports pmc_par_io]

# Assigments for node pmc_perr_io 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_perr_io]
set_input_delay -clock pmc_pci_clk_in -max 23.0 [get_ports pmc_perr_io]
set_input_delay -clock pmc_pci_clk_in -min 0.0 [get_ports pmc_perr_io]

# Assigments for node pmc_stop_io 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_stop_io]
set_input_delay -clock pmc_pci_clk_in -max 23.0 [get_ports pmc_stop_io]
set_input_delay -clock pmc_pci_clk_in -min 0.0 [get_ports pmc_stop_io]

# Assigments for node pmc_trdy_io 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_trdy_io]
set_input_delay -clock pmc_pci_clk_in -max 23.0 [get_ports pmc_trdy_io]
set_input_delay -clock pmc_pci_clk_in -min 0.0 [get_ports pmc_trdy_io]

# Assigments for node pmc_ad_io 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_ad_io[*]]
set_input_delay -clock pmc_pci_clk_in -max 23.0 [get_ports pmc_ad_io[*]]
set_input_delay -clock pmc_pci_clk_in -min 0.0 [get_ports pmc_ad_io[*]]

# Assigments for node pmc_c_be_io 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_c_be_io[*]]
set_input_delay -clock pmc_pci_clk_in -max 23.0 [get_ports pmc_c_be_io[*]]
set_input_delay -clock pmc_pci_clk_in -min 0.0 [get_ports pmc_c_be_io[*]]

# Assigments for node pmc_idsel_i 
###############################
set_input_delay -clock pmc_pci_clk_in -max 23.0 [get_ports pmc_idsel_i]
set_input_delay -clock pmc_pci_clk_in -min 0.0 [get_ports pmc_idsel_i]

# Assigments for node pmc_gnt_i 
###############################
set_input_delay -clock pmc_pci_clk_in -max 20.0 [get_ports pmc_gnt_i]
set_input_delay -clock pmc_pci_clk_in -min 0.0 [get_ports pmc_gnt_i]

# Assigments for node pmc_intX_o 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_inta_o]
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_intb_o]
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_intc_o]
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_intd_o]

# Assigments for node pmc_serr_io 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_serr_io]

# Assigments for node pmc_req_o 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_req_o]

# Assigments for node pmc_busmode_io 
###############################
set_output_delay -clock pmc_pci_clk_in 19.0 [get_ports pmc_busmode_io[*]]

####################################################
# Assignments to meet PCI "Float to Active" Delay
####################################################
set_max_delay 47.0 -from [get_registers *wb_pmc_host_bridge*pci_io_mux*ad_iob*en_out    ] -to [get_ports pmc_ad_io[*]  ]

set_max_delay 47.0 -from [get_registers *wb_pmc_host_bridge*pci_io_mux*par_iob*en_out   ] -to [get_ports pmc_par_io    ]
set_max_delay 47.0 -from [get_registers *wb_pmc_host_bridge*pci_io_mux*devsel_iob*en_out] -to [get_ports pmc_devsel_io ]
set_max_delay 47.0 -from [get_registers *wb_pmc_host_bridge*pci_io_mux*stop_iob*en_out  ] -to [get_ports pmc_stop_io   ]
set_max_delay 47.0 -from [get_registers *wb_pmc_host_bridge*pci_io_mux*trdy_iob*en_out  ] -to [get_ports pmc_trdy_io   ]
set_max_delay 47.0 -from [get_registers *wb_pmc_host_bridge*pci_io_mux*perr_iob*en_out  ] -to [get_ports pmc_perr_io   ]

set_max_delay 47.0 -from [get_registers *wb_pmc_host_bridge*pci_io_mux*cbe_iob*en_out   ] -to [get_ports pmc_c_be_io[*]]
set_max_delay 47.0 -from [get_registers *wb_pmc_host_bridge*pci_io_mux*frame_iob*en_out ] -to [get_ports pmc_frame_io  ]
set_max_delay 47.0 -from [get_registers *wb_pmc_host_bridge*pci_io_mux*irdy_iob*en_out  ] -to [get_ports pmc_irdy_io   ]


