-------------------------------------------------------------------------------
-- Title      : Deterministic Altera GXB wrapper - Arria 2
-- Project    : White Rabbit Switch
-------------------------------------------------------------------------------
-- File       : wr_gxb_phy_arriaii.vhd
-- Author     : Wesley W. Terpstra
-- Company    : GSI
-- Created    : 2010-11-18
-- Last update: 2013-03-12
-- Platform   : FPGA-generic
-- Standard   : VHDL'93
-------------------------------------------------------------------------------
-- Description: Single channel wrapper for deterministic GXB
-------------------------------------------------------------------------------
--
-- Copyright (c) 2013 GSI / Wesley W. Terpstra
--
-- This source file is free software; you can redistribute it   
-- and/or modify it under the terms of the GNU Lesser General   
-- Public License as published by the Free Software Foundation; 
-- either version 2.1 of the License, or (at your option) any   
-- later version.                                               
--
-- This source is distributed in the hope that it will be       
-- useful, but WITHOUT ANY WARRANTY; without even the implied   
-- warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR      
-- PURPOSE.  See the GNU Lesser General Public License for more 
-- details.                                                     
--
-- You should have received a copy of the GNU Lesser General    
-- Public License along with this source; if not, download it   
-- from http://www.gnu.org/licenses/lgpl-2.1.html
-- 
--
-------------------------------------------------------------------------------
-- Revisions  :
-- Date        Version  Author    Description
-- 2013-03-12  1.0      terpstra  Rewrote using deterministic mode
-------------------------------------------------------------------------------


-- Before you edit this file, read all of the following documents:
--   Transceiver Architecture in Arria II Devices     <http://www.altera.com/literature/hb/arria-ii-gx/aiigx_52001.pdf>
--   Transceiver Clocking in Arria II Devices         <http://www.altera.com/literature/hb/arria-ii-gx/aiigx_52002.pdf>
--   Reset Control and Power Down in Arria II Devices <http://www.altera.com/literature/hb/arria-ii-gx/aiigx_52004.pdf>
--   Recommended Design Practices (Clock Gating)      <http://www.altera.com/literature/hb/qts/qts_qii51006.pdf>
--   AN 610: Implementing Deterministic Latency for CPRI and OBSAI Protocols in Altera Devices
--                                                    <http://www.altera.com/literature/an/an610.pdf>
--   Achieving Timing Closure in Basic (PMA Direct) Functional Mode 
--                                                    <http://www.altera.com/literature/an/an580.pdf>

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.gencores_pkg.all;
use work.disparity_gen_pkg.all;
use work.altera_networks_pkg.all;

entity wr_arria2_phy is
  generic (
    g_tx_latch_edge : std_logic := '1';
    g_rx_latch_edge : std_logic := '0');
  port (
    clk_reconf_i : in  std_logic; -- 50 MHz
    clk_pll_i    : in  std_logic; -- feeds transmitter PLL
    clk_cru_i    : in  std_logic; -- trains data recovery clock
    clk_free_i   : in  std_logic; -- Used to reset the core
    rst_i        : in  std_logic; -- must last >= 1us
    locked_o     : out std_logic; -- Is the rx_rbclk valid? (clk_free domain)
    loopen_i     : in  std_logic;  -- local loopback enable (Tx->Rx), active hi
    drop_link_i  : in  std_logic; -- Kill the link?

    -- clocked by tx_clk_i
    tx_clk_i       : in  std_logic;
    tx_data_i      : in  std_logic_vector(7 downto 0);   -- data input (8 bits, not 8b10b-encoded)
    tx_k_i         : in  std_logic;  -- 1 when tx_data_i contains a control code, 0 when it's a data byte
    tx_disparity_o : out std_logic;  -- disparity of the currently transmitted 8b10b code (1 = plus, 0 = minus).
    tx_enc_err_o   : out std_logic;  -- error encoding

    rx_rbclk_o    : out std_logic;  -- RX recovered clock
    rx_data_o     : out std_logic_vector(7 downto 0);  -- 8b10b-decoded data output. 
    rx_k_o        : out std_logic;   -- 1 when the byte on rx_data_o is a control code
    rx_enc_err_o  : out std_logic;   -- encoding error indication
    rx_bitslide_o : out std_logic_vector(3 downto 0); -- RX bitslide indication, indicating the delay of the RX path of the transceiver (in UIs). Must be valid when rx_data_o is valid.

    pad_txp_o : out std_logic;
    pad_rxp_i : in std_logic := '0');

end wr_arria2_phy;

