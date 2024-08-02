--wishbone slave testbench
--bare bones testbench to test slave communication

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library std;
use std.textio.all;
library work;
use work.wishbone_pkg.all;


--Entity(empty)
entity enc_err_counter_tb is
end;

architecture enc_err_counter_tb_arc of enc_err_counter_tb is
  --Testbench settings
  constant c_reset_time       : time  := 190 ns;
  constant c_sys_clock_cycle  : time  := 16.1 ns;
  constant c_ref_clock_cycle  : time  := 7.9 ns;
  
  -- Other constants
  constant c_cyc_on       : std_logic := '1';
  constant c_cyc_off      : std_logic := '0';
  constant c_str_on       : std_logic := '1';
  constant c_str_off      : std_logic := '0';
  constant c_we_on        : std_logic := '1';
  constant c_we_off       : std_logic := '0';
  constant c_reg_all_zero : std_logic_vector(31 downto 0) := x"00000000";
  
  -- Basic device signals
  signal s_clk_sys  : std_logic := '0';
  signal s_clk_ref  : std_logic := '0';
  signal s_rst_n    : std_logic := '0';
  signal s_rst      : std_logic := '0';
  
  -- Wishbone connections
  signal s_wb_slave_in  : t_wishbone_slave_in;
  signal s_wb_slave_out : t_wishbone_slave_out;
  signal s_wb_desc_out  : t_wishbone_device_descriptor;
  
  -- Testbench logic
  signal s_ack            : std_logic             := '0';
  signal enc_err          : std_logic             := '0';
  signal enc_err_aux      : std_logic             := '0';
  signal nerrors          : unsigned(31 downto 0) := (others => '0');
  signal nerrors_aux      : unsigned(31 downto 0) := (others => '0');

  -- control signals error generator
  signal s_err_idle         : std_logic := '0';
  signal s_err_error        : std_logic := '0';
  signal s_err_await        : std_logic := '0';
  signal s_done_generating  : std_logic := '0';

  -- control signal assertion
  signal s_ass_idle             : std_logic := '0';
  signal s_ass_set_wb           : std_logic := '0';
  signal s_ass_reset            : std_logic := '0';
  signal s_done_checking_error  : std_logic := '0';

  type t_gen_err_FSM is (idle, error, await_test);
  signal s_gen_err_FSM  : t_gen_err_FSM := idle;
  signal s_next_err     : t_gen_err_FSM := idle;
  
  type t_assertion_FSM is (idle, set_wb1, check_wb1, set_wb2, check_wb2, reset, reset2, reset3, reset4, reset5, reset6, reset7, reset8);
  signal s_assertion_FSM  : t_assertion_FSM := idle;
  signal s_next_assert    : t_assertion_FSM := idle;

  -- Functions
  -- Function wb_stim -> Helper function to create a human-readable testbench
  function wb_stim(cyc : std_logic; stb : std_logic; we : std_logic; adr : t_wishbone_address; dat : t_wishbone_data) return t_wishbone_slave_in is
    variable v_setup : t_wishbone_slave_in;
  begin
    v_setup.cyc := cyc;
    v_setup.stb := stb;
    v_setup.we  := we;
    v_setup.adr := adr;
    v_setup.dat := dat;
    v_setup.sel := (others => '0'); -- Don't care
    return v_setup;
  end function wb_stim;
  
  -- Procedures
  -- Procedure wb_expect -> Check WB slave answer
  procedure wb_expect(msg : string; dat_from_slave : t_wishbone_data; compare_value : t_wishbone_data) is
  begin
    if (to_integer(unsigned(dat_from_slave)) = to_integer(unsigned(compare_value))) then
      report "Test passed: " & msg;
    else
      report "Test errored: " & msg;
      report "-> Info:  Answer from slave:          " & integer'image(to_integer(unsigned(dat_from_slave)));
      report "-> Error: Expected answer from slave: " & integer'image(to_integer(unsigned(compare_value)));
    end if;
  end procedure wb_expect;
  
  component enc_err_counter
    generic (
      g_aux_phy_interface : in boolean
    );
    port (
      clk_sys_i     : in std_logic;
      clk_ref_i     : in std_logic;
      rstn_sys_i    : in std_logic;
      rstn_ref_i    : in std_logic;

      slave_o       : out t_wishbone_slave_out;
      slave_i       : in  t_wishbone_slave_in;

      enc_err_i     : in std_logic;
      enc_err_aux_i : in std_logic
    );
  end component;
begin------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  dut : enc_err_counter
    generic map (
      g_aux_phy_interface => true    
    )
    port map (
      clk_sys_i     => s_clk_sys,
      clk_ref_i     => s_clk_ref,
      rstn_sys_i    => s_rst_n,
      rstn_ref_i    => s_rst_n,

      slave_o       => s_wb_slave_out,
      slave_i       => s_wb_slave_in,

      enc_err_i     => enc_err,
      enc_err_aux_i => enc_err_aux
    );

  -- Reset controller
  p_reset : process
  begin
    wait for c_reset_time;
    s_rst_n <= '1';
  end process;
  s_rst <= not(s_rst_n);
  
  -- Short ack
   s_ack <= s_wb_slave_out.ack;

