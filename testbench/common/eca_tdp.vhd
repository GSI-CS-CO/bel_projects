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
entity eca_tdp is
  generic(
    g_addr_bits  : natural;
    g_data_bits  : natural);
  port(
    clk_i    : in  std_logic;
    a_wen_i  : in  std_logic;
    a_addr_i : in  std_logic_vector(g_addr_bits-1 downto 0);
    a_data_i : in  std_logic_vector(g_data_bits-1 downto 0); 
    a_data_o : out std_logic_vector(g_data_bits-1 downto 0);
    b_wen_i  : in  std_logic;
    b_addr_i : in  std_logic_vector(g_addr_bits-1 downto 0);
    b_data_i : in  std_logic_vector(g_data_bits-1 downto 0); 
    b_data_o : out std_logic_vector(g_data_bits-1 downto 0));
end eca_tdp;

architecture rtl of eca_tdp is
  -- Quartus 11+ goes crazy and infers 7 M9Ks in an altshift_taps! Stop it.
  attribute altera_attribute : string; 
  attribute altera_attribute of rtl : architecture is "-name AUTO_SHIFT_REGISTER_RECOGNITION OFF";

  constant c_depth : natural := 2**g_addr_bits;
  
  type t_memory is array(c_depth-1 downto 0) of std_logic_vector(g_data_bits-1 downto 0);
  shared variable v_memory : t_memory := (others => (others => '-'));
  
begin

  a : process(clk_i) 
  begin
    if rising_edge(clk_i) then
      assert (a_wen_i xnor a_wen_i) = '1' report "Write enable has bad value on port A" severity failure;
      
      if a_wen_i = '1' then
        assert f_eca_safe(a_addr_i) = '1' report "Write to undefined address on port A" severity failure;
        v_memory(to_integer(unsigned(a_addr_i))) := a_data_i;
      end if;
      
      -- Invalid address => invalid data
      if f_eca_safe(a_addr_i) = '1' then
        a_data_o <= v_memory(to_integer(unsigned(a_addr_i)));
      else
        a_data_o <= (others => 'X');
      end if;
      -- Output undefined during write
      if a_wen_i = '1' or (b_wen_i = '1' and a_addr_i = b_addr_i) then
        a_data_o <= (others => 'X');
      end if;
    end if;
  end process;
  
  b : process(clk_i) 
  begin
    if rising_edge(clk_i) then
      --report "b_wen_i" & std_logic'image(b_wen_i);
      --assert (b_wen_i xnor b_wen_i) = '1' report "Write enable has bad value on port B" severity failure;
      
      if b_wen_i = '1' then
        assert f_eca_safe(b_addr_i) = '1' report "Write to undefined address on port B" severity failure;
        v_memory(to_integer(unsigned(b_addr_i))) := b_data_i;
      end if;
      
      -- Invalid address => invalid data
      if f_eca_safe(b_addr_i) = '1' then
        b_data_o <= v_memory(to_integer(unsigned(b_addr_i)));
      else
        b_data_o <= (others => 'X');
      end if;
      -- Output undefined during write
      if b_wen_i = '1' or (a_wen_i = '1' and a_addr_i = b_addr_i) then
        b_data_o <= (others => 'X');
      end if;
    end if;
  end process;
  
  fatal : process(clk_i) is
  begin
    if rising_edge(clk_i) then
      assert a_wen_i = '0' or b_wen_i = '0' or a_addr_i /= b_addr_i
      report "Two writes to the same address in eca_tdp"
      severity failure;
    end if;
  end process;

end rtl;
