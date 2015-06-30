--==============================================================================
-- GSI Helmholz center for Heavy Ion Research GmbH
-- SERDES clock generator
--==============================================================================
--
-- author: Theodor Stana (t.stana@gsi.de)
--
-- date of creation: 2015-03-24
--
-- version: 1.0
--
-- description:
--    This module implements a clock generator via a SERDES interface. It drives
--    the data input of a SERDES transceiver with the necessary bit pattern in
--    order to generate a clock at the SERDES data rate.
--
--==============================================================================
-- GNU LESSER GENERAL PUBLIC LICENSE
--==============================================================================
-- This source file is free software; you can redistribute it and/or modify it
-- under the terms of the GNU Lesser General Public License as published by the
-- Free Software Foundation; either version 2.1 of the License, or (at your
-- option) any later version. This source is distributed in the hope that it
-- will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
-- of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
-- See the GNU Lesser General Public License for more details. You should have
-- received a copy of the GNU Lesser General Public License along with this
-- source; if not, download it from http://www.gnu.org/licenses/lgpl-2.1.html
--==============================================================================
-- last changes:
--    2015-03-24   Theodor Stana     File created
--==============================================================================
-- TODO: -
--==============================================================================

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.genram_pkg.all;


entity serdes_clk_gen is
  generic
  (
    g_num_serdes_bits       : natural;
    g_with_frac_counter     : boolean := false;
    g_selectable_duty_cycle : boolean := false
  );
  port
  (
    -- Clock and reset signals
    clk_i                : in  std_logic;
    rst_n_i              : in  std_logic;

    -- Inputs from registers, synchronous to clk_i
    ld_reg_p0_i          : in  std_logic;
    per_i                : in  std_logic_vector(31 downto 0);
    per_hi_i             : in  std_logic_vector(31 downto 0);
    frac_i               : in  std_logic_vector(31 downto 0);
    bit_pattern_normal_i : in  std_logic_vector(31 downto 0);
    bit_pattern_skip_i   : in  std_logic_vector(31 downto 0);

    -- Counter load ports for external synchronization machine
    ld_lo_p0_i           : in  std_logic;
    ld_hi_p0_i           : in  std_logic;
    per_count_i          : in  std_logic_vector(31 downto 0);
    frac_count_i         : in  std_logic_vector(31 downto 0);
    frac_carry_i         : in  std_logic;
    last_bit_i           : in  std_logic;

    -- Data output to SERDES, synchronous to clk_i
    serdes_dat_o         : out std_logic_vector(g_num_serdes_bits-1 downto 0)
  );
end entity serdes_clk_gen;


architecture arch of serdes_clk_gen is

  --============================================================================
  -- Signal declarations
  --============================================================================
  signal per_count_hi   : unsigned(31 downto 0);
  signal per_add_hi     : unsigned(33 downto 0);
  signal frac_count_hi  : unsigned(31 downto 0);
  signal frac_add_hi    : unsigned(32 downto 0);
  signal frac_carry_hi  : std_logic;

  signal patt_hi        : unsigned(31 downto 0);
  signal shpatt_hi      : unsigned(31 downto 0);
  signal bit_pattern_hi : std_logic_vector(g_num_serdes_bits-1 downto 0);

  signal outp_hi        : std_logic_vector(g_num_serdes_bits-1 downto 0);
  signal outp_hi_d0     : std_logic;

  signal per_count_lo   : unsigned(31 downto 0);
  signal per_add_lo     : unsigned(33 downto 0);
  signal frac_count_lo  : unsigned(31 downto 0);
  signal frac_add_lo    : unsigned(32 downto 0);
  signal frac_carry_lo  : std_logic;

  signal patt_lo        : unsigned(31 downto 0);
  signal shpatt_lo      : unsigned(31 downto 0);
  signal bit_pattern_lo : std_logic_vector(g_num_serdes_bits-1 downto 0);

  signal outp_lo        : std_logic_vector(g_num_serdes_bits-1 downto 0);
  signal outp_lo_d0     : std_logic;

