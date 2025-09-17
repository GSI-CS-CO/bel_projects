library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.monster_pkg.all;

package a10ts_pkg is

  constant c_a10ts_sdb : t_sdb_device := (
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
    device_id     => x"a1075000",
    version       => x"00000001",
    date          => x"20240420",
    name          => "ARRIA10_TS         "))
  );

  component a10ts is
    generic (
      g_use_ext_trigger : boolean := false);
    port (
      clk_i     : in  std_logic := '0';
      rst_n_i   : in  std_logic := '1';
      clk_20m_i : in  std_logic := '0';
      ge_85_c_o : out std_logic := '0';
      slave_i   : in  t_wishbone_slave_in;
      slave_o   : out t_wishbone_slave_out);
  end component;

  component a10ts_ip is
    port (
      corectl : in  std_logic := 'X';
      eoc     : out std_logic;
      reset   : in  std_logic := 'X';
      tempout : out std_logic_vector(9 downto 0)
    );
  end component a10ts_ip;

end a10ts_pkg;
