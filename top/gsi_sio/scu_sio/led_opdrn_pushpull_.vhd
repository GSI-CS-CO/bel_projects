LIBRARY ieee;
USE ieee.std_logic_1164.all; 

LIBRARY work;

ENTITY led_opdrn_pushpull IS
	GENERIC (
			Opendrain_is_1: integer range 0 to 1 := 1;
			stretch_cnt:	integer := 3
			); 
	port	(
			ena:		in	STD_LOGIC;
			CLK:		in	STD_LOGIC;
			Sig_In:		in	STD_LOGIC;
			nLED:		out	STD_LOGIC
			);
END led_opdrn_pushpull;

ARCHITECTURE arch_led_opdrn_pushpull OF led_opdrn_pushpull IS 

component div_n
	generic	(
			n:			integer := stretch_cnt;
			diag_on:	integer range 0 to 1 := 0
			);
	port	(
			res:		in	std_logic := '0';
			clk:		in	std_logic;
			ena:		in	std_logic := '1';
			div_o:		out	std_logic
			);
end component;

signal	nled_active:	std_logic;

BEGIN

P_stretch:	div_n
	generic map	(
				n		=> stretch_cnt,
				diag_on	=> 1
				)
	port map	(
				res		=> sig_in,					-- löscht div_n; div_n zählt bis zum Überlauf
				clk		=> clk,
				ena		=> ena and not nled_active,	-- wenn der Überlauf erreicht ist, wird wird div_n gestoppt.
				div_o	=> nled_active
				);

	
P_Led_on:	PROCESS (nled_active)
	BEGIN
		IF nled_active = '0' THEN
			nLed <= '0';
		ELSE
			if Opendrain_is_1 = 1 then
				nLed <= 'Z';
			else
				nLed <= '1';
			end if;
		END IF;
	END PROCESS P_Led_on;

END arch_led_opdrn_pushpull; 