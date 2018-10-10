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
  wr_lock_i                     : std_logic;

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

  signal r_tm_tai8ns_i                        : std_logic_vector(64-1 downto 0) := (others => '0');
  signal r_tai_observer_cnt                   : unsigned(63 downto 0) := (others => '0');
  signal s_tai_observer_dec                   : unsigned(63 downto 0) := (others => '0');
  signal r_tai_old                            : signed(63 downto 0)   := (others => '0');
  signal r_time_dif, r_time_dif_pos,
         r_time_dif_neg                       : signed(63 downto 0)             := (others => '0');
  signal r_time_dif_pos_ts                    : std_logic_vector(64-1 downto 0) := (others => '0'); -- (approximate) timestamp of last pos dif update
  signal r_time_dif_neg_ts                    : std_logic_vector(64-1 downto 0) := (others => '0'); -- (approximate) timestamp of last neg dif update

  signal r_wr_lock_acqu_last_ts, r_wr_lock_loss_last_ts : std_logic_vector(64-1 downto 0) := (others => '0');
  signal s_wr_lock_cnt_i                      : std_logic_vector(64-1 downto 0) := (others => '0');
  signal r_wr_lock_cnt                        : unsigned(64-1 downto 0)         := (others => '0');
  signal r_wr_lock, s_wr_lock_loss,
         s_wr_lock_acqu                       : std_logic := '0';

  signal s_stall_observation_cycles           : unsigned(31 downto 0)           := (others => '0');
  signal ra_stall_cnt, ra_stall_observer_cnt,
         ra_stall_max                         : u32_array(g_cores-1 downto 0)   := (others => (others => '0'));
  signal s_stall_observer_dec, s_stall_inc    : std_logic_vector(g_cores-1 downto 0) := (others => '0');
  signal ra_stall_max_ts                      : u64_array(g_cores-1 downto 0)   := (others => (others => '0')); -- Timestamp of last max update

  signal s_selector                           : natural;

  ---------------------------------------------------------------------------------
  --Workaround (deadtime) for weirdly jittering WR core status signals
  signal s_wr_lock_scan,
         s_deadtime_start,
         s_deadtime_stop : std_logic := '0';
  signal r_deadtime : unsigned(15 downto 0);
  signal r_deadtime_run : std_logic_vector(0 downto 0);
  ---------------------------------------------------------------------------------


