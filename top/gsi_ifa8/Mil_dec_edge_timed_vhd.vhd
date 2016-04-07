--------------------------------------------------------------------------------------------------------------
-- Mil_dec_edge_timed_vhd empf�ngt einen manchester kodierten Datenstrom nach nach MIL-STD-1553B Protokoll.	--
-- Autor: W. Panschow, GSI Darmstadt, Abt. BEL/BELAB, Tel. 2341												--
-- Stand: 16.02.2010																						--
-- Version: 01																								--
--------------------------------------------------------------------------------------------------------------

library IEEE;
USE IEEE.STD_LOGIC_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;

ENTITY Mil_dec_edge_timed_vhd IS
	GENERIC(
			CLK_in_Hz		: INTEGER := 40_000_000;	-- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu k�nnen 
														-- (k�rzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
														-- Die tats�chlich angelegte Frequenz, muss vor der Synthese in "CLK_in_Hz" in Hertz beschrieben werden.  
			Uni_Pol_Neg_In	: INTEGER RANGE 0 TO 1 := 0	-- 0 => der Manchester-Datenstrom wird bipolar �ber �bertrager empfangen.
														-- 1 => der Datenstrom wird unipolar �ber Optokoppler empfangen.

			);
	PORT(
		Manchester_In			: IN	STD_LOGIC;		-- Eingangsdatenstrom MIL-1553B
		nManchester_In			: IN	STD_LOGIC;
		RD_MIL					: IN	STD_LOGIC;		-- setzt Rvc_Cmd, Rcv_Rdy und Rcv_Error zur�ck. Muss synchron zur Clock 'clk' und mindesten eine
														-- Periode lang aktiv sein!
		Res						: IN	STD_LOGIC;		-- Muss mindestens einmal f�r eine Periode von 'clk' aktiv ('1') gewesen sein.
		High_Speed				: IN	STD_LOGIC := '0';	-- Der Dekoder w�re auch in der Lage Manchester-Telegramme mit 2Mb/s zu empfangen. 'Clk' m�sste
														-- dann mindestens 40 MHz betragen. Wird aber nicht genutzt deshalb High_Speed = '0'.
		Clk						: IN	STD_LOGIC;
		Rcv_Cmd					: OUT	STD_LOGIC;		-- '1' es wurde ein Kommando empfangen.
		Rcv_Error				: OUT	STD_LOGIC;		-- ist bei einem Fehler f�r einen Takt aktiv '1'.
		Rcv_Rdy					: OUT	STD_LOGIC;		-- '1' es wurde ein Kommand oder Datum empfangen. Wenn Rcv_Cmd = '0' => Datum. Wenn Rcv_Cmd = '1' => Kommando
		Mil_Rcv_Data			: OUT	STD_LOGIC_VECTOR(15 DOWNTO 0);	-- Empfangenes Datum oder Komando
		Mil_Decoder_Diag		: OUT	STD_LOGIC_VECTOR(15 DOWNTO 0)	-- Diagnoseausg�nge f�r Logikanalysator
		);

END Mil_dec_edge_timed_vhd;

ARCHITECTURE Arch_Mil_dec_edge_timed_vhd OF Mil_dec_edge_timed_vhd IS

	TYPE	T_RCV_SM	IS
								(
								RCV_Idle,
								Sync1or2,
								Sync2orData,		
								Data,
								Parity,
								Err
								);

	SIGNAL	RCV_SM :	T_RCV_SM;

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

	CONSTANT	Data_Size			: INTEGER	:= 16;

	CONSTANT	Delta_in_ns			: INTEGER	:= 200;
														
	CONSTANT	CLK_in_ps			: INTEGER	:= (1000000000 / (CLK_in_Hz / 1000 / 2)); -- muss eigentlich clk-halbe sein
	
	CONSTANT	Sync_max_ns			: INTEGER	:= 1500 + Delta_in_ns;
	CONSTANT	Sync_max_cnt		: INTEGER	:= Sync_max_ns * 1000 / CLK_in_ps;
	CONSTANT	Sync_min_ns			: INTEGER	:= 1500 - Delta_in_ns;
	CONSTANT	Sync_min_cnt		: INTEGER	:= Sync_min_ns * 1000 / CLK_in_ps;
	
	CONSTANT	Sync_with_bit_max_ns	: INTEGER	:= 2000 + Delta_in_ns;
	CONSTANT	Sync_with_bit_max_cnt	: INTEGER	:= Sync_with_bit_max_ns * 1000 / CLK_in_ps;
	CONSTANT	Sync_with_bit_min_ns	: INTEGER	:= 2000 - Delta_in_ns;
	CONSTANT	Sync_with_bit_min_cnt	: INTEGER	:= Sync_with_bit_min_ns * 1000 / CLK_in_ps;
	
	CONSTANT	Bit_short_time_max_ns	: INTEGER	:= 500 + Delta_in_ns;
	CONSTANT	Bit_short_time_max_cnt	: INTEGER	:= Bit_short_time_max_ns * 1000 / CLK_in_ps;
	CONSTANT	Bit_short_time_min_ns	: INTEGER	:= 500 - Delta_in_ns;
	CONSTANT	Bit_short_time_min_cnt	: INTEGER	:= Bit_short_time_min_ns * 1000 / CLK_in_ps;
	
	CONSTANT	Bit_long_time_max_ns	: INTEGER	:= 1000 + Delta_in_ns;
	CONSTANT	Bit_long_time_max_cnt	: INTEGER	:= Bit_long_time_max_ns * 1000 / CLK_in_ps;
	CONSTANT	Bit_long_time_min_ns	: INTEGER	:= 1000 - Delta_in_ns;
	CONSTANT	Bit_long_time_min_cnt	: INTEGER	:= Bit_long_time_min_ns * 1000 / CLK_in_ps;

	CONSTANT	Timeout_Time_ns			: INTEGER	:= Sync_with_bit_max_ns + Delta_in_ns;
	CONSTANT	Timeout_Time_cnt		: INTEGER	:= Timeout_Time_ns * 1000 / CLK_in_ps;
	
	CONSTANT	C_Time_between_2_Edges_cnt_width	: INTEGER	:= How_many_bits(Sync_with_bit_max_cnt)+1;	-- um ein Bit gr��er


	--------------------------------------------------
	-- Z�hler misst die Zeit zwischen zwei Flanken	--
	--------------------------------------------------
	SIGNAL	S_Time_between_2_Edges_cnt	: STD_LOGIC_VECTOR(C_Time_between_2_Edges_cnt_width-1 DOWNTO 0);

	SIGNAL	S_Is_Sync			: STD_LOGIC;
	SIGNAL	S_Clr_Is_Sync		: STD_LOGIC;
	SIGNAL	S_Is_Sync_with_bit	: STD_LOGIC;
	SIGNAL	S_Clr_Is_Sync_with_bit	: STD_LOGIC;
	SIGNAL	S_Is_Bit_short		: STD_LOGIC;
	SIGNAL	S_Clr_Is_Bit_Short	: STD_LOGIC;
	SIGNAL	S_Is_Bit_long		: STD_LOGIC;
	SIGNAL	S_Clr_Is_Bit_Long	: STD_LOGIC;
	SIGNAL	S_Is_Timeout		: STD_LOGIC;
	SIGNAL	S_Leave_Parity_Tst	: STD_LOGIC;

	SIGNAL	S_Next_Short		: STD_LOGIC;

	SIGNAL	S_Mil_Rcv_Shift_Reg	: STD_LOGIC_VECTOR(Data_Size+2-1 DOWNTO 0);
																		-- + 2, weil in Bit [0] das Parity-Bit gespeichert wird,	--
																		--	und weil eine vorgeladene '1' als implizite Endekennung --
																		--	verwendet wird. So lange Bit[17] = '0' ist l�uft die	--
																		--	Datenaufnahme inklusive des Paritybits.					--

	SIGNAL	S_Mil_Rcv_Data		: STD_LOGIC_VECTOR(Data_Size-1 DOWNTO 0);

	SIGNAL	S_Manchester_Sync	: STD_LOGIC_VECTOR(2 DOWNTO 0);
	SIGNAL	S_nManchester_Sync	: STD_LOGIC_VECTOR(2 DOWNTO 0);
	SIGNAL	S_Edge_Detect		: STD_LOGIC;
	SIGNAL	S_nEdge_Detect		: STD_LOGIC;

	SIGNAL	S_Is_Cmd			: STD_LOGIC;
	SIGNAL	S_Is_Cmd_Memory		: STD_LOGIC;

	SIGNAL	S_MiL_Parity_Tst		: STD_LOGIC;

	SIGNAL	S_Rcv_Rdy			: STD_LOGIC;
	SIGNAL	S_Rcv_Rdy_Delayed			: STD_LOGIC;

	SIGNAL	S_Norm_Speed_Clk_2_Ena		: STD_LOGIC;

	SIGNAL	S_Shift_Ena			: STD_LOGIC;

	SIGNAL	S_Parity_Ok			: STD_LOGIC;

	SIGNAL	S_Rcv_Error			: STD_LOGIC;
	

--+-----------------------------------------------------+
--|  CMD 8080hex Unipolar (negiert �ber Optokoppler)    |
--|       3   2             2 2             2           |
--|  _____   _  _ _ _ _ _ _ __  _ _ _ _ _ _ __ _____    |
--|       ___ __ _ _ _ _ _ _  __ _ _ _ _ _ _  _         |
--|       <------------ 18,5 us -------------->         |
--|                                                     |
--|  CMD 8000hex Unipolar (negiert �ber Optokoppler)    |
--|       3   2                                         | 
--|  _____   _  _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ ______    |
--|       ___ __ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _          |
--|       <------------ 18,0 us ------------->          |
--|                                                     |
--|  CMD 8080hex Bipolar                            	|
--|       3  3   2             2 2             2        |
--|       ___   _  _ _ _ _ _ _ __  _ _ _ _ _ _ __       |
--|  _____   ___ __ _ _ _ _ _ _  __ _ _ _ _ _ _  _____  |
--|       <------------  19,5 us --------------->       |
--|                                                     |
--|                                                     |
--|  CMD 8000hex Bipolar                                |
--|       3  3   2                                      | 
--|       ___   _  _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _      |
--|  _____   ___ __ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _____ |
--|       <-------------  20,0 us --------------->      |
--|                                                     |
--+-----------------------------------------------------+
	
BEGIN

ASSERT NOT (CLK_in_Hz < 20_000_000)
	REPORT "Die Freq. f�r 'Mil_dec_edge_timed_vhd' ist " & integer'image(Clk_in_Hz) & " Hz. Sie sollte aber > 20 MHz sein!"
SEVERITY Error;

ASSERT NOT (CLK_in_Hz >= 20_000_000)
	REPORT "Die Freq. f�r 'Mil_dec_edge_timed_vhd' ist " & integer'image(Clk_in_Hz) & " Hz."
SEVERITY WARNING;

P_Edge_Detect:	PROCESS	(clk, S_Manchester_Sync)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			S_Manchester_Sync(2 DOWNTO 0) <= (S_Manchester_Sync(1 DOWNTO 0) & not Manchester_In);
		END IF;
		S_Edge_Detect 	<= (not S_Manchester_Sync(2) and     S_Manchester_Sync(1))
						OR (    S_Manchester_Sync(2) and not S_Manchester_Sync(1));
	END PROCESS P_Edge_Detect;

P_nEdge_Detect:	PROCESS	(clk, S_nManchester_Sync)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			S_nManchester_Sync(2 DOWNTO 0) <= (S_nManchester_Sync(1 DOWNTO 0) & not nManchester_In);
		END IF;
		S_nEdge_Detect 	<= (not S_nManchester_Sync(2) and     S_nManchester_Sync(1))
						OR (    S_nManchester_Sync(2) and not S_nManchester_Sync(1));
	END PROCESS P_nEdge_Detect;


P_Norm_Speed:	PROCESS (clk, Res)
	BEGIN
		IF Res = '1' THEN
			S_Norm_Speed_Clk_2_Ena <= '1';
		ELSIF clk'EVENT AND clk = '1' THEN
			IF High_Speed = '0' THEN
				S_Norm_Speed_Clk_2_Ena <= not S_Norm_Speed_Clk_2_Ena;
			ELSE
				S_Norm_Speed_Clk_2_Ena <= '1';
			END IF;
		END IF;
	END PROCESS P_Norm_Speed;


P_Time_between_2_Edges_cnt:	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF S_Edge_Detect = '1' OR RCV_SM = RCV_Idle THEN
				S_Time_between_2_Edges_cnt <= (OTHERS => '0');
			ELSIF S_Time_between_2_Edges_cnt(S_Time_between_2_Edges_cnt'high) = '0' AND S_Norm_Speed_Clk_2_Ena = '1' THEN
				S_Time_between_2_Edges_cnt <= S_Time_between_2_Edges_cnt + 1;
			ELSE
				S_Time_between_2_Edges_cnt <= S_Time_between_2_Edges_cnt;
			END IF;
		END IF;
	END PROCESS P_Time_between_2_Edges_cnt;


P_Is_Sync:	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF S_Time_between_2_Edges_cnt = Sync_min_cnt AND (RCV_SM = Sync1or2 or RCV_SM = Sync2orData) THEN
				S_Is_Sync <= '1';
			ELSIF S_Time_between_2_Edges_cnt = Sync_max_cnt OR S_Clr_Is_Sync = '1' THEN
				S_Is_Sync <= '0';
			END IF;
		END IF;
	END PROCESS P_Is_Sync;
	

P_Is_Sync_with_bit:	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF S_Time_between_2_Edges_cnt = Sync_with_bit_min_cnt AND (RCV_SM = Sync1or2 or RCV_SM = Sync2orData) THEN
				S_Is_Sync_with_bit <= '1';
			ELSIF S_Time_between_2_Edges_cnt = Sync_with_bit_max_cnt OR S_Clr_Is_Sync_with_Bit = '1' THEN
				S_Is_Sync_with_bit <= '0';
			END IF;
		END IF;
	END PROCESS P_Is_Sync_with_bit;
	

P_Is_Bit_short:	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF S_Time_between_2_Edges_cnt = Bit_short_time_min_cnt AND (RCV_SM = Data or RCV_SM = Sync2orData) THEN
				S_Is_Bit_short <= '1';
			ELSIF S_Time_between_2_Edges_cnt = Bit_short_time_max_cnt OR S_Clr_Is_Bit_Short = '1' THEN
				S_Is_Bit_short <= '0';
			END IF;
		END IF;
	END PROCESS P_Is_Bit_short;


P_Is_Bit_long:	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF S_Time_between_2_Edges_cnt = Bit_long_time_min_cnt AND RCV_SM = Data THEN
				S_Is_Bit_Long <= '1';
			ELSIF S_Time_between_2_Edges_cnt = Bit_long_time_max_cnt OR S_Clr_Is_Bit_Long = '1' THEN
				S_Is_Bit_Long <= '0';
			END IF;
		END IF;
	END PROCESS P_Is_Bit_long;
	

P_Is_Timeout:	PROCESS (clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF S_Time_between_2_Edges_cnt = Timeout_Time_cnt - 3 AND RCV_SM = Parity THEN
				S_Leave_Parity_Tst <= '1';
			ELSIF S_Time_between_2_Edges_cnt = Timeout_Time_cnt THEN
				S_Is_Timeout <= '1';
			ELSIF RCV_SM = RCV_Idle THEN
				S_Leave_Parity_Tst <= '0';
				S_Is_Timeout <= '0';
			END IF;
		END IF;
	END PROCESS P_Is_Timeout;


P_Mil_Rcv_Data:	PROCESS (clk, Res)
	BEGIN
		IF Res = '1' THEN
			S_Mil_Rcv_Data <= (OTHERS => '0');
		ELSIF clk'EVENT AND clk = '1' THEN
			IF S_Parity_OK = '1' THEN
				IF Uni_Pol_Neg_In = 0 THEN		
					S_Mil_Rcv_Data <= not S_Mil_Rcv_Shift_Reg(16 DOWNTO 1);
				ELSE
					S_Mil_Rcv_Data <= S_Mil_Rcv_Shift_Reg(16 DOWNTO 1);
				END IF;
			END IF;
		END IF;
	END PROCESS P_Mil_Rcv_Data;
	

P_Mil_Rcv_Shift_Reg:	PROCESS	(clk)
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF RCV_SM = RCV_Idle THEN
				S_Mil_Parity_Tst <= '0';
				S_Mil_Rcv_Shift_Reg <= conv_std_logic_vector(1, S_Mil_Rcv_Shift_Reg'length); 
			ELSIF S_Shift_Ena = '1' THEN
				IF S_Manchester_Sync(2) = '1' THEN
					S_Mil_Parity_Tst <= not S_Mil_Parity_Tst;
				END IF;
				S_Mil_Rcv_Shift_Reg <= (S_Mil_Rcv_Shift_Reg(S_Mil_Rcv_Shift_Reg'high-1 DOWNTO 0) & S_Manchester_Sync(2));
			ELSE
				S_Mil_Rcv_Shift_Reg <= S_Mil_Rcv_Shift_Reg;
			END IF;
		END IF;
	END PROCESS P_Mil_Rcv_Shift_Reg;


P_Rcv_Error:	PROCESS	(Res, clk)
	BEGIN
		IF Res = '1' THEN
			S_Rcv_Error <= '0';
		ELSIF clk'EVENT AND clk = '1' THEN
			IF RCV_SM = Err OR  S_Is_Timeout = '1' THEN
				S_Rcv_Error <= '1';
			ELSE
				S_Rcv_Error <= '0';
			END IF;
		END IF;
	END PROCESS P_Rcv_Error;


P_RCV_SM:	PROCESS	(clk, Res, S_Is_Timeout)
	BEGIN
		IF Res = '1' OR S_Is_Timeout = '1' THEN
			RCV_SM <= RCV_Idle;
			S_Shift_Ena <= '0';
			S_Parity_OK <= '0';
			S_Clr_Is_Sync_with_bit <= '1';
			S_Clr_Is_Sync <= '1';
			S_Clr_Is_Bit_Long <= '1';
			S_Clr_Is_Bit_Short <= '1';
		ELSIF clk'EVENT AND clk = '1' THEN
			S_Shift_Ena <= '0';
			S_Parity_OK <= '0';			
			S_Clr_Is_Sync_with_bit <= '0';
			S_Clr_Is_Sync <= '0';
			S_Clr_Is_Bit_Long <= '0';
			S_Clr_Is_Bit_Short <= '0';
			CASE RCV_SM IS
			
				WHEN RCV_Idle =>
					S_Is_Cmd <= '1';
					S_Next_Short <= '0';
 			  		IF S_Edge_Detect = '1' THEN
						RCV_SM <= Sync1or2;
					END IF;
					
				WHEN Sync1or2 =>
			  		IF S_Edge_Detect = '1' THEN
						IF S_Is_Sync_with_bit = '1' THEN
							S_Clr_Is_Sync_with_bit <= '1';
							S_Shift_Ena <= '1';
							RCV_SM <= Data;
						ELSIF S_Is_Sync = '1' THEN
							S_Clr_Is_Sync <= '1';
							RCV_SM <= Sync2orData;
						ELSE
							RCV_SM <= RCV_Idle;
						END IF;
					END IF;
				
				WHEN Sync2orData =>
			  		IF S_Edge_Detect = '1' THEN
						IF S_Is_Sync_with_bit = '1' THEN
							S_Clr_Is_Sync_with_bit <= '1';
							S_Shift_Ena <= '1';
							S_Is_Cmd <= '0';
							RCV_SM <= Data;
						ELSIF S_Is_Sync = '1' THEN
							S_Clr_Is_Sync <= '1';
							S_Is_Cmd <= '0';
							S_Next_Short <= '1';
							RCV_SM <= Data;
						ELSIF S_Is_Bit_short = '1' THEN
							S_Clr_Is_Bit_Short <= '1';
							S_Is_Cmd <= '1';
							S_Shift_Ena <= '1';
							RCV_SM <= Data;
						ELSE
							RCV_SM <= Err;
						END IF;
					END IF;

				WHEN Data =>
					IF S_Mil_Rcv_Shift_Reg(S_Mil_Rcv_Shift_Reg'high) = '0' THEN			
						IF S_Edge_Detect = '1' THEN
							IF (S_Is_Bit_short = '1' AND S_Next_Short = '1') OR (S_Is_Bit_long = '1' AND S_Next_Short = '0') THEN
								S_Clr_Is_Bit_long <= '1';
								S_Clr_Is_Bit_short <= '1';
								S_Shift_Ena <= '1';
								S_Next_Short <= '0';
								RCV_SM <= Data;
							ELSIF S_Is_Bit_short = '1' AND S_Next_Short = '0' THEN
								S_Clr_Is_Bit_short <= '1';
								S_Next_Short <= '1';
								RCV_SM <= Data;
							ELSIF S_Is_Bit_long = '1' AND S_Next_Short = '1' THEN
								S_Clr_Is_Bit_Long <= '1';
								RCV_SM <= Err;
							ELSE
								S_Clr_Is_Bit_Long <= '1';
								RCV_SM <= Err;
							END IF;
						ELSE
							RCV_SM <= Data;
						END IF;
					ELSE
						RCV_SM <= Parity;
					END IF;

				WHEN Parity =>
					IF (S_Mil_Rcv_Shift_Reg(0) = '0' AND Uni_Pol_Neg_In = 0) OR
					   (S_Mil_Rcv_Shift_Reg(0) = '1' AND Uni_Pol_Neg_In = 1) THEN
					--+-------------------------------------------------------------------------------------------------------------------------+
					--| Das Empfangene letzte Bit war eine Null, d.h. es kommt noch eine Flanke! Diese muss noch abgewartet werden,	sonst ist	|
					--| die Ablaufsteuerung zu fr�h in RCV_Idle und wertet die letzte Flanke als Sync-Flanke, was zu einem Fehler f�hrt.		|
					--+-------------------------------------------------------------------------------------------------------------------------+
					IF S_Edge_Detect = '1' OR S_Leave_Parity_Tst = '1' THEN	-- Falls die Flanke zu sp�t kommt, soll kein Timeout auftreten.
																		-- Es kann trotzdem zu einem Fehler kommen, wenn der 'Rcv_Idle' durch
																		-- die 'sp�te' Flanke verlassen wird und ein g�ltiges Telegramm in einem
																		-- 'passenden' Abstand beginnt und damit die 'sp�te' Flanke nicht verworfen
																		-- werden kann. Der Fehler ist nur im Blockmode m�glich.
						IF S_Mil_Parity_Tst = S_Mil_Rcv_Shift_Reg(0) THEN
							S_Parity_OK			<= '1';
							RCV_SM <= RCV_Idle;
						ELSE
							RCV_SM <= Err;
						END IF;
					END IF;
		
					--+-------------------------------------------------------------------------------------------------------------------------+
					--| Das Empfangene letzte Bit war eine Eins, d.h. es kommt keine Flanke mehr! Es kann direkt nach RCV_Idle verzweigt		|
					--| werden, wenn der Parity-Test keinen Fehler entdeckt.																	|
					--+-------------------------------------------------------------------------------------------------------------------------+
					ELSIF S_Mil_Parity_Tst = not S_Mil_Rcv_Shift_Reg(0) THEN
						S_Parity_OK				<= '1';
						RCV_SM <= RCV_Idle;
					ELSE
						RCV_SM <= Err;
					END IF;

				WHEN Err =>
					RCV_SM <= RCV_Idle;

				WHEN OTHERS =>
					RCV_SM <= RCV_Idle;
					
				END CASE;
		END IF;

	END PROCESS P_RCV_SM;	


P_Read:	PROCESS	(clk, Res)
	BEGIN
		IF Res = '1' THEN
			S_Rcv_Rdy <= '0';
			S_Is_Cmd_Memory <= '0';
		ELSIF clk'EVENT AND clk = '1' THEN
			S_Rcv_Rdy_Delayed <= S_Rcv_Rdy;
			IF Rd_Mil = '1' OR RCV_SM = Err THEN
				S_Rcv_Rdy <= '0';
				S_Is_Cmd_Memory <= '0';
			ELSIF S_Parity_Ok = '1' THEN
				IF S_Is_CMD = '1' THEN
					S_Is_Cmd_Memory <= '1';
				ELSE
					S_Is_Cmd_Memory <= '0';
				END IF;
				S_Rcv_Rdy <= '1';
			END IF;
		END IF;
	END PROCESS P_Read;


Mil_Rcv_Data <= S_Mil_Rcv_Data;

Rcv_Rdy		<= S_Rcv_Rdy_Delayed;
Rcv_Cmd 	<= S_Is_Cmd_Memory;
Rcv_Error	<= S_Rcv_Error;

P_Diag:	PROCESS	(
				S_Is_Cmd_Memory, RCV_SM, S_Rcv_Rdy_Delayed, S_Edge_Detect, S_Is_Timeout,
				S_Is_Sync_with_bit, S_Is_Sync, S_Is_Bit_long, S_Is_Bit_short, S_Rcv_Error,
				S_Leave_Parity_Tst, S_nEdge_Detect
				)
	BEGIN
		
		Mil_Decoder_Diag(15)	<=	S_nEdge_Detect;
		Mil_Decoder_Diag(14)	<=	S_Leave_Parity_Tst;
		Mil_Decoder_Diag(13)	<=	S_Is_Cmd_Memory;
		IF RCV_SM = RCV_Idle THEN
			Mil_Decoder_Diag(12)	<=	'1';
		ELSE
			Mil_Decoder_Diag(12)	<=	'0';
		END IF;
		IF RCV_SM = Sync1or2 THEN
			Mil_Decoder_Diag(11)	<=	'1';
		ELSE
			Mil_Decoder_Diag(11)	<=	'0';
		END IF;
		IF RCV_SM = Parity THEN
			Mil_Decoder_Diag(10)	<=	'1';
		ELSE
			Mil_Decoder_Diag(10)	<=	'0';
		END IF;
		IF RCV_SM = Sync2orData THEN
			Mil_Decoder_Diag(9)	<=	'1';
		ELSE
			Mil_Decoder_Diag(9)	<=	'0';
		END IF;
		IF RCV_SM = Data THEN
			Mil_Decoder_Diag(8)	<=	'1';
		ELSE
			Mil_Decoder_Diag(8)	<=	'0';
		END IF;
		Mil_Decoder_Diag(7)		<=	S_Rcv_Error;
		Mil_Decoder_Diag(6)		<=	S_Rcv_Rdy_Delayed;	
		Mil_Decoder_Diag(5)		<=	S_Edge_Detect;	
		Mil_Decoder_Diag(4)		<=	S_Is_Timeout;	
		Mil_Decoder_Diag(3)		<=	S_Is_Sync_with_bit;	
		Mil_Decoder_Diag(2)		<=	S_Is_Sync;
		Mil_Decoder_Diag(1)		<=	S_Is_Bit_long;
		Mil_Decoder_Diag(0)		<=	S_Is_Bit_short;
	END PROCESS P_Diag;
	
END Arch_Mil_dec_edge_timed_vhd;
