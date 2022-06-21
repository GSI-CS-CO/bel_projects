LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
 

entity up_down_counter is
    generic (
    	n            : integer :=6;        -- Counter_input width
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
        UP_IN       : in std_logic_vector(n-1 downto 0);    -- Load counter register up input
        DOWN_IN     : in std_logic_vector(n-1 downto 0);    -- Load counter register down input
        UP_OVERFLOW    : out std_logic ;     -- UP_Counter overflow
        DOWN_OVERFLOW    : out std_logic      -- UP_Counter overflow
    
    );
end up_down_counter;

