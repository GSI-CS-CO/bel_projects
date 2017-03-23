-- libraries and packages
-- ieee
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- wishbone/gsi/cern
library work;
use work.wishbone_pkg.all;

-- package declaration
package wb_temp_sense_pkg is
  component wb_temp_sense
    generic (
      g_address_size  : natural := 32;  -- in bit(s)
      g_data_size     : natural := 32;  -- in bit(s)
      g_spi_data_size : natural := 8    -- in bit(s)
    );
    port (
      -- generic system interface
      clk_sys_i  : in  std_logic;
      rst_n_i    : in  std_logic;
      -- wishbone slave interface
      slave_i    : in  t_wishbone_slave_in;
      slave_o    : out t_wishbone_slave_out;
      clr_o	 : out std_logic);
  end component;

  constant c_temp_sense_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"4", -- 32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"00000000000000ff",
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"7E3D5E25",
    version       => x"00000001",
    date          => x"20160309",
    name          => "ALTERA_TEMP_SENSOR ")));

end wb_temp_sense_pkg;

