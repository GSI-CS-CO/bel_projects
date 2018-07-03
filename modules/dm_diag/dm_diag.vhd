--! @file        dm_diag.xml
--  DesignUnit   dm_diag
--! @author      M. Kreider <m.kreider@gsi.de>
--! @date        28/06/2018
--! @version     0.0.1
--! @copyright   2018 GSI Helmholtz Centre for Heavy Ion Research GmbH
--!

--TODO: This is a stub, finish/update it yourself
--! @brief *** ADD BRIEF DESCRIPTION HERE ***
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
--------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;
use work.wbgenplus_pkg.all;
use work.genram_pkg.all;
use work.dm_diag_auto_pkg.all;


entity dm_diag is
generic(
  g_cores : natural := 16 --CPU cores
);
Port(
  clk_ref_i                     : std_logic;                            -- Clock input for ref domain
  rst_ref_n_i                   : std_logic;                            -- Reset input (active low) for ref domain
  tm_tai8ns_i                   : std_logic_vector(63 downto 0) := (others => '0');
  cyc_diag_i                    : std_logic_vector(g_cores-1 downto 0);
  stall_diag_i                  : std_logic_vector(g_cores-1 downto 0);
  
  ctrl_i                        : in  t_wishbone_slave_in;
  ctrl_o                        : out t_wishbone_slave_out

  
);
end dm_diag;

architecture rtl of dm_diag is

  -- Regs/Sigs WB interface
  signal s_ctrl_reset_o                       : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Resets/clears the diagnostic
  signal s_ctrl_enable_o                      : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Enables/disables update. Default is enabled
  signal s_ctrl_time_observation_interval_o   : std_logic_vector(64-1 downto 0) := (others => '0'); -- TAI time observation interval in ns
  signal s_ctrl_time_dif_pos_i                : std_logic_vector(64-1 downto 0) := (others => '0'); -- Observed max pos. ECA time difference in ns between ref clock ticks
  signal s_ctrl_time_dif_pos_ts_i             : std_logic_vector(64-1 downto 0) := (others => '0'); -- (approximate) timestamp of last pos dif update
  signal s_ctrl_time_dif_neg_i                : std_logic_vector(64-1 downto 0) := (others => '0'); -- Observed max neg. ECA time difference in ns between ref clock ticks
  signal s_ctrl_time_dif_neg_ts_i             : std_logic_vector(64-1 downto 0) := (others => '0'); -- (approximate) timestamp of last neg dif update
  signal s_ctrl_stall_observation_interval_o  : std_logic_vector(32-1 downto 0) := (others => '0'); -- Stall observation interval in cycles
  signal s_ctrl_stall_stat_select_WR_o        : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Write enable flag - stall_stat_select
  signal s_ctrl_stall_stat_select_o           : std_logic_vector(8-1 downto 0)  := (others => '0'); -- Page selector register
  signal s_ctrl_stall_streak_max_i            : std_logic_vector(32-1 downto 0) := (others => '0'); -- Observed max continuous stall in cycles
  signal s_ctrl_stall_cnt_i                   : std_logic_vector(32-1 downto 0) := (others => '0'); -- Stall time within observation interval in cycles
  signal s_ctrl_stall_max_ts_i                : std_logic_vector(64-1 downto 0) := (others => '0'); -- Timestamp of last max update

  -- Regs/Sigs Diagnostics Module
  type u32_array is array (natural range <>) of unsigned(31 downto 0);
  type u64_array is array (natural range <>) of unsigned(63 downto 0);
  
  signal r_tai_observer_cnt                   : unsigned(63 downto 0) := (others => '0');
  signal s_tai_observer_dec                   : unsigned(63 downto 0) := (others => '0');
  signal r_tai_old                            : signed(63 downto 0)   := (others => '0');
  signal r_time_dif, r_time_dif_pos, 
         r_time_dif_neg                       : signed(63 downto 0)             := (others => '0');
  signal r_time_dif_pos_ts                    : std_logic_vector(64-1 downto 0) := (others => '0'); -- (approximate) timestamp of last pos dif update
  signal r_time_dif_neg_ts                    : std_logic_vector(64-1 downto 0) := (others => '0'); -- (approximate) timestamp of last neg dif update

  signal s_stall_observation_cycles           : unsigned(31 downto 0)           := (others => '0');
  signal ra_stall_cnt, ra_stall_observer_cnt, 
         ra_stall_max                         : u32_array(g_cores-1 downto 0)   := (others => (others => '0'));
  signal s_stall_observer_dec, s_stall_inc    : std_logic_vector(g_cores-1 downto 0) := (others => '0');
  signal ra_stall_max_ts                      : u64_array(g_cores-1 downto 0)   := (others => (others => '0')); -- Timestamp of last max update
    
  signal s_selector                           : natural;