--==============================================================================
--  architecture begin
--==============================================================================
begin

--==============================================================================
-- Generate code for fractional division ratio
--==============================================================================
-- The period and fraction counters control flipping of bits on the output
-- port. Flipping of bits is done via the bit pattern presented at the input,
-- and the period and fraction counters control how this bit pattern is shifted
-- so an edge occurs in the clock.
--
-- The period counter decrements on each clk_i period by the number of SERDES bits
-- and its value is used to shift the bit pattern right. When the counter is less
-- than the number of SERDES bits, an edge on the output clock should arrive
-- within the next period, thus the mask should be shifted to the appropriate
-- position of the edge in the data that goes to the SERDES.
--
-- The fraction counter increments on each period by the value set by the
-- input port. The fraction is given on the input port as a value "a", where
-- fraction_value = a / 2**32. When the fraction counter overflows, it
-- generates a carry, which will lead to the introduction of a zero in the bit
-- pattern. Since a zero is introduced by the hardware in the bit pattern, the
-- period counter needs to count one bit less.
--==============================================================================
gen_frac_yes : if (g_with_frac_counter = true) generate
  -- Period subtractor and fractional period adder, both using the fractional
  -- carry as carry in.
  per_add_hi  <= ('0' & per_count_hi & '1') +
                 ('1' & unsigned(to_signed(-g_num_serdes_bits, 32)) & frac_carry_hi);
  frac_add_hi <= ('0' & frac_count_hi) + ('0' & unsigned(frac_i));

  p_counters : process (clk_i, rst_n_i)
  begin
    if (rst_n_i = '0') then
      per_count_hi  <= (others => '0');
      frac_count_hi <= (others => '0');
      frac_carry_hi <= '0';
    elsif rising_edge(clk_i) then
      -- reset the counters on register load
      if (ld_reg_p0_i = '1') then
        per_count_hi  <= (others => '0');
        frac_count_hi <= (others => '0');
      -- synchronizer load
      elsif (ld_hi_p0_i = '1') then
        per_count_hi  <= unsigned(per_count_i);
        frac_count_hi <= unsigned(frac_count_i);
        frac_carry_hi <= frac_carry_i;
      -- period adder overflow
      elsif (per_add_hi(per_add_hi'high) = '1') then
        per_count_hi  <= per_add_hi(32 downto 1) + unsigned(per_i);
        frac_count_hi <= frac_add_hi(frac_count_hi'range);
        frac_carry_hi <= frac_add_hi(frac_add_hi'high);
      else
        per_count_hi  <= per_add_hi(32 downto 1);
        frac_carry_hi <= '0';
      end if;
    end if;
  end process p_counters;

  -- The counter logic is doubled for selectable duty cycles. A second,
  -- phase-shifted clock is generated and the main and secondary clocks are
  -- XORed together (see below) to make up a clock with the selected duty cycle.
  gen_secondary_counters : if (g_selectable_duty_cycle = true) generate
    per_add_lo  <= ('0' & per_count_lo & '1') +
                   ('1' & unsigned(to_signed(-g_num_serdes_bits, 32)) & frac_carry_lo);
    frac_add_lo <= ('0' & frac_count_lo) + ('0' & unsigned(frac_i));

    p_secondary_counters : process (clk_i, rst_n_i)
    begin
      if (rst_n_i = '0') then
        per_count_lo  <= (others => '0');
        frac_count_lo <= (others => '0');
        frac_carry_lo <= '0';
      elsif rising_edge(clk_i) then
        -- reset the counters on register load
        if (ld_reg_p0_i = '1') then
          per_count_lo  <= unsigned(per_hi_i);
          frac_count_lo <= (others => '0');
        -- synchronizer load
        elsif (ld_lo_p0_i = '1') then
          per_count_lo  <= unsigned(per_count_i);
          frac_count_lo <= unsigned(frac_count_i);
          frac_carry_lo <= frac_carry_i;
        -- period adder overflow
        elsif (per_add_lo(per_add_lo'high) = '1') then
          per_count_lo  <= per_add_lo(32 downto 1) + unsigned(per_i);
          frac_count_lo <= frac_add_lo(frac_count_lo'range);
          frac_carry_lo <= frac_add_lo(frac_add_lo'high);
        else
          per_count_lo  <= per_add_lo(32 downto 1);
          frac_carry_lo <= '0';
        end if;
      end if;
    end process p_secondary_counters;

  end generate gen_secondary_counters;

end generate gen_frac_yes;
--==============================================================================

--==============================================================================
-- Generate code for integer division ratio
--==============================================================================
-- When integer division ratio is sufficient for the user's purposes, the extra
-- fractional counter logic is unnecessary and the user can set
-- g_with_frac_counter to false to remove it. If this is the case, only the
-- period counter logic above is produced.
--==============================================================================
gen_frac_no : if (g_with_frac_counter = false) generate

  frac_carry_hi <= '0';

  per_add_hi <= ('0' & per_count_hi & '1') +
                ('1' & unsigned(to_signed(-g_num_serdes_bits, 32)) & frac_carry_hi);

  p_counter : process (clk_i, rst_n_i)
  begin
    if (rst_n_i = '0') then
      per_count_hi  <= (others => '0');
    elsif rising_edge(clk_i) then
      -- reset the counters on register load
      if (ld_reg_p0_i = '1') then
        per_count_hi <= (others => '0');
      -- synchronizer load
      elsif (ld_hi_p0_i = '1') then
        per_count_hi <= unsigned(per_count_i);
      -- period adder overflow
      elsif (per_add_hi(per_add_hi'high) = '1') then
        per_count_hi <= per_add_hi(32 downto 1) + unsigned(per_i);
      else
        per_count_hi <= per_add_hi(32 downto 1);
      end if;
    end if;
  end process p_counter;

  gen_secondary_counter : if (g_selectable_duty_cycle = true) generate

    frac_carry_lo <= '0';

    per_add_lo <= ('0' & per_count_lo & '1') +
                  ('1' & unsigned(to_signed(-g_num_serdes_bits, 32)) & frac_carry_lo);

    p_secondary_counter : process (clk_i, rst_n_i)
    begin
      if (rst_n_i = '0') then
        per_count_lo <= (others => '0');
      elsif rising_edge(clk_i) then
        -- reset the counters on register load
        if (ld_reg_p0_i = '1') then
          per_count_lo <= unsigned(per_hi_i);
        -- synchronizer load
        elsif (ld_lo_p0_i = '1') then
          per_count_lo <= unsigned(per_count_i);
        -- period adder overflow
        elsif (per_add_lo(per_add_lo'high) = '1') then
          per_count_lo <= per_add_lo(32 downto 1) + unsigned(per_i);
        else
          per_count_lo <= per_add_lo(32 downto 1);
        end if;
      end if;
    end process p_secondary_counter;

  end generate gen_secondary_counter;

end generate gen_frac_no;
--==============================================================================

  -- Saturated barrel shifter, shifts the bit pattern by the number of bits indicated by
  -- per_count, or fully if the counter is saturated from the point of view of the
  -- shifter (shift value > SERDES number of bits).
  --
  -- The lower bits of the bit pattern are presented to the XOR chain below prior to
  -- outputting to the SERDES.
  --
  -- This shifted bit pattern then assures that an edge occurs on the clock
  -- output where it's supposed to occur.
  patt_hi   <= unsigned(bit_pattern_skip_i) when (frac_carry_hi = '1') else
               unsigned(bit_pattern_normal_i);
  shpatt_hi <= shift_right(patt_hi, to_integer(per_count_hi(f_log2_size(2*g_num_serdes_bits)-1 downto 0)))
                 when (per_count_hi < 2*g_num_serdes_bits) else
               (others => '0');

  bit_pattern_hi <= std_logic_vector(shpatt_hi(g_num_serdes_bits-1 downto 0));

  -- Output bit-flip based on bit pattern and value of output on prev. cycle
  outp_hi(g_num_serdes_bits-1) <= outp_hi_d0 xor bit_pattern_hi(g_num_serdes_bits-1);
  gen_outp_bits : for i in g_num_serdes_bits-2 downto 0 generate
    outp_hi(i) <= outp_hi(i+1) xor bit_pattern_hi(i);
  end generate gen_outp_bits;

  p_outp_delay : process (clk_i, rst_n_i)
  begin
    if (rst_n_i = '0') then
      outp_hi_d0 <= '0';
    elsif rising_edge(clk_i) then
      -- clear on register load
      if (ld_reg_p0_i = '1') then
        outp_hi_d0 <= '0';
      -- external sync FSM sets value of the last bit
      elsif (ld_hi_p0_i = '1') then
        outp_hi_d0 <= last_bit_i;
      else
        outp_hi_d0 <= outp_hi(0);
      end if;
    end if;
  end process p_outp_delay;

gen_secondary_outp_logic : if (g_selectable_duty_cycle = true) generate

  patt_lo   <= unsigned(bit_pattern_skip_i) when (frac_carry_lo = '1') else
               unsigned(bit_pattern_normal_i);
  shpatt_lo <= shift_right(patt_lo, to_integer(per_count_lo(f_log2_size(2*g_num_serdes_bits)-1 downto 0)))
                 when (per_count_lo < 2*g_num_serdes_bits) else
               (others => '0');

  bit_pattern_lo <= std_logic_vector(shpatt_lo(g_num_serdes_bits-1 downto 0));

  outp_lo(g_num_serdes_bits-1) <= outp_lo_d0 xor bit_pattern_lo(g_num_serdes_bits-1);
  gen_secondary_outp_bits : for i in g_num_serdes_bits-2 downto 0 generate
    outp_lo(i) <= outp_lo(i+1) xor bit_pattern_lo(i);
  end generate gen_secondary_outp_bits;

  p_secondary_outp_delay : process (clk_i, rst_n_i)
  begin
    if (rst_n_i = '0') then
      outp_lo_d0 <= '0';
    elsif rising_edge(clk_i) then
      -- clear on resgister load
      if (ld_reg_p0_i = '1') then
        outp_lo_d0 <= '0';
      -- external sync FSM sets value of the last bit
      elsif (ld_lo_p0_i = '1') then
        outp_lo_d0 <= last_bit_i;
      else
        outp_lo_d0 <= outp_lo(0);
      end if;
    end if;
  end process p_secondary_outp_delay;

end generate gen_secondary_outp_logic;

  --===========================================================================
  -- Output register
  --===========================================================================
  -- When 50/50 duty cycle is needed, the output is simply the state of the high
  -- counter. Note that this also means the effective period is twice the period
  -- at the per_i input (see the diagram below).
gen_outp_reg_simple : if (g_selectable_duty_cycle = false) generate
  p_outp_reg : process (clk_i, rst_n_i)
  begin
    if (rst_n_i = '0') then
      serdes_dat_o <= (others => '0');
    elsif rising_edge(clk_i) then
      serdes_dat_o <= outp_hi;
    end if;
  end process p_outp_reg;
end generate gen_outp_reg_simple;

-- When a selectable duty cycle is needed, the high clock and low clock
-- (the one phase-offset by per_hi_i) are XORed together, to output the
-- clock with the selected duty cycle
--                _______          ________
--    clk_hi ____/       \________/        \________
--                   ________          ________
--    clk_lo _______/        \________/        \_____
--                __       __       __       __
--    outp   ____/  \_____/  \_____/  \_____/  \_____
--
gen_outp_reg_xored : if (g_selectable_duty_cycle = true) generate
  p_outp_reg : process (clk_i, rst_n_i)
  begin
    if (rst_n_i = '0') then
      serdes_dat_o <= (others => '0');
    elsif rising_edge(clk_i) then
      serdes_dat_o <= outp_hi xor outp_lo;
    end if;
  end process p_outp_reg;
end generate gen_outp_reg_xored;

end architecture arch;
--==============================================================================
--  architecture end
--==============================================================================
