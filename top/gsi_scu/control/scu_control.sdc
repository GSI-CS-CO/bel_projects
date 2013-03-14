derive_pll_clocks -create_base_clocks
create_clock -period  33Mhz -name LPC_FPGA_CLK   [get_ports {LPC_FPGA_CLK}]
create_clock -period 100Mhz -name pcie_refclk_i  [get_ports {pcie_refclk_i}]
create_clock -period 125Mhz -name sfp2_ref_clk_i [get_ports {sfp2_ref_clk_i}]
derive_clock_uncertainty

# Cut the clock domains from each other
# Note: ref_inst|* and wr_gxb*|transmit_pcs0 are actually the same clock
# However, we cut them to enforce a synchronizer between them in the GXB.
set_clock_groups -asynchronous                \
 -group { altera_reserved_tck               } \
 -group { LPC_FPGA_CLK                      } \
 -group { clk_20m_vcxo_i    dmtd_inst|*     } \
 -group { clk_125m_local_i  sys_inst|*      } \
 -group { clk_125m_pllref_i ref_inst|*      } \
 -group { wr_gxb*|tx_pll0|*                   \
          wr_gxb*|ch_clk_div0|*               \
          wr_gxb*|transmit_pma0|*             \
          wr_gxb*|transmit_pcs0|*           } \
 -group { sfp2_ref_clk_i                      \
          wr_gxb*|rx_cdr_pll0|*               \
          wr_gxb*|receive_pma0|*              \
          wr_gxb*|receive_pcs0|*            } \
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
