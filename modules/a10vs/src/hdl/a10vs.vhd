--------------------------------------------------------------------------------
-- Title         : Top level WB module
-- Project       : Wishbone vB.4
--------------------------------------------------------------------------------
-- File          : a10vs.vhd
-- Author        : Enkhbold Ochirsuren
-- Organisation  : GSI, TOS
-- Created       : 2025-03-03
-- Platform      : Arria 10
-- Standard      : VHDL'93
-- Repository    : https://github.com/GSI-CS-CO/bel_projects
--------------------------------------------------------------------------------
--
-- Description: This module contains RTL description that connects the
-- WB slave component together with the voltage sensor IP core component.
-- It provides only the WB slave interface.
--
--------------------------------------------------------------------------------

-- ieee
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- general-cores
library work;
use work.wishbone_pkg.all;

-- gsi
use work.a10vs_pkg.all;

-- Intel IP (qsys)
library a10vs_ip_altera_voltage_sensor_231;
use a10vs_ip_altera_voltage_sensor_231.a10vs_ip_pkg.all;

entity a10vs is
    port (
        -- wishbone syscon
        clk_i   : in  std_logic := '0';
        rst_n_i : in  std_logic := '1';

        -- wishbone slave interface
        slave_i : in  t_wishbone_slave_in;
        slave_o : out t_wishbone_slave_out
    );
end a10vs;

architecture a10vs_rtl of a10vs is

    -- voltage sensor (IP core) interface (Avalon-MM)
    signal s_vs_ctrl_csr_addr     : std_logic;                     -- address
    signal s_vs_ctrl_csr_rd       : std_logic;                     -- read
    signal s_vs_ctrl_csr_wr       : std_logic;                     -- write
    signal s_vs_ctrl_csr_wrdata   : std_logic_vector(31 downto 0); -- writedata
    signal s_vs_ctrl_csr_rddata   : std_logic_vector(31 downto 0); -- readdata
    signal s_vs_sample_csr_addr   : std_logic_vector(3 downto 0);  -- address
    signal s_vs_sample_csr_rd     : std_logic;                     -- read
    signal s_vs_sample_csr_wr     : std_logic;                     -- write
    signal s_vs_sample_csr_wrdata : std_logic_vector(31 downto 0); -- writedata
    signal s_vs_sample_csr_rddata : std_logic_vector(31 downto 0); -- readdata
    signal s_vs_sample_irq        : std_logic;                     -- irq;                                    -- ack for avalon-mm (ensure valid data on avalon bus)
    signal s_rst                  : std_logic;                     -- sink reset

begin

    s_rst <= not rst_n_i;

    a10vs_wb_0: component a10vs_wb
        generic map (
            g_data_size          => 32
        )
        port map (
            clk_i                => clk_i,
            rst_n_i              => rst_n_i,
            slave_i              => slave_i,
            slave_o              => slave_o,
            vs_ctrl_csr_addr     => s_vs_ctrl_csr_addr,
            vs_ctrl_csr_rd       => s_vs_ctrl_csr_rd,
            vs_ctrl_csr_wr       => s_vs_ctrl_csr_wr,
            vs_ctrl_csr_wrdata   => s_vs_ctrl_csr_wrdata,
            vs_ctrl_csr_rddata   => s_vs_ctrl_csr_rddata,
            vs_sample_csr_addr   => s_vs_sample_csr_addr,
            vs_sample_csr_rd     => s_vs_sample_csr_rd,
            vs_sample_csr_wr     => s_vs_sample_csr_wr,
            vs_sample_csr_wrdata => s_vs_sample_csr_wrdata,
            vs_sample_csr_rddata => s_vs_sample_csr_rddata,
            vs_sample_irq        => s_vs_sample_irq
    );

    a10vs_ip_0: component a10vs_ip
        port map (
            clock_clk                  => clk_i,
            controller_csr_address     => s_vs_ctrl_csr_addr,
            controller_csr_read        => s_vs_ctrl_csr_rd,
            controller_csr_write       => s_vs_ctrl_csr_wr,
            controller_csr_writedata   => s_vs_ctrl_csr_wrdata,
            controller_csr_readdata    => s_vs_ctrl_csr_rddata,
            reset_sink_reset           => s_rst,
            sample_store_csr_address   => s_vs_sample_csr_addr,
            sample_store_csr_read      => s_vs_sample_csr_rd,
            sample_store_csr_write     => s_vs_sample_csr_wr,
            sample_store_csr_writedata => s_vs_sample_csr_wrdata,
            sample_store_csr_readdata  => s_vs_sample_csr_rddata,
            sample_store_irq_irq       => s_vs_sample_irq
	);

end a10vs_rtl;