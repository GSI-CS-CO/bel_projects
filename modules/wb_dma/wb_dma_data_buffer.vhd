library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.genram_pkg.all;

entity wb_dma_data_buffer is
generic(
    g_block_size : integer
);

port(
    clk_i   : in std_logic;
    rstn_i  : in std_logic;

    buffer_empty_o  : out std_logic;
    buffer_full_o   : out std_logic;

    buffer_we_i : in std_logic;
    buffer_rd_i : in std_logic;
    
    rd_master_i     : in t_wishbone_master_in;
    rd_master_snoop : in t_wishbone_master_out;

    wr_master_i     : in t_wishbone_master_in;
    wr_master_snoop : in t_wishbone_master_out;

    -- rd_master_cyc   : in std_logic;
    -- rd_master_ack   : in std_logic;
    -- wr_master_stb   : in std_logic;
    -- wr_master_stall : in std_logic;
    -- buffer_input    : in t_wishbone_data;
    buffer_output   : out t_wishbone_data
);
end entity;

architecture rtl of wb_dma_data_buffer is

begin

data_FIFO : generic_sync_fifo
generic map(
    g_data_width => t_wishbone_data'length,
    g_size => g_block_size+1
)
port map(
    rst_n_i =>  rstn_i,
    clk_i   => clk_i,

    d_i     => rd_master_i.dat,
    we_i    => buffer_we_i,

    q_o     => buffer_output,
    rd_i    => buffer_rd_i,

    empty_o => buffer_empty_o,
    full_o  => buffer_full_o,
    almost_empty_o  => open,
    almost_full_o   => open,
    count_o         => open
);

end architecture;
