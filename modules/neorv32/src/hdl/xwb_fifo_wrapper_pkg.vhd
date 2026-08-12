library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.genram_pkg.all;

package xwb_fifo_wrapper_pkg is
  component xwb_fifo_wrapper is
    generic (
      g_fifo_size  : natural := 64;
      g_show_ahead : boolean := true;
      g_with_almost_empty  : boolean := true;
      g_with_almost_full   : boolean := true;
      g_almost_empty_thres : integer := 1; 
      g_almost_full_thres  : integer := 60
    );
    port (
      clk_i        : in  std_logic;
      rstn_i       : in  std_logic;
      slave_i      : in  t_wishbone_slave_in;
      slave_o      : out t_wishbone_slave_out
    );
  end component;

  constant c_fifo_wrapper_sdb : t_sdb_device := (
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
    device_id     => x"fa142cae",
    version       => x"00000001",
    date          => x"20260730",
    name          => "XWB_FIFO_WRAPPER   ")));

  end xwb_fifo_wrapper_pkg;
