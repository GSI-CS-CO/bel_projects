-- "Global_Status für den 'Chopper'-Macro   W.Panschow V01 d.07.03.01";

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity Global_Stat is
		port (
				Frei:					IN std_logic_vector(15 downto 8) := x"00";
				Chopper_Vers:			IN std_logic_vector(7 downto 0);
				Global_Status:			OUT std_logic_vector(15 downto 0)
			);
end Global_Stat;

------------------------------------------------------------------------------------------

architecture Global_Stat_arch of Global_Stat is

begin
	Global_Status <= (Frei & Chopper_Vers);

end Global_Stat_arch;
