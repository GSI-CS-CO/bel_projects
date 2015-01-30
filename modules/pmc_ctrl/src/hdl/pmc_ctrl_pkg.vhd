library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;
use work.pmc_ctrl_auto_pkg.all;
  
package pmc_ctrl_pkg is
  
   component pmc_ctrl is
   Port(
      clk_sys_i      : in  std_logic := '0';
      rst_n_i        : in  std_logic := '0';
      slave_i        : in  t_wishbone_slave_in  := ('0', '0', x"00000000", x"F", '0', x"00000000");
      slave_o        : out t_wishbone_slave_out;
      clock_enable_o : out std_logic;
      hex_switch_i   : in  std_logic_vector(3 downto 0) := (others => '0')
   );
   end component;
  
end pmc_ctrl_pkg;
package body pmc_ctrl_pkg is
end pmc_ctrl_pkg;
