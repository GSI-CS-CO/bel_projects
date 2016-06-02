library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wr_fabric_pkg.all;
use work.time_counter_pkg.all;
use work.endpoint_pkg.all;

entity time_counter is
port ( 
    clk_i           : in  std_logic;
    rst_n_i         : in  std_logic;
    ctrl_reg_i      : in  t_counter_ctrl_reg;
    stat_reg_o      : out t_counter_stat_reg;
    counter_tm_tai_i     : in  std_logic_vector(39 downto 0);
    counter_tm_cycle_i   : in  std_logic_vector(27 downto 0));

end time_counter;    

architecture rtl of time_counter is

  signal s_counter_ctrl : t_counter_ctrl_reg := c_counter_ctrl_default;
  signal s_counter_stat : t_counter_stat_reg := c_counter_stat_default;
  signal start_old_stat : std_logic := '0';

  begin

  -- Start/Stop time counter
  counter : process(clk_i)
    begin
    if rising_edge(clk_i) then
      if rst_n_i = '0' then
  
      else
        if s_counter_ctrl.s_counter_start = '1' and start_old_stat = '0' then
           s_counter_stat.s_counter_ts_start_tai        <= counter_tm_tai_i;
           s_counter_stat.s_counter_ts_start_cyc        <= counter_tm_cycle_i;  
        end if;

        if s_counter_ctrl.s_counter_start = '0' and start_old_stat = '1'then
           s_counter_stat.s_counter_ts_stop_tai        <= counter_tm_tai_i;
           s_counter_stat.s_counter_ts_stop_cyc        <= counter_tm_cycle_i;  
        end if;
        start_old_stat <= s_counter_ctrl.s_counter_start; 
      end if;   
    end if;   
  end process;

  ctrl_stat_reg: process (clk_i)
    begin
      if rising_edge(clk_i) then
        if rst_n_i = '0' then

        else
          s_counter_ctrl <= ctrl_reg_i;
          stat_reg_o     <= s_counter_stat;
        end if;  
      end if;
    end process;

  end rtl;
