-------------------------------------------------------------------------------
-- Title      : Butis Altera clock alignment
-- Project    : White Rabbit
-------------------------------------------------------------------------------
-- File       : altera_butis.vhd
-- Author     : Wesley W. Terpstra
-- Company    : GSI
-- Created    : 2013-08-23
-- Last update: 2013-08-23
-- Platform   : Altera
-- Standard   : VHDL'93
-------------------------------------------------------------------------------
-- Description: Aligns a 200 MHz clock to the PPS
-------------------------------------------------------------------------------
--
-- Copyright (c) 2013 GSI / Wesley W. Terpstra
--
-- This source file is free software; you can redistribute it   
-- and/or modify it under the terms of the GNU Lesser General   
-- Public License as published by the Free Software Foundation; 
-- either version 2.1 of the License, or (at your option) any   
-- later version.                                               
--
-- This source is distributed in the hope that it will be       
-- useful, but WITHOUT ANY WARRANTY; without even the implied   
-- warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR      
-- PURPOSE.  See the GNU Lesser General Public License for more 
-- details.                                                     
--
-- You should have received a copy of the GNU Lesser General    
-- Public License along with this source; if not, download it   
-- from http://www.gnu.org/licenses/lgpl-2.1.html
-- 
--
-------------------------------------------------------------------------------
-- Revisions  :
-- Date        Version  Author    Description
-- 2013-08-23  1.0      terpstra  First stab at state machine
-- 2013-09-24  1.1      terpstra  Move phase shifting to a general purpose core
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.pll_pkg.all;

entity altera_butis is
  port(
    clk_ref_i   : in  std_logic;
    clk_25m_i   : in  std_logic;
    pps_i       : in  std_logic;
    phase_o     : out phase_offset);
end altera_butis;

-- It is not possible to reliably align 200MHz and 125MHz directly.
-- The problem is that periods of 8ns and 5ns have a gcd=1ns.
-- So, any attempt to measure one clock with the other will have
--  some edges which are within 0.5ns of each other. Too fast.
--
-- Instead, we take the following approach:
--   Setup the PLL to output 125MHz, 200MHz, and 25MHz clocks.
--   Require that all are phase aligned when the PLL is locked.
--   This means that every 40ns they have a common rising edge.
--   Once we find the PPS on the 125MHz clock, latch it's (mod 5*8ns) period.
--   Now, compare a 25 MHz toggling flip-flop to the position of the PPS.
--   We can safely inspect the 12.5 MHz toggle signal in the 125MHz domain.
--   It will look like five '1's then five '0's, repeating.
--   We KNOW that the rising (and falling) edge of the toggle line-up
--     with the 200MHz clock, because the PLL starts up locked this way.
--   Thus, whatever shift would line up the 25MHz signal will also line
--     up the 200MHz signal. Output this to the PLL phase controller.

architecture rtl of altera_butis is

  signal toggle_25m  : std_logic;
  signal clk25_shift : std_logic_vector(4 downto 0);
  signal clk25_reg   : std_logic_vector(4 downto 0);
  signal pps_count   : unsigned(2 downto 0);
  signal r_pps       : std_logic;
          
begin

  toggle : process(clk_25m_i) is
  begin
    if rising_edge(clk_25m_i) then
      toggle_25m <= not toggle_25m;
    end if;
  end process;

  sample : process(clk_ref_i) is
  begin
    if rising_edge(clk_ref_i) then
      clk25_shift <= clk25_shift(clk25_shift'length-2 downto 0) & toggle_25m;
      
      r_pps <= pps_i;
      if (pps_i = '1' and r_pps = '0') or pps_count = 0 then
        pps_count <= to_unsigned(4, pps_count'length);
      else
        pps_count <= pps_count - 1;
      end if;
      
      if pps_count = 0 then
        clk25_reg <= clk25_shift;
      end if;
    end if;
  end process;
  
  -- Phase offsets are 1/8th VCO. At 1GHZ, we need 8 steps to move 1ns.
  phase : process(clk_ref_i) is
  begin
    if rising_edge(clk_ref_i) then
      case clk25_reg is
        when "00000" => phase_o <= to_unsigned((0*8 mod 5)*8, phase_o'length); 
        when "00001" => phase_o <= to_unsigned((1*8 mod 5)*8, phase_o'length); 
        when "00011" => phase_o <= to_unsigned((2*8 mod 5)*8, phase_o'length); 
        when "00111" => phase_o <= to_unsigned((3*8 mod 5)*8, phase_o'length); 
        when "01111" => phase_o <= to_unsigned((4*8 mod 5)*8, phase_o'length); 
        when "11111" => phase_o <= to_unsigned((0*8 mod 5)*8, phase_o'length); 
        when "11110" => phase_o <= to_unsigned((1*8 mod 5)*8, phase_o'length); 
        when "11100" => phase_o <= to_unsigned((2*8 mod 5)*8, phase_o'length); 
        when "11000" => phase_o <= to_unsigned((3*8 mod 5)*8, phase_o'length); 
        when "10000" => phase_o <= to_unsigned((4*8 mod 5)*8, phase_o'length); 
        when others  => phase_o <= (others => '-'); -- impossible; optimize however
      end case;
    end if;
  end process;
  
end rtl;
