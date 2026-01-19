-- Auto-generated VHDL package from blackbox_config.vh
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package blackbox_config_pkg is
	constant BB_VERSION_MAJOR : integer := 0;
	constant BB_VERSION_MINOR : integer := 1;
	constant BB_ADDR_BUS_WIDTH : integer := 16;
	constant BB_DATA_BUS_WIDTH : integer := 16;
	constant BB_NR_DIOB_IOS : integer := 127;
	constant BB_NR_VIRT_IOS : integer := 128;
	constant BB_NR_BACKPLANE_IOS : integer := 16;
	constant BB_NR_IRQ_LINES : integer := 16;
	constant BB_MAX_FRONTEND_PLUGINS : integer := 256;
	constant BB_MAX_PROC_PLUGINS : integer := 16;
	constant BB_MAX_USER_PLUGINS : integer := 256;
	constant BB_FRONTEND_STATUS_BITS : integer := 128;
	constant BB_CLOCK : integer := 125000000;
	constant BB_ID_config : integer := 0;
	constant BB_ID_frontend_default : integer := 255;
	constant BB_ID_proc_default : integer := 0;
	constant BB_ID_user_default : integer := 0;

	-- Note: macros need manual translation (e.g., `slice`)
end package blackbox_config_pkg;

package body blackbox_config_pkg is
end package body blackbox_config_pkg;
