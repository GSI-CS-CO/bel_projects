LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;

ENTITY Skal_Test IS
	GENERIC (
			ST_160_Pol			: INTEGER := 1;
			No_Port_Dir_Test	: INTEGER := 0;
			K3_Input			: INTEGER := 1;
			K3D_Def_Level		: INTEGER := 0;		-- Vorgabe fuer den Default Level der Kanal_3 Datenleitungen. Wird veglichen mit A_K3D_SPG.  
			K3C_Def_Level		: INTEGER := 1;		-- Vorgabe fuer den Default Level der Kanal_3 Steuerleitungen. Wird veglichen mit A_K3C_SPG.
			K2_Input			: INTEGER := 1;
			K2D_Def_Level		: INTEGER := 0;		-- Vorgabe fuer den Default Level der Kanal_2 Datenleitungen. Wird veglichen mit A_K2D_SPG.  
			K2C_Def_Level		: INTEGER := 1;		-- Vorgabe fuer den Default Level der Kanal_2 Steuerleitungen. Wird veglichen mit A_K2C_SPG.
			Gr1_APK_ID			: INTEGER := 1;
			Gr1_16Bit			: INTEGER := 1;
			K1_Input			: INTEGER := 1;
			K1D_Def_Level		: INTEGER := 0;		-- Vorgabe fuer den Default Level der Kanal_1 Datenleitungen. Wird veglichen mit A_K1D_SPG.  
			K1C_Def_Level		: INTEGER := 1;		-- Vorgabe fuer den Default Level der Kanal_1 Steuerleitungen. Wird veglichen mit A_K1C_SPG.
			K0_Input			: INTEGER := 1;
			K0D_Def_Level		: INTEGER := 0;		-- Vorgabe fuer den Default Level der Kanal_0 Datenleitungen. Wird veglichen mit A_K0D_SPG.  
			K0C_Def_Level		: INTEGER := 1;		-- Vorgabe fuer den Default Level der Kanal_0 Steuerleitungen. Wird veglichen mit A_K0C_SPG.
			Gr0_APK_ID			: INTEGER := 1;
			Gr0_16Bit			: INTEGER := 1;
			No_Level_Test		: INTEGER := 0;
			No_Logic_Test		: INTEGER := 0;
			Logic_Nr_Start		: INTEGER RANGE 0 TO 31 := 0;
			Logic_Nr_End		: INTEGER RANGE 0 TO 31 := 0
			);
	PORT
	(
		VG_Mod_Skal			: IN STD_LOGIC_VECTOR(7 downto 0);
		St_160_Skal			: IN STD_LOGIC_VECTOR(7 downto 0);
		Logik				: IN STD_LOGIC_VECTOR(5 downto 0);
		A_K3D_SPG			: IN STD_LOGIC;		-- zeigt den Default Level der Kanal_3 Datenleitungen an ("0" = pull down, "1" = pull up).  
		A_K3C_SPG			: IN STD_LOGIC;		-- zeigt den Default Level der Kanal_3 Kontrollsignale an ("0" = pull down, "1" = pull up).
		A_K2D_SPG			: IN STD_LOGIC;		-- zeigt den Default Level der Kanal_2 Datenleitungen an ("0" = pull down, "1" = pull up).  
		A_K2C_SPG			: IN STD_LOGIC;		-- zeigt den Default Level der Kanal_2 Kontrollsignale an ("0" = pull down, "1" = pull up).
		A_K1D_SPG			: IN STD_LOGIC;		-- zeigt den Default Level der Kanal_1 Datenleitungen an ("0" = pull down, "1" = pull up).  
		A_K1C_SPG			: IN STD_LOGIC;		-- zeigt den Default Level der Kanal_1 Kontrollsignale an ("0" = pull down, "1" = pull up).
		A_K0D_SPG			: IN STD_LOGIC;		-- zeigt den Default Level der Kanal_0 Datenleitungen an ("0" = pull down, "1" = pull up).  
		A_K0C_SPG			: IN STD_LOGIC;		-- zeigt den Default Level der Kanal_0 Kontrollsignale an ("0" = pull down, "1" = pull up).
		clk 				: IN STD_LOGIC;
		All_Okay 			: OUT STD_LOGIC;
		Mod_Skal_Ok 		: OUT STD_LOGIC;
		Level_Ok 			: OUT STD_LOGIC;
		Logic_Nr_Ok 		: OUT STD_LOGIC;
		nK0_Switch_Ena		: OUT STD_LOGIC;
		nK1_Switch_Ena		: OUT STD_LOGIC;
		nK2_Switch_Ena		: OUT STD_LOGIC;
		nK3_Switch_Ena		: OUT STD_LOGIC;
		nSkal_Okay_Led		: OUT STD_LOGIC
	);
