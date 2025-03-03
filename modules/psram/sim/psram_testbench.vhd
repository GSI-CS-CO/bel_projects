-- Libraries
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library std;
use std.textio.all;
library work;
use work.wishbone_pkg.all;
use work.psram_pkg.all;

-- Entity (empty)
entity psram_testbench is

end;

-- Architecture
architecture rtl of psram_testbench is

  -- Testbench settings
  constant c_reset_time  : time    := 200 ns;
  constant c_clock_cycle : time    := 16 ns;

  signal s_rst_n : std_logic := '0';
  signal s_rst   : std_logic := '0';
  signal s_clk   : std_logic := '0';

  signal s_psram_wait : std_logic := '0';

  signal s_wb_slave_in  : t_wishbone_slave_in;
  signal s_wb_slave_out : t_wishbone_slave_out;

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

  s_psram_wait <= '0';

  -- I2C master (XWB Wrapper)
  u_psram_dut : psram
    port map (
      clk_i    => s_clk,
      rstn_i   => s_rst_n,
      slave_i  => s_wb_slave_in,
      slave_o  => s_wb_slave_out,
      ps_wait  => s_psram_wait);


end;
