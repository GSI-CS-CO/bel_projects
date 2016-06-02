library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wr_fabric_pkg.all;

package time_counter_pkg is 

  type t_counter_ctrl_reg is
    record
      s_counter_start : std_logic;
    end record;

  type t_counter_stat_reg is
    record
      s_counter_ts_start_tai  :std_logic_vector(39 downto 0);
      s_counter_ts_start_cyc  :std_logic_vector(27 downto 0);
      s_counter_ts_stop_tai   :std_logic_vector(39 downto 0);
      s_counter_ts_stop_cyc   :std_logic_vector(27 downto 0);
    end record;

  constant c_time_counter_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"4", -- 32-bit port granularity
    sdb_component => (
      addr_first    => x"0000000000000000",
      addr_last     => x"000000000000ffff",
      product => (
      vendor_id     => x"0000000000000651", -- GSI
      device_id     => x"53bee0e2",
      version       => x"00000001",
      date          => x"20160201",
      name          => "GSI:TIME_COUNTER   ")));
   
  constant c_counter_ctrl_default : t_counter_ctrl_reg := (
    s_counter_start => '0');


  constant c_counter_stat_default : t_counter_stat_reg := (
    s_counter_ts_start_tai => x"1111111111",
    s_counter_ts_start_cyc => x"1111111",
    s_counter_ts_stop_tai  => x"2222222222",
    s_counter_ts_stop_cyc  => x"2222222");

  component time_counter is
    port (
      clk_i       : in  std_logic;
      rst_n_i     : in  std_logic;
      ctrl_reg_i  : in  t_counter_ctrl_reg;
      stat_reg_o  : out t_counter_stat_reg;
      counter_tm_tai_i   : in std_logic_vector(39 downto 0); 
      counter_tm_cycle_i : in std_logic_vector(27 downto 0));
  end component;

  component xwb_time_counter is
    port(
      clk_i                 : in std_logic;
      rst_n_i               : in std_logic;
      tm_tai_i          : in std_logic_vector(39 downto 0);
      tm_cycle_i        : in std_logic_vector(27 downto 0);
      wb_ctrl_stat_slave_o  : out t_wishbone_slave_out;
      wb_ctrl_stat_slave_i  : in  t_wishbone_slave_in);
  end component;

  component wb_slave_time_counter is
    port (
      clk_i          : in  std_logic;
      rst_n_i        : in  std_logic;
      wb_slave_i     : in  t_wishbone_slave_in;
      wb_slave_o     : out t_wishbone_slave_out;
      stat_reg_i     : in  t_counter_stat_reg;
      ctrl_reg_o     : out t_counter_ctrl_reg);
  end component;

end package time_counter_pkg;

package body time_counter_pkg is

end time_counter_pkg;
