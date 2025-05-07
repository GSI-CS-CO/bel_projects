library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;

package cellular_ram_pkg is

  function f_cellular_ram_sdb(g_bits : natural) return t_sdb_device;

  component cellular_ram is
    generic(
      g_bits     : natural := 24);
    port(
      clk_i      : in    std_logic;
      rstn_i     : in    std_logic;
      slave_i    : in    t_wishbone_slave_in;
      slave_o    : out   t_wishbone_slave_out;
      cr_clk_o   : out   std_logic;
      cr_addr_o  : out   std_logic_vector(g_bits-1 downto 0);
      cr_data_io : inout std_logic_vector(15 downto 0);
      cr_ubn_o   : out   std_logic_vector(3 downto 0);
      cr_lbn_o   : out   std_logic_vector(3 downto 0);
      cr_cen_o   : out   std_logic_vector(3 downto 0);
      cr_oen_o   : out   std_logic_vector(3 downto 0);
      cr_wen_o   : out   std_logic_vector(3 downto 0);
      cr_cre_o   : out   std_logic_vector(3 downto 0);
      cr_advn_o  : out   std_logic_vector(3 downto 0);
      cr_wait_i  : in    std_logic_vector(3 downto 0));
  end component;

  type t_cellular_ram_out is record
    cre : std_logic;
    oen : std_logic;
    wen : std_logic;
    cen : std_logic;
    ubn : std_logic;
    lbn : std_logic;
  end record t_cellular_ram_out;

  function f_cellular_ram_set_outputs(cre : std_logic; oen : std_logic; wen : std_logic;
                                      cen : std_logic; ubn : std_logic; lbn : std_logic)
                                      return t_cellular_ram_out;
  function f_cellular_ram_set_read    return t_cellular_ram_out; -- Read from RAM
  function f_cellular_ram_set_write   return t_cellular_ram_out; -- Write to RAM
  function f_cellular_ram_set_standby return t_cellular_ram_out; -- Standby
  function f_cellular_ram_set_noop    return t_cellular_ram_out; -- No operation
  function f_cellular_ram_set_cfg_w   return t_cellular_ram_out; -- Configuration register write
  function f_cellular_ram_set_cfg_r   return t_cellular_ram_out; -- Configuration register read
  function f_cellular_ram_set_dpd     return t_cellular_ram_out; -- Deep power-down

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
    result.sdb_component.addr_last  := std_logic_vector(to_unsigned(((2**(g_bits+1)-1)*4), 64)); -- One RAM
    --result.sdb_component.addr_last := x"000000000200001f"; -- 4 RAMs + configuration registers
    --result.sdb_component.addr_last  := std_logic_vector(to_unsigned(2**(g_bits+1)-1, 64)); -- One RAM

    result.sdb_component.product.vendor_id := x"0000000000000651";
    result.sdb_component.product.device_id := x"169edcb8";
    result.sdb_component.product.version   := x"00000001";
    result.sdb_component.product.date      := x"20250401";
    result.sdb_component.product.name      := "Cellular RAM       ";

    return result;
  end f_cellular_ram_sdb;

  function f_cellular_ram_set_outputs(cre : std_logic; oen : std_logic; wen : std_logic;
                                     cen : std_logic; ubn : std_logic; lbn : std_logic) return t_cellular_ram_out is
  variable v_setup : t_cellular_ram_out;
  begin
    v_setup.cre := cre;
    v_setup.oen := oen;
    v_setup.wen := wen;
    v_setup.cen := cen;
    v_setup.ubn := ubn;
    v_setup.lbn := lbn;
    return v_setup;
  end function f_cellular_ram_set_outputs;

  -- Table                             CRE OEN WEN CEN UBN LBN
  function f_cellular_ram_set_read return t_cellular_ram_out is
  begin
     return f_cellular_ram_set_outputs('0','0','1','0','0','0');
  end function f_cellular_ram_set_read;

  function f_cellular_ram_set_write return t_cellular_ram_out is
  begin
    return f_cellular_ram_set_outputs('0','0','0','0','0','0');
  end function f_cellular_ram_set_write;

  function f_cellular_ram_set_standby return t_cellular_ram_out is
  begin
    return f_cellular_ram_set_outputs('0','1','1','1','1','1');
  end function f_cellular_ram_set_standby;

  function f_cellular_ram_set_noop return t_cellular_ram_out is
  begin
    return f_cellular_ram_set_outputs('0','0','0','0','0','0');
  end function f_cellular_ram_set_noop;

  function f_cellular_ram_set_cfg_w return t_cellular_ram_out is
  begin
    return f_cellular_ram_set_outputs('1','1','0','0','0','0');
  end function f_cellular_ram_set_cfg_w;

  function f_cellular_ram_set_cfg_r return t_cellular_ram_out is
  begin
    return f_cellular_ram_set_outputs('1','0','1','0','0','0');
  end function f_cellular_ram_set_cfg_r;

  function f_cellular_ram_set_dpd return t_cellular_ram_out is
  begin
    return f_cellular_ram_set_outputs('0','0','0','1','0','0');
  end function f_cellular_ram_set_dpd;

end cellular_ram_pkg;
