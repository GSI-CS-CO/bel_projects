-- Libraries
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library work;
use work.wishbone_pkg.all;

-- Entity (empty)
entity i2c_testbench is
end;

-- Architecture
architecture rtl of i2c_testbench is

  -- Testbench settings
  constant c_interfaces  : integer := 5;
  constant c_reset_time  : time    := 100 ns;
  constant c_clock_cycle : time    := 16 ns;

  -- Basic device signals
  signal s_clk   : std_logic := '0';
  signal s_rst_n : std_logic := '1';

  -- Wishbone connections
  signal s_wb_slave_in  : t_wishbone_slave_in;
  signal s_wb_slave_out : t_wishbone_slave_out;
  signal s_wb_desc_out  : t_wishbone_device_descriptor;

  -- Interrupt
  signal s_int : std_logic;

  -- I2C pads
  signal s_scl_pad_in     : std_logic_vector(c_interfaces-1 downto 0);
  signal s_scl_pad_out    : std_logic_vector(c_interfaces-1 downto 0);
  signal s_scl_padoen_out : std_logic_vector(c_interfaces-1 downto 0);
  signal s_sda_pad_in     : std_logic_vector(c_interfaces-1 downto 0);
  signal s_sda_pad_out    : std_logic_vector(c_interfaces-1 downto 0);
  signal s_sda_padoen_out : std_logic_vector(c_interfaces-1 downto 0);

begin

  -- Clock generator
  p_clock : process
  begin
    s_clk <= '0';
    wait for c_clock_cycle/2;
    s_clk <= '1' and not(s_rst_n);
    wait for c_clock_cycle/2;
  end process;

  -- Reset controller
  p_reset : process
  begin
    wait for c_reset_time;
    s_rst_n <= '0';
  end process;

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

end;
