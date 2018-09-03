------------------------------------------------------------------------------
-- Title      : Current time Clock crossing
-- Project    : TLU
------------------------------------------------------------------------------
-- File       : time_clk_cross.vhd
-- Author     : Mathias Kreider
-- Company    : GSI
-- Created    : 2013-08-10
-- Last update: 2013-08-10
-- Platform   : FPGA-generic
-- Standard   : VHDL'93
-------------------------------------------------------------------------------
-- Description: Converts whiterabbit TAI to 64b cycle count and syncs it to another clock domain
-------------------------------------------------------------------------------
-- Copyright (c) 2013 GSI
-------------------------------------------------------------------------------
-- Revisions  :
-- Date        Version  Author          Description
-- 2013-08-10  1.0      mkreider        Created
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.genram_pkg.all;
use work.eca_internals_pkg.all;
use work.gencores_pkg.all;


entity time_clk_cross is
generic (g_delay_comp      : natural := 16);
port    (clk_ref_i            : in std_logic;
         rst_ref_n_i          : in std_logic;
         clk_sys_i            : in std_logic;
         rst_sys_n_i          : in std_logic;

         tm_time_valid_i      : in  std_logic;                       -- timestamp valid flag
         tm_tai_i             : in  std_logic_vector(39 downto 0);   -- TAI Timestamp
         tm_cycles_i          : in  std_logic_vector(27 downto 0);   -- refclock cycle count

         tm_ref_tai_cycles_o  : out std_logic_vector(63 downto 0);
         tm_sys_tai_cycles_o  : out std_logic_vector(63 downto 0)
  );
end entity;


architecture behavioral of time_clk_cross is


signal   s_time_ref,
         s_time_ref_cor,
         s_time_ref_gray,
         s_time_sys_gray,
         s_time_sys_bin    : std_logic_vector(63 downto 0);

--debug
signal delta : integer;


begin

   --debug
   delta <= to_integer(signed(s_time_ref(31 downto 0))) - to_integer(signed(s_time_sys_bin(31 downto 0)));

   T1 : eca_wr_time
   port map(
      clk_i    => clk_ref_i,
      rst_n_i  => rst_ref_n_i,
      tai_i    => tm_tai_i,
      cycles_i => tm_cycles_i,
      time_o   => s_time_ref);

  tm_ref_tai_cycles_o <= s_time_ref;


   comp_sys_delay : eca_offset
    generic map(
      g_data_bits => c_time_bits,
      g_parts     => 4,
      g_offset    => g_delay_comp)
    port map(
      clk_i => clk_ref_i,
      a_i   => s_time_ref,
      c1_o  => open,
      x2_o  => s_time_ref_cor,
      c2_o  => open);



   gray_en : process(clk_ref_i)
   begin
      if rising_edge(clk_ref_i) then
        s_time_ref_gray <= f_gray_encode(s_time_ref_cor);
      end if;
   end process gray_en;

   G1: for I in 0 to s_time_ref'length-1 generate
      sync_trig_edge_reg : gc_sync_ffs
      generic map (
        g_sync_edge => "positive")
      port map (
        clk_i    => clk_sys_i,
        rst_n_i  => rst_sys_n_i,
        data_i   => s_time_ref_gray(I),
        synced_o => s_time_sys_gray(i),
        npulse_o => open,
        ppulse_o => open);
   end generate;

   gray_de : process(clk_sys_i)
   begin
      if rising_edge(clk_sys_i) then
        s_time_sys_bin <= f_gray_decode(s_time_sys_gray, 1);
        end if;
   end process gray_de;

  tm_sys_tai_cycles_o <= s_time_sys_bin;


end architecture behavioral;
