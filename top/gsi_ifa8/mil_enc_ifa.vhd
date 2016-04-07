--TITLE "Manchester_II Encoder in VHDL, W. Panschow Vers.03  d. 20.09.2010";


library IEEE;
USE IEEE.STD_LOGIC_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;

ENTITY mil_enc_ifa IS

	PORT(
		Mil_TRM_D		: IN STD_LOGIC_VECTOR(15 DOWNTO 0);		-- solange 'Mil_TRM' aktiv ist muß hier das zu sendende Datum anliegen.					-- 
		Cmd_Trm			: IN STD_LOGIC;							-- Cmd_Trm = Eins während 'Wr_Mil' aktiv => ein Command-Sync. wird erzeugt, sonst		--
																-- wird ein Data-Sync. generiert.														--
		Wr_Mil			: IN STD_LOGIC;							-- Startet ein Mil-Send, muß mindestens 1 Takt aktiv sein.								--
		CLK				: IN STD_LOGIC;							-- Die Frequenz muß mindestens 4 MHz betragen.											--
		Ena_Every_500ns : IN STD_LOGIC;							-- V03: Muss von CLK abgeleitet sein! Für Standard-Speed								--
		Ena_Every_250ns : IN STD_LOGIC;							-- V03: Muss von CLK abgeleitet sein! Für High-Speed									--
		Standard_Speed	: IN STD_LOGIC;							-- V03: Standard-Speed = 1 us für ein Datenbit => Ena_Every_500ns verwenden				--
		Reset			: IN STD_LOGIC;							-- Die Ablaufsteuerung 'Mil_TRM_SM' zurückgesetzt, unterbricht ein laufendes Mil-Send.	--
		nMil_Out_Pos	: OUT STD_LOGIC;						-- Der positive Bipolare Ausgang ist null-aktiv.										--
		nMil_Out_Neg	: OUT STD_LOGIC;						-- Der negative Bipolare Ausgang ist null-aktiv.										--
		nSel_Mil_Drv	: OUT STD_LOGIC;						-- Soll die ext. bipolaren Treiber von 'nMil_Out_Pos(_Neg)' einschalten (null-aktiv).	--
		Mil_Rdy_4_Wr	: OUT STD_LOGIC;						-- Das Sende-Register ist frei.															--
		SD				: OUT STD_LOGIC							-- V02: Bildet das Signal "SD" des 6408-ICs nach, wird für den Blockmode der Interfacekarte benötigt.	--
		);
		
END mil_enc_ifa;

ARCHITECTURE Arch_mil_enc_vhdl OF mil_enc_ifa IS

TYPE	T_Encode_SM	IS
					(
					TRM_Idle,
					Sync1,
					Sync2,
					Sync3,
					Sync4,
					Sync5,
					Sync6,
					IDLE_LO,
					IDLE_HI,
					TRM_LO,
					TRM_HI
					);

SIGNAL	S_Encode_SM	: T_Encode_SM;

SIGNAL	S_Shift_Reg			: STD_LOGIC_VECTOR(15 DOWNTO 0);	-- In 'shift' wird das Sendedatum[15..0] in		--
																-- einen seriellen Bitstrom umgewandelt.		--

SIGNAL	S_Bit_cnt			: STD_LOGIC_VECTOR(4 DOWNTO 0);		-- Der Zähler bestimmt das Ende des seriellen	--
																-- Datenstromes (inklusive Parity-Bit).		 	--
																		
SIGNAL	S_Odd_Parity		: STD_LOGIC;

SIGNAL	S_Wr_Mil_Merker		: STD_LOGIC;

SIGNAL	S_Mil_Rdy_4_Wr_Dly	: STD_LOGIC_VECTOR(3 DOWNTO 0);		-- V03: Damit zwischen zwei TRMs mindestens 1 us Pause gemacht wird. 

SIGNAL	S_Cmd_Trm			: STD_LOGIC;

SIGNAL	S_Out_pos			: STD_LOGIC;
SIGNAL	S_Out_neg			: STD_LOGIC;

SIGNAL	S_Bipol_Out_Pos		: STD_LOGIC;	-- Der bipolare positive Ausgang ist ein getaktetes Register, damit werden etwaige	--
											-- Übergangsfunktionen von 'Out_Pos' unterdrückt.									--
SIGNAL	S_Bipol_Out_Neg		: STD_LOGIC;	-- Der bipolare negative Ausgang ist ein getaktetes Register, damit werden etwaige	--
											-- Übergangsfunktionen von 'Out_Neg' unterdrückt.									--

SIGNAL	S_Sel_Mil_Drv		: STD_LOGIC;	-- Sind 'Out_Pos' und 'Out_Neg' ungleich in ihrem Logikpegel werden die Mil-Treiber aktiviert.	--
											-- Damit etwaige Übergangsfunktionen weggefiltert werden ist das Register getaktet.				--

SIGNAL	S_SD				: STD_LOGIC;	-- Bildet das Signal "SD" des 6408-ICs nach, wird für den Blockmode der Interfacekarte benötigt.	--

