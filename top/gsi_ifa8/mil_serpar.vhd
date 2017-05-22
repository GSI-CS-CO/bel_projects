---------------------------------------------------------------------------------
-- filename: mil_serpar.vhd
-- desc: wrapper for soft- and hardware mil end/decoder
-- creation date: 12.05.2017
-- last modified:
-- author: Stefan Rauch <s.rauch@gsi.de>
--
-- Copyright (C) 2017 GSI Helmholtz Centre for Heavy Ion Research GmbH 
--
---------------------------------------------------------------------------------
-- This library is free software; you can redistribute it and/or
-- modify it under the terms of the GNU Lesser General Public
-- License as published by the Free Software Foundation; either
-- version 3 of the License, or (at your option) any later version.
--
-- This library is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- Lesser General Public License for more details.
--  
-- You should have received a copy of the GNU Lesser General Public
-- License along with this library. If not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------- 
library ieee;
use ieee.std_logic_1164.all;
use ieee.math_real.all;
use ieee.numeric_std.all;

library work;
use work.ifa8_pkg.all;

entity mil_serpar is
  port (
    powerup_flag    : in std_logic;
    sclr            : in std_logic;
    clk             : in std_logic;
    manchester_clk  : in std_logic;
    sel_mm_io       : in std_logic;
    ena_every_500ns : in std_logic;
    clr_me_vw_err   : in std_logic;
    clr_me_data_err : in std_logic;
    
    dsc             : in std_logic;
    esc             : in std_logic;
    td              : in std_logic;
    vw              : in std_logic;
    in_neg          : in std_logic;
    in_pos          : in std_logic;
    sdo             : in std_logic;
    cds             : in std_logic;
    bzo             : in std_logic;
    boo             : in std_logic;
    sd              : in std_logic;
    
    wr_mil          : in std_logic;
    sts_data        : in std_logic_vector(15 downto 0);
    res_vw_err      : in std_logic;

    mm_io_ena       : out std_logic;
    ee              : out std_logic;
    ss              : out std_logic;
    sdi             : out std_logic;
    sd_me_mm        : out std_logic;
    rcv_error_ff    : buffer std_logic;
    cmd_rcv         : buffer std_logic;
    rcv_rdy         : buffer std_logic;

    mil_rcv_d       : out std_logic_vector(15 downto 0);
    sender_en       : out std_logic;
    nempf_en        : out std_logic;
    nbzo_out        : out std_logic;
    nboo_out        : out std_logic;
    
    mil_dec_diag    : out std_logic_vector(15 downto 0);
    mil_dec_diag_p  : out std_logic_vector(15 downto 0);
    mil_dec_diag_n  : out std_logic_vector(15 downto 0);
    me_dec_err      : out std_logic_vector(15 downto 0);
    me_error_limit  : out std_logic;
    me_vw_err       : out std_logic_vector(15 downto 0);
    me_data_err     : out std_logic_vector(15 downto 0));
end entity mil_serpar;

architecture arch of mil_serpar is
  signal man_rcv_ready   : std_logic;
  signal man_cmd_rcv     : std_logic;
  signal man_d           : std_logic_vector(15 downto 0);
  signal man_sdi         : std_logic;
  signal man_ee          : std_logic;
  signal nman_send_en    : std_logic;
  signal nman_rcv_en     : std_logic;
  signal man_rcv_err     : std_logic;
  signal nsel_mil_rcv    : std_logic;
  signal mm_cmd_rcv      : std_logic;
  signal mm_rcv_error    : std_logic;
  signal mm_rcv_rdy      : std_logic;
  signal mm_d            : std_logic_vector(15 downto 0);
  signal nmil_out_neg    : std_logic;
  signal nmil_out_pos    : std_logic;
  signal send_data       : std_logic;
  signal nsel_mil_drv 	: std_logic;
  signal mm_rcv_error_ff : std_logic;
  signal send_en         : std_logic;


