derive_pll_clocks -create_base_clocks
derive_clock_uncertainty

# Named clocks
set clk_ref0_125m_ref_clk              [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[0].*}]
set clk_ref1_200m_butis_clk            [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[1].*}]
set clk_ref2_25m_phase_butis_clk       [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[2].*}]
set clk_ref3_1000m_clk_lvds            [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[3].*}]
set clk_ref4_125m_clk_lvds_enable_18dc [get_clocks {main|\ref_a5:ref_inst|ref_pll5_inst|altera_pll_i|arriav_pll|counter[4].*}]
set clk_sys0_62_5_sys_clk              [get_clocks {main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[0].*}]
set clk_sys1_100_reconf_clk            [get_clocks {main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[1].*}]
set clk_sys2_20_lcd_clk                [get_clocks {main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[2].*}]
set clk_sys3_10_update_clk             [get_clocks {main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[3].*}]
set clk_sys3_20_flash_ext_clk          [get_clocks {main|\sys_a5:sys_inst|sys_pll5_inst|altera_pll_i|general[4].*}]

create_clock -name {monster:main|ref_pll5:\ref_a5:ref_inst|ref_pll5_0002:ref_pll5_inst|altera_pll:altera_pll_i|altera_arriav_pll:arriav_pll|altera_arriav_pll_base:fpll_0|PLL_RECONFIG~FMAX_CAP_FF} -period 8.000 [get_pins {main|phase|raw_trap|clk}]

# Cut the clock domains from each other
set_clock_groups -asynchronous                           \
 -group { altera_reserved_tck                          } \
 -group { clk_20m_vcxo_i    main|\dmtd_a5:dmtd_inst|*  } \
 -group { clk_125m_local_i  main|\sys_a5:sys_inst|*    } \
 -group { sfp234_ref_clk_i  main|\ref_a5:ref_inst|*      \
          main|\phy_a5:phy|*.cdr_refclk*                 \
          main|\phy_a5:phy|*.cmu_pll.*                   \
          main|\phy_a5:phy|*|av_tx_pma|*                 \
          main|\phy_a5:phy|*|inst_av_pcs|*|tx*         } \
 -group { main|\phy_a5:phy|*|clk90bdes                   \
          main|\phy_a5:phy|*|clk90b                      \
          main|\phy_a5:phy|*|rcvdclkpma                } \
 -group { pcie_refclk_i                                  \
          main|\pcie_y:pcie|*.cdr_refclk*                \
          main|\pcie_y:pcie|*.cmu_pll.*                  \
          main|\pcie_y:pcie|*|av_tx_pma|*                \
          main|\pcie_y:pcie|*|inst_av_pcs|*|tx*        } \
 -group { main|\pcie_y:pcie|*|rx_pmas[0]*|clk90bdes      \
          main|\pcie_y:pcie|*|rx_pmas[0]*|clk90b       } \
 -group { main|\pcie_y:pcie|*|rx_pmas[1]*|clk90bdes      \
          main|\pcie_y:pcie|*|rx_pmas[1]*|clk90b       } \
 -group { main|\pcie_y:pcie|*|rx_pmas[2]*|clk90bdes      \
          main|\pcie_y:pcie|*|rx_pmas[2]*|clk90b       } \
 -group { main|\pcie_y:pcie|*|rx_pmas[3]*|clk90bdes      \
          main|\pcie_y:pcie|*|rx_pmas[3]*|clk90b       } \
 -group { main|\pcie_y:pcie|*|coreclkout               }

# cut: wb sys <=> wb flash   (different frequencies and using xwb_clock_crossing)
set_false_path -from [get_clocks {main|\sys_a5:sys_inst|*|general[0].*}] -to [get_clocks {main|\sys_a5:sys_inst|*|general[4].*}]
set_false_path -from [get_clocks {main|\sys_a5:sys_inst|*|general[4].*}] -to [get_clocks {main|\sys_a5:sys_inst|*|general[0].*}]
# cut: wb sys <=> wb display (different frequencies and using xwb_clock_crossing)
set_false_path -from [get_clocks {main|\sys_a5:sys_inst|*|general[0].*}] -to [get_clocks {main|\sys_a5:sys_inst|*|general[1].*}]
set_false_path -from [get_clocks {main|\sys_a5:sys_inst|*|general[1].*}] -to [get_clocks {main|\sys_a5:sys_inst|*|general[0].*}]
# cut: wr-ref <=> butis
set_false_path -from [get_clocks {main|\ref_a5:ref_inst|*|counter[0].*}] -to [get_clocks {main|\ref_a5:ref_inst|*|counter[1].*}]
set_false_path -from [get_clocks {main|\ref_a5:ref_inst|*|counter[1].*}] -to [get_clocks {main|\ref_a5:ref_inst|*|counter[0].*}]
# cut: butis <=> sys
set_false_path -from [get_clocks {main|\sys_a5:sys_inst|*|general[0].*}] -to [get_clocks {main|\ref_a5:ref_inst|*|counter[1].*}]
set_false_path -from [get_clocks {main|\sys_a5:sys_inst|*|general[0].*}] -to [get_clocks {main|\ref_a5:ref_inst|*|counter[2].*}]
set_false_path -from [get_clocks {main|\ref_a5:ref_inst|*|counter[1].*}] -to [get_clocks {main|\sys_a5:sys_inst|*|general[0].*}]
set_false_path -from [get_clocks {main|\ref_a5:ref_inst|*|counter[2].*}] -to [get_clocks {main|\sys_a5:sys_inst|*|general[0].*}]
# cut: ref <=> sys
set_false_path -from [get_clocks {main|\sys_a5:sys_inst|*|general[0].*}] -to [get_clocks {main|\ref_a5:ref_inst|*|counter[0].*}]
set_false_path -from [get_clocks {main|\ref_a5:ref_inst|*|counter[0].*}] -to [get_clocks {main|\sys_a5:sys_inst|*|general[0].*}] 
