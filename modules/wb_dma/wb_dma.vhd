library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;

entity wb_dma is
    -- generic (
    --   g_aux_phy_interface : boolean
    -- );
    port(
        clk_sys_i     : in std_logic;
        clk_ref_i     : in std_logic;
        rstn_sys_i    : in std_logic;
        rstn_ref_i    : in std_logic;
    
        master_o       : out t_wishbone_slave_out;
        master_i       : in  t_wishbone_slave_in
        );
end entity;

architecture rtl of wb_dma is

    signal s_queue_full : std_logic;
    signal s_write_data_cache_enable : std_logic;

    -- DMA ENGINE SIGNALS
    ------------------------------------------
    -- read ops signals
    signal s_read_ops_we             : std_logic;

    -- read logic
    signal s_ops_queue_full          : std_logic;
    signal s_ops_queue_empty         : std_logic;
    signal s_read_enable             : std_logic;
    signal s_store_read_op           : std_logic_vector(2*c_wishbone_address_width downto 0);
    signal s_data_cache_write_enable : std_logic;

    -- REGISTER FILE SIGNALS
    ------------------------------------------
    signal s_read_ops_read_en     : std_logic;
    signal s_read_ops             : std_logic_vector((2*c_wishbone_address_width)-1 downto 0);
    signal s_read_ops_cache_full  : std_logic;
    signal s_read_ops_cache_empty : std_logic;
    signal s_read_init_address    : std_logic_vector(c_wishbone_address_width-1 downto 0);

    -- read FSM signals
    signal cache_we          : std_logic; -- write enable
    signal data_cache_empty  : std_logic;

    -- write FSM signals
    signal cache_re          : std_logic; -- read enable
    signal data_cache_full   : std_logic;

begin

    dma_engine: wb_dma_engine
    port map (
        clk_i => clk_sys_i,
        rstn_i => rstn_sys_i,

        -- read ops signals
        s_read_ops_we_o => s_read_ops_we,

        -- read logic
        s_queue_full_i => s_ops_queue_full,
        s_queue_empty_i => s_ops_queue_empty,
        s_read_enable_o => s_read_enable,
        s_store_read_op_o => s_store_read_op,
        s_data_cache_write_enable_i => s_data_cache_write_enable
    );    

    register_file: wb_dma_ch_rf
    generic map (
        g_data_cache_size => 16,
        g_read_cache_size => 8
    )
    port map (
      clk_i => clk_i,
      rstn_i => rstn_i,
  
      -- module IOs
      wb_data_i => (others => '0'), 
      wb_data_o => open, 

      -- read ops FIFO control signals
      s_read_ops_we_i           => s_read_ops_we,
      s_read_ops_read_en_i      => s_read_enable,
      s_read_op_i               => (others => '0'),
      s_read_ops_cache_full_o   => s_ops_queue_full,
      s_read_ops_cache_empty_o  => s_ops_queue_empty,
  
      -- read FSM signals
      cache_we          => '0',
      data_cache_empty  => open,
  
      -- write FSM signals
      cache_re          => '0',
      data_cache_full   => open
    );
end architecture;