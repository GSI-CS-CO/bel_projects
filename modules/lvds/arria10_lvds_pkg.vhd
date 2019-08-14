--! @file arria10_lvds_pkg.vhd
--! @brief LVDS mega-wizard created component definitions
--! @author A. Hahn <a.hahn@gsi.de>
--!
--! Copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
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

package arria10_lvds_pkg is

  component arria10_scu4_lvds_ibuf is
    port(
      datain   : in  std_logic_vector(0 downto 0);
      datain_b : in  std_logic_vector(0 downto 0);
      dataout  : out std_logic_vector(0 downto 0));
  end component;

  component arria10_scu4_lvds_obuf is
    port(
      datain           : in  std_logic_vector(0 downto 0);
      dataout          : out std_logic_vector(0 downto 0);
      dataout_b        : out std_logic_vector(0 downto 0));
  end component;

  component arria10_scu4_lvds_rx is
    port(
      rx_inclock : in  std_logic;
      rx_enable  : in  std_logic;
      rx_in      : in  std_logic_vector(0 downto 0);
      rx_out     : out std_logic_vector(7 downto 0));
  end component;

  component arria10_scu4_lvds_tx is
    port(
      tx_inclock : in  std_logic;
      tx_enable  : in  std_logic;
      tx_in      : in  std_logic_vector(7 downto 0);
      tx_out     : out std_logic_vector(0 downto 0));
  end component;

end package;
