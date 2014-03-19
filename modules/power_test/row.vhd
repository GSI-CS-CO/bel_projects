library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
 
entity row is
	generic(
		row_width:  integer := 64
    );
  Port(
		clk_i:    in  std_logic;
    nrst_i:   in  std_logic;
    row_i:    in  unsigned(row_width-1 downto 0);
		or_o:     out std_logic
    );
end row;
 
architecture row_arch of row is
 
signal the_row: unsigned(row_width-1 downto 0);

attribute preserve: boolean;
attribute preserve of the_row: signal is true;
 
begin
 
p_row:  process (clk_i, nrst_i)
  begin
    if nrst_i = '0' then
      the_row <= to_unsigned(0, row_width);
    elsif rising_edge(clk_i) then
      the_row <= row_i; 
      if the_row = 0 then
      	or_o 	<= '0'; 
      else
        or_o <= '1';
      end if;
    end if;
  end process p_row;

end row_arch;

