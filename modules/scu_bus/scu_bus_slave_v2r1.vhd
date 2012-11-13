--TITLE "'SCU_Bus_Slave_Interface'
--
------------------------------------------------------------------------------------------------------------------------------------------
--	Vers: 1 Revi: 0: erstellt am 23.07.2009, Autor: W.Panschow																			--
--																																		--
--	Das SCU_Bus-Slave-Interface (FSI) soll Entwicklern von SCU_Bus-Slave-Karten ein standardisiertes und getestetes Interface zum		--
--	SCU_Bus	bereitstellen. Im wesentlichen �bernimmt das FSI drei Funktionen:															--
--		a)	Die Datenkommunikation mit dem SCU_Bus-Master.																				--
--		b)	Den Empfang, der vom SCU_Bus-Master �ber den SCU_Bus verteilten, Timing-Informationen.										--
--		c)	Den Interrupt-Controller, der 16 m�gliche Interruptquellen einer Slave-Karte auf ein Service-Request-Signal zum				--
--			SCU_Bus-Master abbildet.																									--
------------------------------------------------------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------------------------------------------------------
--	Vers_1_Revi_1: erstellt am 19.01.2010, Autor: W.Panschow																			--
--																																		--
--		Vorgenommene �nderungen:																										--
--			a) Das Interface des SCU_Bus_Slaves f�r nachgelagerte (externe) User-Funktionen war nicht optimal ausgelegt.				--
--				a1)	Der Lese-Zugriff auf User-Register musste durch eine UND-Verkn�pfung des SCU_Bus_Slave-Ausgangs "ADR_Val" mit dem	--
--					externen SCU_Bus-Signal "SCUB_RDnWR" ausserhalb dieses Macros erzeugt werden. Diese Verkn�pfung wird jetzt			--
--					innerhalb des Makros vorgenommen und als neues Ausgangssignal "Ext_Rd_active" bereitgestellt.						--
--				a2) Schreibzugriffe auf Useer-Register wurden mit dem SCU_Bus_Slave-Ausgang "DS_Val" signalisiert. Zur Verdeutlichung,	--
--					ist der Ausgang "DS_Val" in "Ext_Wr_active" umbenannt worden.														--
--			b)	Es kann die Version und die Revision dieses Makros zur�ckgelesen werden.												--
--				Unter der Adresse X"0006" kann im High Byte die Version und im Low Byte die Revision ausgelesen werden.					--
--				Achtung das funktioniert nur, wenn bei �nderungen auch die Konstanten "C_Version" und "C_Revision" ge�ndert werden!		--
--																																		--
------------------------------------------------------------------------------------------------------------------------------------------
			
