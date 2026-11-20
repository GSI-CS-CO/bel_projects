-- Self checking regression testbench for fg_quad_datapath
--
-- A reference model recalculates every output sample (sw_strobe) from the parameter sets
-- the testbench writes. The firmware is emulated with three different response times:
--   fast:  next set is written a few clocks after dreq
--   slow:  next set is written in the middle of the segment
--   edge:  coefficient write falls exactly into the segment boundary cycle
-- Checked: sample values and order of the sets, number of dreq pulses, the stop irq is not
-- masked by dreq, generator stops after the last set and keeps its last output value.
-- Shift values above 48 are used.
--
-- VHDL-2008 (external names). Run from this directory:
--   ghdl:      ghdl -a --std=08 ../../fg_quad_pkg.vhd ../../fg_quad_datapath.vhd fg_quad_datapath_tb.vhd
--              ghdl -e --std=08 fg_quad_datapath_tb
--              ghdl -r --std=08 fg_quad_datapath_tb [-gACU=true]
--   modelsim:  vsim -c -do fg_quad.do

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.fg_quad_pkg.all;

entity fg_quad_datapath_tb is
  generic (
    ACU:  boolean := false
  );
end entity;

architecture sim of fg_quad_datapath_tb is

  function clk_hz(acu_mode: boolean) return integer is
  begin
    if acu_mode then
      return 100_000_000;
    end if;
    return 125_000_000;
  end function;

  constant c_clk_in_hz:   integer := clk_hz(ACU);
  constant c_clk_period:  time    := 1 sec / c_clk_in_hz;
  constant c_steps:       integer := 250;               -- step_sel "000"
  constant c_step_sel:    std_logic_vector(2 downto 0) := "000";
  constant c_freq_sel:    std_logic_vector(2 downto 0) := "111";   -- 2 MHz

  type t_set is record
    a:    integer;
    b:    integer;
    c:    integer;
    sa:   integer;
    sb:   integer;
  end record;
  type t_set_array is array (natural range <>) of t_set;

  constant c_sets: t_set_array := (
    (a =>    100, b =>   1000, c =>     1000, sa => 10, sb => 20),
    (a =>   -300, b =>  20000, c =>   500000, sa => 12, sb => 22),
    (a =>  18392, b =>  26508, c =>   508086, sa =>  8, sb =>  7),
    (a =>      1, b =>      1, c =>  -700000, sa => 63, sb => 60),   -- shift > 48
    (a => -30016, b =>  17776, c =>   704503, sa =>  6, sb =>  6)
  );

  type t_mode is (fast, slow, edge);

  signal clk:               std_logic := '0';
  signal sysclk:            std_logic := '0';
  signal nrst:              std_logic := '0';
  signal sim_done:          boolean   := false;

  signal data_a:            std_logic_vector(15 downto 0) := (others => '0');
  signal data_b:            std_logic_vector(15 downto 0) := (others => '0');
  signal data_c:            std_logic_vector(31 downto 0) := (others => '0');
  signal shift_a:           integer range 0 to 63 := 0;
  signal shift_b:           integer range 0 to 63 := 0;
  signal a_en:              std_logic := '0';
  signal load_start:        std_logic := '0';
  signal sync_start:        std_logic := '0';

  signal state_change_irq:  std_logic;
  signal dreq:              std_logic;
  signal ramp_sec_fin:      std_logic;
  signal sw_out:            std_logic_vector(31 downto 0);
  signal sw_strobe:         std_logic;
  signal fg_is_running:     std_logic;

  -- shared between stimulus and checker
  signal n_sets:            integer := 0;      -- number of sets in the running test
  signal n_samples:         integer := 0;
  signal n_dreq:            integer := 0;
  signal n_stop_irq:        integer := 0;
  signal n_errors:          integer := 0;
  signal last_out:          std_logic_vector(31 downto 0) := (others => '0');   -- last output sample

