library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.gencores_pkg.all;
use work.scu_bus_slave_pkg.all;
use work.scu_diob_pkg.all;

architecture diob_module1_arch of diob_module is
begin
  vect_i  <= x"0000000000000000000000000000000"& "001";
  vect_o  <= x"0000000000000000000000000000000"& "010";
  vect_en <= x"0000000000000000000000000000000"& "010";

end architecture diob_module1_arch;
