create_clock -period  33Mhz -name LPC_FPGA_CLK   [get_ports {LPC_FPGA_CLK}]
create_clock -period 100Mhz -name pcie_refclk_i  [get_ports {pcie_refclk_i}]
create_clock -period 125Mhz -name sfp2_ref_clk_i [get_ports {sfp2_ref_clk_i}]
create_clock -period 1Mhz   -name mil_dsc        [get_ports {IO_2_5V[4]}]
create_clock -period 1Mhz   -name mil_esc        [get_ports {IO_2_5V[9]}]

derive_pll_clocks -create_base_clocks
derive_clock_uncertainty

# Cut the clock domains from each other
set_clock_groups -asynchronous                           \
 -group { altera_reserved_tck                          } \
 -group { LPC_FPGA_CLK                                 } \
 -group { clk_20m_vcxo_i    main|\dmtd_a2:dmtd_inst|*  } \
 -group { clk_125m_local_i  main|\sys_a2:sys_inst|*    } \
 -group { clk_125m_pllref_i main|\ref_a2:ref_inst|*      \
          main|\phy_a2:phy|*|tx_pll0|*                   \
          main|\phy_a2:phy|*|ch_clk_div0|*               \
          main|\phy_a2:phy|*|transmit_pma0|*             \
          main|\phy_a2:phy|*|transmit_pcs0|*           } \
 -group { sfp2_ref_clk_i                                 \
          main|\phy_a2:phy|*|rx_cdr_pll0|*               \
          main|\phy_a2:phy|*|receive_pma0|*              \
          main|\phy_a2:phy|*|receive_pcs0|*            } \
 -group { pcie_refclk_i                                  \
          main|\pcie_y:pcie|*|tx_pll0|*                  \
          main|\pcie_y:pcie|*|central_clk_div0|*         \
          main|\pcie_y:pcie|*|pllfixedclk                \
          main|\pcie_y:pcie|*|coreclkout               } \
 -group { main|\pcie_y:pcie|*|rx_cdr_pll0|*              \
          main|\pcie_y:pcie|*|receive_pma0|*           } \
 -group { main|\pcie_y:pcie|*|rx_cdr_pll1|*              \
          main|\pcie_y:pcie|*|receive_pma1|*           } \
 -group { main|\pcie_y:pcie|*|rx_cdr_pll2|*              \
          main|\pcie_y:pcie|*|receive_pma2|*           } \
 -group { main|\pcie_y:pcie|*|rx_cdr_pll3|*              \
          main|\pcie_y:pcie|*|receive_pma3|*           } \
 -group { mil_dsc                                      } \
 -group { mil_esc                                      }

# cut: wb sys <=> wb flash   (different frequencies and using xwb_clock_crossing)
set_false_path -from [get_clocks {main|\sys_a2:sys_inst|*|clk[0]}] -to [get_clocks {main|\sys_a2:sys_inst|*|clk[1]}]
set_false_path -from [get_clocks {main|\sys_a2:sys_inst|*|clk[1]}] -to [get_clocks {main|\sys_a2:sys_inst|*|clk[0]}]
# cut: wr-ref <=> butis
set_false_path -from [get_clocks {main|\ref_a2:ref_inst|*|clk[0]}] -to [get_clocks {main|\ref_a2:ref_inst|*|clk[1]}]
set_false_path -from [get_clocks {main|\ref_a2:ref_inst|*|clk[1]}] -to [get_clocks {main|\ref_a2:ref_inst|*|clk[0]}]
