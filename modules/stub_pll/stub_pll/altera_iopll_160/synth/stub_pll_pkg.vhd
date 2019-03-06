library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

package stub_pll_pkg is
	component stub_pll_altera_iopll_160_z2kwsvq is
		port (
			rst      : in  std_logic := 'X'; -- reset
			refclk   : in  std_logic := 'X'; -- clk
			locked   : out std_logic;        -- export
			outclk_0 : out std_logic;        -- clk
			outclk_1 : out std_logic;        -- clk
			outclk_2 : out std_logic;        -- clk
			outclk_3 : out std_logic         -- clk
		);
	end component stub_pll_altera_iopll_160_z2kwsvq;

end stub_pll_pkg;
