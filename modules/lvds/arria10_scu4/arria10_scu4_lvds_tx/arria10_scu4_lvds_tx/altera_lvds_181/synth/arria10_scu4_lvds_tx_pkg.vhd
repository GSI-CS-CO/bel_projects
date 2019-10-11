library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

package arria10_scu4_lvds_tx_pkg is
	component arria10_scu4_lvds_tx_altera_lvds_181_q6uosdy is
		port (
			tx_in         : in  std_logic_vector(7 downto 0) := (others => 'X'); -- export
			tx_out        : out std_logic_vector(0 downto 0);                    -- export
			tx_coreclock  : out std_logic;                                       -- export
			ext_fclk      : in  std_logic                    := 'X';             -- export
			ext_loaden    : in  std_logic                    := 'X';             -- export
			ext_coreclock : in  std_logic                    := 'X'              -- export
		);
	end component arria10_scu4_lvds_tx_altera_lvds_181_q6uosdy;

end arria10_scu4_lvds_tx_pkg;
