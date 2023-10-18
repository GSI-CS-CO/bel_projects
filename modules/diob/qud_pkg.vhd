library ieee;  
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library work;

package qud_pkg is

    TYPE    t_qud_mask     is array (0 to 23)  of std_logic_vector(53 downto 0);
    TYPE    t_qud_cnt     is array (0 to 23)  of integer range 0 to 53;
end qud_pkg;