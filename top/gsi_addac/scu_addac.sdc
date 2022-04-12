create_clock -name clk_local   -period   125MHz   [get_ports     CLK_FPGA]
create_clock -name scubus_clk  -period   12.5MHz  [get_ports     A_SysClock]

#derive_pll_clocks 
create_generated_clock -name {addac_clk_sw|local_clk|altpll_component|auto_generated|pll1|clk[0]} -source {addac_clk_sw|local_clk|altpll_component|auto_generated|pll1|inclk[0]} -divide_by 10 -multiply_by 1 -duty_cycle 50.00 { addac_clk_sw|local_clk|altpll_component|auto_generated|pll1|clk[0] }

create_generated_clock -name {addac_clk_sw|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[0]} -source {addac_clk_sw|sys_or_local_pll|altpll_component|auto_generated|pll1|inclk[0]} -divide_by 1 -multiply_by 10 -duty_cycle 50.00 { addac_clk_sw|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[0] }
create_generated_clock -name {addac_clk_sw|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[1]} -source {addac_clk_sw|sys_or_local_pll|altpll_component|auto_generated|pll1|inclk[0]} -divide_by 5 -multiply_by 4 -duty_cycle 50.00 { addac_clk_sw|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[1] }
create_generated_clock -name {addac_clk_sw|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[2]} -source {addac_clk_sw|sys_or_local_pll|altpll_component|auto_generated|pll1|inclk[0]} -divide_by 1 -multiply_by 4 -duty_cycle 50.00 { addac_clk_sw|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[2] }
create_generated_clock -name {addac_clk_sw|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[3]} -source {addac_clk_sw|sys_or_local_pll|altpll_component|auto_generated|pll1|inclk[0]} -divide_by 1 -multiply_by 20 -duty_cycle 50.00 { addac_clk_sw|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[3] }

derive_clock_uncertainty

set_clock_groups -asynchronous                                                             \
 -group { altera_reserved_tck                           }                                  \
 -group { clk_local                                                                    \
          addac_clk_sw|local_clk|altpll_component|auto_generated|pll1|clk[0] }         \
 -group {addac_clk_sw|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[0]}    \
 -group {addac_clk_sw|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[1]}    \
 -group {addac_clk_sw|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[2]}
