LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use work.scu_diob_pkg.all;
 
entity BLM_gate_err_mux is

    port (
        CLK             : in std_logic;      -- Clock
        nRST            : in std_logic;      -- Reset
        sel             : in t_BLM_12_to_6_mux_sel_array ;
        in_s            : in std_logic_vector(11 downto 0);
        out_s           : out std_logic_vector(5 downto 0)
      
    );
end BLM_gate_err_mux;

architecture rtl of BLM_gate_err_mux is

TYPE t_sel is array (0 to 5) of integer range 0 to 11;
signal int_sel: t_sel;
signal out_signal: std_logic_vector(5 downto 0);


 

begin
      
  proc_b: process (clk,nRST)
    begin
        if not nRST='1' then 
             for i in 0 to 5 loop
             	int_sel(i) <=0;
             end loop;
            out_signal <=( others =>'0');
           
             
        elsif (clk'EVENT AND clk= '1') then 
        
           for i in 0 to 5 loop
             int_sel(i) <= to_integer(unsigned (sel(i)));
             out_signal(i)<= in_s(int_sel(i));
            
           end loop;
           
        end if;
        out_s <= out_signal;
    end process;
end architecture;
