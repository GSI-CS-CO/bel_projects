library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

package dmtd_pll10_pkg is
	component dmtd_pll10_altera_iopll_160_fd67bvy is
		port (
			rst      : in  std_logic := 'X'; -- reset
			refclk   : in  std_logic := 'X'; -- clk
			locked   : out std_logic;        -- export
			outclk_0 : out std_logic         -- clk
		);
	end component dmtd_pll10_altera_iopll_160_fd67bvy;

end dmtd_pll10_pkg;
