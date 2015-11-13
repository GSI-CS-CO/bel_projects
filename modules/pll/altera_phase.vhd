-------------------------------------------------------------------------------
-- Title      : Altera dynamic PLL phase adjustment
-- Project    : White Rabbit
-------------------------------------------------------------------------------
-- File       : altera_phase.vhd
-- Author     : Wesley W. Terpstra
-- Company    : GSI
-- Created    : 2013-09-24
-- Last update: 2013-09-24
-- Platform   : Altera
-- Standard   : VHDL'93
-------------------------------------------------------------------------------
-- Description: Shifts clock outputs on a PLL to requested values
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
-- 2013-09-24  1.0      terpstra  Move phase shifting to a general purpose core
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.pll_pkg.all;

entity altera_phase is
  generic(
    g_select_bits   : natural;
    g_outputs       : natural;
    g_base          : integer;
    g_vco_freq      : natural;
    g_output_freq   : natural_vector;
    g_output_select : natural_vector);
  port(
    clk_i       : in  std_logic; 
    rstn_i      : in  std_logic; -- phase counters were zero'd
    clks_i      : in  std_logic_vector(g_outputs-1 downto 0);
    rstn_o      : out std_logic_vector(g_outputs-1 downto 0);
    offset_i    : in  phase_offset_vector(g_outputs-1 downto 0);
    phasedone_i : in  std_logic;
    phasesel_o  : out std_logic_vector(g_select_bits-1 downto 0);
    phasestep_o : out std_logic);
end altera_phase;

