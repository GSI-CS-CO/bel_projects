LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;

ENTITY Independent_Clk IS
	PORT
	(
		nMaster_Clk_Ena		: OUT STD_LOGIC;
		nSlave_Clk_Ena		: OUT STD_LOGIC;
		nIndenpend_Clk_Ena	: OUT STD_LOGIC;
		Extern_Clk_OK		: OUT STD_LOGIC
	);
END Independent_Clk;


ARCHITECTURE Arch_Independent_Clk OF Independent_Clk IS
begin
	nMaster_Clk_Ena <= '1';
	nSlave_Clk_Ena	<= '1';
	nIndenpend_Clk_Ena <= '0';

	Extern_Clk_OK <= '1';
	
END Arch_Independent_Clk;
