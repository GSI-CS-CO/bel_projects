library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity BLM_watchdog is
generic (

    hold    : integer range 1 TO 10:= 2 --timeout
  
);
port(
    clk_i : in std_logic;     -- chip-internal pulsed clk signal
    rstn_i : in std_logic;   -- reset signal
    in_watchdog : in std_logic;     -- input signal
    ena_i : in std_logic;     -- enable '1' for input connected to the counter
    INTL_out: out std_logic   -- interlock output for signal that doesn't change for a given time (2 clocks)...question: when interlock, it remains INTL_out ='1' for other 2 clocks?
 
);
end BLM_watchdog;

architecture rtl of BLM_watchdog is

type  watchdog_state_t is   (idle, check,interlock);
signal watchdog_state:   watchdog_state_t:= idle;
constant timeout_reset : unsigned := to_unsigned(hold, (2));
signal new_val_wait   : std_logic :='0';
signal timeout : unsigned(1 downto 0) := timeout_reset;
signal curr_val   : std_logic :='0';
begin
 

  curr_val <= in_watchdog;


watchdog_proc: process (clk_i, rstn_i, ena_i)

  begin

      if ((rstn_i= '0') or (ena_i='0'))   then
        INTL_out  <= '0';
         watchdog_state <= idle;
         new_val_wait   <= '0';
         timeout <= timeout_reset;

	       watchdog_state <= idle;
	 
      elsif rising_edge(clk_i) then
         
          case watchdog_state is

              when idle =>
              	       if ena_i ='0' then
                   
                          new_val_wait   <= '0';
                      else
                      timeout <= timeout_reset; 
                      new_val_wait <= in_watchdog;
                    
                      if (to_integer (timeout) >0) then
                       
                        
                        timeout <= timeout - 1;
                          
        			  end if;
                      if (to_integer (timeout) = 0) then
                       
              	            watchdog_state <= check;
              	       end if; 
                      end if;    
                      
	       when check =>
	        
                      if  new_val_wait = curr_val then
                        
                           INTL_out  <= '1';
                          watchdog_state <= interlock;
                      else 
                
                      INTL_out  <= '0';
                         watchdog_state <= idle;
          		      end if;
                
                when interlock => 
                
                   INTL_out <='1';

                   watchdog_state <= idle;
               
                	 
                when others => null;
               end case;
            end if;
         
         end process;
         	    
 end rtl;          		 
	
