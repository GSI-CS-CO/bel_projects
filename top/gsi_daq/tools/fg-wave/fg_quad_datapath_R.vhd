library ieee;
USE ieee.std_logic_1164.all;
use ieee.math_real.all;
use ieee.numeric_std.all;

library work;
use work.fg_quad_pkg.all;

---------------------------------------------------------------------------------------------------
entity FG_QUAD_DATAPATH is
   generic
   (
      CLK_in_Hz:  integer := 125_000_000
   );

   port
   (
      data_a_in:             in  std_logic_vector(15 downto 0);
      data_b_in:             in  std_logic_vector(15 downto 0);
      data_c_in:             in  std_logic_vector(31 downto 0);
      clk_in:                in  std_logic;
      nrst_in:               in  std_logic;                   -- async active low reset
      sync_rst_in:           in  std_logic;                   -- sync active low reset
      a_en_in:               in  std_logic;                   -- data register enable
      load_start_in:         in  std_logic;
      sync_start_in:         in  std_logic;
      step_sel_in:           in  std_logic_vector(2 downto 0);
      shift_a_in:            in  integer range 0 to 48;          -- shiftvalue coeff b
      shift_b_in:            in  integer range 0 to 48;          -- shiftvalue coeff b
      freq_sel_in:           in  std_logic_vector(2 downto 0);
      state_change_irq_out:  out std_logic;
      dreq_out:              out std_logic;
      ramp_sec_fin_out:      out std_logic;
      sw_out:                out std_logic_vector(31 downto 0);
      sw_strobe_out:         out std_logic;
      fg_is_running_out:     out std_logic
   );
end entity FG_QUAD_DATAPATH;

