---------------------------------------------------------------------------------
-- filename: mil_serpar.vhd
-- desc: wrapper for soft- and hardware mil end/decoder
-- creation date: 12.05.2017
-- last modified:
-- author: Stefan Rauch <s.rauch@gsi.de>

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

-- Modified
-- anpassung/ Überarbeitung der IOs
-- ergänzung Mute_MILBus und  MIL_BUSY
---------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.math_real.all;
use ieee.numeric_std.all;

library work;
use work.ifk10_pkg.all;

entity mil_serpar is
  port (

    sys_clk         : in std_logic;
    sys_reset       : in std_logic;

    nsel_6408       : in std_logic;       -- select 0-Hardware- or 1-VHDL- encoding/decoding

   --Daten zum Encoder
    wr_mil          : in std_logic;       -- Strobe for Daten senden
    sts_data        : in std_logic_vector(15 downto 0); -- Daten zu senden

    Mute_MILBus     : in STD_LOGIC;       -- 1 - Daten werden nicht über den MIL-Bus ausgegeben - nur sichtbar im LAN -Register
                                          -- 0 - Daten werden über den MIL-Bus ausgegeben - und können sichtbar im LAN-Register reglesen werden

   --Daten vom Decoder
    rcv_rdy         : buffer std_logic;   -- Strobe meldet, dass neue RX Daten vorhanden sind
    cmd_rcv         : buffer std_logic;   -- Datenwort oder Kommandowort empfangen
    mil_rcv_d       : out std_logic_vector(15 downto 0); --empfangene Daten
    rcv_error_ff    : buffer std_logic;   -- Empfangsfehler

    -- für vhdl encoder/decoder
    powerup_flag    : in std_logic;
    clk_24MHz       : in std_logic;
    ena_every_500ns : in std_logic;
    ena_every_250ns : in std_logic;
    clr_me_vw_err   : in std_logic;    -- Fehlerzähler im VHDL-Decoder löschen
    clr_me_data_err : in std_logic;    -- Fehlerzähler im VHDL-Decoder löschen

    --Fehlermeldungen von VHDL decoder  -- nur aktiv, wenn VHDL de/encoder aktiv ist -> nsel_6408=1-> no jumper
    mil_dec_diag    : out std_logic_vector(15 downto 0);
    mil_dec_diag_p  : out std_logic_vector(15 downto 0);
    mil_dec_diag_n  : out std_logic_vector(15 downto 0);
    me_dec_err      : out std_logic_vector(15 downto 0);
    me_error_limit  : out std_logic;
    me_vw_err       : out std_logic_vector(15 downto 0);
    me_data_err     : out std_logic_vector(15 downto 0);

    --für MIL-Device Decoder
    A_ME_DSC        : in std_logic;       -- Decoder-Shift-Clock
    A_ME_TD         : in std_logic;       -- Take-Data -Frame für datenbits vom MIL-Baustein
    A_ME_VW         : in std_logic;       -- Valid Word
    A_ME_SDO        : in std_logic;       -- serielle Daten vom 6408
    A_ME_CDS        : in std_logic;       -- Data/Command Worttyp vom MIL-Baustein
    A_ME_BOI        : out std_logic;      -- differentielle Signale zum 6408 Decoder
    A_ME_BZI        : out std_logic;

    res_vw_err      : in std_logic;       -- Fehlermeldung zurücksetzen

   --Für Device 6408 Encoder
    A_ME_ESC        : in std_logic;       -- Encoder shift clock
    A_ME_EE         : out std_logic;      -- Encoder Startpuls
    A_ME_SS         : out std_logic;      -- Sync-Typ Auswahl
    A_ME_SDI        : out std_logic;      -- serielle Daten zum 6408
    A_ME_SD         : in std_logic;       -- send-data vom MIL-Baustein
    A_ME_nBZO       : in std_logic;       -- differentielle Signale vom 6408
    A_ME_nBOO       : in std_logic;

    -- Ausgang
    A_MIL1_nBZO     : out std_logic;      -- differentielle Signale zum Treiber
    A_MIL1_nBOO     : out std_logic;
    A_MIL1_OUT_en   : out std_logic;      -- Sendetreiber einschalten

    -- Eingang
    A_MIL1_BZI_n    : in std_logic;       -- differentielle Signale vom MIL-Bus-Eingang
    A_MIL1_BOI_p    : in std_logic;
    A_MIL1_nIN_ENA  : out std_logic;      -- Empfängerbaustein 0-enable, 1-gesperrt

    --Busy-Signale
    sd_me_mm        : out std_logic;      -- send_data Zustand weiterleiten an VG
    MIL_TX_Busy     : out STD_LOGIC;      -- Encoder arbeitet gerade-> bisher nur von HW-Encoder, active from WR_MIL signal strobe LH to Send_En HL
    MIL_BUSY        : out std_logic       -- zeigt an, ob irgendetwas auf dem MIL-Bus pasiert, 1-MIL-Bus arbeitet gerade differentiell- 0-Alles still
                                          -- MIL_BUSY wird direkt aus den Eingangssignalen vom Empfangsbaustein generiert
    );
