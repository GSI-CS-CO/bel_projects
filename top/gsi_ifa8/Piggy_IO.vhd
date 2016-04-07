--TITLE "'Piggy_IO' => Piggy-Stecker-IO für IFA8 , Autor: R. Hartmann, Stand: 11.06.2012, Vers: V05";

LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;
use IEEE.MATH_REAL.ALL;

ENTITY Piggy_IO IS


	PORT
	(
		SCLR			: IN STD_LOGIC;
		clk				: IN STD_LOGIC;
		Ena_Every_20ms	: IN STD_LOGIC;
		FG_Mode			: IN STD_LOGIC;		-- FG-Mode der Interfacekarte
--
		Piggy_Led		: IN STD_LOGIC_VECTOR(15 DOWNTO 0);  -- "LED-Register"    der IFK ==> Piggy (Fg 380.751)
		Piggy_Led_Out	: IN STD_LOGIC_VECTOR(15 DOWNTO 0);  -- "Output-Register" der IFK ==> Output-Buchen + LED's vom Piggy
--
		STG_STR			: IN STD_LOGIC;						 -- Steigungs-Stobe vom Funktionsgenerator ==> ADC-Trigger (Piggy)
		FG_LED			: IN STD_LOGIC_VECTOR(15 DOWNTO 0);  -- LED's vom Funktionsgenerator ==> Piggy (Fg 380.751)
--
		A_P				: INOUT STD_LOGIC_VECTOR(45 DOWNTO 0);	-- Piggy-Stecker
		BU_IN1			: OUT STD_LOGIC;                  		-- Input-Buchse 1 
		Piggy_ID		: INOUT STD_LOGIC_VECTOR( 7 DOWNTO 0)	-- Piggy-Ident
	);


END Piggy_IO;


ARCHITECTURE Arch_Piggy_IO OF Piggy_IO IS


	COMPONENT	LED
	PORT(	ENA :  		IN  STD_LOGIC;
			CLK :		IN  STD_LOGIC;
			Sig_In :	IN  STD_LOGIC;
			nLED :		OUT STD_LOGIC);
	END COMPONENT;

-- +-----------------------------------------------------------------+
-- |     Output's des LED-Multiplexer zu den "LED-Mono" Eingängen    |
-- +-----------------------------------------------------------------+
	SIGNAL		S_Mux_LED			: STD_LOGIC_VECTOR(15 DOWNTO 0);
	SIGNAL		S_Mux_LED_BU_IN1	: STD_LOGIC;
	SIGNAL		S_Mux_LED_BU_Out1	: STD_LOGIC;
	SIGNAL		S_Mux_LED_BU_Out2	: STD_LOGIC;
	SIGNAL		S_Mux_LED_BU_Trig1	: STD_LOGIC;
	SIGNAL		S_Mux_LED_BU_Trig2	: STD_LOGIC;
--
-- +-------------------------------------------------------+
-- |        Output's des Lemo-Buchsen-Multiplexer          |
-- +-------------------------------------------------------+
	SIGNAL		S_Mux_BU_IN1		: STD_LOGIC;	-- Input-Buchse		
	SIGNAL		S_Mux_BU_Out1		: STD_LOGIC;	-- Output-Buchse		
	SIGNAL		S_Mux_BU_Out2		: STD_LOGIC;	-- Output-Buchse		
	SIGNAL		S_Mux_BU_Trig1		: STD_LOGIC;	-- Output-Buchse		
	SIGNAL		S_Mux_BU_Trig2		: STD_LOGIC;	-- Output-Buchse		
                                  
--
-- +--------------------------------------------------------+
-- |          "LED-Mono" Ausgänge zu den LED-Output's       |
-- +--------------------------------------------------------+
	SIGNAL		S_nLED			: STD_LOGIC_VECTOR(15 DOWNTO 0);
	SIGNAL		S_nLED_BU_IN1	: STD_LOGIC;
	SIGNAL		S_nLED_BU_Out1	: STD_LOGIC;
	SIGNAL		S_nLED_BU_Out2	: STD_LOGIC;
	SIGNAL		S_nLED_BU_Trig1	: STD_LOGIC;
	SIGNAL		S_nLED_BU_Trig2	: STD_LOGIC;
--

BEGIN


--=================== Multiplexer =================================

