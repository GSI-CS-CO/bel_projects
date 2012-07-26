LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;
LIBRARY lpm;
USE lpm.lpm_components.all;

ENTITY I2C_Cntrl IS
	GENERIC(
			RdWr_I2C_Adr		: INTEGER := 244;
			Clk_in_Hz			: INTEGER := 250000000;
			I2C_Freq_in_Hz		: INTEGER := 400000;
			Use_LPM				: INTEGER := 0;
			Test				: INTEGER := 0
			);
			
	PORT
	(
		Sub_Adr_La			: IN STD_LOGIC_VECTOR(7 downto 1);
		Data_In				: IN STD_LOGIC_VECTOR(15 downto 0);
		Extern_Wr_Activ		: IN STD_LOGIC;
		Extern_Wr_Fin		: IN STD_LOGIC;
		Extern_Rd_Activ		: IN STD_LOGIC;
		Extern_Rd_Fin		: IN STD_LOGIC;
		Powerup_Res			: IN STD_LOGIC;
		Clk					: IN STD_LOGIC;
	 	VG_Mod_ID			: INOUT STD_LOGIC_VECTOR(7 downto 0);
		nSEL_I2C			: OUT STD_LOGIC;
		VG_ID_Out			: OUT STD_LOGIC_VECTOR(7 downto 0);

		I2C_Reg_Rd_Port		: OUT STD_LOGIC_VECTOR(15 downto 0);
		Rd_I2C_Reg_Activ	: OUT STD_LOGIC;
		Dtack_I2C_Reg		: OUT STD_LOGIC
	);
	
END I2C_Cntrl;



ARCHITECTURE Arch_I2C_Cntrl OF I2C_Cntrl IS

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
				lpm_type		: STRING;
				lpm_direction	: STRING;
				lpm_svalue		: STRING
				);
		PORT(
			clock	: IN STD_LOGIC ;
			cnt_en	: IN STD_LOGIC := '1';
			q		: OUT STD_LOGIC_VECTOR (lpm_width-1 DOWNTO 0);
			sset	: IN STD_LOGIC
			);
	END COMPONENT;


	COMPONENT i2c
		GENERIC (DIVISOR : INTEGER);
		PORT(SysClk			: IN STD_LOGIC;
			 clk_en			: IN STD_LOGIC;
			 nReset			: IN STD_LOGIC;
			 Ack_tx			: IN STD_LOGIC;
			 Cmd_stop		: IN STD_LOGIC;
			 Cmd_start		: IN STD_LOGIC;
			 Cmd_send		: IN STD_LOGIC;
			 Cmd_receive	: IN STD_LOGIC;
			 Execute		: IN STD_LOGIC;
			 SDA			: INOUT STD_LOGIC;
			 SCL			: INOUT STD_LOGIC;
			 Din			: IN STD_LOGIC_VECTOR(7 downto 0);
			 Ack_rx			: OUT STD_LOGIC;
			 Status			: OUT STD_LOGIC;
			 DValid			: OUT STD_LOGIC;
			 DEnable		: OUT STD_LOGIC;
			 Busy			: OUT STD_LOGIC;
			 Dout			: OUT STD_LOGIC_VECTOR(7 downto 0)
		);
	END COMPONENT;

	FUNCTION	prod_or_test  (production, test_data, test : INTEGER) RETURN INTEGER IS

		VARIABLE data : INTEGER;
			
		BEGIN
			IF Test = 1 THEN
				data := test_data;
			ELSE
				data := production;
			END IF;

			RETURN data;

		END prod_or_test;


	TYPE	T_Multiplex_SM	IS	(
								ID_Mode,
								Prep_I2C,		
								I2C_Mode,
								Stop_Fin,	
								Prep_ID
								);


	CONSTANT	C_Divisor			: INTEGER := prod_or_test(Clk_in_Hz / I2C_Freq_in_Hz, 8, Test);
	CONSTANT	C_RdWr_I2C			: STD_LOGIC_VECTOR(7 DOWNTO 0) := CONV_STD_LOGIC_VECTOR(RdWr_I2C_Adr, 8);

	SIGNAL		S_Wr_I2C			: STD_LOGIC;
	SIGNAL		S_Wr_I2C_Macro		: STD_LOGIC;
	SIGNAL		S_Rd_I2C			: STD_LOGIC;
	SIGNAL		S_I2C_OUT			: STD_LOGIC_VECTOR(12 DOWNTO 0);
	
	SIGNAL		S_nRESET			: STD_LOGIC;

	SIGNAL		S_Rd_Dtack			: STD_LOGIC;
	SIGNAL		S_Wr_Dtack			: STD_LOGIC;
	SIGNAL		S_Hold_Wr_Dtack		: STD_LOGIC;
	SIGNAL		S_Dtack_I2C_Reg		: STD_LOGIC;
	
	SIGNAL		Multiplex_SM		: T_Multiplex_SM;
	
	CONSTANT	C_Cmd_Receive		: STD_LOGIC_VECTOR(12 DOWNTO 9) := "1000";	
	CONSTANT	C_Cmd_Send			: STD_LOGIC_VECTOR(12 DOWNTO 9) := "0100";	
	CONSTANT	C_Cmd_Start			: STD_LOGIC_VECTOR(12 DOWNTO 9) := "0010";	
	CONSTANT	C_Cmd_Stop			: STD_LOGIC_VECTOR(12 DOWNTO 9) := "0001";	
	ALIAS		Cmd_Bits			: STD_LOGIC_VECTOR(12 DOWNTO 9) IS Data_In(12 DOWNTO 9);
	
	ALIAS		Busy				: STD_LOGIC is S_I2C_OUT(12);
	ALIAS		Status				: STD_LOGIC is S_I2C_OUT(9);

	CONSTANT	Clk_in_ps			: INTEGER	:= 1000000000 / (Clk_in_Hz / 1000);
	
	CONSTANT	C_Wait_in_ns		: INTEGER	:= 620;
	CONSTANT	C_Wait_cnt			: INTEGER	:= (C_Wait_in_ns * 1000 / Clk_in_ps);
	CONSTANT	c_wait_cnt_width 	: INTEGER := How_many_Bits(C_Wait_cnt);
	SIGNAL		S_Wait_Cnt			: STD_LOGIC_VECTOR(c_wait_cnt_width downto 0);
	SIGNAL		S_Wait_Cnt_Ena		: STD_LOGIC;
	SIGNAL		S_Wait_Cnt_Ld		: STD_LOGIC;
	
	SIGNAL		S_VG_ID_Memo		: STD_LOGIC_VECTOR(7 DOWNTO 0);


