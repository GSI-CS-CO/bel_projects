------------------------------------------------------------------------------
-- Title      : Wishbone pSRAM / cellular RAM core
-- Project    : General Cores
------------------------------------------------------------------------------
-- File       : psram.vhd
-- Author     : Wesley W. Terpstra
-- Company    : GSI
-- Created    : 2014-12-05
-- Last update: 2014-12-05
-- Platform   : FPGA-generic
-- Standard   : VHDL'93
-------------------------------------------------------------------------------
-- Description: Maps an pSRAM chip to wishbone memory
-------------------------------------------------------------------------------
-- Copyright (c) 2013 GSI
-------------------------------------------------------------------------------
-- Revisions  :
-- Date        Version  Author          Description
-- 2014-12-05  1.0      terpstra        Created
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;

entity psram is
  generic(
    g_bits : natural := 24);
  port(
    clk_i     : in    std_logic;
    rstn_i    : in    std_logic;
    slave_i   : in    t_wishbone_slave_in;
    slave_o   : out   t_wishbone_slave_out;
    ps_clk    : out   std_logic;
    ps_addr   : out   std_logic_vector(g_bits-1 downto 0);
    ps_data   : inout std_logic_vector(15 downto 0);
    ps_seln   : out   std_logic_vector(1 downto 0);
    ps_cen    : out   std_logic;
    ps_oen    : out   std_logic;
    ps_wen    : out   std_logic;
    ps_cre    : out   std_logic;
    ps_advn   : out   std_logic;
    ps_wait   : in    std_logic);
end entity;

-- 104MHz or 70ns, 20ns in page
-- support burst mode like in SPI flash => fetch more
-- mode: continuous burst wrap (255, 0, 1, ...), variable latency

-- asynchronous mode: tAA and tAPA
-- synchronous mode: tABA and tCLK

-- BCR=1D1F = continuous burst nowrap, 1/2 drive, wait=0=data, 4 cycles, latency=variable, 0=sync


-- Bug? CEn needs pull up! (150us to config)
architecture rtl of psram is
  signal r_adr : std_logic_vector(ps_addr'range);
  signal r_dat : std_logic_vector(31 downto 0);
begin

  ps_clk  <= clk_i;
  ps_addr <= r_adr;
  ps_data <= r_dat(15 downto 0) when r_dat(24)='1' else (others => 'Z');
  ps_seln <= r_dat(17 downto 16);
  ps_oen  <= r_dat(24);
  ps_wen  <= r_dat(25);
  ps_cre  <= r_dat(26);
  ps_advn <= r_dat(27);
  ps_cen  <= r_dat(28);
  
  slave_o.err   <= '0';
  slave_o.stall <= '0';
  
  main : process(clk_i, rstn_i) is
  begin
    if rstn_i = '0' then
      r_adr <= (others => '0');
      r_dat <= (others => '1');
      
      slave_o.ack <= '0';
      slave_o.dat <= (others => '0');
    elsif rising_edge(clk_i) then
      slave_o.ack <= slave_i.cyc and slave_i.stb;
      slave_o.dat <= (others => '0');
      slave_o.dat(ps_data'range) <= ps_data;
      
      r_adr <= slave_i.adr(r_adr'range);
      if (slave_i.cyc and slave_i.stb and slave_i.we) = '1' then
        r_dat <= f_wb_wr(r_dat, slave_i.dat, slave_i.sel, "owr");
      end if;
    end if;
  end process;
  
end rtl;
