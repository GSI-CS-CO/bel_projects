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

constant c_oled_display : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"000000000003ffff",
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"93a6f3c4",
    version       => x"00000001",
    date          => x"20130130",
    name          => "OLED_Display       ")));



end oled_display_pkg;