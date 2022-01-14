LIBRARY ieee;
USE ieee.std_logic_1164.all; 
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;
LIBRARY lpm;
USE lpm.lpm_components.all;

LIBRARY work;

ENTITY led IS
	GENERIC (
			Use_LPM : INTEGER := 0
			); 
	port
	(
		ENA :  IN  STD_LOGIC;
		CLK :  IN  STD_LOGIC;
		Sig_In :  IN  STD_LOGIC;
		nLED :  OUT  STD_LOGIC
	);
END led;

ARCHITECTURE arch_led OF led IS 

COMPONENT lpm_counter
	GENERIC (
			lpm_width		: NATURAL;
			lpm_type		: STRING;
			lpm_direction	: STRING
			);
	PORT(
		clock	: IN STD_LOGIC ;
		cnt_en	: IN STD_LOGIC := '1';
		q		: OUT STD_LOGIC_VECTOR (lpm_width-1 DOWNTO 0);
		aclr	: IN STD_LOGIC 
		);
END COMPONENT;


CONSTANT	C_Cnt_Len	: INTEGER := 3;
SIGNAL		S_Cnt		: STD_LOGIC_VECTOR(C_Cnt_Len-1 DOWNTO 0);


BEGIN

Led_with_lpm: IF Use_LPM = 1 GENERATE --------------------------------------

	SIGNAL S_Ena : STD_LOGIC;

	BEGIN
	S_Ena <= '1' WHEN (S_Cnt(S_Cnt'high) = '0') AND (Ena = '1') ELSE '0';

	led_cnt : lpm_counter
		GENERIC MAP (
					lpm_width	=> C_Cnt_Len,
					lpm_type	=> "LPM_COUNTER",
					lpm_direction => "UP"
					)
		PORT MAP(
				clock	=> clk,
				aclr	=> Sig_In,
				cnt_en	=> S_Ena,
				q		=> S_Cnt
				);


--led_opndrn : opndrn
--PORT MAP(A_IN => S_Cnt(S_Cnt'high),
--		 A_OUT => nLED);
		
END GENERATE Led_with_lpm; -------------------------------------------------


Led_without_lpm: IF Use_LPM = 0 GENERATE -----------------------------------

BEGIN
P_Led_Stretch:	PROCESS (clk, Sig_in, S_Cnt)
	BEGIN
		IF Sig_in = '1' THEN
			S_Cnt <= CONV_STD_LOGIC_VECTOR(0, S_Cnt'LENGTH);
		ELSIF clk'EVENT AND CLK = '1' THEN
			IF Ena = '1' AND S_Cnt(S_Cnt'HIGH) = '0' THEN
				S_Cnt <= S_Cnt + 1;
			END IF;
		END IF;
	END PROCESS P_Led_Stretch;
	
END GENERATE Led_without_lpm; ----------------------------------------------
	
P_Led_on:	PROCESS (S_Cnt(S_Cnt'HIGH))
	BEGIN
		IF S_Cnt(S_Cnt'HIGH) = '0' THEN
			nLed <= '0';
		ELSE
			nLed <= 'Z';
		END IF;
	END PROCESS P_Led_on;

END arch_led; 