end entity mil_serpar;

--------------------------------------------------------------------
architecture arch of mil_serpar is


COMPONENT mil_enc_ifa
   PORT(
       Cmd_Trm          : IN STD_LOGIC;
       Wr_Mil           : IN STD_LOGIC;
       sys_clk          : IN STD_LOGIC;
       Ena_Every_500ns  : IN STD_LOGIC;
       Ena_Every_250ns  : IN STD_LOGIC;
       Standard_Speed   : IN STD_LOGIC;
       Reset            : IN STD_LOGIC;
       Mil_TRM_D        : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
       nMil_Out_Pos     : OUT STD_LOGIC;
       nMil_Out_Neg     : OUT STD_LOGIC;
       nSel_Mil_Drv     : OUT STD_LOGIC;
       nSel_Mil_rcv     : OUT STD_LOGIC;
       Mil_Rdy_4_Wr     : OUT STD_LOGIC;
       SD               : OUT STD_LOGIC
   );
END COMPONENT;


COMPONENT mil_bipol_dec
GENERIC (
         CLK_in_Hz : INTEGER;
         threshold_no_VW_err : INTEGER;
         threshold_not_equal_err : INTEGER
         );
   PORT(
      sys_clk           : IN STD_LOGIC;
      sys_reset         : IN STD_LOGIC;

      M_in_p            : IN STD_LOGIC;
      M_in_n            : IN STD_LOGIC;
      RD_MIL            : IN STD_LOGIC;
      Clr_No_VW_Cnt     : IN STD_LOGIC;
      Clr_Not_Equal_Cnt : IN STD_LOGIC;
      Power_up          : IN STD_LOGIC;
      Manchester_clk    : IN STD_LOGIC;
      Mil_Out_Pos       : IN STD_LOGIC;
      Mil_Out_Neg       : IN STD_LOGIC;
      Test              : IN STD_LOGIC;

      Res_VW_Err        : in  std_logic;
      rcv_error_ff      : out std_logic;
      Rcv_Cmd           : OUT STD_LOGIC;
      Rcv_Rdy           : OUT STD_LOGIC;
      Error_detect      : OUT STD_LOGIC;
      Rcv_Error_p       : OUT STD_LOGIC;
      Rcv_Error_n       : OUT STD_LOGIC;
      No_VW_p           : OUT STD_LOGIC;
      No_VW_n           : OUT STD_LOGIC;
      Data_not_equal    : OUT STD_LOGIC;
      CMD_not_equal     : OUT STD_LOGIC;
      error_limit_reached : OUT STD_LOGIC;
      Mil_Decoder_Diag_n: OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
      Mil_Decoder_Diag_p: OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
      Mil_Rcv_Data      : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
      No_VW_Cnt         : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
      Not_Equal_Cnt     : OUT STD_LOGIC_VECTOR(15 DOWNTO 0)
   );
END COMPONENT;


signal man_rcv_ready    : std_logic:= '0';
signal man_cmd_rcv      : std_logic:= '0';
signal rcv_d            : std_logic_vector(15 downto 0);

signal Send_MIL_en      : std_logic:= '0';
signal nman_rcv_en      : std_logic;
signal man_rcv_err      : std_logic;
signal nsel_mil_rcv     : std_logic;
signal mm_cmd_rcv       : std_logic;
signal mm_rcv_error     : std_logic;
signal mm_rcv_rdy       : std_logic;
signal rcv_d_vhdl       : std_logic_vector(15 downto 0);
signal BOO_vhdl_out     : std_logic;
signal BZO_vhdl_out     : std_logic;
signal sd_vhdl          : std_logic;
signal nsel_mil_drv     : std_logic;
signal mm_rcv_error_ff  : std_logic;

