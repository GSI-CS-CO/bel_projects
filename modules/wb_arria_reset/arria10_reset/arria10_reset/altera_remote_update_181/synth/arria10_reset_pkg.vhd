library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

package arria10_reset_pkg is
	component arria10_reset_altera_remote_update_181_2d3hvga is
		port (
			busy        : out std_logic;                                        -- busy
			data_out    : out std_logic_vector(31 downto 0);                    -- data_out
			param       : in  std_logic_vector(2 downto 0)  := (others => 'X'); -- param
			read_param  : in  std_logic                     := 'X';             -- read_param
			reconfig    : in  std_logic                     := 'X';             -- reconfig
			reset_timer : in  std_logic                     := 'X';             -- reset_timer
			ctl_nupdt   : in  std_logic                     := 'X';             -- ctl_nupdt
			clock       : in  std_logic                     := 'X';             -- clk
			reset       : in  std_logic                     := 'X'              -- reset
		);
	end component arria10_reset_altera_remote_update_181_2d3hvga;

end arria10_reset_pkg;
