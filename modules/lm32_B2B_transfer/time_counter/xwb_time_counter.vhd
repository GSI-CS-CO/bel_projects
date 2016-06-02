library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wr_fabric_pkg.all;
use work.time_counter_pkg.all;

entity xwb_time_counter is
  port(
    clk_i                 : in std_logic;
    rst_n_i               : in std_logic;

    tm_tai_i          : in  std_logic_vector(39 downto 0);
    tm_cycle_i        : in  std_logic_vector(27 downto 0);

    wb_ctrl_stat_slave_o  : out t_wishbone_slave_out;
    wb_ctrl_stat_slave_i  : in  t_wishbone_slave_in);

end xwb_time_counter;


architecture rtl of xwb_time_counter is

  signal s_counter_ctrl : t_counter_ctrl_reg;
  signal s_counter_stat : t_counter_stat_reg;

begin

  c1 : time_counter
    port map (
      clk_i          => clk_i,
      rst_n_i        => rst_n_i,
      ctrl_reg_i     => s_counter_ctrl,
      stat_reg_o     => s_counter_stat,
      counter_tm_tai_i    => tm_tai_i,
      counter_tm_cycle_i  => tm_cycle_i);

  ctrl_stat_reg : wb_slave_time_counter
  
    port map (
      clk_i          => clk_i,
      rst_n_i        => rst_n_i,
      wb_slave_i     => wb_ctrl_stat_slave_i,
      wb_slave_o     => wb_ctrl_stat_slave_o,
      stat_reg_i     => s_counter_stat,
      ctrl_reg_o     => s_counter_ctrl);

end rtl;
