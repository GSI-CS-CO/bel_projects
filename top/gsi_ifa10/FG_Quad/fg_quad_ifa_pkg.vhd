library IEEE;
use IEEE.STD_LOGIC_1164.all;
use ieee.numeric_std.all;

library work;

package fg_quad_ifa_pkg is

component fg_quad_datapath is
  generic (
    CLK_in_Hz:  integer := 125_000_000);
  port (
  data_a:             in  std_logic_vector(15 downto 0);
  data_b:             in  std_logic_vector(15 downto 0);
  data_c:             in  std_logic_vector(31 downto 0);
  clk:                in  std_logic;
  nrst:               in  std_logic;
  sync_rst:           in  std_logic;
  a_en:               in  std_logic;                      -- data register enable
  load_start:         in  std_logic;
  sync_start:         in  std_logic;
  step_sel:           in  std_logic_vector(2 downto 0);
  shift_a:            in  integer range 0 to 48;          -- shiftvalue coeff b
  shift_b:            in  integer range 0 to 48;          -- shiftvalue coeff b
  freq_sel:           in  std_logic_vector(2 downto 0);
  state_change_irq:   out std_logic;
  dreq:               out std_logic;
  ramp_sec_fin:       out std_logic;
  sw_out:             out std_logic_vector(31 downto 0);
  sw_strobe:          out std_logic;
  fg_is_running:      out std_logic);
end component;




end package fg_quad_ifa_pkg;
