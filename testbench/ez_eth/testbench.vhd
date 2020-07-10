library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;  

-- package with component to test on this testbench
--use work.pcie_tlp.all;

use work.wishbone_pkg.all;
use work.mbox_pkg.all;
use work.etherbone_pkg.all;
use work.wr_fabric_pkg.all;
use work.ez_eth_pkg.all;

entity testbench is
end entity;

architecture simulation of testbench is

  -- clock/reset generation
  signal rst              : std_logic := '1';
  signal rst_n            : std_logic := '0';
  signal rstn_sys         : std_logic := '0';
  constant clk_50_period  : time      := 20 ns;
  constant clk_125_period : time      :=  8 ns;
  constant clk_sys_period : time      := 16 ns;
  signal clk_50           : std_logic := '1';
  signal clk_125          : std_logic := '1';
  signal clk_sys          : std_logic := '1';


  
  signal wrf_loopback_out : t_wrf_source_out;
  signal wrf_loopback_in  : t_wrf_source_in;

  ---
  


begin


  ---- generate clock and reset signal -------
  clk_50  <= not clk_50  after clk_50_period/2;
  clk_125 <= not clk_125 after clk_125_period/2;
  clk_sys <= not clk_sys after clk_sys_period/2;
  rst     <= '0'         after clk_50_period*20;
  rst_n   <= not rst;
  rstn_sys<= not rst;
  --------------------------------------------

  ---- instance of EZETH-chip 
  -- virtual NIC
  chip : ez_eth_chip
    port map (
      rstn_i    => rst_n ,
      clk_i   => clk_125,
      rx_src_o => wrf_loopback_out,
      rx_src_i => wrf_loopback_in,
      tx_snk_i => wrf_loopback_out,
      tx_snk_o => wrf_loopback_in


      );

end architecture;



