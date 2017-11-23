--TITLE "'FairBusInterface' Autor: W.Panschow, Stand: 09.11.10, Version = 1, Revision = 2";

--+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
--+ Beschreibung:																											+
--+																															+
--+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
--
--+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
--|	Vers_1_Revi_1																											|
--|	Autor:	W. Panschow																										|
--| Datum:	14.09.10																										|
--| �nderung A)	Bei einem Lesezugriff auf eine nicht vorhandene Slave-Resource wurde der Zyklus zwar richtig mit Timeout	|
--|			abgebrochen, aber es wurde trotzdem das Signal "nStart_DB_Trm" aktiviert und die Interfacekarte sendete ein		|
--|			zuf�lliges Datum, d.h. der Device-Bus-Master bekam den Timeout nicht mit.										|
--|			Der Fehler wurde in der Ablaufsteuerung "FB_SM"	dadurch behoben, dass vom Timeout-State "TO_Rd_Cyc"	direkt		|
--|			zum "Idle"-State gesprungen wird.																				|
--|	�nderung B) Es gibt ein neues Signal "Read_Port_active", dass aktiv '1' wird, wenn das "Status"-Port eine interne		|
--|			Lese-Ressource ausgibt.																							|
--+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
--
--+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
--|	Vers_1_Revi_2																											|
--|	Autor:	W. Panschow																										|
--| Datum:	09.11.10																										|
--| �nderung A)	Mit dem Signal "nFair_Adr_Sel" hat dieses Makro einen weiteren Ausgang bekommen mit dem die					|
--|			Fair-Adresstreiber der Interfacekarte in den Tristate geschaltet werden k�nnen. Bis Vers_1_Revi_1 wurde am Ende	|
--|			eines Schreib-Lese-Zyklusses die Fair-Adresse aktiv wieder auf X"FFFF" gesetzt. Wenn die Fair-Adresse des		|
--|			des zuendegehenden Zykluses viele "Null-Anteile" hatte, wurde durch das Umladen auf Eins un�tige St�rungen		|
--|			auf anderen Bussignalen hervorgerufen. Wenn jetzt am Ende des Buszyklusses lediglich der Fair-Adr-Treiber in	|
--|			Tristate geschaltet wird, dann schwingen die Adr-Leitungen nur durch den Busabschluss "getrieben"				|
--|			auf "Eins-Pegel" zur�ck.																						|
--|			Mit dem Generic "Drv_Adr_Idle_High" kann der Fair-Adr-Treiber auf zwei unterschiedliche Verhalten programmiert	|
--|				1) "Drv_Adr_Idle_High" = 0 => w�hrend des Buszyklusses wird der Fair-Adr-Treiber aktiv geschschaltet, sonst	|
--|				ist er im Tristate.																							|
--|				2) "Drv_Adr_Idle_High" = 1 => der Fair-Adr-Treiber ist immer selektiert, am Ende des Buszyklusses wird		|
--|				von diesem Makro aktiv die Adresse auf X"FFFF" zur�ckgesetzt.												|
--+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
--
--+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
--|	Vers_1_Revi_3																											|
--|	Autor:	W. Panschow																										|
--| Datum:	15.11.10																										|
--| �nderung Der IFK_FairBus-Makro hatte bisher einen Reset-Eingang mit dem Namen "Reset", der nur auf diesen Makro wirkte.	|
--|			Das Reset-Signal des SCU_Busses wurde vom IFK_FairBus-Makro bisher nicht bedient.							 	|
--|			a)	Zur besseren Unterscheidung ist der bisherige Reset-Eingang in "Reset_this_Macro" umbenannt worden, er dient|
--|				als asynchronens Reset f�r das IFK_FairBus-Makro.															|
--|			b)	Ein SCUB_Reset wird durch setzen von Bit(15) w�hrend eines "Wr_Status" gestartet. Das Bit(15) bleibt so		|
--|				lange im Rd_Status gesetzt bis die im Generic "Reset_SCUB_in_ns" festgelegte Zeit abgelaufen ist. Genauso	|
--|				ist der Aus|
--+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
--
--+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
--|	Vers_1_Revi_4																											|
--|	Autor:	W. Panschow																										|
--| Datum:	10.01.13																										|
--| Bisher konnte die IFK bei den bis zu zw�lf m�glichen Slaves nur die Adressen [11..0] ansprechen. Um Zeit zu sparen		|
--|	wurde mit den Adressen[15..12] decodiert welcher Slave angesprochen werden sollte.									 	|
--| F�r die ACU soll der komplette Adressraum zur Verf�gung stehen. Da die ACU nur mit einem eigenen speziellen Bus  		| 
--|	betrieben wird, in dem nur eine ACU best�ckt werden kann, braucht es nicht den obigen Adressierungsmechanismus.			|
--| Soll die IFK eine ACU bedienen, reicht es, diesen Mode mit dem IFK-Mode-Register einzuschalten.							|
--|	Zu diesem Zweck hat das IFK-FairBus-Makro ein weiters Eingangssignal "Sel_ACU_Mode" bekommen.							|
--| 	"Sel_ACU_Mode" = '1',	es wird immer ein Slave-1-Zugriff (=ACU) mit vollst�ndiger 16 Bit Adresse durchgef�hrt.	|
--| 	"Sel_ACU_Mode" = '0',	die oberen 4 Bit der Adresse dienen zur Dekodierung des Slave-Steckplatzes. Die unteren		|
--|								12 Bit stehen zur Register-Adressierung des entsprechenen Slaves zur Verf�gung.				|
--+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
--

library IEEE;
USE IEEE.STD_LOGIC_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;

