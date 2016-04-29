--! @file mbox_pkg.vhd
--! @brief Watchdog package
--! @author Wesley W. Terpstra <w.terpstra@gsi.de>
--!
--! Copyright (C) 2013 GSI Helmholtz Centre for Heavy Ion Research GmbH 
--!
--! This package defines a component which generates MSIs on demand.
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

package mbox_pkg is

 constant c_mbox_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"00",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"00000000000007ff",
    product => (
    vendor_id     => x"0000000000000651",
    device_id     => x"fab0bdd8",
    version       => x"00000001",
    date          => x"20160428",
    name          => "GSI:MSI_MAILBOX    ")));
    
  component mbox is
    port(
      clk_i        : in  std_logic;
      rst_n_i      : in  std_logic;
      bus_slave_i  : in  t_wishbone_slave_in;
      bus_slave_o  : out t_wishbone_slave_out;
      msi_master_o : out t_wishbone_master_out;
      msi_master_i : in  t_wishbone_master_in);
  end component;

end package;
