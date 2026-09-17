LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
 

entity up_down_counter is
    generic (
    	--c            : integer :=1;        -- Counter_input width
        WIDTH        : integer := 32      -- Counter width

        
    );
    port (
        CLK         : in std_logic;      -- Clock
        nRST         : in std_logic;      -- Reset
        CLEAR       : in std_logic;      -- Clear counter register
        ENABLE      : in std_logic;      -- Enable count operation
        UP_IN       : in std_logic;    -- Load counter register up input
        DOWN_IN     : in std_logic;    -- Load counter register down input
        pos_threshold: in integer;
        neg_threshold: in integer;
        cnt_val    : out std_logic_vector(WIDTH-1 downto 0);    --Counter register

        UP_OVERFLOW    : out std_logic ;     -- UP_Counter overflow
        DOWN_OVERFLOW    : out std_logic      -- UP_Counter overflow
    
    );
end up_down_counter;

architecture rtl of up_down_counter is

 
  signal int_count: integer:=0;
  
--  signal int_n_count,int_p_count: integer:=0;
    signal up_OVERFLOW_FLAG: std_logic;
    signal  down_OVERFLOW_FLAG:std_logic;
signal up_input, down_input: std_logic;
signal up_sig, down_sig: std_logic;
signal res_cnt : integer :=0;

    component BLM_ct_pulse_former is

        port (
            CLK         : in std_logic;      -- Clock
            nRST         : in std_logic;      -- Reset
            SIG_IN       : in std_logic;    -- Load counter register up input
            SIG_OUT    : out std_logic     -- UP_Counter overflow
        
        );
        end component BLM_ct_pulse_former;
begin



    Pulse_enable_UP: BLM_ct_pulse_former 

        port map(
            CLK       => CLK,    -- Clock
            nRST       => nRST,      -- Reset
            SIG_IN    => up_sig,
            SIG_OUT   => up_input
        );

    Pulse_enable_DOWN: BLM_ct_pulse_former 

        port map(
            CLK       => CLK,    -- Clock
            nRST       => nRST,      -- Reset
            SIG_IN    => down_sig,
            SIG_OUT   => DOWN_input
        );


    -- Counter process
    COUNT_SHIFT: process (nRST, CLK)
    begin
        if (nRST = '0') then    

        
            up_OVERFLOW_FLAG <='0';
            down_OVERFLOW_FLAG <='0';
            int_count <=0;
            res_cnt <= 0;
            
        elsif (CLK'event and CLK='1') then
            res_cnt <= int_count;
        
            if (CLEAR = '1') then

                int_count <= 0;
       
                up_OVERFLOW_FLAG <='0';
                down_OVERFLOW_FLAG <='0';

            elsif ( ENABLE = '1') then   
       
----------------------------
            if (up_input ='1') and (down_input ='0') then 

                int_count <= int_count +1;

            elsif (down_input ='1') and (up_input ='0') then
    
                int_count <= int_count -1;

            end if;
    
        end if;

-- comparation to threshold out from the enable ='1' condition

            if res_cnt > pos_threshold then         -- pos_threshold reached

                up_OVERFLOW_FLAG <='1';

            elsif res_cnt < neg_threshold then  -- neg_threshold reached

                down_OVERFLOW_FLAG <='1';                 

            end if;

      --  end if;
    end if;
end process;
-----------------------------
UP_OVERFLOW <= up_OVERFLOW_FLAG;
DOWN_OVERFLOW <= down_OVERFLOW_FLAG;
cnt_val <=  std_logic_vector(to_signed(int_count, WIDTH));
up_sig <= UP_IN;
down_sig <= DOWN_IN;

end rtl;


