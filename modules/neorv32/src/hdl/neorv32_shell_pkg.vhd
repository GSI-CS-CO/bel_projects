library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.genram_pkg.all;

library neorv32;
use neorv32.neorv32_package.all;

package neorv32_shell_pkg is

  function f_neorv32_ram_sdb(g_ram_size : natural) return t_sdb_device;
  function f_neorv32_report_wb_range(start_addr : std_ulogic_vector(31 downto 0); sdb_addr :t_wishbone_address) return std_logic;

  component neorv32_shell is
    generic (
      g_clock_frequency           : natural := 62500000;                            -- clock frequency of clk_i in Hz
      g_sdb_addr                  : t_wishbone_address := x"00001000";              -- base SDB record
      g_mem_wishbone_imem_size    : natural := 4*8000;                              -- memory size in bytes
      g_mem_wishbone_imem_addr    : std_ulogic_vector(31 downto 0) := x"71000000";  -- imem RAM start address
      g_mem_wishbone_init_file    : string := "";                                   -- init file Wishbone instruction memory
      g_mem_int_imem_size         : natural := 16*1024;                             -- size of processor-internal instruction memory in bytes
      g_mem_int_dmem_size         : natural := 16*1024;                             -- size of processor-internal data memory in bytes
      g_use_wb_adapter            : boolean := false;                               -- use wishbone slave adapter CLASSIC/PIPELINED
      g_en_debugging              : boolean := false                                -- enable OCD debugging
    );
    port (
      -- Global control
      clk_i      : in std_logic;
      rstn_i     : in std_logic;
      rstn_ext_i : in std_logic;
      -- Peripherals
      uart_o     : out std_logic;
      -- Wishbone
      slave_i    : in  t_wishbone_slave_in;
      slave_o    : out t_wishbone_slave_out;
      master_i   : in  t_wishbone_master_in;
      master_o   : out t_wishbone_master_out;
      -- JTAG
      jtag_tck_i : in  std_logic;
      jtag_tdi_i : in  std_logic;
      jtag_tdo_o : out std_logic;
      jtag_tms_i : in  std_logic);
    end component;

end package;

  package body neorv32_shell_pkg is

    function f_neorv32_report_wb_range(start_addr : std_ulogic_vector(31 downto 0); sdb_addr : t_wishbone_address)
      return std_logic is
    begin
      report "NEORV32: Wishbone start address (dec) = " & integer'image(to_integer(unsigned(start_addr)));
      report "NEORV32: SDB address (dec)            = " & integer'image(to_integer(unsigned(sdb_addr)));
      return '0';
    end f_neorv32_report_wb_range;

    function f_neorv32_ram_sdb(g_ram_size : natural) return t_sdb_device
    is
      variable result : t_sdb_device;
    begin
      result.abi_class                       := x"0001";
      result.abi_ver_major                   := x"01";
      result.abi_ver_minor                   := x"00";
      result.wbd_width                       := x"7";
      result.wbd_endian                      := c_sdb_endian_big;
      result.sdb_component.addr_first        := (others => '0');
      result.sdb_component.addr_last         := std_logic_vector(to_unsigned(g_ram_size - 1, 64));
      result.sdb_component.product.vendor_id := x"0000000000000651";
      result.sdb_component.product.device_id := x"83075320";
      result.sdb_component.product.version   := x"00000001";
      result.sdb_component.product.date      := x"20250725";
      result.sdb_component.product.name      := "NEORV32_RAM        ";
      return result;
    end f_neorv32_ram_sdb;

end neorv32_shell_pkg;
