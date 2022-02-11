#
#
create_clock -name {master_clk} -period 10.0 [get_ports {clk_base_i}] -add

derive_clock_uncertainty
derive_pll_clocks