SIGNAL	Ena_2x_TRM_Bitrate	: STD_LOGIC;	-- Multiplexer für Ena_Every_500ns (Standard-Speed) oder Ena_Every_250ns (High-Speed)			--

BEGIN

Ena_2x_TRM_Bitrate <= Ena_Every_500ns WHEN Standard_Speed = '1' ELSE Ena_Every_250ns;

P_Cmd_Trm:	PROCESS (clk, Reset)		
	BEGIN
		IF Reset = '1' THEN
			S_Cmd_Trm <= '0';
		ELSIF rising_edge(clk) THEN
			IF Wr_Mil = '1' THEN
				S_Cmd_Trm <= Cmd_Trm;
			END IF;
		END IF;
	END PROCESS P_Cmd_Trm;


P_Odd_Parity:	PROCESS (clk, Reset)		
	BEGIN
		IF Reset = '1' THEN
			S_Odd_Parity <= '0';
		ELSIF rising_edge(clk) THEN
			IF (S_Encode_SM = Idle_HI AND Ena_2x_TRM_Bitrate = '1') OR (S_Encode_SM = Trm_Idle AND S_Odd_Parity = '1') THEN
				S_Odd_Parity <= not S_Odd_Parity;
			END IF;
		END IF;
	END PROCESS P_Odd_Parity;


P_Wr_Mil_Merker:	PROCESS (clk, Reset)		
	BEGIN
		IF Reset = '1' THEN
			S_Wr_Mil_Merker <= '0';
		ELSIF rising_edge(clk) THEN
			IF S_Encode_SM = Sync1 THEN
				S_Wr_Mil_Merker <= '0';
			ELSIF Wr_Mil = '1' THEN
				S_Wr_Mil_Merker <= '1';
			END IF;
		END IF;
	END PROCESS P_Wr_Mil_Merker;


