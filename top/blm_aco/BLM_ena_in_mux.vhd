LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use work.scu_diob_pkg.all;
 
entity BLM_ena_in_mux is
    port (
        CLK               : in std_logic;      -- Clock
        nRST              : in std_logic;      -- Reset
        mux_sel           : in t_BLM_reg_Array;  -- I need only bit 15..12 of each BLM_ena_Reg register
        in_mux            : in std_logic_vector(11 downto 0);
        cnt_enable        : out std_logic_vector(255 downto 0)
    );
end BLM_ena_in_mux;

architecture rtl of BLM_ena_in_mux is

TYPE gate_sel_array is ARRAY (0 to 255) of INTEGER RANGE 0 to 15;
signal gate_sel: gate_sel_array;
signal uns_in: unsigned(11 downto 0);
signal uns_enable: unsigned(255 downto 0);


begin
 uns_in <= unsigned(in_mux);            
cnt_enable_proc: process (clk,nRST)
begin
    if not nRST='1' then 
  
     gate_sel  <= (others => 0);
     uns_enable <= (others =>'0');
      
elsif (clk'EVENT AND clk= '1') then 

for i in 0 to 255 loop

    gate_sel(i) <=  to_integer(unsigned(mux_sel(i)(15 downto 12)));

    case gate_sel(i) is

        when 0  =>  uns_enable(i) <= uns_in(0);
        when 1  =>  uns_enable(i) <= uns_in(1);
        when 2  =>  uns_enable(i) <= uns_in(2);
        when 3  =>  uns_enable(i) <= uns_in(3);
        when 4  =>  uns_enable(i) <= uns_in(4);
        when 5  =>  uns_enable(i) <= uns_in(5);
        when 6  =>  uns_enable(i) <= uns_in(6);
        when 7  =>  uns_enable(i) <= uns_in(7);
        when 8  =>  uns_enable(i) <= uns_in(8);
        when 9  =>  uns_enable(i) <= uns_in(9);
        when 10 =>  uns_enable(i) <= uns_in(10);
        when 11 =>  uns_enable(i) <= uns_in(11);
        when others => NULL;
       
    end case;
end loop;

end if;

 
end process;
cnt_enable <= std_logic_vector(uns_enable);
end architecture;