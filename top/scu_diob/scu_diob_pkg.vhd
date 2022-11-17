library ieee;  
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.gencores_pkg.all;

package scu_diob_pkg is

  TYPE    t_IO_Reg_1_to_7_Array     is array (1 to 7)  of std_logic_vector(15 downto 0);
  TYPE    t_IO_Reg_0_to_7_Array     is array (0 to 7)  of std_logic_vector(15 downto 0);
  TYPE    t_led_array               is array (1 to 12) of std_logic_vector(6 downto 1);
  TYPE    t_id_array                is array (1 to 12) of std_logic_vector(7 downto 0);
  TYPE    t_IOBP_array              is array (1 to 12) of std_logic_vector(5 downto 0);
  TYPE    t_IO_Reg_0_to_4_Array     is array (0 to 4)  of std_logic_vector(15 downto 0);
  TYPE    t_in_array                is array (0 to 8) of std_logic_vector(5 downto 0);
  TYPE    t_gate_counter_in_Array   is array (0 to 5) of std_logic_vector(11 downto 0);
  TYPE    t_IO_Reg_1_to_8_Array     is array (1 to 8)  of std_logic_vector(15 downto 0);
end scu_diob_pkg;
