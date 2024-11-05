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

    -- channel control signals
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
end entity;

architecture rtl of wb_dma_ch_rf is

signal r_desc_csr_and_sz  : std_logic_vector(c_wishbone_data_width-1 downto 0);
signal r_desc_addr0       : std_logic_vector(c_wishbone_data_width-1 downto 0);
signal r_desc_addr1       : std_logic_vector(c_wishbone_data_width-1 downto 0);
signal r_pointer          : std_logic_vector(c_wishbone_address_width-1 downto 0);

signal r_desc_valid       : std_logic := '0';

signal s_read_addr    : std_logic_vector(c_wishbone_address_width-1 downto 0);
signal s_write_addr   : std_logic_vector(c_wishbone_address_width-1 downto 0);
signal s_read_op      : std_logic_vector((2*c_wishbone_address_width)-1 downto 0);
signal r_hold_op      : std_logic_vector((2*c_wishbone_address_width)-1 downto 0); -- register not possible?

component generic_sync_fifo is

  generic (
    g_data_width : natural;
    g_size       : natural;
    g_show_ahead : boolean := false;

    -- Read-side flag selection
    g_with_empty        : boolean := true;   -- with empty flag
    g_with_full         : boolean := true;   -- with full flag
    g_with_almost_empty : boolean := false;
    g_with_almost_full  : boolean := false;
    g_with_count        : boolean := false;  -- with words counter

    g_almost_empty_threshold : integer;  -- threshold for almost empty flag
    g_almost_full_threshold  : integer;  -- threshold for almost full flag
    g_register_flag_outputs  : boolean := true
    );

  port (
    rst_n_i : in std_logic := '1';

    clk_i : in std_logic;
    d_i   : in std_logic_vector(g_data_width-1 downto 0);
    we_i  : in std_logic;

    q_o  : out std_logic_vector(g_data_width-1 downto 0);
    rd_i : in  std_logic;

    empty_o        : out std_logic;
    full_o         : out std_logic;
    almost_empty_o : out std_logic;
    almost_full_o  : out std_logic;
    count_o        : out std_logic_vector(f_log2_size(g_size)-1 downto 0)
    );

end component generic_sync_fifo;

begin

p_load_desc: process(clk_i, rstn_i)
begin
  if rstn_i = '0' then
    r_desc_csr_and_sz <= (others => '0');
    r_desc_addr0      <= (others => '0');
    r_desc_addr1      <= (others => '0');
    r_pointer         <= (others => '0');
  elsif rising_edge(clk_i) then
    if s_desc_csr_sz_we = '1' then
      r_desc_csr_and_sz <= data_in;
    end if;
    
    if s_desc_addr0_we = '1' then
      r_desc_addr0 <= data_in;
    end if;

    if s_desc_addr1_we = '1' then
      r_desc_addr1 <= data_in;
    end if;

    if s_pointer_we = '1' then
      r_pointer <= data_in;
    end if;
  end if;
end process;

data_and_addr_cache : generic_sync_fifo
generic map (
  g_data_width => 64,
  g_size => g_data_cache_size,
  g_almost_empty_threshold => 0,
  g_almost_full_threshold => 0
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