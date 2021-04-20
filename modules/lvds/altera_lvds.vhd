--! @file altera_lvds.vhd
--! @brief LVDS interface
--! @author Wesley W. Terpstra <w.terpstra@gsi.de>
--!
--! Copyright (C) 2013 GSI Helmholtz Centre for Heavy Ion Research GmbH
--!
--! This combines all the common GSI components together
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

library work;
use work.eca_pkg.all;
use work.altera_lvds_pkg.all;
use work.arria5_lvds_pkg.all;
use work.arria10_lvds_pkg.all;
use work.gencores_pkg.all;

entity altera_lvds is
  generic(
    g_family   : string;
    g_inputs   : natural;
    g_outputs  : natural;
    g_tx_multi : boolean := false;
    g_rx_multi : boolean := false;
    g_invert   : boolean := false);
  port(
    clk_ref_i    : in  std_logic;
    rstn_ref_i   : in  std_logic;
    clk_lvds_i   : in  std_logic;
    clk_enable_i : in  std_logic;
    dat_o        : out t_lvds_byte_array(g_inputs-1  downto 0);
    lvds_p_i     : in  std_logic_vector(g_inputs-1  downto 0);
    lvds_n_i     : in  std_logic_vector(g_inputs-1  downto 0);
    lvds_i_led_o : out std_logic_vector(g_inputs-1  downto 0);
    dat_i        : in  t_lvds_byte_array(g_outputs-1 downto 0);
    lvds_p_o     : out std_logic_vector(g_outputs-1 downto 0);
    lvds_n_o     : out std_logic_vector(g_outputs-1 downto 0);
    lvds_o_led_o : out std_logic_vector(g_outputs-1 downto 0));
end altera_lvds;

architecture rtl of altera_lvds is
  function f_1(x : boolean) return std_logic is begin
    if x then return '1'; else return '0'; end if;
  end function;

  constant c_toggle : t_lvds_byte := (others => f_1(g_invert));

  signal clk_lvds   : std_logic;
  signal clk_enable : std_logic;
  signal clk_core   : std_logic;
  signal lvds_idat  : std_logic_vector(g_inputs-1  downto 0);
  signal lvds_odat  : std_logic_vector(g_outputs-1 downto 0);
  signal s_dat_i    : t_lvds_byte_array(g_outputs-1 downto 0);
  signal s_dat_o    : t_lvds_byte_array(g_inputs-1  downto 0);
  signal s_led      : std_logic_vector(g_inputs-1 downto 0);
