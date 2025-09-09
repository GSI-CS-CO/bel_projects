-------------------------------------------------------------------------------
-- Title      : Test slave
-- Project    : all Arria platforms
-------------------------------------------------------------------------------
-- File       : virtualRAM.vhd
-- Author     : Lucas Herfurth
-- Company    : GSI
-- Created    : 2024-09-04
-- Last update: 2024-09-04
-- Platform   : Altera
-- Standard   : VHDL'93
-------------------------------------------------------------------------------
-- Description: Virtual RAM module to test DMA.
--
--
-------------------------------------------------------------------------------
--
-- Copyright (c) 2024 GSI / Lucas Herfurth
--
-- This source file is free software; you can redistribute it
-- and/or modify it under the terms of the GNU Lesser General
-- Public License as published by the Free Software Foundation;
-- either version 2.1 of the License, or (at your option) any
-- later version.
--
-- This source is distributed in the hope that it will be
-- useful, but WITHOUT ANY WARRANTY; without even the implied
-- warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
-- PURPOSE.  See the GNU Lesser General Public License for more
-- details.
--
-- You should have received a copy of the GNU Lesser General
-- Public License along with this source; if not, download it
-- from http://www.gnu.org/licenses/lgpl-2.1.html

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.gencores_pkg.all;
use work.genram_pkg.all;

entity wb_dma_RAM is
  generic (
    g_size    : natural
  );

  port(
    clk_sys_i : in std_logic;
    rst_n_i   : in std_logic;

    wb_slave_i : in  t_wishbone_slave_in;
    wb_slave_o : out t_wishbone_slave_out;

    proc_slave_i : in  t_wishbone_slave_in;
    proc_slave_o : out t_wishbone_slave_out
  );
end entity;

architecture rtl of wb_dma_RAM is

begin

  RAM1 : xwb_dpram
  generic map (
    g_size                  => g_size,
    g_init_file             => "../../../modules/wb_dma/descriptor.mif",
    g_must_have_init_file   => false,
    g_slave1_interface_mode => PIPELINED,
    g_slave1_granularity    => BYTE
  )

  port map (
    clk_sys_i => clk_sys_i,
    rst_n_i   => rst_n_i,

    -- the two ports don't seem to behave identically. When swapped the DMA doesn't work as expected.
    slave1_i => wb_slave_i,
    slave1_o => wb_slave_o,
    slave2_i => proc_slave_i,
    slave2_o => proc_slave_o
  );

end architecture;