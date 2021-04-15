--! @file altera_lvds_pkg.vhd
--! @brief LVDS mega-wizard created component definitions
--! @author Wesley W. Terpstra <w.terpstra@gsi.de>
--!
--! Copyright (C) 2013 GSI Helmholtz Centre for Heavy Ion Research GmbH
--!
--! This combines all the common GSI components together
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

library work;
use work.eca_pkg.all;

package altera_lvds_pkg is

  subtype t_lvds_byte is std_logic_vector(7 downto 0); -- 7 goes first
  type t_lvds_byte_array is array(natural range <>) of t_lvds_byte;

  component altera_lvds is
    generic(
      g_family   : string;
      g_inputs   : natural;
      g_outputs  : natural;
      g_tx_multi : boolean := false;
      g_invert   : boolean := false);
    port(
      clk_ref_i    : in  std_logic;
      rstn_ref_i   : in  std_logic;
      clk_lvds_i   : in  std_logic;
      clk_enable_i : in  std_logic;
      dat_o        : out t_lvds_byte_array(g_inputs-1  downto 0);
      lvds_p_i     : in  std_logic_vector(g_inputs-1  downto 0);
      lvds_n_i     : in  std_logic_vector(g_inputs-1  downto 0);
      lvds_i_led_o : out std_logic_vector(g_inputs-1  downto 0);
      dat_i        : in  t_lvds_byte_array(g_outputs-1 downto 0);
      lvds_p_o     : out std_logic_vector(g_outputs-1 downto 0);
      lvds_n_o     : out std_logic_vector(g_outputs-1 downto 0);
      lvds_o_led_o : out std_logic_vector(g_outputs-1 downto 0));
  end component;

  component eca_lvds_channel is
    port(
      clk_i     : in  std_logic;
      rst_n_i   : in  std_logic;
      channel_i : in  t_channel;
      lvds_o    : out t_lvds_byte_array(11 downto 0));
  end component;

  component altera_lvds_ibuf is
    generic(
      g_family : string);
    port(
      datain   : in  std_logic;
      datain_b : in  std_logic;
      dataout  : out std_logic);
  end component;

  component altera_lvds_obuf is
    generic(
      g_family : string);
    port(
      datain           : in  std_logic;
      dataout          : out std_logic;
      dataout_b        : out std_logic);
  end component;

  component altera_lvds_rx is
    generic(
      g_family : string);
    port(
      rx_core    : in  std_logic;
      rx_inclock : in  std_logic;
      rx_enable  : in  std_logic;
      rx_in      : in  std_logic;
      rx_out     : out std_logic_vector(7 downto 0));
  end component;

  component altera_lvds_tx is
    generic(
      g_family : string);
    port(
      tx_core    : in  std_logic;
      tx_inclock : in  std_logic;
      tx_enable  : in  std_logic;
      tx_in      : in  std_logic_vector(7 downto 0);
      tx_out     : out std_logic);
  end component;

  component altera_lvds_tx_multi_scu4 is
    generic(
      g_family : string);
    port(
      tx_core    : in  std_logic;
      tx_inclock : in  std_logic;
      tx_enable  : in  std_logic;
      tx_in      : in  std_logic_vector(23 downto 0);
      tx_out     : out std_logic_vector(2 downto 0));
  end component;

  component altera_lvds_tx_multi_scu4_wrap is
    generic(
      g_family : string);
    port(
      tx_core    : in  std_logic;
      tx_inclock : in  std_logic;
      tx_enable  : in  std_logic;
      tx_in      : in  std_logic_vector(23 downto 0);
      tx_out     : out std_logic_vector(2 downto 0));
  end component;

end package;
