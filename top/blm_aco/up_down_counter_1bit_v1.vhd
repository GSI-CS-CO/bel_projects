LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
 

entity up_down_counter is
    generic (
    	--c            : integer :=1;        -- Counter_input width
        WIDTH        : integer := 20      -- Counter width

        
    );
    port (
        CLK         : in std_logic;      -- Clock
        nRST         : in std_logic;      -- Reset
        CLEAR       : in std_logic;      -- Clear counter register
    --    LOAD        : in std_logic;      -- Load counter register
        ENABLE      : in std_logic;      -- Enable count operation
        UP_IN       : in std_logic;    -- Load counter register up input
        DOWN_IN     : in std_logic;    -- Load counter register down input
        pos_threshold: in integer;
        neg_threshold: in integer;
        UP_OVERFLOW    : out std_logic ;     -- UP_Counter overflow
        DOWN_OVERFLOW    : out std_logic      -- UP_Counter overflow
    
    );
end up_down_counter;

architecture rtl of up_down_counter is
    signal up_Counter  : signed(WIDTH-1 downto 0);         -- up Counter register
    signal down_Counter: signed(WIDTH-1 downto 0);       -- down Counter register
    signal UP_D        : std_logic_vector(WIDTH-1 downto 0) :=(others =>'0'); 
    signal DOWN_D      : std_logic_vector(WIDTH-1 downto 0) :=(others =>'0');
    signal up_cnt: integer range 0 to (WIDTH -1);
    signal down_cnt: integer range 0 to (WIDTH -1):=19;
    signal up_OVERFLOW_FLAG: std_logic:='0';
    signal  down_OVERFLOW_FLAG:std_logic:='0';
begin
    -- Counter process
    COUNT_SHIFT: process (nRST, CLK)
    begin
        if (nRST = '0') then
        
            up_Counter <= (others => '0');                -- Reset up_counter register
            down_Counter <= (others => '0');              -- Reset down_counter register
            up_OVERFLOW_FLAG <='0';
            down_OVERFLOW_FLAG <='0';
            UP_D <= (others => '0');
            DOWN_D <= (others => '0');
            up_cnt<= 0;
            down_Cnt <= WIDTH-1;
            
        elsif (CLK'event and CLK='1') then
        
            if (CLEAR = '1') then
            
                up_Counter <= (others => '0');            -- Clear up_counter register
                down_Counter <= (others => '0');          -- Clear down_counter register
                up_cnt <= 0;
                down_cnt <= WIDTH-1;
                UP_D <= (others => '0');
                DOWN_D <= (others => '0');
                up_OVERFLOW_FLAG <='0';
                down_OVERFLOW_FLAG <='0';

            elsif ( ENABLE = '1') then   
            up_cnt <= 0;
            UP_D(0) <= UP_IN;
            DOWN_D(0) <= DOWN_IN; 
            down_cnt <= WIDTH-1;
                            
            if up_cnt < WIDTH -1 then
                 
               for i in 1 to (WIDTH-1) loop     
                
                    UP_D(0) <= UP_IN;
                    UP_D(i)<=UP_D(i-1);
                end loop;

                up_cnt <= up_cnt +1; 
            end if;

            
            if down_cnt >0 then 
                       

                for j in 1 to (WIDTH-1) loop                                
 
                  --  DOWN_D(0) <= DOWN_IN; 
                    DOWN_D(j)  <= DOWN_D(j-1); 
                    
                     
                end loop;

                    down_cnt <= down_cnt -1;             
           
                    
             end if;
                        up_Counter <= signed(UP_D);

   
                        down_Counter <= signed (DOWN_D);
                        
            
            
                        if (up_Counter = to_signed(pos_threshold, WIDTH)) then             -- pos_threshold reached
            
                          up_OVERFLOW_FLAG <='1';
             
                      
                        end if;
            
                        if (down_Counter = to_signed(neg_threshold,WIDTH)) then         -- neg_threshold reached
             
                          down_OVERFLOW_FLAG <='1';
               
                       
                
                       end if;
            
            end if;
        end if;
 end process;
UP_OVERFLOW <= up_OVERFLOW_FLAG;
DOWN_OVERFLOW <= down_OVERFLOW_FLAG;

end rtl;


