
---! Standard library
library IEEE;
--! Standard packages
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;
use std.textio.all;
use work.oled_display_pkg.all;


entity char_render is
port(
			clk_i : in std_logic;
			nRst_i : std_logic;
			addr_char_i : in std_logic_vector(7 downto 0);
			load_i : in std_logic;
			valid_o : out std_logic;
			q_o			: out std_logic_vector(7 downto 0)
);
end char_render;

architecture behavioral of char_render is




signal q : pixmap;
signal sh_cnt : std_logic_vector(3 downto 0);
alias done : std_logic is sh_cnt(3); 


FUNCTION bit_reverse(reverseMe:std_logic_vector) return std_logic_vector is
     variable back2front : std_logic_vector(reverseMe'length-1 downto 0);
  begin
    for i in reverseMe'left downto 0 loop
      back2front(i) := reverseMe(reverseMe'left-i);
    end loop;
    return back2front;
  end bit_reverse; 




begin



valid_o <= not done;
q_o <= bit_reverse(q(4));

main : process(clk_i)

variable addr : natural;

begin


	if(rising_edge(clk_i)) then
		if(nRst_i = '0') then
		  sh_cnt <= x"f";
		else
		if(load_i = '1') then
		  sh_cnt <= x"5";
		  addr := to_integer(unsigned(addr_char_i));
		
		if((addr  >= 32) and (addr  <= 128))  then
			q <= mymap(addr-32);
		else
			--ASCII address remapping to non continuous block
			case (addr) is
					
				when 129 => q <= mymap(133-32); -- 
				when 132 => q <= mymap(134-32); -- 
				when 142 => q <= mymap(129-32); -- 
				when 148 => q <= mymap(135-32); -- 
				when 153 => q <= mymap(130-32); -- 
				when 154 => q <= mymap(131-32); -- 
				when 167 => q <= mymap(128-32); -- degree
				when 181 => q <= mymap(137-32); -- 
				when 225 => q <= mymap(132-32); --
				when 230 => q <= mymap(137-32); --  
				when 234 => q <= mymap(136-32); -- Omega
				when 241 => q <= mymap(144-32); -- plusminus
				when others => q <= mymap(143-32); -- FULLBLOCK
			end case;	
		end if;	
		else
		  if(done = '0') then
		    q <= x"00" & q(0 to 3) ;
		    sh_cnt <= std_logic_vector(unsigned(sh_cnt) -1);
		  end if;  
		end if;	
	end if;
	end if;	
end process;




end architecture;

