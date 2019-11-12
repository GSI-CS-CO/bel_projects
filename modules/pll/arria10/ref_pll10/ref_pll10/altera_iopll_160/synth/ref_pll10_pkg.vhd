library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

package ref_pll10_pkg is
	component ref_pll10_altera_iopll_160_d3szf3y is
		port (
			rst              : in  std_logic                    := 'X';             -- reset
			refclk           : in  std_logic                    := 'X';             -- clk
			locked           : out std_logic;                                       -- export
			scanclk          : in  std_logic                    := 'X';             -- scanclk
			phase_en         : in  std_logic                    := 'X';             -- phase_en
			updn             : in  std_logic                    := 'X';             -- updn
			cntsel           : in  std_logic_vector(4 downto 0) := (others => 'X'); -- cntsel
			phase_done       : out std_logic;                                       -- phase_done
			num_phase_shifts : in  std_logic_vector(2 downto 0) := (others => 'X'); -- num_phase_shifts
			outclk_0         : out std_logic;                                       -- clk
			outclk_1         : out std_logic;                                       -- clk
			outclk_2         : out std_logic;                                       -- clk
			outclk_3         : out std_logic;                                       -- clk
			outclk_4         : out std_logic                                        -- clk
		);
	end component ref_pll10_altera_iopll_160_d3szf3y;

end ref_pll10_pkg;
