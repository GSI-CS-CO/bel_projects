library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.gencores_pkg.all;
use work.scu_bus_slave_pkg.all;
use work.scu_diob_pkg.all;

architecture diob_module2_arch of diob_module is

  alias A_nLED7: std_logic is vect_en(17);
  alias A_nLED6: std_logic is vect_en(19);
  alias A_nLED5: std_logic is vect_en(21);
  alias A_nLED4: std_logic is vect_en(23);
  alias A_nLED3: std_logic is vect_en(25);
  alias A_nLED2: std_logic is vect_en(27);
  alias A_nLED1: std_logic is vect_en(29);
  alias A_nLED0: std_logic is vect_en(31);
begin
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
    A_nLED0 <= count(26);
  end process; 
  
  A_nLED7 <= '1'; --led off
  A_nLED6 <= '1'; --led off
  A_nLED5 <= '1'; --led off
  A_nLED4 <= '1'; --led off
  A_nLED3 <= '1'; --led off
  A_nLED2 <= '1'; --led off
  A_nLED1 <= '1'; --led off
  
  irq_o <= (others => '0'); -- no irqs for this module

end architecture diob_module2_arch;
