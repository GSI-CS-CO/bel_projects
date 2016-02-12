--! @file        wb_rem_update.vhd
--  DesignUnit   wb_rem_update
--! @author      S. Rauch <>
--! @date        12/08/2015
--! @version     0.0.1
--! @copyright   2015 GSI Helmholtz Centre for Heavy Ion Research GmbH
--!

--! @brief wishbone wrapper for the Altera Remote Update Core
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
use work.wb_rem_update_auto_pkg.all;
use work.remote_update_pkg.all;

entity wb_rem_update is
Port(
   clk_sys_i   : in  std_logic;
   rst_n_i     : in  std_logic;

   ru_i        : in  t_wishbone_slave_in;
   ru_o        : out t_wishbone_slave_out
   
);
end wb_rem_update;

architecture rtl of wb_rem_update is

   signal s_ru_regs_clk_sys_o : t_ru_regs_clk_sys_o;
   signal s_ru_regs_clk_sys_i : t_ru_regs_clk_sys_i;
   signal s_ru_i              : t_wishbone_slave_in;
   signal s_ru_o              : t_wishbone_slave_out;
   signal s_asmi_busy         : std_logic;
   signal s_asmi_data_valid   : std_logic;
   signal s_asmi_dataout      : std_logic_vector(7 downto 0);
   signal s_asmi_addr         : std_logic_vector(23 downto 0);
   signal s_asmi_rden         : std_logic;
   signal s_asmi_read         : std_logic;
   signal s_read_strobe       : std_logic;
   signal s_write_strobe      : std_logic;
   signal s_data_in           : std_logic_vector(23 downto 0);
   signal s_busy              : std_logic;
   signal s_data_out          : std_logic_vector(23 downto 0);
    

begin
  
  asmi: altasmi
    port map (
      addr        => s_asmi_addr,
      clkin       => clk_sys_i,
      rden        => s_asmi_rden,
      read        => s_asmi_read,
      reset       => not rst_n_i,
      busy        => s_asmi_busy,
      data_valid  => s_asmi_data_valid,
      dataout     => s_asmi_dataout);

  aru:  remote_update
    port map (
      asmi_busy         => s_asmi_busy,
      asmi_data_valid   => s_asmi_data_valid,
      asmi_dataout      => s_asmi_dataout,
      clock             => clk_sys_i,
      data_in           => s_data_in,
      param             => s_ru_i.adr(4 downto 2),
      read_param        => s_read_strobe,
      reconfig          => '0',
      reset             => not rst_n_i,
      reset_timer       => '0',
      write_param       => s_write_strobe,
      asmi_addr         => s_asmi_addr,
      asmi_rden         => s_asmi_rden,
      asmi_read         => s_asmi_read,
      busy              => s_busy,
      data_out          => s_data_out,
      pof_error         => open);


    s_read_strobe <=  s_ru_regs_clk_sys_o.CONFIG_SRC_RE or
                        s_ru_regs_clk_sys_o.WATCH_VALUE_RE or
                        s_ru_regs_clk_sys_o.WATCH_ENA_RE or
                        s_ru_regs_clk_sys_o.PAGE_SEL_RE or
                        s_ru_regs_clk_sys_o.CONFIG_MODE_RE;
                        
    s_write_strobe <= s_ru_regs_clk_sys_o.WATCH_VALUE_WE or
                        s_ru_regs_clk_sys_o.WATCH_ENA_WE or
                        s_ru_regs_clk_sys_o.PAGE_SEL_WE or
                        s_ru_regs_clk_sys_o.CONFIG_MODE_WE;
      
   INST_wb_rem_update_auto : wb_rem_update_auto
   port map (
      clk_sys_i         => clk_sys_i,
      rst_n_i           => rst_n_i,

      ru_regs_clk_sys_o => s_ru_regs_clk_sys_o,
      ru_regs_clk_sys_i => s_ru_regs_clk_sys_i,
      ru_i              => s_ru_i,
      ru_o              => s_ru_o
   );
end rtl;
