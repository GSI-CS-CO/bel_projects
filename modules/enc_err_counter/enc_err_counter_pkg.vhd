library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
--use work.monster_pkg.all;

package enc_err_counter_pkg is

  constant c_enc_err_counter_sdb : t_sdb_device := (
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
    
    
  component enc_err_counter is
    generic (
      g_aux_phy_interface : boolean
    );
    port(
    clk_sys_i     : in std_logic;
    clk_ref_i     : in std_logic;
    rstn_sys_i    : in std_logic;
    rstn_ref_i    : in std_logic;

    slave_o       : out t_wishbone_slave_out;
    slave_i       : in  t_wishbone_slave_in;

    enc_err_i     : in std_logic;
    enc_err_aux_i : in std_logic
    );
  end component;

end package;
