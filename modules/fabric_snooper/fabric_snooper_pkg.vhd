library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wr_fabric_pkg.all;

package fabric_snooper_pkg is

  constant c_snooper_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"4", -- 32-bit port granularity
    sdb_component => (
      addr_first    => x"0000000000000000",
      addr_last     => x"000000000000ffff",
      product => (
      vendor_id     => x"0000000000000651", -- GSI
      device_id     => x"88baa0f2",
      version       => x"00000001",
      date          => x"20140701",
      name          => "WR_FABRIC_SNOOPER  ")));

  component fabric_snooper is
    port (
      clk_i     : in  std_logic;
      rstn_i    : in  std_logic;
      snk_i     : in  t_wrf_sink_in;
      slave_o   : out t_wishbone_slave_out;
      slave_i   : in  t_wishbone_slave_in
        );
  end component;

end fabric_snooper_pkg;



