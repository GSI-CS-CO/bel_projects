----------------------------------------------------------------------------------
--Debounce Inputs
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity debounce is
    Port ( clk      : in   STD_LOGIC;
           input    : in   STD_LOGIC;
           output   : out  STD_LOGIC;
           en_deb : in  STD_LOGIC);
end debounce;

architecture Behavioral of debounce is
signal inputsr   : std_logic_vector(3 downto 0);
begin
   process begin
   wait until rising_edge(clk);
   if (en_deb='1') then
      -- Pegel zuweisen
      if (inputsr = "0000") then output<='0'; end if;
      if (inputsr = "1111") then output<='1'; end if;
      -- von rechts Eintakten
      inputsr <= inputsr(2 downto 0) & input;
   end if;
   end process;
end Behavioral;
