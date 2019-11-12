library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

package arria10_scu4_lvds_pll_pkg is
	component arria10_scu4_lvds_pll_altera_iopll_181_cn6etwi is
		port (
			rst      : in  std_logic                    := 'X'; -- reset
			refclk   : in  std_logic                    := 'X'; -- clk
			locked   : out std_logic;                           -- export
			lvds_clk : out std_logic_vector(1 downto 0);        -- lvds_clk
			loaden   : out std_logic_vector(1 downto 0);        -- loaden
			outclk_2 : out std_logic                            -- clk
		);
	end component arria10_scu4_lvds_pll_altera_iopll_181_cn6etwi;

end arria10_scu4_lvds_pll_pkg;
