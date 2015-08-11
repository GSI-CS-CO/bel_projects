create_clock -name pin_a_me_esc   -period    1MHz  [get_ports   A_ME_ESC]
create_clock -name pin_a_me_dsc   -period    1MHz  [get_ports   A_ME_DSC]
create_clock -name pin_clk_20     -period   20MHz  [get_ports     clk_20]

#the following clocks are base clocks to plls and dont need to be declared
#when using -create_base_clocks option
#create_clock -name pin_A_SYSCLOCK -period 12.5MHz  [get_ports A_SYSCLOCK]
#create_clock -name pin_clk_fpga   -period  125MHz  [get_ports   clk_fpga]

derive_pll_clocks -create_base_clocks
derive_clock_uncertainty



set_clock_groups -asynchronous                           \
 -group { altera_reserved_tck                           }\
 -group { pin_a_me_esc                                  }\
 -group { pin_a_me_dsc                                  }\
 -group { pin_clk_20                                    }\
 -group {sio3_clk_sw|local_clk|altpll_component|auto_generated|pll1|clk[0]}\
 -group {sio3_clk_sw|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[0]}\
 -group {sio3_clk_sw|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[0]~1}

# the output clocks for clk[0] and clk[0]~1 have to be cut since it is an Switchover PLL.
# clk[0] is the clock depending on pll input0, clk[0]~1 is depending on pll input1 