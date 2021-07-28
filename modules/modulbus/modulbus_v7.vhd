--TITLE "'ModulBus_V7' => Modul-Bus-Macro mit 16Bit-Anwender-I/O, Autor: W.Panschow, Stand: 04.04.08, Vers: V07 ";

-- Version 7: Das Echo-Register ist jetzt Bestandteil dieses Macros.

--+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
--+ Beschreibung:																											+
--+																															+
--+   Das Macro 'Modul_Bus_Macro' realisiert die Schnittstelle zum Modul-Bus, es hat zwei Betriebsarten.					+
--+																															+
--+     1)	Der Normalbetrieb:																								+
--+				Hierbei muessen  a l l e  fest verdrahtete Signalgruppen (VG-Leiste)											+
--+				mit den Modul-Bus-Signalen und Kartenkennungen uebereinstimmen.												+
--+				Wenn:																										+
--+																															+
--+					I)	VG_Mod_ID[7..0] == MOD_ID[7..0] ist, steckt richtige Karte auf dem richtigen Busplatz.				+
--+								Wobei VG_Mod_ID[] den Kartentyp vorgibt, der an diesem Steckplatz erwartet wird, und		+
--+								MOD_ID[] den Typ der bestueckten Karte beschreibt. Ueber den Parameter 'MOD_ID' wird			+
--+								MOD_ID[] festgelegt, dezimal 37 entspricht z.B. dem Kartentyp Event-Sequencer.				+
--+																															+
--+					II)	VG_Mod_Adr[4..0] == Mod_Adr[4..0] ist, wird der entsprechende Busplatz adressiert.					+
--+								Wobei VG_Mod_Adr[] die Adresse des Steckplatzes festlegt und Mod_Adr[] die aktuell			+
--+								angesprochene Modul_Bus_Adresse darstellt.													+
--+																															+
--+				Ausserdem sollte die Sub_Adresse des Modul-Busses 'Sub_Adr[7..0]' eine gueltige Funktion beim jeweiligen		+
--+				Kartentyp ausloesen. Nur dann sollte von der Adressdekodierung ein 'DT_Adr_Deco' kommen und der Daten-Bus-	+
--+				Treiber zum Modul-Bus aktiviert werden.																		+
--+																															+
--+     2) Der Diagnosebetrieb:																								+
--+				Hier steckt  n i c h t  die richtige Karte auf dem richtigen Busplatz ( VG_Mod_ID[] <> MOD_ID[] ).			+
--+				Um solch eine Fehlkonfiguration mit Software erkennen zu koennen gibt es sechs standardisierte Subadressen.	+
--+				Stimmt die am Steckplatz fest verdrahtete VG_Mod_Adr[4..0] mit der auf den Modul-Bus angelegten				+
--+				Mod_Adr[4..0] ueberein, liefert der Macro zumindest bei diesen sechs Subadressen:							+
--+        			1) Die am Steckplatz verdrahtetete VG_Mod_ID[].															+
--+					2) Die MOD_ID[] des bestueckten Kartentyps.																+
--+						Siehe auch im Konstanten-Definitionsteil nach 'C_Rd_ID'.       										+
--+					3) Das VG_Skalierungsbyte.																				+
--+					4) Die VG_Mod_Adresse.																					+
--+						Siehe auch im Konstanten-Definitionsteil nach 'C_Rd_Skal_Adr'. 										+
--+					5) Die EPLD_Vers[7..0].																					+
--+					6) Das Status_Reg[7..0]:	Bit0 = 'Power_Up_Reset'														+
--+												Bit1 = 'Timeout', das LB des Modulbustransfers ist nicht rechtzeitig vom	+
--+														Modulbus-Kontroller geliefert worden.								+
--+												Bit2..7 = Stat_IN[2..7]														+
--+						Lesen: liefert das Status_Reg[], Schreiben: Eine '1' an die Bits[1..0] geschrieben, loescht die 	+
--+						Bits[1..0] im Status_Reg[].																			+
--+					7)* Lese 'ST_160_Skal[7..0]'																			+
--+					8)* Lese 'Macro_Activ'([7]), 'Macro_Skal_OK'([6]) und 'ST_160_Auxi[5..0]'								+
--+				Der Macro gibt nur fuer diese 6 (8)* Lese-Subadressen das Dtack 'DT_Mod_Bus'. Alle anderen Subadressen 		+
--+				belassen den 'Modul_Bus_Macro' im passiven Zustand.															+
--+	*) Nur wenn 'St_160pol' gleich '1' ist.																					+
--+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

library IEEE;
USE IEEE.STD_LOGIC_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;
USE IEEE.MATH_REAL.ALL;
LIBRARY lpm;
USE lpm.lpm_components.all;

--library work;
--USE PW_VHDL_LIB.all;

