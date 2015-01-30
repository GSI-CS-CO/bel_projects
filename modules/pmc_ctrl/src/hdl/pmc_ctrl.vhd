--! @file        pmc_ctrl.vhd
--  DesignUnit   pmc_ctrl
--! @author      A. Hahn <a.hahn@gsi.de>
--! @date        30/01/2015
--! @version     0.0.1
--! @copyright   2015 GSI Helmholtz Centre for Heavy Ion Research GmbH
--!

--! @brief       Used to control the clock enable signal (external clock) and
--!              to get the hex switch state.
--!
--------------------------------------------------------------------------------
--! This library is free software; you can redistribute it and/or
--! modify it under the terms of the GNU Lesser General Public
--! License as published by the Free Software Foundation; either
--! version 3 of the License, or (at your option) any later version.
--!
--! This library is distributed in the hope that it will be useful,
--! but WITHOUT ANY WARRANTY; without even the implied warranty of
--! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
--! Lesser General Public License for more details.
--!
--! You should have received a copy of the GNU Lesser General Public
--! License along with this library. If not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;
use work.pmc_ctrl_pkg.all;
use work.pmc_ctrl_auto_pkg.all;

entity pmc_ctrl is
Port(
  clk_sys_i      : in  std_logic;
  rst_n_i        : in  std_logic;
  slave_i        : in  t_wishbone_slave_in  := ('0', '0', x"00000000", x"F", '0', x"00000000");
  slave_o        : out t_wishbone_slave_out;
  clock_enable_o : out std_logic;
  hex_switch_i   : in  std_logic_vector(3 downto 0)
);
end pmc_ctrl;

architecture rtl of pmc_ctrl is
  
  signal s_slave_i        : t_wishbone_slave_in   := ('0', '0', x"00000000", x"F", '0', x"00000000");
  signal s_slave_o        : t_wishbone_slave_out;
  signal s_slave_regs_o   : t_slave_regs_o;
  signal s_slave_regs_i   : t_slave_regs_i;
  
begin

  INST_pmc_ctrl : pmc_ctrl_auto
  port map (
    clk_sys_i      => clk_sys_i,
    rst_n_i        => rst_n_i,
    slave_regs_o   => s_slave_regs_o,
    slave_regs_i   => s_slave_regs_i,
    slave_i        => s_slave_i,
    slave_o        => s_slave_o
  );
  
  -- wishbone
  slave_o   <= s_slave_o;
  s_slave_i <= slave_i;
  
  -- hex switch
  s_slave_regs_i.HEX_SWITCH <= hex_switch_i;
  
  -- clock enable
  clock_enable_o <= not(s_slave_regs_o.CLOCK_CONTROL(0));
  
end rtl;
