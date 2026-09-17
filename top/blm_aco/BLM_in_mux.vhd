LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use work.scu_diob_pkg.all;
 
entity BLM_in_mux is
    port (
        CLK               : in std_logic;      -- Clock
        nRST              : in std_logic;      -- Reset
        mux_sel           : in std_logic_vector(11 downto 0);
        in_mux            : in std_logic_vector(63 downto 0);
        cnt_up            : out std_logic;     -- UP_Counter input 
        cnt_down          : out std_logic    -- DOWN_Counter input 
    );
end BLM_in_mux;

architecture rtl of BLM_in_mux is
signal up_sig_sel: integer range 0 to 63;
signal down_sig_sel: integer range 0 to 63;
signal out_mux: std_logic;
signal up, down : std_logic;
 

    begin
      
        up_down_signals_to_counter_proc: process (clk,nRST)
        begin
            if not nRST='1' then 
          
             up_sig_sel  <= 0;
             down_sig_sel  <= 0;
             up   <='0';
             down <='0';
              
       elsif (clk'EVENT AND clk= '1') then 
           
            down_sig_sel <= to_integer(unsigned (mux_sel(5 downto 0)));
            up_sig_sel <= to_integer(unsigned (mux_sel(11 downto 6)));

               for i in 0 to  63 loop
                if up_sig_sel = i then cnt_up<= in_mux(i);
                end if;
               end loop;

                for i in 0 to  63 loop
                   if down_sig_sel= i then cnt_down <= in_mux(i);
                    end if;
                end loop;
          
        end if;
    end process;
end architecture;
