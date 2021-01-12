LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;

ENTITY Signal_Stretch IS
	GENERIC (cnt : INTEGER := 4);
	port
	(
		Sig_in :			IN		STD_LOGIC;
		Stretch_on :		IN		STD_LOGIC;
		Ena_Cnt :			IN		STD_LOGIC;
		clk :				IN		STD_LOGIC;
		Sig_Out :			OUT		STD_LOGIC
	);
END Signal_Stretch;


ARCHITECTURE Arch_Signal_Stretch OF Signal_Stretch IS

    FUNCTION	How_many_Bits  (int: INTEGER) RETURN INTEGER IS

	VARIABLE i, tmp : INTEGER;

	BEGIN
		tmp		:= int;
		i		:= 0;
		WHILE tmp > 0 LOOP
			tmp := tmp / 2;
			i := i + 1;
		END LOOP;
		RETURN i;
	END How_many_bits;


SIGNAL	S_Stretch_Cnt :		STD_LOGIC_VECTOR ((How_many_Bits(Cnt-2)) DOWNTO 0);

BEGIN

P_Stretch_Cnt:	PROCESS (clk, Sig_in)
	BEGIN
		IF Sig_in = '1' THEN
			S_Stretch_Cnt <= CONV_STD_LOGIC_VECTOR(Cnt-2, S_Stretch_Cnt'LENGTH);
		ELSIF clk'EVENT AND CLK = '1' THEN
			IF Ena_Cnt = '1' AND S_Stretch_Cnt(S_Stretch_Cnt'left) = '0' THEN
				S_Stretch_Cnt <= S_Stretch_Cnt - 1;
			END IF;
		END IF;
	END PROCESS P_Stretch_Cnt;

P_Stretch:	PROCESS (Sig_in, Stretch_on, S_Stretch_Cnt(S_Stretch_Cnt'left))
	BEGIN
		Sig_Out <= '0';
		IF Stretch_on = '1' THEN
			Sig_Out <= not S_Stretch_Cnt(S_Stretch_Cnt'left);
		ELSE
			Sig_Out <= Sig_in;
		END IF;
	END PROCESS P_Stretch;

END Arch_Signal_Stretch;
