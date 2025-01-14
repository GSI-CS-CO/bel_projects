--! @file        wb_dma_slave.xml
--  DesignUnit   wb_dma_slave
--! @author      M. Kreider <>
--! @date        14/01/2025
--! @version     0.0.1
--! @copyright   2025 GSI Helmholtz Centre for Heavy Ion Research GmbH
--!

--TODO: This is a stub, finish/update it yourself
--! @brief *** ADD BRIEF DESCRIPTION HERE ***
--!
--------------------------------------------------------------------------------
--! This library is free software; you can redistribute it and/or
--! modify it under the terms of the GNU Lesser General Public
--! License as published by the Free Software Foundation; either
--! version 3 of the License, or (at your option) any later version.
--!
--! This library is distributed in the hope that it will be useful,
--! but WITHOUT ANY WARRANTY; without even the implied warranty of
--! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
--! Lesser General Public License for more details.
--!
--! You should have received a copy of the GNU Lesser General Public
--! License along with this library. If not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;
use work.wbgenplus_pkg.all;
use work.genram_pkg.all;
use work.wb_dma_slave_auto_pkg.all;

entity wb_dma_slave is
generic(
  g_channels  : natural := 16 --Number of DMA channels
);
Port(
  clk_sys_i             : std_logic;                                        -- Clock input for sys domain
  rst_sys_n_i           : std_logic;                                        -- Reset input (active low) for sys domain
  error_i               : in  std_logic_vector(1-1 downto 0);               -- Error control
  stall_i               : in  std_logic_vector(1-1 downto 0);               -- flow control
  ch_sel_o              : out std_logic_vector(8-1 downto 0);               -- Channel select register
  channel_csr_o         : out matrix(g_channels-1 downto 0, 32-1 downto 0); -- DMA channel CSR
  descr_queue_intake_o  : out std_logic_vector(32-1 downto 0);              -- DMA channel descriptor queue intake
  dma_csr_o             : out std_logic_vector(32-1 downto 0);              -- DMA controller control and status register
  
  data_i                : in  t_wishbone_slave_in;
  data_o                : out t_wishbone_slave_out

  
);
end wb_dma_slave;

architecture rtl of wb_dma_slave is

  signal s_data_error_i               : std_logic_vector(1-1 downto 0)                := (others => '0'); -- Error control
  signal s_data_stall_i               : std_logic_vector(1-1 downto 0)                := (others => '0'); -- flow control
  signal s_data_dma_csr_o             : std_logic_vector(32-1 downto 0)               := (others => '0'); -- DMA controller control and status register
  signal s_data_channel_csr_o         : matrix(g_channels-1 downto 0, 32-1 downto 0)  := (others => (others => '0')); -- DMA channel CSR
  signal s_data_descr_queue_intake_o  : std_logic_vector(32-1 downto 0)               := (others => '0'); -- DMA channel descriptor queue intake
  signal s_data_ch_sel_o              : std_logic_vector(8-1 downto 0)                := (others => '0'); -- Channel select register
  


begin

  INST_wb_dma_slave_auto : wb_dma_slave_auto
  port map (
    clk_sys_i             => clk_sys_i,
    rst_sys_n_i           => rst_sys_n_i,
    error_i               => s_data_error_i,
    stall_i               => s_data_stall_i,
    dma_csr_o             => s_data_dma_csr_o,
    channel_csr_o         => s_data_channel_csr_o,
    descr_queue_intake_o  => s_data_descr_queue_intake_o,
    ch_sel_o              => s_data_ch_sel_o,
    data_i                => data_i,
    data_o                => data_o  );
end rtl;
