create_clock -name clk_local   -period   20MHz   [get_ports     CLK_20MHz_D]
create_clock -name scubus_clk  -period   12.5MHz  [get_ports     A_SysClock]

create_generated_clock -name {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[0]} -source {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|inclk[0]} -divide_by 1 -multiply_by 10 -duty_cycle 50.00 { diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[0] }
create_generated_clock -name {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[1]} -source {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|inclk[0]} -divide_by 5 -multiply_by 4 -duty_cycle 50.00 { diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[1] }
#create_generated_clock -name {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[2]} -source {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|inclk[0]} -divide_by 1 -multiply_by 2 -duty_cycle 50.00 { diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[2] }
create_generated_clock -name {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[2]} -source {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|inclk[0]} -divide_by 1 -multiply_by 4 -duty_cycle 50.00 { diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[2] }
create_generated_clock -name {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[3]} -source {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|inclk[0]} -divide_by 1 -multiply_by 20 -duty_cycle 50.00 { diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[3] }

set_multicycle_path -hold 1 \
  -from [get_clocks {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[0]}] \
  -to [get_registers {Beam_Loss_check:BLM_Module|VALUE_IN[60]}]
 
set_multicycle_path -hold 1 \
  -from [get_clocks {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[0]}] \
  -to [get_registers {Beam_Loss_check:BLM_Module|VALUE_IN[62]}] 

set_multicycle_path 2 -setup \
  -from [get_clocks {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[0]}] \
  -to [get_registers {slave_clk_switch:diob_clk_switch|f_local_12p5_mhz_sync[0]}]   

set_multicycle_path -hold 1 \
  -from [get_clocks {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[0]}] \
  -to [get_registers {slave_clk_switch:diob_clk_switch|f_local_12p5_mhz_sync[0]}]   

derive_clock_uncertainty

set_clock_groups -asynchronous                                                            \
 -group { altera_reserved_tck }                                                           \
 -group {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[0]}    \
 -group {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[1]}    \
 -group {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[2]}    \
 -group {diob_clk_switch|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[3]}   
