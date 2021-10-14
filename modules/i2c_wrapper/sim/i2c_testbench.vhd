-- Synopsys
-- Small testbench to take this i2c master into operation and observe Wishbone wrapper handling
--
-- Prescaler:
-- Value = (wb_clk_frequency)/(5*desired_frequency)
-- Reset = 0xffff

-- Libraries
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library std;
use std.textio.all;
library work;
use work.wishbone_pkg.all;

-- Entity (empty)
entity i2c_testbench is
  generic (g_interfaces : integer := 5);
  port  (scl_pad_io : inout std_logic_vector(g_interfaces-1 downto 0);
         sda_pad_io : inout std_logic_vector(g_interfaces-1 downto 0));
end;

-- Architecture
architecture rtl of i2c_testbench is

  -- Testbench settings
  constant c_interfaces  : integer := g_interfaces;
  constant c_reset_time  : time    := 200 ns;
  constant c_clock_cycle : time    := 16 ns;

 -- I2C master registers
  constant c_reg_prer_lo : std_logic_vector(31 downto 0) := x"00000000"; -- RW clock prescale low-byte
  constant c_reg_prer_hi : std_logic_vector(31 downto 0) := x"00000001"; -- RW clock prescale high-byte
  constant c_reg_ctr     : std_logic_vector(31 downto 0) := x"00000002"; -- RW control register
  constant c_reg_txr     : std_logic_vector(31 downto 0) := x"00000003"; -- #W transmit register
  constant c_reg_rxr     : std_logic_vector(31 downto 0) := x"00000003"; -- R# receive register
  constant c_reg_cr      : std_logic_vector(31 downto 0) := x"00000004"; -- #W command register
  constant c_reg_sr      : std_logic_vector(31 downto 0) := x"00000004"; -- R# status register

  -- I2c master registers
  constant c_reg_ctr_core_enable_bit            : std_logic_vector(31 downto 0) := x"00000080";
  constant c_reg_ctr_interrupt_enable_bit       : std_logic_vector(31 downto 0) := x"00000040";
  constant c_reg_txr_next_byte_to_transmit_mask : std_logic_vector(31 downto 0) := x"000000fe";
  constant c_reg_txr_reading_from_slave_bit     : std_logic_vector(31 downto 0) := x"00000001";
  constant c_reg_txr_writing_fo_slave_bit       : std_logic_vector(31 downto 0) := x"00000000";
  constant c_reg_rxr_last_byte_received_mask    : std_logic_vector(31 downto 0) := x"000000ff";

  -- Other constants
  constant c_reg_all_zero                : std_logic_vector(31 downto 0) := x"00000000";
  constant c_cyc_on                      : std_logic := '1';
  constant c_cyc_off                     : std_logic := '0';
  constant c_str_on                      : std_logic := '1';
  constant c_str_off                     : std_logic := '0';
  constant c_we_on                       : std_logic := '1';
  constant c_we_off                      : std_logic := '0';

  -- Basic device signals
  signal s_clk   : std_logic := '0';
  signal s_rst_n : std_logic := '0';
  signal s_rst   : std_logic := '0';

  -- Wishbone connections
  signal s_wb_slave_in  : t_wishbone_slave_in;
  signal s_wb_slave_out : t_wishbone_slave_out;
  signal s_wb_desc_out  : t_wishbone_device_descriptor;

  -- Interrupt
  signal s_int : std_logic; -- Don't care

  -- I2C pads
  signal s_scl_pad_in     : std_logic_vector(c_interfaces-1 downto 0);
  signal s_scl_pad_out    : std_logic_vector(c_interfaces-1 downto 0);
  signal s_scl_padoen_out : std_logic_vector(c_interfaces-1 downto 0);
  signal s_sda_pad_in     : std_logic_vector(c_interfaces-1 downto 0);
  signal s_sda_pad_out    : std_logic_vector(c_interfaces-1 downto 0);
  signal s_sda_padoen_out : std_logic_vector(c_interfaces-1 downto 0);

  -- Testbench logic
  signal s_sequence_cnt   : std_logic_vector(15 downto 0);

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
    if (dat_from_slave = compare_value) then
      report "Test passed: " & msg;
    else
      report "Test errored: " & msg;
      report "-> Info:  Answer from slave:          " & integer'image(to_integer(unsigned(dat_from_slave)));
      report "-> Error: Expected answer from slave: " & integer'image(to_integer(unsigned(compare_value)));
    end if;
  end procedure wb_expect;

