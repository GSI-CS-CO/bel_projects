--! @file iodir.vhd
--! @brief A quick hack to support bidirectional IO
--! @author Wesley W. Terpstra <w.terpstra@gsi.de>
--!
--! Copyright (C) 2013 GSI Helmholtz Centre for Heavy Ion Research GmbH 
--!
--! In order to facilitate testing of new cards, we must control IO direction.
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
use work.monster_pkg.all;

entity monster_iodir is
  generic(
    g_gpio_inout : natural := 0;
    g_lvds_inout : natural := 0);
  port(
    clk_i      : in  std_logic;
    rst_n_i    : in  std_logic;
    slave_i    : in  t_wishbone_slave_in;
    slave_o    : out t_wishbone_slave_out;
    gpio_oen_o : out std_logic_vector(f_sub1(g_gpio_inout) downto 0);
    lvds_oen_o : out std_logic_vector(f_sub1(g_lvds_inout) downto 0));
end monster_iodir;

architecture rtl of monster_iodir is
  signal r_ack  : std_logic := '0';
  signal r_dat  : t_wishbone_data := (others => '0');
  signal r_gpio : std_logic_vector(f_sub1(g_gpio_inout) downto 0) := (others => '1');
  signal r_lvds : std_logic_vector(f_sub1(g_lvds_inout) downto 0) := (others => '1');
begin

  slave_o.dat   <= r_dat;
  slave_o.ack   <= r_ack;
  slave_o.err   <= '0';
  slave_o.stall <= '0';
  
  -- to be removed:
  slave_o.int <= '0';
  slave_o.rty <= '0';
  
  gpio_oen_o <= r_gpio;
  lvds_oen_o <= r_lvds;

  main : process(clk_i, rst_n_i) is
  begin
    if rst_n_i = '0' then
      r_ack  <= '0';
      r_dat  <= (others => '0');
      r_gpio <= (others => '1');
      r_lvds <= (others => '1');
    elsif rising_edge(clk_i) then
      r_ack <= slave_i.cyc and slave_i.stb;
      r_dat <= (others => '0');
      
      if (slave_i.cyc and slave_i.stb and slave_i.we) = '1' then
        case slave_i.adr(3 downto 2) is
          when "00" => r_gpio <= not f_wb_wr(not r_gpio, slave_i.dat, slave_i.sel);
          when "01" => r_lvds <= not f_wb_wr(not r_lvds, slave_i.dat, slave_i.sel);
          when others => null;
        end case;
      end if;
      
      case slave_i.adr(3 downto 2) is
        when "00" => r_dat(r_gpio'range) <= not r_gpio;
        when "01" => r_dat(r_lvds'range) <= not r_lvds;
        when others => null;
      end case;
    end if;
  end process;
  
end rtl;
