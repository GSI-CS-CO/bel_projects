library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.genram_pkg.all;
use work.wb_dma_slave_auto_pkg.all;

entity wb_dma is
  generic(
    g_host_ram_size  : Integer := 16
  );
  port(
    clk_sys_i     : in std_logic;
    rstn_sys_i    : in std_logic;

    slave_i : in t_wishbone_slave_in;
    slave_o : out t_wishbone_slave_out
    );
end entity;

architecture rtl of wb_dma is

  -- HOST RAM SIGNALS
  ------------------------------------------
  signal s_host_ram_wea       : std_logic;
  signal s_host_ram_address_a : std_logic_vector(c_wishbone_address_width-1 downto 0);
  signal s_host_ram_data_a    : std_logic_vector(c_wishbone_data_width-1 downto 0);
  signal s_host_ram_out_a     : std_logic_vector(c_wishbone_data_width-1 downto 0);
  signal s_host_ram_web       : std_logic;
  signal s_host_ram_address_b : std_logic_vector(c_wishbone_address_width-1 downto 0);
  signal s_host_ram_data_b    : std_logic_vector(c_wishbone_data_width-1 downto 0);
  signal s_host_ram_out_b     : std_logic_vector(c_wishbone_data_width-1 downto 0);

  -- DMA ENGINE SIGNALS
  ------------------------------------------
  -- read ops signals
  signal s_read_ops_we             : std_logic := '0';

  signal s_load_descriptor_en      : std_logic := '0';

  signal s_desc_csr_sz_we  : std_logic;
  signal s_desc_addr0_we   : std_logic;
  signal s_desc_addr1_we   : std_logic;
  signal s_pointer_we      : std_logic;

  -- read logic
  signal s_ops_queue_full          : std_logic := '0';
  signal s_ops_queue_empty         : std_logic := '1';
  signal s_read_enable             : std_logic := '0';
  signal s_store_read_op           : std_logic_vector(2*c_wishbone_address_width downto 0) := (others => '0');
  signal s_data_cache_write_enable : std_logic := '0';

  -- REGISTER FILE SIGNALS
  ------------------------------------------
  signal s_read_ops_read_en     : std_logic := '0';
  signal s_read_ops             : std_logic_vector((2*c_wishbone_address_width)-1 downto 0) := (others => '0');
  signal s_read_ops_cache_full  : std_logic := '0';
  signal s_read_ops_cache_empty : std_logic := '1';
  signal s_read_init_address    : std_logic_vector(c_wishbone_address_width-1 downto 0) := (others => '0');

  -- read FSM signals
  signal cache_we          : std_logic := '0'; -- write enable
  signal data_cache_empty  : std_logic := '1';

  -- write FSM signals
  signal cache_re          : std_logic := '0'; -- read enable
  signal data_cache_full   : std_logic := '0';



  -- component wb_dma_slave is
  --   generic(
  --     g_channels  : natural := 16 --Number of DMA channels
  --   );
  --   Port(
  --     clk_sys_i             : std_logic;                                        -- Clock input for sys domain
  --     rst_sys_n_i           : std_logic;                                        -- Reset input (active low) for sys domain
  --     error_i               : in  std_logic_vector(1-1 downto 0);               -- Error control
  --     stall_i               : in  std_logic_vector(1-1 downto 0);               -- flow control
  --     channel_csr_o         : out matrix(g_channels-1 downto 0, 32-1 downto 0); -- DMA channel CSR
  --     descr_queue_intake_o  : out matrix(g_channels-1 downto 0, 32-1 downto 0); -- DMA channel descriptor queue intake
  --     dma_csr_o             : out std_logic_vector(32-1 downto 0);              -- DMA controller control and status register
      
  --     data_i                : in  t_wishbone_slave_in;
  --     data_o                : out t_wishbone_slave_out
    
      
  --   );
  -- end component;

  -- component wb_dma_engine is
  -- port(
  --   clk_i : in std_logic;
  --   rstn_i : in std_logic;

  --   s_load_descriptor_en_o      : out std_logic;

  --   s_desc_csr_sz_we_o  : out std_logic;
  --   s_desc_addr0_we_o   : out std_logic;
  --   s_desc_addr1_we_o   : out std_logic;
  --   s_pointer_we_o      : out std_logic;

  --   -- read logic
  --   s_queue_full_i              : in std_logic;
  --   s_queue_empty_i             : in std_logic;
  --   s_read_enable_o             : out std_logic := '0';
  --   s_data_cache_write_enable_o : out std_logic := '0';
  --   s_read_ack                  : in std_logic;

  --   --only for testing!!!!
  --   s_start_desc                : in std_logic;
  --   s_read_init_address         : in std_logic_vector(c_wishbone_address_width-1 downto 0);
  --   s_descriptor_active         : in std_logic;

  --   s_data_in_ack_i             : in std_logic
  -- );
  -- end component;

  component wb_dma_ch_rf is
    generic(
      g_data_cache_size : natural := 1
    );
    port(
      clk_i : in std_logic;
      rstn_i : in std_logic;

      -- module IOs
      --wb_data_i : in std_logic_vector(c_wishbone_data_width-1 downto 0);
      --wb_data_o : out std_logic_vector(c_wishbone_data_width-1 downto 0);

      -- control signals
      s_desc_csr_sz_we  : in std_logic;
      s_desc_addr0_we   : in std_logic;
      s_desc_addr1_we   : in std_logic;
      s_pointer_we      : in std_logic;

      data_in           : in std_logic_vector(c_wishbone_data_width-1 downto 0);
  
      -- read FSM signals
      cache_we          : in std_logic; -- write enable
      data_cache_empty  : out std_logic;
  
      -- write FSM signals
      cache_re          : in std_logic; -- read enable
      data_cache_full   : out std_logic
    );
  end component;
    

