LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
 

entity up_down_counter is
    generic (
    	c            : integer range 0 to 6:=1;        -- Counter_input width
        WIDTH        : integer := 20;      -- Counter width
        pos_threshold: integer:= 262144;
        neg_threshold: integer:= -262144
        
    );
    port (
        CLK         : in std_logic;      -- Clock
        nRST         : in std_logic;      -- Reset
        CLEAR       : in std_logic;      -- Clear counter register
        LOAD        : in std_logic;      -- Load counter register
        ENABLE      : in std_logic;      -- Enable count operation
        UP_IN       : in std_logic_vector(c-1 downto 0);    -- Load counter register up input
        DOWN_IN     : in std_logic_vector(c-1 downto 0);    -- Load counter register down input
        UP_OVERFLOW    : out std_logic ;     -- UP_Counter overflow
        DOWN_OVERFLOW    : out std_logic      -- UP_Counter overflow
    
    );
end up_down_counter;

architecture rtl of up_down_counter is
    signal up_Counter  : signed(WIDTH downto 0);         -- up Counter register
    signal down_Counter: signed(WIDTH downto 0);       -- down Counter register
    signal UP_D        : std_logic_vector(WIDTH-1 downto 0);    -- Load counter register up input
    signal DOWN_D      : std_logic_vector(WIDTH-1 downto 0);    -- Load counter register down input
begin
    -- Counter process
    COUNT_SHIFT: process (nRST, CLK)
    begin
        if (nRST = '0') then
        
            up_Counter <= (others => '0');                -- Reset up_counter register
            down_Counter <= (others => '0');              -- Reset down_counter register
            UP_OVERFLOW <= '0';
            DOWN_OVERFLOW <= '0';
            
        elsif (CLK'event and CLK='1') then
        
            if (CLEAR = '1') then
            
                up_Counter <= (others => '0');            -- Clear up_counter register
                down_Counter <= (others => '0');          -- Clear down_counter register
                
            elsif (LOAD = '1') then   
            
             -- Load up counter register
                UP_D((WIDTH-1) downto c)<= (others =>'0');
                UP_D((c-1) downto 0)<= UP_IN;
                up_Counter <= signed('0' & UP_D);
                
             -- Load down counter register
                DOWN_D((WIDTH-1) downto c)<= (others =>'0');
                DOWN_D((c-1) downto 0)<= DOWN_IN;
                down_Counter <= signed ('1' & DOWN_D);
                
            elsif ( ENABLE = '1') then                   -- Enable counter
            
                    up_Counter <= up_Counter + 1;     -- Count up                    
                    down_Counter <= down_Counter - 1; -- Count down
      
            end if;
            
            if (up_Counter = to_signed(pos_threshold, WIDTH-1)) then             -- pos_threshold reached
            
                UP_OVERFLOW <='1';
                up_Counter(WIDTH) <= '0';
                
            end if;
            
             if (down_Counter = to_signed(neg_threshold,WIDTH-1)) then         -- neg_threshold reached
             
                DOWN_OVERFLOW <='1';
                down_Counter(WIDTH) <= '0';                          -- clear down_counter register
                
            end if;
            
        end if;
 end process;

end rtl;

