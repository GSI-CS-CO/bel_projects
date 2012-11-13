LIBRARY ieee;
USE ieee.std_logic_1164.all; 

LIBRARY work;

ENTITY led_n IS
	GENERIC (
			stretch_cnt:	integer := 3
			); 
	port	(
			ena:		in	STD_LOGIC;
			CLK:		in	STD_LOGIC;
			Sig_In:		in	STD_LOGIC;
			nLED:		out	STD_LOGIC
			);
END led_n;

ARCHITECTURE arch_led_n OF led_n IS 

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
				diag_on	=> 0
				)
	port map	(
				res		=> sig_in,					-- löscht div_n; div_n zählt bis zum Überlauf
				clk		=> clk,
				ena		=> ena and not nled_active,	-- wenn der Überlauf erreicht ist, wird wird div_n gestoppt.
				div_o	=> nled_active
				);

	
nLed <= nled_active;

END arch_led_n; 