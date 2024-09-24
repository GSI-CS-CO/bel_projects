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

begin

    wb_dma_engine
    port map (
        clk_i => clk_sys_i,
        rstn_i => rstn_sys_i,

        -- read logic
        s_queue_full  => s_queue_full,
        s_write_data_cache_enable => s_write_data_cache_enable
    );    

    wb_dma_ch_rf
    generic map (
        g_data_cache_size => 16,
        g_read_cache_size => 8
    )
    port map (
        clk_i => clk_sys_i,
        rstn_i => rstn_sys_i,
    
        -- module IOs
        wb_data_i => master_i,
        wb_data_o => master_o,
    
        -- read FSM signals
        cache_we          => in std_logic, -- write enable
        data_cache_empty  => out std_logic,
    
        -- write FSM signals
        cache_re          => in std_logic, -- read enable
        data_cache_full   => out std_logic  
    );
end architecture;