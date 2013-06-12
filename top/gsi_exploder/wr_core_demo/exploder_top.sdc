create_clock -period 125Mhz -name sfp_ref_clk_i [get_ports {sfp_ref_clk_i}]
derive_pll_clocks -create_base_clocks
derive_clock_uncertainty

# Cut the clock domains from each other
set_clock_groups -asynchronous                \
 -group { altera_reserved_tck               } \
 -group { clk_20m_vcxo_i    dmtd_inst|*     } \
 -group { clk_125m_local_i  sys_inst|*      } \
 -group { clk_125m_pllref_i ref_inst|*        \
          wr_gxb*|tx_pll0|*                   \
          wr_gxb*|ch_clk_div0|*               \
          wr_gxb*|transmit_pma0|*             \
          wr_gxb*|transmit_pcs0|*           } \
 -group { sfp_ref_clk_i                       \
          wr_gxb*|rx_cdr_pll0|*               \
          wr_gxb*|receive_pma0|*              \
          wr_gxb*|receive_pcs0|*            }
