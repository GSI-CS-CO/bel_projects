LIBRARY ieee;
USE ieee.std_logic_1164.all; 
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;
LIBRARY lpm;
USE lpm.lpm_components.all;

LIBRARY work;

ENTITY Kicker_Leds IS
	GENERIC	(
			Use_LPM : INTEGER := 0
			); 
	port
	(
		Led_Sig :				IN		STD_LOGIC_VECTOR(15 downto 0);
		clk :					IN		STD_LOGIC;
		Led_Ena :				IN		STD_LOGIC := '1';
		nLed_Out :				OUT		STD_LOGIC_VECTOR(15 downto 0)
	);
END Kicker_Leds;

	
ARCHITECTURE Arch_Kicker_Leds OF Kicker_Leds IS

COMPONENT	Led
	GENERIC	(
			Use_LPM : INTEGER := 0
			); 
	PORT(
			clk		: IN	STD_LOGIC;
			ena		: IN	STD_LOGIC;
			Sig_In	: IN	STD_LOGIC;
			nled	: OUT	STD_LOGIC
		);
END COMPONENT;

SIGNAL S_Led_Sig:	STD_LOGIC_VECTOR(15 downto 0);
SIGNAL S_Led_Ena:	STD_LOGIC;

BEGIN

Pipeline_Ena: PROCESS (CLK)
BEGIN
	IF rising_edge(clk) THEN
		S_Led_Ena <= Led_Ena;
	END IF;
END PROCESS Pipeline_Ena;

S_Led_Sig <= Led_Sig;
				
Gen_leds: FOR i IN 0 TO 15 GENERATE
	L:	Led GENERIC MAP(Use_LPM => Use_LPM) 
			PORT MAP(clk => clk,
			 		 ena => S_Led_Ena,
					 Sig_In => S_Led_Sig(I),
					 nled => nLed_Out(I));
	END GENERATE;
END; 
