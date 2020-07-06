--! @file eca_sdp.vhd
--! @brief ECA Simple dual-ported memory
--! @author Wesley W. Terpstra <w.terpstra@gsi.de>
--!
--! Copyright (C) 2013 GSI Helmholtz Centre for Heavy Ion Research GmbH 
--!
--! Both Altera and Xilinx can provide wider data words when there
--! is only one reader and one writer. This component provides a
--! memory interface that can be implemented on an FPGA efficiently.
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
---------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.eca_internals_pkg.all;

-- Registers its inputs. Async outputs. 
-- When r_clk_i=w_clk_i, r_data_o is undefined.
entity eca_sdp is
  generic(
    g_addr_bits  : natural;
    g_data_bits  : natural;
    g_bypass     : boolean;
    g_dual_clock : boolean);
  port(
    r_clk_i  : in  std_logic;
    r_addr_i : in  std_logic_vector(g_addr_bits-1 downto 0);
    r_data_o : out std_logic_vector(g_data_bits-1 downto 0);
    w_clk_i  : in  std_logic;
    w_en_i   : in  std_logic;
    w_addr_i : in  std_logic_vector(g_addr_bits-1 downto 0);
    w_data_i : in  std_logic_vector(g_data_bits-1 downto 0));
end eca_sdp;

architecture rtl of eca_sdp is
  -- Quartus 11+ goes crazy and infers 7 M9Ks in an altshift_taps! Stop it.
  attribute altera_attribute : string; 
  attribute altera_attribute of rtl : architecture is "-name AUTO_SHIFT_REGISTER_RECOGNITION OFF";

  constant c_depth : natural := 2**g_addr_bits;
  
  type t_memory is array(c_depth-1 downto 0) of std_logic_vector(g_data_bits-1 downto 0);
  signal r_memory : t_memory := (others => (others => '-'));
  signal r_data   : std_logic_vector(g_data_bits-1 downto 0);
  signal r_bypass : std_logic;
  
  -- Use an MLAB if less than or equal to 64 entries
  function f_style return string is
  begin
    if g_addr_bits <= 6 then return "MLAB,no_rw_check"; else return "no_rw_check"; end if;
  end f_style;
  attribute ramstyle : string;
  attribute ramstyle of r_memory : signal is f_style;

begin

  bug :
    assert not g_dual_clock or not g_bypass
    report "eca_sdp cannot support read-write-bypass when using two distinct clocks"
    severity failure;

  write : process(w_clk_i) is
  begin
    if rising_edge(w_clk_i) then
      --assert (w_en_i xnor w_en_i) = '1' report "Write enable has bad value" severity failure;
      if w_en_i = '1' then
        --assert f_eca_safe(w_addr_i) = '1' report "Attempt to write to a meta-values address" severity failure;
        r_memory(to_integer(unsigned(w_addr_i))) <= w_data_i;
      end if;
    end if;
  end process;
  
  read : process(r_clk_i) is
  begin
    if rising_edge(r_clk_i) then
      r_bypass <= w_en_i and f_eca_eq(r_addr_i, w_addr_i);
      if f_eca_safe(r_addr_i) = '1' then
        r_data <= r_memory(to_integer(unsigned(r_addr_i)));
      else
        r_data <= (others => 'X');
      end if;
    end if;
  end process;
  
  -- These blocks are only needed to avoid stupid 'unused' warnings
  bypass : if g_bypass generate
    bl : block is
      signal w_data : std_logic_vector(g_data_bits-1 downto 0);
    begin
      regs : process(w_clk_i) is
      begin
        if rising_edge(w_clk_i) then
          w_data   <= w_data_i;
        end if;
      end process;
      r_data_o <= f_eca_mux(r_bypass, w_data, r_data);
    end block;
  end generate;
  
  nobypass : if not g_bypass generate
    bl : block is
      constant c_undef : std_logic_vector(g_data_bits-1 downto 0) := (others => 'X');
    begin
      r_data_o <= f_eca_mux(r_bypass, c_undef, r_data);
    end block;
  end generate;

end rtl;