------------------------------------------------------------------------------------------------------------------------------------------
--	Vers_1_Revi_2: erstellt am 21.05.2010, Autor: W.Panschow																			--
--																																		--
--		Vorgenommene �nderungen:																										--
--			a) Zwei neue Signale sollen das Interface zu externen Registern (oder Fifos) erleichtern.									--
--					"Ext_Wr_fin" steht f�r extern-write-finished. Es signalisiert f�r eine Clockperiode von "clk" mit einem				--
--					Aktiv-Eins-Pegel das Ende eines Schreibzyklusses vom SCU-Bus. Das Signal ist nur f�r ausserhalb dieses Makros		--
--					liegende (externe) Registerzugriffe aktiv.																			--
--					"Ext_Rd_fin" steht f�r extern-read-finished. Es signalisiert f�r eine Clockperiode von "clk" mit einem				--
--					Aktiv-Eins-Pegel das Ende eines Lesezyklusses vom SCU-Bus. Das Signal ist nur f�r ausserhalb dieses Makros			--
--					liegende (externe) Registerzugriffe aktiv.																			--
--																																		--
--			b) Generic-Parameter "Ext_Fin_Sig_overlap" ist hinzugekommen.																--
--																																		--
--					"Ext_Fin_Sig_overlap" = 1 => Die Signale �berlappen sich am Ende des exterenen Buszyklusses f�r einen Takt.			--
--						"Ext_Rd_Fin" oder "Ext_Wr_Fin"		_________________________|===|___________									--
--						"Ext_Rd_active oder "Ext_Wr_active	_________|===================|___________									--
--						"Clk"								_|=|_|=|_|=|_|=|_|=|_|=|_|=|_|=|_|=|_|=|_									--
--																																		--
--					"Ext_Fin_Sig_overlap" = 0 => Die Signale folgen einen Takt nach dem Ende des externen Buszyklusses.					--
--						"Ext_Rd_Fin" oder "Ext_Wr_Fin"		_____________________________|===|_______									--
--						"Ext_Rd_active oder "Ext_Wr_active	_________|===================|___________									--
--						"Clk"								_|=|_|=|_|=|_|=|_|=|_|=|_|=|_|=|_|=|_|=|_									--
--																																		--
--			c) Anpassungen vorgenommen, damit die Standard-Bibliothek "IEEE.numeric_std.all" verwendet werden kann.						--
--																																		--
--			d) Die Adressen X"0000" bis X"0026" sind jetzt durchg�ngig f�r Resourcen innerhalb dieses Makros reserviert.				--
--			   D.h. externe (user) Register k�nnen erst ab der Adresse X"0027" angesprochen werden.										-- 
--																																		--
------------------------------------------------------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------------------------------------------------------
--	Vers_2_Revi_0: erstellt am 26.07.2010, Autor: W.Panschow																			--
--																																		--
--		Vorgenommene �nderungen:																										--
--			a)	Die unter Vers_1_Revi_2 hinzugef�gten Signale "Ext_Wr_Fin" und "Ext_Rd_Fin" konnten mit dem Generic-Parameter			--
--				"Ext_Fin_Sig_overlap" so eingestellt werden, dass sie gleichzeitig mit dem Ende des jeweiligen Schreib- oder 			--
--				Lesezyklusses aktiv werden, oder erst nach dem Abschluss des Zugriffs.													--
--				Dies hat den Nachteil, dass die Signale f�r alle externen Makros nur in der einen oder anderen Form zur Verf�gung		--
--				stehen. Besser ist es, beide Formen parallel anzubieten. Deshalb sind zwei neue Signale hinzugekommen,					--
--					"Ext_Wr_Fin_ovl" und "Ext_Rd_Fin_ovl".																				--
--					Beide Signale beenden den jeweiligen Zyklus mit einen Takt �berlappung.												--
--				"Ext_Wr_Fin" und "Ext_Rd_Fin" werden jetzt immer erst nach dem Ende des jeweiligen Zyklusses f�r einen Takt aktiv		--
--				sein (keine	�berlappung).																								--
--				Der Generic-Parameter "Ext_Fin_Sig_overlap" wird aus obigen Gr�nden nicht mehr be�tigt.									--
--																																		--
--			b)	Auf Vorschlag von S. Sch�fer ist der Lesezyklus des SCU_Busses �berarbeitet worden.										--
--				b1)	Zu Beginn des Lesezugriffes wurde immer f�r einen Takt das Datum des vorherigen Lesezugriffes auf den SCU_Bus-		--
--					Datenbus angelegt bevor das gew�nschte Datum nachfolgte. Verursacht unn�tige St�rungen.								--
--				b2)	Bei einem Lesezugriff auf eine  n i c h t  existierende extene Resource wurde auf den SCU_Bus-Datenbus das Datum	--
--					des zuvor ausgef�hrten Lesezugriffes ausgegeben. Es wurde zwar kein Dtack generiert, aber den Datenbus-Einschwing-	--
--					vorgang ist un�tig.																									--
--																																		-- 
------------------------------------------------------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------------------------------------------------------
--	Vers_2_Revi_1: erstellt am 02.08.2010, Autor: W.Panschow																			--
--																																		--
--		Vorgenommene �nderungen:																										--
--			a)	Mit der unter Vers_2_Revi_0 -b) vorgenommenen �nderung in der Aktivierung des SCU_Bus-Datentreibers, wird das Dtack		--
--				der User-Register um ca. 20 ns verz�gert.																				--
--																																		-- 
------------------------------------------------------------------------------------------------------------------------------------------

library IEEE;
USE IEEE.STD_LOGIC_1164.all;
USE IEEE.numeric_std.all;
use ieee.math_real.all;