END Skal_Test;


ARCHITECTURE Arch_Skal_Test OF Skal_Test IS

	CONSTANT	C_K3_Input		: STD_LOGIC_VECTOR(0 DOWNTO 0) := CONV_STD_LOGIC_VECTOR(K3_Input, 1);
	CONSTANT	C_K2_Input		: STD_LOGIC_VECTOR(0 DOWNTO 0) := CONV_STD_LOGIC_VECTOR(K2_Input, 1);
	CONSTANT	C_Gr1_APK_ID	: STD_LOGIC_VECTOR(0 DOWNTO 0) := CONV_STD_LOGIC_VECTOR(Gr1_APK_ID, 1);
	CONSTANT	C_Gr1_16Bit		: STD_LOGIC_VECTOR(0 DOWNTO 0) := CONV_STD_LOGIC_VECTOR(Gr1_16Bit, 1);

	CONSTANT	C_K1_Input		: STD_LOGIC_VECTOR(0 DOWNTO 0) := CONV_STD_LOGIC_VECTOR(K1_Input, 1);
	CONSTANT	C_K0_Input		: STD_LOGIC_VECTOR(0 DOWNTO 0) := CONV_STD_LOGIC_VECTOR(K0_Input, 1);
	CONSTANT	C_Gr0_APK_ID	: STD_LOGIC_VECTOR(0 DOWNTO 0) := CONV_STD_LOGIC_VECTOR(Gr0_APK_ID, 1);
	CONSTANT	C_Gr0_16Bit		: STD_LOGIC_VECTOR(0 DOWNTO 0) := CONV_STD_LOGIC_VECTOR(Gr0_16Bit, 1);

	CONSTANT	C_K0D_Def_Level	: STD_LOGIC_VECTOR(0 DOWNTO 0) := CONV_STD_LOGIC_VECTOR(K0D_Def_Level, 1);  
	CONSTANT	C_K0C_Def_Level	: STD_LOGIC_VECTOR(0 DOWNTO 0) := CONV_STD_LOGIC_VECTOR(K0C_Def_Level, 1);
	CONSTANT	C_K1D_Def_Level	: STD_LOGIC_VECTOR(0 DOWNTO 0) := CONV_STD_LOGIC_VECTOR(K1D_Def_Level, 1);  
	CONSTANT	C_K1C_Def_Level	: STD_LOGIC_VECTOR(0 DOWNTO 0) := CONV_STD_LOGIC_VECTOR(K1C_Def_Level, 1);
	CONSTANT	C_K2D_Def_Level	: STD_LOGIC_VECTOR(0 DOWNTO 0) := CONV_STD_LOGIC_VECTOR(K2D_Def_Level, 1);  
	CONSTANT	C_K2C_Def_Level	: STD_LOGIC_VECTOR(0 DOWNTO 0) := CONV_STD_LOGIC_VECTOR(K2C_Def_Level, 1);
	CONSTANT	C_K3D_Def_Level	: STD_LOGIC_VECTOR(0 DOWNTO 0) := CONV_STD_LOGIC_VECTOR(K3D_Def_Level, 1);  
	CONSTANT	C_K3C_Def_Level	: STD_LOGIC_VECTOR(0 DOWNTO 0) := CONV_STD_LOGIC_VECTOR(K3C_Def_Level, 1);

	
	SIGNAL		S_All_Okay		: STD_LOGIC;
	SIGNAL		S_Mod_Skal_Ok	: STD_LOGIC;
	SIGNAL		S_Logic_Ok		: STD_LOGIC;
	SIGNAL		S_Level_Ok		: STD_LOGIC;
	
BEGIN

ST_160: IF ST_160_Pol = 1 GENERATE -----------------------------------------------