architecture rtl of wr_arria2_phy is

  component arria2_phy
    port (
      cal_blk_clk                 : in  std_logic;
      pll_inclk                   : in  std_logic;
      pll_powerdown               : in  std_logic_vector (0 downto 0);
      reconfig_clk                : in  std_logic;
      reconfig_togxb              : in  std_logic_vector (3 downto 0);
      rx_analogreset              : in  std_logic_vector (0 downto 0);
      rx_cruclk                   : in  std_logic_vector (0 downto 0);
      rx_datain                   : in  std_logic_vector (0 downto 0);
      rx_digitalreset             : in  std_logic_vector (0 downto 0);
      rx_enapatternalign          : in  std_logic_vector (0 downto 0);
      rx_seriallpbken             : in  std_logic_vector (0 downto 0);
      tx_bitslipboundaryselect    : in  std_logic_vector (4 downto 0);
      tx_datain                   : in  std_logic_vector (9 downto 0);
      tx_digitalreset             : in  std_logic_vector (0 downto 0);
      pll_locked                  : out std_logic_vector (0 downto 0);
      reconfig_fromgxb            : out std_logic_vector (16 downto 0);
      rx_bitslipboundaryselectout : out std_logic_vector (4 downto 0);
      rx_clkout                   : out std_logic_vector (0 downto 0);
      rx_dataout                  : out std_logic_vector (9 downto 0);
      rx_freqlocked               : out std_logic_vector (0 downto 0);
      rx_patterndetect            : out std_logic_vector (0 downto 0);
      rx_pll_locked               : out std_logic_vector (0 downto 0);
      rx_syncstatus               : out std_logic_vector (0 downto 0);
      tx_clkout                   : out std_logic_vector (0 downto 0);
      tx_dataout                  : out std_logic_vector (0 downto 0));
  end component;

  component arria2_phy_reconf
    port (
      reconfig_clk     : in  std_logic;
      reconfig_fromgxb : in  std_logic_vector (16 downto 0);
      busy             : out std_logic;
      reconfig_togxb   : out std_logic_vector (3 downto 0));
  end component;
  
  component dec_8b10b
    port (
      clk_i       : in  std_logic;
      rst_n_i     : in  std_logic;
      in_10b_i    : in  std_logic_vector(9 downto 0);
      ctrl_o      : out std_logic;
      code_err_o  : out std_logic;
      rdisp_err_o : out std_logic;
      out_8b_o    : out std_logic_vector(7 downto 0));
  end component;

  component enc_8b10b
    port (
      clk_i     : in  std_logic;
      rst_n_i   : in  std_logic;
      ctrl_i    : in  std_logic;
      in_8b_i   : in  std_logic_vector(7 downto 0);
      err_o     : out std_logic;
      dispar_o  : out std_logic;
      out_10b_o : out std_logic_vector(9 downto 0));
  end component;

  signal clk_rx_gxb    : std_logic; -- pre clkctrl
  signal clk_rx        : std_logic; -- global clock
  signal clk_tx_gxb    : std_logic; -- pre clkctrl
  signal clk_tx        : std_logic; -- local  clock
  signal pll_locked    : std_logic;
  signal rx_freqlocked : std_logic;
  
  type t_state is (WAIT_POWER, WAIT_CMU, WAIT_CONFIG, WAIT_LOCK, DONE);
  
  signal rst_state         : t_state := WAIT_POWER;
  signal rst_delay         : unsigned(6 downto 0) := (others => '1'); -- must span >= 4us (128@20MHz=6.4us)
  signal pll_powerdown     : std_logic;
  signal tx_digitalreset   : std_logic; -- sys domain
  signal rx_analogreset    : std_logic; -- sys domain
  signal rx_digitalreset   : std_logic; -- sys domain
  
  signal free_rstn          : std_logic_vector(2 downto 0);
  signal free_pll_locked    : std_logic_vector(2 downto 0);
  signal free_reconfig_busy : std_logic_vector(2 downto 0);
  signal free_rx_freqlocked : std_logic_vector(2 downto 0);
  signal free_drop_link     : std_logic_vector(2 downto 0);
  
  signal tx_8b10b_rstn : std_logic_vector(2 downto 0); -- tx domain
  signal rx_8b10b_rstn : std_logic_vector(2 downto 0); -- rx domain
  
  signal reconfig_busy    : std_logic;
  signal reconfig_togxb   : std_logic_vector (3 downto 0);
  signal reconfig_fromgxb : std_logic_vector (16 downto 0);
  
  signal rx_dump_link                : std_logic_vector(6 downto 0); -- Long enough to kill ep_sync_detect
  signal rx_enc_err                  : std_logic;
  signal rx_bitslipboundaryselectout : std_logic_vector (4 downto 0);
  
  signal rx_gxb_dataout              : std_logic_vector (9 downto 0); -- signal out of GXB
  signal rx_glbl_dataout             : std_logic_vector (9 downto 0); -- globally clocked register
  
  signal rx_gxb_syncstatus           : std_logic;
  signal rx_glbl_syncstatus          : std_logic;
  
  signal tx_enc_datain               : std_logic_vector (9 downto 0); -- registered encoder output (tx_clk_i)
  signal tx_reg_datain               : std_logic_vector (9 downto 0); -- clock transfer register   (tx_clk_i)
  signal tx_gxb_datain               : std_logic_vector (9 downto 0); -- clock transfer register   (clk_tx)
  
