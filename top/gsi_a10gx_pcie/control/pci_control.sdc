derive_pll_clocks -create_base_clocks
derive_clock_uncertainty

# Cut the clock domains from each other
set_clock_groups -asynchronous \
 -group [get_clocks { altera_reserved_tck }] \
 -group [get_clocks { clk_20m_vcxo_i }] \
 -group [get_clocks { main|\dmtd_a10:dmtd_inst|iopll_0|outclk0 }] \
 -group [get_clocks { clk_125m_local_i }] \
 -group [get_clocks { main|\sys_a10:sys_inst|iopll_0|outclk0 \
                      main|\sys_a10:sys_inst|iopll_0|outclk1 \
		                main|\sys_a10:sys_inst|iopll_0|outclk2 }] \
 -group [get_clocks { clk_125m_pllref_i }] \
 -group [get_clocks { main|\ref_a10:ref_inst|iopll_0|outclk0 \
 			             main|\ref_a10:ref_inst|iopll_0|outclk1 \
 			             main|\ref_a10:ref_inst|iopll_0|outclk2 }] \
 -group [get_clocks { main|\ref_a10:ref_inst|iopll_0|outclk3 \
 			             main|\ref_a10:ref_inst|iopll_0|outclk4 }] \
 -group [get_clocks { clk_125m_sfpref_i }] \
 -group [get_clocks { main|\phy_a10_e3p1:phy|inst_phy|xcvr_native_a10_0|rx_pma_clk \
 		                main|\phy_a10_e3p1:phy|inst_phy|xcvr_native_a10_0|tx_coreclkin \
 			             main|\phy_a10_e3p1:phy|inst_phy|xcvr_native_a10_0|tx_pma_clk }] \
 -group [get_clocks { pcie_refclk_i \
 		             	 main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|coreclkout \
 			             main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts{[0]}|pma_hclk_by2 main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts{[0]}|rx_clk \
			             main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts{[0]}|rx_clkout main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts{[0]}|rx_fref \
			             main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts{[0]}|rx_pma_clk main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts{[0]}|tx_clk \
			             main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts{[1]}|pma_hclk_by2 main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts{[1]}|rx_clk \
			             main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts{[1]}|rx_clkout main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts{[1]}|rx_fref \
			             main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts{[1]}|rx_pma_clk main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts{[1]}|tx_clk \
			             main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts{[2]}|pma_hclk_by2 main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts{[2]}|rx_clk \
			             main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts{[2]}|rx_clkout main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts{[2]}|rx_fref \
			             main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts{[2]}|rx_pma_clk main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts{[2]}|tx_clk \
			             main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts{[3]}|pma_hclk_by2 main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts{[3]}|rx_clk \
			             main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts{[3]}|rx_clkout main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts{[3]}|rx_fref \
			             main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts{[3]}|rx_pma_clk main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|g_xcvr_native_insts{[3]}|tx_clk \
			             main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|hip_cmn_clk{[0]} \
			             main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|pld_clk main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|pll_pcie_clk \
			             main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|tx_clkout main|\phy_a10_e3p1:phy|inst_phy|xcvr_native_a10_0|rx_coreclkin }]

