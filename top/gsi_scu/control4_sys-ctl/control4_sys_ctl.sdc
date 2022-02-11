#
#
create_clock -name {master_clk} -period 10.0 [get_ports {clk_base_i}] -add

derive_clock_uncertainty
derive_pll_clocks
set_false_path -from [get_ports scu_cb_revision*] -to *
set_false_path -from * -to [get_ports led_status_o*]