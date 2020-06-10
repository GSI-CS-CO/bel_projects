# Timing constraints
derive_pll_clocks -create_base_clocks
derive_clock_uncertainty

# Named clocks
set clk_ref0_125m_ref_clk              [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk2}]
set clk_ref1_200m_butis_clk            [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk3}]
set clk_ref2_25m_phase_butis_clk       [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk4}]
set clk_ref3_1000m_clk_lvds            [get_clocks {main|\ref_a10:ref_inst|iopll_0|altera_iopll_i|twentynm_pll|lvds_clk[0]}]
set clk_ref4_125m_clk_lvds_enable_18dc [get_clocks {main|\ref_a10:ref_inst|iopll_0|altera_iopll_i|twentynm_pll|iopll_inst|loaden[0]}]
set clk_sys0_62_5_sys_clk              [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk0}]

# Special device input clocks
create_clock -period 125Mhz -name clk_125m_tcb_local_in  [get_ports {clk_125m_tcb_local_in}]
create_clock -period 125Mhz -name clk_125m_tcb_pllref_in [get_ports {clk_125m_tcb_pllref_in}]
create_clock -period 125Mhz -name clk_125m_tcb_sfpref_in [get_ports {clk_125m_tcb_sfpref_in}]
create_clock -period 20Mhz  -name clk_20m_vcxo_in        [get_ports {clk_20m_vcxo_i}]

# Cut the clock domains from each other
set_clock_groups -asynchronous \
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
-group [get_clocks {main|\phy_a10:phy|\det_phy:scu4_phy:inst_phy|xcvr_native_a10_0|avmmclk \
                    main|\phy_a10:phy|\det_phy:scu4_phy:inst_phy|xcvr_native_a10_0|rx_clkout \
                    main|\phy_a10:phy|\det_phy:scu4_phy:inst_phy|xcvr_native_a10_0|rx_pma_clk \
                    main|\phy_a10:phy|\det_phy:scu4_phy:inst_phy|xcvr_native_a10_0|tx_pma_clk}] \
-group [get_clocks {main|\sys_a10:sys_inst|iopll_0|outclk0}] \
-group [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk2 \
                    main|\ref_a10:ref_inst|iopll_0|altera_iopll_i|twentynm_pll|lvds_clk[0] \
                    main|\ref_a10:ref_inst|iopll_0|altera_iopll_i|twentynm_pll|iopll_inst|loaden[0]}] \
-group [get_clocks {main|\ref_a10:ref_inst|iopll_0|outclk3 \
                    main|\ref_a10:ref_inst|iopll_0|outclk4}] \