# cut phy and dmtd
set_false_path -from [get_clocks {main|\phy_a10_e3p1:det_phy|inst_phy|xcvr_native_a10_0|rx_pma_clk}] -to [get_clocks {main|\dmtd_a10:dmtd_inst|iopll_0|outclk0}]
set_false_path -from [get_clocks {main|\phy_a10_e3p1:det_phy|inst_phy|xcvr_native_a10_0|tx_pma_clk}] -to [get_clocks {main|\dmtd_a10:dmtd_inst|iopll_0|outclk0}]
set_false_path -from [get_clocks {main|\phy_a10_e3p1:det_phy|inst_phy|xcvr_native_a10_0|tx_coreclkin}] -to [get_clocks {main|\dmtd_a10:dmtd_inst|iopll_0|outclk0}]
set_false_path -from [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_clkout}] -to [get_clocks {main|\dmtd_a10:dmtd_inst|iopll_0|outclk0}]
set_false_path -from [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_clkout}] -to [get_clocks {main|\dmtd_a10:dmtd_inst|iopll_0|outclk0}]
# cut ref and sys
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk0}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk0}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk0}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk1}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk0}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk2}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk1}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk0}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk1}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk1}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk1}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk2}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk2}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk0}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk2}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk1}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk2}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk2}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk3}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk0}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk3}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk1}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk3}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk2}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk4}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk0}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk4}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk1}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk4}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk2}]
# cut sys and phy
set_false_path -from [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_pma_clk}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk0}]
set_false_path -from [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_pma_clk}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk1}]
set_false_path -from [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_pma_clk}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk2}]
set_false_path -from [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_pma_clk}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk0}]
set_false_path -from [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_pma_clk}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk1}]
set_false_path -from [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_pma_clk}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk2}]
set_false_path -from [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_coreclkin}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk0}]
set_false_path -from [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_coreclkin}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk1}]
set_false_path -from [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_coreclkin}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk2}]
set_false_path -from [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_clkout}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk0}]
set_false_path -from [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_clkout}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk1}]
set_false_path -from [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_clkout}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk2}]
set_false_path -from [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk0}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_clkout}]
set_false_path -from [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk1}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_clkout}]
set_false_path -from [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk2}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_clkout}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk0}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_clkout}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk1}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_clkout}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk2}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_clkout}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk3}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_clkout}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk4}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_clkout}]
# cut phy and sys
set_false_path -from [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk0}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_pma_clk}]
set_false_path -from [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk1}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_pma_clk}]
set_false_path -from [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk2}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_pma_clk}]
set_false_path -from [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk0}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_pma_clk}]
set_false_path -from [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk1}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_pma_clk}]
set_false_path -from [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk2}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_pma_clk}]
set_false_path -from [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk0}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_coreclkin}]
set_false_path -from [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk1}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_coreclkin}]
set_false_path -from [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk2}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_coreclkin}]
# cut phy and ref
set_false_path -from [get_clocks {main|\ref_a10:sys_inst|iopll_0|outclk0}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_pma_clk}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk1}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_pma_clk}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk2}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_pma_clk}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk4}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_pma_clk}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk5}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_pma_clk}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk0}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_pma_clk}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk1}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_pma_clk}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk2}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_pma_clk}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk3}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_pma_clk}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk4}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_pma_clk}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk0}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_coreclkin}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk1}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_coreclkin}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk2}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_coreclkin}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk3}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_coreclkin}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk4}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_coreclkin}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk0}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_clkout}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk1}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_clkout}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk2}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_clkout}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk3}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_clkout}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk4}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_clkout}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk0}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_clkout}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk1}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_clkout}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk2}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_clkout}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk3}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_clkout}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk4}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_clkout}]
set_false_path -from [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_clkout}] -to [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk0}]
set_false_path -from [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_clkout}] -to [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk0}]
# cut sys and pcie
set_false_path -from [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk0}] -to [get_clocks {main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|coreclkout}]
set_false_path -from [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk1}] -to [get_clocks {main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|coreclkout}]
set_false_path -from [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk2}] -to [get_clocks {main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|coreclkout}]
# cut ref and pcie
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk0}] -to [get_clocks {main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|coreclkout}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk1}] -to [get_clocks {main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|coreclkout}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk2}] -to [get_clocks {main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|coreclkout}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk3}] -to [get_clocks {main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|coreclkout}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk4}] -to [get_clocks {main|\pcie_y:pcie|pcie_phy|\arria10gx_e3p1:hip|pcie_a10_hip_0|coreclkout}]
# cut ref and butis 200MHz
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk0}] -to [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk1}]
set_false_path -from [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk1}] -to [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk0}]
# input clocks
set_false_path -from [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_clkout}] -to [get_clocks {clk_125m_sfpref_i}]
set_false_path -from [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_clkout}] -to [get_clocks {clk_125m_sfpref_i}]
# cut phy reconf and sys clock
set_false_path -from [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|avmmclk}] -to [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk0}]
set_false_path -from [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk0}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|avmmclk}]
# sfp clk and phy clk out
set_false_path -from [get_clocks {clk_125m_sfpref_i}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_clkout}]
set_false_path -from [get_clocks {clk_125m_sfpref_i}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_clkout}]
# phy reconf and phy clk out
set_false_path -from [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|avmmclk}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|rx_clkout}]
set_false_path -from [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|avmmclk}] -to [get_clocks {main|\phy_a10_e3p1:phy|\det_phy:inst_phy|xcvr_native_a10_0|tx_clkout}]
