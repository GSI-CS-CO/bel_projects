library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

package arria10_scu4_lvds_ibuf_pkg is
	component arria10_scu4_lvds_ibuf_altera_gpio_181_zbulp3i is
		port (
			dout     : out std_logic_vector(0 downto 0);                    -- export
			pad_in   : in  std_logic_vector(0 downto 0) := (others => 'X'); -- export
			pad_in_b : in  std_logic_vector(0 downto 0) := (others => 'X')  -- export
		);
	end component arria10_scu4_lvds_ibuf_altera_gpio_181_zbulp3i;

end arria10_scu4_lvds_ibuf_pkg;
