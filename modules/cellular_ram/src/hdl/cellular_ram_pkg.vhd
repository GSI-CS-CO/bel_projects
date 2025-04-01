library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;

package cellular_ram_pkg is

  function f_cellular_ram_sdb(g_bits : natural) return t_sdb_device;

  component cellular_ram is
    generic(
      g_bits     : natural := 24;
      g_row_bits : natural := 8);
    port(
      clk_i     : in    std_logic;
      rstn_i    : in    std_logic;
      slave_i   : in    t_wishbone_slave_in;
      slave_o   : out   t_wishbone_slave_out;
      ps_clk    : out   std_logic;
      ps_addr   : out   std_logic_vector(g_bits-1 downto 0);
      ps_data   : inout std_logic_vector(15 downto 0);
      ps_seln   : out   std_logic_vector(1 downto 0);
      ps_cen    : out   std_logic;
      ps_oen    : out   std_logic;
      ps_wen    : out   std_logic;
      ps_cre    : out   std_logic;
      ps_advn   : out   std_logic;
      ps_wait   : in    std_logic);
  end component;

end package;

package body cellular_ram_pkg is

  function f_cellular_ram_sdb(g_bits : natural) return t_sdb_device
  is
    variable result : t_sdb_device;
  begin
    result.abi_class     := x"0001";
    result.abi_ver_major := x"01";
    result.abi_ver_minor := x"00";
    result.wbd_width     := x"7";
    result.wbd_endian    := c_sdb_endian_big;

    result.sdb_component.addr_first := (others => '0');
    result.sdb_component.addr_last  := std_logic_vector(to_unsigned(2**(g_bits+1)-1, 64));

    result.sdb_component.product.vendor_id := x"0000000000000651";
    result.sdb_component.product.device_id := x"169edcb8";
    result.sdb_component.product.version   := x"00000001";
    result.sdb_component.product.date      := x"20250401";
    result.sdb_component.product.name      := "Cellular RAM       ";

    return result;
  end f_cellular_ram_sdb;

end cellular_ram_pkg;
