---------------------------------------------------------------------------------
-- filename: sweep_cntrl.vhd
-- desc:
-- creation date: 08.12.2016
-- last modified:
-- author: Stefan Rauch <s.rauch@gsi.de>
-- based on graphical design by W.Panschow
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
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.math_real.all;

entity sweep_cntrl is
  generic (
    dw:       integer;
    f_in_khz: integer
  );
  port (
    clk:      in std_logic;
    freq_en:  in std_logic;
    reset:    in std_logic;
    
    ena_soft_trig:  in std_logic;
    ld_delta:       in std_logic;
    ld_delay:       in std_logic;
    ld_flattop_int: in std_logic;
    set_flattop:    in std_logic;
    stop_in:        in std_logic;
    hw_trig:        in std_logic;
    sw_trig:        in std_logic;
    ramp_fin:       in std_logic;
    delta:          in unsigned(dw-1 downto 0);
    d_in:           in unsigned(dw-1 downto 0);
    
    wr_delta:       out std_logic;
    s_stop_delta:   out std_logic;
    wr_ft_int:      out std_logic;
    wr_flattop:     out std_logic;
    idle:           out std_logic;
    w_start:        out std_logic;
    work:           out std_logic;
    stop:           out std_logic;
    stop_exec:      out std_logic;
    to_err:         out std_logic;
    seq_err:        out std_logic;
    trigger:        out std_logic;
    init_hw:        out std_logic
    
  );
end entity;

architecture arch of sweep_cntrl is
  constant c_to_time:         integer := 2995;                      -- timeout in microseconds
  constant c_to_count:        integer := f_in_khz * c_to_time / 1000;
  constant c_to_count_width:  integer := integer(ceil(log2(real(c_to_count)))) + 1;
  
  signal s_trig_sync, s_trig_sync1, s_trig_sync2: std_logic;
  signal s_trigger_ff:      std_logic;
  signal delay_tmr_cnt:     unsigned(dw downto 0); -- width dw + 1
  signal to_tmr_cnt:        unsigned(c_to_count_width - 1 downto 0);
  signal s_delay_fin:       std_logic;
  signal s_delta_not_zero:  std_logic;
  
  type sm_type is (sm_idle, sm_wr_delta, sm_wr_delay, sm_wr_ft_int, sm_set_flattop, sm_w_start, sm_work, sm_stop, sm_seq_err);
  signal control_sm:  sm_type;
  
  signal s_to_err:          std_logic;
  signal s_seq_err:         std_logic;
  signal s_stop_exec:       std_logic;
  signal s_init_hw:         std_logic;
  signal s_w_start:         std_logic;
  signal s_work:            std_logic;
  signal s_stop:            std_logic;
  signal s_timeout:         std_logic;
  signal s_wr_delay:        std_logic;
  

