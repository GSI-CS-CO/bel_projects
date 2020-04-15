------------------------------------------------------------------------------
-- Title      : Etherbone FIFO
-- Project    : Etherbone Core
------------------------------------------------------------------------------
-- File       : eb_fifo.vhd
-- Author     : Wesley W. Terpstra
-- Company    : GSI
-- Created    : 2013-04-08
-- Last update: 2013-04-08
-- Platform   : FPGA-generic
-- Standard   : VHDL'93
-------------------------------------------------------------------------------
-- Description: A convenience wrapper for FIFOs used in Etherbone
-------------------------------------------------------------------------------
-- Copyright (c) 2013 GSI
-------------------------------------------------------------------------------
-- Revisions  :
-- Date        Version  Author          Description
-- 2013-04-08  1.0      terpstra        Created
-- 2019-06-26           reese           Use array instead of explicit 
--                                        resource instantiation
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.wishbone_pkg.f_ceil_log2;

-- r_dat_o is valid when r_empty_o=0 (show ahead)
-- w_dat_i is valid when w_push_i =1
-- r_pop_i  affects r_empty_o on the next cycle
-- w_push_i affects w_full_o  on the next cycle
entity eb_fifo is
  generic(
    g_width : natural;
    g_size  : natural);
  port(
    clk_i     : in  std_logic;
    rstn_i    : in  std_logic;
    w_full_o  : out std_logic;
    w_push_i  : in  std_logic;
    w_dat_i   : in  std_logic_vector(g_width-1 downto 0);
    r_empty_o : out std_logic;
    r_pop_i   : in  std_logic;
    r_dat_o   : out std_logic_vector(g_width-1 downto 0));
end eb_fifo;

architecture rtl of eb_fifo is

  constant c_depth : natural := f_ceil_log2(g_size);
  
  signal r_idx  : unsigned(c_depth downto 0);
  signal w_idx  : unsigned(c_depth downto 0);
  signal r_idx1 : unsigned(c_depth downto 0);
  signal w_idx1 : unsigned(c_depth downto 0);
  
  constant c_low  : unsigned(c_depth-1 downto 0) := (others => '0');
  constant c_high : unsigned(c_depth   downto 0) := '1' & c_low;
  
  signal empty : std_logic := '1';

  type data_array_t is array ( 0 to g_size-1) 
            of std_logic_vector ( g_width-1 downto 0);
  signal data : data_array_t := (others => (others => 'X'));

begin

  r_empty_o <= empty;
  r_dat_o <= data(to_integer(r_idx(c_depth-1 downto 0))); 

  r_idx1 <= (r_idx+1) when r_pop_i ='1' else r_idx;
  w_idx1 <= (w_idx+1) when w_push_i='1' else w_idx;
  
  main : process(rstn_i, clk_i) is
  begin
    if rstn_i = '0' then
      r_idx     <= (others => '0');
      w_idx     <= (others => '0');
      w_full_o  <= '0';
      empty <= '1';
    elsif rising_edge(clk_i) then
      r_idx <= r_idx1;
      w_idx <= w_idx1;
      
      if w_push_i = '1' then
        data(to_integer(w_idx(c_depth-1 downto 0))) <= w_dat_i;
      end if;

      -- Compare the newest pointers
      if (w_idx1 xor c_high) = r_idx1 then
        w_full_o <= '1';
      else
        w_full_o <= '0';
      end if;
      
      -- Use the OLD write pointer to prevent read-during-write
      if w_idx = r_idx1 then
        empty <= '1';
      else
        empty <= '0';
      end if;
      
    end if;
  end process;

end rtl;
