library ieee;
USE ieee.std_logic_1164.all;
use ieee.math_real.all;
use ieee.numeric_std.all;

library work;
use work.fg_quad_pkg.all;


-- ACU unmodified design runs with 100MHz
-- ACU modified design with use of sysclk_pulses runs with 25MHz
-- Following changes needed beside of sysclk_pulses
--      s_step_cnt and freq_cnt must run with 25MHz instead 100MHz
--      to achieve same times for ramps, counter span for freq_qnt is 1/4 of origin
--      for proper FSM timing all datapath components must also run with 25MHz sysclk_pulses
--      incoming signals like sync_rst,a_en,load_start, sync_start has to be stretched in pulse width
--      outgoing signals like ramp_sec_fin has to be shrinked in pulse width


entity fg_quad_datapath is
  generic (
    CLK_in_Hz:  integer := 125_000_000;
    ACU :       boolean := false
          );
  port (
  data_a:             in  std_logic_vector(15 downto 0);
  data_b:             in  std_logic_vector(15 downto 0);
  data_c:             in  std_logic_vector(31 downto 0);
  clk:                in  std_logic;
  sysclk:             in  std_logic;
  nrst:               in  std_logic;
  sync_rst:           in  std_logic;
  a_en:               in  std_logic;                      -- data register enable
  load_start:         in  std_logic;
  sync_start:         in  std_logic;
  step_sel:           in  std_logic_vector(2 downto 0);
  shift_a:            in  integer range 0 to 48;          -- shiftvalue coeff b
  shift_b:            in  integer range 0 to 48;          -- shiftvalue coeff b
  freq_sel:           in  std_logic_vector(2 downto 0);
  state_change_irq:   out std_logic;
  dreq:               out std_logic;
  ramp_sec_fin:       out std_logic;
  sw_out:             out std_logic_vector(31 downto 0);
  sw_strobe:          out std_logic;
  fg_is_running:      out std_logic);
end entity;

architecture fg_quad_dp_arch of fg_quad_datapath is

-- shifter signals
signal s_a_shifted:     signed(63 downto 0);
signal s_b_shifted:     signed(63 downto 0);

-- registerblock
signal s_a_reg:         signed(63 downto 0);
signal s_Q_reg:         signed(63 downto 0);
signal s_X_reg:         signed(63 downto 0);
signal freq_sel_reg:    std_logic_vector(2 downto 0);
signal step_sel_reg:    std_logic_vector(2 downto 0);


-- signals statemachine
signal s_inc_quad:      std_logic;
--signal s_inc_lin:     std_logic;
signal s_add_lin_quad:  std_logic;

type cntrl_type is (idle, load, quad_inc, lin_inc, addXQ);
signal control_state:   cntrl_type;
signal s_coeff_rcvd:    std_logic;


-- constants for frequency and step value counters
type int_array is array(7 downto 0) of integer;

constant c_freq_cnt: int_array := (
                                    (CLK_in_Hz / 2000000) - 2,
                                    (CLK_in_Hz / 1000000) - 2,
                                    (CLK_in_Hz / 500000) -2,
                                    (CLK_in_Hz / 250000) - 2,
                                    (CLK_in_Hz / 125000) - 2,
                                    (CLK_in_Hz / 64000) - 2,
                                    (CLK_in_Hz / 32000) - 2,
                                    (CLK_in_Hz / 16000) - 2
                                    );


constant c_freq_cnt_acu: int_array := (
                                    (CLK_in_Hz / 2000000/(CLK_in_Hz/25000000)) - 2,
                                    (CLK_in_Hz / 1000000/(CLK_in_Hz/25000000)) - 2,
                                    (CLK_in_Hz /  500000/(CLK_in_Hz/25000000))  -2,
                                    (CLK_in_Hz /  250000/(CLK_in_Hz/25000000)) - 2,
                                    (CLK_in_Hz /  125000/(CLK_in_Hz/25000000)) - 2,
                                    (CLK_in_Hz /   64000/(CLK_in_Hz/25000000)) - 2,
                                    (CLK_in_Hz /   32000/(CLK_in_Hz/25000000)) - 2,
                                    (CLK_in_Hz /   16000/(CLK_in_Hz/25000000)) - 2
                                    );


