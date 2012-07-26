LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;

ENTITY epld_vers IS
	GENERIC(Test : integer := 0 );
	PORT
	(
		Vers_Rev :		OUT STD_LOGIC_VECTOR(7 downto 0);
		Test_Activ :	OUT STD_LOGIC
	);
	
END epld_vers;


ARCHITECTURE Arch_epld_vers OF epld_vers IS

TYPE t_Version IS RECORD
	Vers	: STD_LOGIC_VECTOR(3 DOWNTO 0);
	Revi	: STD_LOGIC_VECTOR(3 DOWNTO 0);
END RECORD;

SIGNAL	S_Version : t_Version;


BEGIN
	S_Version.Vers <= CONV_STD_LOGIC_VECTOR(1, 4);
	S_Version.Revi <= CONV_STD_LOGIC_VECTOR(0, 4);

P_EPLD_Vers:	PROCESS (S_Version)
	BEGIN	
		IF Test = 1 THEN
			Vers_Rev(7 DOWNTO 0) <= CONV_STD_LOGIC_VECTOR(0, 8);
			Test_Activ <= '1';
		ELSE
			Vers_Rev(7 DOWNTO 4) <= S_Version.Vers;
			Vers_Rev(3 DOWNTO 0) <= S_Version.Revi;
			Test_Activ <= '0';
		END IF;
	END PROCESS P_EPLD_Vers;
	
END Arch_epld_vers;
