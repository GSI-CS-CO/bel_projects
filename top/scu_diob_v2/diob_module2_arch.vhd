library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.gencores_pkg.all;
use work.scu_bus_slave_pkg.all;
use work.scu_diob_pkg.all;

architecture diob_module2_arch of diob_module is
  signal en:    std_logic;
  signal sreg:  std_logic_vector(7 downto 0) := "10000000";
begin
  vect_i  <= x"0000000000000000000000000000000"& "000";
  
  vect_o(17) <= '1'; --A_nLED7
  vect_o(19) <= '1'; --A_nLED6
  vect_o(21) <= '1'; --A_nLED5
  vect_o(23) <= '1'; --A_nLED4
  vect_o(25) <= '1'; --A_nLED3
  vect_o(27) <= '1'; --A_nLED2
  vect_o(29) <= '1'; --A_nLED1
  vect_o(31) <= '1'; --A_nLED0
  
  clk_divider: process (clk)
    variable count: unsigned(26 downto 0);
  begin
    if rising_edge(clk) then
        count := count - 1;
    end if;
    vect_en(19) <= count(26);
  end process; 
  
  irq_o <= (others => '0'); -- no irqs for this module

end architecture diob_module2_arch;
