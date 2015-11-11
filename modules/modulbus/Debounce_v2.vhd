library IEEE;
USE IEEE.STD_LOGIC_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;
LIBRARY lpm;
USE lpm.lpm_components.all;


ENTITY Debounce_v2 IS
	GENERIC(
			DB_Tst_Cnt: INTEGER := 1;
			Use_LPM:    INTEGER := 1;
			Test:       INTEGER := 0
			);
	PORT(
	   DB_Cnt:     in  integer range 0 to 131072 := 3;   
		DB_In:      IN	 STD_LOGIC;
		Reset:      IN	 STD_LOGIC;
		Clk:        IN	 STD_LOGIC;
		DB_Out:     OUT STD_LOGIC
		);
		
END Debounce_v2;

ARCHITECTURE Arch_Debounce_v2 OF Debounce_v2 IS

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

COMPONENT lpm_counter
	GENERIC (
			lpm_width		: NATURAL;
			lpm_type		: STRING
			);
	PORT(
		clock	: IN STD_LOGIC ;
		cnt_en	: IN STD_LOGIC := '1';
		updown	: IN STD_LOGIC := '1';
		aclr	: IN STD_LOGIC := '0';
		q		: OUT STD_LOGIC_VECTOR (lpm_width-1 DOWNTO 0)
		);
END COMPONENT;
	SUBTYPE	T_DB_Cnt	IS INTEGER RANGE 0 TO DB_CNT;
		SIGNAL	S_DB_In_Sync	: STD_LOGIC;
	SIGNAL	S_DB_Cnt		: STD_LOGIC_VECTOR((How_many_Bits(DB_Cnt)-1) DOWNTO 0);
	SIGNAL	S_DB_Out		: STD_LOGIC;
	
	SIGNAL	S_DB_On_Cnt		: STD_LOGIC_VECTOR(S_DB_Cnt'RANGE);
	SIGNAL	S_DB_Cnt_Ena	: STD_LOGIC;

BEGIN

S_DB_On_Cnt <= conv_std_logic_vector(DB_Cnt, S_DB_Cnt'length) WHEN (Test = 0)
			   ELSE conv_std_logic_vector(DB_Tst_Cnt, S_DB_Cnt'length);


DB_without_lpm: IF Use_LPM = 0 GENERATE
BEGIN
P_Debounce:	PROCESS (Reset, Clk) 
	BEGIN
		IF Reset = '1' THEN
			S_DB_In_Sync <= '0';
			S_DB_Cnt <= (Others => '0');
			S_DB_Out <= '0';
		ELSIF clk'EVENT AND clk = '1' THEN
			S_DB_In_Sync <= DB_In;
			IF S_DB_In_Sync = '1' THEN
				IF S_DB_Cnt < S_DB_On_Cnt THEN
					S_DB_Cnt <= S_DB_Cnt + 1;
				ELSE
					S_DB_Out <= '1';
				END IF;
			ELSE
				IF S_DB_Cnt > 0 THEN
					S_DB_Cnt <= S_DB_Cnt - 1;
				ELSE
					S_DB_Out <= '0';
				END IF;
			END IF;
		END IF;
	END PROCESS P_Debounce;
END GENERATE DB_without_lpm;


DB_with_lpm: IF Use_LPM = 1 GENERATE

BEGIN

S_DB_Cnt_Ena <= '1' WHEN ((S_DB_Cnt < S_DB_On_Cnt) AND S_DB_In_Sync = '1') OR ((S_DB_Cnt > 0  AND S_DB_In_Sync = '0')) ELSE '0';

P_Debounce : lpm_counter
	GENERIC MAP (
				lpm_width	=> S_DB_Cnt'length,
				lpm_type	=> "LPM_COUNTER"
				)
	PORT MAP(
			clock	=> clk,
			cnt_en	=> S_DB_Cnt_Ena,
			updown	=> S_DB_In_Sync,
			aclr	=> Reset,
			q		=> S_DB_Cnt
			);

P_DB_Out: PROCESS (Reset, Clk) 
	BEGIN
	IF Reset = '1' THEN
		S_DB_out <= '0';
	ELSIF clk'EVENT AND clk = '1' THEN
			S_DB_In_Sync <= DB_In;
		IF (S_DB_Cnt = S_DB_On_Cnt) AND S_DB_In_Sync = '1' THEN
			S_DB_out <= '1';
		ELSIF (S_DB_Cnt = 0) AND S_DB_In_Sync = '0' THEN
			S_DB_out <= '0';
		END IF;
	END IF;
	END PROCESS P_DB_Out;
	
END GENERATE DB_with_lpm;

DB_Out <= S_DB_Out;

END Arch_Debounce_v2;