begin

  -- Interfaces
  interfaces : for i in 0 to (c_interfaces-1) generate
    scl_pad_io(i)   <= s_scl_pad_out(i) when (s_scl_padoen_out(i) = '0') else 'Z';
    sda_pad_io(i)   <= s_sda_pad_out(i) when (s_sda_padoen_out(i) = '0') else 'Z';
    s_scl_pad_in(i) <= scl_pad_io(i);
    s_sda_pad_in(i) <= sda_pad_io(i);
  end generate;

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

  -- I2C master
  u_i2c_dut : xwb_i2c_master
    generic map (
      g_interface_mode      => CLASSIC,
      g_address_granularity => WORD,
      g_num_interfaces      => c_interfaces)
    port map (
      clk_sys_i    => s_clk,
      rst_n_i      => s_rst_n,
      slave_i      => s_wb_slave_in,
      slave_o      => s_wb_slave_out,
      desc_o       => s_wb_desc_out,
      int_o        => s_int,
      scl_pad_i    => s_scl_pad_in,
      scl_pad_o    => s_scl_pad_out,
      scl_padoen_o => s_scl_padoen_out,
      sda_pad_i    => s_sda_pad_in,
      sda_pad_o    => s_sda_pad_out,
      sda_padoen_o => s_sda_padoen_out);

  -- I2C slave
  p_slave : process
  begin
    wait for c_reset_time;
    s_scl_pad_in <= (others => '0');
    s_sda_pad_in <= (others => '0');
  end process;

  -- Testbench controller
  p_tb_ctl : process(s_clk, s_rst_n) is
  begin
    if s_rst_n = '0' then
      s_sequence_cnt <= (others => '0');
    elsif rising_edge(s_clk) then
      s_sequence_cnt <= std_logic_vector(unsigned(s_sequence_cnt) + 1);
    end if;
  end process;

  -- Wishbone controller
  p_wishbone_stim : process(s_clk, s_rst_n) is
  begin
    if s_rst_n = '0' then
      s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_all_zero, c_reg_all_zero);
    elsif rising_edge(s_clk) then
      case s_sequence_cnt is
        -- Check if core is disabled (read configuration)
        when x"0001" => s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_off, c_reg_ctr,      c_reg_all_zero);
        when x"0002" => s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_reg_ctr,      c_reg_all_zero);
        when x"0003" => s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_ctr,      c_reg_all_zero);
                        wb_expect("Core disabled?", s_wb_slave_out.dat, c_reg_all_zero);
        -- Enable core
        when x"0011" => s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_on,  c_reg_ctr,      c_reg_ctr_core_enable_bit);
        when x"0012" => s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_on,  c_reg_ctr,      c_reg_ctr_core_enable_bit);
        when x"0013" => s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_ctr,      c_reg_all_zero);
        -- Read back configuration
        when x"0021" => s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_on,  c_we_off, c_reg_ctr,      c_reg_all_zero);
        when x"0022" => s_wb_slave_in <= wb_stim(c_cyc_on,  c_str_off, c_we_off, c_reg_ctr,      c_reg_all_zero);
        when x"0023" => s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_ctr,      c_reg_all_zero);
                        wb_expect("Core enabled?", s_wb_slave_out.dat, c_reg_ctr_core_enable_bit);
        -- Default
        when others  => s_wb_slave_in <= wb_stim(c_cyc_off, c_str_off, c_we_off, c_reg_all_zero, c_reg_all_zero);
      end case;
    end if;
  end process;

end;
