library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity out_modulator_el is

    Port (
        clk : in  STD_LOGIC;
        nRST : in STD_LOGIC;
        mod_out: out STD_LOGIC
    );
end out_modulator_el;

architecture behavioral of out_modulator_el is

    signal mod_temp: std_logic:='0';
    signal mod_cnt : integer range 0 to 15:=0;
begin

    modulator_proc: process (nRST, clk) 
    begin
    if (nRST = '0') then
           mod_temp <='0' ;
        mod_cnt <=0;
    elsif (clk'EVENT and clk = '1') then
            if mod_cnt = 15 then 
                mod_temp <= not mod_temp;
                mod_cnt <= 0;
            else
                mod_cnt <= mod_cnt +1;
                
            end if;
        
        end if;
    end process;
    
    mod_out <= mod_temp;
end behavioral;