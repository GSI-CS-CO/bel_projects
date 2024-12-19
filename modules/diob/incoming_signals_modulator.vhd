library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity incoming_signals_modulator is
    Port ( clk : in STD_LOGIC;
           nRST : in STD_LOGIC;
           in_sig : in STD_LOGIC;
           out_sig: out STD_LOGIC)
;
end incoming_signals_modulator;

architecture behavioral of incoming_signals_modulator is

type  debouncer_state_t is   (zero, zero_debounced, wait_one, one, one_debounced, wait_zero, bad, reset);
signal deb_state:   debouncer_state_t:= zero;

signal cnt: integer range 0 to 7;
signal out_signal: std_logic;
begin

 debouncer_proc: process (clk, nRST)
    begin
        if nRST = '0' then
           

            deb_state <= zero;
          
        
            cnt <= 0;
            out_signal <= '0';

        elsif (clk'EVENT and clk = '1') then

            case (deb_state) is
                when zero => 
                
                cnt <= 0;
  
                        if cnt= 3 then 
                            deb_state <= zero_debounced;
                            cnt <=0;
                        else 
                            cnt <= cnt +1;
                        end if;
                  
                when zero_debounced =>
              
                   
                    if in_sig ='1' then
                        deb_state <= bad;
                        cnt <=0;
                    else
                        if cnt= 7 then
                            cnt <=0;
                            deb_state <= wait_one;
                        else                             
                            cnt <= cnt +1;
                        end if;
                    end if;
                
                when wait_one =>
               
                    if in_sig ='1' then
                        cnt <= 0;
                        deb_state <= one;
                        
                    else
                        if cnt= 7 then
                            deb_state <= bad;
                            cnt <=0;
                        else
                            cnt<= cnt+1;
                        end if;
                    end if;

                when one =>    
         
                    out_signal <= '1';
                      
                        if cnt = 3 then
                            deb_state <= one_debounced;
                       
                            cnt <= 0;
                        else
                            cnt<= cnt + 1;
                        end if;                       
                   
                   
                when one_debounced =>
                if in_sig ='0' then     
                    deb_state <= bad;  
                    cnt <=0; 
                 else
                    if cnt = 7 then
                        deb_state <= wait_zero;
                        cnt<= 0;
                    else
                        cnt <= cnt + 1;
              
                    end if;
                end if;

                when wait_zero => 
                    if in_sig ='0' then
                        deb_state <= zero;
                        cnt <=0;
                    else
                        if cnt = 7 then
                            deb_state <= bad;
                            cnt <=0;

                        else
                        cnt <= cnt + 1;
                        end if;
                    end if;

                when bad =>
                    out_signal <= '0';
                    if in_sig = '1' then
                        deb_state <= reset;
                        cnt <=0;
                    end if;

                when reset =>
                    if in_sig ='0' then
                        deb_state <= zero;
                        cnt<=0;
                    end if;
                
                when others => NULL;
             end case;
      

    end if;
end process;
out_sig <= out_signal;
end architecture behavioral;


                    


                 