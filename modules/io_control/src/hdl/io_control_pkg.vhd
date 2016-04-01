--! @file io_control_pkg.vhd
--! @brief Control unit for bidirectional IO and more
--! @author CSCO-TG <csco-tg@gsi.de>
--!
--! Copyright (C) 2015 GSI Helmholtz Centre for Heavy Ion Research GmbH 
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
---------------------------------------------------------------------------------
-- Libraries
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.monster_pkg.all;
use work.altera_lvds_pkg.all;

package io_control_pkg is
  
  -- SERDES INOUT (in case altera_lvds_pkg/t_lvds_byte_array is not available/suitable)
  subtype t_serdes_byte is std_logic_vector(7 downto 0);
  type t_serdes_byte_array is array(natural range <>) of t_serdes_byte;
  
  constant c_io_control_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"00",
    abi_ver_minor => x"00",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"000000000000ffff",
    product => (
    vendor_id     => x"0000000000000651",
    device_id     => x"10C05791",
    version       => x"00000001",
    date          => x"20150916",
    name          => "IO_CONTROL         "))
    );
  
  component io_control is
    generic(
      g_project     : string;
      g_syn_target  : string  := "Simulation";
      g_rom_delay   : natural := 1;
      g_version     : natural := 1;
      g_gpio_in     : natural := 0;
      g_gpio_out    : natural := 0;
      g_gpio_inout  : natural := 0;
      g_lvds_in     : natural := 0;
      g_lvds_out    : natural := 0;
      g_lvds_inout  : natural := 0;
      g_fixed       : natural := 0;
      g_io_table    : t_io_mapping_table_arg_array(natural range <>));
    port(
      clk_i           : in  std_logic;
      rst_n_i         : in  std_logic;
      slave_i         : in  t_wishbone_slave_in;
      slave_o         : out t_wishbone_slave_out;
      gpio_input_i    : in  std_logic_vector(f_sub1(g_gpio_in+g_gpio_inout)   downto 0);
      gpio_output_i   : in  std_logic_vector(f_sub1(g_gpio_out+g_gpio_inout)  downto 0);
      gpio_output_o   : out std_logic_vector(f_sub1(g_gpio_out+g_gpio_inout)  downto 0);
      lvds_input_i    : in  t_lvds_byte_array(f_sub1(g_lvds_in+g_lvds_inout)  downto 0);
      lvds_output_i   : in  t_lvds_byte_array(f_sub1(g_lvds_out+g_lvds_inout) downto 0);
      lvds_output_o   : out t_lvds_byte_array(f_sub1(g_lvds_out+g_lvds_inout) downto 0);
      gpio_oe_o       : out std_logic_vector(f_sub1(g_gpio_inout+g_gpio_out)  downto 0);
      gpio_term_o     : out std_logic_vector(f_sub1(g_gpio_inout+g_gpio_in)   downto 0);
      gpio_spec_out_o : out std_logic_vector(f_sub1(g_gpio_inout+g_gpio_out)  downto 0);
      gpio_spec_in_o  : out std_logic_vector(f_sub1(g_gpio_inout+g_gpio_in)   downto 0);
      gpio_mux_o      : out std_logic_vector(f_sub1(g_gpio_inout+g_gpio_out)  downto 0);
      gpio_sel_o      : out std_logic_vector(f_sub1(g_gpio_inout+g_gpio_out)  downto 0);
      lvds_oe_o       : out std_logic_vector(f_sub1(g_lvds_inout+g_lvds_out)  downto 0);
      lvds_term_o     : out std_logic_vector(f_sub1(g_lvds_inout+g_lvds_in)   downto 0);
      lvds_spec_out_o : out std_logic_vector(f_sub1(g_lvds_inout+g_lvds_out)  downto 0);
      lvds_spec_in_o  : out std_logic_vector(f_sub1(g_lvds_inout+g_lvds_in)   downto 0);
      lvds_mux_o      : out std_logic_vector(f_sub1(g_lvds_inout+g_lvds_out)  downto 0);
      lvds_sel_o      : out std_logic_vector(f_sub1(g_lvds_inout+g_lvds_out)  downto 0));
  end component;
  
end package;
