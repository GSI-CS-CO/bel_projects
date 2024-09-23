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
    
        slave_o       : out t_wishbone_slave_out;
        slave_i       : in  t_wishbone_slave_in;
    
        enc_err_i     : in std_logic;
        enc_err_aux_i : in std_logic
        );
end entity;

architecture rtl of wb_dma is

begin

    wb_dma_engine
    generic map (
        
    )
    port map (
        
    );    

end architecture;