constant c_step_cnt: int_array := (
                                   32000 - 2,
                                   16000 - 2,
                                   8000 - 2,
                                   4000 - 2,
                                   2000 - 2,
                                   1000 - 2,
                                   500 - 2,
                                   250 - 2
                                   );



-- calculate width from the biggest value. high bit needs to be 0
constant  c_freq_cnt_width:       integer := integer(ceil(log2(real(c_freq_cnt(0))))) + 1;
constant  c_freq_cnt_acu_width:   integer := integer(ceil(log2(real(c_freq_cnt_acu(0))))) + 1;

constant  c_step_cnt_width:       integer := integer(ceil(log2(real(c_step_cnt(7))))) + 1;



signal s_cnt:                     unsigned(7 downto 0);
signal s_freq_cnt:                unsigned(c_freq_cnt_width - 1 downto 0);
signal s_freq_cnt_acu:            unsigned(c_freq_cnt_acu_width - 1 downto 0);
signal s_freq_en:                 std_logic;
signal s_freq_cnt_en:             std_logic := '1';

signal s_step_cnt:                unsigned(c_step_cnt_width - 1 downto 0);
signal s_stp_reached:             std_logic;
signal s_cont:                    std_logic;

signal s_is_running:              std_logic;
signal s_signal_state_change:     std_logic;
signal s_segment_finished:        std_logic;

signal sysclk_pulses:             std_logic;
signal sysclk_dly1:               std_logic;
signal sysclk_dly2:               std_logic;
signal sysclk_dly3:               std_logic;
signal s_segment_finished_dly:    std_logic;

signal sync_start_stretched:      std_logic;
signal load_start_stretched:      std_logic;
signal a_en_stretched:            std_logic;
signal sync_rst_stretched:        std_logic;


begin



SCU_SLAVES_IMPLEM: if ACU = false generate

-- registers a, b, Q, X, start und adder for lin und quad term
reg_file: process (clk, nrst, sync_rst, a_en, load_start)
begin
  if nrst = '0' then
    s_a_reg       <=  (others => '0');
    s_Q_reg       <=  (others => '0');
    s_X_reg       <=  (others => '0');
    freq_sel_reg  <=  (others => '0');
    step_sel_reg  <=  (others => '0');
  elsif rising_edge(clk) then
    if sync_rst = '1' then
      s_a_reg       <=  (others => '0');
      s_Q_reg       <=  (others => '0');
      s_X_reg       <=  (others => '0');
      freq_sel_reg  <=  (others => '0');
      step_sel_reg  <=  (others => '0');
    else
      if (s_stp_reached = '1' or sync_start = '1') and s_coeff_rcvd = '1' then
        -- shifting for quadratic coefficient a
        s_a_reg <= shift_left(resize(signed(data_a), 64), shift_a);
        s_Q_reg <= shift_left(resize(signed(data_b), 64), shift_b); -- Q0 = b
        s_X_reg <= shift_left(resize(signed(data_c), 64), 32);      -- X0 = c
      end if;

      if (s_stp_reached = '1' or load_start = '1') and s_coeff_rcvd = '1' then
        step_sel_reg <= step_sel;
        freq_sel_reg <= freq_sel;
      end if;

      -- increment quad term
      if s_inc_quad = '1' then
        s_Q_reg <= s_Q_reg + s_a_reg;
        s_X_reg <= s_X_reg + s_Q_reg;
      end if;

      -- sum of linear and quadratic term
      if s_add_lin_quad = '1' then
        --s_X_reg <= signed(s_X_reg) + signed(s_Q_reg);
      end if;
    end if;
  end if;