---------------------------------------------------------------------------------------------------
architecture FG_QUAD_DP_ARCH of FG_QUAD_DATAPATH is

   -- shifter signals
   -- signal s_a_shifted:   signed(63 downto 0);
   -- signal s_b_shifted:   signed(63 downto 0);

   -- registerblock
   signal s_a_reg:       signed(63 downto 0);
   signal s_Q_reg:       signed(63 downto 0);
   signal s_X_reg:       signed(63 downto 0);
   signal freq_sel_reg:  std_logic_vector(2 downto 0);
   signal step_sel_reg:  std_logic_vector(2 downto 0);


   -- signals statemachine
   signal s_inc_quad:      std_logic;
   --signal s_inc_lin:       std_logic;
   signal s_add_lin_quad:  std_logic;

   type STATE_T is ( ST_IDLE, ST_LOAD, ST_QUAD_INC, ST_LIN_INC, ST_ADD_XQ );
   signal control_state: STATE_T;

   signal s_coeff_rcvd:  std_logic;


   -- constants for frequency and step value counters
   type INT_ARRAY_T is array(7 downto 0) of integer;
  
   constant aFrequCount: INT_ARRAY_T :=
   (
      (CLK_in_Hz / 2000000) - 2,
      (CLK_in_Hz / 1000000) - 2,
      (CLK_in_Hz / 500000) -2,
      (CLK_in_Hz / 250000) - 2,
      (CLK_in_Hz / 125000) - 2,
      (CLK_in_Hz / 64000) - 2,
      (CLK_in_Hz / 32000) - 2,
      (CLK_in_Hz / 16000) - 2
   );

   constant aStepCount: INT_ARRAY_T :=
   (
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
   constant  C_FREQ_CNT_WIDTH:       integer := integer( ceil( log2( real(aFrequCount(0))))) + 1;
   constant  C_STEP_CNT_WIDTH:       integer := integer( ceil( log2( real(aStepCount(7))))) + 1;


   signal s_cnt:         unsigned( 7 downto 0 );
   signal s_freq_cnt:    unsigned( C_FREQ_CNT_WIDTH - 1 downto 0 );
   signal s_freq_en:     std_logic;
   signal s_freq_cnt_en: std_logic := '1';

   signal s_step_cnt:      unsigned( C_STEP_CNT_WIDTH - 1 downto 0 );
   signal s_stp_reached:   std_logic;
   signal s_cont:          std_logic;

   signal s_is_running:          std_logic;
   signal s_signal_state_change: std_logic;
   signal s_segment_finished:    std_logic;

begin

   -- registers a, b, Q, X, start and adder for linear and quadratic term
   REG_FILE: process( clk_in, nrst_in, sync_rst_in, a_en_in, load_start_in )
   begin
      if nrst_in = '0' then
         s_a_reg       <=  (others => '0');
         s_Q_reg       <=  (others => '0');
         s_X_reg       <=  (others => '0');
         freq_sel_reg  <=  (others => '0');
         step_sel_reg  <=  (others => '0');
      elsif rising_edge( clk_in ) then
         if sync_rst_in = '1' then
            s_a_reg       <=  (others => '0');
            s_Q_reg       <=  (others => '0');
            s_X_reg       <=  (others => '0');
            freq_sel_reg  <=  (others => '0');
            step_sel_reg  <=  (others => '0');
         else
            if s_stp_reached = '1' or sync_start_in = '1' then
              -- shifting for quadratic coefficient a
              s_a_reg <= shift_left(resize(signed(data_a_in), 64), shift_a_in);
              s_Q_reg <= shift_left(resize(signed(data_b_in), 64), shift_b_in); -- Q0 = b
              s_X_reg <= shift_left(resize(signed(data_c_in), 64), 32);         -- X0 = c
            end if;

            if s_stp_reached = '1' or load_start_in = '1' then
               step_sel_reg <= step_sel_in;
               freq_sel_reg <= freq_sel_in;
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
   end process REG_FILE;


  -- downcounter for frequency division
   FREQ_CNT: process( clk_in, nrst_in, sync_rst_in, freq_sel_in, sync_start_in, freq_sel_reg )
   begin
     if nrst_in = '0' then
        s_freq_cnt <= to_unsigned(aFrequCount(0), C_FREQ_CNT_WIDTH);
     elsif rising_edge(clk_in) then
        if sync_rst_in = '1' or sync_start_in = '1' then
           s_freq_cnt <= to_unsigned( aFrequCount(to_integer(unsigned(freq_sel_reg))), C_FREQ_CNT_WIDTH);
        else
           -- reload count at overflow
           if s_freq_en = '1' then
             -- choosing constant from array
              s_freq_cnt <= to_unsigned( aFrequCount(to_integer(unsigned(freq_sel_reg))), C_FREQ_CNT_WIDTH);
           elsif s_freq_cnt_en = '1' then
              s_freq_cnt <= s_freq_cnt - 1;
           end if;
        end if;
     end if;
   end process FREQ_CNT;

  s_freq_en     <= s_freq_cnt( s_freq_cnt'high );
  s_stp_reached <= std_logic( s_step_cnt( s_step_cnt'high ));

-- control state machine
   CONTROL_FSM: process (clk_in, nrst_in, sync_rst_in, s_cont)
   begin
      if nrst_in = '0' then
         control_state         <= ST_IDLE;
         s_is_running          <= '0';
         s_inc_quad            <= '0';
         s_signal_state_change <= '0';
         s_segment_finished    <= '0';
      elsif rising_edge(clk_in) then
         if sync_rst_in = '1' then
            control_state         <= ST_IDLE;
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
            if a_en_in = '1' then
               s_coeff_rcvd <= '1';
            end if;

            case control_state is
               when ST_IDLE =>
                 s_freq_cnt_en   <= '0';
                 -- load step counter from array
                 s_step_cnt <= to_unsigned( aStepCount(to_integer(unsigned(step_sel_reg))), C_STEP_CNT_WIDTH);
                 if sync_start_in = '1' then
                   s_coeff_rcvd <= '0';
                   control_state <= ST_QUAD_INC;
                 end if;

               when ST_QUAD_INC =>
                  s_is_running <= '1';
                  if s_stp_reached = '1' and s_freq_en = '1' then
                     if s_coeff_rcvd = '1' then
                      -- continue with new parameter set
                      -- reload step counter from array
                        s_step_cnt <= to_unsigned( aStepCount(to_integer(unsigned(step_sel_reg))), C_STEP_CNT_WIDTH);
                        s_coeff_rcvd <= '0';
                        s_inc_quad <= '1';
                        control_state <= ST_LIN_INC;
                     else
                     -- no parameters received
                     -- go to stop mode
                        s_signal_state_change <= '1';
                        control_state <= ST_IDLE;
                     end if;
                  elsif s_freq_en = '1' then
                     s_step_cnt    <= s_step_cnt - 1;
                     s_inc_quad    <= '1';
                     control_state <= ST_LIN_INC;
                  end if;

               when ST_LIN_INC =>
                  s_is_running <= '1';
                  if s_stp_reached = '1' then
                     s_segment_finished <= '1';
                  end if;
                  control_state <= ST_ADD_XQ;

               when ST_ADD_XQ =>
                  s_is_running <= '1';
                  s_add_lin_quad <= '1';
                  control_state <= ST_QUAD_INC;

               when others =>

            end case; --control_state
         end if;
      end if;
   end process CONTROL_FSM;

   state_change_irq_out <= sync_start_in or s_signal_state_change;
   dreq_out             <= s_stp_reached;
   ramp_sec_fin_out     <= s_segment_finished;
   -- output register for the 24 most significant bits
   sw_out               <= std_logic_vector(s_X_reg(63 downto 32));
   sw_strobe_out        <= s_add_lin_quad;
   fg_is_running_out    <= s_is_running;

end FG_QUAD_DP_ARCH;

------------------------------------ EOF --------------------------------------