signal wr_mil_local     : std_logic := '0';
signal BZO_out          : std_logic := '0';
signal BOO_out          : std_logic := '0';
signal mil_enc_busy     : std_logic := '0';

SIGNAL   RCV_ERROR_n    : STD_LOGIC:= '0';
SIGNAL   RCV_ERROR_p    : STD_LOGIC:= '0';

SIGNAL   No_VW_n        : STD_LOGIC:= '0';
SIGNAL   No_VW_p        : STD_LOGIC:= '0';

SIGNAL   Data_not_Eq    : STD_LOGIC:= '0';
SIGNAL   CMD_not_Eq     : STD_LOGIC:= '0';

SIGNAL   En_MIL_Input   : STD_LOGIC:= '0';
SIGNAL   MIL_InL_N   	: STD_LOGIC:= '0'; --local mil input
SIGNAL   MIL_InL_P   	: STD_LOGIC:= '0'; --local mil input

-------------------------------------------------
begin

--Eingangspuffer mit Decoderbaustein verbinden

A_ME_BOI <=MIL_InL_P;
A_ME_BZI <=MIL_InL_N;


MIL_InL_P <= A_MIL1_BOI_p when En_MIL_Input='0' else '0';
MIL_InL_N <= A_MIL1_BZI_n when En_MIL_Input='0' else '0';

--Durchschleifen von BOI,BZI, um MIL_BUSY zu generieren
MIL_BUSY <=MIL_InL_N xor MIL_InL_P; --Zeigt an, ob der MIL-Bus belegt ist -- vk: Debouncer notwendig?

Aio6408D:A6408_decoder
  Port map
  (
    sys_clk    => sys_clk,
    sys_reset  => sys_reset,
    enable     => '1',           -- enable this modul -NC

    DSC        => A_ME_DSC,      -- Decoder-Shift-Clock
    SDO        => A_ME_SDO,      -- serielle Daten input
    TD         => A_ME_TD,       -- Take-Data -Frame für Datenbits vom MIL-Baustein
    CDS        => A_ME_CDS,      -- Data/Command Worttyp vom MIL-Baustein
    VW         => A_ME_VW,       -- Valid Word
    Res_VW_Err => res_vw_err,    -- Reset Fehlerspeicher für VW-Test --NC? war im TDF ebenfalls NC

    rcv_ready  => man_rcv_ready, -- Daten empfangen - strome 1xsys_clk
    cds_out    => man_cmd_rcv,   -- Datentyp 1-command 0-data
    mil_rcv_d  => rcv_d,         -- RX datenwort 16 bit
    rcv_err    => man_rcv_err    -- Empfangsfehler

   );


 wr_mil_local <= wr_mil when  Mute_MILBus='0' else '0';  -- MIL-Bus TX abschalten

 mil_dec_diag <= ( others =>'0');


--neue  Encoder-Variante
   Aio6408E:A6408_encoder
  Port map
  (
  --system controls
    sys_clk    => sys_clk,
    sys_reset  => sys_reset,
    enable     => '1',        -- enable this modul -NC

    ESC        => A_ME_ESC,   -- Encoder-Shift-Clock

  -- encode data controls
    WR_MIL     => wr_mil_local,--start encoding with a LH change here -- Startet ein Mil-Send, muß mindestens 1 Takt (SYS_Clock) aktiv sein.                --
    Sts_Da     => sts_data,   -- data word to be transmitted
    SS_sel     => '0',        -- vorerst Sync select nur Datenwörter
    BZO        => A_ME_nBZO,  -- differentielle Signale vom '6408'
    BOO        => A_ME_nBOO,

    SD         => A_ME_SD,    -- frame for serial data bits in SDI
    SDI        => A_ME_SDI,   -- Serielle Daten zum '6408' für zu sendende Daten

    EE         => A_ME_EE,    -- Encoder Enable -> Startet Sendesequenz im '6408'
    BZO_out    => BZO_out,    -- differentielle Signale zum Bustreiber
    BOO_out    => BOO_out,
    enc_busy   => mil_enc_busy,  -- Encoder arbeitet

    Send_En    => Send_MIL_en,-- Enable-Signal zur Sender-Freigabe
    SS         => A_ME_SS     -- set sync polarity

  );


  vhdl_encoder : mil_enc_ifa
