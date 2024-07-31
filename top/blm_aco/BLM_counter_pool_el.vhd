LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use work.scu_diob_pkg.all;
 
entity BLM_counter_pool_el is
    generic (      
        WIDTH        : integer := 32      -- Counter width
            
    );
    port (
        CLK               : in std_logic;      -- Clock
        nRST              : in std_logic;      -- Reset
        gate_reset_ena    : in std_logic;
        RESET             : in std_logic;      -- global reset
        ENABLE            : in std_logic;      -- Enable count operation (gate signals)
        pos_threshold     : in std_logic_vector(31 downto 0);
        neg_threshold     : in std_logic_vector(31 downto 0);
        in_counter        : in std_logic_vector(63 downto 0);
        BLM_cnt_Reg     : in std_logic_vector(15 downto 0);  --bit 11..0
        cnt    : out std_logic_vector (WIDTH-1 downto 0);    --  Counter register
        
        UP_OVERFLOW       : out std_logic;     -- UP_Counter overflow for the input signals
        DOWN_OVERFLOW     : out std_logic    -- DOWN_Counter overflow for the input signals

    );
end BLM_counter_pool_el;

architecture rtl of BLM_counter_pool_el is

signal cnt_enable: std_logic;
signal cnt_up, cnt_down: std_logic;

    component BLM_in_mux is
        port (
            CLK               : in std_logic;      -- Clock
            nRST              : in std_logic;      -- Reset
            mux_sel           : in std_logic_vector(11 downto 0);
            in_mux            : in std_logic_vector(63 downto 0);
            cnt_up            : out std_logic;     -- UP_Counter input 
            cnt_down          : out std_logic    -- DOWN_Counter input  
        );
    end component BLM_in_mux;

    component up_down_counter is
        generic (

            WIDTH        : integer := 32     -- Counter width
          
        );
        port (

        CLK              : in std_logic;      -- Clock
        nRST             : in std_logic;      -- Reset
        CLEAR            : in std_logic;      -- Clear counter registers

        ENABLE           : in std_logic;      -- Enable count operation
        pos_threshold    : in integer;
        neg_threshold    : in integer;
        UP_IN            : in std_logic;      -- Load counter register up input
        DOWN_IN          : in std_logic;      -- Load counter register down input
        cnt_val    : out std_logic_vector (WIDTH-1 downto 0);    -- Counter register
        
        UP_OVERFLOW      : out std_logic;     -- UP_Counter overflow
        DOWN_OVERFLOW    : out std_logic      -- UP_Counter overflow
        );
    end component up_down_counter; 
    

    
signal CLEAR: std_logic;

begin


 clear_and_cnt_enable_process: process (clk, nRST)
   begin
        if not nRST='1' then 
          
         
           cnt_enable <='0';
             CLEAR <='1';
              
        elsif (clk'EVENT AND clk= '1') then 

            if (RESET ='1' or gate_reset_ena = '1') then --counter reset

                cnt_enable <='0';
                CLEAR <='1';

            --elsif ENABLE = '1' then 

            else
                
                cnt_enable <= ENABLE;
          --      if  gate_reset_ena = '0' then
                    CLEAR <='0';   
          --      else 
          --          CLEAR <= ENABLE;
           --     end if;
                
            end if;
        end if;
end process;





    in_multiplexer: BLM_in_mux 
        port map(
            CLK           => clk,    -- Clock
            nRST          => nRST,      -- Reset
            mux_sel       =>  BLM_cnt_Reg(11 downto 0),
            in_mux        => in_counter,
            cnt_up        => cnt_up,   -- UP_Counter input 
            cnt_down      => cnt_down    -- DOWN_Counter input  
        );



    Counter_module: up_down_counter 
     generic map
        (   
            WIDTH         => 32 --WIDTH   -- Counter width
                     
        )
    port map
        (   CLK           => clk,    -- Clock
            nRST          => nRST,      -- Reset
            CLEAR         => CLEAR,     -- Clear counter register
            ENABLE        => cnt_enable,   -- Enable count operation
          
            pos_threshold => to_integer(signed(pos_threshold)),
            neg_threshold => to_integer(signed(neg_threshold)),
            UP_IN         => cnt_up,   -- Load counter register up input
            DOWN_IN       => cnt_down,  -- Load counter register down input
            cnt_val   => cnt,
            
            
            UP_OVERFLOW   => UP_OVERFLOW,    -- UP_Counter overflow 
            DOWN_OVERFLOW => DOWN_OVERFLOW   -- UP_Counter overflow 
        );


end rtl;