begin
  io6408: io_6408_a
  port map (  
    clk        => clk,
    sclr       => sclr,
    sdo        => sdo,
    dsc        => dsc,
    td         => td,
    cds        => cds,
    vw         => vw,
    res_vw_err => res_vw_err,
    wr_mil     => wr_mil,
    sts_da     => sts_data,
    esc        => esc,
    sd         => sd,
    bzo        => bzo,
    boo        => boo,
    
    rcv_ready  => man_rcv_ready,
    cmd_rcv    => man_cmd_rcv,
    mil_rcv_d  => man_d,
    sdi        => man_sdi,
    ee         => man_ee,
    nsend_en   => nman_send_en,
    nrcv_en    => nman_rcv_en,
    rcv_err    => man_rcv_err);
    
  sw_en_dec: mil_en_decoder_ifa
  port map (
    mil_in_pos      => in_neg, --FIXME
    mil_in_neg      => in_pos,
    clr_me_vw_err   => clr_me_vw_err,
    clr_me_data_err => clr_me_data_err,
    wr_mil          => wr_mil,
    cmd_trm         => '0',
    mil_trm_d       => sts_data,
    clk             => clk,
    manchester_clk  => manchester_clk,
    ena_every_500ns => ena_every_500ns,
    sclr            => sclr,
    powerup_flag    => powerup_flag,
    test            => '0',
    
    nsel_mil_rcv    => nsel_mil_rcv,
    cmd_rcv         => mm_cmd_rcv,
    rcv_error       => mm_rcv_error,
    rcv_rdy       	=> mm_rcv_rdy,
    mil_rcv_d       => mm_d,
    nmil_out_pos    => nmil_out_neg,
    nmil_out_neg    => nmil_out_pos,
    send_data       => send_data,
    nsel_mil_drv    => nsel_mil_drv,
    mil_rdy_4_wr    => open,
    mil_dec_diag_p  => mil_dec_diag_p,
    mil_dec_diag_n  => mil_dec_diag_n,
    me_dec_err      => me_dec_err,
    me_vw_err       => me_vw_err,
    me_data_err     => me_data_err,
    me_error_limit  => me_error_limit);

  rcv_err_ff: process(clk)
  begin
    if res_vw_err = '1' then
      mm_rcv_error_ff <= '0';
    elsif rising_edge(clk) then
      mm_rcv_error_ff <= '1';
    end if;
  end process;

  mm_io_ena <= sel_mm_io;
  ee        <= man_ee;
  ss        <= '0';
  sdi       <= man_sdi;

  sd_me_mm  <= sd when sel_mm_io = '0' else send_data;

  mil_rcv_d <= man_d when sel_mm_io = '0' else mm_d;

  rcv_mux: process(rcv_error_ff, cmd_rcv, rcv_rdy, sel_mm_io)
  begin
    if sel_mm_io = '0' then
      rcv_error_ff <= man_rcv_err;
      cmd_rcv      <= man_cmd_rcv;
      rcv_rdy      <= man_rcv_ready;
    else
      rcv_error_ff <= mm_rcv_error_ff;
      cmd_rcv      <= mm_cmd_rcv;
      rcv_rdy      <= mm_rcv_rdy;
    end if;
  end process;

  send_ena: process(esc)
  begin
    if nman_send_en = '0' then
      send_en <= '0';
    elsif rising_edge(esc) then
      send_en <= '1';
    end if;
  end process;

  mux2: process(sel_mm_io, send_en, bzo, boo, nsel_mil_drv, nsel_mil_rcv, nmil_out_neg, nmil_out_pos)
  begin
    if sel_mm_io = '0' then
      sender_en <= not send_en; -- FIXME only one can be correct
      nempf_en  <= not send_en; -- FIXME
      nboo_out  <= not bzo;
      nbzo_out  <= not boo;
    else
      sender_en <= nsel_mil_drv;
      nempf_en  <= not nsel_mil_rcv;
      nboo_out  <= nmil_out_pos;
      nbzo_out  <= nmil_out_neg;
    end if;
  end process;



end architecture;
    
     
