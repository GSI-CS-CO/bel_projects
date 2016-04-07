
library IEEE;
USE IEEE.STD_LOGIC_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;
use IEEE.MATH_REAL.ALL;

--+---------------------------------------------------------------------------------------------------------------------------------------------------------+
--|	Mil_bipol_dec:																																			|
--|		Das positive und negative Manchester-Signal mit wird von zwei unhabh�ngen Makros (Mil_dec_edge_timed_v1) empfangen.									|
--|		a)	Melden beide innerhalb der "max_jitter_ns" Zeit Valid Word, dann wird gepr�ft ob das empfangene Datum und die Kommando-Daten-Kennung 			|
--|			�bereinstimmt. Falls nicht wird "Data_not_equal" oder "CMD_not_Equal" generiert. Es wird kein "Rcv_Rdy" erzeugt.								|
--|			Stimmt alles �berein wird "Rcv_Rdy" erzeugt.																									|
--|		b)	Hat einer der unabh�ngigen Manchester-Dekoder ein g�ltiges Telegramm empfangen, dann wird dies mit der entsprechenden Kommando-Daten-Kennung	|
--| 		weitergegeben und "Rcv_Rdy" erzeugt. Der Manchester-Dekoder, der kein Telegramm empfangen hat wird mit dem Fehlersignal "No_VW_p(n)" 			|
--|			gekennzeichnet.																																	|
--|																																							|
--|	Autor:		W.Panschow																																	|
--|	Version:	1																																			|
--|	Datum:		29.02.12																																	|
--|																																							|
--|	Version: 2	Datum:	27.03.12	Autor:	W.Panschow																										|
--|	Grund:		Wenn beide Decoder_n(p) ein Valid-Word geliefert haben, aber die Daten oder die Kommand-Daten-Kennung nicht �bereinstimmt, wird kein		|
--|				"Rcv_Rdy" erzeugt. Dies hat zur Folge, dass die nachgeschaltete Datensenke auch kein "Rd_Mil" durchf�hrt. Dies hat wiederum zur Folge, dass	|
--|				die beiden Decoder nicht schnellst m�glich in den Idle-Zustand wechseln. In Version 2 generieren die Fehler-Signale "Data_not_equal" oder	|
--|				"CMD_not_Equal" ein automatisches "RD_Mil"-Signal. D.H. beide Decoder werden sofort zur�ckgesetzt.											|
--|																																							|
--|	Version: 3	Datum:	28.03.12	Autor:	W.Panschow																										|
--|	Grund:		Die Zeitdifferenz "max_jitter_ns" in der positive und negative Manchester-Encoder das "valid word" liefern m�ssen, war zu kurz definiert.	|
--|				Dadurch wurde der Fehler "No_VW_p(n)" generiert, obwohl das Datum etwas sp�ter empfangen wurde. Die erlaubte Differenz wurde von 100 ns auf	|
--|				300 ns vergr��ert.																															|
--|																																							|
--|	Version: 4	Datum:	30.03.12	Autor:	W.Panschow																										|
--|	Grund:		Zwei Fehlerz�hler die das no valid word des positiven und negativen Dekoders erfassen wurden eingebaut. Sie sind im Ausgang No_VW_Cnt		|
--|				zusammengefasst worden. Sie k�nnen mit dem Eingang Clr_No_VW_Cnt = '1' (muss mindesten eine 'clk'-Periode lang aktiv sein) zur�ckgesetzt	|
--|				werden.																																		|
--|				Zwei weitere Fehlerz�hler erfassen, wenn beide Dekoder ein Telegram empfangen haben, ob ein Unterschied in den Daten oder in der 			|
--|				Komando-Daten-Kennung besteht. Beide Z�hler werden im Ausgang Not_Equal_Cnt zusammengefasst. Beide Z�hler werden mit dem Eingang			|
--|				Clr_Not_Equal_Cnt = '1' (muss mindesten eine 'clk'-Periode lang aktiv sein) zur�ckgesetzt.													|
--|																																							|
--|	Version: 5	Datum:	02.04.12	Autor:	W.Panschow																										|
--|	Grund:		Beim Einschalten der Interface-Karte k�nnte diese einen laufenden Transfer mit einer anderen Interface-Karte als Fehler auswerten. Dies		|
--|				wird vermieden, wenn die Fehlerz�hler der Interface-Karte erst aktivert werden, wenn das Power-up-Bit der Interfacekarte quittiert wurde.	|
--|																																							|
--|	Version: 6	Datum:	23.04.12	Autor:	W.Panschow																										|
--|	Grund:		Die Zeitdifferenz die zwischen Valid Word des positiven und des negativen Dekoders auftreten kann ist von der Devicebusl�nge abh�ngig.		|
--|				Einer der beiden Dekoder muss immer darauf warten, dass das Paritiy-Bit die letzten Flanke mit einem Spannungspegel beendet, die nicht		|
--|				dem Ruhepegel entspricht. Deshalb kommt am Ende der Parity-Bit-Zeit noch eine Flanke die durch das Ausschwingen in die Ruhespannung erzeugt	|
--|				wird. Dies wird aber nicht mehr aktiv betrieben, und dauert bei einem l�ngeren Device-Bus durch die gr��ere kapatizive Last entsprechend	|
--|				l�nger.	Bis Version 5 war die Zeitdifferenz "max_jitter_ns" auf 300 ns festgelegt.															|
--|				In Version 6 ist "max_jitter_ns" auf 900 ns erh�ht worden.																					|
--|				Das hat zur Folge, dass die L�cke zwischen den Telegrammen gr��er als 900 ns sein muss.														|
--+---------------------------------------------------------------------------------------------------------------------------------------------------------+