begin
  trigger_sync: process(clk)
  begin
    if rising_edge(clk) then
      if freq_en = '1' then
        if (ena_soft_trig = '1' and sw_trig = '1') or (ena_soft_trig = '0' and hw_trig = '1') then
          s_trig_sync <= '1';
        else
          s_trig_sync <= '0';
        end if;
        s_trig_sync1 <= s_trig_sync;
        s_trig_sync2 <= s_trig_sync1;
      end if;
    end if;
  end process;
  
  trigger_ff: process(clk)
  begin
    if rising_edge(clk) then
      if freq_en = '1' then
        if s_trig_sync1 = '1' and s_trig_sync2 = '0' and control_sm = sm_w_start then
          s_trigger_ff <= '1';
        end if;
      end if;
    end if;
  end process;
  
  to_tmr: process(clk)
  begin
    if rising_edge(clk) then
      if freq_en = '1' and (control_sm = sm_set_flattop or control_sm = sm_w_start) then
        if control_sm = sm_wr_delta then
          to_tmr_cnt <= to_unsigned(c_to_count, c_to_count_width);
        else
          to_tmr_cnt <= to_tmr_cnt - 1;
        end if;
      end if;
    end if;
  end process;
  
  timeout: process(clk)
  begin
    if rising_edge(clk) then
      if freq_en = '1' then
        s_timeout <= to_tmr_cnt(c_to_count_width - 1);
      end if;
    end if;
  end process;
  
  delay_tmr: process(clk)
  begin
    if rising_edge(clk) then
      if freq_en = '1' and s_trigger_ff = '1' and control_sm = sm_w_start then
        if s_wr_delay = '1' then
          delay_tmr_cnt <= '0' & d_in(11 downto 0);
        else
          delay_tmr_cnt <= delay_tmr_cnt - 1;
        end if;
      end if;
    end if;
  end process;
  
  dly_fin: process(clk)
  begin
    if rising_edge(clk) then
      if freq_en = '1' then
        s_delay_fin <= delay_tmr_cnt(dw) and s_trigger_ff;
      end if;
    end if;
  end process;
  
  delta_not_zero: process(clk)
  begin
    if rising_edge(clk) then
      if freq_en = '1' then
        if delta /= to_unsigned(0, dw) then
          s_delta_not_zero <= '1';
        else
          s_delta_not_zero <= '0';
        end if;
      end if;
    end if;
  end process;

  sm: process(clk)
  begin
    if reset <= '1' then
      control_sm <= sm_idle;
    elsif rising_edge(clk) then
      if freq_en = '1' then
        case control_sm is
          when sm_idle =>
            if ld_delta = '1' then
              control_sm <= sm_wr_delta;
            elsif (ld_flattop_int = '1' or ld_delay = '1') and not s_seq_err = '1' then
              control_sm <= sm_seq_err;
            end if;
          
          when sm_wr_delta =>
            if ld_delta = '1' then
              s_to_err      <= '0';
              s_seq_err     <= '0';
              s_trigger_ff  <= '0';
              wr_delta      <= '1';
              s_stop_exec   <= '0';
              control_sm    <= sm_wr_delta;
            elsif ld_delay = '1' then
              control_sm <= sm_wr_delay;
            elsif (ld_flattop_int = '1' or set_flattop = '1') then
              control_sm <= sm_seq_err;
            end if;
            
          when sm_wr_delay =>
            if ld_delta = '1' then
              s_wr_delay <= '1';
              control_sm <= sm_wr_delay;
            elsif (ld_delta = '1' or set_flattop = '1') and not s_seq_err = '1' then
              control_sm <= sm_seq_err;
            elsif ld_flattop_int = '1' then
              control_sm <= sm_wr_ft_int;
            end if;
          
          when sm_wr_ft_int =>
            if ld_flattop_int = '1' then
              wr_ft_int <= '1';
              control_sm <= sm_wr_ft_int;
            elsif (ld_delta = '1' or ld_delay = '1') and not s_seq_err = '1' then
              control_sm <= sm_seq_err;
            else
              control_sm <= sm_set_flattop;
            end if;
            
          when sm_set_flattop =>
            if set_flattop = '1' then
              wr_flattop <= '1';
              control_sm <= sm_set_flattop;
            elsif (ld_delta = '1' or ld_delay = '1' or ld_flattop_int = '1') and not s_seq_err = '1' then
              control_sm <= sm_seq_err;
            else
              control_sm <= sm_w_start;
            end if;
          
          when sm_w_start =>
            if s_delay_fin = '1' and s_delta_not_zero = '1' then
              control_sm <= sm_work;
            elsif stop_in = '1' then
              s_stop_delta  <= '1';
              stop_exec     <= '1';
              control_sm    <= sm_stop;
            elsif s_timeout = '1' then
              s_to_err      <= '1';
              s_stop_delta  <= '1';
              control_sm    <= sm_stop;
            else
              control_sm    <= sm_w_start;
            end if;
          
          when sm_work =>
            if ramp_fin = '1' then
              control_sm <= sm_idle;
            else
              control_sm <= sm_stop;
            end if;
          
          
          when sm_stop =>
            if ramp_fin = '1' then
              control_sm <= sm_idle;
            else
              control_sm <= sm_stop;
            end if;
          
          when sm_seq_err =>
            s_seq_err   <= '1';
            control_sm  <= sm_idle;
        
          when others =>
            null;
         end case;
      end if; -- freq_en
    end if; -- rising_edge
  end process;
  
  inithw: process(clk)
  begin
    if freq_en = '1' then
      if control_sm = sm_idle or ramp_fin = '1' then
        s_init_hw <= '1';
      else
        s_init_hw <= '0';
      end if;
    end if;
  end process;
    
  wstart: process(clk)
  begin
    if freq_en = '1' then
      if control_sm = sm_w_start then
        s_w_start <= '1';
      else
        s_w_start <= '0';
      end if;
    end if;
  end process;
  
  workff: process(clk)
  begin
    if freq_en = '1' then
      if control_sm = sm_work then
        s_work <= '1';
      else
        s_work <= '0';
      end if;
    end if;
  end process;
  
  stopff: process(clk)
  begin
    if freq_en = '1' then
      if control_sm = sm_stop then
        s_stop <= '1';
      else
        s_stop <= '0';
      end if;
    end if;
  end process;
  
  init_hw   <= s_init_hw;
  w_start   <= s_w_start;
  work      <= s_work;
  stop      <= s_stop;
  to_err    <= s_to_err;
  stop_exec <= s_stop_exec;
  trigger   <= s_trigger_ff;
  idle      <= '1' when (control_sm = sm_idle or control_sm = sm_seq_err) else '0';
  
end architecture;
