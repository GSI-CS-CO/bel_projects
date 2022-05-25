library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity BLM_gate_timing_seq is

generic (
  freq    : natural range 500 TO 125000000 := 125000000;
  hold    : integer range 2 TO 10:= 2;
  n       : integer range 0 TO 11 :=4
);
port(
  clk_i : in std_logic;          -- chip-internal pulsed clk signal
  rstn_i : in std_logic;        -- reset signal
  gate_in : in std_logic_vector(n-1 downto 0);        -- input signal
  initialize : in std_logic;     -- enable '1' for input connected to the counter
  timeout_error : out std_logic; -- gate doesn't start within the given timeout
  gate_out: out std_logic_vector(n-1 downto 0)        -- out gate signal
);
end BLM_gate_timing_seq;

architecture rtl of BLM_gate_timing_seq is

type   gate_state_t is   (idle, ready, timeout_state, gate_out_state);
signal gate_state:   gate_state_t:= idle;

constant timeout_reset : unsigned := to_unsigned(hold, (n));

signal new_val_wait: unsigned(n-1 downto 0) := (others => '0');
signal timeout : unsigned((n-1) downto 0) := timeout_reset;
signal curr_val   : unsigned((n-1) downto 0);


begin

  

  curr_val <= unsigned(gate_in((n-1) downto 0));


gate_proc: process (clk_i, rstn_i, initialize)

  begin

      if ((rstn_i= '0') or (initialize='0'))   then
        timeout_error  <= '0';
         gate_state <= idle;
         new_val_wait   <= (others => '0');
         timeout <= timeout_reset;
	       gate_state <= idle;
	 
      elsif rising_edge(clk_i) then
    
          case gate_state is

              when idle =>
              	        if initialize ='0' then
                          
                          new_val_wait   <= (others => '0');
      			            else
                        if (to_integer (timeout) >0) then
                          if timeout = timeout_reset then
                              new_val_wait <= unsigned(gate_in(n-1 downto 0));
                          end if;
                          timeout <= timeout - 1;
                        
                        end if;
                        if (to_integer (timeout) = 0) then
                          timeout_error <='0';  
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
                   gate_out <= (others =>'0');
                	gate_state <= idle;
                	
              when gate_out_state =>
                
                	 timeout_error <='0';
                	 timeout <= timeout_reset;
                	 
                
                   gate_out  <= std_logic_vector(curr_val);
                   gate_state <= idle;
              when others => null;
          end case;
        end if;

  end process;
         	    
 end rtl;          		 
	
