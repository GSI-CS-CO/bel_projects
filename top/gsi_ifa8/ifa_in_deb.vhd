---------------------------------------------------------------------------------
-- filename: ifa_in_deb.vhd
-- desc:
-- creation date: 07.04.2016
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

entity ifa_in_deb is
  port (
    clk:        in std_logic;
    cnt_ena:    in std_logic;
    sclr:       in std_logic;
    sig_i:      in std_logic_vector(7 downto 0);
    sig_o:      out std_logic_vector(7 downto 0));
end entity;

architecture ifa_in_arch of ifa_in_deb is
  component debounce_ifa
    generic
      (
      cnt:     integer := 4 );
    port
      (
       sig:     in std_logic;
       sel:     in std_logic;
       cnt_en:  in std_logic;
       clk:     in std_logic;
       res:     in std_logic;
       sig_deb: out std_logic);
   end component;

begin

  deb_gen:
  for I in 0 to 7 generate
    ena_deb: debounce_ifa
    generic map (cnt => 4)
    port map (  sig => sig_i(I),
                sel => '0',
                cnt_en => cnt_ena,
                clk => clk,
                res => sclr,
                sig_deb => sig_o(I));
  end generate deb_gen;
        
end architecture;
  
