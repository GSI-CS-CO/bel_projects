library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.monster_pkg.all;

package beam_dump_pkg is

  constant c_beam_dump_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"00",
    abi_ver_minor => x"00",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
      sdb_component => (
      addr_first    => x"0000000000000000",
      addr_last     => x"00000000000000ff",
      product => (
      vendor_id     => x"0000000000000651",
      device_id     => x"11223344",
      version       => x"00000001",
      date          => x"20191013",
      name          => "GSI:BEAM_DUMP      "))
    );

  component beam_dump is
    port(
      clk_i      : in  std_logic;
      rst_n_i    : in  std_logic;
      slave_i    : in  t_wishbone_slave_in;
      slave_o    : out t_wishbone_slave_out);
  end component;

end package;
