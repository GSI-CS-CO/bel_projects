LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;

ENTITY K12_K23_Logik_Leds IS
	GENERIC(Test : INTEGER := 0);
	PORT
	(
		VG_Mod_Skal :		IN STD_LOGIC_VECTOR(7 downto 0);
		St_160_Skal :		IN STD_LOGIC_VECTOR(7 downto 0);
		Logik :				IN STD_LOGIC_VECTOR(5 downto 0);
		Logik_Aktiv :		IN STD_LOGIC;
		Test_Vers_Aktiv :	IN STD_LOGIC;
		Live_LED_In0 :		IN STD_LOGIC;
		Live_LED_In1 :		IN STD_LOGIC;
		Live_LED_In2 :		IN STD_LOGIC;
		Live_LED_In3 :		IN STD_LOGIC;
		Live_LED_In4 :		IN STD_LOGIC;
		Live_LED_In5 :		IN STD_LOGIC;
		Live_LED_In6 :		IN STD_LOGIC;
		Live_LED_In7 :		IN STD_LOGIC;
		Ena :				IN STD_LOGIC;
		Sel :				IN STD_LOGIC_VECTOR(1 downto 0);
		clk :				IN STD_LOGIC;
		nLED_Skal :			OUT STD_LOGIC_VECTOR(7 downto 0)
	);
END K12_K23_Logik_Leds;


ARCHITECTURE Arch_K12_K23_Logik_Leds OF K12_K23_Logik_Leds IS

	COMPONENT	Signal_Stretch
		GENERIC (cnt: INTEGER);
		PORT(
			Sig_in :			IN		STD_LOGIC;
			Stretch_on :		IN		STD_LOGIC;
			Ena_Cnt :			IN		STD_LOGIC;
			clk :				IN		STD_LOGIC;
			Sig_out :			OUT		STD_LOGIC
			);
	END COMPONENT;


	CONSTANT	C_Ena_Time_in_ms		: INTEGER := 15;
	CONSTANT	C_Return_To_Live_Cnt	: INTEGER := 5000 / C_Ena_Time_in_ms;
	

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
	

	SIGNAL		S_LED					: STD_LOGIC_VECTOR(7 DOWNTO 0);
	SIGNAL		S_Return_To_Live		: STD_LOGIC_VECTOR(How_many_bits(C_Return_To_Live_Cnt)+1 DOWNTO 0);
	
	SIGNAL		Sel_Old					: STD_LOGIC_VECTOR(1 DOWNTO 0);		-- Merker um betaetigten Schalter zu erkennen.

	SIGNAL		S_sel					: STD_LOGIC_VECTOR(2 DOWNTO 0);		-- Das hoechste Bit = 1 bedeutet auf Live_Sigale umschalten. 

	SIGNAL		S_Live_LED_Out			: STD_LOGIC_VECTOR(7 DOWNTO 0);
	SIGNAL		S_Live_LED_In			: STD_LOGIC_VECTOR(7 DOWNTO 0);
		
	BEGIN

S_Live_LED_In <= (	Live_LED_In7 & Live_LED_In6 & Live_LED_In5 & Live_LED_In4
				  & Live_LED_In3 & Live_LED_In2 & Live_LED_In1 & Live_LED_In0
				 );

