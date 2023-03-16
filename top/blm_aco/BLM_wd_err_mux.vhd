LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use work.scu_diob_pkg.all;
 
entity BLM_wd_err_mux is

    port (
        CLK               : in std_logic;      -- Clock
        nRST              : in std_logic;      -- Reset
        mux_sel           : in t_BLM_wd_mux_sel_array ;
        in_mux            : in std_logic_vector(53 downto 0);
        out_mux           : out std_logic_vector(5 downto 0)
      
    );
end BLM_wd_err_mux;

architecture rtl of BLM_wd_err_mux is
TYPE int_wd_err_sel is array (0 to 5) of integer range 0 to 53;
signal int_sel: int_wd_err_sel;


 

    begin
      
        out_gate_err_proc: process (clk,nRST)
        begin
            if not nRST='1' then 
             for i in 0 to 5 loop
             	int_sel(i) <=0;
             end loop;
             
            out_mux <= (others =>'0');
             
       elsif (clk'EVENT AND clk= '1') then 
           for i in 0 to 5 loop
            int_sel(i) <= to_integer(unsigned (mux_sel(i)));
            out_mux(i)<= in_mux(int_sel(i));
           
               end loop;
           
        end if;
    end process;
end architecture;


