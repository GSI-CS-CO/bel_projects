--! @file wb_timer.vhd
--! @brief configurable timer interrupt source 
--! @author Stefan Rauch <s.rauch@gsi.de>
--!
--! Copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH 
--!
--! This module will generate an cyclic pulse with an configurable interval
--! time. It can be started and stopped via configuration register.
--! The configuration interface is mapped via wishbone interface.
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

entity wb_timer is
  generic(
    freq    : natural := 62500000);
  port(
    clk_i   : in  std_logic;
    rst_n_i : in  std_logic;
    slave_i : in  t_wishbone_slave_in;
    slave_o : out t_wishbone_slave_out;
    irq_o   : out std_logic);
end wb_timer;

architecture arch of wb_timer is

  signal r_ack            : std_logic  := '0';
  signal r_config_reg     : std_logic_vector(31 downto 0);
  signal r_counter_preset : std_logic_vector(31 downto 0);
  

begin

  slave_o.ack   <= r_ack;
  slave_o.err   <= '0';
  slave_o.stall <= '0';
  
  
  reg : process(rst_n_i, clk_i) is
  begin
    if rst_n_i = '0' then
      r_ack            <= '0';
      r_config_reg     <= (others => '0');
      r_counter_preset <= (others => '0');
      slave_o.dat      <= (others => '0');
    elsif rising_edge(clk_i) then
      
      -- this slave never stalls
      r_ack <= slave_i.cyc and slave_i.stb;
      
      if (slave_i.cyc and slave_i.stb) = '1' then
        -- access config register
        if slave_i.adr(3 downto 0) = x"0" and slave_i.we = '1' then
          r_config_reg <= slave_i.dat;

        -- access counter preset register
        elsif slave_i.adr(3 downto 0)= x"4" and slave_i.we = '1' then
          r_counter_preset <= slave_i.dat;
        end if;

        -- read counter registers
        if slave_i.we = '0' then
          if slave_i.adr(3 downto 0) = x"0" then
            slave_o.dat <= r_config_reg;
          elsif slave_i.adr(3 downto 0) = x"4" then
            slave_o.dat <= r_counter_preset;
          end if;
        end if;
      end if;
    end if;
  end process;

  counter : process(rst_n_i, clk_i, r_config_reg, r_counter_preset)
    variable cnt : unsigned(31 downto 0);
  begin
    if rst_n_i = '0' then
      cnt := (others => '0');
      irq_o <= '0';
    elsif rising_edge(clk_i) then
      irq_o <= '0';
      -- cnt enabled
      if r_config_reg(0) = '1' then
        -- use a downcounter with overflow
        cnt := cnt - 1;
        if cnt(cnt'high) = '1' then
          cnt := unsigned(r_counter_preset);
          irq_o <= '1';
        end if;
      end if;
    end if;
  end process;
end arch;
