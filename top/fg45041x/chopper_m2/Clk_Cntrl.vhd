LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;

ENTITY Clk_Cntrl IS
	GENERIC(Test : INTEGER := 0);
	PORT
	(
		Logic				: IN STD_LOGIC_VECTOR(5 downto 0);
		Skal_Okay 			: IN STD_LOGIC;
		Master_Clk 			: IN STD_LOGIC;
		Extern_Clk 			: IN STD_LOGIC;
		Reset				: IN STD_LOGIC;
		nMaster_Clk_Ena		: OUT STD_LOGIC;
		nSlave_Clk_Ena		: OUT STD_LOGIC;
		nIndenpend_Clk_Ena	: OUT STD_LOGIC;
		Extern_Clk_OK		: OUT STD_LOGIC
	);
END Clk_Cntrl;


ARCHITECTURE Arch_Clk_Cntrl OF Clk_Cntrl IS


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

	
	CONSTANT	C_Max_Cnt		: INTEGER := 10000000-2;
	CONSTANT	C_Max_Tst_Cnt	: INTEGER := 1000-2;
	
	
	SIGNAL	S_Master_Cnt		: STD_LOGIC_VECTOR(How_many_Bits(C_Max_Cnt) DOWNTO 0);
	SIGNAL	S_Extern_Cnt		: STD_LOGIC_VECTOR(S_Master_Cnt'Range);
	SIGNAL	S_Sync1_Extern_Cnt	: STD_LOGIC_VECTOR(S_Master_Cnt'Range);
	SIGNAL	S_Sync2_Extern_Cnt	: STD_LOGIC_VECTOR(S_Master_Cnt'Range);
	SIGNAL	S_Diff				: STD_LOGIC_VECTOR(S_Master_Cnt'Range);
	SIGNAL	S_Extern_Clk_OK		: STD_LOGIC;
	SIGNAL	S_Sync_Mess			: STD_LOGIC;
	
	SIGNAL	S_Max_Cnt			: STD_LOGIC_VECTOR(S_Master_Cnt'Range);
	
BEGIN


S_Max_Cnt 	<= CONV_STD_LOGIC_VECTOR(C_Max_Cnt, S_Master_Cnt'Length) WHEN Test = 0 ELSE CONV_STD_LOGIC_VECTOR(C_Max_Tst_Cnt, S_Master_Cnt'Length);


P_Clk_Sel:	PROCESS (Logic, Skal_Okay)
	BEGIN
		CASE Logic IS
			WHEN "000001" =>					-- Karte soll Master_Clk liefern
				IF Skal_Okay = '1' THEN	
					nMaster_Clk_Ena		<= '0';
					nSlave_Clk_Ena		<= '1';
					nIndenpend_Clk_Ena	<= '0';
				ELSE							-- interne Clock
					nMaster_Clk_Ena		<= '1';
					nSlave_Clk_Ena		<= '1';
					nIndenpend_Clk_Ena	<= '0';
				END IF;
			WHEN "000010" =>					-- Karte verwendet Slave_Clk
				IF Skal_Okay = '1' THEN
					nMaster_Clk_Ena		<= '1';
					nSlave_Clk_Ena		<= '0';
					nIndenpend_Clk_Ena	<= '1';
				ELSE							-- interne Clock
					nMaster_Clk_Ena		<= '1';
					nSlave_Clk_Ena		<= '1';
					nIndenpend_Clk_Ena	<= '0';
				END IF;
			WHEN OTHERS =>						-- Allen anderen Faellen interne Clock
					nMaster_Clk_Ena		<= '1';
					nSlave_Clk_Ena		<= '1';
					nIndenpend_Clk_Ena	<= '0';
		END CASE;
	END PROCESS P_Clk_Sel;


P_Master_Clk:	PROCESS (Master_Clk, Reset, S_Max_Cnt)
	BEGIN
	IF Reset = '1' THEN
		S_Master_Cnt 		<= S_Max_Cnt;
		S_Sync1_Extern_Cnt	<= (OTHERS => '0');
		S_Sync2_Extern_Cnt	<= (OTHERS => '0');
		S_Diff				<= (OTHERS => '0');
		S_Extern_Clk_OK		<= '0';
	ELSIF rising_edge(Master_Clk) THEN
		S_Sync1_Extern_Cnt <= S_Extern_Cnt;						-- erstes syncronisieren
		S_Sync2_Extern_Cnt <= S_Sync1_Extern_Cnt;				-- zweites syncronisieren
		IF S_Master_Cnt(S_Master_Cnt'left) = '1' THEN
			IF S_Sync2_Extern_Cnt > S_Max_Cnt THEN
				S_Diff <= S_Sync2_Extern_Cnt - S_Max_Cnt;
			ELSE
				S_Diff <=  S_Max_Cnt - S_Sync2_Extern_Cnt;
			END IF;
			S_Master_Cnt <= S_Max_Cnt;
		ELSE
			IF S_Diff > 25 THEN
				S_Extern_Clk_OK <= '0';
			ELSE
				S_Extern_Clk_OK <= '1';
			END IF;
			S_Master_Cnt <= S_Master_Cnt - 1;
		END IF;
	END IF;
	END PROCESS P_Master_Clk;
	

P_Extern_Clk:	PROCESS	(Extern_Clk, Reset)
	BEGIN
	IF Reset = '1' THEN
		S_Extern_Cnt	<= CONV_STD_LOGIC_VECTOR( 1, S_Master_Cnt'Length);
		S_Sync_Mess		<= '0';
	ELSIF rising_edge(Extern_Clk) THEN
		S_Sync_Mess <= S_Master_Cnt(S_Master_Cnt'left);
		IF S_Sync_Mess = '1' THEN
			S_Extern_Cnt <=  CONV_STD_LOGIC_VECTOR( 2, S_Master_Cnt'Length);
		ELSE
			S_Extern_Cnt <= S_Extern_Cnt + 1;
		END IF;
	END IF;
	END PROCESS P_Extern_Clk;

Extern_Clk_OK <= S_Extern_Clk_OK;
	
END Arch_Clk_Cntrl;
