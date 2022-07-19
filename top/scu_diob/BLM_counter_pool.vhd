LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
 
entity BLM_counter_pool is
    generic (
    	n            : integer range 0 to 62 :=62;        -- counter pool inputs:  hardware inputs plus one test signal
        WIDTH        : integer := 20;      -- Counter width
        pos_threshold: integer:= 262144;
        neg_threshold: integer:= -262144       
    );
    port (
        CLK         : in std_logic;      -- Clock
        nRST         : in std_logic;      -- Reset
        CLEAR       : in std_logic;      -- Clear counter register
        LOAD        : in std_logic_vector(n-2 downto 0);      -- Load counter register
        ENABLE      : in std_logic_vector(n-2 downto 0);      -- Enable count operation
        UP_IN       : in std_logic_vector(n-2 downto 0);    -- Load counter register up input
        DOWN_IN     : in std_logic_vector(n-2 downto 0);    -- Load counter register down input
        UP_OVERFLOW    : out std_logic_vector((n-8)*(n-2) -1 downto 0) ;     -- UP_Counter overflow
        DOWN_OVERFLOW    : out std_logic_vector((n-8)*(n-2)-1 downto 0)      -- UP_Counter overflow
    );
end BLM_counter_pool;

architecture rtl of BLM_counter_pool is

    component up_down_counter is
        generic (
          --  c            : integer :=1;        -- Counter_input width
            WIDTH        : integer := 20;      -- Counter width
            pos_threshold: integer:= 262144;
            neg_threshold: integer:= -262144
        );
        port (
    --        CLK         : in std_logic;      -- Clock
    --        nRST         : in std_logic;      -- Reset
     --       CLEAR       : in std_logic;      -- Clear counter register
      --      LOAD        : in std_logic;      -- Load counter register
      --      ENABLE      : in std_logic;      -- Enable count operation
     --       UP_IN       : in std_logic_vector(c-1 downto 0);    -- Load counter register up input
     --       DOWN_IN     : in std_logic_vector(c-1 downto 0);    -- Load counter register down input
      --      UP_OVERFLOW    : out std_logic ;     -- UP_Counter overflow
      --      DOWN_OVERFLOW    : out std_logic      -- UP_Counter overflow
     --   );
        CLK         : in std_logic;      -- Clock
        nRST         : in std_logic;      -- Reset
        CLEAR       : in std_logic;      -- Clear counter register
        LOAD        : in std_logic;      -- Load counter register
        ENABLE      : in std_logic;      -- Enable count operation
        UP_IN       : in std_logic;    -- Load counter register up input
        DOWN_IN     : in std_logic;    -- Load counter register down input
        UP_OVERFLOW    : out std_logic ;     -- UP_Counter overflow
        DOWN_OVERFLOW    : out std_logic      -- UP_Counter overflow
        );
    end component up_down_counter; 

begin

    -- Counter process
 --   pool_proc: process (nRST, CLK)  -- i and i+1 (i=0..n-2) tot 3240 counters 60x54
    
  --      begin
  --          if not nRST='1' then 

   --         Int_event_counter <=  (OTHERS => (OTHERS => '0')); 
    --        UP_OVERFLOW <= (OTHERS =>'0');
    --        DOWN_OVERFLOW <= (OTHERS =>'0');
     --   elsif (CLK'EVENT AND CLK = '1') then
            
            counter_pool:for i in 1 to (n-8) generate --only inputs from the 9 input cards counter_pool: 

                --begin
          
                    gen_count_pool: for j in 0 to (n-2-i) generate --n take accounts of the inputs and the test signals             
                        begin

                            Counter_module: up_down_counter 
                                generic map
                                (   --c             => 1,       -- Counter_input width
                                    WIDTH         => WIDTH,    -- Counter width
                                    pos_threshold => pos_threshold,
                                    neg_threshold => neg_threshold
                                )
                                port map
                                (   CLK           => clk,    -- Clock
                                    nRST          => nRST,      -- Reset
                                    CLEAR         => CLEAR,     -- Clear counter register
                                    LOAD          => LOAD(j),      -- Load counter register
                                    ENABLE        => ENABLE(j),   -- Enable count operation
                                    UP_IN         => UP_IN(j+ i),   -- Load counter register up input
                                    DOWN_IN       => DOWN_IN(j),  -- Load counter register down input
                                    UP_OVERFLOW   => UP_OVERFLOW(j+(i-1)*(n-1)) ,    -- UP_Counter overflow
                                    DOWN_OVERFLOW => DOWN_OVERFLOW(j+(i-1)*(n-1))   -- UP_Counter overflow tot 3940 bit
                                );
        
                    end generate gen_count_pool;
    
        end generate counter_pool;
  --  end if;
  --  end process;

end rtl;