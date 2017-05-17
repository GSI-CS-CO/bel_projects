--------------------------------------------------------------------------------
-- filename: io_dec.vhd
-- desc: decoder for fc reads and writes
-- creation date: 16/05/2017
-- last modified: 16/05/2017
-- author: Stefan Rauch <s.rauch@gsi.de>
-- base on: "IO_Dec_d.tdf"; Stand: 25.05.2012 / R.Hartmann
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

entity io_dec is
  port (
    clk             : in std_logic;                     -- systen clk
    ena_every_100ns : in std_logic;                     -- Count_Enable
    sclr            : in std_logic;                     -- Sychron Clear
                                                        --
    cmd_rcv         : in std_logic;                     -- 1 = CMD-Sync. / 0 = Data-Sync -+-> signal from mil_en_decoder
    rcv_rdy         : in std_logic;                     -- new cmd or data ready 
    mil_rcv_d       : in std_logic_vector(15 downto 0); -- received word, either command or data 
                                                        --
    adr             : in std_logic_vector(7 downto 0);  -- interface card address from VG connector
    mb_grp_cnt      : in std_logic_vector(3 downto 0);  -- number of virtual interface card addresses
    mb_mode         : in std_logic;                     -- interface card is in modulbus mode
    fb_mode         : in std_logic;                     -- interface card is in scubus mode
    broadcast_en    : in std_logic;                     -- Broadcast Enable, locks transmitter in broadcast mode 
    send_str        : in std_logic;                     -- starts data transmit
    n_ex_send_ena   : in std_logic;                     -- enable for external transmitter control (Modulbus, IO-Bus, Blockmode or Piggy)
    n_ex_send_str   : in std_logic;                     -- starts transmitting of data in external control mode (Modulbus, IO-Bus, Blockmode or Piggy)


    ifk_sel         : out std_logic;                     -- interface card is selected 
    fc_str          : out std_logic;                     -- function code strobe 
    fc              : out std_logic_vector(7 downto 0);  -- functioncode 
    sw_int          : out std_logic_vector(15 downto 0); -- data used inside interface card 

    wr_mil          : out std_logic;                     -- start transmission  -- > connects to mil_en_decoder 
                                                         --
    bus_fc_str      : out std_logic;                     -- function code strobe for bus operation 
    mb_virt_adr     : out std_logic_vector(3 downto 0);  -- virtual interface card address for modbus operation
    ifk_adr_ok      : out std_logic;                     -- interface card address decoded 
    ifk_virt_adr_ok : out std_logic;                     -- virtual interface card address decoded
    ena_wr_mil_int  : out std_logic;                     -- enable of the internal wr_mil signal
    fc_rd           : out std_logic;                     -- read fc
    fc_wr           : out std_logic;                     -- write fc
    reg_clear       : out std_logic;                     -- register clear after read
    fc_count        : out std_logic_vector(6 downto 0)); -- do not connect
end entity io_dec;

architecture arch of io_dec is

  constant c_fc_wr_a       : integer    := 4;     -- counter value: Funktionscode write start
  constant c_fc_wr_e       : integer    := 15;    -- counter value: Funktionscode write end

  constant c_fc_rd_a       : integer    := 4;     -- counter value: Funktionscode start, read
  constant c_ee            : integer    := 10;    -- counter value: Encoder-Enable
  constant c_fc_rd_e       : integer    := 75;    -- counter value: Funktionscode end, read
  constant c_reg_clear     : integer    := 80;    -- counter value: register clear after read
  constant c_cnt_max       : integer    := 127;   -- max. counter value

  constant c_broadcast_adr : std_logic_vector(7 downto 0)    := x"ff"; -- broadcast address
  constant c_rd_blockmode  : std_logic_vector(7 downto 0)    := x"8f"; -- read blockmode address
  signal s_ifk_adr_ok      : std_logic;
  signal s_brc_adr_ok      : std_logic;
  signal s_rd_blk_mode_ok  : std_logic;
  signal s_virt_ifk_adr_ok : std_logic;
  signal s_mb_virt_adr_ok  : std_logic;
  signal s_mb_virt_adr     : std_logic_vector(3 downto 0);
  signal s_ifk_sel         : std_logic;
  signal s_broadcast       : std_logic;
  signal s_fc_str          : std_logic;
  signal s_fc_str_syn      : std_logic;
  signal s_fc_cnt          : integer range 0 to c_cnt_max       := 0;
  signal s_fc_cnt_en      : std_logic;
  signal s_fc              : std_logic_vector(7 downto 0);
  signal s_data_str        : std_logic;
  signal s_wr_mil_comb     : std_logic;
  signal s_wr_mil1         : std_logic;
  signal s_wr_mil2         : std_logic;
  signal s_wr_mil          : std_logic;
  signal s_reg_clear       : std_logic;
  signal s_fc_wr           : std_logic;
  signal s_fc_rd           : std_logic;
  signal s_bus_fc_str      : std_logic;
  signal s_di              : std_logic_vector(15 downto 0);
  signal s_sw_int          : std_logic_vector(15 downto 0);
