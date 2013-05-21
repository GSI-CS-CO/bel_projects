derive_pll_clocks -create_base_clocks
create_clock -period 100Mhz -name pcie_refclk_i  [get_ports {pcie_refclk_i}]
create_clock -period 125Mhz -name sfp2_ref_clk_i [get_ports {sfp2_ref_clk_i}]
derive_clock_uncertainty

# Cut the clock domains from each other
set_clock_groups -asynchronous                \
 -group { altera_reserved_tck               } \
 -group { clk_20m_vcxo_i    dmtd_inst|*     } \
 -group { clk_125m_local_i  sys_inst|*      } \
 -group { clk_125m_pllref_i ref_inst|*        \
          wr_arria5_phy*cmu_pll.tx_pll|*      \
          wr_arria5_phy*|av_tx_pma|*          \
          wr_arria5_phy*|txpmalocalclk      } \
 -group { sfp2_ref_clk_i                      \
          wr_arria5_phy*|av_rx_pma|*          \
          wr_arria5_phy*|rcvdclkpma         } \
 -group { pcie_refclk_i                       \
          PCIe*|tx_pll0|*                     \
          PCIe*|central_clk_div0|*            \
          PCIe*|pllfixedclk                   \
          PCIe*|coreclkout                  } \
 -group { PCIe*|rx_cdr_pll0|*                 \
          PCIe*|receive_pma0|*              } \
 -group { PCIe*|rx_cdr_pll1|*                 \
          PCIe*|receive_pma1|*              } \
 -group { PCIe*|rx_cdr_pll2|*                 \
          PCIe*|receive_pma2|*              } \
 -group { PCIe*|rx_cdr_pll3|*                 \
          PCIe*|receive_pma3|*              }

# *|av? 
# 125  MHz cdr_refclk_mux0|clkout    sfp2_ref_clk_i
# *|rx....
# 625  MHz clk90bdes                 <--- base
# 125  MHz clk90b                    clk90bdes
# *|rcvdclkpma
# 125  MHz rcvdclkpma                90b

# *cmu_pll.tx_pll|*
# 625  MHz tx_pll|clkcdr             ref_inst
# *|av_tx_pma|*
# 125  MHz tx_cgb|cpulse             clkcdr
# 625  MHz tx_cgb|hfclkp             clkcdr
# 125  MHz tx_cgb|lfclkp             clkcdr
# 62.50MHz tx_cgb|pclk0              clkcdr
# 31.25MHz tx_cgb|pclk1              clkcdr
# 15.63MHz tx_cgb|pclk1              clkcdr
# *|txpmalocalclk
# 125  MHz txpmalocalclk             lfclp