p_mux:	PROCESS (clk, SCLR,
				 FG_Mode, FG_LED, STG_STR, Piggy_LED, Piggy_Led_Out, S_Mux_BU_IN1)
	BEGIN


	if FG_Mode = '1' then

		S_Mux_LED			<= FG_LED;
		S_Mux_LED_BU_IN1	<= '0';
		S_Mux_LED_BU_Out1	<= '0';
		S_Mux_LED_BU_Out2	<= '0';
		S_Mux_LED_BU_Trig1	<= STG_STR;
		S_Mux_LED_BU_Trig2	<= STG_STR;
                          
		BU_IN1				<= '0';		-- Dummy wird nicht benutzt
		S_Mux_BU_Out1		<= '0';				
		S_Mux_BU_Out2		<= '0';				
		S_Mux_BU_Trig1		<= STG_STR;				
		S_Mux_BU_Trig2		<= STG_STR;				


	else
		S_Mux_LED			<= Piggy_LED;
		S_Mux_LED_BU_IN1	<= Piggy_Led_Out(8);
		S_Mux_LED_BU_Out1	<= Piggy_Led_Out(7);
		S_Mux_LED_BU_Out2	<= Piggy_Led_Out(6);
		S_Mux_LED_BU_Trig1	<= Piggy_Led_Out(5);
		S_Mux_LED_BU_Trig2	<= Piggy_Led_Out(4);
                             
		BU_IN1				<= S_Mux_BU_IN1;
		S_Mux_BU_Out1		<= Piggy_Led_Out(3);				
		S_Mux_BU_Out2		<= Piggy_Led_Out(2);				
		S_Mux_BU_Trig1		<= Piggy_Led_Out(1);				
		S_Mux_BU_Trig2		<= Piggy_Led_Out(0);				
	end if;                
END PROCESS p_mux;


--========================== Start: LED-"Monoflopp" ================================================

L_15:       LED PORT MAP (CLK => CLK, ENA => Ena_Every_20ms, Sig_In => S_Mux_LED(15),	 nLED => s_nLED(15));
L_14:       LED PORT MAP (CLK => CLK, ENA => Ena_Every_20ms, Sig_In => S_Mux_LED(14),	 nLED => s_nLED(14));
L_13:       LED PORT MAP (CLK => CLK, ENA => Ena_Every_20ms, Sig_In => S_Mux_LED(13),	 nLED => s_nLED(13));
L_12:       LED PORT MAP (CLK => CLK, ENA => Ena_Every_20ms, Sig_In => S_Mux_LED(12),	 nLED => s_nLED(12));
L_11:       LED PORT MAP (CLK => CLK, ENA => Ena_Every_20ms, Sig_In => S_Mux_LED(11),	 nLED => s_nLED(11));
L_10:       LED PORT MAP (CLK => CLK, ENA => Ena_Every_20ms, Sig_In => S_Mux_LED(10),	 nLED => s_nLED(10));
L_9:        LED PORT MAP (CLK => CLK, ENA => Ena_Every_20ms, Sig_In => S_Mux_LED(9),	 nLED => s_nLED(9));
L_8:        LED PORT MAP (CLK => CLK, ENA => Ena_Every_20ms, Sig_In => S_Mux_LED(8),	 nLED => s_nLED(8));
L_7:        LED PORT MAP (CLK => CLK, ENA => Ena_Every_20ms, Sig_In => S_Mux_LED(7),	 nLED => s_nLED(7));
L_6:        LED PORT MAP (CLK => CLK, ENA => Ena_Every_20ms, Sig_In => S_Mux_LED(6),	 nLED => s_nLED(6));
L_5:        LED PORT MAP (CLK => CLK, ENA => Ena_Every_20ms, Sig_In => S_Mux_LED(5),	 nLED => s_nLED(5));
L_4:        LED PORT MAP (CLK => CLK, ENA => Ena_Every_20ms, Sig_In => S_Mux_LED(4),	 nLED => s_nLED(4));
L_3:        LED PORT MAP (CLK => CLK, ENA => Ena_Every_20ms, Sig_In => S_Mux_LED(3),	 nLED => s_nLED(3));
L_2:        LED PORT MAP (CLK => CLK, ENA => Ena_Every_20ms, Sig_In => S_Mux_LED(2),	 nLED => s_nLED(2));
L_1:        LED PORT MAP (CLK => CLK, ENA => Ena_Every_20ms, Sig_In => S_Mux_LED(1),	 nLED => s_nLED(1));
L_0:        LED PORT MAP (CLK => CLK, ENA => Ena_Every_20ms, Sig_In => S_Mux_LED(0),	 nLED => s_nLED(0));
--                                                                           
L_BU_IN1:   LED PORT MAP (CLK => CLK, ENA => Ena_Every_20ms, Sig_In => S_Mux_LED_BU_IN1,  nLED => S_nLED_BU_IN1);
L_BU_Out1:  LED PORT MAP (CLK => CLK, ENA => Ena_Every_20ms, Sig_In => S_Mux_LED_BU_Out1, nLED => S_nLED_BU_Out1);
L_BU_Out2:  LED PORT MAP (CLK => CLK, ENA => Ena_Every_20ms, Sig_In => S_Mux_LED_BU_Out2, nLED => S_nLED_BU_Out2);
L_BU_Trig1: LED PORT MAP (CLK => CLK, ENA => Ena_Every_20ms, Sig_In => S_Mux_LED_BU_Trig1,nLED => S_nLED_BU_Trig1);
L_BU_Trig2: LED PORT MAP (CLK => CLK, ENA => Ena_Every_20ms, Sig_In => S_Mux_LED_BU_Trig2,nLED => S_nLED_BU_Trig2);
                                                                                           