ENTITY SCU_Bus_Slave_V2R1 IS
		
	GENERIC
		(
		CLK_in_Hz			: INTEGER := 100_000_000;						-- frequenz of the "SCU_Bus_Slave" clock in Hz, should be higher then 100 Mhz
		Slave_ID			: INTEGER	RANGE 0 TO 16#FFFF#	:= 0;			-- ID of the realisied slave board function
		Firmware_Version	: INTEGER	RANGE 0 to 16#FFFF#	:= 0;			-- 
		Firmware_Release	: INTEGER	RANGE 0 to 16#FFFF#	:= 0;
		Hardware_Version	: INTEGER	RANGE 0 to 16#FFFF#	:= 0;
		Hardware_Release	: INTEGER	RANGE 0 to 16#FFFF#	:= 0;
		Intr_Level_Neg	: STD_LOGIC_VECTOR(15 DOWNTO 1) := B"0000_0000_0000_000";	-- the bit positions are corresponding to Intr_In.
																			-- A '1' set default level of this Intr_In(n) to neg. level or neg. edge
		Intr_Edge_Trig	: STD_LOGIC_VECTOR(15 DOWNTO 1) := B"1111_1111_1111_110";	-- the bit positions are corresponding to Intr_In.	
																			-- A '1' set default of this Intr_In(n) to edge triggered, '0' is level triggered
		Intr_Enable		: STD_LOGIC_VECTOR(15 DOWNTO 1) := B"0000_0000_0000_001"	-- the bit positions are corresponding to Intr_In.	
																			-- A '1' enable Intr_In(n), '0' disable Intr_In(n)
		);
					
	PORT(
		SCUB_Addr			: IN		STD_LOGIC_VECTOR(15 DOWNTO 0);		-- SCU_Bus: address bus
		nSCUB_Timing_Cyc	: IN		STD_LOGIC;							-- SCU_Bus signal: low active SCU_Bus runs timing cycle
		SCUB_Data			: INOUT		STD_LOGIC_VECTOR(15 DOWNTO 0);		-- SCU_Bus: data bus (FPGA tri state buffer)
		nSCUB_Slave_Sel		: IN		STD_LOGIC;							-- SCU_Bus: '0' => SCU master select slave
		nSCUB_DS			: IN		STD_LOGIC;							-- SCU_Bus: '0' => SCU master activate data strobe
		SCUB_RDnWR			: IN		STD_LOGIC;							-- SCU_Bus: '1' => SCU master read slave
		clk					: IN		STD_LOGIC;							-- clock of "SCU_Bus_Slave"
		nReset				: IN		STD_LOGIC;							-- SCU_Bus: '0' => nReset is active
		Data_to_SCUB		: IN		STD_LOGIC_VECTOR(15 DOWNTO 0);		-- connect read sources from external user functions
		Dtack_to_SCUB		: IN		STD_LOGIC;							-- connect Dtack from from external user functions
		Intr_In				: IN		STD_LOGIC_VECTOR(15 DOWNTO 1) := NOT Intr_Level_Neg(15 Downto 1);		-- 15 interrupts from external user functions
		User_Ready			: IN		STD_LOGIC;							-- '1' => the user function(s), device, is ready to work with the control system
		Data_from_SCUB_LA	: OUT		STD_LOGIC_VECTOR(15 DOWNTO 0);		-- latched data from SCU_Bus for external user functions 
		ADR_from_SCUB_LA	: OUT		STD_LOGIC_VECTOR(15 DOWNTO 0);		-- latched address from SCU_Bus for external user functions
		Timing_Pattern_LA	: OUT		STD_LOGIC_VECTOR(31 DOWNTO 0);		-- latched timing pattern from SCU_Bus for external user functions
		Timing_Pattern_RCV	: OUT		STD_LOGIC;							-- timing pattern received
		nSCUB_Dtack_Opdrn	: OUT		STD_LOGIC;							-- for direct connect to SCU_Bus opendrain signal - '0' => slave give dtack to SCU master
		SCUB_Dtack			: OUT		STD_LOGIC;							-- for connect via ext. open collector driver - '1' => slave give dtack to SCU master
		nSCUB_SRQ_Opdrn		: OUT		STD_LOGIC;							-- for direct connect to SCU_Bus opendrain signal - '0' => slave service request to SCU master
		SCUB_SRQ			: OUT		STD_LOGIC;							-- for connect via ext. open collector driver - '1' => slave service request to SCU master
		nSel_Ext_Data_Drv	: OUT		STD_LOGIC;							-- '0' => select the external data driver on the SCU_Bus slave
		Ext_Data_Drv_Rd		: OUT		STD_LOGIC;							-- '1' => direction of the external data driver on the SCU_Bus slave is to the SCU_Bus
		Standard_Reg_Acc	: OUT		STD_LOGIC;							-- '1' => mark the access to register of this macro
		Ext_Adr_Val			: OUT		STD_LOGIC;							-- for external user functions: '1' => "ADR_from_SCUB_LA" is valid
		Ext_Rd_active		: OUT		STD_LOGIC;							-- '1' => Rd-Cycle to external user register is active
		Ext_Rd_fin			: OUT		STD_LOGIC;							-- marks end of read cycle, active one for one clock period of clk past cycle end (no overlap)
		Ext_Rd_Fin_ovl		: OUT		STD_LOGIC;							-- marks end of read cycle, active one for one clock period of clk during cycle end (overlap)
		Ext_Wr_active		: OUT		STD_LOGIC;							-- '1' => Wr-Cycle to external user register is active
		Ext_Wr_fin			: OUT		STD_LOGIC;							-- marks end of write cycle, active one for one clock period of clk past cycle end (no overlap)
		Ext_Wr_fin_ovl		: OUT		STD_LOGIC;							-- marks end of write cycle, active one for one clock period of clk during cycle end (overlap)
		nPowerup_Res		: OUT		STD_LOGIC							-- '0' => the FPGA make a powerup
		);
		
	CONSTANT	Clk_in_ps			: INTEGER	:= 1000000000 / (Clk_in_Hz / 1000);
	CONSTANT	Clk_in_ns			: INTEGER	:= 1000000000 / Clk_in_Hz;
	
	CONSTANT	C_Version			: STD_LOGIC_VECTOR(7 DOWNTO 0) :=  X"02";		-- increment by major changes
	CONSTANT	C_Revision			: STD_LOGIC_VECTOR(7 DOWNTO 0) :=  X"01";		-- increment by minor changes
	
	CONSTANT	C_Slave_ID_Adr		: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0001";		-- address of slave ident code (rd)
	CONSTANT	C_FW_Version_Adr	: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0002";		-- address of firmware version (rd)
	CONSTANT	C_FW_Release_Adr	: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0003";		-- address of firmware release (rd)
	CONSTANT	C_HW_Version_Adr	: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0004";		-- address of hardware version (rd)
	CONSTANT	C_HW_Release_Adr	: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0005";		-- address of hardware release (rd)
	CONSTANT	C_Vers_Revi_of_this_Macro: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0006";	-- address of version and revision register of this macro (rd)
	CONSTANT	C_Echo_Reg_Adr		: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0010";		-- address of echo register (rd/wr)
	CONSTANT	C_Status_Reg_Adr	: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0011";		-- address of status register (rd)
	
	CONSTANT	C_Free_Intern_A_12	: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0012";		-- reserved internal address 12hex
	CONSTANT	C_Free_Intern_A_13	: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0013";		-- reserved internal address 13hex
	CONSTANT	C_Free_Intern_A_14	: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0014";		-- reserved internal address 14hex
	CONSTANT	C_Free_Intern_A_15	: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0015";		-- reserved internal address 15hex
	CONSTANT	C_Free_Intern_A_16	: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0016";		-- reserved internal address 16hex
	CONSTANT	C_Free_Intern_A_17	: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0017";		-- reserved internal address 17hex
	CONSTANT	C_Free_Intern_A_18	: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0018";		-- reserved internal address 18hex
	CONSTANT	C_Free_Intern_A_19	: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0019";		-- reserved internal address 19hex
	CONSTANT	C_Free_Intern_A_1A	: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"001A";		-- reserved internal address 1Ahex
	CONSTANT	C_Free_Intern_A_1B	: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"001B";		-- reserved internal address 1Bhex
	CONSTANT	C_Free_Intern_A_1C	: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"001C";		-- reserved internal address 1Chex
	CONSTANT	C_Free_Intern_A_1D	: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"001D";		-- reserved internal address 1Dhex
	CONSTANT	C_Free_Intern_A_1E	: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"001E";		-- reserved internal address 1Ehex
	CONSTANT	C_Free_Intern_A_1F	: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"001F";		-- reserved internal address 1Fhex

	CONSTANT	C_Intr_In_Adr		: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0020";		-- address of interrupt In register (rd)
	CONSTANT	C_Intr_Ena_Adr		: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0021";		-- address of interrupt enable register (rd/wr)
	CONSTANT	C_Intr_Pending_Adr	: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0022";		-- address of interrupt pending register (rd/wr)
	CONSTANT	C_Intr_Mask_Adr		: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0023";		-- address of interrupt mask register (rd/wr)
	CONSTANT	C_Intr_Active_Adr	: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0024";		-- address of interrupt active register (rd)
	CONSTANT	C_Intr_Level_Adr	: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0025";		-- address of interrupt level register (rd/wr)
	CONSTANT	C_Intr_Edge_Adr		: STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0026";		-- address of interrupt edge register (rd/wr)

	SIGNAL		S_nReset			: STD_LOGIC;							-- '0' => S_nReset is active

	SIGNAL		S_ADR_from_SCUB_LA	: STD_LOGIC_VECTOR(15 DOWNTO 0);		-- latched address from SCU_Bus
	SIGNAL		S_Adr_Val			: STD_LOGIC;							-- for external address decoding "ADR_from_SCUB_LA" is valid
	SIGNAL		S_Data_from_SCUB_LA	: STD_LOGIC_VECTOR(15 DOWNTO 0);		-- latched data from SCU_Bus
	SIGNAL		S_Timing_Pattern_LA	: STD_LOGIC_VECTOR(31 DOWNTO 0);		-- latched timing pattern from SCU_Bus
	SIGNAL		S_Timing_Pat_RCV	: STD_LOGIC;							-- generate pulse if a new timing pattern latched
	SIGNAL		S_Timing_Pat_RCV_Dly: STD_LOGIC;							-- generate delayed pulse if a new timing pattern latched

	SIGNAL		S_nSync_Board_Sel	: STD_LOGIC_VECTOR(1 DOWNTO 0);			-- synchronize nSCUB_Slave_Sel and generate pulse on neg edge
	SIGNAL		S_nSync_DS			: STD_LOGIC_VECTOR(1 DOWNTO 0);			-- synchronize nSCUB_DS and generate pulse on neg edge
	SIGNAL		S_nSync_Timing_Cyc	: STD_LOGIC_VECTOR(1 DOWNTO 0);			-- synchronize nSCUB_Timing_Cyc and generate pulse on neg edge

	SIGNAL		S_DS_Val			: STD_LOGIC;

	SIGNAL		S_SCUB_Dtack		: STD_LOGIC;

	SIGNAL		S_Echo_Reg			: STD_LOGIC_VECTOR(15 DOWNTO 0);
	
	SIGNAL		S_Read_Out			: STD_LOGIC_VECTOR(15 DOWNTO 0);

	SIGNAL		S_Intr_In_Sync1		: STD_LOGIC_VECTOR(Intr_In'range);		--
	SIGNAL		S_Intr_In_Sync2		: STD_LOGIC_VECTOR(Intr_In'range);		--
	SIGNAL		S_Intr_Enable		: STD_LOGIC_VECTOR(Intr_In'range);		--
	SIGNAL		S_Intr_Mask			: STD_LOGIC_VECTOR(Intr_In'range);		--
	SIGNAL		S_Intr_Pending		: STD_LOGIC_VECTOR(Intr_In'range);		--
	SIGNAL		S_Wr_Intr_Pending	: STD_LOGIC_VECTOR(1 DOWNTO 0);
	SIGNAL		S_Intr_Active		: STD_LOGIC_VECTOR(Intr_In'range);		--
	SIGNAL		S_Wr_Intr_Active	: STD_LOGIC_VECTOR(1 DOWNTO 0);
	SIGNAL		S_Intr_Level_Neg	: STD_LOGIC_VECTOR(Intr_In'range);		--
	SIGNAL		S_Intr_Edge_Trig	: STD_LOGIC_VECTOR(Intr_In'range);		--
	SIGNAL		S_SRQ				: STD_LOGIC;
	
	SIGNAL		S_Standard_Reg_Acc	: STD_LOGIC;

	SIGNAL		S_Powerup_Res_Cnt	: unsigned(3 DOWNTO 0) := (OTHERS => '0');
	SIGNAL		S_nPowerup_Res		: STD_LOGIC;
	SIGNAL		S_Powerup_Done		: STD_LOGIC;
	
	CONSTANT	C_Dtack_dly_in_ns:		integer	:= 20;
	CONSTANT	C_Dtack_dly_cnt:		integer	:= (C_Dtack_dly_in_ns * 1000 / Clk_in_ps)-1;
	SIGNAL		S_Dtack_to_SCUB_Dly:	STD_LOGIC_VECTOR(C_Dtack_dly_cnt downto 0);
	SIGNAL		start_dt_dly:			STD_LOGIC;

	END SCU_Bus_Slave_V2R1;


ARCHITECTURE Arch_SCU_Bus_Slave_V2R1 OF SCU_Bus_Slave_V2R1 IS


BEGIN

ASSERT NOT (Clk_in_Hz < 100000000)
	REPORT "Achtung Generic Clk_in_Hz ist auf " & integer'image(Clk_in_Hz)
			& " gesetzt. Mit der Periodendauer von " & integer'image(Clk_in_ns)
			& " ns l��t sich kein schnelles Slaveinterface realisieren"
SEVERITY Warning;


P_Reset:	PROCESS	(clk, nReset, S_nPowerup_Res)
	BEGIN
		IF rising_edge(clk) THEN
			IF nReset = '0' OR S_nPowerup_Res = '0' THEN
				S_nReset <= '0';
			ELSE
				S_nReset <= '1';
			END IF;
		END IF;
	END PROCESS P_Reset;
	

P_Powerup:	PROCESS (clk)
	BEGIN
		IF rising_edge(clk) THEN
			IF S_Powerup_Res_Cnt <= 2**(S_Powerup_Res_Cnt'length) - 2 THEN
				S_Powerup_Res_Cnt <= S_Powerup_Res_Cnt + 1;
				S_nPowerup_Res <= '0';
			ELSE
				S_nPowerup_Res <= '1';
			END IF;
			IF S_Powerup_Res_Cnt = 2**(S_Powerup_Res_Cnt'length) - 2 THEN
				S_Powerup_Done <= '1';
			END IF;
			IF S_Wr_Intr_Active(0) = '1' AND S_Data_from_SCUB_LA(0) = '1' THEN
				S_Powerup_Done <= '0';
			END IF;
		END IF;
	END PROCESS P_Powerup;


P_Adr_LA:	PROCESS (clk, S_nReset)
	BEGIN
		IF S_nReset = '0' THEN
			S_ADR_from_SCUB_LA <= (OTHERS => '0');
			S_nSync_Board_Sel <= (OTHERS => '1');
		ELSIF rising_edge(clk) THEN
			S_nSync_Board_Sel <= (S_nSync_Board_Sel(S_nSync_Board_Sel'high-1 DOWNTO 0) & nSCUB_Slave_Sel);
			IF S_nSync_Board_Sel = "10" THEN
				S_ADR_from_SCUB_LA <= SCUB_Addr;
			END IF;
		END IF;
	END PROCESS P_Adr_LA;


P_Data_LA:	PROCESS (clk, S_nReset)
	BEGIN
		IF S_nReset = '0' THEN
			S_Data_from_SCUB_LA <= (OTHERS => '0');
			S_nSync_DS <= (OTHERS => '1');
		ELSIF rising_edge(clk) THEN
			S_nSync_DS <= (OTHERS => '1');
			IF S_nSync_Board_Sel = "00" THEN
				S_nSync_DS <= (S_nSync_DS(S_nSync_DS'high-1 DOWNTO 0) & nSCUB_DS);
				IF S_nSync_DS = "10" AND SCUB_RDnWR = '0' THEN
					S_Data_from_SCUB_LA <= SCUB_Data;
				END IF;
			END IF;
		END IF;
	END PROCESS P_Data_LA;


P_Timing_LA:	PROCESS (clk, S_nReset)
	BEGIN
		IF S_nReset = '0' THEN
			S_Timing_Pattern_LA <= (OTHERS => '0');
			S_nSync_Timing_Cyc <= (OTHERS => '1');
			S_Timing_Pat_RCV <= '0';
			S_Timing_Pat_RCV_Dly <= '0';
		ELSIF rising_edge(clk) THEN
			S_Timing_Pat_RCV <= '0';
			S_Timing_Pat_RCV_Dly <= S_Timing_Pat_RCV;
			S_nSync_Timing_Cyc <= (S_nSync_Timing_Cyc(S_nSync_Timing_Cyc'high-1 DOWNTO 0) & nSCUB_Timing_Cyc);
			IF S_nSync_Timing_Cyc = "10" AND SCUB_RDnWR = '0' THEN
				S_Timing_Pattern_LA <= (SCUB_Addr & SCUB_Data);
				S_Timing_Pat_RCV <= '1';
			END IF;
		END IF;
	END PROCESS P_Timing_LA;


P_Intr:	PROCESS (clk, S_nReset, S_Powerup_Done)
	BEGIN
		IF S_nReset = '0' THEN
			S_Intr_In_Sync1 <= (OTHERS => '0');
			S_Intr_In_Sync2 <= (OTHERS => '0');
			S_Intr_Pending <= (OTHERS => '0');
			FOR i IN Intr_In'low TO Intr_In'high LOOP 
				S_Intr_Active(i) <= '0';
				END LOOP;

		ELSIF rising_edge(clk) THEN
			FOR i IN Intr_In'low TO Intr_In'high LOOP
				S_Intr_In_Sync1(i) <= Intr_In(i) XOR S_Intr_Level_Neg(i);				-- convert 'Intr_In' to positive level and first sync.
				S_Intr_In_Sync2(i) <= S_Intr_In_Sync1(i);								-- second sync
				IF S_Intr_Enable(i) = '1' THEN
					IF S_Intr_Edge_Trig(i) = '1' THEN
						IF S_Intr_In_Sync1(i) = '1' AND S_Intr_In_Sync2(i) = '0' THEN		-- positive edge detect (1 clock pulse)
							IF S_Intr_Mask(i) = '1' THEN
								S_Intr_Pending(i) <= '1';
							ELSIF S_Intr_Active(i) = '1' THEN
								S_Intr_Pending(i) <= '1';
							ELSE
								S_Intr_Active(i) <= '1';
							END IF;
						ELSIF S_Wr_Intr_Active = "01" AND S_Data_from_SCUB_LA(i) = '1' THEN
							S_Intr_Active(i) <= '0';
						ELSIF S_Wr_Intr_Pending = "01" AND S_Data_from_SCUB_LA(i) = '1' THEN
							S_Intr_Pending(i) <= '0';
						ELSIF S_Intr_Pending(i) = '1' AND S_Intr_Active(i) = '0' AND S_Intr_Mask(i) = '0' THEN
							S_Intr_Pending(i) <= '0';
							S_Intr_Active(i) <= '1';
						END IF;
					ELSE
						S_Intr_Pending(i) <= S_Intr_In_Sync2(i);							-- follows synchronized Intr_IN(i) level
						IF S_Intr_Pending(i) = '1' AND S_Intr_Mask(i) = '0' THEN
							S_Intr_Active(i) <= '1';
						ELSE
							S_Intr_Active(i) <= '0';
						END IF;
					END IF;
				ELSE
					S_Intr_Pending(i) <= '0';
					S_Intr_Active(i) <= '0';
				END IF;
			END LOOP;
			IF unsigned(S_Intr_Active) /= 0 OR S_Powerup_Done = '1' THEN
				S_SRQ <= '1';
			ELSE
				S_SRQ <= '0';
			END IF;
		END IF;
	END PROCESS P_Intr;


P_Tri_Buff:	PROCESS (S_nReset, SCUB_RDnWR, nSCUB_DS, nSCUB_Slave_Sel, S_Read_Out)
	BEGIN
		IF S_nReset = '0' THEN
			SCUB_Data <= (OTHERS => 'Z');
			nSel_Ext_Data_Drv <= '1';
		ELSIF nSCUB_Slave_Sel = '0' THEN	-- setzt voraus, dass der SCU_BusMaster w�hrend eines Timing-Cycles die nSCUB_Slave_Sel bedient.
			nSel_Ext_Data_Drv <= '0';
			IF SCUB_RDnWR = '1' AND nSCUB_DS = '0' THEN
				SCUB_Data <= S_Read_Out;
			ELSE
				SCUB_Data <= (OTHERS => 'Z');
			END IF;
		ELSE
			nSel_Ext_Data_Drv <= '1';
			SCUB_Data <= (OTHERS => 'Z');
		END IF;
	END PROCESS P_Tri_Buff;
	

P_no_fin_sig_overap: PROCESS (clk)
	BEGIN
		IF rising_edge(clk) THEN
			IF (S_Adr_Val = '1' AND SCUB_RDnWR = '1' AND S_nSync_Board_Sel = "01") THEN
				Ext_Rd_fin <= '1';
			ELSE
				Ext_Rd_fin <= '0';
			END IF;
			IF (S_DS_Val = '1' AND SCUB_RDnWR = '0' AND S_nSync_Board_Sel = "01") THEN
				Ext_Wr_fin <= '1';
			ELSE
				Ext_Wr_fin <= '0';
			END IF;
		END IF;
	END PROCESS P_no_fin_sig_overap;
	

Ext_Rd_fin_ovl	<= '1' WHEN (S_Adr_Val = '1' AND SCUB_RDnWR = '1' AND S_nSync_Board_Sel = "01") ELSE '0';
Ext_Wr_fin_ovl	<= '1' WHEN (S_DS_Val = '1' AND SCUB_RDnWR = '0' AND S_nSync_Board_Sel = "01") ELSE '0';


P_Standard_Reg: PROCESS (clk, S_nReset)
	BEGIN
		IF S_nReset = '0' THEN
			S_Echo_Reg <= (OTHERS => '0');
			S_Intr_Mask <= (OTHERS => '0');
			S_Intr_Level_Neg <= Intr_Level_Neg(Intr_In'range);
			S_Intr_Edge_Trig <= Intr_Edge_Trig(Intr_In'range);
			S_Intr_Enable <= Intr_Enable;
			S_Adr_Val <= '0';
			S_DS_Val <= '0';
			S_SCUB_Dtack <= '0';
			S_Standard_Reg_Acc <= '0';
			S_Read_Out <= (OTHERS => 'Z');			-- Vers_2 Revi_0: Vorschlag S. Sch�fer, vermeidet un�tige Daten�berg�nge auf dem SCU-Datenbus 
		ELSIF rising_edge(clk) THEN
			S_Adr_Val <= '0';
			S_DS_Val <= '0';
			S_SCUB_Dtack <= '0';
			S_Wr_Intr_Pending <= "00";
			S_Wr_Intr_Active <= "00";
			S_Standard_Reg_Acc <= '0';
			S_Dtack_to_SCUB_Dly <= (OTHERS => '0');
			S_Read_Out <= (OTHERS => 'Z');			-- Vers_2 Revi_0: Vorschlag S. Sch�fer, vermeidet un�tige Daten�berg�nge auf dem SCU-Datenbus 
			IF S_nSync_Board_Sel = "00" AND S_nSync_Timing_Cyc = "11" THEN
				CASE S_ADR_from_SCUB_LA IS
					WHEN C_Slave_ID_Adr =>
						S_Standard_Reg_Acc <= '1';
						IF SCUB_RDnWR = '1' THEN
							S_Read_Out <= std_logic_vector(to_unsigned(Slave_ID, S_Read_Out'length));
							S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
						END IF;
					WHEN C_FW_Version_Adr =>
						S_Standard_Reg_Acc <= '1';
						IF SCUB_RDnWR = '1' THEN
							S_Read_Out <= std_logic_vector(to_unsigned(Firmware_Version, S_Read_Out'length));
							S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
						END IF;
					WHEN C_FW_Release_Adr =>
						S_Standard_Reg_Acc <= '1';
						IF SCUB_RDnWR = '1' THEN
							S_Read_Out <= std_logic_vector(to_unsigned(Firmware_Release, S_Read_Out'length));
							S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
						END IF;
					WHEN C_HW_Version_Adr =>
						S_Standard_Reg_Acc <= '1';
						IF SCUB_RDnWR = '1' THEN
							S_Read_Out <= std_logic_vector(to_unsigned(Hardware_Version, S_Read_Out'length));
							S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
						END IF;
					WHEN C_HW_Release_Adr =>
						S_Standard_Reg_Acc <= '1';
						IF SCUB_RDnWR = '1' THEN
							S_Read_Out <= std_logic_vector(to_unsigned(Hardware_Release, S_Read_Out'length));
							S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
						END IF;
					WHEN C_Echo_Reg_Adr =>
						S_Standard_Reg_Acc <= '1';
						IF SCUB_RDnWR = '1' THEN
							S_Read_Out <= S_Echo_Reg;
							S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
						ELSIF S_nSync_DS = "00" THEN
							S_Echo_Reg <= S_Data_from_SCUB_LA;
							S_SCUB_Dtack <= '1';
						ELSE
							S_SCUB_Dtack <= '0';
						END IF;
					WHEN C_Status_Reg_Adr =>
						S_Standard_Reg_Acc <= '1';
						IF SCUB_RDnWR = '1' THEN
							S_Read_Out <= (X"000" & '0' & '0' & User_Ready & S_Powerup_Done);	-- User_Ready must be synchron with clk
							S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
						END IF;
					WHEN C_Intr_In_Adr =>
						S_Standard_Reg_Acc <= '1';
						IF SCUB_RDnWR = '1' THEN
							S_Read_Out <= (S_Intr_In_Sync2 & '0');
							S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
						END IF;
					WHEN C_Intr_Ena_Adr =>
						S_Standard_Reg_Acc <= '1';
						IF SCUB_RDnWR = '1' THEN
							S_Read_Out <= (S_Intr_Enable & '1');
							S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
						ELSIF S_nSync_DS = "00" THEN
							S_Intr_Enable <= S_Data_from_SCUB_LA(Intr_In'range);
							S_SCUB_Dtack <= '1';
						END IF;
					WHEN C_Intr_Pending_Adr =>
						S_Standard_Reg_Acc <= '1';
						IF SCUB_RDnWR = '1' THEN
							S_Read_Out <= (S_Intr_Pending & '0');	-- register latchen ???
							S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
						ELSIF S_nSync_DS = "00" THEN
							S_Wr_Intr_Pending <= S_Wr_Intr_Pending(0) & '1';
							S_SCUB_Dtack <= '1';	-- ???
						END IF;
					WHEN C_Intr_Mask_Adr =>
						S_Standard_Reg_Acc <= '1';
						IF SCUB_RDnWR = '1' THEN
							S_Read_Out <= (S_Intr_Mask & '0');
							S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
						ELSIF S_nSync_DS = "00" THEN
							S_Intr_Mask <= S_Data_from_SCUB_LA(Intr_In'range);
							S_SCUB_Dtack <= '1';
						END IF;
					WHEN C_Intr_Active_Adr =>
						S_Standard_Reg_Acc <= '1';
						IF SCUB_RDnWR = '1' THEN
							S_Read_Out <= S_Intr_Active & S_Powerup_Done;	-- register latchen ???
							S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
						ELSIF S_nSync_DS = "00" THEN
							S_Wr_Intr_Active <= S_Wr_Intr_Active(0) & '1';
							S_SCUB_Dtack <= '1';	-- ??
						END IF;
					WHEN C_Intr_Level_Adr =>
						S_Standard_Reg_Acc <= '1';
						IF SCUB_RDnWR = '1' THEN
							S_Read_Out <= (S_Intr_Level_Neg & '0');
							S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
						ELSIF S_nSync_DS = "00" THEN
							S_Intr_Level_Neg <= S_Data_from_SCUB_LA(Intr_In'range);
							S_SCUB_Dtack <= '1';
						END IF;
					WHEN C_Intr_Edge_Adr =>
						S_Standard_Reg_Acc <= '1';
						IF SCUB_RDnWR = '1' THEN
							S_Read_Out <= (S_Intr_Edge_Trig & '0');
							S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
						ELSIF S_nSync_DS = "00" THEN
							S_Intr_Edge_Trig <= S_Data_from_SCUB_LA(Intr_In'range);
							S_SCUB_Dtack <= '1';
						END IF;
					WHEN C_Vers_Revi_of_this_Macro =>
						S_Standard_Reg_Acc <= '1';
						IF SCUB_RDnWR = '1' THEN
							S_Read_Out <= (C_Version & C_Revision);
							S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
						END IF;
					WHEN   C_Free_Intern_A_12
						 | C_Free_Intern_A_13
						 | C_Free_Intern_A_14
						 | C_Free_Intern_A_15
						 | C_Free_Intern_A_16
						 | C_Free_Intern_A_17
						 | C_Free_Intern_A_18
						 | C_Free_Intern_A_19
						 | C_Free_Intern_A_1A
						 | C_Free_Intern_A_1B
						 | C_Free_Intern_A_1C
						 | C_Free_Intern_A_1D
						 | C_Free_Intern_A_1E
						 | C_Free_Intern_A_1F  =>
						S_Standard_Reg_Acc <= '1';
					WHEN OTHERS =>								-- der Zugriff soll au�erhalb dieses Makros erfolgen (externe User-Register)
						S_Adr_Val <= '1';

						S_Dtack_to_SCUB_Dly <= S_Dtack_to_SCUB_Dly(S_Dtack_to_SCUB_Dly'high-1 downto 0) & Dtack_to_SCUB;	-- Vers_2 Revi_1

						IF Dtack_to_SCUB = '1' and start_dt_dly = '0' THEN													-- Vers_2 Revi_1
							start_dt_dly <= '1';																			-- Vers_2 Revi_1
						ELSIF S_Dtack_to_SCUB_Dly(S_Dtack_to_SCUB_Dly'high) = '1' THEN										-- Vers_2 Revi_1
							start_dt_dly <= '0';																			-- Vers_2 Revi_1
						END IF;																								-- Vers_2 Revi_1
						
						IF SCUB_RDnWR = '1' and (Dtack_to_SCUB = '1' or start_dt_dly = '1') THEN	-- Vers_2 Revi_1: Vorschlag S. Sch�fer, nur wenn Resource existiert	--
							S_Read_Out <= Data_to_SCUB;												-- wird das Lesedatum auf den SCU_Bus gelegt.						--
							S_SCUB_Dtack <= S_Dtack_to_SCUB_Dly(S_Dtack_to_SCUB_Dly'high);			-- Vers_2 Revi_1
						ELSIF SCUB_RDnWR = '0' and S_nSync_DS = "00" THEN							-- Vers_2 Revi_1
							S_DS_Val <= '1';
							S_SCUB_Dtack <= S_Dtack_to_SCUB_Dly(S_Dtack_to_SCUB_Dly'high);			-- Vers_2 Revi_1
						END IF;
				END CASE;
			END IF;

		END IF;
	END PROCESS P_Standard_Reg;
	

nSCUB_Dtack_Opdrn	<= '0' WHEN (S_SCUB_Dtack = '1' AND nSCUB_Slave_Sel = '0') ELSE 'Z';
SCUB_Dtack			<= '1' WHEN (S_SCUB_Dtack = '1' AND nSCUB_Slave_Sel = '0') ELSE '0';

ADR_from_SCUB_LA <= S_ADR_from_SCUB_LA;

Data_from_SCUB_LA <= S_Data_from_SCUB_LA;

Timing_Pattern_LA <= S_Timing_Pattern_LA;
Timing_Pattern_RCV <= S_Timing_Pat_RCV_Dly;

nSCUB_SRQ_Opdrn	<= '0' WHEN (S_SRQ = '1') ELSE 'Z';
SCUB_SRQ		<= '1' WHEN (S_SRQ = '1') ELSE '0';

Ext_Adr_Val <= S_Adr_Val;

Ext_Rd_active	<= '1' WHEN (S_Adr_Val = '1' AND SCUB_RDnWR = '1') ELSE '0';

Ext_Wr_active	<= '1' WHEN (S_DS_Val = '1' AND SCUB_RDnWR = '0') ELSE '0';

nPowerup_Res <= S_nPowerup_Res;

Ext_Data_Drv_Rd <= SCUB_RDnWR;

Standard_Reg_Acc <= S_Standard_Reg_Acc;

END Arch_SCU_Bus_Slave_V2R1;
