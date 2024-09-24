library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wb_dma_pkg.all;

entity wb_dma_ch_rf is
  generic(
    g_data_cache_size : natural := 16;
    g_read_cache_size : natural := 8
  );
  port(
    clk_i : in std_logic;
    rstn_i : in std_logic;

    -- module IOs
    wb_data_i : std_logic_vector(c_wishbone_data_width-1 downto 0);
    wb_data_o : std_logic_vector(c_wishbone_data_width-1 downto 0); 

    -- register file update control signals
    -- de_csr_i
    -- de_txsz_i
    -- de_adr0_i
    -- de_adr1_i
    -- de_csr_we_i
    -- de_txsz_we_i
    -- de_adr0_we_i
    -- de_adr1_we_i        : in 
    -- de_fetch_descr_o  : out std_logic;
    -- dma_rest
    -- ptr_set

    -- read ops FIFO control signals
    s_read_ops_we_i           : in std_logic;
    s_read_ops_read_en_i      : in std_logic;
    s_read_ops                : in std_logic_vector((2*c_wishbone_address_width)-1 downto 0);
    s_read_ops_cache_full_o   : out std_logic;
    s_read_ops_cache_empty_o  : out std_logic;
    s_read_init_address       : in std_logic_vector(c_wishbone_address_width-1 downto 0);

    -- read FSM signals
    cache_we          : in std_logic; -- write enable
    data_cache_empty  : out std_logic;

    -- write FSM signals
    cache_re          : in std_logic; -- read enable
    data_cache_full   : out std_logic


    
  );
end entity;

architecture rtl of wb_dma_ch_rf is

signal s_desc_csr_sz  : std_logic_vector(c_wishbone_data_width-1 downto 0);
signal s_desc_addr0   : std_logic_vector(c_wishbone_data_width-1 downto 0);
signal s_desc_addr1   : std_logic_vector(c_wishbone_data_width-1 downto 0);
signal s_pointer      : std_logic_vector(c_wishbone_address_width-1 downto 0);

signal s_write_addr   : std_logic_vector(c_wishbone_address_width-1 downto 0);
signal s_read_op      : std_logic_vector((2*c_wishbone_address_width)-1 downto 0);
signal r_hold_op      : std_logic_vector((2*c_wishbone_address_width)-1 downto 0); -- register not possible?

begin

read_op_buffer: generic_async_fifo
generic map (
  g_data_width => 2*c_wishbone_address_width,  -- save the read and the write address, to then write the data with the corresponding write address into cache
  g_size => g_read_cache_size
)
port map (
  rst_n_i => rstn_i,

  -- write port
  clk_wr_i => clk_i,
  d_i      => TODO & TODO,
  we_i     => s_read_ops_we_i,

  wr_empty_o        => s_read_ops_cache_full_o,
  wr_full_o         => open,
  wr_almost_empty_o => open,
  wr_almost_full_o  => open,
  wr_count_o        => open,

  -- read port
  clk_rd_i => clk_i,
  q_o      => wb_data_o,
  rd_i     => data_cache_full,

  rd_empty_o        => s_read_ops_cache_empty_o,
  rd_full_o         => open,
  rd_almost_empty_o => open,
  rd_almost_full_o  => open,
  rd_count_o        => open
);

data_cache : generic_async_fifo
generic map (
  g_data_width => 64,
  g_size => g_data_cache_size
)
port map (
  rst_n_i => rstn_i,

  -- write port
  clk_wr_i => clk_i,
  d_i      => wb_data_i & addr,
  we_i     => cache_we,

  wr_empty_o        => data_cache_full,
  wr_full_o         => open,
  wr_almost_empty_o => open,
  wr_almost_full_o  => open,
  wr_count_o        => open,

  -- read port
  clk_rd_i => clk_i,
  q_o      => wb_data_o,
  rd_i     => data_cache_full,

  rd_empty_o        => read_queue_empty,
  rd_full_o         => open,
  rd_almost_empty_o => open,
  rd_almost_full_o  => open,
  rd_count_o        => open
);

end architecture;