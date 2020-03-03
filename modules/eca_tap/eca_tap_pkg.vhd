library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.eca_tap_auto_pkg.c_eca_tap_ctrl_sdb;

package eca_tap_pkg is

  component eca_tap is generic
  (
    g_build_tap : boolean := true

  );
  port (
    clk_sys_i       : in  std_logic;  -- system clock 
    rst_sys_n_i     : in  std_logic;  -- reset, active low
    clk_ref_i       : in  std_logic;  -- system clock 
    rst_ref_n_i     : in  std_logic;  -- reset, active low 
    time_ref_i      : in  std_logic_vector(63 downto 0) := (others => '0');
      
    ctrl_o        : out t_wishbone_slave_out;
    ctrl_i        : in  t_wishbone_slave_in;

    tap_out_o     : out t_wishbone_master_out; 
    tap_out_i     : in  t_wishbone_master_in := ('0', '0', '0', '0', x"00000000");
    tap_in_o      : out t_wishbone_slave_out;
    tap_in_i      : in  t_wishbone_slave_in

  );
  end component;

  constant c_eca_tap_sdb : t_sdb_device := c_eca_tap_ctrl_sdb;
  
end package;
