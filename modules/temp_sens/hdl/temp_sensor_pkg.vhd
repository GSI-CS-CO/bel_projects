library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package temp_sensor_pkg is


component temp_sens is
                port (
                        clk        : in  std_logic                    := 'X'; -- clk
                        tsdcalo    : out std_logic_vector(7 downto 0);        -- tsdcalo
                        tsdcaldone : out std_logic;                           -- tsdcaldone
                        ce         : in  std_logic                    := 'X'; -- ce
                        clr        : in  std_logic                    := 'X'  -- reset
                );

end component;

end temp_sensor_pkg;

