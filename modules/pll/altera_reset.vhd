-------------------------------------------------------------------------------
-- Title      : Cleanly reset PLLs at power-on
-- Project    : White Rabbit
-------------------------------------------------------------------------------
-- File       : altera_reset.vhd
-- Author     : Wesley W. Terpstra
-- Company    : GSI
-- Created    : 2013-08-23
-- Last update: 2013-08-23
-- Platform   : Altera
-- Standard   : VHDL'93
-------------------------------------------------------------------------------
-- Description: Synchronize PLLs and generate reset lines
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
-- 2013-09-16  1.0      terpstra  First version
-------------------------------------------------------------------------------


library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;

entity altera_reset is
  generic(
    g_plls     : natural := 4;
    g_clocks   : natural := 2;
    g_areset   : natural := 1024;  -- length of pll_arst_o
    g_stable   : natural := 1024); -- duration locked must be stable
  port(
    clk_free_i : in  std_logic; -- external free running clock
    rstn_i     : in  std_logic; -- external reset button
    pll_lock_i : in  std_logic_vector(g_plls-1 downto 0);
    pll_arst_o : out std_logic; -- reset analog lock
    clocks_i   : in  std_logic_vector(g_clocks-1 downto 0);
    rstn_o     : out std_logic_vector(g_clocks-1 downto 0));
end altera_reset;

architecture rtl of altera_reset is

  function f_max(a : natural; b : natural) return natural is
  begin
    if a > b then return a; else return b; end if;
  end function;

  constant ones  : std_logic_vector(g_plls-1 downto 0) := (others => '1');
  constant count_bits : natural := f_ceil_log2(f_max(g_areset, g_stable));
  
  subtype t_count is unsigned(count_bits-1 downto 0);
  subtype t_sync  is std_logic_vector(2 downto 0);
  type t_sync_array is array (natural range <>) of t_sync;

  -- async
  signal s_locked  : std_logic;
  
  -- clk_free registers
  signal locked  : t_sync  := (others => '0');
  signal stable  : boolean := false; -- counter expired
  signal reset   : std_logic := '1';   -- async reset of PLL   (g_areset)
  signal waiting : std_logic := '1';   -- waiting for g_stable (g_stable)
  signal count   : t_count := to_unsigned(g_areset-1, t_count'length);
  
  -- clocks_i registers
  signal nresets : t_sync_array(g_clocks-1 downto 0) := (others => (others => '0'));
  
  -- We ensure timing between these nodes via the state machine
  attribute altera_attribute : string;
  attribute altera_attribute OF rtl : architecture is
    ("-name SDC_STATEMENT ""set_false_path                                  -to {*|altera_reset:*|locked[2]}"";"  &
     "-name SDC_STATEMENT ""set_false_path -from {*|altera_reset:*|waiting} -to {*|altera_reset:*|nresets*}""");

begin

  s_locked <= rstn_i when (pll_lock_i = ones) else '0';
  
  -- Reset PLLs and wait till all have locked.
  -- If any PLL loses lock, reset all of them.
  main : process(clk_free_i) is
  begin
    if rising_edge(clk_free_i) then
      locked  <= s_locked & locked(locked'left downto 1);
      stable  <= count = 1;
      
      -- We don't use a traditional state machine here.
      -- This code has to be clk_free_i glitch safe!
      -- Every case has at most 6 inputs (ie: fits in one 6-LUT)
      -- (reset, waiting, stable, locked(0), count(i-1))
      
      if reset = '1' then
        if stable then
          reset   <= '0';
          waiting <= '1';
          count   <= to_unsigned(g_stable-1, t_count'length);
        else
          reset   <= '1';
          waiting <= '1';
          count   <= count - 1;
        end if;
      elsif waiting = '1' then
        if locked(0) = '0' then
          reset   <= '0';
          waiting <= '1';
          count   <= to_unsigned(g_stable-1, t_count'length);
        elsif stable then
          reset   <= '0';
          waiting <= '0';
          count   <= (others => '-');
        else
          reset   <= '0';
          waiting <= '1';
          count   <= count - 1;
        end if;
      else
        if locked(0) = '0' then
          reset   <= '1';
          waiting <= '1';
          count   <= to_unsigned(g_areset-1, t_count'length);
        else
          reset  <= '0';
          count  <= (others => '-');
        end if;
      end if;
    end if;
  end process;
  
  pll_arst_o <= reset;
  
  -- Generate per-clock reset lines
  resets : for i in g_clocks-1 downto 0 generate
    rstn_o(i) <= nresets(i)(0);
    reset : process(waiting, clocks_i(i)) is
    begin
      if waiting = '1' then
        nresets(i) <= (others => '0');
      elsif rising_edge(clocks_i(i)) then
        nresets(i) <= '1' & nresets(i)(t_sync'left downto 1);
      end if;
    end process;
  end generate;

end rtl;