begin



  --Decrement for TAI observer process
  s_tai_observer_dec        <= resize(unsigned(s_ctrl_enable_o(0 downto 0)), r_tai_observer_cnt'length); --divide by 8 ns, as we cannot count faster than clock period
  s_ctrl_time_dif_pos_i     <= std_logic_vector(r_time_dif_pos);
  s_ctrl_time_dif_pos_ts_i  <= r_time_dif_pos_ts;
  s_ctrl_time_dif_neg_i     <= std_logic_vector(r_time_dif_neg);
  s_ctrl_time_dif_neg_ts_i  <= r_time_dif_neg_ts;

  tai_fanout : process (clk_ref_i)
  begin
    if rising_edge(clk_ref_i) then
      r_tm_tai8ns_i <= tm_tai8ns_i; -- we need to register TAI to reduce fanout
    end if;
  end process;    

  --TAI diff observer process
  tai_observer : process (clk_ref_i)
  begin

    if rising_edge(clk_ref_i) then
      if(rst_ref_n_i = '0' OR s_ctrl_reset_o(0) = '1' ) then
        r_tai_old         <= signed(r_tm_tai8ns_i);
        r_time_dif_neg    <= (others => '0');
        r_time_dif_neg_ts <= r_tm_tai8ns_i;
        r_time_dif_pos    <= (others => '0');
        r_time_dif_pos_ts <= r_tm_tai8ns_i;
        r_time_dif        <= (others => '0');
        r_tai_observer_cnt <= (others => '1'); -- this makes sure s_ctrl_time_observation_interval_o is always copied 1 cycle after clear
      else
        if (r_tai_observer_cnt(r_tai_observer_cnt'left) = '1') then
          --re-init observer count down
          r_tai_observer_cnt <= "000" & unsigned(s_ctrl_time_observation_interval_o(63 downto 3)); -- ticks are 8ns, shift by 3b

          --assign new extreme values penending on diff sign, timestamp the update (probably not exact, but enough to correlate with other log files)
          if (r_time_dif > r_time_dif_pos) then
            r_time_dif_pos    <= r_time_dif;
            r_time_dif_pos_ts <= r_tm_tai8ns_i;
          end if;
          if (r_time_dif < r_time_dif_neg) then
            r_time_dif_neg    <= r_time_dif;
            r_time_dif_neg_ts <= r_tm_tai8ns_i;
          end if;
          r_tai_old <= signed(r_tm_tai8ns_i); --save current time for next diff
        else
          --count down for observer
          r_tai_observer_cnt <= r_tai_observer_cnt - s_tai_observer_dec;
        end if;
        --TODO: ECA adder might be a good idea, this requires some fast 64b magic...
        r_time_dif <= signed(r_tm_tai8ns_i) - r_tai_old;
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
          ra_stall_max_ts(I)       <= unsigned(r_tm_tai8ns_i);
        else
          if (ra_stall_observer_cnt(I)(ra_stall_observer_cnt(I)'left) = '1') then -- observer count down is over

            if (ra_stall_cnt(I) > ra_stall_max(I)) then -- if the stall cnt is a new record, we save the value
              ra_stall_max(I)     <= ra_stall_cnt(I);
              ra_stall_max_ts(I)  <= unsigned(r_tm_tai8ns_i);
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




 -- --WR state observer process
  s_wr_lock_loss  <= not wr_lock_i and     r_wr_lock and s_wr_lock_scan; --falling edge of wr state is lock lost
  s_wr_lock_acqu  <=     wr_lock_i and not r_wr_lock and s_wr_lock_scan; --rising edge of wr state is lock acquired
  s_wr_lock_cnt_i <= std_logic_vector(r_wr_lock_cnt);


  wr_observer : process (clk_ref_i)
  begin
    if rising_edge(clk_ref_i) then
      r_wr_lock <= wr_lock_i;
      if(rst_ref_n_i = '0' OR s_ctrl_reset_o(0) = '1' ) then
        r_wr_lock_cnt          <= (others => '0');
        r_wr_lock_loss_last_ts <= (others => '0');
        r_wr_lock_acqu_last_ts <= (others => '0');
      else
        if (s_wr_lock_loss = '1') then
          r_wr_lock_cnt <= r_wr_lock_cnt + 1;
          r_wr_lock_loss_last_ts <= r_tm_tai8ns_i; 
        end if;
        if (s_wr_lock_acqu = '1') then
          r_wr_lock_acqu_last_ts <= r_tm_tai8ns_i;
        end if;
      end if;
    end if;
  end process;

  ---------------------------------------------------------------------------------
  --Workaround (deadtime) for weirdly jittering WR core status signals

  s_wr_lock_scan    <= not r_deadtime_run(0);
  s_deadtime_start  <= wr_lock_i xor r_wr_lock;
  s_deadtime_stop   <= r_deadtime(r_deadtime'left); --overflow bit
  workaround_wr_status_bug : process(clk_ref_i)
  begin
    if rising_edge(clk_ref_i) then
      if(rst_ref_n_i = '0' OR s_ctrl_reset_o(0) = '1' OR s_deadtime_stop = '1') then
        r_deadtime     <= "0" & to_unsigned(-1, r_deadtime'length-1);
        r_deadtime_run <= "0";
      else
        r_deadtime_run(0) <= (r_deadtime_run(0) OR s_deadtime_start);
        r_deadtime <= r_deadtime - resize(unsigned(r_deadtime_run), r_deadtime'length);
      end if;
    end if;
  end process;
  ---------------------------------------------------------------------------------


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
    wr_lock_acqu_last_ts_i        => r_wr_lock_acqu_last_ts,
    wr_lock_acqu_last_ts_V_i      => "1",
    wr_lock_cnt_i                 => s_wr_lock_cnt_i,
    wr_lock_cnt_V_i               => "1",
    wr_lock_loss_last_ts_i        => r_wr_lock_loss_last_ts,
    wr_lock_loss_last_ts_V_i      => "1",
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