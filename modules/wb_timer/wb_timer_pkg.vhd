--! @file wb_timer_pkg.vhd
--! @brief wb_timer package
--! @author Stefan Rauch <s.rauch@gsi.de>
--!
--! Copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH 
--!
--! This package defines the interface and sdb for the wb_timer core
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

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;

package wb_timer_pkg is

 constant c_wb_timer_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"00",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"4", -- 32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"00000000000000ff",
    product => (
    vendor_id     => x"0000000000000651",
    device_id     => x"d8baaa13",
    version       => x"00000002",
    date          => x"20200224",
    name          => "GSI:WB_TIMER       ")));
    
  component wb_timer is
    generic(
      freq    : natural := 62500000);
    port(
      clk_i   : in  std_logic;
      rst_n_i : in  std_logic;
      slave_i : in  t_wishbone_slave_in;
      slave_o : out t_wishbone_slave_out;
      irq_o   : out std_logic);
  end component;

end package;
