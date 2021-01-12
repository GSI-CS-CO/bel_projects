derive_pll_clocks -create_base_clocks

derive_clock_uncertainty
set_false_path -from [get_clocks {master_clk1|altpll_component|pll|clk[1]}] -to [get_clocks {inst1|altpll_component|pll|clk[0]}]
set_false_path -from [get_clocks {inst1|altpll_component|pll|clk[0]}] -to [get_clocks {master_clk1|altpll_component|pll|clk[1]}]
