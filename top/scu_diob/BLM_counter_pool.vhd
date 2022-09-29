LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use work.scu_diob_pkg.all;
 
entity BLM_counter_pool is
    generic (      
        WIDTH        : integer := 20      -- Counter width
            
    );
    port (
        CLK               : in std_logic;      -- Clock
        nRST              : in std_logic;      -- Reset
        CLEAR             : in std_logic;      -- Clear counter register
        LOAD              : in std_logic_vector(7 downto 0);      -- Load counter register
        ENABLE            : in std_logic_vector(7 downto 0);      -- Enable count operation
        pos_threshold     : in std_logic_vector(31 downto 0);
        neg_threshold     : in std_logic_vector(31 downto 0);
        VALUE_IN          : in std_logic_vector(63 downto 0);    -- Load counter register input
        GATE_OUT          : in std_logic_vector(11 downto 0);
        UP_OVERFLOW       : out t_counter_in_Array ;     -- UP_Counter overflow
        DOWN_OVERFLOW     : out t_counter_in_Array;      -- UP_Counter overflow
        gate_UP_OVERFLOW  : out t_gate_counter_in_Array;
        gate_DOWN_OVERFLOW: out t_gate_counter_in_Array
    );
end BLM_counter_pool;

architecture rtl of BLM_counter_pool is

    component up_down_counter is
        generic (

            WIDTH        : integer := 20      -- Counter width
          
        );
        port (

        CLK              : in std_logic;      -- Clock
        nRST             : in std_logic;      -- Reset
        CLEAR            : in std_logic;      -- Clear counter register
        LOAD             : in std_logic;      -- Load counter register
        ENABLE           : in std_logic;      -- Enable count operation
        pos_threshold    : in integer;
        neg_threshold    : in integer;
        UP_IN            : in std_logic;      -- Load counter register up input
        DOWN_IN          : in std_logic;      -- Load counter register down input
        UP_OVERFLOW      : out std_logic;     -- UP_Counter overflow
        DOWN_OVERFLOW    : out std_logic      -- UP_Counter overflow
        );
    end component up_down_counter; 
    

signal cnt_DOWN_IN: t_counter_in_Array;
signal cnt_UP_IN  : t_counter_in_Array;

signal gate_UP_IN        : t_gate_counter_in_Array;
signal gate_DOWN_IN      : t_gate_counter_in_Array;


begin

counter_input_process: process (LOAD)



 begin

  
 case LOAD(7 downto 0) is 
 when "00000001" => --
                    cnt_UP_IN(0)   <= VALUE_IN(53 downto 0) & VALUE_IN(63 downto 54);
                    cnt_DOWN_IN(0) <= VALUE_IN(52 downto 0) & VALUE_IN(53 downto 43); --the 10 MSB are tested with a choiced test signals

                    gate_UP_IN(0)  <= GATE_OUT;
                    gate_DOWN_IN(0) <= GATE_OUT(10 downto 0)& GATE_OUT(11);
                    
 when "00000010" =>  --
                    cnt_UP_IN(1)   <= VALUE_IN(53 downto 0) & VALUE_IN(63 downto 54);
                    cnt_DOWN_IN(1) <= VALUE_IN(51 downto 0) &  VALUE_IN(53 downto 52) & VALUE_IN(42 downto 33);

                    gate_UP_IN(1)  <= GATE_OUT;
                    gate_DOWN_IN(1) <= GATE_OUT(9 downto 0)& GATE_OUT(11 downto 10);

 when "00000100" =>  --
                    cnt_UP_IN(2)   <= VALUE_IN(53 downto 0) & VALUE_IN(63 downto 54);
                    cnt_DOWN_IN(2) <= VALUE_IN(50 downto 0) & VALUE_IN(53 downto 51) &VALUE_IN(32 downto 23);    

                    gate_UP_IN(2)  <= GATE_OUT;
                    gate_DOWN_IN(2) <= GATE_OUT(8 downto 0)& GATE_OUT(11 downto 9);

when "00001000" =>  --
                    cnt_UP_IN(3)   <= VALUE_IN(53 downto 0) & VALUE_IN(63 downto 54);
                    cnt_DOWN_IN(3) <= VALUE_IN(49 downto 0) & VALUE_IN(53 downto 50) &VALUE_IN(22 downto 13);   

                    gate_UP_IN(3)  <= GATE_OUT;
                    gate_DOWN_IN(3) <= GATE_OUT(7 downto 0)& GATE_OUT(11 downto 8);
                   
