library IEEE;
use IEEE.STD_LOGIC_1164.all;
use ieee.numeric_std.all;

library work;

package fg_quad_pkg is

component fg_quad_datapath is
  generic (
    CLK_in_Hz:  integer := 125000000);
  port (
  data_a:             in  std_logic_vector(15 downto 0);
  data_b:             in  std_logic_vector(15 downto 0);
  clk:                in  std_logic;
  rst:                in  std_logic;
  a_en, b_en:         in  std_logic;                      -- data register enable
  load_start, s_en:   in  std_logic;
  status_reg_changed: in  std_logic;   
  step_sel:           in  std_logic_vector(2 downto 0);   -- shiftvalue coeff a
  b_shift:            in  integer range 0 to 16;          -- shiftvalue coeff b
  freq_sel:           in  std_logic_vector(2 downto 0);
  sw_out:             out std_logic_vector(23 downto 0);
  set_out:            out std_logic);                     -- debug out
end component;

end package fg_quad_pkg;