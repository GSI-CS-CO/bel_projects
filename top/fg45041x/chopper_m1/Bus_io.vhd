LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;

ENTITY Bus_IO IS
	GENERIC (
			ST_160_Pol			: INTEGER := 1;
			I0_5_is_Input		: INTEGER := 1;
			I0_4_is_Input		: INTEGER := 1;
			I0_3_is_Input		: INTEGER := 1;
			I0_2_is_Input		: INTEGER := 1;
			I0_1_is_Input		: INTEGER := 1
			);
	PORT
	(
		To_IO_5				: IN STD_LOGIC;
		To_IO_4				: IN STD_LOGIC;
		To_IO_3				: IN STD_LOGIC;
		To_IO_2				: IN STD_LOGIC;
		To_IO_1				: IN STD_LOGIC;
		A_Bus_IO			: INOUT STD_LOGIC_VECTOR(5 DOWNTO 1)
	);
END Bus_IO;


ARCHITECTURE Arch_Bus_IO OF Bus_IO IS

	
BEGIN

ST_160: IF ST_160_Pol = 1 GENERATE -----------------------------------------------

P_Bus_IO:	PROCESS (To_IO_5, To_IO_4, To_IO_3, To_IO_2, To_IO_1)
	BEGIN
	    IF I0_5_is_Input = 1 THEN
			A_Bus_IO(5) <= 'Z';
		ELSE
			A_Bus_IO(5) <= To_IO_5;
		END IF;
		
	    IF I0_4_is_Input = 1 THEN
			A_Bus_IO(4) <= 'Z';
		ELSE
			A_Bus_IO(4) <= To_IO_4;
		END IF;

	    IF I0_3_is_Input = 1 THEN
			A_Bus_IO(3) <= 'Z';
		ELSE
			A_Bus_IO(3) <= To_IO_3;
		END IF;

	    IF I0_2_is_Input = 1 THEN
			A_Bus_IO(2) <= 'Z';
		ELSE
			A_Bus_IO(2) <= To_IO_2;
		END IF;

	    IF I0_1_is_Input = 1 THEN
			A_Bus_IO(1) <= 'Z';
		ELSE
			A_Bus_IO(1) <= To_IO_1;
		END IF;

	END PROCESS;
	
END GENERATE ST_160; -------------------------------------------------------------

ST_96: IF ST_160_Pol = 0 GENERATE -----------------------------------------------

A_Bus_IO(5 DOWNTO 1) <= (OTHERS => 'Z');

END GENERATE ST_96; -------------------------------------------------------------

END Arch_Bus_IO;