when "00010000" =>  --
                    cnt_UP_IN(4)   <= VALUE_IN(53 downto 0) & VALUE_IN(63 downto 54);
                    cnt_DOWN_IN(4) <= VALUE_IN(48 downto 0) & VALUE_IN(53 downto 49) &VALUE_IN(12 downto 3); 

                    gate_UP_IN(4)  <= GATE_OUT;
                    gate_DOWN_IN(4) <= GATE_OUT(6 downto 0)& GATE_OUT(11 downto 7);

when "00100000" =>  --
                    cnt_UP_IN(5)   <= VALUE_IN(53 downto 0) & VALUE_IN(63 downto 54);
                    cnt_DOWN_IN(5) <= VALUE_IN(47 downto 0) & VALUE_IN(53 downto 48) & (VALUE_IN(2 downto 0)& VALUE_IN(53 downto 47)); 

                    gate_UP_IN(5)  <= GATE_OUT;
                    gate_DOWN_IN(5) <= GATE_OUT(5 downto 0)& GATE_OUT(11 downto 6);
                    
when "01000000" =>  --
                    cnt_UP_IN(6)   <= VALUE_IN(53 downto 0) & VALUE_IN(63 downto 54);
                    cnt_DOWN_IN(6) <= VALUE_IN(46 downto 0) & VALUE_IN(53 downto 47)& VALUE_IN(46 downto 37); 
                    
when "10000000" =>  --
                    cnt_UP_IN(7)   <= VALUE_IN(53 downto 0) & VALUE_IN(63 downto 54);
                    cnt_DOWN_IN(7) <= VALUE_IN(45 downto 0) & VALUE_IN(53 downto 46) & VALUE_IN(36 downto 27);

when others => NULL;
 end case;
 end process;


 
  counter_pool:for i in 0 to 7 generate 

  begin
    gen_count_pool: for j in 0 to 63 generate

    Counter_module: up_down_counter 
     generic map
        (   
            WIDTH         => WIDTH   -- Counter width
                     
        )
    port map
        (   CLK           => clk,    -- Clock
            nRST          => nRST,      -- Reset
            CLEAR         => CLEAR,     -- Clear counter register
            LOAD          => LOAD(i),      -- Load counter register
            ENABLE        => ENABLE(i),   -- Enable count operation
            pos_threshold => to_integer(signed(pos_threshold)),
            neg_threshold => to_integer(signed(neg_threshold)),
            UP_IN         => cnt_UP_IN(i)(j),   -- Load counter register up input
            DOWN_IN       => cnt_DOWN_IN(i)(j),  -- Load counter register down input
            UP_OVERFLOW   => UP_OVERFLOW(i)(j) ,    -- UP_Counter overflow 
            DOWN_OVERFLOW => DOWN_OVERFLOW(i)(j)   -- UP_Counter overflow 
        );

      end generate gen_count_pool;

end generate counter_pool;

gate_counter_pool:for i in 0 to 5 generate 

begin
  gate_gen_count_pool: for j in 0 to 11 generate

  gen_Counter_module: up_down_counter 
   generic map
      (   
          WIDTH         => WIDTH   -- Counter width
                   
      )
  port map
      (   CLK           => clk,    -- Clock
          nRST          => nRST,      -- Reset
          CLEAR         => CLEAR,     -- Clear counter register
          LOAD          => LOAD(i),      -- Load counter register
          ENABLE        => ENABLE(i),   -- Enable count operation
          pos_threshold => to_integer(signed(pos_threshold)),
          neg_threshold => to_integer(signed(neg_threshold)),
          UP_IN         => gate_UP_IN(i)(j),   -- Load counter register up input
          DOWN_IN       => gate_DOWN_IN(i)(j),  -- Load counter register down input
          UP_OVERFLOW   => gate_UP_OVERFLOW(i)(j) ,    -- UP_Counter overflow 
          DOWN_OVERFLOW => gate_DOWN_OVERFLOW(i)(j)   -- UP_Counter overflow 
      );

    end generate gate_gen_count_pool;

end generate gate_counter_pool;
end rtl;