# Original Clock Setting Name: A_ME_ESC
create_clock -period "8.333 ns" \
             -name {A_ME_ESC} {A_ME_ESC}
# ---------------------------------------------


# Original Clock Setting Name: A_ME_TD
create_clock -period "8.333 ns" \
             -name {A_ME_TD} {A_ME_TD}
# ---------------------------------------------


# Original Clock Setting Name: A_ME_DSC
create_clock -period "8.333 ns" \
             -name {A_ME_DSC} {A_ME_DSC}
# ---------------------------------------------

# Original Clock Setting Name: A_ME_VW
create_clock -period "8.333 ns" \
             -name {A_ME_VW} {A_ME_VW}
# ---------------------------------------------

#create_clock -name A_Loader_CLK  -period   125MHz   [get_ports  A_Loader_CLK]


derive_pll_clocks -create_base_clocks

set_clock_groups -asynchronous \
                 -group { A_ME_ESC  } \
                 -group { A_ME_TD   } \
                 -group { A_ME_VW   } \
                 -group { inst8|altpll_component|pll|clk[0] } \
                 -group { inst8|altpll_component|pll|clk[1] } \
                 -group { A_ME_DSC  }



