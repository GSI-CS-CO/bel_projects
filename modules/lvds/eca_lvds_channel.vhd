--! @file eca_lvds_channel.vhd
--! @brief ECA-LVDS channel adapter
--! @author Wesley W. Terpstra <w.terpstra@gsi.de>
--!
--! Copyright (C) 2014 GSI Helmholtz Centre for Heavy Ion Research GmbH 
--!
--! This component takes an action channel and turns it into a LVDS controller.
--! The 32-bit tag is (5-bit reserved, 3-bit offset, 12-bit clear, 12-bit set).
--! When both clear and set appear, the output is instead toggled.
--! The high bits of the tef field + the offset are added to create the delay.
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

use work.wishbone_pkg.all;
use work.eca_pkg.all;
use work.altera_lvds_pkg.all;

entity eca_lvds_channel is
  port(
    clk_i     : in  std_logic;
    rst_n_i   : in  std_logic;
    channel_i : in  t_channel;
    lvds_o    : out t_lvds_byte_array(11 downto 0));
end eca_lvds_channel;

architecture rtl of eca_lvds_channel is
  constant c_width     : natural := 12;
  constant c_frac_bits : natural := 3;
  constant c_frac_len  : natural := 2**(c_frac_bits+1);
  
  subtype t_lvds_shifter is std_logic_vector(c_frac_len-1 downto 0);
  type t_lvds_shifters is array(natural range <>) of t_lvds_shifter;
  
  signal r_lvds : t_lvds_shifters(c_width-1 downto 0) := (others => (others => '0'));
begin

  main : process(clk_i) is
    variable v_set  : std_logic_vector(c_width-1 downto 0);
    variable v_clr  : std_logic_vector(c_width-1 downto 0);
    variable v_off1 : unsigned(c_frac_bits downto 0);
    variable v_off2 : unsigned(c_frac_bits downto 0);
    variable v_off  : unsigned(c_frac_bits downto 0);
    variable v_setv : t_lvds_shifters(c_width-1 downto 0);
    variable v_clrv : t_lvds_shifters(c_width-1 downto 0);
    variable v_lvds : t_lvds_shifters(c_width-1 downto 0);
  begin
    if rising_edge(clk_i) then
      v_clr  := channel_i.tag(c_width+c_width-1 downto c_width);
      v_set  := channel_i.tag(        c_width-1 downto       0);
      v_off1 := "0" & unsigned(channel_i.tag(31 downto 32-c_frac_bits));
      v_off2 := "0" & unsigned(channel_i.tef(31 downto 32-c_frac_bits));
      v_off  := v_off1 + v_off2;
      
      for j in 0 to c_frac_len-1 loop
        if channel_i.valid = '1' and c_frac_len-j > to_integer(v_off) then
          for i in 0 to c_width-1 loop
            v_setv(i)(j) := v_set(i);
            v_clrv(i)(j) := v_clr(i);
          end loop;
        else
          for i in 0 to c_width-1 loop
            v_setv(i)(j) := '0';
            v_clrv(i)(j) := '0';
          end loop;
        end if;
      end loop;
      
      for i in 0 to c_width-1 loop
        v_lvds(i)(c_frac_len-1 downto 8) := r_lvds(i)(c_frac_len-9 downto 0);
        v_lvds(i)(7 downto 0) := (others => r_lvds(i)(0));
      
        r_lvds(i) <= ((not v_setv(i)) and (not v_clrv(i)) and (    v_lvds(i))) or -- unmodified
                     ((    v_setv(i)) and (    v_clrv(i)) and (not v_lvds(i))) or -- toggled
                     ((    v_setv(i)) and (not v_clrv(i)));                    -- set
      end loop;
    end if;
  end process;
  
  -- Output the high bits
  outs : for i in 0 to c_width-1 generate
    lvds_o(i) <= r_lvds(i)(c_frac_len-1 downto c_frac_len-8);
  end generate;

end rtl;
