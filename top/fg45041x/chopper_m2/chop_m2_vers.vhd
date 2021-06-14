--TITLE "chop_m2_vers sollte alle Aenderungsbemerkungen des 'Chopper_Macro2' enthalten.";

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;

entity chop_m2_vers is
	generic (Test:			integer := 0);	
	port (	chop_m2_vers : 		out std_logic_vector(7 downto 0);
			Test_Vers_Aktiv : 	out std_logic
		);
end chop_m2_vers;

architecture chop_m2_vers_arch of chop_m2_vers is

function prod_or_test(production, test_data, test : integer) return integer is
	variable data: integer;
begin
	if Test = 1 then
		data := test_data;
	else
		data := production;
	end if;
	
	return data;
end prod_or_test;
	
constant Versions_nr : std_logic_vector	:= conv_std_logic_vector(prod_or_test(3, 0, Test), 8);

begin
	chop_m2_vers  <= Versions_nr;
	

	Test_Vers_Aktiv <= '1'  when Test = 1 else '0';
	
	
	assert not(Versions_nr = X"00")
  	report "      ----------- Dies ist eine Testversion des Chopper-Macro2 !!!!  -----------"
	severity Warning;

	assert not(VERSIONS_NR /= X"00")
  REPORT "      Die Design-Version des Chopper-Macro2 lautet    -------->  03  <---------"
SEVERITY Warning;
	
	
end chop_m2_vers_arch;