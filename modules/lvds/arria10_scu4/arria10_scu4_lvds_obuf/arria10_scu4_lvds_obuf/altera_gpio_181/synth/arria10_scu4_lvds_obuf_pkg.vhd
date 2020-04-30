library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

package arria10_scu4_lvds_obuf_pkg is
	component arria10_scu4_lvds_obuf_altera_gpio_181_7mecp6a is
		port (
			din       : in  std_logic_vector(0 downto 0) := (others => 'X'); -- export
			pad_out   : out std_logic_vector(0 downto 0);                    -- export
			pad_out_b : out std_logic_vector(0 downto 0)                     -- export
		);
	end component arria10_scu4_lvds_obuf_altera_gpio_181_7mecp6a;

end arria10_scu4_lvds_obuf_pkg;