begin

  rx_rbclk_o   <= clk_rx;
  U_RxClkout : single_region
    port map (
      inclk  => clk_rx_gxb,
      outclk => clk_rx);
  
  U_TxClkout : single_region
    port map (
      inclk  => clk_tx_gxb,
      outclk => clk_tx);
  
  -- Altera PHY calibration block
  U_Reconf : arria2_phy_reconf
    port map (
      reconfig_clk     => clk_reconf_i,
      reconfig_fromgxb => reconfig_fromgxb,
      busy             => reconfig_busy,
      reconfig_togxb   => reconfig_togxb);

  --- The serializer and byte aligner
  U_The_PHY : arria2_phy
    port map (
      -- Clocks feeding the CMU and CRU of the transceiver
      pll_inclk                   => clk_pll_i,
      rx_cruclk(0)                => clk_cru_i,
      -- Derived clocks used for tx/rx lines
      tx_clkout(0)                => clk_tx_gxb,
      pll_locked(0)               => pll_locked,
      rx_clkout(0)                => clk_rx_gxb,
      rx_freqlocked(0)            => rx_freqlocked,
      rx_pll_locked               => open,
      -- Calibration control of the GXB
      cal_blk_clk                 => clk_reconf_i,
      reconfig_clk                => clk_reconf_i,
      reconfig_togxb              => reconfig_togxb,
      reconfig_fromgxb            => reconfig_fromgxb,
      rx_seriallpbken(0)          => loopen_i,
      -- Reset logic of the GXB
      pll_powerdown(0)            => pll_powerdown,
      tx_digitalreset(0)          => tx_digitalreset,
      rx_analogreset(0)           => rx_analogreset,
      rx_digitalreset(0)          => rx_digitalreset,
      -- Word alignment of the serializer
      rx_enapatternalign(0)       => '1',
      rx_patterndetect            => open,
      rx_syncstatus(0)            => rx_gxb_syncstatus,
      rx_bitslipboundaryselectout => rx_bitslipboundaryselectout,
      tx_bitslipboundaryselect    => (others => '0'),
      -- Actual data lines
      rx_datain(0)                => pad_rxp_i,
      rx_dataout                  => rx_gxb_dataout,
      tx_dataout(0)               => pad_txp_o,
      tx_datain                   => tx_gxb_datain);
  
  -- Encode the TX data
  encoder : enc_8b10b
    port map(
      clk_i     => tx_clk_i,
      rst_n_i   => tx_8b10b_rstn(0),
      ctrl_i    => tx_k_i,
      in_8b_i   => tx_data_i,
      err_o     => tx_enc_err_o,
      dispar_o  => tx_disparity_o,
      out_10b_o => tx_enc_datain);
  
  -- Decode the RX data
  decoder : dec_8b10b
    port map(
      clk_i       => clk_rx,
      rst_n_i     => rx_8b10b_rstn(0),
      in_10b_i    => rx_glbl_dataout,
      ctrl_o      => rx_k_o,
      code_err_o  => rx_enc_err,
      rdisp_err_o => open,
      out_8b_o    => rx_data_o);
  rx_enc_err_o <= rx_enc_err or rx_dump_link(0);
  
  p_sync : process(clk_free_i, rst_i) is
  begin
    if rst_i = '1' then
      free_rstn <= (others => '0');
    elsif rising_edge(clk_free_i) then
      free_rstn <= '1' & free_rstn(free_rstn'left downto 1);
    end if;
  end process;
  
  -- Reset procedure follows Figure 4-4 of Reset Control and Power Down in Arria II Devices
  p_reset : process(clk_free_i, free_rstn(0)) is
  begin
    if free_rstn(0) = '0' then
      rst_state       <= WAIT_POWER;
      rst_delay       <= (others => '1');
      pll_powerdown   <= '1';
      rx_analogreset  <= '1';
      locked_o        <= '0';
      tx_digitalreset <= '1';
      rx_digitalreset <= '1';
    elsif rising_edge(clk_free_i) then
      -- Synchronize foreign signals
      free_pll_locked    <= pll_locked    & free_pll_locked   (free_pll_locked'left    downto 1);
      free_reconfig_busy <= reconfig_busy & free_reconfig_busy(free_reconfig_busy'left downto 1);
      free_rx_freqlocked <= rx_freqlocked & free_rx_freqlocked(free_rx_freqlocked'left downto 1);
      free_drop_link     <= drop_link_i   & free_drop_link    (free_drop_link'left     downto 1);
      
      case rst_state is
        when WAIT_POWER =>
          pll_powerdown   <= '1';
          rx_analogreset  <= '1';
          locked_o        <= '0';
          tx_digitalreset <= '1';
          rx_digitalreset <= '1';
          
          rst_delay <= rst_delay - 1;
          
          if rst_delay = 0 then
            rst_delay <= (others => '1');
            rst_state <= WAIT_CMU;
          end if;
          
        when WAIT_CMU =>
          pll_powerdown <= '0';
          
          if free_pll_locked(0) = '0' then
            rst_delay <= (others => '1');
          else
            rst_delay <= rst_delay - 1;
          end if;
          
          if rst_delay = 0 then
            rst_delay <= (others => '1');
            rst_state <= WAIT_CONFIG;
          end if;
          
        when WAIT_CONFIG =>
          if free_reconfig_busy(0) = '1' then
            rst_delay <= (others => '1');
          else
            rst_delay <= rst_delay - 1;
          end if;
          
          if rst_delay = 0 then
            rst_delay <= (others => '1');
            rst_state <= WAIT_LOCK;
          end if;
          
          if free_pll_locked(0) = '0' then
            rst_delay <= (others => '1');
            rst_state <= WAIT_POWER;
          end if;
        
        when WAIT_LOCK =>
          rx_analogreset <= '0';
          
          if free_rx_freqlocked(0) = '0' then
            rst_delay <= (others => '1');
          else
            rst_delay <= rst_delay - 1;
          end if;
          
          if rst_delay = 0 then
            rst_delay <= (others => '1');
            rst_state <= DONE;
          end if;
          
          if free_pll_locked(0) = '0' then
            rst_delay <= (others => '1');
            rst_state <= WAIT_POWER;
          end if;
        
        when DONE =>
          -- RX clock is now locked and safe
          locked_o <= '1';
          
          -- Kill the link upon request
          tx_digitalreset <= free_drop_link(0);
          rx_digitalreset <= free_drop_link(0);
          
          if free_pll_locked(0) = '0' then
            rst_delay <= (others => '1');
            rst_state <= WAIT_POWER;
          end if;
          
      end case;
    end if;
  end process;
  
  
  -- Generate reset for 8b10b encoder
  p_pll_reset : process(tx_clk_i) is
  begin
    if rising_edge(tx_clk_i) then
      tx_8b10b_rstn <= (not tx_digitalreset) & tx_8b10b_rstn(tx_8b10b_rstn'left downto 1);
    end if;
  end process;
  
  -- Generate reset for the 8b10b decoder and ep_sync_detect
  -- should use global version of clk_rx
  p_rx_reset : process(clk_rx) is
  begin
    if rising_edge(clk_rx) then
      rx_8b10b_rstn <= (not rx_digitalreset) & rx_8b10b_rstn(rx_8b10b_rstn'left downto 1);
    end if;
  end process;
  
  -- Dump the link if the bitslide changes
  p_dump_link : process(clk_rx) is
  begin
    if rising_edge(clk_rx) then
      if rx_glbl_syncstatus = '1' then
        rx_dump_link <= (others => '1');
      else
        rx_dump_link <= '0' & rx_dump_link(rx_dump_link'left downto 1);
      end if;
    end if;
  end process;
  
  -- Cross clock domain from tx_clk_i to tx_clk
  -- These clocks must be phase aligned
  -- Registers tx_reg_datain and tx_gxb_datain must be logic locked
  -- to the same ALM, preferrably directly beside the GXB.
  p_tx_path0 : process(tx_clk_i) is
  begin
    if tx_clk_i'event and tx_clk_i = (not g_tx_latch_edge) then
      tx_reg_datain <= tx_enc_datain;
    end if;
  end process;
  p_tx_path1 : process(clk_tx) is
  begin
    if clk_tx'event and clk_tx = g_tx_latch_edge then
      tx_gxb_datain <= tx_reg_datain;
    end if;
  end process;
  
  -- Additional register to improve timings
  p_rx_path : process(clk_rx) is
  begin
    if clk_rx'event and clk_rx = g_rx_latch_edge then
      rx_glbl_dataout <= rx_gxb_dataout;
      rx_glbl_syncstatus <= rx_gxb_syncstatus;
    end if;
  end process;
  
  -- Slow registered signals out of the GXB
  p_rx_regs : process(clk_rx) is
  begin
    if rising_edge(clk_rx) then
      rx_bitslide_o <= rx_bitslipboundaryselectout(3 downto 0);
    end if;
  end process;
  
end rtl;