ENTITY IFK_FairBus IS

	GENERIC(
			CLK_in_Hz			: INTEGER := 100000000;
			Time_Out_in_ns		: INTEGER := 250;
			Sel_dly_in_ns		: INTEGER := 30;							-- delay to the I/O pins is not included
			Sel_release_in_ns	: INTEGER := 30;							-- delay to the I/O pins is not included
			D_Valid_to_DS_in_ns	: INTEGER := 30;							-- delay to the I/O pins is not included
			Timing_str_in_ns	: INTEGER := 80;							-- delay to the I/O pins is not included
			SRQ_Polling_in_ns	: INTEGER := 3000;
			Drv_Adr_Idle_High	: INTEGER Range 0 TO 1 := 0;		-- Vers_1_Revi_2: 0 => Adr-Treiber werden am Ende des Buszykluss auf
																	-- Tristate geschaltet. 1 => Adr-Treiber bleiben immer selektiert,
																	-- am Ende des Buszyklusses wird die Adr aktiv auf X"FFFF" geschaltet.
			Reset_SCUB_in_ns	: INTEGER := 600;		-- Vers_1_Revi_3: Zeit in ns die gestartetes SCUB-Reset aktiv bleibt (wird durch ein WR_Status D[15] = '1' gestartet).
			Test				: INTEGER := 0
			);

	PORT(
		Data_from_DB		: IN		STD_LOGIC_VECTOR(15 DOWNTO 0);		-- data from device bus
		ADR_from_DB			: IN		STD_LOGIC_VECTOR(15 DOWNTO 0);
		Start_Wr_Fair		: IN		STD_LOGIC;							-- start write cycle to fair bus
		Start_Rd_Fair		: IN		STD_LOGIC;							-- start read cycle from fair bus
		Timing_In			: IN		STD_LOGIC_VECTOR(15 DOWNTO 0);
		Start_Timing_Cycle	: IN		STD_LOGIC;							-- start timing cycle to fair bus
		Data_from_Fair		: IN		STD_LOGIC_VECTOR(15 DOWNTO 0);
		clk					: IN		STD_LOGIC;
		Reset_this_Macro	: IN		STD_LOGIC;							-- Vers_1_Revi_3
		Data_to_DB			: OUT		STD_LOGIC_VECTOR(15 DOWNTO 0);		-- data to device bus
		Data_to_Fair		: OUT		STD_LOGIC_VECTOR(15 DOWNTO 0);		-- data to fair bus
		Ena_Tri_Buff		: OUT		STD_LOGIC;							-- select for FPGA internal tristate buffer
		nFair_DS			: OUT		STD_LOGIC;
		nFair_Dtack			: IN		STD_LOGIC;
		Fair_Addr			: OUT		STD_LOGIC_VECTOR(15 DOWNTO 0);
		nFair_Adr_Sel		: OUT		STD_LOGIC;							-- Vers_1_Revi_2
		Fair_RDnWR			: OUT		STD_LOGIC;
		nSCUB_Reset			: OUT		STD_LOGIC;							-- Vers_1_Revi_3
		nFair_SRQ_Slaves	: IN		STD_LOGIC_VECTOR(11 DOWNTO 0);
		Wr_Status			: IN		STD_LOGIC;
		Rd_Status			: IN		STD_LOGIC;
		Wr_SRQ_Ena			: IN		STD_LOGIC;
		Rd_Srq_Ena			: IN		STD_LOGIC;
		Rd_Srq_active		: IN		STD_LOGIC;
		Rd_Srq_In			: IN		STD_LOGIC;
		Rd_Vers_Revi		: IN		STD_LOGIC;
		Sel_ACU_Mode		: IN		STD_LOGIC := '0';
		nFair_Board_Sel		: OUT		STD_LOGIC_VECTOR(11 DOWNTO 0);
		nFair_Timing_Cycle	: OUT		STD_LOGIC;
		nSel_Ext_Data_Drv	: OUT		STD_LOGIC;
		Ext_Data_Drv_WRnRd	: OUT		STD_LOGIC;
		nStart_DB_Trm		: OUT		STD_LOGIC;
		nInterlock			: OUT		STD_LOGIC;
		nData_RDY			: OUT		STD_LOGIC;
		nData_REQ			: OUT		STD_LOGIC;
		Read_Port			: OUT		STD_LOGIC_VECTOR(15 DOWNTO 0);
		Read_Port_active	: OUT		STD_LOGIC;
		FB_Rd_Err_FB_Busy	: OUT		STD_LOGIC;
		FB_Rd_Err_no_Dtack	: OUT		STD_LOGIC;
		FB_Rd_Fin			: OUT		STD_LOGIC;
		FB_Rd_active		: OUT		STD_LOGIC;
		FB_Wr_Err_FB_Busy	: OUT		STD_LOGIC;
		FB_Wr_Err_no_Dtack	: OUT		STD_LOGIC;
		FB_Wr_Fin			: OUT		STD_LOGIC;
		FB_Wr_active		: OUT		STD_LOGIC;
		FB_Ti_Fin			: OUT		STD_LOGIC;
		Status				: OUT		STD_LOGIC_VECTOR(15 DOWNTO 0);
		IFK_FB_Version		: OUT		STD_LOGIC_VECTOR(3 DOWNTO 0);
		IFK_FB_Revision		: OUT		STD_LOGIC_VECTOR(3 DOWNTO 0)
		);
		
	FUNCTION set_vers_or_revi( vers_or_revi, Test: INTEGER) RETURN INTEGER IS
		BEGIN
			IF test > 1 THEN
				RETURN 0;
			ELSE
				RETURN vers_or_revi;
			END IF;
		END set_vers_or_revi;

	CONSTANT	C_IFK_FB_Version	: INTEGER RANGE 0 TO 15 := set_vers_or_revi(1, Test);		-- define the version of this macro
	CONSTANT	C_IFK_FB_Revision	: INTEGER RANGE 0 TO 15 := set_vers_or_revi(4, Test);		-- define the revision of this macro
	
	CONSTANT	Clk_in_ps			: INTEGER	:= 1000000000 / (Clk_in_Hz / 1000);
	CONSTANT	Clk_in_ns			: INTEGER	:= 1000000000 / Clk_in_Hz;


	FUNCTION set_ge_1  (a : INTEGER) RETURN INTEGER IS
		BEGIN
			IF a > 1 THEN
				RETURN a;
			ELSE
				RETURN 1;
			END IF;
		END set_ge_1;



	FUNCTION prod_or_test (production, test_data, test : INTEGER) RETURN INTEGER IS
		BEGIN
			IF Test = 1 THEN
				RETURN test_data;
			ELSE
				RETURN production;
			END IF;
		END prod_or_test;


	FUNCTION How_many_Bits (int: INTEGER) RETURN INTEGER IS

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


	FUNCTION return_max (a, b : INTEGER) RETURN INTEGER IS
		BEGIN
			IF a >= b THEN
				RETURN a;
			ELSE
				RETURN b;
			END IF;
		END return_max;


	CONSTANT	C_Sel_dly_cnt			: INTEGER	:= set_ge_1(Sel_dly_in_ns * 1000 / Clk_in_ps)-2;		--	-2 because counter needs two more clock for unerflow
	SIGNAL		S_Sel_dly_cnt			: STD_LOGIC_VECTOR(How_many_Bits(C_Sel_dly_cnt) DOWNTO 0);

	CONSTANT	C_Sel_release_cnt		: INTEGER	:= set_ge_1(Sel_release_in_ns * 1000 / Clk_in_ps)-2;	--	-2 because counter needs two more clock for unerflow
	SIGNAL		S_Sel_release_cnt		: STD_LOGIC_VECTOR(How_many_Bits(C_Sel_release_cnt) DOWNTO 0);

	CONSTANT	C_Timing_str_cnt		: INTEGER	:= set_ge_1(Timing_str_in_ns * 1000 / Clk_in_ps)-2;		--	-2 because counter needs two more clock for unerflow
	SIGNAL		S_Timing_str_cnt		: STD_LOGIC_VECTOR(How_many_Bits(C_Timing_str_cnt) DOWNTO 0);

	CONSTANT	C_D_Valid_to_DS_cnt		: INTEGER	:= set_ge_1(D_Valid_to_DS_in_ns * 1000 / Clk_in_ps)-2;	--	-2 because counter needs two more clock for unerflow
	SIGNAL		S_D_Valid_to_DS_cnt		: STD_LOGIC_VECTOR(How_many_Bits(C_D_Valid_to_DS_cnt) DOWNTO 0);

	CONSTANT	C_time_out_cnt			: INTEGER	:= set_ge_1(time_out_in_ns * 1000 / Clk_in_ps)-2;		--	-2 because counter needs two more clock for unerflow
	SIGNAL		s_time_out_cnt			: STD_LOGIC_VECTOR(How_many_Bits(C_time_out_cnt) DOWNTO 0);

	CONSTANT	C_SRQ_Polling_in_ns		: INTEGER	:= SRQ_Polling_in_ns;
	CONSTANT	C_Wait_cnt				: INTEGER	:= set_ge_1(C_SRQ_Polling_in_ns * 1000 / Clk_in_ps)-2;	--	-2 because counter needs two more clock for unerflow
	SIGNAL		S_Wait_cnt				: STD_LOGIC_VECTOR(How_many_Bits(C_wait_cnt) DOWNTO 0);

	CONSTANT	C_Reset_SCUB_Cnt		: INTEGER	:= set_ge_1(Reset_SCUB_in_ns * 1000 / Clk_in_ps)-2;		--	Vers_1_Revi_3: -2 because counter needs two more clock for unerflow
	SIGNAL		S_Reset_SCUB_Cnt		: STD_LOGIC_VECTOR(How_many_Bits(C_Reset_SCUB_Cnt) DOWNTO 0);		--	Vers_1_Revi_3


	SIGNAL		S_Reset_this_Macro		: STD_LOGIC;
	SIGNAL		S_Sync_Reset_this_Macro	: STD_LOGIC;

	SIGNAL		S_Fair_Addr				: STD_LOGIC_VECTOR(15 DOWNTO 0);
	SIGNAL		S_nFair_Adr_Sel			: STD_LOGIC;							-- Vers_1_Revi_2

	SIGNAL		S_Fair_RDnWR			: STD_LOGIC;
	SIGNAL		S_Fair_DS				: STD_LOGIC;

	SIGNAL		S_Slave_Index			: INTEGER RANGE nFair_SRQ_Slaves'low TO nFair_SRQ_Slaves'high;
	SIGNAL		S_Slave_Nr				: STD_LOGIC_VECTOR(3 DOWNTO 0);
	SIGNAL		S_Fair_Board_Sel		: STD_LOGIC_VECTOR(11 DOWNTO 0);
	SIGNAL		S_Board_Sel				: STD_LOGIC_VECTOR(11 DOWNTO 0);

	SIGNAL		S_Sel_Ext_Data_Drv		: STD_LOGIC;

	SIGNAL		S_Data_to_DB			: STD_LOGIC_VECTOR(15 DOWNTO 0);

	SIGNAL		S_FB_Rd					: STD_LOGIC_VECTOR(1 DOWNTO 0);		-- shift reg to generate pulse
	SIGNAL		S_Start_FB_Rd			: STD_LOGIC;

	SIGNAL		S_FB_Wr					: STD_LOGIC_VECTOR(1 DOWNTO 0);		-- shift reg to generate pulse
	SIGNAL		S_Start_FB_Wr			: STD_LOGIC;
	SIGNAL		S_Data_from_DB			: STD_LOGIC_VECTOR(15 DOWNTO 0);	-- store write pattern

	SIGNAL		S_Ti_Cy					: STD_LOGIC_VECTOR(1 DOWNTO 0);		-- shift reg to generate pulse
	SIGNAL		S_Start_Ti_Cy			: STD_LOGIC;

	SIGNAL		S_Wr_Sta				: STD_LOGIC_VECTOR(1 DOWNTO 0);		-- shift reg to generate pulse

	SIGNAL		S_nSync_Dtack			: STD_LOGIC;
	SIGNAL		S_Last_Cycle_Timing		: STD_LOGIC;
	SIGNAL		S_Fair_Timing_Cycle		: STD_LOGIC;

	SIGNAL		S_FB_Rd_Err_no_Dtack	: STD_LOGIC;
	SIGNAL		S_FB_Rd_Err_FB_Busy		: STD_LOGIC;
	SIGNAL		S_FB_Wr_Err_no_Dtack	: STD_LOGIC;
	SIGNAL		S_FB_Wr_Err_FB_Busy		: STD_LOGIC;

	SIGNAL		S_Ti_Cyc_Err			: STD_LOGIC;
	SIGNAL		S_Timing_In				: STD_LOGIC_VECTOR(15 DOWNTO 0);	-- store input timing_in
	SIGNAL		S_FB_Ti_Fin				: STD_LOGIC;

	SIGNAL		S_nSRQ_Ena				: STD_LOGIC_VECTOR(nFair_SRQ_Slaves'range);
	SIGNAL		S_nSRQ_Sync				: STD_LOGIC_VECTOR(nFair_SRQ_Slaves'range);
	SIGNAL		S_nSRQ_active			: STD_LOGIC_VECTOR(nFair_SRQ_Slaves'range);
	SIGNAL		S_one_or_more_SRQs_act	: STD_LOGIC;

	SIGNAL		S_Rd_Slave_Active_Intr_Flag	: STD_LOGIC;

	SIGNAL		Interlock_R			 	: STD_LOGIC_VECTOR(5 DOWNTO 0);
	SIGNAL		Data_RDY_R				: STD_LOGIC_VECTOR(10 DOWNTO 6);
	SIGNAL		Data_REQ_R				: STD_LOGIC_VECTOR(15 DOWNTO 11);
	SIGNAL		S_nInterlock			: STD_LOGIC;
	SIGNAL		S_nData_RDY				: STD_LOGIC;
	SIGNAL		S_nData_REQ				: STD_LOGIC;

	TYPE		T_Slave_Intr_act_array	IS ARRAY (nFair_SRQ_Slaves'high DOWNTO nFair_SRQ_Slaves'low) OF STD_LOGIC_VECTOR(15 DOWNTO 0);
	SIGNAL		S_Slave_Intr_act_array	: T_Slave_Intr_act_array;

	SIGNAL		S_Status				: STD_LOGIC_VECTOR(15 DOWNTO 0);
	SIGNAL		S_Read_Port				: STD_LOGIC_VECTOR(15 DOWNTO 0);

	SIGNAL		S_IFK_FB_Version		: STD_LOGIC_VECTOR(3 DOWNTO 0);
	SIGNAL		S_IFK_FB_Revision		: STD_LOGIC_VECTOR(3 DOWNTO 0);

	END IFK_FairBus;


ARCHITECTURE Arch_IFK_FairBus OF IFK_FairBus IS


	TYPE	T_FB_SM	IS	(
						Idle,
						S_Rd_Cyc,		-- start read fair bus cycle
						Rd_Cyc,			-- read fair bus read active
						E_Rd_Cyc,		-- end read fair bus
						F_Rd_Cyc,		-- finish read fair bus
						TO_Rd_Cyc,		-- time out read cycle
						S_Wr_Cyc,		-- start write fair bus cycle
						Wr_Cyc,			-- write fair bus active
						E_Wr_Cyc,		-- end write fair bus
						F_Wr_Cyc,		-- finish write fair bus
						TO_Wr_Cyc,		-- time out write cycle
						S_Ti_Cyc,		-- start Timing cycle
						Ti_Cyc,			-- Timing cycle active
						E_Ti_Cyc,		-- end Timing cycle
						F_Ti_Cyc		-- finish time cycle
						);

	SIGNAL	FB_SM	: T_FB_SM;


BEGIN

S_IFK_FB_Version	<= conv_std_logic_vector(C_IFK_FB_Version, 4);		-- set the version of this macro
S_IFK_FB_Revision	<= conv_std_logic_vector(C_IFK_FB_Revision, 4);		-- set the revision of this macro

ASSERT (False)
	REPORT "IFK_FairBus_Macro: Version --> " & integer'image(C_IFK_FB_Version)
			& ", Revision is --> " & integer'image(C_IFK_FB_Revision)
SEVERITY Warning;


ASSERT NOT (Clk_in_Hz < 100000000)
	REPORT "Achtung Generic Clk_in_Hz ist auf " & integer'image(Clk_in_Hz)
			& " gesetzt. Mit der Periodendauer von " & integer'image(Clk_in_ns)
			& " ns lassen sich keine genauen Verz�gerungen erzeugen!"
SEVERITY Warning;

ASSERT (False)
	REPORT "time_out_in_ns = " & integer'image(time_out_in_ns)
			& ",   Clk_in_ns = " & integer'image(Clk_in_ns)
			& ",   C_time_out_cnt = " & integer'image(C_time_out_cnt+2)
SEVERITY NOTE;

ASSERT (False)
	REPORT "Sel_dly_in_ns = " & integer'image(Sel_dly_in_ns)
			& ",   C_Sel_dly_cnt = " & integer'image(C_Sel_dly_cnt+2)
			& ",   Sel_release_in_ns = " & integer'image(Sel_release_in_ns)
			& ",   Sel_release_cnt = " & integer'image(C_Sel_release_cnt+2)
SEVERITY NOTE;

ASSERT (False)
	REPORT "Timing_str_in_ns = " & integer'image(Timing_str_in_ns)
			& ",   C_Timing_str_cnt = " & integer'image(C_Timing_str_cnt+2)
			& ",   D_Valid_to_DS_in_ns = " & integer'image(D_Valid_to_DS_in_ns)
			& ",   C_D_Valid_to_DS_cnt = " & integer'image(C_D_Valid_to_DS_cnt+2)
SEVERITY NOTE;

ASSERT NOT (C_SRQ_Polling_in_ns < 2000)
	REPORT "  Error! SRQ_Polling_in_ns is should be >= 2000 ns. Is is: " & integer'image(SRQ_Polling_in_ns) & " ns."
SEVERITY Error;

	S_Reset_this_Macro <= Reset_this_Macro; -- reset ist already synchronized

P_SCUB_Reset:	PROCESS	(clk, S_Reset_this_Macro)								-- Vers_1_Revi_3
	BEGIN																		-- Vers_1_Revi_3
		IF S_Reset_this_Macro = '1' THEN										-- Vers_1_Revi_3
			S_Reset_SCUB_Cnt <= (OTHERS => '0');								-- Vers_1_Revi_3
		ELSIF rising_edge(clk) THEN 											-- Vers_1_Revi_3
			IF S_Wr_Sta = "01" AND Data_from_DB(15) = '1' THEN					-- Vers_1_Revi_3
				S_Reset_SCUB_Cnt <= conv_std_logic_vector(C_Reset_SCUB_Cnt, S_Reset_SCUB_Cnt'length);	-- Vers_1_Revi_3
			ELSIF S_Reset_SCUB_Cnt(S_Reset_SCUB_Cnt'high) = '0' THEN			-- Vers_1_Revi_3
				S_Reset_SCUB_Cnt <= S_Reset_SCUB_Cnt - 1;						-- Vers_1_Revi_3
			END IF;																-- Vers_1_Revi_3
		END IF;																	-- Vers_1_Revi_3
	END PROCESS P_SCUB_Reset;


P_FB_Cntrl: PROCESS (clk, S_Reset_this_Macro)
	BEGIN
		IF S_Reset_this_Macro = '1' THEN
			S_FB_Rd(S_FB_Rd'range)	<= (OTHERS => '0');			-- shift reg to generate pulse
			S_FB_Wr(S_FB_Wr'range)	<= (OTHERS => '0');			-- shift reg to generate pulse
			S_Ti_Cy(S_Ti_Cy'range)	<= (OTHERS => '0');			-- shift reg to generate pulse
			S_Wr_Sta(S_Wr_Sta'range) <= (OTHERS => '0');		-- shift reg to generate pulse
			S_Start_FB_Rd			<= '0';						-- reset start fair bus read
			S_Start_FB_Wr			<= '0';						-- reset start fair bus write
			S_Start_Ti_Cy			<= '0';						-- reset start fair bus timing cycle
			S_FB_Rd_Err_FB_Busy		<= '0';						-- reset bus read error flag
			S_FB_Wr_Err_FB_Busy		<= '0';						-- reset bus write error flag
			S_Ti_Cyc_Err			<= '0';						-- reset timing error flag
			S_FB_Rd_Err_no_Dtack	<= '0';						-- reset read timeout flag
			S_FB_Wr_Err_no_Dtack	<= '0';						-- reset write timeout flag

		ELSIF rising_edge(clk) THEN

			S_FB_Rd(S_FB_Rd'range) <= (S_FB_Rd(S_FB_Rd'high-1 DOWNTO 0) & start_rd_fair);			-- shift reg to generate pulse
			S_FB_Wr(S_FB_Wr'range) <= (S_FB_Wr(S_FB_Wr'high-1 DOWNTO 0) & start_wr_fair);			-- shift reg to generate pulse
			S_Ti_Cy(S_Ti_Cy'range) <= (S_Ti_Cy(S_Ti_Cy'high-1 DOWNTO 0) & Start_Timing_Cycle);		-- shift reg to generate pulse
			S_Wr_Sta(S_Wr_Sta'range) <= (S_Wr_Sta(S_Wr_Sta'high-1 DOWNTO 0) & Wr_Status);		-- shift reg to generate pulse

			IF S_FB_Rd = "01" THEN														-- positive edge off start_rd_fair
				IF S_Start_FB_Rd = '1' OR S_Start_FB_Wr = '1' THEN
					S_FB_Rd_Err_FB_Busy <= '1';											-- fair bus read error bus is busy
				ELSE
					S_Start_FB_Rd <= '1';												-- store read request
				END IF;
			ELSIF S_Wr_Sta = "01" AND Data_from_DB(6) = '1' THEN						-- look to the bit position in status!
				S_FB_Rd_Err_FB_Busy <= '0';												-- reset fair bus read error
			END IF;

			IF  (FB_SM = F_Rd_Cyc OR FB_SM = TO_Rd_Cyc) AND S_Rd_Slave_Active_Intr_Flag = '0' THEN
				S_Start_FB_Rd <= '0';
			END IF;


			IF S_FB_Wr = "01" THEN														-- positive edge off start_wr_fair
				IF S_Start_FB_Wr = '1' OR S_Start_FB_Rd = '1' THEN
					S_FB_Wr_Err_FB_Busy <= '1';											-- fair bus write error bus is busy
				ELSE
					S_Start_FB_Wr <= '1';												-- store write request
					S_Data_from_DB <= Data_from_DB;										-- store write pattern
				END IF;
			ELSIF S_Wr_Sta = "01" AND Data_from_DB(2) = '1' THEN						-- look to the bit position in status!
				S_FB_Wr_Err_FB_Busy <= '0';												-- reset fair bus write error
			END IF;

			IF (FB_SM = F_Wr_Cyc) OR (FB_SM = TO_Wr_Cyc) THEN
				S_Start_FB_Wr <= '0';													-- write request finished
			END IF;


			IF S_Ti_Cy = "01" THEN														-- positive edge off start_timing_cycle
				IF S_Start_Ti_Cy = '1' THEN
					S_Ti_Cyc_Err <= '1';												-- fair bus timing error, new request but old request not finished
				ELSE
					S_Start_Ti_Cy <= '1';												-- store timing request
					S_Timing_In <= Timing_In;											-- store timing pattern
				END IF;
			ELSIF S_Wr_Sta = "01" AND Data_from_DB(9) = '1' THEN						-- look to the bit position in status!
				S_Ti_Cyc_Err <= '0';													-- reset fair bus timing error
			END IF;

			IF (FB_SM = F_Ti_Cyc) THEN
				S_Start_Ti_Cy <= '0';													-- timing request finished
			END IF;

			IF FB_SM = TO_Rd_Cyc THEN
				S_FB_Rd_Err_no_Dtack <= '1';											-- fair bus read error no dtack
			ELSIF S_Wr_Sta = "01" AND Data_from_DB(5) = '1' THEN						-- look to the bit position in status!
				S_FB_Rd_Err_no_Dtack <= '0';											-- reset fair bus read error no dtack
			END IF;

			IF FB_SM = TO_Wr_Cyc THEN
				S_FB_Wr_Err_no_Dtack <= '1';											-- fair bus write error no dtack
			ELSIF S_Wr_Sta = "01" AND Data_from_DB(1) = '1' THEN						-- look to the bit position in status
				S_FB_Wr_Err_no_Dtack <= '0';											-- fair bus write error no dtack
			END IF;


		END IF;
	END PROCESS P_FB_CNTRL;


P_FB_SM:	PROCESS (clk, S_Reset_this_Macro)

BEGIN
	IF S_Reset_this_Macro = '1' THEN
		FB_SM <= Idle;
		S_Last_Cycle_Timing	<= '0';
		S_Fair_Timing_Cycle	<= '0';
		S_Fair_RDnWR		<= '1';
		S_Fair_DS			<= '0';
		S_Fair_Board_Sel	<= (OTHERS => '0');
		S_Sel_Ext_Data_Drv	<= '0';
		S_nFair_Adr_Sel		<= '1';							-- Vers_1_Revi_2
		Ena_Tri_Buff		<= '0';		-- default: FPGA internal tristate buffer is tri state
		S_Slave_Index		<= nFair_SRQ_Slaves'low;
		FOR i IN nFair_SRQ_Slaves'low TO nFair_SRQ_Slaves'high LOOP
			S_Slave_Intr_act_array(i) <=(OTHERS => '0');
		END LOOP;
		S_Wait_Cnt <= conv_std_logic_vector(C_Wait_Cnt, S_Wait_Cnt'length);


	ELSIF rising_edge(clk) THEN

		S_nSync_Dtack <= nFair_Dtack;	-- Fair_Dtack is an asynchronous Signal. S_nSync_Dtack is the synchronized Fair_Dtack
		Ena_Tri_Buff <= '0';			-- default: FPGA internal tristate buffer is tri state

		IF S_Wait_Cnt(S_Wait_Cnt'high) = '0' THEN
			S_Wait_Cnt <= S_Wait_Cnt - 1;
		END IF;

		CASE FB_SM IS					-- = Fair Bus State Machine

			WHEN Idle =>
				S_Sel_dly_cnt		<= conv_std_logic_vector(C_Sel_dly_cnt, S_Sel_dly_cnt'length);
				S_D_Valid_to_DS_cnt	<= conv_std_logic_vector(C_D_Valid_to_DS_cnt, S_D_Valid_to_DS_cnt'length);
				S_Sel_release_cnt	<= conv_std_logic_vector(C_Sel_release_cnt, S_Sel_release_cnt'length);
				S_Fair_Board_Sel	<= (OTHERS => '0');
				S_nFair_Adr_Sel		<= '1';							-- Vers_1_Revi_2
				IF Drv_Adr_Idle_High = 1 THEN						-- Vers_1_Revi_2
					S_Fair_Addr			<= (OTHERS => '1');			-- Vers_1_Revi_2
				END IF;												-- Vers_1_Revi_2
				S_Fair_RDnWR		<= '1';
				S_Fair_Timing_Cycle	<= '0';
				S_Fair_DS			<= '0';
				S_Sel_Ext_Data_Drv	<= '0';


				IF ((S_Start_FB_Rd = '1') AND (S_Start_Ti_Cy = '0')) THEN
					S_nFair_Adr_Sel <= '0';								-- Vers_1_Revi_2
					if Sel_ACU_Mode = '0' then
						S_Fair_Addr	<= (X"0" & ADR_from_DB(11 DOWNTO 0));	-- set slave address; ADR_from_DB(15 DOWNTO 12) = coded slave number
																			-- is set in S_Rd_Cyc
					else
						S_Fair_Addr	<= ADR_from_DB(15 DOWNTO 0);			-- use hole ADR_from_DB, no coded slave number
					end if;
					S_Slave_Nr <= ADR_from_DB(15 DOWNTO 12);			-- ADR_from_DB(15 DOWNTO 12) = coded slave number
					S_Rd_Slave_Active_Intr_Flag <= '0';
					FB_SM <= S_Rd_Cyc;									-- jump to start read cycle
				ELSIF ((S_Start_FB_Wr = '1') AND (S_Start_Ti_Cy = '0')) THEN
					S_nFair_Adr_Sel <= '0';								-- Vers_1_Revi_2
					if Sel_ACU_Mode = '0' then							-- Vers_1_Revi_4
						S_Fair_Addr	<= (X"0" & ADR_from_DB(11 DOWNTO 0));	-- set slave address; ADR_from_DB(15 DOWNTO 12) = coded slave number
																			-- is set in S_Wr_Cyc
					else
						S_Fair_Addr	<= ADR_from_DB(15 DOWNTO 0);			-- use hole ADR_from_DB, no coded slave number
					end if;
					S_Slave_Nr <= ADR_from_DB(15 DOWNTO 12);			-- ADR_from_DB(15 DOWNTO 12) = coded slave number
					S_Fair_RDnWR <= '0';								-- set master writes
					FB_SM <= S_Wr_Cyc;									-- jump to start write cycle
				ELSIF ((S_Start_FB_Rd = '0') AND (S_Start_FB_Wr = '0') AND (S_Start_Ti_Cy = '1')) THEN
						S_Fair_RDnWR <= '0';								-- set master writes
						FB_SM <= S_Ti_Cyc;									-- jump to start Timing cycle
				ELSIF ((S_Start_FB_Wr = '1') AND (S_Start_Ti_Cy = '1')) THEN
					IF (S_Last_Cycle_Timing = '1') THEN
						S_nFair_Adr_Sel <= '0';								-- Vers_1_Revi_2
						if Sel_ACU_Mode = '0' then							-- Vers_1_Revi_4
							S_Fair_Addr	<= (X"0" & ADR_from_DB(11 DOWNTO 0));	-- set slave address; ADR_from_DB(15 DOWNTO 12) = coded slave number
																				-- is set in S_Wr_Cyc
						else
							S_Fair_Addr	<= ADR_from_DB(15 DOWNTO 0);			-- use hole ADR_from_DB, no coded slave number
						end if;
						S_Slave_Nr <= ADR_from_DB(15 DOWNTO 12);			-- ADR_from_DB(15 DOWNTO 12) = coded slave number
						S_Fair_RDnWR <= '0';								-- set master writes
						FB_SM <= S_Wr_Cyc;									-- jump to start write cycle
					ELSE
						S_Fair_RDnWR <= '0';								-- set master writes
						FB_SM <= S_Ti_Cyc;									-- jump to start Timing cycle
					END IF;
				ELSIF ((S_Start_FB_Rd = '1') AND (S_Start_Ti_Cy = '1')) THEN
					IF (S_Last_Cycle_Timing = '1') THEN
						S_nFair_Adr_Sel <= '0';								-- Vers_1_Revi_2
						if Sel_ACU_Mode = '0' then							-- Vers_1_Revi_4
							S_Fair_Addr	<= (X"0" & ADR_from_DB(11 DOWNTO 0));	-- set slave address; ADR_from_DB(15 DOWNTO 12) = coded slave number
																				-- is set in S_Rd_Cyc
						else
							S_Fair_Addr	<= ADR_from_DB(15 DOWNTO 0);			-- use hole ADR_from_DB, no coded slave number
						end if;
						S_Slave_Nr <= ADR_from_DB(15 DOWNTO 12);			-- ADR_from_DB(15 DOWNTO 12) = coded slave number
						S_Rd_Slave_Active_Intr_Flag <= '0';
						FB_SM <= S_Rd_Cyc;									-- jump to start read cycle
					ELSE
						S_Fair_RDnWR <= '0';								-- set master writes
						FB_SM <= S_Ti_Cyc;									-- jump to start Timing cycle
					END IF;
				ELSIF S_one_or_more_SRQs_act = '1' THEN
					IF S_nSRQ_active(S_Slave_Index) = '0' THEN
						IF S_Wait_Cnt(S_Wait_Cnt'high) = '1' THEN
							S_nFair_Adr_Sel <= '0';							-- Vers_1_Revi_2
							S_Fair_Addr	<= (X"0" & X"024");					--
							S_Slave_Nr <= conv_std_logic_vector(S_Slave_Index + 1, S_Slave_Nr'length);	-- S_nSRQ_active(11 Downto 0) = Slave_Nr 12 to 1
							S_Wait_Cnt <= conv_std_logic_vector(C_Wait_Cnt, S_Wait_Cnt'length);
							S_Rd_Slave_Active_Intr_Flag <= '1';
							FB_SM <= S_Rd_Cyc;									-- jump to start read cycle
						END IF;
					ELSE
						S_Slave_Intr_act_array(S_Slave_Index) <= (OTHERS => '0');		-- Clear the copy of the slave_active_intr_reg
						IF S_Slave_Index < nFair_SRQ_Slaves'high THEN
							S_Slave_Index <= S_Slave_Index + 1;
						ELSE
							S_Slave_Index <= nFair_SRQ_Slaves'low;
						END IF;
					END IF;
				ELSE
					FOR i IN nFair_SRQ_Slaves'low TO nFair_SRQ_Slaves'high LOOP
						S_Slave_Intr_act_array(i) <= (OTHERS => '0');		-- Clear the copy of the slave_active_intr_reg
					END LOOP;
					S_Slave_Index <= nFair_SRQ_Slaves'low;
				END IF;

			WHEN S_Rd_Cyc =>											-- start read cycle
				S_Sel_Ext_Data_Drv <= '1';
				S_Last_Cycle_Timing <= '0';								-- last fair bus cycle is a data transfer cycle
				IF S_Sel_dly_cnt(S_Sel_dly_cnt'high) = '1' THEN
					S_Fair_Board_Sel <= S_Board_Sel(11 DOWNTO 0);		-- select slave
					FB_SM <= Rd_Cyc;									-- jump to active read cycle
				END IF;

			WHEN Rd_Cyc =>												-- read cycle active
				IF S_D_Valid_to_DS_cnt(S_D_Valid_to_DS_cnt'high) = '1' THEN
					S_Fair_DS <= '1';
					IF S_nSync_Dtack = '0' THEN							-- wait for Dtack
						IF S_Rd_Slave_Active_Intr_Flag = '0' THEN
							S_Data_to_DB <= Data_from_Fair;
						ELSE
							S_Slave_Intr_act_array(S_Slave_Index) <= Data_from_Fair;
						END IF;
						S_Fair_DS <= '0';
						S_Fair_Board_Sel <= (OTHERS => '0');
						FB_SM <= E_Rd_Cyc;								-- jump to end read cycle
					ELSIF s_time_out_cnt(s_time_out_cnt'high) = '1' THEN
						S_Fair_DS <= '0';
						S_Fair_Board_Sel <= (OTHERS => '0');
						FB_SM <= TO_Rd_Cyc;									-- jump to read timeout
					END IF;
				END IF;

			WHEN TO_Rd_Cyc =>											-- read timeout
				FB_SM <= Idle;											-- Vers_1_Revi_1: jump to Idle

			WHEN E_Rd_Cyc =>											-- end read cycle
				S_Sel_Ext_Data_Drv <= '0';
				IF S_Sel_release_cnt(S_Sel_release_cnt'high) = '1' THEN
					IF S_Rd_Slave_Active_Intr_Flag = '1' THEN
						IF S_Slave_Index < nFair_SRQ_Slaves'high THEN
							S_Slave_Index <= S_Slave_Index + 1;
						ELSE
							S_Slave_Index <= nFair_SRQ_Slaves'low;
						END IF;
					END IF;
					FB_SM <= F_Rd_Cyc;									-- jump to finish read cycle
				END IF;

			WHEN F_Rd_Cyc =>
				FB_SM <= Idle;											-- jump to Idle

			WHEN S_Wr_Cyc =>											-- start write cycle
				S_Last_Cycle_Timing <= '0';								-- last fair bus cycle is a data transfer cycle
				Data_to_Fair <= S_Data_from_DB;							-- stored Data_from_DB (S_Data_from_DB) to Data_to_Fair
				Ena_Tri_Buff <= '1';									-- select FPGA internal tristate buffer for outut
				S_Sel_Ext_Data_Drv <= '1';
				IF S_Sel_dly_cnt(S_Sel_dly_cnt'high) = '1' THEN
					S_Fair_Board_Sel <= S_Board_Sel(11 DOWNTO 0);		-- select slave
					FB_SM <= Wr_Cyc;									-- jump to active write cycle
				END IF;

			WHEN Wr_Cyc =>												-- write cycle active
				Ena_Tri_Buff <= '1';									-- select FPGA internal tristate buffer for outut
				IF S_D_Valid_to_DS_cnt(S_D_Valid_to_DS_cnt'high) = '1' THEN
					S_Fair_DS <= '1';
					IF (S_nSync_Dtack = '0') OR (s_time_out_cnt(s_time_out_cnt'high) = '1') THEN	-- wait for Dtack or timeout
						S_Fair_DS <= '0';
						S_Sel_Ext_Data_Drv <= '0';
						S_Fair_Board_Sel <= (OTHERS => '0');
						IF s_time_out_cnt(s_time_out_cnt'high) = '0' THEN	-- no timeout
							FB_SM <= E_Wr_Cyc;								-- jump to end write cycle
						ELSE
							FB_SM <= TO_Wr_Cyc;								-- jump to write timeout
						END IF;
					END IF;
				END IF;

			WHEN TO_Wr_Cyc =>											-- write timeout
					S_Fair_RDnWR <= '1';								-- set master reades
					FB_SM <= Idle;										-- jump to Idle

			WHEN E_Wr_Cyc =>											-- end write cycle
				IF S_Sel_release_cnt(S_Sel_release_cnt'high) = '1' THEN
					S_Fair_RDnWR <= '1';								-- set master reades
					FB_SM <= F_Wr_Cyc;									-- jump to finish write cycle
				END IF;

			WHEN F_Wr_Cyc =>
					FB_SM <= Idle;										-- jump to Idle

			WHEN S_Ti_Cyc =>											-- start Timing cycle
				S_Last_Cycle_Timing <= '1';								-- last fair bus cycle is a timing cycle
				Ena_Tri_Buff <= '1';									-- select FPGA internal tristate buffer for outut
				S_nFair_Adr_Sel <= '0';									-- Vers_1_Revi_2
				S_Fair_Addr	<= Timing_In;								-- Timing to Fair_Addr
				Data_to_Fair <= S_Timing_In;							-- stored Timing_In (S_Timing_In) to Data_to_Fair
				IF S_Sel_dly_cnt(S_Sel_dly_cnt'high) = '1' THEN
					S_Sel_Ext_Data_Drv <= '1';
					S_Fair_Board_Sel <= (OTHERS => '1');				-- in this version select all slaves.
					S_Timing_str_cnt <= conv_std_logic_vector(C_Timing_str_cnt, S_Timing_str_cnt'length);
					FB_SM <= Ti_Cyc;									-- jump to active Timing cycle
				END IF;

			WHEN Ti_Cyc =>												-- Timing cycle active
				S_Fair_Timing_Cycle <= '1';								-- timing cycle signal active
				Ena_Tri_Buff <= '1';									-- select FPGA internal tristate buffer for outut
				IF S_Timing_str_cnt(S_Timing_str_cnt'high) = '1' THEN
					S_Fair_Timing_Cycle <= '0';							-- timing cycle signal inactive
					S_Fair_Board_Sel <= (OTHERS => '0');				-- deselect all slaves.
					FB_SM <= E_Ti_Cyc;									-- jump to end Timing cycle
				END IF;

			WHEN E_Ti_Cyc =>											-- end Timing cycle
				IF S_Sel_release_cnt(S_Sel_release_cnt'high) = '1' THEN
					S_Fair_RDnWR <= '1';								-- set master reades
					S_Sel_Ext_Data_Drv <= '0';
					FB_SM <= F_Ti_Cyc;									-- jump to finish time cycle
				END IF;

			WHEN F_Ti_Cyc =>
					FB_SM <= Idle;										-- jump to Idle

			WHEN OTHERS =>
				FB_SM <= Idle;

		END CASE;


		IF ((FB_SM = S_Wr_Cyc) OR (FB_SM = S_Rd_Cyc) OR (FB_SM = S_Ti_Cyc)) AND S_Sel_dly_cnt(S_Sel_dly_cnt'high) = '0' THEN
			S_Sel_dly_cnt <= S_Sel_dly_cnt - 1;
		END IF;

		IF ((FB_SM = Wr_Cyc) OR (FB_SM = Rd_Cyc)) AND S_D_Valid_to_DS_cnt(S_D_Valid_to_DS_cnt'high) = '0' THEN
			S_D_Valid_to_DS_cnt <= S_D_Valid_to_DS_cnt - 1;
		END IF;

		IF ((FB_SM = E_Wr_Cyc) OR (FB_SM = E_Rd_Cyc) OR (FB_SM = E_Ti_Cyc)) AND S_Sel_release_cnt(S_Sel_release_cnt'high) = '0' THEN
			S_Sel_release_cnt <= S_Sel_release_cnt - 1;
		END IF;

		IF FB_SM = Ti_Cyc AND S_Timing_str_cnt(S_Timing_str_cnt'high) = '0' THEN
			S_Timing_str_cnt <= S_Timing_str_cnt - 1;
		END IF;

	END IF;
END PROCESS P_FB_SM;


p_board_sel:	PROCESS (clk, S_Reset_this_Macro)
	BEGIN
		IF S_Reset_this_Macro = '1' THEN
			S_Board_Sel <= "000000000000";						-- no board select
		ELSIF rising_edge(clk) THEN
			if Sel_ACU_Mode = '0' then 
				CASE S_Slave_Nr IS
					WHEN X"0" =>	S_Board_Sel <= "000000000000";
					WHEN X"1" =>	S_Board_Sel <= "000000000001";	-- select board 1
					WHEN X"2" =>	S_Board_Sel <= "000000000010";
					WHEN X"3" =>	S_Board_Sel <= "000000000100";
					WHEN X"4" =>	S_Board_Sel <= "000000001000";
					WHEN X"5" =>	S_Board_Sel <= "000000010000";
					WHEN X"6" =>	S_Board_Sel <= "000000100000";
					WHEN X"7" =>	S_Board_Sel <= "000001000000";
					WHEN X"8" =>	S_Board_Sel <= "000010000000";
					WHEN X"9" =>	S_Board_Sel <= "000100000000";
					WHEN X"A" =>	S_Board_Sel <= "001000000000";
					WHEN X"B" =>	S_Board_Sel <= "010000000000";
					WHEN X"C" =>	S_Board_Sel <= "100000000000";	-- select board 12
					WHEN OTHERS =>  S_Board_Sel <= "000000000000";	-- no board select
				END CASE;
			else
				S_Board_Sel <= "000000000001";	-- select board 1
			end if;
		END IF;
	END PROCESS p_board_sel;


p_intr:	PROCESS (clk, S_Reset_this_Macro)
	BEGIN
		IF S_Reset_this_Macro = '1' THEN
			S_nSRQ_Ena		<= "111111111110";					-- only nSRQ(1) is enabled
			S_nSRQ_Sync		<= "111111111111";					-- set synchronized nSRQs to no nSRQ
			S_nSRQ_active	<= "111111111111";					-- set active nSRQs to no nSRQ
		ELSIF rising_edge(clk) THEN
			S_nSRQ_Sync <= nFair_SRQ_Slaves;					-- synchronize nFair_SRQ_Slave Signals; Bit(n) = '0' => SRQ(n) is active
			IF Wr_SRQ_Ena = '1' THEN
				S_nSRQ_Ena <= not Data_from_DB(nFair_SRQ_Slaves'high DOWNTO 0);
			END IF;

			FOR i IN nFair_SRQ_Slaves'range LOOP
				IF S_nSRQ_Ena(i) = '0' THEN
					IF S_nSRQ_Sync(i) = '0' THEN
						S_nSRQ_active(i) <= '0';
					ELSE
						S_nSRQ_active(i) <= '1';
					END IF;
				ELSE
					S_nSRQ_active(i) <= '1';	-- ???
				END IF;
			END LOOP;

			IF S_nSRQ_active /= 2**(nFair_SRQ_Slaves'length)-1 THEN
				S_one_or_more_SRQs_act <= '1';
			ELSE
				S_one_or_more_SRQs_act <= '0';
			END IF;

			S_nInterlock <= '1';
			S_nData_RDY <= '1';
			S_nData_REQ <= '1';
			FOR n IN nFair_SRQ_Slaves'range LOOP
				IF S_Slave_Intr_act_array(n)(Interlock_R'range) /= conv_std_logic_vector(0, Interlock_R'length) THEN
					S_nInterlock <= '0';
				END IF;
				IF S_Slave_Intr_act_array(n)(Data_RDY_R'range) /= conv_std_logic_vector(0, Data_RDY_R'length) THEN
					S_nData_RDY <= '0';
				END IF;
				IF S_Slave_Intr_act_array(n)(Data_REQ_R'range) /= conv_std_logic_vector(0, Data_REQ_R'length) THEN
					S_nData_REQ <= '0';
				END IF;
			END LOOP;

		END IF;
	END PROCESS p_intr;


p_read_mux:	PROCESS (
					Rd_Vers_Revi, S_IFK_FB_Version, S_IFK_FB_Revision,
					Rd_Srq_Ena, S_nSRQ_Ena,
					Rd_Srq_active, S_nSRQ_active,
					Rd_Srq_In, S_nSRQ_Sync,
					Rd_Status, S_Status
					)

VARIABLE	 sel : STD_LOGIC_VECTOR(4 DOWNTO 0);										-- Vers_1_Revi_1

	BEGIN
		sel := (Rd_Status, Rd_Vers_Revi, Rd_Srq_Ena, Rd_Srq_active, Rd_Srq_In);			-- Vers_1_Revi_1
		CASE sel IS
			WHEN "00001"	=> S_Read_Port <= ("0000" & S_nSRQ_Sync);
							Read_Port_active <= '1';									-- Vers_1_Revi_1
			WHEN "00010"	=> S_Read_Port <= ("0000" & S_nSRQ_active);
							Read_Port_active <= '1';									-- Vers_1_Revi_1
			WHEN "00100"	=> S_Read_Port <= ("0000" & S_nSRQ_Ena);
							Read_Port_active <= '1';									-- Vers_1_Revi_1
			WHEN "01000"	=> S_Read_Port <= (X"00" & S_IFK_FB_Version & S_IFK_FB_Revision);
							Read_Port_active <= '1';									-- Vers_1_Revi_1
			WHEN "10000" 	=> S_Read_Port <= S_Status;									-- Vers_1_Revi_1
							Read_Port_active <= '1';									-- Vers_1_Revi_1
			WHEN OTHERS	=> S_Read_Port <= S_Status;
							Read_Port_active <= '0';									-- Vers_1_Revi_1
		END CASE;
	END PROCESS p_read_mux;


p_time_out:	PROCESS (Clk, S_Reset_this_Macro)
	BEGIN
		IF S_Reset_this_Macro = '1' THEN
			s_time_out_cnt <= conv_std_logic_vector(C_time_out_cnt, s_time_out_cnt'length);
		ELSIF rising_edge(Clk) THEN
			IF FB_SM = Idle THEN
				s_time_out_cnt <= conv_std_logic_vector(C_time_out_cnt, s_time_out_cnt'length);
			ELSIF s_time_out_cnt(s_time_out_cnt'high) = '0' THEN									-- no underflow
				s_time_out_cnt <= s_time_out_cnt - 1;												-- count down
			END IF;
		END IF;
	END PROCESS p_time_out;


Fair_Addr			<= S_Fair_Addr;
nFair_Adr_Sel		<= '0' WHEN Drv_Adr_Idle_High = 1 ELSE S_nFair_Adr_Sel;				-- Vers_1_Revi_2 wenn der ext. Adr-Treiber am Buszyklusende auf Tristate geschaltet
																						-- werden soll, wird das Signal S_nFair_Adr_Sel verwendet. im anderen Fall bleibt der
																						-- ext. Adr-Treiber immer aktiv
Fair_RDnWR			<= S_Fair_RDnWR;
nFair_DS			<= NOT S_Fair_DS;
nFair_Board_Sel		<= NOT S_Fair_Board_Sel;

nFair_Timing_Cycle	<= NOT S_Fair_Timing_Cycle;

nSCUB_Reset <= S_Reset_SCUB_Cnt(S_Reset_SCUB_Cnt'high); -- Vers_1_Revi_3, solange S_Reset_SCUB_Cnt nicht in den �berlauf heruntergez�hlt hat ist das Bit = '0', d.h. der Reset ist null aktiv.

Data_to_DB	<= S_Read_Port WHEN Rd_Srq_Ena = '1' OR Rd_Srq_active = '1' OR Rd_Srq_In = '1' OR Rd_Status = '1' ELSE S_Data_to_DB;

nSel_Ext_Data_Drv	<= NOT S_Sel_Ext_Data_Drv;
Ext_Data_Drv_WRnRd	<= NOT S_Fair_RDnWR;

nStart_DB_Trm		<= '0' WHEN (FB_SM = F_Rd_Cyc AND S_Rd_Slave_Active_Intr_Flag = '0') ELSE '1';

FB_Rd_active		<= S_Start_FB_Rd;
FB_Rd_Fin			<= '1' WHEN FB_SM = F_Rd_Cyc ELSE '0';
FB_Rd_Err_no_Dtack	<= S_FB_Rd_Err_no_Dtack;
FB_Rd_Err_FB_Busy	<= S_FB_Rd_Err_FB_Busy;

FB_Wr_active		<= S_Start_FB_Wr;
FB_Wr_Fin			<= '1' WHEN FB_SM = F_Wr_Cyc ELSE '0';
FB_Wr_Err_no_Dtack	<= S_FB_Wr_Err_no_Dtack;
FB_Wr_Err_FB_Busy	<= S_FB_Wr_Err_FB_Busy;

S_FB_Ti_Fin			<= '1' WHEN FB_SM = F_Ti_Cyc ELSE '0';
FB_Ti_Fin			<= S_FB_Ti_Fin;

S_Status	<=	(
				  not S_Reset_SCUB_Cnt(S_Reset_SCUB_Cnt'high)	&				    '0' & S_Wait_Cnt(S_Wait_Cnt'high) & S_Rd_Slave_Active_Intr_Flag	-- Bit 15..12 -- Vers_1_Revi_3 : Bit[15] = '1' = SCUB_Reset aktiv
				& '0'											&					'0' & S_Ti_Cyc_Err		   & S_Start_Ti_Cy	-- Bit 11..8
				& '0'											& S_FB_Rd_Err_FB_Busy & S_FB_Rd_Err_no_Dtack & S_Start_FB_Rd	-- Bit 7..4
				& '0'											& S_FB_Wr_Err_FB_Busy & S_FB_Wr_Err_no_Dtack & S_Start_FB_Wr	-- Bit 3..0
				);

Status <= S_Status;

Read_Port <= S_Read_Port;

nInterlock <= S_nInterlock;
nData_RDY <= S_nData_RDY;
nData_REQ <= S_nData_REQ;

IFK_FB_Version <= S_IFK_FB_Version;
IFK_FB_Revision <= S_IFK_FB_Revision;

END Arch_IFK_FairBus;
