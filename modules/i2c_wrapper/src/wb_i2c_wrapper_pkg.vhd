-- libraries and packages
-- ieee
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- wishbone/gsi/cern
library work;
use work.wishbone_pkg.all;

-- package declaration
package wb_i2c_wrapper_pkg is
  --component xwb_i2c_master
  --  generic (
  --    g_interface_mode      : t_wishbone_interface_mode      := CLASSIC;
  --    g_address_granularity : t_wishbone_address_granularity := WORD;
  --    g_num_interfaces      : integer := 1);
  --  port (
  --    clk_sys_i    : in std_logic;
  --    rst_n_i      : in std_logic;
  --    slave_i      : in  t_wishbone_slave_in;
  --    slave_o      : out t_wishbone_slave_out;
  --    desc_o       : out t_wishbone_device_descriptor;
  --    int_o        : out std_logic;
  --    scl_pad_i    : in  std_logic_vector(g_num_interfaces-1 downto 0);
  --    scl_pad_o    : out std_logic_vector(g_num_interfaces-1 downto 0);
  --    scl_padoen_o : out std_logic_vector(g_num_interfaces-1 downto 0);
  --    sda_pad_i    : in  std_logic_vector(g_num_interfaces-1 downto 0);
  --    sda_pad_o    : out std_logic_vector(g_num_interfaces-1 downto 0);
  --    sda_padoen_o : out std_logic_vector(g_num_interfaces-1 downto 0));
  --end component;

  constant c_i2c_wrapper_sdb : t_sdb_device := (
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
    device_id     => x"12575a95",
    version       => x"00000001",
    date          => x"20211012",
    name          => "I2C_MULTI_WRAPPER  ")));

end wb_i2c_wrapper_pkg;
