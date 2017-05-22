---------------------------------------------------------------------------------
-- filename: ifa.vhd
-- desc: version registers and irq mask
-- creation date: 22.05.2017
-- last modified: 22.05.2017
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
use ieee.numeric_std.all;

library work;
use work.ifa8_pkg.all;

entity ifa is
  generic (
    ifa_id : std_logic_vector(7 downto 0) := x"00";
    ifa_vers : std_logic_vector(7 downto 0) := x"00");
  port (
    clk             : in std_logic;
    sclr            : in std_logic;
    ena_every_166ns : in std_logic;
    clr_after_rd    : in std_logic;

    fc_str          : in std_logic;
    fc              : in std_logic_vector(7 downto 0);
    sw_int          : in std_logic_vector(15 downto 0);
    diw             : in std_logic_vector(15 downto 0);
    sts             : in std_logic_vector(7 downto 0);

    hw_sel_mb       : in std_logic;
    hw_sel_fb       : in std_logic;
    jp_sel_6408     : in std_logic;
    if_mode         : in std_logic_vector(15 downto 0);
	 fb_mode			  : in std_logic;
    ifa_adr         : in std_logic_vector(7 downto 0);
    fwl_data_sts    : in std_logic_vector(15 downto 0);
    me_vw_err       : in std_logic_vector(15 downto 0);
    me_data_err     : in std_logic_vector(15 downto 0);
    
    piggy_in        : in std_logic;
    piggy_id        : in std_logic_vector(7 downto 0);
    
    rcv_err         : in std_logic;
    n_utr_15v       : in std_logic;
    irq_ext         : in std_logic;
    slave_ready     : in std_logic;
    n_inl           : in std_logic;
    n_drq           : in std_logic;
    n_drdy          : in std_logic;
    n_sel_err       : in std_logic;

    res_vw_err      : out std_logic;
    reset_cmd       : out std_logic;

    send_str        : out std_logic;
    fc_ext          : out std_logic;
    ifa_fc_int      : out std_logic;
    ifa_sd_sel      : out std_logic_vector(3 downto 0);
    ifa_sd          : out std_logic_vector(15 downto 0);
    vg_data         : out std_logic_vector(15 downto 0);

    n_opt_inl       : out std_logic;
    n_opt_drdy      : out std_logic;
    n_opt_drq       : out std_logic;
    powerup_flag    : out std_logic;

    broad_en        : buffer std_logic;

    wr_fwl_data     : out std_logic;
    wr_fwl_ctrl     : out std_logic;

    rd_fwl_data     : out std_logic;
    rd_fwl_sts      : out std_logic;

    rd_me_vw_err    : out std_logic;
    rd_me_data_err  : out std_logic;

    clr_me_vw_err   : out std_logic;
    clr_me_data_err : out std_logic;

    i2c_sda         : inout std_logic;
    i2c_scl         : inout std_logic;

    ifa_vers_o		  : out std_logic_vector(7 downto 0);
    piggy_led       : out std_logic_vector(15 downto 0);
    piggy_led_out   : out std_logic_vector(15 downto 0));
end entity ifa;

architecture arch of ifa is
  signal inrm           : std_logic_vector(7 downto 0);
  signal s_powerup_flag : std_logic;
  signal i2c_data			: std_logic_vector(12 downto 0);
  signal wr_i2c			: std_logic;
  signal wr_irm			: std_logic;
begin
  ctl: ifactl
  generic map (
    ifa_id => ifa_id)
  port map (
    clk                => clk,
    sclr               => sclr,
    ifa_adr            => ifa_adr,
    fc_str             => fc_str,
    clr_after_rd       => clr_after_rd,
    fc                 => fc,
    di                 => sw_int,
    diw                => diw,
    sts                => sts,
    inrm               => inrm,
    ifa_ctrl           => not n_utr_15v & n_sel_err & irq_ext & hw_sel_mb 
                          & hw_sel_fb & jp_sel_6408 & broad_en & rcv_err,
    ifp_id             => piggy_id,
    ifp_in             => piggy_in,
    ifa_epld_vers      => ifa_vers,
    i2c_data           => i2c_data,
    fwl_data_sts       => fwl_data_sts,
    me_vw_err          => me_vw_err,
    me_data_err        => me_data_err,
    if_mode            => if_mode,

    
    ifa_sd_mux_sel     => ifa_sd_sel,
    ifa_sd_mux         => ifa_sd,
    vg_data            => vg_data,
    send_str           => send_str,
    fc_ext             => fc_ext,
    ifa_fc_int         => ifa_fc_int,
    wr_i2c             => wr_i2c,
    wr_irm             => wr_irm,
    wr_fwl_data        => wr_fwl_data,
    wr_fwl_ctrl        => wr_fwl_ctrl,
    rd_fwl_data        => rd_fwl_data,
    rd_fwl_sts         => rd_fwl_sts,
    rd_me_vw_err       => rd_me_vw_err,
    rd_me_data_err     => rd_me_data_err,
    wr_clr_me_vw_err   => clr_me_vw_err,
    wr_clr_me_data_err => clr_me_data_err,
    ifp_led            => piggy_led,
    ifp_led_out        => piggy_led_out,
    broad_en           => broad_en,
    reset_cmd          => reset_cmd,
    res_vw_err         => res_vw_err);

  i2c_ctrl: i2c
  generic map (
    divisor => 15)
  port map (
    sysclk      => clk,
    clk_en      => ena_every_166ns,
    nreset      => not sclr,
    din         => sw_int(7 downto 0),
    ack_tx      => sw_int(8),
    cmd_stop    => sw_int(9),
    cmd_start   => sw_int(10),
    cmd_send    => sw_int(11),
    cmd_receive => sw_int(12),
    execute     => wr_i2c,
    dout        => i2c_data(7 downto 0),
    ack_rx      => i2c_data(8),
    status      => i2c_data(9),
    dvalid      => i2c_data(10),
    denable     => i2c_data(11),
    busy        => i2c_data(12),
    sda         => i2c_sda,
    scl         => i2c_scl);
  
  irq: irq_mask
  port map (
    intl_d       => sw_int(15),
    drdy_d       => sw_int(14),
    drq_d        => sw_int(13),
    pures_d      => sw_int(8),
    n_intl_si    => n_inl,
    n_drdy_si    => n_drdy,
    n_drq_si     => n_drq,
    clk          => clk,
    wr_irm       => wr_irm,
    sclr         => sclr,
    intl_q       => inrm(7),
    drdy_q       => inrm(6),
    drq_q        => inrm(5),
    n_intl_out   => inrm(4),
    n_drdy_out   => inrm(3),
    n_drq_out    => inrm(2),
    powerup_flag => s_powerup_flag,
    n_opt_inl    => n_opt_inl,
    n_opt_drdy   => n_opt_drdy,
    n_opt_drq    => n_opt_drq);

  inrm(0) <= not s_powerup_flag;
  inrm(1) <= slave_ready when fb_mode = '1' else '1';

  powerup_flag <= s_powerup_flag;
  ifa_vers_o <= ifa_vers;

end architecture arch;
