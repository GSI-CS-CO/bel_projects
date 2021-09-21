library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library work;
use work.wishbone_pkg.all;

entity i2c_testbench is
end;

architecture rtl of i2c_testbench is

  -- Testbench settings
  constant c_interfaces : integer := 5;

  -- Basic device signals
  signal s_clk   : std_logic := '0';
  signal s_rst_n : std_logic := '1';

  -- Wishbone connections
  signal s_wb_slave_in  : t_wishbone_slave_in;
  signal s_wb_slave_oot : t_wishbone_slave_out;
  signal s_wb_desc_o    : t_wishbone_device_descriptor;

  -- Interrupt
  signal s_int : std_logic;

  -- I2C pads
  signal s_scl_pad_in      : std_logic_vector(c_interfaces-1 downto 0);
  signal s_scl_pad_out     : std_logic_vector(c_interfaces-1 downto 0);
  signal s_scl_padoen_out  : std_logic_vector(c_interfaces-1 downto 0);
  signal s_sda_pad_in      : std_logic_vector(c_interfaces-1 downto 0);
  signal s_sda_pad_out     : std_logic_vector(c_interfaces-1 downto 0);
  signal s_sda_padoen_oout : std_logic_vector(c_interfaces-1 downto 0);

begin

  p_clock : process
  begin
    s_clk <= '0';
    wait for 8 ns;
    s_clk <= '1' and not(s_rst_n);
    wait for 8 ns;
  end process;

  p_reset : process
  begin
    wait for 80 ns;
    s_rst_n <= '0';
  end process;





end;
