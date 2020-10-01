-- ------------------------------------------------------------------------------------------------
-- Simple Reconf interface to wishbone adapter
-- Attention: Clock crossing is ignored, for debugging purpose only
-- ------------------------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.monster_pkg.all;

package cpri_phy_reconf_pkg is

  constant c_cpri_phy_reconf_sdb : t_sdb_device := (
    abi_class     => x"0000",
    abi_ver_major => x"00",
    abi_ver_minor => x"00",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7",
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"000000000000000f",
    product => (
    vendor_id     => x"0000000000000651",
    device_id     => x"c961980f",
    version       => x"00000001",
    date          => x"20190404",
    name          => "CPRI_PHY_RECONF    "))
    );

  component cpri_phy_reconf is
    port(
      clk_i                  : in  std_logic := '0';
      rst_n_i                : in  std_logic := '1';
      slave_i                : in  t_wishbone_slave_in;
      slave_o                : out t_wishbone_slave_out;
      reconfig_write_o       : out std_logic;
      reconfig_read_o        : out std_logic;
      reconfig_address_o     : out std_logic_vector(31 downto 0);
      reconfig_writedata_o   : out std_logic_vector(31 downto 0);
      reconfig_readdata_i    : in  std_logic_vector(31 downto 0) := (others => '0');
      reconfig_waitrequest_i : in  std_logic_vector(0 downto 0) := (others => '0')
    );
  end component;

end cpri_phy_reconf_pkg;