begin

    -- dma_engine: wb_dma_engine
    -- port map (
    --     clk_i => clk_sys_i,
    --     rstn_i => rstn_sys_i,

    --     s_load_descriptor_en_o => s_load_descriptor_en,

    --     s_desc_csr_sz_we_o  => s_desc_csr_sz_we,
    --     s_desc_addr0_we_o   => s_desc_addr0_we,
    --     s_desc_addr1_we_o   => s_desc_addr1_we,
    --     s_pointer_we_o      => s_pointer_we,

    --     -- read logic
    --     s_queue_full_i => s_ops_queue_full,
    --     s_queue_empty_i => s_ops_queue_empty,
    --     s_read_enable_o => s_read_enable,
    --     s_data_cache_write_enable_o => s_data_cache_write_enable,
    --     s_read_ack => '0',
        
    --     s_start_desc => '0',--s_start_desc_i,
    --     s_read_init_address => (others => '0'),--s_read_init_address_i,
    --     s_descriptor_active => '0',--s_descriptor_active_i,

    --     s_data_in_ack_i => '0'--s_data_in_ack_i
    -- );    

    -- register_file: wb_dma_ch_rf
    -- generic map (
    --     g_data_cache_size => 16
    -- )
    -- port map (
    --   clk_i => clk_sys_i,
    --   rstn_i => rstn_sys_i,
  
    --   -- module IOs
    --   --wb_data_i => (others => '0'), 
    --   --wb_data_o => open, 
    --   -- read FSM signals

    --   s_desc_csr_sz_we  => s_desc_csr_sz_we,
    --   s_desc_addr0_we   => s_desc_addr0_we,
    --   s_desc_addr1_we   => s_desc_addr1_we,
    --   s_pointer_we      => s_pointer_we,

    --   data_in => (others => '0'),--s_data_in_i,

    --   cache_we          => '0',
    --   data_cache_empty  => open,
  
    --   -- write FSM signals
    --   cache_re          => '0',
    --   data_cache_full   => open
    -- );

    register_file_ram: generic_dpram
    generic map (
      g_data_width               => c_wishbone_data_width,
      g_size                     => g_host_ram_size,
      g_with_byte_enable         => false,
      g_dual_clock               => false
    )
    port map (
      rst_n_i => rstn_sys_i,      -- synchronous reset, active LO

      -- Port A
      clka_i => clk_sys_i,
      bwea_i => (others => '0'),
      wea_i  => s_host_ram_wea,
      aa_i   => s_host_ram_address_a(f_log2_size(g_host_ram_size)-1 downto 0),
      da_i   => s_host_ram_data_a,
      qa_o   => s_host_ram_out_a,
      
      -- Port B
      clkb_i => clk_sys_i,
      bweb_i => (others => '0'),
      web_i  => s_host_ram_web,
      ab_i   => s_host_ram_address_b(f_log2_size(g_host_ram_size)-1 downto 0),
      db_i   => s_host_ram_data_b,
      qb_o   => s_host_ram_out_b
    );

    wishbone_slave : wb_dma_slave_auto
    generic map (
      g_channels => 16 --Number of DMA channels
    )
    port map (
      clk_sys_i              => clk_sys_i,  -- Clock input for sys domain
      rst_sys_n_i            => rstn_sys_i, -- Reset input (active low) for sys domain
      error_i                => (others => '0'),        -- Error control
      stall_i                => (others => '0'),        -- flow control
      channel_csr_o          =>  open,      -- DMA channel CSR
      descr_queue_intake_o   =>  open,      -- DMA channel descriptor queue intake
      dma_csr_o              =>  open,      -- DMA controller control and status register
      
      data_i                => slave_i,
      data_o                => slave_o
    );
end architecture;
