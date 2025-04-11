--! @file        wb_read_access_tester.xml
--  DesignUnit   wb_read_access_tester
--! @author      M. Kreider <>
--! @date        11/02/2025
--! @version     0.0.1
--! @copyright   2025 GSI Helmholtz Centre for Heavy Ion Research GmbH
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
use work.wbgenplus_pkg.all;
use work.genram_pkg.all;
use work.wb_read_access_tester_auto_pkg.all;

entity wb_read_access_tester is
Port(
  clk_sys_i   : std_logic;                            -- Clock input for sys domain
  rst_sys_n_i : std_logic;                            -- Reset input (active low) for sys domain
  
  data_i      : in  t_wishbone_slave_in;
  data_o      : out t_wishbone_slave_out

);
end wb_read_access_tester;

architecture rtl of wb_read_access_tester is

  signal s_data_error_i   : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Error control
  signal s_data_stall_i   : std_logic_vector(1-1 downto 0)  := (others => '0'); -- flow control
  signal s_data_fuse_V_i  : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Valid flag - fuse
  signal s_data_fuse_i    : std_logic_vector(32-1 downto 0) := (others => '0'); -- Fuse to indicate if the register below has been read.
  
  signal r_data_fuse_V_i  : std_logic_vector(1-1 downto 0)  := (others => '1'); -- Valid flag - fuse
  signal r_data_fuse_i    : std_logic_vector(32-1 downto 0) := (others => '0'); -- Fuse to indicate if the register below has been read.

begin

  INST_wb_read_access_tester_auto : wb_read_access_tester_auto
  port map (
    clk_sys_i   => clk_sys_i,
    rst_sys_n_i => rst_sys_n_i,
    error_i     => s_data_error_i,
    stall_i     => s_data_stall_i,
    fuse_V_i    => s_data_fuse_V_i,
    fuse_i      => s_data_fuse_i,
    data_i      => data_i,
    data_o      => data_o  );

  data : process(clk_sys_i, rst_sys_n_i) begin
    if (rst_sys_n_i = '0') then
      s_data_fuse_V_i <= (others => '1');
      s_data_fuse_i <= (others => '0');
    else
      if rising_edge(clk_sys_i) then
        if(to_integer(unsigned(data_i.adr(2 downto 0))) = c_read_test_reg_GET and data_i.we /= '1') then
          s_data_fuse_V_i <= "1";
          s_data_fuse_i <= X"DEADBEEF";
        end if; -- s_e
  
      end if; -- clk edge
    end if;
  end process;

  -- process(clk_sys_i) begin
  --   if (rst_sys_n_i = '0') then
  --     r_data_fuse_V_i <= (others => '0');
  --     r_data_fuse_i <= (others => '0');
  --   else
  --     if rising_edge(clk_sys_i) then
  --       r_data_fuse_V_i <= s_data_fuse_V_i;
  --       r_data_fuse_i <= s_data_fuse_i;
  --     end if;
  --   end if;
  -- end process;
end rtl;