begin



  --Decrement for TAI observer process
  s_tai_observer_dec        <= resize(unsigned(s_ctrl_enable_o(63 downto 3)), r_tai_observer_cnt'length); --divide by 8 ns, as we cannot count faster than clock period
  s_ctrl_time_dif_pos_i     <= std_logic_vector(r_time_dif_pos);
  s_ctrl_time_dif_pos_ts_i  <= r_time_dif_pos_ts;
  s_ctrl_time_dif_neg_i     <= std_logic_vector(r_time_dif_neg);
  s_ctrl_time_dif_neg_ts_i  <= r_time_dif_neg_ts;
  
  --TAI diff observer process
  tai_observer : process (clk_ref_i)
  begin
 
    if rising_edge(clk_ref_i) then
      if(rst_ref_n_i = '0' OR s_ctrl_reset_o(0) = '1' ) then
        r_tai_old         <= signed(tm_tai8ns_i);
        r_time_dif_neg    <= (others => '0');
        r_time_dif_neg_ts <= tm_tai8ns_i;
        r_time_dif_pos    <= (others => '0');
        r_time_dif_pos_ts <= tm_tai8ns_i;
        r_time_dif        <= (others => '0');
        r_tai_observer_cnt <= (others => '1');
      else
        if (r_tai_observer_cnt(r_tai_observer_cnt'left) = '1') then
          --re-init observer count down
          r_tai_observer_cnt <= unsigned(s_ctrl_time_observation_interval_o);
          
          --assign new extreme values penending on diff sign, timestamp the update (probably not exact, but enough to correlate with other log files)
          if (r_time_dif > r_time_dif_pos) then
            r_time_dif_pos    <= r_time_dif;
            r_time_dif_pos_ts <= tm_tai8ns_i;
          end if;
          if (r_time_dif < r_time_dif_neg) then
            r_time_dif_neg    <= r_time_dif;
            r_time_dif_neg_ts <= tm_tai8ns_i;
          end if;
          r_tai_old <= signed(tm_tai8ns_i); --save current time for next diff
        else
          --count down for observer
          r_tai_observer_cnt <= r_tai_observer_cnt - s_tai_observer_dec;
        end if;
        --TODO: ECA adder might be a good idea, this requires some fast 64b magic...
        r_time_dif <= signed(tm_tai8ns_i) - r_tai_old; 
      end if;
    end if;
  end process;

  --*** MUXes for reading channels of WB interface
  s_selector                <= to_integer(unsigned(s_ctrl_stall_stat_select_o));
  s_ctrl_stall_streak_max_i <= std_logic_vector(ra_stall_max(s_selector));
  s_ctrl_stall_cnt_i        <= std_logic_vector(ra_stall_cnt(s_selector));
  s_ctrl_stall_max_ts_i     <= std_logic_vector(ra_stall_max_ts(s_selector));



  -- Stall observer processes, one for each CPU core
  -- Listens until given number of bus transfer cycles are collected, i.e., you need bus traffic to get a result
  G1: for I in 0 to g_cores-1 generate

    --Increment & Decrement for Stall observer process
    s_stall_observer_dec(I)  <= cyc_diag_i(I) AND s_ctrl_enable_o(0);         -- only decrement observer countdown if cycle line and enable were high
    s_stall_inc(I)           <= s_stall_observer_dec(I) AND stall_diag_i(I);  -- only increment stall count if cycle line and stall line and enable were high

    stall_observer : process (clk_ref_i)
    begin
   
      if rising_edge(clk_ref_i) then
        if(rst_ref_n_i = '0' OR s_ctrl_reset_o(0) = '1') then
          ra_stall_max(I)          <= (others => '0');
          ra_stall_cnt(I)          <= (others => '0');
          ra_stall_observer_cnt(I) <= (others => '1');
          ra_stall_max_ts(I)       <= unsigned(tm_tai8ns_i);
        else
          if (ra_stall_observer_cnt(I)(ra_stall_observer_cnt(I)'left) = '1') then -- observer count down is over
            
            if (ra_stall_cnt(I) > ra_stall_max(I)) then -- if the stall cnt is a new record, we save the value
              ra_stall_max(I)     <= ra_stall_cnt(I);
              ra_stall_max_ts(I)  <= unsigned(tm_tai8ns_i);
            end if;

            -- re-init observer countdown, clear stall cnt
            ra_stall_observer_cnt(I) <= unsigned(s_ctrl_stall_observation_interval_o);
            ra_stall_cnt(I)          <= (others => '0');
            
          else
            -- countdown and stall count
            ra_stall_observer_cnt(I) <= ra_stall_observer_cnt(I)  - resize(unsigned(s_stall_observer_dec(I downto I)), ra_stall_observer_cnt(I)'length);
            ra_stall_cnt(I)          <= ra_stall_cnt(I)           + resize(unsigned(s_stall_inc(I downto I)),          ra_stall_cnt(I)'length);         
          end if;
        end if;
      end if;
    end process;

  end generate G1;   


    INST_dm_diag_auto : dm_diag_auto
  port map (
    clk_ref_i                     => clk_ref_i,
    rst_ref_n_i                   => rst_ref_n_i,
    error_i                       => "0",
    stall_i                       => "0",
    reset_o                       => s_ctrl_reset_o,
    enable_o                      => s_ctrl_enable_o,
    time_observation_interval_o   => s_ctrl_time_observation_interval_o,
    time_dif_pos_V_i              => "1",
    time_dif_pos_i                => s_ctrl_time_dif_pos_i,
    time_dif_pos_ts_V_i           => "1",
    time_dif_pos_ts_i             => s_ctrl_time_dif_pos_ts_i,
    time_dif_neg_V_i              => "1",
    time_dif_neg_i                => s_ctrl_time_dif_neg_i,
    time_dif_neg_ts_V_i           => "1",
    time_dif_neg_ts_i             => s_ctrl_time_dif_neg_ts_i,
    stall_observation_interval_o  => s_ctrl_stall_observation_interval_o,
    stall_stat_select_WR_o        => s_ctrl_stall_stat_select_WR_o,
    stall_stat_select_RD_o        => open,
    stall_stat_select_o           => s_ctrl_stall_stat_select_o,
    stall_streak_max_V_i          => "1",
    stall_streak_max_i            => s_ctrl_stall_streak_max_i,
    stall_cnt_V_i                 => "1",
    stall_cnt_i                   => s_ctrl_stall_cnt_i,
    stall_max_ts_V_i              => "1",
    stall_max_ts_i                => s_ctrl_stall_max_ts_i,
    ctrl_i                        => ctrl_i,
    ctrl_o                        => ctrl_o  );
  
end rtl;