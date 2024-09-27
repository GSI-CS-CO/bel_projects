library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;

package wb_dma_pkg is

  constant c_wb_dma_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"00",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"00000000000000ff",
    product       => (
    vendor_id     => x"0000000000000651",
    device_id     => x"434E5452",
    version       => x"00000001",
    date          => x"20240711",
    name          => "GSI:ENC_ERR_COUNTER"))
    );

  constant c_dma_stop : natural := 9;
  constant c_dma_busy : natural := 10;
  constant c_dma_done : natural := 11;
  constant c_dma_err  : natural := 12;

end package;
