create_clock -period 125Mhz -name sfp_ref_clk_i [get_ports {sfp_ref_clk_i}]
create_clock -period 10Mhz -name wr_10Mhz [get_ports {lemo_nim_ttl_i[1]}]
create_clock -period 1MHz -name wr_pps [get_ports {lemo_nim_ttl_i[0]}]

derive_pll_clocks -create_base_clocks
derive_clock_uncertainty

# Cut the clock domains from each other
set_clock_groups -asynchronous                           \
 -group { altera_reserved_tck                          } \
 -group { clk_20m_vcxo_i    main|\dmtd_a2:dmtd_inst|*  } \
 -group { clk_125m_local_i  main|\sys_a2:sys_inst|*    } \
 -group { wr_10Mhz                                     } \
 -group { wr_pps                                       } \
 -group { clk_125m_pllref_i main|\ref_a2:ref_inst|*      \
          main|\phy_a2:phy|*|tx_pll0|*                   \
          main|\phy_a2:phy|*|ch_clk_div0|*               \
          main|\phy_a2:phy|*|transmit_pma0|*             \
          main|\phy_a2:phy|*|transmit_pcs0|*           } \
 -group { sfp_ref_clk_i                                  \
          main|\phy_a2:phy|*|rx_cdr_pll0|*               \
          main|\phy_a2:phy|*|receive_pma0|*              \
          main|\phy_a2:phy|*|receive_pcs0|*            }

# cut: wb sys <=> wb flash   (different frequencies and using xwb_clock_crossing)
set_false_path -from [get_clocks {main|\sys_a2:sys_inst|*|clk[0]}] -to [get_clocks {main|\sys_a2:sys_inst|*|clk[1]}]
set_false_path -from [get_clocks {main|\sys_a2:sys_inst|*|clk[1]}] -to [get_clocks {main|\sys_a2:sys_inst|*|clk[0]}]
# cut: wb sys <=> wb display (different frequencies and using xwb_clock_crossing)
set_false_path -from [get_clocks {main|\sys_a2:sys_inst|*|clk[0]}] -to [get_clocks {main|\sys_a2:sys_inst|*|clk[2]}]
set_false_path -from [get_clocks {main|\sys_a2:sys_inst|*|clk[2]}] -to [get_clocks {main|\sys_a2:sys_inst|*|clk[0]}]
# cut: wr-ref <=> butis
set_false_path -from [get_clocks {main|\ref_a2:ref_inst|*|clk[0]}] -to [get_clocks {main|\ref_a2:ref_inst|*|clk[1]}]
set_false_path -from [get_clocks {main|\ref_a2:ref_inst|*|clk[1]}] -to [get_clocks {main|\ref_a2:ref_inst|*|clk[0]}]
# cut: lemo <=> sys clk
set_false_path -from [get_clocks {wr_10Mhz}] -to [get_clocks {main|\sys_a2:sys_inst|*|clk[0]}]
# cut: lemo <=> ref clk
set_false_path -from [get_clocks {wr_pps}] -to [get_clocks {main|\ref_a2:ref_inst|*|clk[0]}]
