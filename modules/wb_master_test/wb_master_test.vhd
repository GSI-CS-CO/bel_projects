--! @file        wb_master_test.xml
--  DesignUnit   wb_master_test
--! @author      M. Kreider <>
--! @date        11/04/2025
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
use work.wb_master_test_auto_pkg.all;

entity wb_master_test is
Port(
  clk_sys_i         : std_logic;                            -- Clock input for sys domain
  rst_sys_n_i       : std_logic;                            -- Reset input (active low) for sys domain
  -- error_i           : in  std_logic_vector(1-1 downto 0);   -- Error control
  -- fuse_read_i       : in  std_logic_vector(32-1 downto 0);  -- Fuse to indicate if the register below has been read.
  -- fuse_read_V_i     : in  std_logic_vector(1-1 downto 0);   -- Valid flag - fuse_read
  -- fuse_write_i      : in  std_logic_vector(32-1 downto 0);  -- Fuse to indicate if the register below has been written.
  -- fuse_write_V_i    : in  std_logic_vector(1-1 downto 0);   -- Valid flag - fuse_write
  -- stall_i           : in  std_logic_vector(1-1 downto 0);   -- flow control
  -- write_test_reg_o  : out std_logic_vector(32-1 downto 0);  -- Write only register that burns a fuse if it was accessed.
  
  data_i            : in  t_wishbone_slave_in;
  data_o            : out t_wishbone_slave_out

  
);
end wb_master_test;

architecture rtl of wb_master_test is

  signal s_data_error_i           : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Error control
  signal s_data_stall_i           : std_logic_vector(1-1 downto 0)  := (others => '0'); -- flow control
  signal s_data_fuse_read_V_i     : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Valid flag - fuse_read
  signal s_data_fuse_read_i       : std_logic_vector(32-1 downto 0) := (others => '0'); -- Fuse to indicate if the register below has been read.
  signal s_data_fuse_write_V_i    : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Valid flag - fuse_write
  signal s_data_fuse_write_i      : std_logic_vector(32-1 downto 0) := (others => '0'); -- Fuse to indicate if the register below has been written.
  signal s_data_write_test_reg_o  : std_logic_vector(32-1 downto 0) := (others => '0'); -- Write only register that burns a fuse if it was accessed.
  


begin

  INST_wb_master_test_auto : wb_master_test_auto
  port map (
    clk_sys_i         => clk_sys_i,
    rst_sys_n_i       => rst_sys_n_i,
    error_i           => s_data_error_i,
    stall_i           => s_data_stall_i,
    fuse_read_V_i     => s_data_fuse_read_V_i,
    fuse_read_i       => s_data_fuse_read_i,
    fuse_write_V_i    => s_data_fuse_write_V_i,
    fuse_write_i      => s_data_fuse_write_i,
    write_test_reg_o  => s_data_write_test_reg_o,
    data_i            => data_i,
    data_o            => data_o  );

  test : process(clk_sys_i, rst_sys_n_i) begin
    if (rst_sys_n_i = '0') then
      s_data_fuse_read_V_i <= (others => '1');
      s_data_fuse_read_i <= (others => '0');
      s_data_fuse_write_V_i <= (others => '1');
      s_data_fuse_write_i <= (others => '0');      
    else
      if rising_edge(clk_sys_i) then
        if(to_integer(unsigned(data_i.adr(3 downto 0))) = c_read_test_reg_GET and data_i.we = '0') then
          s_data_fuse_read_V_i <= "1";
          s_data_fuse_read_i <= X"DEADBEEF";
        end if;
  
        if(to_integer(unsigned(data_i.adr(3 downto 0))) = c_write_test_reg_RW and data_i.we = '1') then
          s_data_fuse_write_V_i <= "1";
          s_data_fuse_write_i <= X"DEADBEEF";
        end if;
      end if; -- clk edge
    end if;
  end process;
end rtl;
