--#######################################################--
--###############     Martin Hardieck     ###############--
--###############         9.12.13         ###############--
--#######################################################--

--+---------------------------------------------------------------------------------+
--| Modified by W.Panschow                                                          |
--|   1) Removed Generated "Big_Or"                                                 |
--|   2) Remove libraries                                                           |
--+---------------------------------------------------------------------------------+ 

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity row_array is
	generic(
		row_width:    integer := 64;
		row_cnt:      integer := 32
    );
	port(
		nrst_i:       in  std_logic := '0';
		clk_i:        in  std_logic := '0';
		row_i:        in  unsigned(row_width-1 downto 0);
		or_o:         out std_logic
    );
  end entity row_array;

architecture row_array_arch of row_array is

	
signal rows_output:   unsigned(row_cnt-1 downto 0);

	
begin

	row_array: for i in 0 to row_cnt-1 generate
  	row: entity work.row
      generic map(
        row_width	=> row_width
        )
      Port map(
        clk_i   => clk_i,
        nrst_i  => nrst_i,
        row_i   => row_i,
        or_o    => rows_output(i)
        );
    end generate row_array;
	

  p_or: process (clk_i, nrst_i)
    begin
      if nrst_i = '0' then
        or_o <= '0';
      elsif rising_edge(clk_i) then
        if rows_output = 0 then
          or_o <= '0';
        else
          or_o <= '1';
        end if;
      end if;
    end process p_or;

 	
end architecture row_array_arch;