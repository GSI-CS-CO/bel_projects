library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity BLM_watchdog is

port(
    clk_i : in std_logic;     -- chip-internal pulsed clk signal
    rst_i : in std_logic;   -- general reset signal
    wd_reset: in std_logic; -- watchdog reset signal
    hold: in std_logic_vector(15 downto 0);
    in_watchdog : in std_logic;     -- input signal
   -- ena_i : in std_logic;     -- enable '1' I don't know if we need it. 
    INTL_out: out std_logic   -- interlock output for signal that doesn't change for a given hold time 
 
);
end BLM_watchdog;

architecture rtl of BLM_watchdog is

type  watchdog_state_t is   (idle,check,interlock);
signal watchdog_state:   watchdog_state_t:= idle;

-- signal new_val_wait   : std_logic ;--:='0';
signal timeout_reset : unsigned(15 downto 0) ;
signal curr_val   : std_logic ; --:='0';
signal last_val   : std_logic ; --:='0';
signal timeout : unsigned(22 downto 0) ;
 
begin
  
  timeout_reset <= unsigned(hold);
 curr_val <= in_watchdog;


watchdog_proc: process (clk_i, rst_i)

  begin


      if (rst_i= '0') then -- or (ena_i='0')) or (wd_reset = '1') )  then
         INTL_out  <= '0';
         --new_val_wait   <= '0';
         timeout <= timeout_reset&"0000000";
         watchdog_state <= idle;
         
      elsif rising_edge(clk_i) then
        -- if ena_i ='1' then 

          last_val <= curr_val;

          case watchdog_state is

              when idle =>
                    
                   -- if ena_i ='1' then
                if curr_val ='1' and last_val='0' then
                  timeout <= timeout_reset&"0000000";
                else
                  timeout <= timeout -1;
                end if;
                if (to_integer(timeout )=0) then 
                  watchdog_state <= interlock;
                end if;

                when interlock => 
                
                   INTL_out <= '1';
                   
                   if wd_reset ='1' then
                     
                      watchdog_state <= check;
                  
                  end if;

                   

               when check =>
                    if (wd_reset = '0') then
                      INTL_out <= '0';
                      timeout <= timeout_reset&"0000000";
                      watchdog_state <= idle;
                   end if;
                   
                when others => null;
               end case;
           -- end if;
         end if;
         end process;
         	    
 end rtl;          		 