begin

  clk    <= not clk after c_clk_period / 2 when not sim_done;
  sysclk <= not sysclk after 40 ns when not sim_done;    -- 12.5 MHz backplane clock

  dut: fg_quad_datapath
    generic map (
      CLK_in_Hz => c_clk_in_hz,
      ACU       => ACU
    )
    port map (
      data_a            => data_a,
      data_b            => data_b,
      data_c            => data_c,
      clk               => clk,
      sysclk            => sysclk,
      nrst              => nrst,
      sync_rst          => '0',
      a_en              => a_en,
      load_start        => load_start,
      sync_start        => sync_start,
      step_sel          => c_step_sel,
      shift_a           => shift_a,
      shift_b           => shift_b,
      freq_sel          => c_freq_sel,
      state_change_irq  => state_change_irq,
      dreq              => dreq,
      ramp_sec_fin      => ramp_sec_fin,
      sw_out            => sw_out,
      sw_strobe         => sw_strobe,
      fg_is_running     => fg_is_running
    );

  -- reference model and checker
  checker: process (clk)
    variable v_set:     integer;
    variable v_cnt:     integer;
    variable v_len:     integer;
    variable v_x:       signed(63 downto 0);
    variable v_q:       signed(63 downto 0);
    variable v_a:       signed(63 downto 0);
    variable v_dreq_d:  std_logic := '0';
    variable v_strb_d:  std_logic := '0';
    variable v_irq_d:   std_logic := '0';
    variable v_errors:  integer := 0;

    procedure load(idx: integer) is
    begin
      v_a := shift_left(resize(to_signed(c_sets(idx).a, 16), 64), c_sets(idx).sa);
      v_q := shift_left(resize(to_signed(c_sets(idx).b, 16), 64), c_sets(idx).sb);
      v_x := shift_left(resize(to_signed(c_sets(idx).c, 32), 64), 32);
    end procedure;
  begin
    if rising_edge(clk) then
      if sync_start = '1' then
        v_set := 0;
        v_cnt := 0;
        v_len := c_steps - 1;                 -- first segment has one sample less
        load(0);
        n_samples  <= 0;
        n_dreq     <= 0;
        n_stop_irq <= 0;
      else
        -- with ACU the strobe is as long as a 25 MHz period, so count edges
        if sw_strobe = '1' and v_strb_d = '0' then
          if v_cnt = v_len then
            v_set := v_set + 1;
            v_cnt := 0;
            v_len := c_steps;
            if v_set < n_sets then
              load(v_set);
            end if;
          end if;
          if v_set >= n_sets then
            report "sample after the last segment" severity error;
            v_errors := v_errors + 1;
          else
            v_x := v_x + v_q;
            v_q := v_q + v_a;
            last_out <= std_logic_vector(v_x(63 downto 32));
            if sw_out /= std_logic_vector(v_x(63 downto 32)) then
              report "set " & integer'image(v_set) & " sample " & integer'image(v_cnt) &
                     ": sw_out " & integer'image(to_integer(signed(sw_out))) &
                     " expected " & integer'image(to_integer(v_x(63 downto 32))) severity error;
              v_errors := v_errors + 1;
            end if;
          end if;
          v_cnt := v_cnt + 1;
          n_samples <= n_samples + 1;
        end if;

        if dreq = '1' and v_dreq_d = '0' then
          n_dreq <= n_dreq + 1;
        end if;

        -- stop irq must be visible on its own, an or-ed dreq would mask the edge
        if state_change_irq = '1' and v_irq_d = '0' and nrst = '1' then
          n_stop_irq <= n_stop_irq + 1;
          if dreq = '1' then
            report "stop irq while dreq is active" severity error;
            v_errors := v_errors + 1;
          end if;
        end if;
      end if;
      v_dreq_d := dreq;
      v_strb_d := sw_strobe;
      v_irq_d  := state_change_irq;
      n_errors <= v_errors;
    end if;
  end process;

  stimulus: process
    alias load_set      is <<signal .fg_quad_datapath_tb.dut.s_load_set : std_logic>>;
    alias sysclk_pulses is <<signal .fg_quad_datapath_tb.dut.sysclk_pulses : std_logic>>;
    variable v_failed: boolean := false;

    procedure tick(n: integer := 1) is
    begin
      for i in 1 to n loop
        wait until rising_edge(clk);
      end loop;
    end procedure;

    procedure pulse(signal s: out std_logic) is
    begin
      s <= '1';
      tick;
      s <= '0';
    end procedure;

    -- like the bus registers: data changes with the clock edge that sees the write strobe
    procedure write_set(idx: integer) is
    begin
      a_en       <= '1';
      load_start <= '1';
      tick;
      a_en       <= '0';
      load_start <= '0';
      data_a     <= std_logic_vector(to_signed(c_sets(idx).a, 16));
      data_b     <= std_logic_vector(to_signed(c_sets(idx).b, 16));
      data_c     <= std_logic_vector(to_signed(c_sets(idx).c, 32));
      shift_a    <= c_sets(idx).sa;
      shift_b    <= c_sets(idx).sb;
      tick;
    end procedure;

    procedure run_test(mode: t_mode) is
      constant c_seg_cycles: integer := c_steps * (c_clk_in_hz / 2_000_000);
      variable v_errors: integer;
    begin
      nrst <= '0';
      tick(10);
      nrst <= '1';
      tick(10);
      n_sets <= c_sets'length;
      v_errors := n_errors;

      write_set(0);
      tick(10);
      pulse(sync_start);
      if ACU then
        tick(10);
      end if;
      write_set(1);

      for k in 2 to c_sets'high loop
        case mode is
          when fast | slow =>
            wait until rising_edge(dreq) for 2 * c_seg_cycles * c_clk_period;
          when edge =>
            wait until falling_edge(clk) and load_set = '1' and (not ACU or sysclk_pulses = '1')
              for 2 * c_seg_cycles * c_clk_period;
        end case;
        if fg_is_running = '0' then
          report t_mode'image(mode) & ": generator stopped before set " & integer'image(k) & " was written"
            severity error;
          exit;
        end if;
        if mode = fast then
          tick(5);
        elsif mode = slow then
          tick(c_seg_cycles / 2);
        end if;
        write_set(k);
      end loop;

      wait until falling_edge(fg_is_running) for 2 * c_sets'length * c_seg_cycles * c_clk_period;
      tick(4 * c_seg_cycles);

      assert fg_is_running = '0'
        report t_mode'image(mode) & ": generator did not stop" severity error;
      assert n_samples = c_steps * c_sets'length - 1
        report t_mode'image(mode) & ": " & integer'image(n_samples) & " samples, expected " &
               integer'image(c_steps * c_sets'length - 1) severity error;
      assert n_dreq = c_sets'length - 1
        report t_mode'image(mode) & ": " & integer'image(n_dreq) & " dreq pulses, expected " &
               integer'image(c_sets'length - 1) severity error;
      assert n_stop_irq = 1
        report t_mode'image(mode) & ": " & integer'image(n_stop_irq) & " stop irqs, expected 1" severity error;
      assert sw_out = last_out
        report t_mode'image(mode) & ": sw_out " & integer'image(to_integer(signed(sw_out))) &
               " after stop, expected last sample " & integer'image(to_integer(signed(last_out))) severity error;

      if n_errors = v_errors and fg_is_running = '0' and n_samples = c_steps * c_sets'length - 1
         and n_dreq = c_sets'length - 1 and n_stop_irq = 1 and sw_out = last_out then
        report t_mode'image(mode) & ": passed";
      else
        report t_mode'image(mode) & ": FAILED" severity error;
        v_failed := true;
      end if;
    end procedure;

  begin
    report "fg_quad_datapath_tb, ACU = " & boolean'image(ACU);
    run_test(fast);
    run_test(slow);
    run_test(edge);
    assert not v_failed report "fg_quad_datapath_tb FAILED" severity failure;
    report "fg_quad_datapath_tb passed";
    sim_done <= true;
    wait;
  end process;

end architecture;
