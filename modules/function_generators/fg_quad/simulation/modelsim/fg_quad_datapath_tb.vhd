library ieee;
use ieee.std_logic_1164.all;
use ieee.math_real.all;
use ieee.numeric_std.all;

library work;
use work.fg_quad_pkg.all;

entity fg_quad_datapath_tb is
end entity;



architecture fg_tb_arch of fg_quad_datapath_tb is

  
  signal t_sig_clk:             std_logic := '1';
  signal t_data_a:              std_logic_vector(15 downto 0);
  signal t_data_b:              std_logic_vector(15 downto 0);
  signal t_rst:                 std_logic;
  signal t_a_en:                std_logic := '0';
  signal t_b_en:                std_logic := '0';
  signal t_load_start:          std_logic := '0';
  signal t_s_en:                std_logic := '0';
  signal t_status_reg_changed:  std_logic := '0';
  signal t_step_sel:            std_logic_vector(2 downto 0) := "000";
  signal t_b_shift:             integer range 0 to 16 := 0;
  signal t_freq_sel:            std_logic_vector(2 downto 0) := "000";
  signal t_sw_out:              std_logic_vector(23 downto 0);
  signal t_set_out:             std_logic;

begin
  quad_fg: fg_quad_datapath port map (
  data_a              => t_data_a,
  data_b              => t_data_b,
  clk                 => t_sig_clk,
  rst                 => t_rst,
  a_en                => t_a_en,
  b_en                => t_b_en,
  load_start          => t_load_start,
  s_en                => t_s_en,
  status_reg_changed  => t_status_reg_changed,
  step_sel            => t_step_sel,
  b_shift             => t_b_shift,
  freq_sel            => t_freq_sel,
  sw_out              => t_sw_out,
  set_out             => t_set_out     
  );


sim: process
  begin
  t_rst <= '1', '0' after 50 ns;
  t_data_a <= x"0000";
  t_data_b <= x"0000";
  wait;
end process;

clock: process
begin
	loop
		t_sig_clk <= not t_sig_clk;
		wait for 4 ns;
	end loop;                                                      
end process clock;    

end architecture;