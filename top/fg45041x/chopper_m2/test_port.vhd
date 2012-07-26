library IEEE;
USE ieee.std_logic_1164.all;



entity test_port is
	port (
			sel:		in std_logic_vector(3 downto 0);
			in_0:		in std_logic_vector(15 downto 0);
			in_1:		in std_logic_vector(15 downto 0);
			in_2:		in std_logic_vector(15 downto 0);
			in_3:		in std_logic_vector(15 downto 0);
			in_4:		in std_logic_vector(15 downto 0);
			in_5:		in std_logic_vector(15 downto 0);
			in_6:		in std_logic_vector(15 downto 0);
			in_7:		in std_logic_vector(15 downto 0);
--			in_8:		in std_logic_vector(15 downto 0);
--			in_9:		in std_logic_vector(15 downto 0);
--			in_10:		in std_logic_vector(15 downto 0);
--			in_11:		in std_logic_vector(15 downto 0);
--			in_12:		in std_logic_vector(15 downto 0);
----			in_13:		in std_logic_vector(15 downto 0);
----			in_14:		in std_logic_vector(15 downto 0);
----			in_15:		in std_logic_vector(15 downto 0);
			
			test_port_out:	out std_logic_vector(15 downto 0)
		);
end test_port;

architecture test_port_arch of test_port is
begin

test_mux: process (sel, in_0, in_1, in_2, in_3,
						in_4, in_5, in_6, in_7
--						in_8, in_9, in_10, in_11,
--						in_12--, in_13, in_14, in_15
						)
begin
	case (not sel) is
		when x"0" =>
			test_port_out <= in_0;
		when x"1" =>
			test_port_out <= in_1;
		when x"2" =>
			test_port_out <= in_2;
		when x"3" =>
			test_port_out <= in_3;
		when x"4" =>
			test_port_out <= in_4;
		when x"5" =>
			test_port_out <= in_5;
		when x"6" =>
			test_port_out <= in_6;
		when x"7" =>
			test_port_out <= in_7;
--		when x"8" =>
--			test_port_out <= in_8;
--		when x"9" =>
--			test_port_out <= in_9;
--		when x"a" =>
--			test_port_out <= in_10;
--		when x"b" =>
--			test_port_out <= in_11;
--		when x"c" =>
--			test_port_out <= in_12;
----		when x"d" =>
----			test_port_out <= in_13;
----		when x"e" =>
----			test_port_out <= in_14;
----		when x"f" =>
----			test_port_out <= in_15;

		when others => test_port_out <= in_0;
		
	end case;

end process;

end test_port_arch;