begin

  arria2_y : if g_family = "Arria II" generate
    clk_lvds   <= clk_lvds_i;
    clk_enable <= clk_enable_i;
    clk_core   <= clk_ref_i;
  end generate;

  arria5_y : if g_family = "Arria V" generate
    clk : arriav_pll_lvds_output
      port map(
        ccout(1) => clk_enable_i,
        ccout(0) => clk_lvds_i,
        loaden   => clk_enable,
        lvdsclk  => clk_lvds);
    clk_core <= clk_ref_i;
  end generate;

  arria10_no_clock_tree_y : if g_family = "Arria 10 GX SCU4" or g_family = "Arria 10 GX PEX10" or g_family = "Arria 10 GX FTM10" generate
    clk_lvds   <= clk_lvds_i;
    clk_enable <= clk_enable_i;
    clk_core   <= clk_ref_i;
  end generate;

  --arria10_y : if g_family = "Arria 10 GX SCU4" generate
  --  pll : arria10_scu4_lvds_pll
  --    port map(
  --    rst         => not(rstn_ref_i),
  --    refclk      => clk_ref_i,
  --    locked      => open,
  --    lvds_clk(0) => clk_lvds,
  --    loaden(0)   => clk_enable,
  --    outclk_2    => clk_core);
  --end generate;

  tx_scu4_special_handling : if (g_tx_multi = true and g_family = "Arria 10 GX SCU4") generate
    lvds : altera_lvds_tx_multi_scu4_wrap
      generic map(
        g_family   => g_family)
      port map(
        tx_core             => clk_core,
        tx_inclock          => clk_lvds,
        tx_enable           => clk_enable,
        tx_in(7 downto 0)   => s_dat_i(0),
        tx_in(15 downto 8)  => s_dat_i(1),
        tx_in(23 downto 16) => s_dat_i(2),
        tx_out(0)           => lvds_odat(0),
        tx_out(1)           => lvds_odat(1),
        tx_out(2)           => lvds_odat(2));
  end generate;

  rx_scu4_special_handling : if (g_rx_multi = true and g_family = "Arria 10 GX SCU4") generate
    lvds : altera_lvds_rx_multi_scu4_wrap
      generic map(
        g_family   => g_family)
        port map(
          rx_core              => clk_core,
          rx_inclock           => clk_lvds,
          rx_enable            => clk_enable,
          rx_in(0)             => lvds_idat(0),
          rx_in(1)             => lvds_idat(1),
          rx_in(2)             => lvds_idat(2),
          rx_out(7 downto 0)   => s_dat_o(0),
          rx_out(15 downto 8)  => s_dat_o(1),
          rx_out(23 downto 16) => s_dat_o(2));
  end generate;

  tx : for i in 0 to g_outputs-1 generate
    led : gc_extend_pulse
      generic map(
        g_width => 125_000_000/20) -- 20 Hz
      port map(
        clk_i      => clk_ref_i,
        rst_n_i    => rstn_ref_i,
        pulse_i    => dat_i(i)(0),
        extended_o => lvds_o_led_o(i));

    s_dat_i(i) <= dat_i(i) xor c_toggle;

    lvds_single_out : if g_tx_multi = false generate
      lvds : altera_lvds_tx
        generic map(
          g_family   => g_family)
        port map(
          tx_core    => clk_core,
          tx_inclock => clk_lvds,
          tx_enable  => clk_enable,
          tx_in      => s_dat_i(i),
          tx_out     => lvds_odat(i));
    end generate;

    arria5_arria2_obuf : if not(g_family = "Arria 10 GX SCU4" or g_family = "Arria 10 GX PEX10" or g_family = "Arria 10 GX FTM10") generate
    buf : altera_lvds_obuf
      generic map(
        g_family  => g_family)
      port map(
        datain    => lvds_odat(i),
        dataout   => lvds_p_o(i),
        dataout_b => lvds_n_o(i));
    end generate;

    arria10_obuf : if g_family = "Arria 10 GX SCU4" or g_family = "Arria 10 GX PEX10" or g_family = "Arria 10 GX FTM10" generate
     lvds_p_o <= lvds_odat;
    end generate;

    --buf : altera_lvds_obuf
    --  generic map(
    --    g_family  => g_family)
    --  port map(
    --    datain    => lvds_odat(i),
    --    dataout   => lvds_p_o(i),
     --   dataout_b => lvds_n_o(i));

  end generate;

  rx : for i in 0 to g_inputs-1 generate
    buf : altera_lvds_ibuf
      generic map(
        g_family  => g_family)
      port map(
        datain    => lvds_p_i(i),
        datain_b  => lvds_n_i(i),
        dataout   => lvds_idat(i));

    lvds_single_in : if g_rx_multi = false generate
      lvds : altera_lvds_rx
        generic map(
          g_family   => g_family)
        port map(
          rx_core    => clk_core,
          rx_inclock => clk_lvds,
          rx_enable  => clk_enable,
          rx_in      => lvds_idat(i),
          rx_out     => s_dat_o(i));
    end generate;

    dat_o(i) <= s_dat_o(i) xor c_toggle;
    s_led(i) <= s_dat_o(i)(0) xor c_toggle(0);

    led : gc_extend_pulse
      generic map(
        g_width => 125_000_000/20) -- 20 Hz
      port map(
        clk_i      => clk_ref_i,
        rst_n_i    => rstn_ref_i,
        pulse_i    => s_led(i),
        extended_o => lvds_i_led_o(i));
  end generate;

end rtl;