P_Skal_Test:	PROCESS (clk)
	BEGIN
	    IF clk'EVENT AND clk = '1' THEN

			IF No_Logic_Test = 0 THEN
				IF 		VG_Mod_Skal(7 DOWNTO 4) = (C_K1_Input(0) & C_K0_Input(0) & C_Gr0_APK_ID(0) & C_Gr0_16Bit(0))
			    	AND St_160_Skal(7 DOWNTO 4) = (C_K3_Input(0) & C_K2_Input(0) & C_Gr1_APK_ID(0) & C_Gr1_16Bit(0)) THEN
					S_Mod_Skal_Ok <= '1';
				ELSE
					S_Mod_Skal_Ok <= '0';
				END IF;
			ELSE
				S_Mod_Skal_Ok <= '1';
			END IF;

			IF No_Logic_Test = 0 THEN
				IF 		Logik >= conv_std_logic_vector( Logic_Nr_Start, Logik'LENGTH)
					AND Logik <= conv_std_logic_vector( Logic_Nr_End, Logik'LENGTH) THEN
					S_Logic_Ok <= '1';
				ELSE
					S_Logic_Ok <= '0';
				END IF;
			ELSE
				S_Logic_Ok <= '1';
			END IF;
			
			IF No_Level_Test = 0 THEN
				IF		A_K0C_SPG = C_K0C_Def_Level(0) AND A_K0D_SPG = C_K0D_Def_Level(0)
					AND A_K1C_SPG = C_K1C_Def_Level(0) AND A_K1D_SPG = C_K1D_Def_Level(0)
					AND A_K2C_SPG = C_K2C_Def_Level(0) AND A_K2D_SPG = C_K2D_Def_Level(0)
					AND A_K3C_SPG = C_K3C_Def_Level(0) AND A_K3D_SPG = C_K3D_Def_Level(0) THEN
					S_Level_Ok <= '1';
				ELSE
					S_Level_Ok <= '0';
				END IF;
			ELSE
				S_Level_Ok <= '1';
			END IF;
			
			IF  S_Mod_Skal_Ok = '1' AND S_Logic_Ok = '1' AND S_Level_Ok = '1' THEN
				S_All_Okay <= '1';
				nSkal_Okay_LED <= '0';
			ELSE
				S_All_Okay <= '0';
				nSkal_Okay_LED <= 'Z';
			END IF;
			
		END IF;
	END PROCESS;
	
nK0_Switch_Ena <= '0';
nK1_Switch_Ena <= '0';
nK2_Switch_Ena <= '0';
nK3_Switch_Ena <= '0';

END GENERATE ST_160; -------------------------------------------------------------

ST_96: IF ST_160_Pol = 0 GENERATE -----------------------------------------------

P_Skal_Test:	PROCESS (clk)
	BEGIN
	    IF clk'EVENT AND clk = '1' THEN
			IF No_Logic_Test = 0 THEN
				IF 	VG_Mod_Skal(7 DOWNTO 4) = (C_K1_Input(0) & C_K0_Input(0) & C_Gr0_APK_ID(0) & C_Gr0_16Bit(0)) THEN
					S_Mod_Skal_Ok <= '1';
				ELSE
					S_Mod_Skal_Ok <= '0';
				END IF;
			ELSE
				S_Mod_Skal_Ok <= '1';
			END IF;

			IF No_Level_Test = 0 THEN
				IF		A_K0C_SPG = C_K0C_Def_Level(0) AND A_K0D_SPG = C_K0D_Def_Level(0)
					AND A_K1C_SPG = C_K1C_Def_Level(0) AND A_K1D_SPG = C_K1D_Def_Level(0) THEN
					S_Level_Ok <= '1';
				ELSE
					S_Level_Ok <= '0';
				END IF;
			ELSE
				S_Level_Ok <= '1';
			END IF;
			
			IF  S_Mod_Skal_Ok = '1' AND S_Level_Ok = '1' THEN
				S_All_Okay <= '1';
				nSkal_Okay_LED <= '0';
			ELSE
				S_All_Okay <= '0';
				nSkal_Okay_LED <= 'Z';
			END IF;
		END IF;
	END PROCESS;
	
nK0_Switch_Ena <= '0';
nK1_Switch_Ena <= '0';
nK2_Switch_Ena <= '0';	-- obwohl der Kanal_2 nicht gebraucht wird muss er selektiert werden, damit die Pulldowns	--
						-- an anderen Seite des FET-Schalter die Einaenge am EPLD auf def. Pegel legen.				-- 
nK3_Switch_Ena <= '0';	-- obwohl der Kanal_3 nicht gebraucht wird muss er selektiert werden, damit die Pulldowns	--
						-- an anderen Seite des FET-Schalter die Einaenge am EPLD auf def. Pegel legen.				-- 


END GENERATE ST_96; -------------------------------------------------------------

All_Okay <= S_All_Okay;

Mod_Skal_Ok <= S_Mod_Skal_Ok;
Level_Ok <= S_Level_Ok;
Logic_Nr_Ok <= S_Mod_Skal_Ok;

END Arch_Skal_Test;
