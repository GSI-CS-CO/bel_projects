derive_pll_clocks -create_base_clocks
create_clock -period  33Mhz -name LPC_FPGA_CLK  [get_ports {LPC_FPGA_CLK}]
create_clock -period 100Mhz -name pcie_refclk_i [get_ports {pcie_refclk_i}]
derive_clock_uncertainty

# cut the clock domains from each other
set_clock_groups -asynchronous                \
 -group { altera_reserved_tck               } \
 -group { clk_20m_vcxo_i    dmtd_inst|*     } \
 -group { clk_125m_pllref_i ref_inst|*      } \
 -group { clk_125m_local_i  sys_inst|*      } \
 -group { pcie_refclk_i     PCIe|*          } \
 -group { wr_gxb_phy*                       } \
 -group { LPC_FPGA_CLK                      }

# PCIe constraints; SERDES Digital Reset inputs are asynchronous:
set_false_path -to {*|altera_pcie_serdes:serdes|*|tx_digitalreset_reg0c[0]}
set_false_path -to {*|altera_pcie_serdes:serdes|*|rx_digitalreset_reg0c[0]}