BEGIN

S_nReset <= '0' WHEN (Multiplex_SM = ID_Mode) ELSE '1';

P_I2C_Adr_Deco:	PROCESS (clk, Powerup_Res)
	BEGIN
		IF Powerup_Res = '1' THEN
			S_Rd_Dtack					<= '0';
			S_Wr_I2C					<= '0';
			S_Rd_I2C					<= '0';
		ELSIF rising_edge(clk) THEN
			S_Rd_Dtack					<= '0';
			S_Wr_I2C					<= '0';
			S_Rd_I2C					<= '0';
			IF Sub_Adr_La = C_RdWr_I2C(7 DOWNTO 1) THEN
				IF Extern_Rd_Activ = '1' THEN
					S_Rd_I2C	<= '1';
					S_Rd_Dtack	<= '1';
				END IF;
				IF Extern_Wr_Activ = '1' THEN
					S_Wr_I2C	<= '1';
				END IF;
			END IF;
		END IF;
	END PROCESS P_I2C_Adr_Deco;


P_I2C_ID_Mux:	PROCESS (CLK, Powerup_Res)

	-- Der I2C-Bus benötigt zwei bidirektionale Leitungen. Beide Leitungen sind bei der Entwicklung der Kicker-Counter-Karte	--
	-- vorgesehen worden. Leider befindet sich das Signal 'SDA' auf der D-Reihe des Modul-Busses, d. h. die I2C-Funktion wäre	--
	-- nur für eine Karte mit 160-Poligen Stecker verfügbar. Um den I2C-Bus auch für einen Modul-Bus mit 96-Poligen Stecker		--
	-- bereitzustellen ist einiges an Aufwand nötig, da im 96-Poligen Bereich des Modul-Busses kein Pin mehr frei ist, der		--
	-- nur direkt zur Kicker-Counter-Karte geht. Aus diesem Grund müssen die beiden I2C-IOs mit zwei anderen Signalen des 96	--
	-- Poligen Modul-Busses gemultiplext werden. Diese Funktion muss auf der Adapterkarte bereitgestellt werden. Als Umschalt-	--
	-- signal für diesen Multiplexer wird der Pin verwendet, der ursprünglich mit dem 'I2C-SLC'-Signal belegt war. Dies ist ja	--
	-- der einzige freie Pin im 96-Pol. Modul-Bus-Layout. Aus dem 'I2C-SLC' wird Chipintern das Umschaltsignal 'nSEL_I2C'.		--
	-- Der Ausgangspin behält seinen irreführenden Namen 'I2C-SLC' da die Pinvergabe aus dem Orcad-Design entnommen wird und	--
	-- für die Kicker-Counter-Karte noch keine Layout-Änderung nötig war, und deshalb auch der Signalname nicht geändert wurde.	--
	--																															--
	-- Die Multiplexer-Umschaltung muss folgendes berücksichtigen:																--
	--																															--
	-- 	   Die Umschalt-Sequenz in den I2C-Mode wird nur einmalig durchlaufen. Sie wird mit dem Absetzen eines Start-Kommandos	--
	--     veranlasst. Der I2C-Mode bleibt dann entweder bis zum Absetzen eines gültigen Stop-Kommandos, oder durch erreichen	--
	--     eines Timeouts, aktiv. Alle anderen Kommandos (auch weitere Start-Kommandos) werden direkt an den I2C-Macro			--
	--     weitergeleitet.																										--
	--																															--
	--     Die Umschaltung in den I2C-Mode erfolgt in der nachfolgend beschriebenen Sequenz.									--
	--																															--
	--		| Der I2C-Mode ist noch nicht geschaltet und vom		| 														|	--	
	--		| Modul-Bus wird ein 'Control Byte' (Start-Kommando)	| 														|	--
	--		| auf die Subadresse I2C-Wr geschrieben. Während des 	| 														|	--
	--		| Schreibens wird das Busy-Flag des I2C-Macros			| 														|	--
	--		| geprüft, nur wenn er frei ist (Busy = '0') akzeptiert	| 														|	--
	--		| der I2C-Macro Kommandos, und nur dann wird nach der	| 														|	--
	--      | Umschaltung ein Dtack zum Modul-Bus erzeugt.			|														|	--
	--		|														| nSEL_I2C geht auf Low Level.							|	--
	--		|														| Die Timeout-Überwachung wird gestartet.				|	--
	--		|														| Die VG-ID-Bits[7..6] werden gespeichert.				|	--
	--		|														| Eine Microsekunde warten, bis die Analogschalter von	|	--
	--		|														| den VG-ID-Bits[7..6] auf die I2C-Signale 'I2C-SLC'	|	--
	--		|														| und 'I2C-SDA' umgeschaltet haben.						|	--
	--		|														| Erst jetzt darf das I2C-Wr am I2C-Macro aktiv werden,	|	--
	--		|														| und das Dtack zum Modul-Bus erzeugt werden.			|	--
	--		| Ab jetzt wird jedes Schreib- oder Lesekommando		|														|	--
	--		| direkt an den I2C-Macro weitergegeben. Ein Dtack		|														|	--
	--		| wird nur erzeugt, wenn der Macro auch frei für		|														|	--
	--		| Kommandos ist!										|														|	--
	--																															--
	--     Zurückgeschaltet in den Standard-Mode wird durch das gültige Absetzen eines Stopp-Kommandos, oder durch einen		--
	--     Timeout. Nach dem Zurückschalten werden die VG-ID-Bits[7..6] wieder direkt gelesen. Die 'I2C-SLC'- 'I2C-SDA'-		--
	--     Signale sind nur noch mit Pullup-Widerständen verbunden.																--
	--     																														--
	--     Die Umschaltung in den Standard-Mode erfolgt in der nachfolgend beschriebenen Sequenz.								--
	--     																														--
	--		| Wenn I2C-Mode eingeschaltet und der I2C-Macro frei 	| 														|	--	
	--		| ist, wird das Stopp-Kommando an den I2C-Macro 		|														|	--
	--		| weitergegeben und nur dann gibt es ein Dtack zum		|														|	--
	--		| Modul-Bus.											| Ist das Stopp-Kommando durch den I2C-Macro ausgeführt	|	--
	--		| 														| ausgeführt worden (das Status-Bit wird '0'), geht		|	--
	--		|														| nSEL_I2C geht auf High Level.							|	--
	--		|														| Eine Microsekunde warten, bis die Analogschalter von	|	--
	--		|														| den I2C-Signalen 'I2C-SLC' und 'I2C-SDA' auf die		|	--
	--		|														| VG-ID-Bits[7..6] umgeschaltet haben.					|	--
	--		|														| Für den ID-Vergleich wieder die ID-Bits[7..6] direkt	|	--
	--		|														| verwenden.

	BEGIN

	IF (Powerup_Res = '1') THEN
			Multiplex_SM <= ID_Mode;
			 
	ELSIF rising_edge(clk) THEN
		
		S_Wr_I2C_Macro <= '0';
		S_Wr_Dtack	<= '0';
	
	 	CASE Multiplex_SM IS

		   	WHEN ID_Mode =>
				IF (S_Wr_I2C = '1') AND (Cmd_Bits = C_Cmd_Start) AND (Busy = '0') THEN 
					Multiplex_SM <= Prep_I2C;
				END IF;

			WHEN Prep_I2C =>
				IF S_Wait_Cnt(S_Wait_Cnt'high) = '1' THEN
					Multiplex_SM <= I2C_Mode;
				END IF;
					
			WHEN I2C_Mode =>
				IF (S_Wr_I2C = '1') AND (Cmd_Bits = C_Cmd_Start) AND (Busy = '0') THEN 
					S_Wr_I2C_Macro <= '1';
					S_Wr_Dtack	<= '1';
				END IF;
				IF (S_Wr_I2C = '1') AND (Cmd_Bits = C_Cmd_Send) AND (Busy = '0') THEN 
					S_Wr_I2C_Macro <= '1';
					S_Wr_Dtack	<= '1';
				END IF;
				IF (S_Wr_I2C = '1') AND (Cmd_Bits = C_Cmd_Receive) AND (Busy = '0') THEN
					S_Wr_I2C_Macro <= '1';
					S_Wr_Dtack	<= '1';
				END IF;
				IF (S_Wr_I2C = '1') AND (Cmd_Bits = C_Cmd_Stop) AND (Busy = '0') THEN
					S_Wr_I2C_Macro <= '1';
					Multiplex_SM <= Stop_Fin;
				END IF;
				
			WHEN Stop_Fin =>
				IF (S_Wr_I2C = '1') AND (Cmd_Bits = C_Cmd_Stop) AND (Status = '0') THEN
					S_Wr_Dtack	<= '1';
					Multiplex_SM <= Prep_ID;
				END IF;
				
			WHEN Prep_ID =>
				IF S_Wait_Cnt(S_Wait_Cnt'high) = '1' THEN
					Multiplex_SM <= ID_Mode;
				END IF;

			WHEN OTHERS =>
				Multiplex_SM <= ID_Mode;
				
		END CASE;
	END IF;

	END PROCESS P_I2C_ID_Mux;
	
S_Wait_Cnt_Ld <=  '1'	WHEN (Multiplex_SM = ID_Mode) OR (Multiplex_SM = I2C_Mode) ELSE '0';
	
S_Wait_Cnt_Ena <=  '1'	WHEN ((Multiplex_SM = Prep_ID) OR (Multiplex_SM = Prep_I2C)) AND (S_Wait_Cnt(S_Wait_Cnt'high) = '0') ELSE '0';


Wait_without_lpm: IF Use_LPM = 0 GENERATE --------------------------------------------------------------------------------------------

	P_Wait: PROCESS (CLK, Powerup_Res)
		BEGIN
		IF  Powerup_Res = '1' THEN
			S_Wait_Cnt <= conv_std_logic_vector(C_Wait_cnt, S_Wait_Cnt'length);
		ELSIF rising_edge(clk) THEN
			IF (S_Wait_Cnt_Ld = '1') THEN
				S_Wait_Cnt <= conv_std_logic_vector(C_Wait_cnt, S_Wait_Cnt'length);
			ELSIF (S_Wait_Cnt_Ena = '1') THEN
				S_Wait_Cnt <= S_Wait_Cnt - 1;
			END IF;
		END IF;
		END PROCESS P_Wait;

END GENERATE wait_without_lpm; -------------------------------------------------------------------------------------------------------


Wait_with_lpm: IF Use_LPM = 1 GENERATE -----------------------------------------------------------------------------------------------

	wait_lpm_cnt : lpm_counter
		GENERIC MAP (
					lpm_width	=> S_Wait_Cnt'length,
					lpm_type	=> "LPM_COUNTER",
					lpm_direction => "DOWN",
					lpm_svalue	=> integer'image(C_Wait_cnt)
					)
		PORT MAP(
				clock	=> clk,
				sset	=> S_Wait_Cnt_Ld,
				cnt_en	=> S_Wait_Cnt_Ena,
				q		=> S_Wait_Cnt
				);

END GENERATE Wait_with_lpm; ----------------------------------------------------------------------------------------------------------


P_VG_ID_Memo:	PROCESS (clk)
	BEGIN
	IF rising_edge(clk) THEN
		IF Multiplex_SM = ID_Mode THEN
			S_VG_ID_Memo <= VG_Mod_ID;
		END IF;
	END IF;
	END PROCESS P_VG_ID_Memo;
	
VG_ID_Out <= S_VG_ID_Memo;


P_RD_I2C:	PROCESS (clk)
	BEGIN
	IF rising_edge(clk) THEN
		IF S_Rd_I2C = '0' THEN
			I2C_Reg_Rd_Port <= ('0' & '0' & '0' & S_I2C_OUT(12) & '0' & S_I2C_OUT(10 DOWNTO 0));
		END IF;
	END IF;
	END PROCESS P_RD_I2C;
	
	
P_Wr_Dtack:	PROCESS (clk, Powerup_Res)
	BEGIN
	IF Powerup_Res = '1' THEN
		S_Hold_Wr_Dtack <= '0';
	ELSIF rising_edge(clk) THEN
		IF S_Wr_Dtack = '1' THEN
			S_Hold_Wr_Dtack <= '1';
		ELSIF Extern_Wr_Activ = '0' THEN
			S_Hold_Wr_Dtack <= '0';
		END IF;
	END IF;
	END PROCESS P_Wr_Dtack;

	
b2v_i2c : i2c
GENERIC MAP(DIVISOR => C_Divisor)
PORT MAP(SysClk			=> clk,
		 nReset			=> S_nRESET,
		 clk_en			=> '1',
		 Cmd_receive	=> Data_In(12),
		 Cmd_send		=> Data_In(11),
		 Cmd_start		=> Data_In(10),
		 Cmd_stop		=> Data_In(9),
		 Ack_tx			=> Data_In(8),
		 Din			=> Data_In(7 DOWNTO 0),
		 Execute		=> S_Wr_I2C_Macro,
		 SDA			=> VG_Mod_ID(6),
		 SCL			=> VG_Mod_ID(7),
		 Busy			=> S_I2C_OUT(12),
		 DEnable		=> S_I2C_OUT(11),
		 DValid			=> S_I2C_OUT(10),
		 Status			=> S_I2C_OUT(9),
		 Ack_rx			=> S_I2C_OUT(8),
		 Dout			=> S_I2C_OUT(7 DOWNTO 0)
		);

nSEL_I2C <= '0' WHEN (Multiplex_SM = Prep_ID) OR (Multiplex_SM = ID_Mode) ELSE '1';

Rd_I2C_Reg_Activ <= S_Rd_I2C;

S_Dtack_I2C_Reg <= S_Hold_Wr_Dtack OR S_Rd_Dtack;

Dtack_I2C_Reg <= S_Dtack_I2C_Reg;

END Arch_I2C_Cntrl;