-- Libraries
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library std;
use std.textio.all;
library work;
use work.wishbone_pkg.all;
use work.cellular_ram_pkg.all;

-- Entity (empty)
entity cellular_ram_testbench is

end;

-- Architecture
architecture rtl of cellular_ram_testbench is

  -- Testbench settings
  constant c_reset_time  : time    := 200 ns;
  constant c_clock_cycle : time    := 16 ns;

  signal s_rst_n : std_logic := '0';
  signal s_rst   : std_logic := '0';
  signal s_clk   : std_logic := '0';

  signal s_psram_clk  : std_logic;
  signal s_psram_addr : std_logic_vector(23 downto 0);
  signal s_psram_data : std_logic_vector(15 downto 0);
  signal s_psram_ubn  : std_logic;
  signal s_psram_lbn  : std_logic;
  signal s_psram_cen  : std_logic;
  signal s_psram_oen  : std_logic;
  signal s_psram_wen  : std_logic;
  signal s_psram_cre  : std_logic;
  signal s_psram_advn : std_logic;
  signal s_psram_wait : std_logic := '0';

  signal s_wb_slave_in  : t_wishbone_slave_in;
  signal s_wb_slave_out : t_wishbone_slave_out;

  -- Other constants
  constant c_reg_all_zero                : std_logic_vector(31 downto 0) := x"00000000";
  constant c_cyc_on                      : std_logic := '1';
  constant c_cyc_off                     : std_logic := '0';
  constant c_str_on                      : std_logic := '1';
  constant c_str_off                     : std_logic := '0';
  constant c_we_on                       : std_logic := '1';
  constant c_we_off                      : std_logic := '0';

  -- Functions
  -- Function wb_stim -> Helper function to create a human-readable testbench
  function wb_stim(cyc : std_logic; stb : std_logic; we : std_logic;
                   adr : t_wishbone_address; dat : t_wishbone_data) return t_wishbone_slave_in is
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

  type t_state is (S_RESET, S_IDLE, S_BCR_WRITE);
  signal r_state : t_state := S_RESET;
  signal s_int_delay_counter : std_logic_vector(31 downto 0);

begin

  -- Clock generator
  p_clock : process
  begin
    s_clk <= '0';
    wait for c_clock_cycle/2;
    s_clk <= '1' and s_rst_n;
    wait for c_clock_cycle/2;
  end process;

  -- Reset controller
  p_reset : process
  begin
    wait for c_reset_time;
    s_rst_n <= '1';
  end process;
  s_rst <= not(s_rst_n);

  -- PSRAM controller
  p_psram_fsm : process(s_clk, s_rst_n) is
  begin
    if s_rst_n = '0' then
     s_psram_wait <= '0';
     r_state <= S_RESET;
     s_int_delay_counter <= (others => '0');
    elsif rising_edge(s_clk) then
      case r_state is
        when S_RESET =>
          r_state <= S_IDLE;
          s_psram_wait <= '0';
          s_int_delay_counter <= (others => '0');
        when S_IDLE =>
          r_state <= S_IDLE;
          s_psram_wait <= '0';
        when S_BCR_WRITE =>
          r_state <= S_IDLE;
          s_psram_wait <= '0';
      end case;
    end if;
  end process;

  -- Cellular RAM
  u_cellular_ram_dut : cellular_ram
    port map (
      clk_i      => s_clk,
      rstn_i     => s_rst_n,
      slave_i    => s_wb_slave_in,
      slave_o    => s_wb_slave_out,
      ps_clk_o   => s_psram_clk,
      ps_addr_o  => s_psram_addr,
      ps_data_io => s_psram_data,
      ps_ubn_o   => s_psram_ubn,
      ps_lbn_o   => s_psram_lbn,
      ps_cen_o   => s_psram_cen,
      ps_oen_o   => s_psram_oen,
      ps_wen_o   => s_psram_wen,
      ps_cre_o   => s_psram_cre,
      ps_advn_o  => s_psram_advn,
      ps_wait_i  => s_psram_wait);

      -- Wishbone controller
      p_wishbone_stim : process
      begin
        -- Reset
        s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_all_zero, c_reg_all_zero);
        wait until rising_edge(s_rst_n);
        wait until rising_edge(s_clk);
      end process;

end;
