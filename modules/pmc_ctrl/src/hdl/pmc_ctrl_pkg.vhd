--! @file        pmc_ctrl_pkg.vhd
--  DesignUnit   pmc_ctrl
--! @author      A. Hahn <a.hahn@gsi.de>
--! @date        03/03/2015
--! @version     0.0.1
--! @copyright   2015 GSI Helmholtz Centre for Heavy Ion Research GmbH
--!

--TODO: This is a stub, finish/update it yourself
--! @brief Package for pmc_ctrl.vhd
--! If you modify the outer entity, don't forget to update this component! 
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
package pmc_ctrl_pkg is

      --| Component ------------------------- pmc_ctrl --------------------------------------------|
   component pmc_ctrl is
   Port(
      clk_sys_i          : in  std_logic;
      rst_n_i            : in  std_logic;
    
      slave_i            : in  t_wishbone_slave_in;
      slave_o            : out t_wishbone_slave_out;
    
      hex_switch_i       : in  std_logic_vector(3 downto 0);
      push_button_i      : in  std_logic_vector(0 downto 0);
      hex_switch_cpld_i  : in  std_logic_vector(3 downto 0);
      push_button_cpld_i : in  std_logic_vector(0 downto 0);
      clock_control_oe_o : out std_logic;
      logic_control_oe_o : out std_logic_vector(16 downto 0);
      logic_output_o     : out std_logic_vector(16 downto 0);
      logic_input_i      : in  std_logic_vector(16 downto 0)
   );
   end component;

   constant c_pmc_ctrl_slave_sdb : t_sdb_device := work.pmc_ctrl_auto_pkg.c_pmc_ctrl_slave_sdb;
   
end pmc_ctrl_pkg;
package body pmc_ctrl_pkg is
end pmc_ctrl_pkg;
