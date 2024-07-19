LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
 

entity BLM_ct_pulse_former is

    port (
        CLK         : in std_logic;      -- Clock
        nRST         : in std_logic;      -- Reset
        SIG_IN       : in std_logic;    -- Load counter register up input
        
        SIG_OUT    : out std_logic      -- UP_Counter overflow
    
    );
end BLM_ct_pulse_former;


architecture rtl of BLM_ct_pulse_former is

 
    signal sig, last_sig: std_logic;
    signal cur_sig : std_logic;
  begin

    cur_sig <= SIG_IN;
      COUNT_Pulse_former: process (nRST, CLK)
      begin
          if (nRST = '0') then    
            sig <= '0';
            last_sig <= '0';  
         
          elsif (CLK'event and CLK='1') then
            last_sig <= cur_sig;
            
            if (cur_sig ='1') and (last_sig ='0') then 

                sig <= '1';
            else
                sig <= '0';
            end if;
        end if;
    end process;

    SIG_OUT <= sig;
end rtl;