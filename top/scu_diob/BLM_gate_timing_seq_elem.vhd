library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity BLM_gate_timing_seq_elem is

port(
  clk_i : in std_logic;          -- chip-internal pulsed clk signal
  rstn_i : in std_logic;        -- reset signal
  gate_in : in std_logic;        -- input signal
  gate_in_ena : in std_logic;     -- enable '1' for input connected to the counter
  hold: in std_logic_vector(7 downto 0);
  timeout_error : out std_logic;  -- gate doesn't start within the given timeout
  gate_out: out std_logic        -- out gate signal
);
end BLM_gate_timing_seq_elem;

architecture rtl of BLM_gate_timing_seq_elem is

type   gate_state_t is   (idle, ready, timeout_state, gate_out_state);
signal gate_state:   gate_state_t:= idle;

signal timeout_reset : unsigned(7 downto 0); 

signal new_val_wait: std_logic :='0';
signal timeout : unsigned(7 downto 0) := timeout_reset;
signal curr_val   :std_logic:='0';


begin

  

  curr_val <= gate_in and gate_in_ena;
timeout_reset <=unsigned(hold); -- to be checked

gate_proc: process (clk_i, rstn_i, gate_in_ena)

  begin

      if ((rstn_i= '0') or (gate_in_ena)='0')  then
        timeout_error  <= '0';
         gate_state <= idle;
         new_val_wait   <= '0';
         timeout <= timeout_reset;
	 gate_state <= idle;
	 
      elsif rising_edge(clk_i) then
    
          case gate_state is

              when idle =>
              	        if curr_val='0' then
                          
                          new_val_wait   <= '0';
      			            else
                        if (to_integer (timeout) >0) then
                          if timeout = timeout_reset then
                              new_val_wait <= gate_in ;
                          end if;
                          timeout <= timeout - 1;
                        
                        end if;
                        if (to_integer (timeout) = 0) then
                          timeout_error<= '0';
                          gate_state <= ready;
                        end if;  
                      end if;
                        
              	        
              	        
	            when ready =>
	        
                      if new_val_wait = curr_val then
                      
                          gate_state <= timeout_state;
                      else
                        gate_state <= gate_out_state;  
                      end if;
                      
              when timeout_state => 
                
                	 timeout_error <='1';
                	 timeout <= timeout_reset;
                   gate_out <= '0';
               	gate_state <= idle;
                	
              when gate_out_state =>
                
                	 timeout_error <='0';
                	 timeout <= timeout_reset;
                	 
                
                   gate_out  <= curr_val;
                   gate_state <= idle;
              when others => null;
          end case;
        end if;

  end process;
         	    
 end rtl;          		 
	

