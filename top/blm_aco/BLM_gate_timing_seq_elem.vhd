library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity BLM_gate_timing_seq_elem is

port(
  clk_i : in std_logic;          -- 
  rstn_i : in std_logic;        -- reset signal
  gate_in : in std_logic;        -- input signal
  gate_in_ena : in std_logic;     -- enable '1' for input connected to the counter
  prepare : in std_logic;
  recover : in std_logic;
  hold: in std_logic_vector(15 downto 0);
  timeout_error : out std_logic;  -- gate doesn't start within the given timeout
  gate_out: out std_logic        -- out gate signal
);
end BLM_gate_timing_seq_elem;

architecture rtl of BLM_gate_timing_seq_elem is

type   gate_state_t is   (idle, timeout_state, check_state, gate_out_state);
signal gate_state:   gate_state_t:= idle;

signal timeout_reset : unsigned(15 downto 0); 

signal last_val: std_logic;-- :='0';
signal timeout : unsigned(15 downto 0);-- := timeout_reset;
signal curr_val   :std_logic; --:='0';


begin

  

  curr_val <= gate_in and gate_in_ena;
timeout_reset <=unsigned(hold); -- to be checked

gate_proc: process (clk_i, rstn_i, gate_in_ena)

  begin

      if ((rstn_i= '0') or (gate_in_ena)='0' ) then 
        timeout_error  <= '0';
   
         timeout <= timeout_reset;
	      gate_state <= idle;
	 
      elsif rising_edge(clk_i) then
        last_val <= curr_val;
          case gate_state is

              when idle =>
                      if prepare ='1' then

                   
                      

                        if curr_val='1' and last_val ='0' then
                          --timeout <= timeout_reset;
                          gate_state <= gate_out_state;
                       else
                          timeout <= timeout - 1;
                        
                        end if;
                        if (to_integer (timeout) = 0) then
                         -- if curr_val='1' and last_val ='0' then 
                      
                         --   gate_state <= gate_out_state;
                         -- else
                            gate_state <= timeout_state;  
                          end if;
                         
                        end if;  
                     --- end if;
                    
                        
              	        
             
              when timeout_state => 

                   timeout_error <='1';
                   gate_out <= '0';
                   if recover ='1' then
                	  timeout <= timeout_reset;
                    timeout_error <='0';
                    gate_state <= check_state;
                  end if;

              when gate_out_state =>
               
                	  timeout_error <='0';
                	  timeout <= timeout_reset;
                    gate_out  <= curr_val;               
                    gate_state <= check_state;
                
              
                  when check_state =>
                    if (prepare ='0' and recover='0') then 
                      gate_state <= idle;
                    end if;

              when others => null;
          end case;
        end if;

  end process;
         	    
 end rtl;          		 
	

