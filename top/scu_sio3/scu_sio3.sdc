create_clock -name pin_a_me_esc   -period    1MHz  [get_ports   A_ME_ESC]
create_clock -name pin_a_me_dsc   -period    1MHz  [get_ports   A_ME_DSC]
create_clock -name pin_clk_20     -period   20MHz  [get_ports     clk_20]

derive_pll_clocks -create_base_clocks

derive_clock_uncertainty
