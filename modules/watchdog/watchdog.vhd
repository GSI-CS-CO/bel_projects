--! @file watchdog.vhd
--! @brief Watchdog entity
--! @author Wesley W. Terpstra <w.terpstra@gsi.de>
--!
--! Copyright (C) 2016 GSI Helmholtz Centre for Heavy Ion Research GmbH 
--!
--! This entity can be used to acquire a lock on hardware that times out.
--! To acquire the lock:
--!    1. Z = read the device
--!    2. if (Z & 0xFFFF) != 0, fail (result = time in seconds till retry)
--!    3. write Z to the device
--!    4. Y = read the device
--!    5. if the ((Z ^ Y)) >> 16 != 0, fail
--!    6. every second, write Z, or you will lose control of the device
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

entity watchdog is
  generic(
    freq    : natural := 62500000;
    hold_s  : natural := 5);
  port(
    clk_i   : in  std_logic;
    rst_n_i : in  std_logic;
    slave_i : in  t_wishbone_slave_in;
    slave_o : out t_wishbone_slave_out);
end watchdog;

architecture rtl of watchdog is

  constant c_count_bits : natural := f_ceil_log2(freq);
  constant c_counter_reset : unsigned := to_unsigned(freq-1, c_count_bits);
  constant c_timeout_reset : unsigned := to_unsigned(hold_s, 16);

  signal r_ack     : std_logic := '0';
  signal r_owner   : unsigned(15 downto 0) := (others => '0');
  signal r_timeout : unsigned(15 downto 0) := c_timeout_reset;
  signal r_counter : unsigned(c_count_bits-1 downto 0) := c_counter_reset;
  
  signal s_owner   : unsigned(15 downto 0);
  signal s_tick    : boolean;
  signal s_idle    : boolean;

begin

  slave_o.ack   <= r_ack;
  slave_o.err   <= '0';
  slave_o.stall <= '0';
  slave_o.dat   <= std_logic_vector(r_owner) & std_logic_vector(r_timeout);
  
  s_owner <= unsigned(slave_i.dat(31 downto 16));
  s_tick  <= r_counter = 0;
  s_idle  <= r_timeout = 0;
  
  main : process(rst_n_i, clk_i) is
  begin
    if rst_n_i = '0' then
      r_ack     <= '0';
      r_owner   <= (others => '0');
      r_timeout <= c_timeout_reset;
      r_counter <= c_counter_reset;
    elsif rising_edge(clk_i) then
      if s_tick then
        r_counter <= c_counter_reset;
        if not s_idle then
          r_timeout <= r_timeout - 1;
        end if;
      else
        r_counter <= r_counter - 1;
      end if;
      
      r_ack <= slave_i.cyc and slave_i.stb;
      
      if (slave_i.cyc and slave_i.stb) = '1' then
        -- Grant access to the watchdog?
        if s_idle then
          if slave_i.we = '0' then
            -- Assign a distinct owner number
            r_owner <= r_owner + 1;
          else
            r_owner   <= s_owner;
            r_timeout <= c_timeout_reset;
          end if;
        else
          -- Refresh the count-down
          if slave_i.we = '1' and r_owner = s_owner then
            r_timeout <= c_timeout_reset;
          end if;
        end if;
      end if;
    end if;
  end process;

end rtl;