p_next_error : process(s_clk_ref, s_rst_n) begin
  if s_rst_n = '1' then
    if rising_edge(s_clk_ref) then
      if s_gen_err_FSM = await_test and (s_next_err = idle or s_next_err = error) then
        nerrors <= (others => '0');
        nerrors_aux <= (others => '0');
      end if;
      s_gen_err_FSM <= s_next_err;
      if s_gen_err_FSM = error then
        nerrors <= nerrors + 1;
        nerrors_aux <= nerrors_aux + 1;
      end if;
    end if;
  else
    s_gen_err_FSM <= idle;
  end if;
end process;

p_err_gen_FSM : process(s_gen_err_FSM, s_err_idle, s_err_error, s_err_await, s_done_checking_error)
begin
  s_done_generating <= '0';

  case s_gen_err_FSM is
    when idle =>
    enc_err <= '0';
    enc_err_aux <= '0';

    if s_err_error = '1' then
      s_next_err <= error;
    elsif s_err_await ='1' then
      s_next_err <= await_test;
    else
      s_next_err <= idle;
    end if;

    when error =>
    enc_err <= '1';
    enc_err_aux <= '1';
  
    if s_err_idle = '1' then
      s_next_err <= idle;
    elsif s_err_await ='1' then
      s_next_err <= await_test;
    else
      s_next_err <= error;
    end if;

    when await_test =>
    enc_err <= '0';
    enc_err_aux <= '0';
    s_done_generating <= '1';

    if s_err_error = '1' and s_done_checking_error = '1' then
      s_next_err <= error;
    elsif s_done_checking_error = '1' then
      s_next_err <= idle;
    else
      s_next_err <= await_test;
    end if;
  end case;
end process;

p_next_assertion : process(s_clk_sys, s_rst_n) begin
  if s_rst_n = '1' then
    if rising_edge(s_clk_sys) then
      s_assertion_FSM <= s_next_assert;
    end if;
    if s_assertion_FSM = reset8 then
      s_done_checking_error <= '1';
    else
      s_done_checking_error <= '0';
    end if;
  else
    s_assertion_FSM <= idle;
  end if;
end process;

p_assertion_FSM : process(s_assertion_FSM, s_done_generating, s_ass_set_wb, s_ass_reset, s_ass_idle) begin
  case s_assertion_FSM is
    when idle =>
    s_wb_slave_in  <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_all_zero, c_reg_all_zero);

    if s_done_generating = '1' and s_ass_set_wb = '1' then
      s_next_assert <= set_wb1;
    elsif s_done_generating = '1' and s_ass_reset = '1' then
      s_next_assert <= reset;
    else
      s_next_assert <= idle;
    end if;

    when set_wb1 =>
    s_wb_slave_in <= wb_stim(c_cyc_on, c_str_on, c_we_off, x"00000000", c_reg_all_zero);
    
    s_next_assert <= check_wb1;

    when check_wb1 =>
    s_wb_slave_in <= wb_stim(c_cyc_on, c_str_on, c_we_off, x"00000000", c_reg_all_zero);
    wb_expect("", s_wb_slave_out.dat, t_wishbone_data(nerrors));

    s_next_assert <= set_wb2;

    when set_wb2 =>
    s_wb_slave_in <= wb_stim(c_cyc_on, c_str_on, c_we_off, x"00000004", c_reg_all_zero);
    
    s_next_assert <= check_wb2;

    when check_wb2 =>
    s_wb_slave_in <= wb_stim(c_cyc_on, c_str_on, c_we_off, x"00000004", c_reg_all_zero);
    wb_expect("", s_wb_slave_out.dat, t_wishbone_data(nerrors_aux));

    if s_ass_reset = '1' then
      s_next_assert <= reset;
    else
      s_next_assert <= idle;
    end if;

    when reset =>
    s_wb_slave_in <= wb_stim(c_cyc_on, c_str_on, c_we_on, x"00000000", x"00000001");

    s_next_assert <= reset2;

    when reset2 =>
    s_wb_slave_in <= wb_stim(c_cyc_on, c_str_on, c_we_on, x"00000000", x"00000001");

    s_next_assert <= reset3;

    when reset3 =>
    s_wb_slave_in <= wb_stim(c_cyc_on, c_str_on, c_we_on, x"00000004", x"00000001");

    s_next_assert <= reset4;

    when reset4 =>
    s_wb_slave_in <= wb_stim(c_cyc_on, c_str_on, c_we_on, x"00000004", x"00000001");

    s_next_assert <= reset5;

    when reset5 =>
    s_wb_slave_in <= wb_stim(c_cyc_on, c_str_on, c_we_on, x"00000000", x"00000000");

    s_next_assert <= reset6;

    when reset6 =>
    s_wb_slave_in <= wb_stim(c_cyc_on, c_str_on, c_we_on, x"00000000", x"00000000");

    s_next_assert <= reset7;

    when reset7 =>
    s_wb_slave_in <= wb_stim(c_cyc_on, c_str_on, c_we_on, x"00000004", x"00000000");

    s_next_assert <= reset8;

    when reset8 =>
    s_wb_slave_in <= wb_stim(c_cyc_on, c_str_on, c_we_on, x"00000004", x"00000000");

    if s_ass_set_wb = '1' then
      s_next_assert <= set_wb1;
    else
      s_next_assert <= idle;
    end if;

  end case;
end process;

