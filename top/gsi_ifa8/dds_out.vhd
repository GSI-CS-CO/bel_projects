---------------------------------------------------------------------------------
-- filename: dds_out.vhd
-- desc:
-- creation date: 08.12.2017
-- last modified:
-- author: Stefan Rauch <s.rauch@gsi.de>
--
-- Copyright (C) 2017 GSI Helmholtz Centre for Heavy Ion Research GmbH 
--
---------------------------------------------------------------------------------
-- This library is free software; you can redistribute it and/or
-- modify it under the terms of the GNU Lesser General Public
-- License as published by the Free Software Foundation; either
-- version 3 of the License, or (at your option) any later version.
--
-- This library is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- Lesser General Public License for more details.
--  
-- You should have received a copy of the GNU Lesser General Public
-- License along with this library. If not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------- 
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity dds_out is
  port (
    clk:          in std_logic;                       -- system clk  
    reset:        in std_logic;                       -- reset for system clk
    fg_mode:      in std_logic;                       -- fg mode active ?
    fg_dds_mode:  in std_logic;                       -- fg dds mode active ?
    fg_new_data:  in std_logic;                       -- strobe from fg macro
    fg:           in std_logic_vector(31 downto 8);   -- 24 MSB from fg macro
    
    new_data:     out std_logic;                      -- strobe signal to VG connector
    fg_dds:       out std_logic_vector(15 downto 0)   -- data to VG connector
  );
end entity;

architecture arch of dds_out is
  constant adr_data_byte1:  std_logic_vector(7 downto 0) := x"04";
  constant adr_data_byte2:  std_logic_vector(7 downto 0) := x"05";
  constant adr_data_byte3:  std_logic_vector(7 downto 0) := x"06";
  constant adr_latch:       std_logic_vector(7 downto 0) := x"80";
  
  signal s_new_data:  std_logic;
  signal s_fg_dds:    std_logic_vector(15 downto 0);
  
  type dds_sm_type is (idle, byte1, byte1_end, byte2, byte2_end, byte3, byte3_end, byte4, byte4_end);
  signal dds_sm: dds_sm_type;

begin

  -- FIXME : data out register has to run with 12Mhz (manchester clk) rate
  sm: process(clk)
  begin
    if reset = '1' then
      dds_sm      <= idle;
      s_new_data  <= '0';
      s_fg_dds    <= (others => '0');
    elsif rising_edge(clk) then
      s_new_data  <= '0';
      case dds_sm is
        when idle =>
          if fg_dds_mode = '1' and fg_new_data = '1' then -- there is new data form the fg which has to be serialized
            dds_sm <= byte1;
          end if;
          
        when byte1 =>
          s_fg_dds  <= fg(31 downto 24) & adr_data_byte1;  -- data and address of the first byte
          dds_sm    <= byte1_end;
          
        when byte1_end =>
          s_new_data  <= '1';
          dds_sm      <= byte2;
          
        when byte2 =>
          s_fg_dds  <= fg(23 downto 16) & adr_data_byte2;  -- data and address of the second byte
          dds_sm    <= byte2_end;
          
        when byte2_end =>
          s_new_data  <= '1';
          dds_sm      <= byte3;
                  
        when byte3 =>
          s_fg_dds  <= fg(15 downto 8) & adr_data_byte3;  -- data and address of the third byte
          dds_sm    <= byte3_end;
          
        when byte3_end =>
          s_new_data  <= '1';
          dds_sm      <= byte4;
          
        when byte4 =>
          s_fg_dds  <= x"00" & adr_latch;                 -- data and address of the fourth byte
          dds_sm    <= byte4_end;
          
        when byte4_end =>
          s_new_data  <= '1';
          dds_sm      <= idle;
      end case;
    end if;
  end process;
  
  new_data  <= s_new_data;
  fg_dds    <= s_fg_dds;
  
end architecture;