begin

  -- ADR-Komperatoren, L-Byte des Commandwortes = Adr
  ifk_adr: process(clk, adr, mil_rcv_d)
  begin
    if rising_edge(clk) then
      if adr = mil_rcv_d(7 downto 0) then
        s_ifk_adr_ok <= '1';
      else
        s_ifk_adr_ok <= '0';
      end if;
    end if;
  end process;

  brc_adr: process(clk, mil_rcv_d)
  begin
    if rising_edge(clk) then
      if c_broadcast_adr = mil_rcv_d(7 downto 0) then
        s_brc_adr_ok <= '1';
      else
        s_brc_adr_ok <= '0';
      end if;
    end if;
  end process;

  rd_blk_mode_ok: process(clk, s_fc)
  begin
    if rising_edge(clk) then
      if c_rd_blockmode = s_fc then
        s_rd_blk_mode_ok <= '1';
      else
        s_rd_blk_mode_ok <= '0';
      end if;
    end if;
  end process;

  -- Komperator virt.-IFK-Adr, L-Byte des Commandwortes = Adr
  virt_ifk_adr: process(clk)
  begin
    if rising_edge(clk) then
      s_mb_virt_adr_ok <= '0';

      case mb_grp_cnt is

        when x"f" =>
          if mil_rcv_d = adr(7 downto 4) & mil_rcv_d(3 downto 0) then
            s_virt_ifk_adr_ok <= '1';
            s_mb_virt_adr <= mil_rcv_d(3 downto 0);
          end if;

        when x"8" =>
          if mil_rcv_d = adr(7 downto 3) & mil_rcv_d(2 downto 0) then
            s_virt_ifk_adr_ok <= '1';
            s_mb_virt_adr <= mil_rcv_d(2 downto 0) and x"7"; 
          end if;

        when x"4" =>
          if mil_rcv_d = adr(7 downto 2) & mil_rcv_d(1 downto 0) then
            s_virt_ifk_adr_ok <= '1';
            s_mb_virt_adr <= mil_rcv_d(2 downto 0) and x"3";
          end if;

        when x"2" =>
          if mil_rcv_d = adr(7 downto 1) & mil_rcv_d(0) then
            s_virt_ifk_adr_ok <= '1';
            s_mb_virt_adr <= x"1";
          end if;
        when others =>
          if mil_rcv_d = adr(7 downto 0) then
            s_virt_ifk_adr_ok <= '1';
            s_mb_virt_adr <= x"0"; 
          end if;
      end case;
    end if;
  end process;


  -- interface card selected
  s_ifk_sel <= s_ifk_adr_ok and rcv_rdy;

  brc_reg: process(clk, sclr, rcv_rdy, cmd_rcv)
  begin
    if sclr = '1' then
      s_broadcast <= '0';
    elsif rising_edge(clk) then
      if rcv_rdy = '1' and cmd_rcv = '1' then
        s_broadcast <= s_brc_adr_ok;
      end if;
    end if;
  end process;

  data_str_reg: process(clk, sclr, rcv_rdy, cmd_rcv)
  begin
    if sclr = '1' then
      s_data_str <= '0';
    elsif rising_edge(clk) then
        s_data_str <= not cmd_rcv and rcv_rdy;
    end if;
  end process;

  fc_str_reg: process(clk, sclr, rcv_rdy, cmd_rcv, s_ifk_adr_ok, s_ifk_adr_ok, s_brc_adr_ok, broadcast_en)
  begin
    if sclr = '1' then
      s_fc_str_syn <= '0';
    elsif rising_edge(clk) then
      s_fc_str_syn <= (((s_brc_adr_ok and broadcast_en) or s_ifk_adr_ok or s_virt_ifk_adr_ok) and cmd_rcv and rcv_rdy);
    end if;
  end process;


  -- function code counter, starts with s_fc_str 
  fc_cnt: process(clk, sclr)
  begin
    if sclr = '1' then
      s_fc_cnt    <= 0;
      s_reg_clear <= '0';
      s_fc_wr     <= '0';
      s_fc_rd     <= '0';
      s_fc_cnt_en <= '0';
    elsif rising_edge(clk) then
      if s_fc_str_syn = '1' then
        s_fc_cnt_en <= '1';
      end if;
      

      if ena_every_100ns = '1' and s_fc_cnt_en = '1' then
        if s_fc_cnt = c_cnt_max then
          s_fc_cnt    <= 0;
          s_fc_cnt_en <= '0';
        else
          s_fc_cnt <= s_fc_cnt + 1;
        end if;
      end if;
      
      -- fc_wr
      if s_fc_cnt = c_fc_wr_a and s_fc(7) = '0' then
        s_fc_wr <= '1';
      end if;

      if s_fc_cnt = c_fc_wr_e and s_fc(7) = '0' then
        s_fc_wr <= '0';
      end if;
      
      -- fc_rd
      if s_fc_cnt = c_fc_rd_a and s_fc(7) = '1' then
        s_fc_rd <= '1';
      end if;
      
      if s_fc_cnt = c_fc_rd_e and s_fc(7) = '1' then
        s_fc_rd <= '0';
      end if;

      -- reg_clear after reset
      if s_fc_cnt = c_reg_clear and s_fc(7) = '1' then
        s_reg_clear <= '1';
      else
        s_reg_clear <= '0';
      end if;
		
    end if;
  end process fc_cnt;



  
  s_wr_mil_comb <= '1' when s_broadcast = '0' and 
                      ((n_ex_send_ena = '1' and send_str = '1' and s_fc_cnt = c_ee and s_rd_blk_mode_ok = '0') or 
                       (n_ex_send_ena = '0' and n_ex_send_str = '0')) else '0'; --  transmit with external transmit control (MB/IO-Bus, Blockmode or Piggy)

  wr_mil_edge: process(clk, sclr)
  begin
    if sclr = '1' then
      s_wr_mil1 <= '0';
      s_wr_mil2 <= '0';
    elsif rising_edge(clk) then
      s_wr_mil1 <= s_wr_mil_comb;
      s_wr_mil2 <= s_wr_mil1;
    end if;
  end process;

  s_wr_mil <= s_wr_mil1 and not s_wr_mil2; -- positive edge

      
  -- fc strobe
  fc_str_out_reg: process(clk)
  begin
    if sclr = '1' then
      s_fc_str <= '0';
    elsif rising_edge(clk) then
      s_fc_str <= (s_fc_rd or s_fc_wr) and ((s_brc_adr_ok and broadcast_en) or s_ifk_adr_ok);
    end if;
  end process;

  bus_fc_str_reg: process(clk, sclr)
  begin
    if sclr = '1' then
      s_bus_fc_str <= '0';
    elsif rising_edge(clk) then
      if s_fc_rd = '1' or s_fc_wr = '1' then
        if (mb_mode = '1' or fb_mode = '1') then
          if (s_brc_adr_ok = '1' and broadcast_en = '1' and s_mb_virt_adr = x"0")
          or s_virt_ifk_adr_ok = '1' or s_ifk_adr_ok = '1' then
            s_bus_fc_str <= '1';
          end if;
        end if;
      else 
        s_bus_fc_str <= '0';
      end if;
    end if;
  end process;
  
  out_data_reg: process(clk)
  begin
    if sclr = '1' then
      s_fc <= (others => '0');
      s_di <= (others => '0');
      s_sw_int <= (others => '0');
    elsif rising_edge(clk) then
      if s_fc_str_syn = '1' then
        s_fc <= mil_rcv_d(15 downto 8);
      end if;
      
      if s_data_str = '1' then
        s_di <= mil_rcv_d;
      end if;

      if s_fc_str_syn = '1' then
        s_sw_int <= s_di;
      end if;
    end if;
  end process out_data_reg;


  ifk_sel         <= s_ifk_sel;

  wr_mil          <= s_wr_mil;
  ena_wr_mil_int  <= '0';

  fc_str          <= s_fc_str;
  fc              <= s_fc;

  sw_int          <= s_sw_int;

  bus_fc_str      <= s_bus_fc_str;
  mb_virt_adr     <= s_mb_virt_adr;

  ifk_adr_ok      <= s_ifk_adr_ok;
  ifk_virt_adr_ok <= s_virt_ifk_adr_ok;

  fc_rd           <= s_fc_rd;
  fc_wr           <= s_fc_wr;
  reg_clear       <= s_reg_clear;
  fc_count        <= "0000000";
  
end architecture arch;
