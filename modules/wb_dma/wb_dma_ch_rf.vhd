library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wb_dma_pkg.all;
use work.genram_pkg.all;

entity wb_dma_ch_rf is
  generic(
    g_data_cache_size : natural := 16
  );
  port(
    clk_i : in std_logic;
    rstn_i : in std_logic;

    -- module IOs
    --wb_data_i : in std_logic_vector(c_wishbone_data_width-1 downto 0);
    --wb_data_o : out std_logic_vector(c_wishbone_data_width-1 downto 0); 

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

    -- read FSM signals
    cache_we          : in std_logic; -- write enable
    data_cache_empty  : out std_logic;

    -- write FSM signals
    cache_re          : in std_logic; -- read enable
    data_cache_full   : out std_logic


    
  );
end entity;

architecture rtl of wb_dma_ch_rf is

signal s_desc_csr_sz  : std_logic_vector(c_wishbone_data_width-1 downto 0) := (others => '0');
signal s_desc_addr0   : std_logic_vector(c_wishbone_data_width-1 downto 0) := (others => '0');
signal s_desc_addr1   : std_logic_vector(c_wishbone_data_width-1 downto 0) := (others => '0');
signal s_pointer      : std_logic_vector(c_wishbone_address_width-1 downto 0) := (others => '0');

signal s_write_addr   : std_logic_vector(c_wishbone_address_width-1 downto 0) := (others => '0');
signal s_read_op      : std_logic_vector((2*c_wishbone_address_width)-1 downto 0) := (others => '0');
signal r_hold_op      : std_logic_vector((2*c_wishbone_address_width)-1 downto 0) := (others => '0'); -- register not possible?

begin

data_and_addr_cache : generic_sync_fifo
generic map (
  g_data_width => 64,
  g_size => g_data_cache_size
)
port map (
  clk_i => clk_i, 
  rst_n_i => rstn_i,
  
  d_i   => (others => '0'),
  we_i  => cache_we,

  q_o   => open,
  rd_i  => cache_re,

  empty_o => data_cache_empty,
  full_o => data_cache_full,
  almost_empty_o => open,
  almost_full_o  => open,
  count_o        => open
);

end architecture;