--=========================== End: LED-"Monoflopp" ================================================




--=========================== Signalzuordnung zum Piggy-Stecker ===================================

A_P(1)	<= s_nLED(0);
A_P(3)	<= s_nLED(1);
A_P(5)	<= s_nLED(2);
A_P(7)	<= s_nLED(3);
A_P(9)	<= s_nLED(4);
A_P(11)	<= s_nLED(5);
A_P(13)	<= s_nLED(6);
A_P(15)	<= s_nLED(7);
A_P(17)	<= s_nLED(8);
A_P(19)	<= s_nLED(9);
A_P(21)	<= s_nLED(10);
A_P(23)	<= s_nLED(11);
A_P(25)	<= s_nLED(12);
A_P(27)	<= s_nLED(13);
A_P(29)	<= s_nLED(14);
A_P(31)	<= s_nLED(15);
--
A_P(0)	<= S_nLED_BU_IN1;	-- LED, Input-Buchse
A_P(2)	<= S_nLED_BU_Out1;	-- LED, Output-Buchse
A_P(4)	<= S_nLED_BU_Out2;	-- LED, Output-Buchse
A_P(6)	<= S_nLED_BU_Trig1;	-- LED, Output-Buchse
A_P(8)	<= S_nLED_BU_Trig2; -- LED, Output-Buchse
--                
S_Mux_BU_IN1	<= A_P(10);			-- Input-Buchse 
A_P(12)			<= S_Mux_BU_Out1;	-- Output-Buchse
A_P(14)			<= S_Mux_BU_Out2;	-- Output-Buchse
A_P(16)			<= S_Mux_BU_Trig1;	-- Output-Buchse
A_P(18)			<= S_Mux_BU_Trig2;	-- Output-Buchse
--                
---- Unbenutzte Treiber auf 'Z' ----------------
--
A_P(20)	<= 'Z';
A_P(22)	<= 'Z';
A_P(24)	<= 'Z';
A_P(26)	<= 'Z';
A_P(28)	<= 'Z';
--
A_P(30)	<= 'Z';
--
A_P(32)	<= 'Z';
A_P(33)	<= 'Z';
A_P(34)	<= 'Z';
A_P(35)	<= 'Z';
A_P(36)	<= 'Z';
A_P(37)	<= 'Z';
--
-- Piggy-ID
--
Piggy_ID(0)	<= A_P(38);
Piggy_ID(1)	<= A_P(39);
Piggy_ID(2)	<= A_P(40); 
Piggy_ID(3)	<= A_P(41); 
Piggy_ID(4)	<= A_P(42); 
Piggy_ID(5)	<= A_P(43);
Piggy_ID(6)	<= A_P(44);
Piggy_ID(7)	<= A_P(45);
           
                                                   	
END Arch_Piggy_IO;                                                          