PORT MAP(
      sys_clk     => sys_clk,
      Reset       => sys_reset,

      Cmd_Trm     => '0',  -- IFA sendet nur Wörter vom Typ Data
      Wr_Mil      => wr_mil_local,
      Ena_Every_500ns => Ena_Every_500ns,
      Ena_Every_250ns => Ena_Every_250ns,
      Standard_Speed => '1',
      Mil_TRM_D      => sts_data,

      nMil_Out_Pos => BZO_vhdl_out,
      nMil_Out_Neg => BOO_vhdl_out,  -- warum hier pos/neg gedreht? vk
      nSel_Mil_Drv => nsel_mil_drv,
      nSel_Mil_rcv => nsel_mil_rcv,
      Mil_Rdy_4_Wr => open,
      SD          => sd_vhdl
      );


vhdl_decoder : mil_bipol_dec
GENERIC MAP(CLK_in_Hz => 24000000,
         threshold_no_VW_err => 5,
         threshold_not_equal_err => 5
         )
PORT MAP(
      sys_clk => sys_clk,
      sys_reset      => sys_reset,
      M_in_p         => MIL_InL_P, --vk fixed
      M_in_n         => MIL_InL_N,
      RD_MIL         => mm_rcv_rdy,
      Clr_No_VW_Cnt  => CLR_ME_VW_Err,
      Clr_Not_Equal_Cnt => CLR_ME_Data_Err,

      Power_up       => powerup_flag,
      Manchester_clk => clk_24MHz,
      Mil_Out_Pos    => BZO_vhdl_out,
      Mil_Out_Neg    => BOO_vhdl_out,
      Test => '0',

      Res_VW_Err     => res_vw_err,    -- Reset Fehlerspeicher für VW-Test --NC? war im tdf ebenfalls NC
      rcv_error_ff   => mm_rcv_error_ff,

      Rcv_Cmd        => mm_cmd_rcv,
      Rcv_Rdy        => mm_rcv_rdy,
      Error_detect   => mm_rcv_error,
      Rcv_Error_p    => RCV_ERROR_p,
      Rcv_Error_n    => RCV_ERROR_n,
      No_VW_p        => No_VW_p,
      No_VW_n        => No_VW_n,
      Data_not_equal => Data_not_Eq,
      CMD_not_equal  => CMD_not_Eq,
      error_limit_reached => me_error_limit,
      Mil_Decoder_Diag_n => mil_dec_diag_n,
      Mil_Decoder_Diag_p => mil_dec_diag_p,
      Mil_Rcv_Data   => rcv_d_vhdl,
      No_VW_Cnt      => me_vw_err,
      Not_Equal_Cnt  => me_data_err
      );


---------------------------------------------------------------


  --select hardware or VHDL encoding/decoding
  --nsel_6408: 0-Hardware,  1-VHDL
  sd_me_mm        <= A_ME_SD        when nsel_6408 = '0' else sd_vhdl;  --SD-Signal weiterleiten
  mil_rcv_d       <= rcv_d          when nsel_6408 = '0' else rcv_d_vhdl;

  rcv_error_ff    <= man_rcv_err    when nsel_6408 = '0' else mm_rcv_error_ff;
  cmd_rcv         <= man_cmd_rcv    when nsel_6408 = '0' else mm_cmd_rcv;
  rcv_rdy         <= man_rcv_ready  when nsel_6408 = '0' else mm_rcv_rdy;

  --mux2:
  A_MIL1_OUT_en   <= Send_MIL_en    when nsel_6408 = '0' else not nsel_mil_drv;
  A_MIL1_nIN_ENA  <= '0'; --immer im FPGA mithorchen
  En_MIL_Input    <= Send_MIL_en    when nsel_6408 = '0' else not nsel_mil_rcv;
  A_MIL1_nBOO     <= not BZO_out    when nsel_6408 = '0' else BZO_vhdl_out;
  A_MIL1_nBZO     <= not BOO_out    when nsel_6408 = '0' else BOO_vhdl_out;

  MIL_TX_Busy     <= mil_enc_busy; --bisher nur von HW-Encoder

end architecture;
