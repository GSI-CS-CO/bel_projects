derive_pll_clocks -create_base_clocks

create_clock -period 1MHz [get_ports A_ME_ESC]
create_clock -period 1MHz [get_ports A_ME_DSC]
