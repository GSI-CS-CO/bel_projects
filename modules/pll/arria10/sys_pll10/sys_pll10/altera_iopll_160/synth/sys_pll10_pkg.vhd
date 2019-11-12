library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

package sys_pll10_pkg is
	component sys_pll10_altera_iopll_160_pcex3oq is
		port (
			rst      : in  std_logic := 'X'; -- reset
			refclk   : in  std_logic := 'X'; -- clk
			locked   : out std_logic;        -- export
			outclk_0 : out std_logic;        -- clk
			outclk_1 : out std_logic;        -- clk
			outclk_2 : out std_logic;        -- clk
			outclk_3 : out std_logic;        -- clk
			outclk_4 : out std_logic         -- clk
		);
	end component sys_pll10_altera_iopll_160_pcex3oq;

end sys_pll10_pkg;
