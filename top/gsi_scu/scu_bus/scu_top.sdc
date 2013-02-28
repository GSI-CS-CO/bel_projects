derive_pll_clocks -create_base_clocks
create_clock -period 33Mhz -name LPC_FPGA_CLK [get_ports {LPC_FPGA_CLK}]
create_clock -period 100Mhz -name pcie_refclk_i [get_ports {pcie_refclk_i}]
derive_clock_uncertainty

set_false_path -to {*|altera_pcie_serdes:serdes|*|tx_digitalreset_reg0c[0]}
set_false_path -to {*|altera_pcie_serdes:serdes|*|rx_digitalreset_reg0c[0]}
