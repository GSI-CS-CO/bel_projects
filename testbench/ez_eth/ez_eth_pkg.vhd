library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wr_fabric_pkg.all;

package ez_eth_pkg is



  component ez_eth_chip is
  port (
      rstn_i   : in  std_logic;
      clk_i    : in  std_logic;
      rx_src_o : out t_wrf_source_out;
      rx_src_i : in t_wrf_source_in;
      tx_snk_i : in t_wrf_sink_in;
      tx_snk_o : out t_wrf_sink_out
    );
  end component;
 
end ez_eth_pkg;
