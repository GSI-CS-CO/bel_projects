derive_pll_clocks -create_base_clocks

derive_clock_uncertainty

create_clock -period 33Mhz -name LPC_FPGA_CLK [get_ports {LPC_FPGA_CLK}]