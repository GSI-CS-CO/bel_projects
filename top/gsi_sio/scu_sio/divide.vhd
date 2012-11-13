
-- TITLE "'divide' => Autor: R.Hartmann, Stand: 06.04.2009, Vers: V01 ";

library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.numeric_std.all;
use ieee.math_real.all;


			
entity divide is

	generic
    (
		n	: integer := 2
    );

	port
    (
		res:    in	std_logic := '0';
		clk:    in	std_logic;
    ena:    in	std_logic := '1';
    div_o:  out	std_logic
    );

	constant  c_divider_width:	integer := integer(floor(log2(real(n))));
														
	signal    s_divide:         unsigned(c_divider_width DOWNTO 0);-- := to_unsigned(n - 2, c_divider_width + 1);
  
end divide;
			

architecture arch_divide of divide is


begin

ASSERT NOT(n < 2 )
	REPORT " Produktions-Count muss >= 2 sein"
SEVERITY ERROR;



p_divider:	process (clk, Res)
	begin
		if res = '1' THEN
			 s_divide <= to_unsigned(n - 2, s_divide'length);
		elsif	rising_edge(clk) THEN
			if s_divide(s_divide'high) = '1' THEN		-- Bei underflow wird neu geladen	--
				 s_divide <= to_unsigned(n - 2, s_divide'length);
			else
        s_divide <= s_divide - 1;				      -- subtrahieren.					--
			end if;
		end if;
	end process p_divider;

div_o <= s_divide(s_divide'high);

end arch_divide;

