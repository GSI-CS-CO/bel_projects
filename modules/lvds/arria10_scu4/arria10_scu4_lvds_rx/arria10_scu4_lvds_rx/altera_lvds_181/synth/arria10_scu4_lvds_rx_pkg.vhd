library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

package arria10_scu4_lvds_rx_pkg is
	component arria10_scu4_lvds_rx_altera_lvds_181_pparsma is
		port (
			rx_in         : in  std_logic_vector(0 downto 0) := (others => 'X'); -- export
			rx_out        : out std_logic_vector(7 downto 0);                    -- export
			ext_fclk      : in  std_logic                    := 'X';             -- export
			ext_loaden    : in  std_logic                    := 'X';             -- export
			ext_coreclock : in  std_logic                    := 'X'              -- export
		);
	end component arria10_scu4_lvds_rx_altera_lvds_181_pparsma;

end arria10_scu4_lvds_rx_pkg;
