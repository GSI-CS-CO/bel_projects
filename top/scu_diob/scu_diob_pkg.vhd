library ieee;  
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.gencores_pkg.all;

package scu_diob_pkg is

  TYPE    t_IO_Reg_1_to_7_Array     is array (1 to 7)  of std_logic_vector(15 downto 0);
  TYPE    t_IO_Reg_0_to_7_Array     is array (0 to 7)  of std_logic_vector(15 downto 0);
  
end scu_diob_pkg;
