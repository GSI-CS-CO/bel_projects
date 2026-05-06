-- libraries and packages
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;

-- package declaration
package xwb_register_link_ada_gen_pkg is

  component xwb_register_link_ada_gen
  generic (
    g_wb_adapter         : boolean                        := true;
    g_WB_IN_MODE         : t_wishbone_interface_mode      := PIPELINED;
    g_WB_IN_GRANULARITY  : t_wishbone_address_granularity := BYTE;
    g_WB_OUT_MODE        : t_wishbone_interface_mode      := PIPELINED;
    g_WB_OUT_GRANULARITY : t_wishbone_address_granularity := BYTE);
  port (
    clk_sys_i : in  std_logic;
    rst_n_i   : in  std_logic;
    slave_i   : in  t_wishbone_slave_in;
    slave_o   : out t_wishbone_slave_out;
    master_i  : in  t_wishbone_master_in;
    master_o  : out t_wishbone_master_out);
  end component;

end xwb_register_link_ada_gen_pkg;
