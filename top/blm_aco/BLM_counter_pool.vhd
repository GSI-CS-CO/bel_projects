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
        LOAD              : in std_logic;      -- Load counter register
        ENABLE            : in std_logic_vector(9 downto 0);      -- Enable count operation
        pos_threshold     : in std_logic_vector(31 downto 0);
        neg_threshold     : in std_logic_vector(31 downto 0);
        in_counter        : in t_in_array;
        test_in_counter   : in std_logic_vector(8 downto 0); 
        UP_OVERFLOW       : out std_logic_vector(255 downto 0) ;     -- UP_Counter overflow for the input signals
        DOWN_OVERFLOW     : out std_logic_vector(255 downto 0)    -- DOWN_Counter overflow for the input signals

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
    

    component BLM_value_in_ram IS
    port(
            clock		: IN STD_LOGIC  := '1';
            data		: IN STD_LOGIC_VECTOR (7 DOWNTO 0);
            rdaddress		: IN STD_LOGIC_VECTOR (7 DOWNTO 0);
            rden		: IN STD_LOGIC  := '1';
            wraddress		: IN STD_LOGIC_VECTOR (5 DOWNTO 0);
            wren		: IN STD_LOGIC  := '0';
            q		: OUT STD_LOGIC_VECTOR (1 DOWNTO 0)
        );

        end component BLM_value_in_ram;


signal VALUE_IN : std_logic_vector(63 downto 0);
signal m: integer range 0 to 7 :=0;
signal word_in: std_logic_vector(5 downto 0);
TYPE    t_counter_in_Array    is array (0 to 255)  of std_logic_vector(1 downto 0);
signal counter_in:  t_counter_in_Array;
signal q_i: integer range 0 to 255;


begin
   q_i <= to_integer(signed(ENABLE(7 downto 0)));
    word_in_process: process (nRST, CLK)  
   
    begin
       if not nRST='1' then 
       
           VALUE_IN<= (OTHERS =>'0');
           word_in <= (OTHERS =>'0');
       
       elsif (CLK'EVENT AND CLK = '1') then
            VALUE_IN   <= test_in_counter & '0' & in_counter(8) & in_counter(7) & in_counter(6) & in_counter(5) & in_counter(4) & in_counter(3) & in_counter(2) & in_counter(1) & in_counter(0); 
            --ground and Test signals are sent to the counter pool together with the input signals
            
            if ENABLE(8) ='1' then 
           m <=0;

        elsif ENABLE(9) ='1' then
          m <= m + 1;
          if m = 7 then      
            m <= 0;
          end if;

        end if;

        word_in <= std_logic_vector(to_unsigned(m,6));
            
     end if;

    end process;

       input_to_up_down_Counter:     BLM_value_in_ram
            PORT MAP
            (
                clock		=>CLK,
                data		=> VALUE_IN((8*m +7) downto (8*m)),
                
                rdaddress	=> ENABLE(7 downto 0),
                rden        => ENABLE(8),
                wraddress	=> word_in,
                wren		=> ENABLE(9),
               q		    => counter_in(q_i)
                
            );
    

 
  counter_pool:for i in 0 to 255 generate 

  begin

    Counter_module: up_down_counter 
     generic map
        (   
            WIDTH         => WIDTH   -- Counter width
                     
        )
    port map
        (   CLK           => clk,    -- Clock
            nRST          => nRST,      -- Reset
            CLEAR         => CLEAR,     -- Clear counter register
            LOAD          => LOAD,      -- Load counter register
            ENABLE        => ENABLE(8),   -- Enable count operation
            pos_threshold => to_integer(signed(pos_threshold)),
            neg_threshold => to_integer(signed(neg_threshold)),
            UP_IN         => counter_in(i)(1),   -- Load counter register up input
            DOWN_IN       => counter_in(i)(0),  -- Load counter register down input
            UP_OVERFLOW   => UP_OVERFLOW(i),    -- UP_Counter overflow 
            DOWN_OVERFLOW => DOWN_OVERFLOW(i)   -- UP_Counter overflow 
        );

end generate counter_pool;

end rtl;