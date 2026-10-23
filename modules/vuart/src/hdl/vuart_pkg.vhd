library ieee;
use ieee.std_logic_1164.all;

library work;
use work.wishbone_pkg.all;

package vuart_pkg is

  -- Serial virtual UART (bridges the WRCORE console to the Etherbone host,
  -- so tools/eb-console.c works). Identity + register map match eb-console.c:
  -- vendor 0xce42 (CERN), device 0xe2d13d04, TX at +0x10, RX at +0x14.
  constant c_vuart_sdb : t_sdb_device := (
    abi_class     => x"0000",
    abi_ver_major => x"00",
    abi_ver_minor => x"00",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
      sdb_component => (
      addr_first    => x"0000000000000000",
      addr_last     => x"000000000000001f",
      product => (
      vendor_id     => x"000000000000CE42",
      device_id     => x"e2d13d04",
      version       => x"00000001",
      date          => x"20261009",
      name          => "WR-Periph-VUART    "))
    );

  component vuart is
    generic (
      g_baud : natural := 966); -- SUART_CALC_BAUD value for the core console
    port(
      clk_i      : in  std_logic;
      rst_n_i    : in  std_logic;
      slave_i    : in  t_wishbone_slave_in;
      slave_o    : out t_wishbone_slave_out;
      uart_rxd_i : in  std_logic := '1'; -- core console TX (serial in)
      uart_txd_o : out std_logic);       -- drives core console RX (idle high)
  end component;

  -- FIFO-only virtual UART (no serialization), core -> host only. Uses
  -- c_vuart_sdb (eb-console.c finds it), tell instances apart by SDB name.
  component vuart_fifo is
    generic (
      g_size : natural := 1024); -- FIFO depth in bytes
    port(
      clk_i   : in  std_logic;
      rst_n_i : in  std_logic;
      slave_i : in  t_wishbone_slave_in;
      slave_o : out t_wishbone_slave_out);
  end component;

end package;
