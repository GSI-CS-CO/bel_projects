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
    addr_last     => x"0000000000000000",
    product       => (
    vendor_id     => x"0000000000000651",
    device_id     => x"434E5452",
    version       => x"00000001",
    date          => x"20240711",
    name          => "GSI:DMA_CONTROLLER "))
    );

  constant c_dma_stop : natural := 9;
  constant c_dma_busy : natural := 10;
  constant c_dma_done : natural := 11;
  constant c_dma_err  : natural := 12;

  constant c_desc_csr_sz  : std_logic_vector (1 downto 0) := "00";
  constant c_addr0        : std_logic_vector (1 downto 0) := "01";
  constant c_addr1        : std_logic_vector (1 downto 0) := "10";
  constant c_next_desc    : std_logic_vector (1 downto 0) := "11";

  component wb_dma is
    generic(
      g_host_ram_size  : Integer := 16;
      g_dma_transfer_block_size : Integer := 4
    );
    port(
      clk_sys_i     : in std_logic;
      rstn_sys_i    : in std_logic;
  
      slave_i     : in t_wishbone_slave_in;
      slave_o     : out t_wishbone_slave_out;
      ram_slave_i : in t_wishbone_slave_in;
      ram_slave_o : out t_wishbone_slave_out;
      master1_i   : in t_wishbone_master_in;
      master1_o   : out t_wishbone_master_out;
      master2_i   : in t_wishbone_master_in;
      master2_o   : out t_wishbone_master_out
      );
  end component;

  function log2_floor(N       : natural) return positive;

end package;

package body wb_dma_pkg is

  function log2_floor(N : natural) return positive is
  begin
    if N <= 2 then
      return 1;
    elsif N mod 2 = 0 then
      return 1 + log2_floor(N/2);
    else
      return 1 + log2_floor((N-1)/2);
    end if;
  end;

end wb_dma_pkg;