end process;


  -- downcounter for frequency division
  freq_cnt: process(clk, nrst, sync_rst, freq_sel, sync_start, freq_sel_reg)
  begin
    if nrst = '0' then
      s_freq_cnt <= to_unsigned(c_freq_cnt(0), c_freq_cnt_width);
    elsif rising_edge(clk) then
      if sync_rst = '1' or sync_start = '1' then
        s_freq_cnt <= to_unsigned(c_freq_cnt(to_integer(unsigned(freq_sel_reg))), c_freq_cnt_width);
      else
        -- reload count at overflow
        if s_freq_en = '1' then
          -- choosing constant from array
          s_freq_cnt <= to_unsigned(c_freq_cnt(to_integer(unsigned(freq_sel_reg))), c_freq_cnt_width);
        elsif s_freq_cnt_en = '1' then
          s_freq_cnt <= s_freq_cnt - 1;
        end if;
      end if;
    end if;
  end process freq_cnt;

  s_freq_en <= s_freq_cnt(s_freq_cnt'high);


  s_stp_reached <= std_logic(s_step_cnt(s_step_cnt'high));

-- control state machine
  control_sm: process (clk, nrst, sync_rst, s_cont)
  begin
    if nrst = '0' then
      control_state         <= idle;
      s_is_running          <= '0';
      s_inc_quad            <= '0';
      s_signal_state_change <= '0';
      s_segment_finished    <= '0';

    elsif rising_edge(clk) then
      if sync_rst = '1' then
        control_state         <= idle;
        s_is_running          <= '0';
        s_inc_quad            <= '0';
        s_signal_state_change <= '0';
        s_segment_finished    <= '0';
      else
        s_inc_quad            <= '0';
        s_add_lin_quad        <= '0';
        s_freq_cnt_en         <= '1';
        s_is_running          <= '0';
        s_signal_state_change <= '0';
        s_segment_finished    <= '0';

        if a_en = '1' then
          s_coeff_rcvd <= '1';
        end if;

        case control_state is
          when idle =>
            s_freq_cnt_en   <= '0';
            -- load step counter from array
            s_step_cnt <= to_unsigned(c_step_cnt(to_integer(unsigned(step_sel_reg))), c_step_cnt_width);
            if sync_start = '1' then
              s_coeff_rcvd <= '0';
              control_state <= quad_inc;
            end if;

          when quad_inc =>
            s_is_running <= '1';
            if s_stp_reached = '1' and s_freq_en = '1' then
              if s_coeff_rcvd = '1' then
                -- continue with new parameter set
                -- reload step counter from array
                s_step_cnt <= to_unsigned(c_step_cnt(to_integer(unsigned(step_sel_reg))), c_step_cnt_width);
                s_coeff_rcvd <= '0';
                s_inc_quad <= '1';
                control_state <= lin_inc;
              else
                -- no parameters received
                -- go to stop mode
                s_signal_state_change <= '1';
                control_state <= idle;
              end if;

            elsif s_freq_en = '1' then
              s_step_cnt <= s_step_cnt - 1;
              s_inc_quad <= '1';
              control_state <= lin_inc;
            end if;

          when lin_inc =>
            s_is_running <= '1';
            if s_stp_reached = '1' then
              s_segment_finished <= '1';
            end if;
            control_state <= addXQ;

          when addXQ =>
              s_is_running <= '1';
              s_add_lin_quad <= '1';
              control_state <= quad_inc;

          when others =>

        end case;
      end if;
    end if;
  end process;

ramp_sec_fin     <= s_segment_finished;


end generate SCU_SLAVES_IMPLEM;

-------------------------------------------------------------------------------------------------------------------------



ACU_IMPLEM: if ACU = true generate





sysclk_pulses        <= sysclk_dly2 xor sysclk_dly3;

-- 4 high-active pulses need to be latched until next sysclk_puls is effective

stretch: process (clk, nrst)
begin
  if nrst = '0' then

    sysclk_dly1              <='0';
    sysclk_dly2              <='0';
    sysclk_dly3              <='0';
    s_segment_finished_dly   <='0';

    sync_start_stretched     <='0';
    a_en_stretched           <='0';
    load_start_stretched     <='0';
    sync_rst_stretched       <='0';


   elsif rising_edge(clk) then
    sysclk_dly1              <= sysclk;
    sysclk_dly2              <= sysclk_dly1;
    sysclk_dly3              <= sysclk_dly2;
    s_segment_finished_dly   <= s_segment_finished;

    if sync_start ='1' then           -- depends on FSM
       sync_start_stretched  <='1';
    elsif sysclk_pulses ='1' then
       sync_start_stretched  <='0';
    else
       null;
    end if;

    if load_start ='1' then           -- depends on Ext_Wr_Active
       load_start_stretched  <='1';
    elsif sysclk_pulses ='1' then
       load_start_stretched  <='0';
    else
       null;
    end if;

    if a_en ='1' then
       a_en_stretched        <='1';    -- depends on Ext_Wr_Active
    elsif sysclk_pulses ='1' then
       a_en_stretched        <='0';
    else
       null;
    end if;

    if sync_rst ='1' then
       sync_rst_stretched    <='1';   -- depends on Ext_Wr_Active
    elsif sysclk_pulses ='1' then
       sync_rst_stretched    <='0';
    else
       null;
    end if;

  end if;

end process stretch;

-- registers a, b, Q, X, start und adder for lin und quad term
reg_file: process (clk, nrst, sync_rst_stretched, a_en_stretched, load_start_stretched)
begin
  if nrst = '0' then

    s_a_reg         <=  (others => '0');
    s_Q_reg         <=  (others => '0');
    s_X_reg         <=  (others => '0');
    freq_sel_reg    <=  (others => '0');
    step_sel_reg    <=  (others => '0');

  elsif rising_edge(clk) then

    if sysclk_pulses ='1' then

      if sync_rst_stretched = '1' then
        s_a_reg       <=  (others => '0');
        s_Q_reg       <=  (others => '0');
        s_X_reg       <=  (others => '0');
        freq_sel_reg  <=  (others => '0');
        step_sel_reg  <=  (others => '0');
      else
        if (s_stp_reached = '1' or sync_start_stretched = '1' ) and s_coeff_rcvd = '1' then
          -- shifting for quadratic coefficient a
          s_a_reg <= shift_left(resize(signed(data_a), 64), shift_a);
          s_Q_reg <= shift_left(resize(signed(data_b), 64), shift_b); -- Q0 = b
          s_X_reg <= shift_left(resize(signed(data_c), 64), 32);      -- X0 = c
        end if;

        if (s_stp_reached = '1' or load_start_stretched = '1') and s_coeff_rcvd = '1' then
          step_sel_reg <= step_sel;
          freq_sel_reg <= freq_sel;
        end if;

        -- increment quad term
        if s_inc_quad = '1' then
          s_Q_reg <= s_Q_reg + s_a_reg;
          s_X_reg <= s_X_reg + s_Q_reg;
        end if;

        -- sum of linear and quadratic term
        if s_add_lin_quad = '1' then
          --s_X_reg <= signed(s_X_reg) + signed(s_Q_reg);
        end if;
      end if;

    else
      null;
    end if; --sysclk_pulses

  end if;

end process;


  -- downcounter for frequency division
  freq_cnt: process(clk, nrst, sync_rst_stretched, freq_sel, sync_start_stretched, freq_sel_reg)
  begin
    if nrst = '0' then
      s_freq_cnt_acu <= to_unsigned(c_freq_cnt_acu(0), c_freq_cnt_acu_width);
    elsif rising_edge(clk) then

      if sysclk_pulses ='1' then

        if sync_rst_stretched = '1' or sync_start_stretched = '1' then
          s_freq_cnt_acu <= to_unsigned(c_freq_cnt_acu(to_integer(unsigned(freq_sel_reg))), c_freq_cnt_acu_width);
        else
          -- reload count at overflow
          if s_freq_en = '1' then
            -- choosing constant from array
            s_freq_cnt_acu <= to_unsigned(c_freq_cnt_acu(to_integer(unsigned(freq_sel_reg))), c_freq_cnt_acu_width);
          elsif s_freq_cnt_en = '1' then
            s_freq_cnt_acu <= s_freq_cnt_acu - 1;
          end if;
        end if;

      else
        null;
      end if;--sysclk_pulses

    end if;
  end process freq_cnt;

  s_freq_en <= s_freq_cnt_acu(s_freq_cnt_acu'high);


  s_stp_reached <= std_logic(s_step_cnt(s_step_cnt'high));

-- control state machine
  control_sm: process (clk, nrst, sync_rst_stretched, s_cont)
  begin
    if nrst = '0' then
      control_state           <= idle;
      s_is_running            <= '0';
      s_inc_quad              <= '0';
      s_signal_state_change   <= '0';
      s_segment_finished      <= '0';

    elsif rising_edge(clk) then

    if sysclk_pulses ='1' then

      if sync_rst_stretched = '1' then
        control_state         <= idle;
        s_is_running          <= '0';
        s_inc_quad            <= '0';
        s_signal_state_change <= '0';
        s_segment_finished    <= '0';
      else
        s_inc_quad            <= '0';
        s_add_lin_quad        <= '0';
        s_freq_cnt_en         <= '1';
        s_is_running          <= '0';
        s_signal_state_change <= '0';
        s_segment_finished    <= '0';

        if a_en_stretched = '1' then
          s_coeff_rcvd <= '1';
        end if;

        case control_state is
          when idle =>
            s_freq_cnt_en   <= '0';
            -- load step counter from array
            s_step_cnt <= to_unsigned(c_step_cnt(to_integer(unsigned(step_sel_reg))), c_step_cnt_width);
            if sync_start_stretched = '1' then
              s_coeff_rcvd <= '0';
              control_state <= quad_inc;
            end if;

          when quad_inc =>
            s_is_running <= '1';
            if s_stp_reached = '1' and s_freq_en = '1' then
              if s_coeff_rcvd = '1' then
                -- continue with new parameter set
                -- reload step counter from array
                s_step_cnt <= to_unsigned(c_step_cnt(to_integer(unsigned(step_sel_reg))), c_step_cnt_width);
                s_coeff_rcvd <= '0';
                s_inc_quad <= '1';
                control_state <= lin_inc;
              else
                -- no parameters received
                -- go to stop mode
                s_signal_state_change <= '1';
                control_state <= idle;
              end if;

            elsif s_freq_en = '1' then
              s_step_cnt <= s_step_cnt - 1;
              s_inc_quad <= '1';
              control_state <= lin_inc;
            end if;

          when lin_inc =>
            s_is_running <= '1';
            if s_stp_reached = '1' then
              s_segment_finished <= '1';
            end if;
            control_state <= addXQ;

          when addXQ =>
              s_is_running <= '1';
              s_add_lin_quad <= '1';
              control_state <= quad_inc;

          when others =>

        end case;
      end if; --sync_rst_stretched

	 else
      null;
    end if;--sysclk_pulses


    end if; --rising_edge
  end process;

ramp_sec_fin     <= s_segment_finished and not s_segment_finished_dly;  --need a pulse for ramp_cnt

end generate ACU_IMPLEM;

-- Interrupts are edge detect on scu_bus_slave_macro , no need to shrink them

state_change_irq <= sync_start or s_signal_state_change;
dreq             <= s_stp_reached;

-- output register for the 24 most significant bits
sw_out    <= std_logic_vector(s_X_reg(63 downto 32));
sw_strobe <= s_add_lin_quad;

fg_is_running <= s_is_running;

end FG_quad_DP_arch;
