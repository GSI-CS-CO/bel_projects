--! @file        microtca_ctrl_pkg.vhd
--  DesignUnit   microtca_ctrl
--! @author      A. Hahn <a.hahn@gsi.de>
--! @date        20/11/2015
--! @version     0.0.1
--! @copyright   2015 GSI Helmholtz Centre for Heavy Ion Research GmbH
--!

--TODO: This is a stub, finish/update it yourself
--! @brief Package for microtca_ctrl.vhd
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
package microtca_ctrl_pkg is

      --| Component ----------------------- microtca_ctrl -----------------------------------------|
   component microtca_ctrl is
   Port(
      clk_sys_i          : in  std_logic;
      rst_n_i            : in  std_logic;
      
      slave_i            : in  t_wishbone_slave_in;
      slave_o            : out t_wishbone_slave_out;
      
      backplane_conf0_o  : out std_logic_vector(31 downto 0);
      backplane_conf1_o  : out std_logic_vector(31 downto 0);
      backplane_conf2_o  : out std_logic_vector(31 downto 0);
      backplane_conf3_o  : out std_logic_vector(31 downto 0);
      backplane_conf4_o  : out std_logic_vector(31 downto 0);
      backplane_conf5_o  : out std_logic_vector(31 downto 0);
      backplane_conf6_o  : out std_logic_vector(31 downto 0);
      backplane_conf7_o  : out std_logic_vector(31 downto 0);

      backplane_stat0_i  : in  std_logic_vector(31 downto 0);
      backplane_stat1_i  : in  std_logic_vector(31 downto 0);
      backplane_stat2_i  : in  std_logic_vector(31 downto 0);
      backplane_stat3_i  : in  std_logic_vector(31 downto 0);
      backplane_stat4_i  : in  std_logic_vector(31 downto 0);
      backplane_stat5_i  : in  std_logic_vector(31 downto 0);
      backplane_stat6_i  : in  std_logic_vector(31 downto 0);
      backplane_stat7_i  : in  std_logic_vector(31 downto 0);
     
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

   constant c_microtca_ctrl_slave_sdb : t_sdb_device := work.microtca_ctrl_auto_pkg.c_microtca_ctrl_slave_sdb;
   
end microtca_ctrl_pkg;
package body microtca_ctrl_pkg is
end microtca_ctrl_pkg;