ENTITY Mil_bipol_dec IS
	GENERIC(
			CLK_in_Hz:					INTEGER := 24_000_000;	-- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu k�nnen 
																-- (k�rzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
																-- Die tats�chlich angelegte Frequenz, muss vor der Synthese in "CLK_in_Hz" in Hertz beschrieben werden.
			threshold_not_equal_err:	INTEGER := 15;			-- �berschreiten die Fehlerz�hler "S_Data_not_equal_cnt" oder "S_CMD_not_equal_cnt" diesen Wert, dann wird der Ausgang "error_limit_reached" aktiv 'eins'. 
			threshold_no_VW_err:		INTEGER := 15			-- �berschreiten die Fehlerz�hler "S_No_VW_p_cnt" oder "S_No_VW_n_cnt" diesen Wert, dann wird der Ausgang "error_limit_reached" aktiv 'eins'. 
			);
	PORT(
		Manchester_In_p			: IN	STD_LOGIC;				-- positiver Eingangsdatenstrom MIL-1553B
		Manchester_In_n			: IN	STD_LOGIC;				-- negativer Eingangsdatenstrom MIL-1553B
		RD_MIL					: IN	STD_LOGIC;				-- setzt Rcv_Rdy zur�ck. Muss synchron zur Clock 'clk' und mindesten eine Periode lang aktiv sein!
		Clr_No_VW_Cnt			: IN	STD_LOGIC;				-- L�scht die no valid word Fehler-Z�hler des positiven und negativen Dekoders. Muss synchron zur Clock 'clk' und mindesten eine Periode lang aktiv sein!
		Clr_Not_Equal_Cnt		: IN	STD_LOGIC;				-- L�scht die Fehlerz�hler f�r Data_not_equal und den Fehlerz�hler f�r unterschiedliche Komando-Daten-Kennung (CMD_not_equal). Muss synchron zur Clock 'clk' und mindesten eine Periode lang aktiv sein!
		Res						: IN	STD_LOGIC;				-- Muss mindestens einmal f�r eine Periode von 'clk' aktiv ('1') gewesen sein.
		Power_up				: IN	STD_LOGIC;				-- so lange Power_up = '1' ist, bleiben alle 4 Fehlerz�hler auf null.
		Clk						: IN	STD_LOGIC;
		Rcv_Cmd					: OUT	STD_LOGIC;				-- '1' es wurde ein Kommando empfangen.
		Rcv_Rdy					: OUT	STD_LOGIC;				-- '1' es wurde ein Kommand oder Datum empfangen. Wenn Rcv_Cmd = '0' => Datum. Wenn Rcv_Cmd = '1' => Kommando
		Mil_Rcv_Data			: OUT	STD_LOGIC_VECTOR(15 DOWNTO 0);	-- Empfangenes Datum oder Komando
		Error_detect			: OUT	STD_LOGIC;				-- Zusammengefassung aller m�glichen Fehlermeldungen (ein Takt aktiv '1').
		Rcv_Error_p				: OUT	STD_LOGIC;				-- im positiven Signalpfad ist ein Fehler aufgetreten (ein Takt aktiv '1').
		Rcv_Error_n				: OUT	STD_LOGIC;				-- im negativen Signalpfad ist ein Fehler aufgetreten (ein Takt aktiv '1').
		No_VW_p					: OUT	STD_LOGIC;				-- im positiven Signalpfad kam kein Valid Word (ein Takt aktiv '1').
		No_VW_n					: OUT	STD_LOGIC;				-- im negativen Signalpfad kam kein Valid Word (ein Takt aktiv '1').
		Data_not_equal			: OUT	STD_LOGIC;				-- das Datum zwischem negativen und positivem Signalpfad ist ungleich (ein Takt aktiv '1').
		CMD_not_equal			: OUT	STD_LOGIC;				-- das Komando zwischem negativen und positivem Signalpfad ist ungleich (ein Takt aktiv '1').
		No_VW_Cnt				: OUT	STD_LOGIC_VECTOR(15 DOWNTO 0);	-- Bit[15..8] Fehlerz�hler f�r No Valid Word des positiven Decoders "No_VW_p", Bit[7..0] Fehlerz�hler f�r No Valid Word des negativen Decoders "No_VM_n"
		Not_Equal_Cnt			: OUT	STD_LOGIC_VECTOR(15 DOWNTO 0);	-- Bit[15..8] Fehlerz�hler f�r Data_not_equal, Bit[7..0] Fehlerz�hler f�r unterschiedliche Komando-Daten-Kennung (CMD_not_equal)
		error_limit_reached		: OUT	STD_LOGIC;						-- wird aktiv 'eins' wenn die Fehlerz�hler die Generics "threshold_not_equal_err" oder "threshold_no_VW_err" �berschritten haben.
		Mil_Decoder_Diag_p		: OUT	STD_LOGIC_VECTOR(15 DOWNTO 0);	-- Diagnoseausg�nge des Manchester Decoders am positiven Signalpfad.
		Mil_Decoder_Diag_n		: OUT	STD_LOGIC_VECTOR(15 DOWNTO 0)	-- Diagnoseausg�nge des Manchester Decoders am negativen Signalpfad.
		);

END Mil_bipol_dec;

ARCHITECTURE Arch_Mil_bipol_dec OF Mil_bipol_dec IS

	TYPE	T_Eval_RCV_SM	IS
								(
								RCV_Idle,
								RCV_p_RCV_n_together,
								RCV_p_first,		
								RCV_n_first,
								RCV_fin,
								Wait_Rd_Mil
								);

	SIGNAL	Eval_RCV_SM :	T_Eval_RCV_SM;

	CONSTANT	CLK_in_ps			: INTEGER	:= (1000000000 / (CLK_in_Hz / 1000));
	

	CONSTANT	max_jitter_ns		: INTEGER	:= 900;									-- maximale Zeitdiffenz zwischen dem valid word des positiven und negativen Dekoders
	CONSTANT	max_jitter_cnt		: INTEGER	:= max_jitter_ns * 1000 / CLK_in_ps;
	CONSTANT	jitter_cnt_Width	: INTEGER	:= INTEGER(ceil(log2(real(max_jitter_cnt))));
	SIGNAL		jitter_cnt          : STD_LOGIC_VECTOR(jitter_cnt_Width DOWNTO 0);
	
	SIGNAL		S_Rcv_Rdy          : STD_LOGIC;
	SIGNAL  	S_Rcv_Cmd          : STD_LOGIC := '0';

COMPONENT Mil_dec_edge_timed_v1
	GENERIC(
			CLK_in_Hz		: INTEGER := 40_000_000;				-- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu k�nnen 
																	-- (k�rzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
																	-- Die tats�chlich angelegte Frequenz, muss vor der Synthese in "CLK_in_Hz" in Hertz beschrieben werden.  
			Receive_pos_lane: INTEGER RANGE 0 TO 1 := 0				-- '1' => der positive Signalstrom ist an Manchester_In angeschlossen
																	-- '0' => der negative Signalstrom ist an Manchester_In angeschlossen.

			);
	PORT(
		Manchester_In		: IN	STD_LOGIC;						-- Eingangsdatenstrom MIL-1553B
		RD_MIL				: IN	STD_LOGIC;						-- setzt Rcv_Rdy zur�ck. Muss synchron zur Clock 'clk' und mindesten eine
																	-- Periode lang aktiv sein!
		Res					: IN	STD_LOGIC;						-- Muss mindestens einmal f�r eine Periode von 'clk' aktiv ('1') gewesen sein.
		Clk					: IN	STD_LOGIC;
		Rcv_Cmd				: OUT	STD_LOGIC;						-- '1' es wurde ein Kommando empfangen.
		Rcv_Error			: OUT	STD_LOGIC;						-- ist bei einem Fehler f�r einen Takt aktiv '1'.
		Rcv_Rdy				: OUT	STD_LOGIC;						-- '1' es wurde ein Kommand oder Datum empfangen. Wenn Rcv_Cmd = '0' => Datum. Wenn Rcv_Cmd = '1' => Kommando
		Mil_Rcv_Data		: OUT	STD_LOGIC_VECTOR(15 DOWNTO 0);	-- Empfangenes Datum oder Komando
		Mil_Decoder_Diag	: OUT	STD_LOGIC_VECTOR(15 DOWNTO 0)	-- Diagnoseausg�nge f�r Logikanalysator
		);
END COMPONENT;


SIGNAL	RCV_Cmd_p:				STD_LOGIC;						-- von Mil_dec_p wurde ein Kommando empfangen.
SIGNAL	Rcv_p:					STD_LOGIC;						-- '1' es wurde von Mil_dec_p ein Kommando oder Datum empfangen. Wenn Rcv_Cmd = '0' => Datum. Wenn Rcv_Cmd = '1' => Kommando
SIGNAL	Mil_Rcv_Data_p:			STD_LOGIC_VECTOR(15 DOWNTO 0);	-- von Mil_dec_p empfangenes Datum oder Komando

SIGNAL	RCV_Cmd_n:				STD_LOGIC;						-- von Mil_dec_n wurde ein Kommando empfangen.
SIGNAL	Rcv_n:					STD_LOGIC;						-- '1' es wurde von Mil_dec_n ein Kommando oder Datum empfangen. Wenn Rcv_Cmd = '0' => Datum. Wenn Rcv_Cmd = '1' => Kommando
SIGNAL	Mil_Rcv_Data_n:			STD_LOGIC_VECTOR(15 DOWNTO 0);	-- von Mil_dec_n empfangenes Datum oder Komando

SIGNAL  S_Rcv_Error_p:			STD_LOGIC;						-- es ist ein Fehler auf der positiven Manchester-Spur aufgetreten
SIGNAL  S_Rcv_Error_n:			STD_LOGIC;						-- es ist ein Fehler auf der negativen Manchester-Spur aufgetreten
SIGNAL  S_No_VW_p:				STD_LOGIC;						-- es wurde kein Valid Word vom positiven Manchester-Dekoder erzeugt
SIGNAL  S_No_VW_n:				STD_LOGIC;						-- es wurde kein Valid Word vom negativen Manchester-Dekoder erzeugt
SIGNAL  S_Data_not_equal:		STD_LOGIC;						-- Fehler: der positve und negative Manchester-Dekoder hat unterschiedliche Daten empfangen
SIGNAL  S_CMD_not_equal:		STD_LOGIC;						-- Fehler: der positve und negative Manchester-Dekoder hat unterschiedliche unterschiedliche Kommando/Daten-Kennung
SIGNAL  S_No_VW_p_cnt:			STD_LOGIC_VECTOR(7 DOWNTO 0);	-- Fehlerz�hler f�r fehlendes Valid Word vom positiven Manchester-Dekoder
SIGNAL  S_No_VW_n_cnt:			STD_LOGIC_VECTOR(7 DOWNTO 0);	-- Fehlerz�hler f�r fehlendes Valid Word vom negativen Manchester-Dekoder
SIGNAL  S_Data_not_equal_cnt:	STD_LOGIC_VECTOR(7 DOWNTO 0);	-- Fehlerz�hler wenn der positve und negative Manchester-Dekoder unterschiedliche Daten empfangen hat
SIGNAL  S_CMD_not_equal_cnt:	STD_LOGIC_VECTOR(7 DOWNTO 0);	-- Fehlerz�hler wenn der positve und negative Manchester-Dekoder unterschiedliche Kommando/Daten-Kennung erzeugt hat
SIGNAL	S_threshold_not_equal_err:	STD_LOGIC;					-- wird aktiv 'eins', wenn die Fehlerz�hler "S_Data_not_equal_cnt" oder "S_CMD_not_equal_cnt" den Generic-Wert "threshold_not_equal_err" �berschritten haben.
SIGNAL	S_threshold_no_VW_err:		STD_LOGIC;					-- wird aktiv 'eins', wenn die Fehlerz�hler "S_No_VW_p_cnt" oder "S_No_VW_n_cnt" den Generic-Wert "threshold_no_VW_err" �berschritten haben.
SIGNAL  S_Rd_Mil:				STD_LOGIC;

SIGNAL	S_CMD_not_eq_pulse:		STD_LOGIC;
SIGNAL	S_Data_not_eq_pulse:	STD_LOGIC;


BEGIN

ASSERT NOT (CLK_in_Hz < 20_000_000)
	REPORT "Die Freq. f�r 'Mil_bipol_dec' ist " & integer'image(Clk_in_Hz) & " Hz. Sie sollte aber > 20 MHz sein!"
SEVERITY Error;

ASSERT NOT (CLK_in_Hz >= 20_000_000)
	REPORT "Die Freq. f�r 'Mil_bipol_dec' ist " & integer'image(Clk_in_Hz) & " Hz."
SEVERITY WARNING;

ASSERT False
	REPORT "Max_jitter_cnt ist " & integer'image(max_jitter_cnt) & ", und die Breite von jitter_cnt ist " & integer'image(jitter_cnt_Width + 1)
SEVERITY WARNING;


Mil_dec_p:	Mil_dec_edge_timed_v1
	GENERIC MAP(
				CLK_in_Hz		=> CLK_in_Hz,			-- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu k�nnen 
														-- (k�rzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
														-- Die tats�chlich angelegte Frequenz, muss vor der Synthese in "CLK_in_Hz" in Hertz beschrieben werden.  
				Receive_pos_lane	=> 1				-- '1' => der positive Signalstrom ist an Manchester_In angeschlossen
														-- '0' => der negative Signalstrom ist an Manchester_In angeschlossen.

				)
	PORT MAP(
			Manchester_In	=> Manchester_In_p,			-- Eingangsdatenstrom MIL-1553B
			RD_MIL			=> S_RD_MIL,				-- setzt Rcv_Rdy zur�ck. Muss synchron zur Clock 'clk' und mindesten eine
														-- Periode lang aktiv sein!
			Res				=> Res,						-- Muss mindestens einmal f�r eine Periode von 'clk' aktiv ('1') gewesen sein.
			Clk				=> Clk,
			Rcv_Cmd			=> RCV_Cmd_p,				-- '1' es wurde ein Kommando empfangen.
			Rcv_Error		=> S_Rcv_Error_p,			-- ist bei einem Fehler f�r einen Takt aktiv '1'.
			Rcv_Rdy			=> Rcv_p,					-- '1' es wurde ein Kommand oder Datum empfangen. Wenn Rcv_Cmd = '0' => Datum. Wenn Rcv_Cmd = '1' => Kommando
			Mil_Rcv_Data	=> Mil_Rcv_Data_p,			-- Empfangenes Datum oder Komando
			Mil_Decoder_Diag	=> Mil_Decoder_Diag_p	-- Diagnoseausg�nge f�r Logikanalysator
			);


Mil_dec_n:	Mil_dec_edge_timed_v1
	GENERIC MAP(
					CLK_in_Hz		=> CLK_in_Hz,		-- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu k�nnen 
														-- (k�rzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
														-- Die tats�chlich angelegte Frequenz, muss vor der Synthese in "CLK_in_Hz" in Hertz beschrieben werden.  
					Receive_pos_lane	=> 0			-- '1' => der positive Signalstrom ist an Manchester_In angeschlossen
														-- '0' => der negative Signalstrom ist an Manchester_In angeschlossen.
	
				)
	PORT MAP(
				Manchester_In	=> Manchester_In_n,		-- Eingangsdatenstrom MIL-1553B
				RD_MIL			=> S_RD_MIL,			-- setzt Rcv_Rdy zur�ck. Muss synchron zur Clock 'clk' und mindesten eine
														-- Periode lang aktiv sein!
				Res				=> Res,					-- Muss mindestens einmal f�r eine Periode von 'clk' aktiv ('1') gewesen sein.
				Clk				=> Clk,
				Rcv_Cmd			=> RCV_Cmd_n,			-- '1' es wurde ein Kommando empfangen.
				Rcv_Error		=> S_Rcv_Error_n,		-- ist bei einem Fehler f�r einen Takt aktiv '1'.
				Rcv_Rdy			=> Rcv_n,				-- '1' es wurde ein Kommand oder Datum empfangen. Wenn Rcv_Cmd = '0' => Datum. Wenn Rcv_Cmd = '1' => Kommando
				Mil_Rcv_Data	=> Mil_Rcv_Data_n,		-- Empfangenes Datum oder Komando
			Mil_Decoder_Diag	=> Mil_Decoder_Diag_n	-- Diagnoseausg�nge f�r Logikanalysator
			);


P_Eval_RCV_SM:	PROCESS	(clk, Res)
	BEGIN
		IF Res = '1' THEN
			Eval_RCV_SM <= RCV_Idle;
			S_CMD_not_equal <= '0';
			S_Data_not_equal <= '0';
			S_No_VW_p <= '0';
			S_No_VW_n <= '0';
			S_Rcv_Rdy <= '0';

		ELSIF rising_edge(clk) THEN

			S_No_VW_p <= '0';
			S_No_VW_n <= '0';
			S_CMD_not_eq_pulse <= '0';
			S_Data_not_eq_pulse <= '0';
			
			IF RD_MIL = '1' THEN
				S_Rcv_Rdy <= '0';
			END IF; 
	

			CASE Eval_RCV_SM IS
			
				WHEN RCV_Idle =>

					S_CMD_not_equal <= '0';
					S_Data_not_equal <= '0';
				
					IF S_Rcv_Rdy = '0' THEN  									-- nur wenn das vorherige Datum gelesen wurde, kann ein neues Datum empfangen werden!
						IF Rcv_p = '1' and Rcv_n = '1' THEN
							Eval_RCV_SM <= RCV_p_RCV_n_together;				-- Beide Encoder haben gleichzeitig ein Telegram empfangen.
						ELSIF  Rcv_p = '1' and Rcv_n = '0' THEN
							Eval_RCV_SM <= RCV_p_first;							-- Der positive Encoder hat zuerst ein Telegram empfangen.
						ELSIF  Rcv_p = '0' and Rcv_n = '1' THEN
							Eval_RCV_SM <= RCV_n_first;							-- Der negative Encoder hat zuerst ein Telegram empfangen.
						END IF;
					END IF;
					
				WHEN RCV_p_RCV_n_together =>									-- Beide Encoder haben ein Telegram empfangen.
					
					IF RCV_Cmd_p = RCV_Cmd_n THEN								-- teste ob beide Encoder die gleiche Kommando-Daten-Kennung haben
						S_RCV_Cmd <= RCV_Cmd_p;									-- ja, �bernehme die Kommando-Daten-Kennung
					ELSE
						S_CMD_not_equal <= '1';									-- signalisiere ungleiche Kommand-Daten-Kennung
					END IF;
					
					IF Mil_Rcv_Data_p = Mil_Rcv_Data_n THEN						-- teste ob beide Encoder das gleiche Datum empfangen haben
						Mil_Rcv_Data <= Mil_Rcv_Data_p;							-- ja, �bernehme das Datum
					ELSE
						S_Data_not_equal <= '1';								-- signalisiere ungleiches Datum
					END IF;
					
					Eval_RCV_SM <= RCV_fin;

				WHEN RCV_p_first =>												-- Der positive Encoder hat zuerst ein Telegram empfangen

					IF jitter_cnt(jitter_cnt'high) = '0' THEN
						IF Rcv_n = '1' THEN										-- innerhalb der erlaubten Zeitspanne hat auch der negative Encoder ein Telegram empfangen
							Eval_RCV_SM <= RCV_p_RCV_n_together;				-- deshalb k�nnen beide Telegrame ausgewertet werden.
						END IF;
					ELSE
						S_RCV_Cmd <= RCV_Cmd_p;									-- Kommando-Datenkennung
						Mil_Rcv_Data <= Mil_Rcv_Data_p;							-- und Datum vom positiven Encoder �bernehmen.
						S_No_VW_n <= '1';										-- fehlender Empfang vom negativen Encoder signalisieren.
						Eval_RCV_SM <= RCV_fin;
					END IF;

				WHEN RCV_n_first =>												-- Der negative Encoder hat zuerst ein Telegram empfangen
				
					IF jitter_cnt(jitter_cnt'high) = '0' THEN
						IF Rcv_p = '1' THEN										-- innerhalb der erlaubten Zeitspanne hat auch der positive Encoder ein Telegram empfangen
							Eval_RCV_SM <= RCV_p_RCV_n_together;				-- deshalb k�nnen beide Telegrame ausgewertet werden.
						END IF;
					ELSE
						S_RCV_Cmd <= RCV_Cmd_n;									-- Kommando-Datenkennung
						Mil_Rcv_Data <= Mil_Rcv_Data_n;							-- und Datum vom negativen Encoder �bernehmen.
						S_No_VW_p <= '1';										-- fehlender Empfang vom positiven Encoder signalisieren.
						Eval_RCV_SM <= RCV_fin;
					END IF;

				WHEN RCV_fin =>
					S_CMD_not_eq_pulse <= S_CMD_not_equal;
					S_Data_not_eq_pulse <= S_Data_not_equal;
					IF S_CMD_not_equal = '0' AND S_Data_not_equal = '0' THEN 
						S_Rcv_Rdy <= '1';
						Eval_RCV_SM <= Wait_Rd_Mil;
					ELSE
						Eval_RCV_SM <= RCV_Idle;								-- zwischen dem positiven und negativen Encoder ist eine Diffenz in der Kommando-Daten-Kennung oder dem empfangenen Datum festgestellt worden.
					END IF;

				WHEN Wait_Rd_Mil =>
					IF Rcv_p = '0' AND Rcv_n = '0'	THEN				
					 Eval_RCV_SM <= RCV_Idle;
					END IF;
					
				WHEN OTHERS =>
				
					Eval_RCV_SM <= RCV_Idle;
					
			END CASE;
		END IF;

	END PROCESS P_Eval_RCV_SM;	


P_jitter_cnt:	PROCESS	(clk, Res)
	BEGIN
		IF Res = '1' THEN
			jitter_cnt <= conv_std_logic_vector(max_jitter_cnt-1, jitter_cnt'length);
		ELSIF rising_edge(clk) THEN
			IF Eval_RCV_SM = RCV_p_first OR Eval_RCV_SM = RCV_n_first THEN
				IF jitter_cnt(jitter_cnt'high) = '0' THEN
					jitter_cnt <= jitter_cnt - 1;
				END IF;
			ELSE
				jitter_cnt <= conv_std_logic_vector(max_jitter_cnt-1, jitter_cnt'length);
			END IF;
		END IF;
	END PROCESS P_jitter_cnt;


P_error_cnt:	PROCESS	(clk, Res, Power_up)
	BEGIN
		IF Res = '1' OR Power_up = '1' THEN
			S_No_VW_p_cnt <= (OTHERS => '0');
			S_No_VW_n_cnt <= (OTHERS => '0');
			S_CMD_not_equal_cnt <= (OTHERS => '0');
			S_Data_not_equal_cnt <= (OTHERS => '0');
			S_threshold_no_VW_err <= '0';
			S_threshold_not_equal_err <= '0';
			
		ELSIF rising_edge(clk) THEN

			IF Clr_No_VW_Cnt = '1' THEN
				S_No_VW_p_cnt <= (OTHERS => '0');
			ELSIF S_No_VW_p_cnt < 255 AND S_No_VW_p = '1' THEN
				S_No_VW_p_cnt <= S_No_VW_p_cnt + 1;
			END IF;

			IF Clr_No_VW_Cnt = '1' THEN
				S_No_VW_n_cnt <= (OTHERS => '0');
			ELSIF S_No_VW_n_cnt < 255 AND S_No_VW_n = '1' THEN
				S_No_VW_n_cnt <= S_No_VW_n_cnt + 1;
			END IF;
			
			IF S_No_VW_p_cnt > threshold_no_VW_err OR S_No_VW_n_cnt > threshold_no_VW_err THEN
				S_threshold_no_VW_err <= '1';
			ELSE
				S_threshold_no_VW_err <= '0';
			END IF;

			IF Clr_Not_Equal_Cnt = '1' THEN
				S_CMD_not_equal_cnt <= (OTHERS => '0');
			ELSIF S_CMD_not_equal_cnt < 255 AND S_CMD_not_eq_pulse = '1' THEN
				S_CMD_not_equal_cnt <= S_CMD_not_equal_cnt + 1;
			END IF;
			
			IF Clr_Not_Equal_Cnt = '1' THEN
				S_Data_not_equal_cnt <= (OTHERS => '0');
			ELSIF S_Data_not_equal_cnt < 255 AND S_Data_not_eq_pulse = '1' THEN
				S_Data_not_equal_cnt <= S_Data_not_equal_cnt + 1;
			END IF;

			IF S_CMD_not_equal_cnt > threshold_not_equal_err OR S_Data_not_equal_cnt > threshold_not_equal_err THEN
				S_threshold_not_equal_err <= '1';
			ELSE
				S_threshold_not_equal_err <= '0';
			END IF;
		
		END IF;
	END PROCESS P_error_cnt;
	
No_VW_Cnt <= S_No_VW_p_cnt & S_No_VW_n_cnt;						-- Bit[15..8] Fehlerz�hler f�r No Valid Word des positiven Decoders "No_VW_p", Bit[7..0] Fehlerz�hler f�r No Valid Word des negativen Decoders "No_VM_n"

Not_Equal_Cnt <= S_Data_not_equal_cnt & S_CMD_not_equal_cnt;	-- Bit[15..8] Fehlerz�hler f�r Data_not_equal, Bit[7..0] Fehlerz�hler f�r unterschiedliche Komando-Daten-Kennung (CMD_not_equal).

error_limit_reached <= '1' WHEN S_threshold_no_VW_err = '1' OR S_threshold_not_equal_err = '1' ELSE '0';	-- wird aktiv 'eins', wenn die Fehlerz�hler die Generics "threshold_not_equal_err" oder "threshold_no_VW_err" �berschritten haben.
	
S_RD_Mil <= Rd_Mil or S_Data_not_equal or S_CMD_not_equal;  	-- S_Rd_Mil setzt 'rvc_rdy_p' und 'rcv_rdy_n' zur�ck, und falls kein Fehler aufgetreten ist
																-- setzt Rd_Mil (das empfangene Datum wird von extern gelesen) die Signale zur�ck.
 
Rcv_Rdy <= S_Rcv_Rdy;

Error_Detect <= S_Rcv_Error_p or S_Rcv_Error_n or S_No_VW_p or S_No_VW_n or S_CMD_not_eq_pulse or S_Data_not_eq_pulse;

Rcv_Error_p <= S_Rcv_Error_p;
Rcv_Error_n <= S_Rcv_Error_n;
No_VW_p <= S_No_VW_p;
No_VW_n <= S_No_VW_n;
CMD_not_equal <= S_CMD_not_eq_pulse;
Data_not_equal <= S_Data_not_eq_pulse;
Rcv_Cmd <= S_Rcv_Cmd;

END Arch_Mil_bipol_dec;
