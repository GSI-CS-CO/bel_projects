library ieee;
use ieee.std_logic_1164.all;

library work;
use work.wishbone_pkg.all;

package wr_info_pkg is

  constant c_wr_info_time_valid_bit : natural := 0;
  constant c_wr_info_link_valid_bit : natural := 1;

  constant c_wr_info_sdb : t_sdb_device := (
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
      device_id     => x"77722d69",
      version       => x"00000001",
      date          => x"20260925",
      name          => "GSI:WR_INFO        "))
    );

  component wr_info is
    port(
      clk_i             : in  std_logic;
      rst_n_i           : in  std_logic;
      wr_time_valid     : in  std_logic := '0';
      wr_link_valid     : in  std_logic := '0';
      wr_aux_time_valid : in  std_logic := '0';
      wr_aux_link_valid : in  std_logic := '0';
      slave_i           : in  t_wishbone_slave_in;
      slave_o           : out t_wishbone_slave_out);
  end component;

end package;