S_sel <= (S_Return_To_Live(S_Return_To_Live'left) & sel(1 downto 0));

P_Led_Mux:	PROCESS (clk)
	BEGIN
	IF clk'EVENT AND clk = '1' THEN
		CASE S_sel IS
			WHEN "010" =>
				S_LED <= VG_Mod_Skal;
			WHEN "011" =>
				S_LED <= St_160_Skal;
			WHEN "001" =>
				S_LED <= (Logik_Aktiv & Test_Vers_Aktiv & Logik);
			WHEN OTHERS =>
				S_LED <= S_Live_LED_Out;
			END CASE;
		END IF;
	END PROCESS P_Led_Mux;


P_Return_To_Live:	PROCESS (Clk)
	BEGIN
	IF rising_edge(clk) THEN
		Sel_Old <= Sel;
		IF Sel /= Sel_Old THEN
			IF Test = 0 THEN
				S_Return_To_Live <= conv_std_logic_vector(C_Return_To_Live_Cnt, S_Return_To_Live'length);
			ELSE
				S_Return_To_Live <= conv_std_logic_vector(50, S_Return_To_Live'length);
			END IF;
		ELSIF S_Return_To_Live(S_Return_To_Live'left) = '0' AND (Ena = '1' OR Test = 1) THEN
			S_Return_To_Live <= S_Return_To_Live - 1;
		END IF;
	END IF;
	END PROCESS P_Return_To_Live;

	
Sig_Str7:	Signal_Stretch
GENERIC MAP(cnt => 4)
PORT MAP(
		Sig_in 		=>	S_Live_LED_In(7),
		Stretch_on 	=>	'0',
		Ena_Cnt 	=>	Ena,
		clk 		=>	clk,
		Sig_out		=>	S_Live_LED_Out(7)
		);

Sig_Str6:	Signal_Stretch
GENERIC MAP(cnt => 4)
PORT MAP(
		Sig_in 		=>	S_Live_LED_In(6),
		Stretch_on 	=>	'0',
		Ena_Cnt 	=>	Ena,
		clk 		=>	clk,
		Sig_out		=>	S_Live_LED_Out(6)
		);
		
Sig_Str5:	Signal_Stretch
GENERIC MAP(cnt => 4)
PORT MAP(
		Sig_in 		=>	S_Live_LED_In(5),
		Stretch_on 	=>	'0',
		Ena_Cnt 	=>	Ena,
		clk 		=>	clk,
		Sig_out		=>	S_Live_LED_Out(5)
		);
		
Sig_Str4:	Signal_Stretch
GENERIC MAP(cnt => 4)
PORT MAP(
		Sig_in 		=>	S_Live_LED_In(4),
		Stretch_on 	=>	'0',
		Ena_Cnt 	=>	Ena,
		clk 		=>	clk,
		Sig_out		=>	S_Live_LED_Out(4)
		);
		
Sig_Str3:	Signal_Stretch
GENERIC MAP(cnt => 4)
PORT MAP(
		Sig_in 		=>	S_Live_LED_In(3),
		Stretch_on 	=>	'0',
		Ena_Cnt 	=>	Ena,
		clk 		=>	clk,
		Sig_out		=>	S_Live_LED_Out(3)
		);
		
Sig_Str2:	Signal_Stretch
GENERIC MAP(cnt => 4)
PORT MAP(
		Sig_in 		=>	S_Live_LED_In(2),
		Stretch_on 	=>	'0',
		Ena_Cnt 	=>	Ena,
		clk 		=>	clk,
		Sig_out		=>	S_Live_LED_Out(2)
		);
		
Sig_Str1:	Signal_Stretch
GENERIC MAP(cnt => 4)
PORT MAP(
		Sig_in 		=>	S_Live_LED_In(1),
		Stretch_on 	=>	'0',
		Ena_Cnt 	=>	Ena,
		clk 		=>	clk,
		Sig_out		=>	S_Live_LED_Out(1)
		);
		
Sig_Str0:	Signal_Stretch
GENERIC MAP(cnt => 4)
PORT MAP(
		Sig_in 		=>	S_Live_LED_In(0),
		Stretch_on 	=>	'1',
		Ena_Cnt 	=>	Ena,
		clk 		=>	clk,
		Sig_out		=>	S_Live_LED_Out(0)
		);
		
P_Open_drain:	PROCESS	(S_LED)
	BEGIN
		L1: FOR i IN 0 TO 7 LOOP
			IF S_LED(i) = '0' THEN
				nLED_Skal(i) <= 'Z';
			ELSE
				nLED_Skal(i) <= '0';
			END IF;
		END LOOP L1;
	END PROCESS P_open_drain;
	
--nLED_Skal <= S_LED;
--nLED_Skal(0) <= S_nLED(0) WHEN S_Return_To_Live(S_Return_To_Live'left) = '0' ELSE S_Live_nLed;
--nLED_Skal(7 DOWNTO 1) <= S_nLED(7 DOWNTO 1);

END Arch_K12_K23_Logik_Leds;
