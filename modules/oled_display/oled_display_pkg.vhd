library IEEE;
use IEEE.STD_LOGIC_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;

package oled_display_pkg is

component display_console is
	port	(
			
			clk_i				: in  std_logic;
			nRst_i   : in std_logic;
			
			slave_i     : in  t_wishbone_slave_in;
			slave_o     : out t_wishbone_slave_out;
    --
			RST_DISP_o			: out std_logic;										-- Display Reset on AI0/pin AB3
			DC_SPI_o				: out  std_logic;
			SS_SPI_o				: out  std_logic;
			SCK_SPI_o				: out  std_logic;
			SD_SPI_o				: out std_logic;
--
			SH_VR_o				: out  std_logic
			);
end component display_console;

end oled_display_pkg;