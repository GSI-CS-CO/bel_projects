--! @file arria5_lvds_pkg.vhd
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

package arria5_lvds_pkg is

  component arria5_lvds_ibuf is
    port(
      datain   : in  std_logic_vector(0 downto 0);
      datain_b : in  std_logic_vector(0 downto 0);
      dataout  : out std_logic_vector(0 downto 0));
  end component;
  
  component arria5_lvds_obuf is
    port(
      datain           : in  std_logic_vector(0 downto 0);
      dataout          : out std_logic_vector(0 downto 0);
      dataout_b        : out std_logic_vector(0 downto 0));
  end component;
  
  component arria5_lvds_rx is
    port(
      rx_inclock : in  std_logic;
      rx_enable  : in  std_logic;
      rx_in      : in  std_logic_vector(0 downto 0);
      rx_out     : out std_logic_vector(7 downto 0));
  end component;
  
  component arria5_lvds_tx is
    port(
      tx_inclock : in  std_logic;
      tx_enable  : in  std_logic;
      tx_in      : in  std_logic_vector(7 downto 0);
      tx_out     : out std_logic_vector(0 downto 0));
  end component;
  
  component arriav_pll_lvds_output is
    generic(
      pll_loaden_enable_disable  : string := "true";
      pll_lvdsclk_enable_disable : string := "true");
    port(
      ccout   : in  std_logic_vector(1 downto 0);
      loaden  : out std_logic;
      lvdsclk : out std_logic);
  end component;
  
end package;
