derive_pll_clocks -create_base_clocks
create_clock -period 100Mhz -name pcie_refclk_i  [get_ports {pcie_refclk_i}]
create_clock -period 125Mhz -name sfp2_ref_clk_i [get_ports {sfp2_ref_clk_i}]
#create_clock -period 125Mhz -name osc_rfck_p [get_ports {osc_rfck_p}]
derive_clock_uncertainty

# Cut the clock domains from each other
set_clock_groups -asynchronous                \
 -group { altera_reserved_tck               } \
 -group { clk_20m_vcxo_i    dmtd_inst|*     } \
 -group { clk_125m_local_i  sys_inst|*      } \
 -group { clk_125m_pllref_i ref_inst|*        \
          wr_arria5_phy*.cmu_pll.*            \
          wr_arria5_phy*|av_tx_pma|*          \
          wr_arria5_phy*|inst_av_pcs|*|tx*  } \
 -group { sfp2_ref_clk_i                      \
          wr_arria5_phy*.cdr_refclk*        } \
 -group { wr_arria5_phy*|clk90bdes            \
          wr_arria5_phy*|clk90b               \
          wr_arria5_phy*|rcvdclkpma         } \
 -group { pcie_refclk_i                       \
          PCIe*.cdr_refclk*                   \
          PCIe*.cmu_pll.*                     \
          PCIe*|av_tx_pma|*                   \
          PCIe*|inst_av_pcs|*|tx*           } \
 -group { PCIe*|rx_pmas[0]*|clk90bdes         \
          PCIe*|rx_pmas[0]*|clk90b          } \
 -group { PCIe*|rx_pmas[1]*|clk90bdes         \
          PCIe*|rx_pmas[1]*|clk90b          } \
 -group { PCIe*|rx_pmas[2]*|clk90bdes         \
          PCIe*|rx_pmas[2]*|clk90b          } \
 -group { PCIe*|rx_pmas[3]*|clk90bdes         \
          PCIe*|rx_pmas[3]*|clk90b          } \
 -group { PCIe*|coreclkout                  }
