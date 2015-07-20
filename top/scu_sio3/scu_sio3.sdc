create_clock -name a_me_esc_clk -period   1000  -waveform {0 500} [get_ports A_ME_ESC]
create_clock -name a_me_dsc_clk -period   1000  -waveform {0 500} [get_ports A_ME_DSC]
#create_clock -period 125MHz [get_ports clk_fpga]
#create_clock -period  20MHz [get_ports   clk_20]
#create_clock -period  25MHz [get_ports A_SYSCLOCK]

derive_pll_clocks -create_base_clocks
derive_clock_uncertainty


set_clock_groups -asynchronous                        \
 -group { [get_ports {altera_reserved_tck}]         } \
 -group { [get_ports {A_SYSCLOCK}]                  } \
 -group { [get_ports {clk_fpga}]                    } \
 -group { a_me_esc_clk                              } \
 -group { a_me_dsc_clk                              } \
 -group { [get_ports {CLK_20MHZ_D}]                 }

set_false_path  \
      -from [get_clocks {sio3_clk_sw|local_clk|altpll_component|auto_generated|pll1|clk[0]}]  \
      -to   [get_clocks {sio3_clk_sw|sys_or_local_pll|altpll_component|auto_generated|pll1|clk[0]}]
