LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;

--+-----------------------------------------------------------------------------------------------------------------+
--| Vers_13:																										|
--|		Autor:	W. Panschow																							|
--|		Datum:	25.08.2010																							|
--|		Grund:	Korrektur eines Fehlers.																			|
--|		Beschreibung: Der Fehler wurde bei Zweistrahlbetrieb festgestellt. Da wurde der HSI-Chopper nur dann		|
--|				aufgemacht,	wenn M3 anfordert, dieser wurde aber vom HLI-Chopper gesteuert.							|
--|		Ursache: Bei der grundlegenden Ueberarbeitung des Makros "chopper_m1_logic" in Vers_12 wurde das Signal		|
--|				"Off_Anforderung_In" nicht mehr abgefragt ob es durch den "HSI_ALV" und "HLI_ALV" gueltig werden		|
--|				darf.																								|
--|		Korrektur: Ist komplett im Makro "chopper_m1_logic" vorgenommen worden.										|
--|				Im Prozess "align_anf_hsi" wird "Off_Anforderung_In" mit "HSI_ALV" maskiert.						|
--|				Und im Prozess "align_anf_hli" wird "Off_Anforderung_In" mit "HLI_ALV" maskiert.					|
--+-----------------------------------------------------------------------------------------------------------------+



ENTITY epld_vers IS
	GENERIC(Test : integer range 0 to 1 := 0 );
	PORT
	(
		Vers_Rev :		OUT STD_LOGIC_VECTOR(7 downto 0);
		Test_Activ :	OUT STD_LOGIC
	);
	
END epld_vers;


ARCHITECTURE Arch_epld_vers OF epld_vers IS

constant	c_vers:	integer range 0 to 15 := 1;
constant	c_revi:	integer range 0 to 15 := 3;

BEGIN

ASSERT not (Test = 0)
	REPORT ">>>>>  chopper_m1 ist fuer Produktion uebersetzt,   Version = " & integer'image(C_Vers) & ",   Revi = " & integer'image(C_Revi)
SEVERITY WARNING;

ASSERT (Test = 0)
	REPORT ">>>>>  chopper_m1 ist fuer Test uebersetzt!!!,   Version = " & integer'image(0) & ",   Revision = " & integer'image(0)
SEVERITY WARNING;


Test_Activ <= '0' when Test = 0 else '1';

Vers_Rev(7 DOWNTO 0) <= (CONV_STD_LOGIC_VECTOR(C_Vers, 4) & CONV_STD_LOGIC_VECTOR(C_Revi, 4)) when Test = 0 else CONV_STD_LOGIC_VECTOR(0, 8);
	
END Arch_epld_vers;