P_Shift_Reg:	PROCESS (clk, Reset)		
	BEGIN
		IF Reset = '1' THEN
			S_Shift_Reg <= (OTHERS => '0');
		ELSIF rising_edge(clk) THEN
			IF Wr_Mil = '1' THEN
				S_Shift_Reg <= Mil_TRM_D;
			ELSIF (S_Encode_SM = Idle_LO OR S_Encode_SM = Idle_HI) AND Ena_2x_TRM_Bitrate = '1' THEN
				S_Shift_Reg <= (S_Shift_Reg(S_Shift_Reg'high-1 DOWNTO 0) & '0');
			END IF;
		END IF;
	END PROCESS P_Shift_Reg;


P_Bit_cnt:	PROCESS (clk, Reset)		
	BEGIN
		IF Reset = '1' THEN
			S_Bit_cnt <= (OTHERS => '0');
		ELSIF rising_edge(clk) THEN
			IF S_Encode_SM = TRM_Idle THEN
				S_Bit_cnt <= (OTHERS => '0');
			ELSIF (S_Encode_SM = Idle_LO OR S_Encode_SM = Idle_HI) AND Ena_2x_TRM_Bitrate = '1' THEN
				S_Bit_cnt <= S_Bit_cnt + 1;
			END IF;
		END IF;
	END PROCESS P_Bit_cnt;


P_Encode_SM:	PROCESS (clk, Reset)		
	BEGIN
		IF Reset = '1' THEN
			S_Encode_SM <= TRM_Idle;
			S_Out_pos <= '0';
			S_Out_neg <= '0';
			S_SD <= '0';									-- V02

		ELSIF rising_edge(clk) THEN
			IF Ena_2x_TRM_Bitrate = '1' THEN
			
				S_Out_pos <= '0';
				S_Out_neg <= '0';
			
				CASE S_Encode_SM IS
					WHEN TRM_Idle =>
						S_SD <= '0';									-- V02
						IF S_Wr_Mil_Merker = '1' AND Wr_Mil = '0' THEN	-- Wr_Mil muss abgeschlossen sein sonst shifted das Shift-Register wärend	--
							S_Encode_SM <= Sync1;						-- ladens, weil Trm_Idle schon während Wr_Mil verlassen würde.				--
						END IF;

					WHEN Sync1 =>
						S_Out_pos <= S_Cmd_Trm;
						S_Out_neg <= NOT S_Cmd_Trm;
						S_Encode_SM <= Sync2;
					WHEN Sync2 =>
						S_Out_pos <= S_Cmd_Trm;
						S_Out_neg <= NOT S_Cmd_Trm;
						S_Encode_SM <= Sync3;
					WHEN Sync3 =>
						S_Out_pos <= S_Cmd_Trm;
						S_Out_neg <= NOT S_Cmd_Trm;
						S_Encode_SM <= Sync4;

					WHEN Sync4 =>
						S_Out_pos <= NOT S_Cmd_Trm;
						S_Out_neg <= S_Cmd_Trm;
						S_Encode_SM <= Sync5;
					WHEN Sync5 =>
						S_Out_pos <= NOT S_Cmd_Trm;
						S_Out_neg <= S_Cmd_Trm;
						S_Encode_SM <= Sync6;
					WHEN Sync6 =>
						S_Out_pos <= NOT S_Cmd_Trm;
						S_Out_neg <= S_Cmd_Trm;
						S_SD <= '1';									-- V02
						IF S_Shift_Reg(S_Shift_Reg'high) = '1' THEN
							S_Encode_SM <= Idle_HI;
						ELSE
							S_Encode_SM <= Idle_LO;
						END IF;

					WHEN IDLE_LO =>
						S_Out_pos <= '0';
						S_Out_neg <= '1';
						S_Encode_SM <= TRM_LO;

					WHEN TRM_LO =>
						S_Out_pos <= '1';
						S_Out_neg <= '0';
						IF S_Bit_cnt < 16 THEN
							IF S_Shift_Reg(S_Shift_Reg'high) = '1' THEN
								S_Encode_SM <= Idle_HI;
							ELSE
								S_Encode_SM <= Idle_LO;
							END IF;
						ELSIF S_Bit_cnt = 16 THEN
							S_SD <= '0';								-- V02
							IF S_Odd_Parity = '0' THEN					-- Das Odd-Parity-Bit zählt selbst mit bei der Parity-Bildung,	--
								S_Encode_SM <= Idle_HI;					-- deshalb wird bei Parity == GND nach Idle_HI verzweigt.		--
							ELSE
								S_Encode_SM <= Idle_LO;
							END IF;
						ELSE
							S_Encode_SM <= TRM_Idle;
						END IF;

					WHEN IDLE_HI =>
						S_Out_pos <= '1';
						S_Out_neg <= '0';
						S_Encode_SM <= TRM_HI;

					WHEN TRM_HI =>
						S_Out_pos <= '0';
						S_Out_neg <= '1';
						IF S_Bit_cnt < 16 THEN
							IF S_Shift_Reg(S_Shift_Reg'high) = '1' THEN
								S_Encode_SM <= Idle_HI;
							ELSE
								S_Encode_SM <= Idle_LO;
							END IF;
						ELSIF S_Bit_cnt = 16 THEN
							S_SD <= '0';								-- V02
							IF S_Odd_Parity = '0' THEN					-- Das Odd-Parity-Bit zählt selbst mit bei der Parity-Bildung,	--
								S_Encode_SM <= Idle_HI;					-- deshalb wird bei Parity == GND nach Idle_HI verzweigt.		--
							ELSE
								S_Encode_SM <= Idle_LO;
							END IF;
						ELSE
							S_Encode_SM <= TRM_Idle;
						END IF;
				
				END CASE;
				
			END IF;
		END IF;
	END PROCESS P_Encode_SM;


P_No_Glitch:	PROCESS (clk)		
	BEGIN
		IF rising_edge(clk) THEN
			S_Bipol_Out_Pos <= S_Out_pos;				-- Um Übergangsfunktionen von 'Out_Pos/Neg' zu vermeiden wird		--
			S_Bipol_Out_Neg <= S_Out_neg;				-- die Logik über Systemtakt getaktete Register geführt.			--
			S_Sel_Mil_Drv <= S_Out_pos XOR S_Out_neg;	-- Nur bei ungleichen Logikpegel werden die externen Treiber der 	--
														-- bipol. Ausgänge aktiviert. Um Übergangsfuktionen zu vermeiden,	--
														-- und um das gleiche Timing zu haben wie 'Bipol_Out_Pos/Neg', wird	--
														-- die Logik über Systemtakt getaktete Register geführt.			--
		END IF;
	END PROCESS P_No_Glitch;


P_Rdy_4_Wr:	PROCESS (clk)		
	BEGIN
		IF rising_edge(clk) THEN
			IF Wr_Mil = '1' THEN																-- V03
				S_Mil_Rdy_4_Wr_Dly(3 downto 0) <= X"0";											-- V03
			ELSIF S_Encode_SM = TRM_Idle AND Ena_2x_TRM_Bitrate = '1' THEN						-- V03
				S_Mil_Rdy_4_Wr_Dly(3 downto 0) <= S_Mil_Rdy_4_Wr_Dly(2 downto 0) & '1';			-- V03
			END IF;
		END IF;
	END PROCESS P_Rdy_4_Wr;


nMil_Out_Pos <= NOT S_Bipol_Out_Pos;			-- Die Bipolaren Ausgänge müssen an den externen Treibern		--
nMil_Out_Neg <= NOT S_Bipol_Out_Neg;			-- 'aktiv null' sein.											--
nSel_Mil_Drv <= NOT S_Sel_Mil_Drv;				-- Die externen Treiber werden mit Null-Pegel aktiviert.		--

Mil_Rdy_4_Wr <= S_Mil_Rdy_4_Wr_Dly(1) WHEN Standard_Speed = '1' ELSE S_Mil_Rdy_4_Wr_Dly(3);				-- V03

SD <= S_SD;				-- Bildet das Signal "SD" des 6408-ICs nach, wird für den Blockmode der Interfacekarte benötigt.	--

END;

