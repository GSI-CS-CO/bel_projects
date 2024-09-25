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

    
  component wb_dma is
    -- generic (
    --   g_aux_phy_interface : boolean
    -- );
    port(
    clk_sys_i     : in std_logic;
    clk_ref_i     : in std_logic;
    rstn_sys_i    : in std_logic;
    rstn_ref_i    : in std_logic;

    master_o      : out t_wishbone_slave_out;
    master_i      : in  t_wishbone_slave_in
    );
  end component;

  component wb_dma_engine is
    port(
      clk_i : in std_logic;
      rstn_i : in std_logic;
  
      -- read ops signals
      s_read_ops_we_o             : out std_logic;
  
      -- read logic
      s_queue_full_i              : in std_logic;
      s_queue_empty_i             : in std_logic;
      s_read_enable_o             : out std_logic;
      s_store_read_op_o           : out std_logic_vector((2*c_wishbone_address_width)-1 downto 0);
      s_data_cache_write_enable_o : out std_logic;
      s_read_ack                  : in std_logic;
  
      --only for testing!!!!
      s_start_desc                : in std_logic;
      s_read_init_address         : in std_logic_vector(c_wishbone_address_width-1 downto 0);
      s_descriptor_active         : in std_logic
    );
  end component;

  component wb_dma_ch_rf is
    generic(
      g_data_cache_size : natural := 16;
      g_read_cache_size : natural := 8
    );
    port(
      clk_i : in std_logic;
      rstn_i : in std_logic;
  
      -- module IOs
      wb_data_i : in std_logic_vector(c_wishbone_data_width-1 downto 0);
      wb_data_o : out std_logic_vector(c_wishbone_data_width-1 downto 0);
  
      -- read ops FIFO control signals
      s_read_ops_we_i           : in std_logic;
      s_read_ops_read_en_i      : in std_logic;
      s_read_op_i               : in std_logic_vector((2*c_wishbone_address_width)-1 downto 0);
      s_read_ops_cache_full_o   : out std_logic;
      s_read_ops_cache_empty_o  : out std_logic;
  
      -- read FSM signals
      cache_we          : in std_logic; -- write enable
      data_cache_empty  : out std_logic;
  
      -- write FSM signals
      cache_re          : in std_logic; -- read enable
      data_cache_full   : out std_logic
    );
  end component;

end package;
