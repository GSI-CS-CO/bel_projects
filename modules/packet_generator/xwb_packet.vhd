library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wr_fabric_pkg.all;
use work.packet_pkg.all;
use work.endpoint_pkg.all;

entity xwb_packet is
  port(
    clk_i                 : in std_logic;
    rst_n_i               : in std_logic;
    wr_src_o              : out t_wrf_source_out;
    wr_src_i              : in  t_wrf_source_in;

 	  timestamps_i      : in  t_txtsu_timestamp;
    tm_tai_i          : in  std_logic_vector(39 downto 0);
    tm_cycle_i        : in  std_logic_vector(27 downto 0);

    wb_ctrl_stat_slave_o  : out t_wishbone_slave_out;
    wb_ctrl_stat_slave_i  : in  t_wishbone_slave_in);

end xwb_packet;


architecture rtl of xwb_packet is

  signal s_pg_ctrl        : t_pg_ctrl_reg;
  signal s_pg_stat        : t_pg_stat_reg;

begin

  pg : packet_gen
    port map (
      clk_i          => clk_i,
      rst_n_i        => rst_n_i,
      ctrl_reg_i     => s_pg_ctrl,
      stat_reg_o     => s_pg_stat,
    	pg_timestamps_i=> timestamps_i,
      pg_tm_tai_i    => tm_tai_i,
      pg_tm_cycle_i  => tm_cycle_i,
      pg_src_i       => wr_src_i,
      pg_src_o       => wr_src_o);

  ctrl_stat_reg : wb_slave_packet
    port map (
      clk_i          => clk_i,
      rst_n_i        => rst_n_i,
      wb_slave_i     => wb_ctrl_stat_slave_i,
      wb_slave_o     => wb_ctrl_stat_slave_o,
      pg_stat_reg_i  => s_pg_stat,
      pg_ctrl_reg_o  => s_pg_ctrl);

end rtl;
