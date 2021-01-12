library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;

entity chop_m1_vers is
	generic (Test:			integer := 0);
	port (	chop_m1_vers : 		out std_logic_vector(7 downto 0);
			Test_Vers_Aktiv : 	out std_logic
		);
end chop_m1_vers;

architecture chop_m1_vers_arch of chop_m1_vers is

function prod_or_test(production, test_data: std_logic_vector(7 downto 0); test : integer) return std_logic_vector is
	variable data: std_logic_vector(7 downto 0);
begin
	if Test = 1 then
		data := test_data;
	else
		data := production;
	end if;

	return data;
end prod_or_test;

constant Versions_nr : std_logic_vector	:= prod_or_test(x"14", x"00", Test);


begin
	chop_m1_vers  <= Versions_nr;


	Test_Vers_Aktiv <= '1'  when Test = 1 else '0';


	assert not(Versions_nr = X"00")
        report "      ----------- Dies ist eine Testversion des Chopper-Macro1 !!!!  -----------"
	severity Warning;

	assert not(VERSIONS_NR /= X"00")
  REPORT "      Die Design-Version des Chopper-Macro1 lautet    -------->  14  <---------"
SEVERITY Warning;


end chop_m1_vers_arch;
