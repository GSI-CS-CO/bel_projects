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

-- 28012020 VK
-- Select mit FG_Mode      zwischen  nDRQ_FG und n_drq

-- Neue IOs FG_Mode, nDRQ_FG          : in std_logic;
-- geändert n_utr_15v nach utr_15v externer Inverter raus
-- jp_sel_6408 zu not jp_sel_6408

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.ifk10_pkg.all;


entity ifa is
  port (
   sys_clk        : in std_logic;
   sys_reset      : in std_logic;
   ena_every_166ns: in std_logic;
   clr_after_rd   : in std_logic;

   fc_str         : in std_logic;
   fc             : in std_logic_vector(7 downto 0);
   sw_int         : in std_logic_vector(15 downto 0);
   DIW            : in std_logic_vector(15 downto 0);  -- Daten-Istwert vom Stecker STPIGGY1
   sts            : in std_logic_vector(7 downto 0);

   jp_sel_6408    : in std_logic;
   hw_sel_mb      : in std_logic;
   hw_sel_fb      : in std_logic;
   if_mode        : in std_logic_vector(15 downto 0);
   fb_mode        : in std_logic;
   ifa_adr        : in std_logic_vector(7 downto 0);
   fwl_data_sts   : in std_logic_vector(15 downto 0);
   me_vw_err      : in std_logic_vector(15 downto 0);
   me_data_err    : in std_logic_vector(15 downto 0);

   --piggy_in       : in std_logic;
   --piggy_id       : in std_logic_vector(7 downto 0);

   rcv_err        : in std_logic;
   n_utr_15v      : in std_logic;
   nIACK          : in std_logic;
   slave_ready    : in std_logic;

   FG_Mode        : in std_logic;
   nDRQ_FG        : in std_logic;

-- MB_Mode        : in std_logic;

   IFA_nINL       : in std_logic;
   IFA_nDRQ       : in std_logic;
   IFA_nDRDY      : in std_logic;

   Sel_Err        : in std_logic;
   FG112_Mode     : in std_logic;      -- card 112 selected
   FG112_BLKErr   : in std_logic;      -- card 112 Blok mode error

   ifa_id         : in std_logic_vector(7 downto 0);

   res_vw_err     : out std_logic;
   reset_cmd      : out std_logic;

   send_str       : out std_logic;
   nIRQ           : out std_logic;
   ifa_fc_int     : out std_logic;
   ifa_sd_sel     : out std_logic_vector(3 downto 0);
   ifa_sd         : out std_logic_vector(15 downto 0);
   vg_data        : out std_logic_vector(15 downto 0);

   n_opt_inl      : out std_logic;
   n_opt_drdy     : out std_logic;
   n_opt_drq      : out std_logic;
   powerup_flag   : out std_logic;

   broad_en       : buffer std_logic;

   wr_fwl_data    : out std_logic;
   wr_fwl_ctrl    : out std_logic;

   rd_fwl_data    : out std_logic;
   rd_fwl_sts     : out std_logic;

   rd_me_vw_err   : out std_logic;
   rd_me_data_err : out std_logic;

   clr_me_vw_err  : out std_logic;
   clr_me_data_err: out std_logic;

   i2c_sda        : inout std_logic;
   i2c_scl        : inout std_logic;

   ifa_vers_o     : out std_logic_vector(7 downto 0);
   IF_Mode_Reg    : out std_logic_vector(15 downto 0);  -- ifa mode register -- FC=0x60
-- piggy_led      : out std_logic_vector(15 downto 0);
-- piggy_led_out  : out std_logic_vector(15 downto 0);

   nDRQ_o         : out std_logic --für LED Ansteuerung

   );

end entity ifa;

architecture arch of ifa is

signal inrm             : std_logic_vector(7 downto 0);
signal s_powerup_flag   : std_logic :='0';
signal i2c_data         : std_logic_vector(12 downto 0);
signal wr_i2c           : std_logic :='0';
signal wr_irm           : std_logic :='0';

signal fc_ext           : std_logic :='0';
signal irq_ext          : STD_LOGIC :='0';

signal utr_15v          : std_logic :='0';

signal nDRQ_in          : std_logic :='0';


begin


utr_15v <= not n_utr_15v;

ctl: ifactl

  port map (
    clk                => sys_clk,
    sclr               => sys_reset,
    ifa_adr            => ifa_adr,
    fc_str             => fc_str,
    clr_after_rd       => clr_after_rd,
    fc                 => fc,
    di                 => sw_int,
    diw                => DIW,
    sts                => sts,
    inrm               => inrm,
    ifa_ctrl           =>  utr_15v & Sel_Err & irq_ext & hw_sel_mb
                          & hw_sel_fb & not jp_sel_6408 & broad_en & rcv_err,
  --  ifp_id             => piggy_id,
  --  ifp_in             => piggy_in,
 --   ifa_epld_vers      => ifa_vers,
    i2c_data           => i2c_data,
    fwl_data_sts       => fwl_data_sts,
    me_vw_err          => me_vw_err,
    me_data_err        => me_data_err,
    if_mode            => if_mode,
    ifa_id             => ifa_id,

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
    IF_Mode_Reg        => IF_Mode_Reg,
  --  ifp_led            => piggy_led,
  --  ifp_led_out        => piggy_led_out,
    broad_en           => broad_en,
    reset_cmd          => reset_cmd,
    res_vw_err         => res_vw_err);

  i2c_ctrl: i2c
  generic map (
    divisor => 15)
  port map (
    sysclk      => sys_clk,
    clk_en      => ena_every_166ns,
    nreset      => not sys_reset,
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
    n_intl_si    => IFA_nINL,
    n_drdy_si    => IFA_nDRDY,
    n_drq_si     => nDRQ_in,
    clk          => sys_clk,
    wr_irm       => wr_irm,
    sclr         => sys_reset,
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


inrm(0) <= not s_powerup_flag when FG112_Mode='0' else FG112_BLKErr;
inrm(1) <= slave_ready when fb_mode = '1' else '1';

powerup_flag <= s_powerup_flag;
ifa_vers_o   <= VERS_IFA;


nDRQ_in <= nDRQ_FG when FG_Mode ='1' else IFA_nDRQ;
nDRQ_o  <= nDRQ_in;   --nur LED Ausgabe

----------------------

--b2v_23 : debounce
--GENERIC MAP(cnt => 4
--       )
--PORT MAP(sig => Sel_Err,
--     cnt_en => '1',
--     sel => '1',
--     clk => sys_clk,
--     res => sys_reset,
--     sig_deb => n_sel_err);




----
-- set and release of nIRQ signal for external devices
-- ifa8 bdf schematic moved to here
PROCESS(sys_clk,sys_reset,nIACK,fc_ext)
BEGIN
   IF ( sys_reset = '1') THEN
      irq_ext <= '0';
   ELSIF (rising_edge(sys_clk)) THEN
      if nIACK = '0' then
        irq_ext <= '0';
      elsif (fc_ext = '1')then
        irq_ext <= '1';
      END IF;
   END IF;
END PROCESS;

nIRQ <= not irq_ext;

end architecture arch;
