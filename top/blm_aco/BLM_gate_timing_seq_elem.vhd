library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity BLM_gate_timing_seq_elem is

port(
  clk_i : in std_logic;          -- 
  rstn_i : in std_logic;        -- reset signal
  gate_in : in std_logic;        -- input signal
 -- gate_in_ena : in std_logic;     -- enable '1' for input connected to the counter
  prepare : in std_logic;
  recover : in std_logic;
  hold: in std_logic_vector(15 downto 0);
  all_thres_ready: in std_logic;
  gate_error : out std_logic;  -- gate doesn't start within the given timeout
  gate_state_nr : out std_logic_vector (2 downto 0); --for tests
  gate_out: out std_logic        -- out gate signal
);
end BLM_gate_timing_seq_elem;

architecture rtl of BLM_gate_timing_seq_elem is

type   gate_state_t is   (idle,prepare_state, gate, waiting, error, recover_state);
signal gate_state:   gate_state_t;
signal gate_state_sn:   gate_state_t:= idle;
signal gate_er: std_logic;
signal timeout_reset : unsigned(15 downto 0); 
signal gate_out_sm: std_logic;
signal timeout : unsigned(15 downto 0);
signal curr_val   :std_logic; --:='0';
signal state_sm: integer range 0 to 5:= 0;
signal state_nr: std_logic_vector (2 downto 0);
signal ready: std_logic;
begin

  state_sm_proc: process(clk_i, rstn_i)--(gate_state)
  begin
  if ((rstn_i= '0')) then 
    state_sm <= 0;
  elsif rising_edge(clk_i) then
    case gate_state is
      when idle           => state_sm <= 0;
      when prepare_state  => state_sm <= 1;
      when gate           => state_sm <= 2;
      when waiting        => state_sm <= 3;
      when error          => state_sm <= 4;
      when recover_state  => state_sm <= 5;
      when others         => null;
    end case;
  end if;
  end process;




gate_proc: process (clk_i, rstn_i)

  begin

--curr_val <= gate_in ;

      if ((rstn_i= '0')) then 
        gate_error  <= '0';
        gate_out <='0';
        gate_er <='0';
        gate_state <= idle;
        gate_state_nr <= "000";
        curr_val <='0';
        ready <= '0';
      elsif rising_edge(clk_i) then

        curr_val <= gate_in ;
        gate_state_nr <=  std_logic_vector(to_unsigned(state_sm, gate_state_nr'length));
        gate_error <= gate_er;
        gate_out <= gate_out_sm;
        timeout_reset <=unsigned(hold); 
        ready <= all_thres_ready;

         case gate_state is

             when idle =>
                    timeout <= timeout_reset;

                    if curr_val='1' then --0
                      gate_state <= error;    
                     
                    elsif prepare ='1' then
                       gate_state <=prepare_state;
                   
                     end if;

             when prepare_state => --1
                      
                     if curr_val ='1' then
                       gate_state <= gate;
                       gate_out_sm <= '1';
                     
                     else
                        if ready ='1' then 
                            timeout <= timeout -1;
                            if (to_integer(timeout )=0) then
                                gate_state <= error;
                            end if;  
                        end if;
                    end if;
 
 
              when gate => --2
                    
                        if curr_val ='0' then
                          gate_state <= waiting;
                          gate_out_sm <='0';
                         
                        end if;

              when waiting => --3
                    
                          if curr_val ='1' then
                            gate_state <= error;
                            
                            gate_er <='1';
                          
                          elsif prepare ='0' then
                           gate_state <= idle;
                       
                          end if;
                
              when error => --4
                            
                            if recover ='1' then
                              gate_state <= recover_state;
                             
                            end if;
                      
              when recover_state => --5
                    
                              if recover ='0' then 
                               gate_state <= idle;
                               gate_er<='0';
                               
                              end if;
                   
              when others => null;
          end case;
        end if;

  end process;
 
         	    
 end rtl;          		 
	