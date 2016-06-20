--! @file mbox.vhd
--! @brief Watchdog entity
--! @author Wesley W. Terpstra <w.terpstra@gsi.de>
--!
--! Copyright (C) 2016 GSI Helmholtz Centre for Heavy Ion Research GmbH 
--!
--! This WB slave can be used to pass MSIs from one master to another.
--! Interface is stateless, the low (4B aligned) address bit is used to configure or send an MSI.
--! 0x0: Send MSI
--! 0x4: Configure MSI
--!
--! Example:
--!
--! WB Write, Mbox AdrOffset 0x1C, 0x1000 
--! -> Configure Mbox Adr 0x18 to point to 0x1000 MSI
--! WB Write, Mbox AdrOffset 0x18, 0xcafe
--! -> Mbox sends "0xcafe" to MSI adr 0x1000
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

entity mbox is
  port(
    clk_i        : in  std_logic;
    rst_n_i      : in  std_logic;
    bus_slave_i  : in  t_wishbone_slave_in;
    bus_slave_o  : out t_wishbone_slave_out;
    msi_master_o : out t_wishbone_master_out;
    msi_master_i : in  t_wishbone_master_in);
end mbox;

architecture rtl of mbox is

  signal r_cyc : std_logic := '0';
  signal r_stb : std_logic := '0';
  signal r_ack : std_logic := '0';
  signal r_adr : t_wishbone_address;
  signal r_dat : t_wishbone_data;
  
  signal s_we     : std_logic;
  signal s_port   : t_wishbone_address;
  signal s_target : t_wishbone_address;
  
begin

  s_we   <= bus_slave_i.cyc and bus_slave_i.stb and bus_slave_i.we and bus_slave_i.adr(2);
  s_port <= bus_slave_i.adr when r_cyc='0' else r_adr;
  
  ram : generic_spram
    generic map(
      g_data_width       => 32,
      g_size             => 256,
      g_with_byte_enable => true,
      g_init_file        => "../../../modules/mbox/mbox_init.mif")
    port map(
      rst_n_i => rst_n_i,
      clk_i   => clk_i,
      bwe_i   => bus_slave_i.sel,
      we_i    => s_we,
      a_i     => s_port(10 downto 3),
      d_i     => bus_slave_i.dat,
      q_o     => s_target);

  main : process(clk_i, rst_n_i) is
  begin
    if rst_n_i = '0' then
      r_cyc <= '0';
      r_stb <= '0';
      r_ack <= '0';
      r_adr <= (others => '-');
      r_dat <= (others => '-');
    elsif rising_edge(clk_i) then
      if msi_master_i.ack = '1' then
        r_cyc <= '0';
      end if;
      if msi_master_i.stall = '0' then
        r_stb <= '0';
      end if;
      r_ack <= '0';
      
      if r_cyc = '0' then
        r_adr <= bus_slave_i.adr;
        r_dat <= bus_slave_i.dat;
      end if;
      
      if (bus_slave_i.cyc and bus_slave_i.stb and not r_cyc) = '1' then
        r_ack <= '1';
        if bus_slave_i.we = '1' and bus_slave_i.adr(2) = '0' then
          r_cyc <= '1';
          r_stb <= '1';
        end if;
      end if;
    end if;
  end process;
  
  msi_master_o.cyc <= r_cyc;
  msi_master_o.stb <= r_stb;
  msi_master_o.adr <= s_target;
  msi_master_o.we  <= '1';
  msi_master_o.sel <= (others => '1');
  msi_master_o.dat <= r_dat;
  
  bus_slave_o.ack   <= r_ack;
  bus_slave_o.err   <= '0';
  bus_slave_o.stall <= r_cyc;
  bus_slave_o.dat   <= s_target;

end rtl;