architecture rtl of altera_phase is

  function f_modulus(vco : natural; freq : natural_vector)
    return natural_vector
  is
    variable result : natural_vector(g_outputs-1 downto 0);
  begin
    for i in 0 to g_outputs-1 loop
      result(i) := vco*8/freq(i);
    end loop;
    return result;
  end function;
  
  function f_init_phase(base : integer; modulus : natural_vector(g_outputs-1 downto 0))
    return phase_offset_vector
  is
    variable result : phase_offset_vector(g_outputs-1 downto 0);
  begin
    for i in 0 to g_outputs-1 loop
      result(i) := to_unsigned((-g_base) mod modulus(i), phase_offset'length);
    end loop;
    return result;
  end function;
  
  function f_active_high(x : boolean) return std_logic is
  begin
    if x then return '1'; else return '0'; end if;
  end function;
  
  -- Requirements for correctness:
  --   sync_trap'length = aligned(i)'length
  --   |PULSE| >= sync_trap'length

  type t_state is (SETUP_REQUEST, PULSE1, PULSE2, PULSE3, WAIT_TRAP);
  subtype t_sync_chain is std_logic_vector(2 downto 0);
  type t_sync_array is array (natural range <>) of t_sync_chain;
  
  constant c_modulus : natural_vector(g_outputs-1 downto 0)
            := f_modulus(g_vco_freq, g_output_freq);
  constant c_init_phase : phase_offset_vector(g_outputs-1 downto 0)
            := f_init_phase(g_base, c_modulus);
  
  signal state      : t_state                                     := SETUP_REQUEST;
  signal prime_trap : std_logic                                   := '1';
  signal raw_trap   : std_logic                                   := '0';
  signal sync_trap  : t_sync_chain                                := (others => '0');
  signal phasesel   : std_logic_vector(g_select_bits-1 downto 0)  := (others => '-');
  signal phasestep  : std_logic                                   := '0';
  
  signal output     : unsigned(f_ceil_log2(g_outputs)-1 downto 0) := (others => '0');
  signal request    : std_logic                                   := '1';
  signal phase      : phase_offset_vector(g_outputs-1 downto 0)   := c_init_phase;
  signal aligned    : t_sync_array(g_outputs-1 downto 0)          := (others => (others => '0'));
  signal gen_rstn   : std_logic_vector(g_outputs-1 downto 0)      := (others => '0');
  signal sync_rstn  : t_sync_array(g_outputs-1 downto 0)          := (others => (others => '0'));
  
  -- We ensure timing between these nodes via the state machine
  attribute altera_attribute : string;
  attribute altera_attribute OF rtl : architecture is 
    ("-name SDC_STATEMENT ""set_false_path -from {*|altera_phase:*|prime_trap} -to {*|altera_phase:*|raw_trap}"";"  &
     "-name SDC_STATEMENT ""set_false_path -from {*|altera_phase:*|raw_trap}   -to {*|altera_phase:*|sync_trap*}"";" &
     "-name SDC_STATEMENT ""set_false_path                                     -to {*|altera_phase:*|aligned*}"";" &
     "-name SDC_STATEMENT ""set_false_path -from {*|altera_phase:*|gen_rstn*}  -to {*|altera_phase:*|sync_rstn*}""");
begin

  -- Pulse width of phasedone_i can be less than clock period... so make a trap
  trap : process(phasedone_i, prime_trap) is
  begin
    if prime_trap = '1' then
      raw_trap <= '0';
    elsif rising_edge(phasedone_i) then
      raw_trap <= '1';
    end if;
  end process;
  
  sync : process(clk_i, rstn_i) is
  begin
    if rstn_i = '0' then
      sync_trap <= (others => '0');
      aligned   <= (others => (others => '0'));
    elsif rising_edge(clk_i) then
      sync_trap <= raw_trap & sync_trap(sync_trap'left downto 1);
      for i in 0 to g_outputs-1 loop
        aligned(i) <= f_active_high(offset_i(i) = phase(i)) & aligned(i)(t_sync_chain'left downto 1);
      end loop;
    end if;
  end process;
  
  -- state         SETUP_REQUEST  PULSE1  PULSE2-x  WAIT_TRAP
  -- prime_trap    /------------\
  --               /            \---------------------------
  -- phasestep                            /-----------------\
  --               -----------------------/                 \
  -- raw_trap      ?????????????????\  /?????????????????????
  --               ?????????????????\--/?????????????????????
  -- sync_trap(0)  ????????????????????????????\  /??????????
  --               ????????????????????????????\--/??????????
  --
  -- The waveform used here allows |skew| <= clock period.
  -- ie: there will be a gap between falling_edge(prime_trap)
  -- and rising_edge(phasestep) as seen at raw_trap. Because
  -- phasedone_i is the result of phasestep, it must follow
  -- that falling_edge(prime_trap) < rising_edge(phasedone_i).
  -- Therefore, the trap never misses the completion signal.

  phasesel_o <= phasesel;
  phasestep_o <= phasestep;
  shift : process(clk_i, rstn_i) is
  begin
    if rstn_i = '0' then
      prime_trap <= '1';
      phasesel   <= (others => '-');
      phasestep  <= '0';
      state      <= SETUP_REQUEST;
      gen_rstn   <= (others => '0');
    elsif rising_edge(clk_i) then
      case state is
      
        when SETUP_REQUEST =>
          if request = '0' then
            prime_trap <= '0';
            phasesel   <= (others => '-');
            phasestep  <= '0';
            state      <= SETUP_REQUEST;
            gen_rstn(to_integer(output)) <= '1';
          else
            prime_trap <= '0';
            phasesel   <= std_logic_vector(to_unsigned(g_output_select(to_integer(output)), g_select_bits));
            phasestep  <= '0';
            state      <= PULSE1;
            gen_rstn(to_integer(output)) <= '0';
          end if;
        
        when PULSE1 =>
          prime_trap <= '0';
          phasesel   <= phasesel;
          phasestep  <= '1';
          state      <= PULSE2;
          
        when PULSE2 =>
          prime_trap <= '0';
          phasesel   <= phasesel;
          phasestep  <= '1';
          state      <= PULSE3;
        
        when PULSE3 =>
          prime_trap <= '0';
          phasesel   <= phasesel;
          phasestep  <= '1';
          state      <= WAIT_TRAP;
        
        when WAIT_TRAP =>
          if sync_trap(0) = '0' then
            prime_trap <= '0';
            phasesel   <= (others => '-');
            phasestep  <= '0';
            state      <= WAIT_TRAP;
          else
            prime_trap <= '1';
            phasesel   <= (others => '-');
            phasestep  <= '0';
            state      <= SETUP_REQUEST;
          end if;
        
      end case;
    end if;
  end process;
  
  track_phases : for i in 0 to g_outputs-1 generate
    track_phase : process(clk_i, rstn_i) is
    begin
      if rstn_i = '0' then
        phase(i) <= c_init_phase(i);
      elsif rising_edge(clk_i) then
        if output = i and request = '1' and state = SETUP_REQUEST then
          -- We slip clocks by increasing their period => each shift increases phase
          if phase(i) = c_modulus(i)-1 then
            phase(i) <= (others => '0');
          else
            phase(i) <= phase(i) + 1;
          end if;
        end if;
      end if;
    end process;
  end generate;
  
  share : process(clk_i, rstn_i) is
  begin
    if rstn_i = '0' then
      request <= '1';
      output  <= (others => '0');
    elsif rising_edge(clk_i) then
      -- It is important that this test fails after the last shift, before SETUP_REQUEST
      -- phase(i)      is set when PULSE1 evaluated
      -- aligned(i)(2) is set when PULSE2
      -- aligned(i)(1) is set when PULSE3
      -- aligned(i)(0) is set when WAIT_TRAP
      -- request       is set when SETUP_REQUEST evaluated (just in time!)
      -- Require: PULSEx >= aligned(i)'length
      if aligned(to_integer(output))(0) = '0' then
        request <= '1';
        output  <= output;
      else
        if output = g_outputs-1 then
          request <= '0';
          output  <= (others => '0');
        else
          request <= '0';
          output  <= output + 1;
        end if;
      end if;
    end if;
  end process;
  
  resets : for i in 0 to g_outputs-1 generate
    rstn_o(i) <= sync_rstn(i)(0);
    reset : process(clks_i(i), gen_rstn(i)) is
    begin
      if gen_rstn(i) = '0' then
        sync_rstn(i) <= (others => '0');
      elsif rising_edge(clks_i(i)) then
        sync_rstn(i) <= '1' & sync_rstn(i)(t_sync_chain'left downto 1);
      end if;
    end process;
  end generate;
  
end rtl;
