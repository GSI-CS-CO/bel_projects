library IEEE;
use IEEE.STD_LOGIC_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;
use work.wishbone_pkg.all;

library work;

package power_test_pkg is

  constant  c_power_test_addr_range:  integer := 16#1# * 4;  -- all resources (byte, word, double word) are alligned to modulo 4 addresses,
                                                                -- so multiply thec_power_test_byte_addr_range by 4.
  constant  c_power_test_addr_width:  integer := integer(ceil(log2(real(c_power_test_addr_range))));

  
  constant c_xwb_power_test: t_sdb_device := (
  abi_class     => x"0000",             -- undocumented device
  abi_ver_major => x"01",
  abi_ver_minor => x"00",
  wbd_endian    => c_sdb_endian_big,    -- '1' = little, '0' = big
  wbd_width     => x"4",                -- only 32-bit port granularity allowed
  sdb_component => (
  addr_first    => x"0000000000000000",
  addr_last     => std_logic_vector(to_unsigned(c_power_test_addr_range-1, t_sdb_component.addr_last'length)),
  product => (
  vendor_id     => x"0000000000000651", -- GSI
  device_id     => x"55aa6b96",
  version       => x"00000001",
  date          => x"20130826",
  name          => "POWER_TEST         ")));  -- should be 19 Char
  
  component power_test is
    generic(
      Clk_in_Hz:  integer := 125_000_000;
      pwm_width:  integer := 16;
      row_width:  integer := 64;
      row_cnt:    integer := 64
      );
    port(
      clk_i:          in    std_logic;
      nRst_i:         in    std_logic;
      slave_i:        in    t_wishbone_slave_in;
      slave_o:        out   t_wishbone_slave_out;
      pwm_o:          out   std_logic;
      or_o:           out   std_logic    
      );
    end component power_test;
  
end package POWer_test_pkg;