ENTITY modulbus_v7 IS
	GENERIC(
			St_160_pol	: INTEGER := 0;			-- 	0 ==> VG96-Modulbus,	1 ==> 160 poliger Modulbus (5-Reihig)				--
			Mod_Id		: INTEGER := 16#55#;
			CLK_in_Hz	: INTEGER := 50000000;	-- Damit das Design schnell auf eine andere Frequenz umgestellt werden kann, wird diese		--
												-- in Hz festgelegt. Zaehler die betimmte Zeiten realisieren sollen z.B. 'time_out_cnt'		--
												-- werden entprechend berechnet.
			Loader_Base_Adr	: INTEGER := 240;
			Res_Deb_in_ns	: INTEGER := 100;
			nDS_Deb_in_ns	: INTEGER := 20;
			Use_LPM		: INTEGER := 0;
			Test		: INTEGER := 0
			);
	PORT(
		Epld_Vers		: IN	STD_LOGIC_VECTOR(7 DOWNTO 0) := "00000000";
		VG_Mod_Id		: IN	STD_LOGIC_VECTOR(7 DOWNTO 0);	-- Der an diesem Modul-Bus-Steckplatz erwartete Karten-Typ (an der VG-Leiste fest verdrahtet).	--
		VG_Mod_Adr		: IN	STD_LOGIC_VECTOR(4 DOWNTO 0);	-- Adresse des Modul-Bus-Steckplatzes (an der VG-Leiste fest verdrahtet).						--
		VG_Mod_Skal		: IN	STD_LOGIC_VECTOR(7 DOWNTO 0);	-- Modul-Bus-Skalierung, fuer jeden Kartentyp unterschiedl. Bedeutung (an VG-Leiste verdrahtet).	--
		St_160_Skal		: IN	STD_LOGIC_VECTOR(7 DOWNTO 0);
		St_160_Auxi		: IN	STD_LOGIC_VECTOR(5 DOWNTO 0);
		Stat_IN			: IN	STD_LOGIC_VECTOR(7 DOWNTO 2);
		Macro_Activ		: IN	STD_LOGIC := '1';
		Macro_Skal_OK	: IN	STD_LOGIC := '1';
		Mod_Adr			: IN	STD_LOGIC_VECTOR(4 DOWNTO 0);	-- Adresse des gerade laufenden Modul-Bus-Zyklusses.											--	
		Sub_Adr			: IN	STD_LOGIC_VECTOR(7 DOWNTO 0);	-- Sub-Adresse des gerade laufenden Modul-Bus-Zyklusses.										--	
		RDnWR			: IN	STD_LOGIC;						-- Lese/Schreibsignal des Modul-Busses. RD/WR = 1 => Lesen.										--
		nDS				: IN	STD_LOGIC;						-- Datenstrobe des Modul-Busses. /DS = 0 => aktiv.												--
		CLK				: IN	STD_LOGIC;						-- Systemtakt des restlichen Designs sollte >= 12 Mhz sein.										--
		nMB_Reset		: IN	STD_LOGIC;
		V_Data_Rd		: IN	STD_LOGIC_VECTOR(15 DOWNTO 0);	-- Data to Modulbus, alle Daten-Quellen die ausserhalb dieses Macros liegen sollten hier ueber	--
																-- Multiplexer angeschlossen werden.															--
		nExt_Data_En	: OUT	STD_LOGIC;						-- Signal = 0, schaltet externen Datentreiber des Modul-Busses ein.

		Mod_Data		: INOUT	STD_LOGIC_VECTOR(7 DOWNTO 0);	-- Daten-Bus des Modul-Busses.																	--

		nDt_Mod_Bus		: OUT	STD_LOGIC;						-- Data-Acknowlege zum Modul-Bus.																--
		Sub_Adr_La		: OUT	STD_LOGIC_VECTOR(7 DOWNTO 1);	
		Data_Wr_La		: OUT	STD_LOGIC_VECTOR(15 DOWNTO 0);	
		Extern_Wr_Activ	: OUT	STD_LOGIC;			
		Extern_Wr_Fin	: OUT	STD_LOGIC;		
		Extern_Rd_Activ	: OUT	STD_LOGIC;	
		Extern_Rd_Fin	: OUT	STD_LOGIC;
		Extern_Dtack	: IN	STD_LOGIC;						-- Alle extern dekodierten Modul-Bus-Aktionen, muessen hier ihr Dtack anlegen.					--
		Powerup_Res		: OUT	STD_LOGIC := '0';
		nInterlock		: OUT	STD_LOGIC;
		Timeout			: OUT	STD_LOGIC;
		Id_OK			: OUT	STD_LOGIC;
		nID_OK_Led		: OUT	STD_LOGIC;
		Led_Ena			: OUT	STD_LOGIC;
		nPower_Up_Led	: OUT	STD_LOGIC;
		nSel_Led		: OUT	STD_LOGIC;
		nDt_Led			: OUT	STD_LOGIC
		);
		
	CONSTANT	Clk_in_ps			: INTEGER	:= 1000000000 / (Clk_in_Hz / 1000);
	CONSTANT	Clk_in_ns			: INTEGER	:= 1000000000 / Clk_in_Hz;
	
	CONSTANT	C_Timeout_in_ns		: INTEGER	:= 2800;  						--  2,8 us	--
	CONSTANT	C_Timeout_cnt		: INTEGER	:= C_timeout_in_ns * 1000 / Clk_in_ps;
	
	CONSTANT	C_Led_Time_in_ns	: INTEGER	:= 25000000;					-- 25 ms	--
	CONSTANT	C_Led_Ena_Cnt		: INTEGER	:= C_Led_Time_in_ns / Clk_in_ns;
	CONSTANT	C_Led_Ena_Tst_Cnt	: INTEGER	:= 3;



	----------------------------------------------------------------------------------------------
	-- Standardisierte Subadresse zum Lesen der IDs.                                            --
	-- Achtung nur Bit[7..1] definiert. Bit0 dient zur HB-LB-Kennung.                           --
	-- Bit0 = 0 => HB = Subadr. FE => lese 'MOD_ID[7..0]'    = ID der Karte,                    --
	-- Bit0 = 1 => LB = Subadr. FF => lese 'VG_MOD_ID[7..0]' = ID der VG-Leiste.                --
	----------------------------------------------------------------------------------------------
	CONSTANT 	C_Rd_ID 					: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"FE";

	----------------------------------------------------------------------------------------------
	-- Standardisierte Subadresse zum Lesen der Skalierung und der Moduladresse.                --
	-- Achtung nur Bit[7..1] definiert. Bit0 dient zur HB-LB-Kennung.                           --
	-- Bit0 = 0 => HB = Subadr. FC => lese 'VG_Mod_Skal[7..0]' = Skalierung an der VG-Leiste    --
	-- Bit0 = 1 => LB = Subadr. FD => lese 'VG_Mod_Adr[4..0]'  = Moduladresse an der VG-Leiste. --
	----------------------------------------------------------------------------------------------
	CONSTANT 	C_Rd_Skal_Adr				: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"FC";

	----------------------------------------------------------------------------------------------
	-- Standardisierte Subadresse zum Lesen der EPLD-Version und Lesen Ruecksetzen				--
	-- des 'Status-Reg[7..0]'.																	--
	-- Achtung nur Bit[7..1] definiert. Bit0 dient zur HB-LB-Kennung.               			--
	-- Bit0 = 0 => HB = Subadr. FA => lese 'Epld_Vers[7..0]'									--
	-- Bit0 = 1 => LB = Subadr. FB => lesen/ruecksetzen des 'Status-Reg[7..0]'.					--
	----------------------------------------------------------------------------------------------
	CONSTANT 	C_Rd_EPLD_Vers_Rd_Wr_Stat	: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"FA";

	----------------------------------------------------------------------------------------------
	-- Standardisierte Subadresse bei selektiertem 160pol. Stecker (Paramerter 'ST_160pol = 1)	--
	-- wird die 2. Skalierung 'ST_160_Skal[]' und von 'ST_160_Auxi[]' gelesen.					--
	-- Achtung nur Bit[7..1] definiert. Bit0 dient zur HB-LB-Kennung.                           --
	-- Bit0 = 0 => HB = Subadr. F8 => lese 'ST_160_Skal[7..0]' = 2tes Skalierungsbyte 		    --
	-- Bit0 = 1 => LB = Subadr. F9 => lese 'ST_160_Auxi[5..0]' = z.B. Logiauswahlschalter bei	--
	-- der 'MB64-APK'.																			--
	----------------------------------------------------------------------------------------------
	CONSTANT 	C_Rd_Skal2_Adr				: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"F8";

	----------------------------------------------------------------------------------------------
	-- Standardisierte Subadresse des Echoregisters												--
	-- Achtung nur Bit[7..1] definiert. Bit0 dient zur HB-LB-Kennung.                           --
	-- Bit0 = 0 => HB = Subadr. F6 => lese/schreibe das HB des Echoregisters		 		    --
	-- Bit0 = 1 => LB = Subadr. F6 => lese/schreibe das HB des Echoregisters		 		    --
	----------------------------------------------------------------------------------------------
	CONSTANT 	C_Rd_Wr_Echo_Reg			: STD_LOGIC_VECTOR(7 DOWNTO 0) := X"F6";
	
	------------------------------------------------------------------------------------------------------
	-- Die Adressen Loader-Base-Adr bis Loader-Base-Adr+3 sollen genau wie die interen Zugriffe unab-	--
	-- haengig vom ID des Steckplatzes funktionieren. Es sollen aber die exteren Strobes erzeugt werden,	--
	-- da der Loader-Macro nicht im Modulbus-Macro integriert werden soll.								--
	------------------------------------------------------------------------------------------------------
	CONSTANT 	C_Loader_Base_Adr	: STD_LOGIC_VECTOR(7 DOWNTO 0) := conv_std_logic_vector(Loader_Base_Adr, 8);
	
	

    FUNCTION	set_cnt_ge_1  (production_cnt : INTEGER) RETURN INTEGER IS

		VARIABLE cnt : INTEGER;
		
		BEGIN
			IF production_cnt > 1 THEN
				cnt := production_cnt;
			ELSE
				cnt := 1;
			END IF;
			RETURN cnt;
		END set_cnt_ge_1;


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
		
	CONSTANT	C_Res_Deb_cnt		: INTEGER	:= set_cnt_ge_1(Res_Deb_in_ns * 1000 / Clk_in_ps);
	
	CONSTANT	C_nDS_Deb_cnt		: INTEGER	:= set_cnt_ge_1(nDS_Deb_in_ns * 1000 / Clk_in_ps);
	
	END modulbus_v7;


ARCHITECTURE Arch_modulbus_v7 OF modulbus_v7 IS

	TYPE	T_State_Mod_SM	IS	(
								Idle,
								Rd_HB,		
								Wait_Rd_LB,	
								Rd_LB,
								Rd_Fin,
								Wr_HB,
								Wait_Wr_LB,
								Wr_LB,
								WR_Fin);
									
	SIGNAL	State_Mod_SM	: T_State_Mod_SM;

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
		

	CONSTANT	C_Ld_Led_cnt	: INTEGER :=  prod_or_test(C_Led_Ena_Cnt, C_Led_Ena_Tst_Cnt, Test);
	


	SIGNAL	S_MB_Macro_Rd_Mux	: STD_LOGIC_VECTOR(7 DOWNTO 0);
	SIGNAL	S_Timeout_cnt		: STD_LOGIC_VECTOR((How_many_Bits(C_Timeout_cnt)) DOWNTO 0);
	SIGNAL 	S_Sel_To_Cnt		: STD_LOGIC;
	SIGNAL	S_Set_TO_Cnt		: STD_LOGIC;
	SIGNAL	S_Timeout			: STD_LOGIC;
	SIGNAL	S_Adr_OK			: STD_LOGIC_VECTOR(1 DOWNTO 0);	-- Als Vektor damit Flankenerkennung moeglich ist.
	SIGNAL	S_ID_OK				: STD_LOGIC;
	SIGNAL	S_DS_Sync			: STD_LOGIC;
	SIGNAL	S_DS				: STD_LOGIC;
	SIGNAL	S_DT_Intern			: STD_LOGIC;
	SIGNAL	S_Status_Reg		: STD_LOGIC_VECTOR(1 DOWNTO 0);
	SIGNAL	S_Wr_Status_Reg		: STD_LOGIC;
	SIGNAL	S_Wr_Status_Reg_r	: STD_LOGIC;
	SIGNAL	S_Powerup_Res_Cnt	: STD_LOGIC_VECTOR(2 DOWNTO 0) := "000";
	SIGNAL	S_Powerup_Res		: STD_LOGIC;
	SIGNAL	S_Sub_Adr_La		: STD_LOGIC_VECTOR(7 DOWNTO 1);
	SIGNAL	S_Data_Wr_La		: STD_LOGIC_VECTOR(15 DOWNTO 0);
	
	SIGNAL	S_DT_Delay			: STD_LOGIC_VECTOR(1 DOWNTO 0);
	
	SIGNAL	S_Led_Ena			: STD_LOGIC;
	
	CONSTANT	C_Led_Time_Width	: INTEGER	:= INTEGER(ceil(log2(real(C_Led_Ena_Cnt))));
	SIGNAL		S_Led_cnt           : STD_LOGIC_VECTOR(C_Led_Time_Width DOWNTO 0);

	SIGNAL	S_Extern_Access		: STD_LOGIC;
	SIGNAL	S_Intern_Access		: STD_LOGIC;
	
	SIGNAL	S_Extern_Wr_Activ	: STD_LOGIC_VECTOR(1 DOWNTO 0);
	SIGNAL	S_Extern_Wr_Fin		: STD_LOGIC;

	SIGNAL	S_Extern_Rd_Activ	: STD_LOGIC;
	SIGNAL	S_Extern_Rd_Fin		: STD_LOGIC;

	SIGNAL	S_Start_DT_Led		: STD_LOGIC;
	
	SIGNAL	S_SM_Reset			: STD_LOGIC;
	
	SIGNAL	S_Adr_Comp_Live		: STD_LOGIC;	-- Das Live Signal des Adressvergleichers
	SIGNAL	S_Adr_Comp_DB		: STD_LOGIC;	-- Das Entprellte Signal des Adressvergleichers

	SIGNAL	S_MB_Reset			: STD_LOGIC;
	SIGNAL	MB_Reset			: STD_LOGIC;	-- um (... <= not nMB_Reset) bei der Instanzierung zu vermeiden

	SIGNAL	S_Echo_Reg			: STD_LOGIC_VECTOR(15 DOWNTO 0);
	SIGNAL	S_Wr_Echo_Reg		: STD_LOGIC;

COMPONENT	Led
	GENERIC	(
			Use_LPM : INTEGER := 1
			);
	PORT(
			clk		: IN	STD_LOGIC;
			ena		: IN	STD_LOGIC;
			Sig_In	: IN	STD_LOGIC;
			nled	: OUT	STD_LOGIC
		);
END COMPONENT;


COMPONENT	Debounce
	GENERIC(
			DB_Cnt		: INTEGER := 3;	
			DB_Tst_Cnt	: INTEGER := 3;
			Use_LPM		: INTEGER := 0;
			Test		: INTEGER := 0
			);
	PORT(
		DB_In		: IN	STD_LOGIC;
		Reset		: IN	STD_LOGIC;
		Clk			: IN	STD_LOGIC;
		DB_Out		: OUT	STD_LOGIC
		);
END COMPONENT;
	

BEGIN

ASSERT (False)
	REPORT "C_Led_Time_in_ns = " & integer'image(C_Led_Time_in_ns) & ",     C_Led_Time_Width = " & integer'image(C_Led_Time_Width+1)
SEVERITY NOTE;

ASSERT (False)
	REPORT "C_nDS_Deb_cnt = " & integer'image(C_nDS_Deb_cnt) & ",     C_Res_Deb_cnt = " & integer'image(C_Res_Deb_cnt)
SEVERITY NOTE;


P_SM_Reset:	PROCESS (clk) 
	BEGIN
		IF clk'EVENT AND clk = '1' THEN
			IF S_Timeout = '1' OR S_Powerup_Res = '1' THEN
				S_SM_Reset <= '1';
			ELSE
				S_SM_Reset <= '0';
			END IF;
		END IF;
	END PROCESS P_SM_Reset;

Mod_SM:	PROCESS (clk, S_SM_Reset)
	
	BEGIN
	    IF  S_SM_Reset = '1' THEN
			State_Mod_SM <= Idle; 
	    ELSIF clk'EVENT AND clk = '1' THEN

			S_Extern_Wr_Fin <= '0';
			S_Extern_Rd_Fin	<= '0';

		 	CASE State_Mod_SM IS

			   	WHEN Idle => 
					S_Extern_Access <= '0';
					S_Intern_Access <= '0';
					S_Extern_Wr_Activ	<= (OTHERS => '0');
					S_Extern_Rd_Activ	<= '0';
					IF S_Adr_Comp_DB = '1' AND S_DS_Sync = '1' AND Sub_Adr(0) = '0' THEN
						S_Sub_Adr_La <= Sub_Adr(7 DOWNTO 1);
						IF Sub_Adr(7 DOWNTO 1)  >= C_Rd_Wr_Echo_Reg(7 DOWNTO 1) THEN						-- V7, vorher C_Rd_Skal2_Adr(7 DOWNTO 1)
							S_Intern_Access <= '1';
							S_Extern_Access <= '0';
						ELSE
							S_Intern_Access <= '0';
							IF S_ID_OK = '1' OR Sub_Adr(7 DOWNTO 2) = C_Loader_Base_Adr(7 DOWNTO 2) THEN	-- Die 2 Loader-Sub-Adr.--
								S_Extern_Access <= '1';														-- sollen unabhaegig vom	--
							END IF;																			-- ID funktionieren!	--
						END IF;
						IF RDnWR = '1' THEN
							State_Mod_SM <= Rd_HB;
						ELSE
							State_Mod_SM <= Wr_HB;
						END IF;
					END IF;

				WHEN Rd_HB =>
					IF S_Extern_Access = '1' THEN
						S_Extern_Rd_Activ <= '1';
					END IF;
					IF S_DS_Sync = '0' THEN
						State_Mod_SM <= Wait_Rd_LB;
					END IF;

				WHEN Wait_Rd_LB =>
					IF S_DS_Sync = '1' AND Sub_Adr(0) = '1' THEN
						State_Mod_SM <= Rd_LB;
					END IF;
					
				WHEN Rd_LB =>
					IF S_DS_Sync = '0' THEN
						S_Extern_Rd_Activ	<= '0';
						S_Extern_Rd_Fin	<= '1';								-- V06, wurde in V05 erst in State "Rd_Fin" gesetzt!
						State_Mod_SM <= Rd_Fin;
					END IF;

				WHEN Rd_Fin =>
					S_Extern_Access <= '0';
					S_Intern_Access <= '0';
					State_Mod_SM <= Idle;

				WHEN Wr_HB =>
					S_Data_Wr_La(15 DOWNTO 8) <= Mod_Data;
					IF S_DS_Sync = '0' THEN
						State_Mod_SM <= Wait_Wr_LB;
					END IF;
					
				WHEN Wait_Wr_LB =>
					IF S_DS_Sync = '1' and Sub_Adr(0) = '1' THEN
						State_Mod_SM <= Wr_LB;
					END IF; 

				WHEN Wr_LB =>
					S_Data_Wr_La(7 DOWNTO 0) <= Mod_Data;
					IF S_Extern_Access = '1' THEN
						S_Extern_Wr_Activ <= (S_Extern_Wr_Activ(0), '1');
					END IF;
					IF S_DS_Sync = '0' THEN
						S_Extern_Wr_Activ <= (OTHERS => '0');
						S_Extern_Wr_Fin <= '1';								-- V06, wurde in V05 erst in State "WR_Fin" gesetzt!
						State_Mod_SM <= WR_Fin;
					END IF;

				WHEN WR_Fin =>
					S_Extern_Access <= '0';
					S_Intern_Access <= '0';
					State_Mod_SM <= Idle;

		   	END CASE;
	    END IF;
	END PROCESS Mod_SM;

P_DT_MOD_Bus: PROCESS (clk, S_SM_Reset)

	BEGIN
		IF S_SM_Reset = '1' THEN
			S_DT_Delay <= (OTHERS => '0');
			S_Start_DT_Led <= '0';
		ELSIF clk'EVENT AND clk = '1' THEN
			S_DT_Delay <= (OTHERS => '0');
			S_Start_DT_Led <= '0';
			IF S_DT_Intern = '1' THEN
				S_DT_Delay <= (S_DT_Delay(0) & '1');
				S_Start_DT_Led <= '1';
			ELSIF S_Extern_Access = '1' THEN
				IF (State_Mod_SM = Rd_HB OR State_Mod_SM = Wr_LB) AND Extern_Dtack = '1' THEN
					S_DT_Delay <= (S_DT_Delay(0) & '1');		-- Beim Lesen muss der externe Macro beim High Byte schon gueltige	--
					S_Start_DT_Led <= '1';						-- Daten liefern, beim Schreiben wird das Datum erst mit dem 		--
																-- Low Byte vom externen Macro gespeichert. Deshalb wird 			--
																-- Extern_Dtack hier ausgewertet.									--
				ELSIF State_Mod_SM = Rd_LB OR State_Mod_SM = Wr_HB THEN
					S_DT_Delay <= (S_DT_Delay(0) & '1');		-- Beim Lesen ist das Low Byte gleichzeitig mit dem High Byte		--
																-- gueltig. Beim Schreiben wird das High Byte erst zwischenge-		--
																-- speichert. In beiden Faellen wird Dtack ohne Abfrage von 			--
																-- Extern_Dtack aktiv.												--
				END IF;
			END IF;
		END IF;
	END PROCESS P_DT_MOD_Bus;
	
nDT_MOD_Bus <= '0' WHEN (S_DT_Delay(1) and not nDS) = '1' ELSE 'Z';

nExt_Data_En <= NOT ((S_Intern_Access OR S_Extern_Access) AND NOT nDS);
	
Sub_Adr_La <= S_Sub_Adr_La;


ID_OK <= S_ID_OK;
	
P_ID_OK:	PROCESS (clk, S_Powerup_Res) 
	BEGIN
		IF S_Powerup_Res = '1' THEN
			S_ID_OK <= '0';
			nID_OK_Led <= 'Z';
		ELSIF clk'EVENT AND clk = '1' THEN
			IF VG_Mod_ID = CONV_STD_LOGIC_VECTOR(MOD_ID, 8) THEN
				S_ID_OK <= '1';
				nID_OK_Led <= '0';
			ELSE
				S_ID_OK <= '0';
				nID_OK_Led <= 'Z';
			END IF;
		END IF;
	END PROCESS;
	
P_Status_Reg:	PROCESS (clk, S_Powerup_Res)
	BEGIN
		IF S_Powerup_Res = '1' THEN
			S_Status_Reg <= ("01");
			S_Wr_Status_Reg_r <= '0';
		ELSIF clk'EVENT AND clk = '1' THEN
			S_Wr_Status_Reg_r <= S_Wr_Status_Reg;
			IF S_Wr_Status_Reg_r = '1' THEN
				S_Status_Reg <= S_Status_Reg AND NOT S_Data_Wr_La(1 DOWNTO 0);
			ELSIF S_Timeout = '1' THEN
				S_Status_Reg(1) <= '1';
			END IF;
		END IF;
	END PROCESS;

	
P_Interlock:	PROCESS (S_Status_Reg(0))	-- Powerup_Res wird dem Modulbus-Kontroller ueber Interlock gemeldet
	BEGIN
		IF S_Status_Reg(0) = '0' THEN
			nInterlock <= 'Z';
		ELSE
			nInterlock <= '0';
		END IF;
	END PROCESS	P_Interlock;

P_Adr_Deco_Read_Mux: PROCESS	(
								S_Sub_Adr_La, State_Mod_SM,
								VG_MOd_ID, VG_Mod_Skal, VG_Mod_Adr, EPLD_Vers,
								ST_160_Skal, Stat_IN, Macro_Activ, Macro_Skal_OK, ST_160_Auxi,
								S_Status_Reg, V_Data_Rd, S_Extern_Access, S_Intern_Access, S_Echo_Reg
								)
	BEGIN
		S_MB_Macro_Rd_Mux	<= (OTHERS => '0');
		S_DT_Intern			<= '0';
		S_Wr_Status_Reg		<= '0';
		S_Wr_Echo_Reg		<= '0';
		IF S_Extern_Access = '1' THEN
			--+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			-- Es besteht kein Diagnose-Zugriff auf die Modul-Buskarte, d.h. andere Lese-Register sind von anderen Macros an den		+
			-- Eingang Data_RD[15..0] zu legen und das Signal 'Extern_Dtack' muss 'S_MB_Macro_Rd_Mux[]' zum Scheiben in Richtung Modul-Bus schalten.	+
			--+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			IF State_Mod_SM = Rd_HB THEN
				S_MB_Macro_Rd_Mux <= V_Data_Rd(15 DOWNTO 8);
			ELSIF State_Mod_SM = Rd_LB THEN
				S_MB_Macro_Rd_Mux <= V_Data_Rd(7 DOWNTO 0);
			END IF;
		ELSIF S_Intern_Access = '1' THEN
			CASE S_Sub_Adr_La(4 DOWNTO 1) IS
				WHEN C_Rd_ID(4 DOWNTO 1) =>
					IF State_Mod_SM = Rd_HB THEN
						S_DT_Intern <= '1';
						S_MB_Macro_Rd_Mux <= CONV_STD_LOGIC_VECTOR(MOD_ID, 8);							-- HB des 'ID' lesen = 'MOD_ID' zum Modul-Bus schalten.			--
					END IF;
					IF State_Mod_SM = Rd_LB THEN
						S_DT_Intern <= '1';
						S_MB_Macro_Rd_Mux <= VG_Mod_ID;							-- LB des 'ID' lesen = 'VG_Mod_ID[]' zum Modul-Bus schalten.	--
					END IF;
				WHEN C_Rd_Skal_Adr(4 DOWNTO 1) =>
					IF State_Mod_SM = Rd_HB THEN
						S_DT_Intern <= '1';
						S_MB_Macro_Rd_Mux <= VG_Mod_Skal;						-- HB der 'Skalierung-Adresse' = 'VG_Mod_Skal[]' zum Modul-Bus.	--
					END IF;
					IF State_Mod_SM = Rd_LB THEN
						S_DT_Intern <= '1';
						S_MB_Macro_Rd_Mux <= ('0'&'0'&'0'& VG_MOD_Adr);			-- LB der 'Skalierung-Adresse' = 'VG_Mod_Adr[]' zum Modul-Bus..	--
					END IF;
				WHEN C_Rd_EPLD_Vers_Rd_Wr_Stat(4 DOWNTO 1) =>
					IF State_Mod_SM = Rd_HB THEN
						S_DT_Intern <= '1';
						S_MB_Macro_Rd_Mux <= EPLD_Vers;							-- HB = 'EPLD-Vers[]' zum Modul-Bus.							--
					END IF;
					IF State_Mod_SM = Rd_LB THEN
						S_DT_Intern <= '1';
						S_MB_Macro_Rd_Mux <= (Stat_IN(7 DOWNTO 2) & S_Status_Reg);-- LB = 'Status_Reg[]' lesen.								--
					END IF;
					IF State_Mod_SM = Wr_HB THEN
						S_DT_Intern <= '1';
					END IF;
					IF State_Mod_SM = Wr_LB THEN
						S_DT_Intern <= '1';
					END IF;
					IF State_Mod_SM = Wr_Fin THEN
						S_Wr_Status_Reg <= '1';
					END IF;
				WHEN C_Rd_Wr_Echo_Reg(4 DOWNTO 1) =>
					IF State_Mod_SM = Rd_HB THEN
						S_DT_Intern <= '1';
						S_MB_Macro_Rd_Mux <= S_Echo_Reg(15 DOWNTO 8);
					END IF;
					IF State_Mod_SM = Rd_LB THEN
						S_DT_Intern <= '1';
						S_MB_Macro_Rd_Mux <= S_Echo_Reg(7 DOWNTO 0);
					END IF;
					IF State_Mod_SM = Wr_HB THEN
						S_DT_Intern <= '1';
					END IF;
					IF State_Mod_SM = Wr_LB THEN
						S_DT_Intern <= '1';
					END IF;
					IF State_Mod_SM = Wr_Fin THEN
						S_Wr_Echo_Reg <= '1';
					END IF;

				WHEN C_Rd_Skal2_Adr(4 DOWNTO 1) =>
					IF St_160_pol = 1 THEN
						IF State_Mod_SM = Rd_HB THEN
							S_DT_Intern <= '1';
							S_MB_Macro_Rd_Mux <= ST_160_Skal;						-- HB von 'Rd_Skal2' = 'ST_160_Skal[7..0]' zum Modul-Bus.		--
						END IF;
						IF State_Mod_SM = Rd_LB THEN
							S_DT_Intern <= '1';
							S_MB_Macro_Rd_Mux <= (Macro_Activ & Macro_Skal_OK & ST_160_Auxi);	-- LB von 'Rd_Skal2' = 'Macro_Activ', 'Macro_Skal_OK', und 		--
						END IF;
					END IF;
				WHEN OTHERS =>
					S_MB_Macro_Rd_Mux	<= (OTHERS => '0');
					S_DT_Intern			<= '0';
					S_Wr_Status_Reg		<= '0';
					S_Wr_Echo_Reg		<= '0';
			END CASE;
		END IF;
		
	END PROCESS P_Adr_Deco_Read_Mux;

P_MB_Tri_State_Buffer:	PROCESS (S_MB_Macro_Rd_Mux, S_Adr_Comp_DB, S_DS_Sync, RDnWR)
	BEGIN
		IF S_Adr_Comp_DB = '1' AND S_DS_Sync = '1' AND RDnWR = '1' THEN
			Mod_Data <= S_MB_Macro_Rd_Mux;
		ELSE
			Mod_Data <= (OTHERS => 'Z');
		END IF;
	END PROCESS P_MB_Tri_State_Buffer;
	
Powerup_Res <= S_Powerup_Res;

P_Powerup:	PROCESS (clk, S_MB_Reset)
	BEGIN
		IF S_MB_Reset = '1' THEN
			S_Powerup_Res_Cnt <= (OTHERS => '0');
		ELSIF clk'EVENT AND clk = '1' THEN
			IF S_Wr_Status_Reg_r = '1' AND S_Data_Wr_La(2) ='1' THEN
				S_Powerup_Res_Cnt <= (OTHERS => '0');
			ELSIF S_Powerup_Res_Cnt(S_Powerup_Res_Cnt'HIGH) = '0' THEN
				S_Powerup_Res_Cnt <= S_Powerup_Res_Cnt + 1;
				S_Powerup_Res <= '1';
			ELSE
				S_Powerup_Res <= '0';
			END IF;
		END IF;
	END PROCESS;

P_Echo:	PROCESS (clk, S_MB_Reset)
	BEGIN
		IF S_MB_Reset = '1' THEN
			S_Echo_Reg <= (OTHERS => '0');
		ELSIF clk'EVENT AND clk = '1' THEN
			IF S_Wr_Echo_Reg = '1' THEN
				S_Echo_Reg <= S_Data_Wr_La;
			END IF;
		END IF;
	END PROCESS P_Echo;


TO_with_lpm: IF Use_LPM = 1 GENERATE -----------------------------------------------

BEGIN
    
S_Sel_TO_Cnt <= '1' WHEN State_Mod_SM /= Idle ELSE '0';
S_Set_TO_Cnt <= not S_Sel_TO_Cnt;

timeout_cnt : lpm_counter
	GENERIC MAP (
				lpm_width	=> S_Timeout_Cnt'LENGTH,
				lpm_type	=> "LPM_COUNTER",
				lpm_direction => "DOWN",
				lpm_avalue	=> integer'image(C_Timeout_Cnt),
				lpm_svalue	=> integer'image(C_Timeout_Cnt)
				)
	PORT MAP(
			clock	=> clk,
			aset	=> S_SM_Reset,
			sset	=> S_Set_TO_Cnt,
			cnt_en	=> S_Sel_TO_Cnt,
			q		=> S_Timeout_Cnt
			);
			
END GENERATE TO_with_lpm; ----------------------------------------------------------
			

TO_without_lpm: IF Use_LPM = 0 GENERATE --------------------------------------------

BEGIN
P_Timeout:	PROCESS (clk, S_SM_Reset)
	BEGIN
		IF S_SM_Reset = '1' THEN
				S_Timeout_Cnt <= conv_std_logic_vector(C_Timeout_Cnt, S_Timeout_Cnt'LENGTH);
		ELSIF clk'EVENT AND clk = '1' THEN
			IF State_Mod_SM = Idle THEN
				S_Timeout_Cnt <= conv_std_logic_vector(C_Timeout_Cnt, S_Timeout_Cnt'LENGTH);
			ELSIF S_Timeout_Cnt(S_Timeout_Cnt'LEFT) = '0' THEN
				S_Timeout_Cnt <= S_Timeout_Cnt - 1;
			ELSE
				S_Timeout_Cnt <= S_Timeout_Cnt;
			END IF;
		END IF;			
	END PROCESS;
	
END GENERATE TO_without_lpm; -------------------------------------------------------

S_Timeout <= S_Timeout_Cnt(S_Timeout_Cnt'LEFT);
Timeout	<= S_Timeout;

Data_Wr_La		<= S_Data_Wr_La;


Extern_Wr_Activ	<= S_Extern_Wr_Activ(1);
Extern_Wr_Fin	<= S_Extern_Wr_Fin;

	
Extern_Rd_Activ	<= S_Extern_Rd_Activ;
Extern_Rd_Fin	<= S_Extern_Rd_Fin;


S_Adr_Comp_Live <= '1' WHEN VG_Mod_Adr(4 DOWNTO 0) = Mod_Adr(4 DOWNTO 0) ELSE '0';

Adr_Debounce:	Debounce
	GENERIC MAP(
				DB_Cnt		=> C_nDS_Deb_cnt,	
				DB_Tst_Cnt	=> 1,
				Use_lpm		=> Use_lpm,
				Test		=> 0
				)
	PORT MAP(
			DB_In		=>  S_Adr_Comp_Live,
			Reset		=>  S_Powerup_Res,
			Clk			=>	clk,
			DB_Out		=>	S_Adr_Comp_DB
			);

	
S_DS <= not nDS;

DS_Debounce:	Debounce
	GENERIC MAP(
				DB_Cnt		=> C_nDS_Deb_cnt,	
				DB_Tst_Cnt	=> 1,
				Use_lpm		=> Use_lpm,
				Test		=> 0
				)
	PORT MAP(
			DB_In		=>  S_DS,
			Reset		=>  S_Powerup_Res,
			Clk			=>	clk,
			DB_Out		=>	S_DS_Sync
			);

MB_Reset <= not nMB_Reset;

Res_Debounce:	Debounce
	GENERIC MAP(
				DB_Cnt		=> C_Res_Deb_cnt,	
				DB_Tst_Cnt	=> 1,
				Use_lpm		=> Use_lpm,
				Test		=> Test
				)
	PORT MAP(
			DB_In		=>  MB_Reset,
			Reset		=>  '0',
			Clk			=>	clk,
			DB_Out		=>	S_MB_Reset
			);




Led_Cnt_with_lpm: IF Use_LPM = 1 GENERATE -----------------------------------------------

BEGIN
led_cnt : lpm_counter
	GENERIC MAP (
				lpm_width	=> s_led_cnt'length,
				lpm_type	=> "LPM_COUNTER",
				lpm_direction => "DOWN",
				lpm_svalue	=> integer'image(C_Ld_Led_cnt)
				)
	PORT MAP(
			clock	=> clk,
			sset	=> s_led_cnt(s_led_cnt'high),
			q		=> s_led_cnt
			);

END GENERATE Led_Cnt_with_lpm; ----------------------------------------------------------


Led_Cnt_without_lpm: IF Use_LPM = 0 GENERATE --------------------------------------------

BEGIN
P_Led_Ena:	PROCESS (clk, S_Powerup_Res)
	BEGIN
		IF S_Powerup_Res = '1' THEN
			S_Led_Cnt <= conv_std_logic_vector(C_Ld_Led_cnt, s_led_cnt'length);
		ELSIF clk'EVENT AND clk = '1' THEN
			IF S_Led_Cnt(S_Led_Cnt'LEFT) = '1' THEN
				S_Led_Cnt <= conv_std_logic_vector(C_Ld_Led_cnt, s_led_cnt'length);
			ELSE
				S_Led_Cnt <= S_Led_Cnt - 1;
			END IF;
		END IF;
	END PROCESS P_Led_Ena;
	
END GENERATE Led_Cnt_without_lpm; -------------------------------------------------------

S_Led_Ena <= s_led_cnt(s_led_cnt'high);
Led_Ena <= S_Led_Ena;

Sel_Led:	Led
GENERIC MAP (
			Use_LPM => Use_LPM
			)
PORT MAP(
		Sig_in 		=>	S_Adr_Comp_DB,
		Ena		 	=>	S_Led_Ena,
		clk 		=>	clk,
		nLed		=>	nSel_Led
		);

DT_Led :	Led
GENERIC MAP (
			Use_LPM => Use_LPM
			)
PORT MAP(
		Sig_in 		=>	S_Start_DT_Led,
		Ena		 	=>	S_Led_Ena,
		clk 		=>	clk,
		nLed		=>	nDT_Led
		);

nPower_Up_Led	<= NOT S_Status_Reg(0);
	
END Arch_modulbus_v7;
