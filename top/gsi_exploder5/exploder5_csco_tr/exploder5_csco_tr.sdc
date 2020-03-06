# Timing constraints
derive_pll_clocks -create_base_clocks
derive_clock_uncertainty

# Named clocks
set clk_ref0_125m_ref_clk              [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[0].output_counter|divclk}]
set clk_ref1_200m_butis_clk            [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[1].output_counter|divclk}]
set clk_ref2_25m_phase_butis_clk       [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[2].output_counter|divclk}]
set clk_ref3_1000m_clk_lvds            [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[3].output_counter|divclk}]
set clk_ref4_125m_clk_lvds_enable_18dc [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[4].output_counter|divclk}]
set clk_sys0_62_5_sys_clk              [get_clocks {main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[0].gpll~PLL_OUTPUT_COUNTER|divclk}]
set clk_sys1_100_reconf_clk            [get_clocks {main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[1].gpll~PLL_OUTPUT_COUNTER|divclk}]
set clk_sys2_20_lcd_clk                [get_clocks {main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[2].gpll~PLL_OUTPUT_COUNTER|divclk}]
set clk_sys3_10_update_clk             [get_clocks {main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[3].gpll~PLL_OUTPUT_COUNTER|divclk}]
set clk_sys3_20_flash_ext_clk          [get_clocks {main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[4].gpll~PLL_OUTPUT_COUNTER|divclk}]
set clk_dmtd_62_5_clk                  [get_clocks {main|\dmtd_a5:dmtd_inst|dmtd_pll5_inst|altera_pll_i|general[0].gpll~PLL_OUTPUT_COUNTER|divclk}]

# Special input clocks
create_clock -period 10Mhz -name wr_10Mhz [get_ports {lvds_clk_p_i}]

# Phy clocks
create_clock -name {monster:main|ref_pll5:\ref_a5:ref_inst|ref_pll5_0002:ref_pll5_inst|altera_pll:altera_pll_i|altera_arriav_pll:arriav_pll|altera_arriav_pll_base:fpll_0|PLL_RECONFIG~FMAX_CAP_FF} -period 8.000 [get_pins {main|phase|raw_trap|clk}]

# Cut the clock domains from each other
set_clock_groups -asynchronous \
 -group [get_clocks { altera_reserved_tck } ]                                                                           \
 -group [get_clocks { alt_cal_av_edge_detect_clk } ]                                                                    \
 -group [get_clocks { clk_20m_vcxo_i } ]                                                                                \
 -group [get_clocks { clk_125m_local_i } ]                                                                              \
 -group [get_clocks { clk_125m_pllref_i } ]                                                                             \
 -group [get_clocks { clk_sfp_i } ]                                                                                     \
 -group [get_clocks { wr_10Mhz } ]                                                                                      \
 -group [get_clocks { main|\nau8811_y:nau8811_audio|\audio_pll_y:x|audio_pll_inst|altera_pll_i|* } ]                    \
 -group [get_clocks { main|\dmtd_a5:dmtd_inst|dmtd_pll5_inst|altera_pll_i|general[0].gpll~PLL_OUTPUT_COUNTER|divclk } ] \
 -group [get_clocks { main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[0].output_counter|divclk      \
                      main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[1].output_counter|divclk      \
                      main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[2].output_counter|divclk      \
                      main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[3].output_counter|divclk      \
                      main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[4].output_counter|divclk } ]  \
 -group [get_clocks { main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[0].gpll~PLL_OUTPUT_COUNTER|divclk        \
                      main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[1].gpll~PLL_OUTPUT_COUNTER|divclk        \
                      main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[2].gpll~PLL_OUTPUT_COUNTER|divclk        \
                      main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[3].gpll~PLL_OUTPUT_COUNTER|divclk        \
                      main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[4].gpll~PLL_OUTPUT_COUNTER|divclk } ]    \
 -group [get_clocks { pcie_refclk_i                                                                                     \
                      main|\pcie_y:pcie|pcie_phy|* } ]                                                                  \
 -group [get_clocks { main|\phy_a5:phy|\gen_arria5_phy8:U_The_PHY|arria5_phy8_inst|A5|*                                 \
                      main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[0].gpll~PLL_OUTPUT_COUNTER|divclk  } ]

# cut: wr-ref <=> butis 200MHz
set_false_path -from [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[0].output_counter|divclk}] \
                 -to [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[1].output_counter|divclk}]
set_false_path -from [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[1].output_counter|divclk}] \
                 -to [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[0].output_counter|divclk}]

# cut: wr-ref <=> butis 25MHz phase 
set_false_path -from [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[0].output_counter|divclk}] \
                 -to [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[2].output_counter|divclk}]
set_false_path -from [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[2].output_counter|divclk}] \
                 -to [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[0].output_counter|divclk}]

# cut: sys <=> flash clock
set_false_path -from [get_clocks {main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[0].gpll~PLL_OUTPUT_COUNTER|divclk}] \
                 -to [get_clocks {main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[4].gpll~PLL_OUTPUT_COUNTER|divclk}]
set_false_path -from [get_clocks {main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[4].gpll~PLL_OUTPUT_COUNTER|divclk}] \
                 -to [get_clocks {main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[0].gpll~PLL_OUTPUT_COUNTER|divclk}]

# cut: sys <=> wr-ref
set_false_path -from [get_clocks {main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[0].gpll~PLL_OUTPUT_COUNTER|divclk}] \
                 -to [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[0].output_counter|divclk}]
set_false_path -from [get_clocks {main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[4].gpll~PLL_OUTPUT_COUNTER|divclk}] \
                 -to [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[0].output_counter|divclk}]
