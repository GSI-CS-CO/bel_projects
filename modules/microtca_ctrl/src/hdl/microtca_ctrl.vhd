--! @file        microtca_ctrl.vhd
--  DesignUnit   microtca_ctrl
--! @author      A. Hahn <a.hahn@gsi.de>
--! @date        20/11/2015
--! @version     0.0.1
--! @copyright   2015 GSI Helmholtz Centre for Heavy Ion Research GmbH
--!

--TODO: This is a stub, finish/update it yourself
--! @brief *** ADD BRIEF DESCRIPTION HERE ***
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
use work.microtca_ctrl_auto_pkg.all;

entity microtca_ctrl is
Port(
   clk_sys_i   : in  std_logic;
   rst_n_i     : in  std_logic;

   slave_i     : in  t_wishbone_slave_in;
   slave_o     : out t_wishbone_slave_out;
   
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
end microtca_ctrl;

architecture rtl of microtca_ctrl is

   signal s_slave_regs_clk_sys_o : t_slave_regs_clk_sys_o;
   signal s_slave_regs_clk_sys_i : t_slave_regs_clk_sys_i;
   signal s_slave_i              : t_wishbone_slave_in;
   signal s_slave_o              : t_wishbone_slave_out;


begin

   INST_microtca_ctrl_auto : microtca_ctrl_auto
   port map (
      clk_sys_i            => clk_sys_i,
      rst_n_i              => rst_n_i,

      slave_regs_clk_sys_o => s_slave_regs_clk_sys_o,
      slave_regs_clk_sys_i => s_slave_regs_clk_sys_i,
      slave_i              => s_slave_i,
      slave_o              => s_slave_o
   );
   
  -- wishbone
  slave_o   <= s_slave_o;
  s_slave_i <= slave_i;
  
  -- misc. signals
  s_slave_regs_clk_sys_i.STALL <= '0';
  s_slave_regs_clk_sys_i.ERR   <= '0';
  
  -- hex switches
  s_slave_regs_clk_sys_i.HEX_SWITCH      <= hex_switch_i;
  s_slave_regs_clk_sys_i.HEX_SWITCH_CPLD <= hex_switch_cpld_i;
  
  -- push buttons
  s_slave_regs_clk_sys_i.PUSH_BUTTON      <= push_button_i;
  s_slave_regs_clk_sys_i.PUSH_BUTTON_CPLD <= push_button_cpld_i;
  
  -- input clock output enable
  clock_control_oe_o <= s_slave_regs_clk_sys_o.CLOCK_CONTROL_OE(0);
  
  -- logic analyzer output enable
  logic_control_oe_o <= s_slave_regs_clk_sys_o.LOGIC_CONTROL_OE(16 downto 0);
  
  -- logic analyzer output
  logic_output_o <= s_slave_regs_clk_sys_o.LOGIC_OUTPUT(16 downto 0);
  
  -- logic analyzer input
  s_slave_regs_clk_sys_i.LOGIC_INPUT <= logic_input_i(16 downto 0);

  -- black plane development
  backplane_conf0_o  <= x"0000" & s_slave_regs_clk_sys_o.BACKPLANE_CONF0;
  backplane_conf1_o  <= x"0000" & s_slave_regs_clk_sys_o.BACKPLANE_CONF1;
  backplane_conf2_o  <= x"0000" & s_slave_regs_clk_sys_o.BACKPLANE_CONF2;
  backplane_conf3_o  <= x"0000" & s_slave_regs_clk_sys_o.BACKPLANE_CONF3;
  backplane_conf4_o  <= x"0000" & s_slave_regs_clk_sys_o.BACKPLANE_CONF4;
  backplane_conf5_o  <= x"0000" & s_slave_regs_clk_sys_o.BACKPLANE_CONF5;
  backplane_conf6_o  <= x"0000" & s_slave_regs_clk_sys_o.BACKPLANE_CONF6;
  backplane_conf7_o  <= x"0000" & s_slave_regs_clk_sys_o.BACKPLANE_CONF7;

  s_slave_regs_clk_sys_i.BACKPLANE_STAT0 <= backplane_stat0_i(15 downto 0);
  s_slave_regs_clk_sys_i.BACKPLANE_STAT1 <= backplane_stat1_i(15 downto 0);
  s_slave_regs_clk_sys_i.BACKPLANE_STAT2 <= backplane_stat2_i(15 downto 0);
  s_slave_regs_clk_sys_i.BACKPLANE_STAT3 <= backplane_stat3_i(15 downto 0);
  s_slave_regs_clk_sys_i.BACKPLANE_STAT4 <= backplane_stat4_i(15 downto 0);
  s_slave_regs_clk_sys_i.BACKPLANE_STAT5 <= backplane_stat5_i(15 downto 0);
  s_slave_regs_clk_sys_i.BACKPLANE_STAT6 <= backplane_stat6_i(15 downto 0);
  s_slave_regs_clk_sys_i.BACKPLANE_STAT7 <= backplane_stat7_i(15 downto 0);

end rtl;
