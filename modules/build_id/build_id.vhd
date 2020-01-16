--! @file build_id.vhd
--! @brief Build ID entity
--! @author Wesley W. Terpstra <w.terpstra@gsi.de>
--!
--! Copyright (C) 2013 GSI Helmholtz Centre for Heavy Ion Research GmbH
--!
--! This entity just wraps a MIF into a ROM using generic rams.
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
use work.genram_pkg.all;

entity build_id is
  port(
    clk_i   : in  std_logic;
    rst_n_i : in  std_logic;
    slave_i : in  t_wishbone_slave_in;
    slave_o : out t_wishbone_slave_out);
end build_id;

architecture rtl of build_id is

  signal r_ack : std_logic := '0';

begin

  rom : generic_simple_dpram
    generic map(
      g_data_width               => 32,
      g_size                     => 256,
      g_with_byte_enable         => false,
      g_addr_conflict_resolution => "dont_care",
      g_init_file                => "build_id.mif",
      g_dual_clock               => false)
    port map(
      rst_n_i => rst_n_i,
      clka_i  => clk_i,
      bwea_i  => (others => '1'),
      wea_i   => '0',
      aa_i    => (others => '0'),
      da_i    => (others => '0'),
      clkb_i  => clk_i,
      ab_i    => slave_i.adr(9 downto 2),
      qb_o    => slave_o.dat);

  slave_o.ack <= r_ack;
  slave_o.err <= '0';
  slave_o.stall <= '0';

  -- to be removed:
  slave_o.rty <= '0';

  main : process(clk_i, rst_n_i) is
  begin
    if rst_n_i = '0' then
      r_ack <= '0';
    elsif rising_edge(clk_i) then
      r_ack <= slave_i.cyc and slave_i.stb;
    end if;
  end process;

end rtl;
