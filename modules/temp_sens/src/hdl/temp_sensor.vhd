library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity temp_sensor is
        port (
                ce         : in  std_logic                    := '0'; --         ce.ce
                clk        : in  std_logic                    := '0'; --        clk.clk
                clr        : in  std_logic                    := '0'; --        clr.reset
                tsdcaldone : out std_logic;                           -- tsdcaldone.tsdcaldone
                tsdcalo    : out std_logic_vector(7 downto 0)         --    tsdcalo.tsdcalo
        );
end entity temp_sensor;

architecture rtl of temp_sensor is

begin

--p_clk_process: process(clk)

--	if rising_edge(clk) then
		
--		if (rst = '0') then

--			tsdcaldone 	<= '1';
--			tsdcalo		<= x"55";
		
--		end if;
--	end if;

--end process;

	

end architecture rtl; -- of test


