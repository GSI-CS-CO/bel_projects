create_clock -name pin_clk_local     -period   20MHz  [get_ports     CLK_FPGA]

derive_pll_clocks -create_base_clocks
derive_clock_uncertainty

set_clock_groups -asynchronous                           \
 -group { altera_reserved_tck                           }\
 -group { pin_clk_local                                 }\
 -group {addac_clk_switch|local_clk|altpll_component|auto_generated|pll1|clk[0]}\
 -group {addac_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[0]}\
 -group {addac_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[0]~1}\
 -group {addac_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[1]}\
 -group {addac_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[1]~1}