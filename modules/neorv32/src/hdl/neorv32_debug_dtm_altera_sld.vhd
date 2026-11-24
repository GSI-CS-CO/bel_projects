-- ================================================================================ --
-- NEORV32 OCD - RISC-V-Compatible Debug Transport Module (DTM)                     --
-- -------------------------------------------------------------------------------- --
-- Compatible to RISC-V debug spec. versions 0.13 and 1.0.                          --
-- -------------------------------------------------------------------------------- --
-- The NEORV32 RISC-V Processor - https://github.com/stnolting/neorv32              --
-- Copyright (c) NEORV32 contributors.                                              --
-- Copyright (c) 2020 - 2026 Stephan Nolting. All rights reserved.                  --
-- Licensed under the BSD-3-Clause license, see LICENSE for details.                --
-- SPDX-License-Identifier: BSD-3-Clause                                            --
-- -------------------------------------------------------------------------------- --
-- Original content by Stephan Nolting.                                             --
-- Modified by Niklaus Leuenberger to use Altera Virtual JTAG Interface (VJI).      --
-- Source: github.com/NikLeberg/bog_video @ a2415301, see also                       --
-- github.com/stnolting/neorv32/discussions/1587                                    --
-- Replaces rtl/core/neorv32_debug_dtm.vhd: the DTM is reached through the Altera   --
-- SLD hub (OpenOCD 'vjtag create', adapter aji_client), the jtag_* ports are unused.--
-- ================================================================================ --

library ieee;
use ieee.std_logic_1164.all;

library neorv32;
use neorv32.neorv32_package.all;

entity neorv32_debug_dtm is
  generic (
    IDCODE_VERSION : std_ulogic_vector(3 downto 0);  -- version
    IDCODE_PARTID  : std_ulogic_vector(15 downto 0); -- part number
    IDCODE_MANID   : std_ulogic_vector(10 downto 0)  -- manufacturer id
  );
  port (
    -- global control --
    clk_i      : in  std_ulogic; -- global clock line
    rstn_i     : in  std_ulogic; -- global reset line, low-active
    -- JTAG connection (TAP access) --
    jtag_tck_i : in  std_ulogic; -- serial clock
    jtag_tdi_i : in  std_ulogic; -- serial data input
    jtag_tdo_o : out std_ulogic; -- serial data output
    jtag_tms_i : in  std_ulogic; -- mode select
    -- debug module interface (DMI) --
    dmi_req_o  : out dmi_req_t;  -- request
    dmi_rsp_i  : in  dmi_rsp_t   -- response
  );
end neorv32_debug_dtm;

architecture neorv32_debug_dtm_altera_sld_rtl of neorv32_debug_dtm is

  -- Virtual JTAG Interface (VJI) megafunction.
  -- Ports marked optional are for debugging purposes only.
  component sld_virtual_jtag
    generic (
      lpm_hint                : string  := "UNUSED";
      lpm_type                : string  := "sld_virtual_jtag";
      sld_auto_instance_index : string  := "YES";
      sld_instance_index      : natural := 0;
      sld_ir_width            : natural := 1;
      sld_sim_action          : string  := "UNUSED";
      sld_sim_n_scan          : natural := 0;
      sld_sim_total_length    : natural := 0
    );
    port (
      ir_in              : out std_ulogic_vector(SLD_IR_WIDTH-1 downto 0);
      ir_out             : in  std_ulogic_vector(SLD_IR_WIDTH-1 downto 0);
      jtag_state_cdr     : out std_ulogic; -- optional
      jtag_state_cir     : out std_ulogic; -- optional
      jtag_state_e1dr    : out std_ulogic; -- optional
      jtag_state_e1ir    : out std_ulogic; -- optional
      jtag_state_e2dr    : out std_ulogic; -- optional
      jtag_state_e2ir    : out std_ulogic; -- optional
      jtag_state_pdr     : out std_ulogic; -- optional
      jtag_state_pir     : out std_ulogic; -- optional
      jtag_state_rti     : out std_ulogic; -- optional
      jtag_state_sdr     : out std_ulogic; -- optional
      jtag_state_sdrs    : out std_ulogic; -- optional
      jtag_state_sir     : out std_ulogic; -- optional
      jtag_state_sirs    : out std_ulogic; -- optional
      jtag_state_tlr     : out std_ulogic; -- optional
      jtag_state_udr     : out std_ulogic; -- optional
      jtag_state_uir     : out std_ulogic; -- optional
      tck                : out std_ulogic;
      tdi                : out std_ulogic;
      tdo                : in  std_ulogic;
      tms                : out std_ulogic; -- optional
      virtual_state_cdr  : out std_ulogic;
      virtual_state_cir  : out std_ulogic;
      virtual_state_e1dr : out std_ulogic;
      virtual_state_e2dr : out std_ulogic;
      virtual_state_pdr  : out std_ulogic;
      virtual_state_sdr  : out std_ulogic;
      virtual_state_udr  : out std_ulogic;
      virtual_state_uir  : out std_ulogic
    );
  end component;

  signal vji_ireg : std_ulogic_vector(4 downto 0);
  signal vji_tck_i, vji_tdi_i, vji_tdo_o : std_ulogic;
  signal vji_state_cdr, vji_state_sdr, vji_state_udr : std_ulogic;

  -- TAP data registers --
  constant addr_idcode_c : std_ulogic_vector(4 downto 0) := "00001";
  constant addr_dtmcs_c  : std_ulogic_vector(4 downto 0) := "10000";
  constant addr_dmi_c    : std_ulogic_vector(4 downto 0) := "10001";
  constant addr_bypass_c : std_ulogic_vector(4 downto 0) := "11111";
  --
  constant size_idcode_c : natural := 32;
  constant size_dtmcs_c  : natural := 32;
  constant size_dmi_c    : natural := 7+32+2; -- 7-bit address + 32-bit data + 2-bit operation/status
  constant size_bypass_c : natural := 1;

  -- DR register --
  signal dreg : std_ulogic_vector(size_dmi_c-1 downto 0); -- max size (= dmi size)

  -- update CDC --
  signal state_udr_ff : std_ulogic_vector(2 downto 0);
  signal update : std_ulogic;

  -- misc --
  signal dmihardreset, dmireset : std_ulogic;

  -- debug module interface controller --
  signal dmi : dmi_req_t;
  signal busy, err : std_ulogic;

begin

  jtag_tdo_o <= '0'; -- unused

  -- vJTAG Instance -------------------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  vji_inst : sld_virtual_jtag
    generic map (
      SLD_AUTO_INSTANCE_INDEX => "YES",
      SLD_IR_WIDTH            => 5
    )
    port map (
      ir_in              => vji_ireg,
      ir_out             => addr_idcode_c,
      tck                => vji_tck_i,
      tdi                => vji_tdi_i,
      tdo                => vji_tdo_o,
      virtual_state_cdr  => vji_state_cdr,
      virtual_state_cir  => open,
      virtual_state_e1dr => open,
      virtual_state_e2dr => open,
      virtual_state_pdr  => open,
      virtual_state_sdr  => vji_state_sdr,
      virtual_state_udr  => vji_state_udr,
      virtual_state_uir  => open
    );


  -- Tap Register Access --------------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  reg_access: process(rstn_i, vji_tck_i)
  begin
    if (rstn_i = '0') then
      dreg      <= (others => '0');
      -- vji_tdo_o <= '0';
    elsif rising_edge(vji_tck_i) then
      -- data register input --
      if (vji_state_cdr = '1') then -- capture phase
        case vji_ireg is -- [NOTE] make data MSB-aligned and fill with zeros
          when addr_idcode_c => dreg <= IDCODE_VERSION & IDCODE_PARTID & IDCODE_MANID & '1' & "000000000";
          when addr_dtmcs_c  => dreg <= x"00000071" & "000000000";
          when addr_dmi_c    => dreg <= dmi.addr & dmi.data & err & err;
          when others        => dreg <= (others => '0');
        end case;
      elsif (vji_state_sdr = '1') then -- access phase; [JTAG-SYNC] evaluate TDI on rising edge of TCK
        dreg <= vji_tdi_i & dreg(dreg'left downto 1);
      end if;
    end if;
  end process reg_access;

  -- output --
  with vji_ireg select
    vji_tdo_o <= -- data is MSB-aligned so select the logical LSB as output
      dreg(dreg'left-(size_idcode_c-1)) when addr_idcode_c,
      dreg(dreg'left-(size_dtmcs_c-1))  when addr_dtmcs_c,
      dreg(dreg'left-(size_dmi_c-1))    when addr_dmi_c,
      dreg(dreg'left-(size_bypass_c-1)) when others;


  -- Update Trigger CDC ---------------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  trg_cdc: process(rstn_i, clk_i)
  begin
    if (rstn_i = '0') then
      state_udr_ff <= (others => '0');
    elsif rising_edge(clk_i) then
      state_udr_ff <= state_udr_ff(1 downto 0) & vji_state_udr;
    end if;
  end process trg_cdc;

  -- DR_UPDATE edge detector --
  update <= '1' when (state_udr_ff(1) = '1') and (state_udr_ff(2) = '0') else '0';


  -- reset control; [NOTE] dreg bits are LSB-aligned --
  dmihardreset <= '1' when (update = '1') and (vji_ireg = addr_dtmcs_c) and (dreg((dreg'left - (size_dtmcs_c-1)) + 17) = '1') else '0';
  dmireset     <= '1' when (update = '1') and (vji_ireg = addr_dtmcs_c) and (dreg((dreg'left - (size_dtmcs_c-1)) + 16) = '1') else '0';


  -- Debug Module Interface -----------------------------------------------------------------
  -- -------------------------------------------------------------------------------------------
  dmi_controller: process(rstn_i, clk_i)
  begin
    if (rstn_i = '0') then
      err  <= '0';
      busy <= '0';
      dmi  <= dmi_req_terminate_c;
    elsif rising_edge(clk_i) then
      -- sticky error: access attempt while DMI is busy --
      if (dmireset = '1') or (dmihardreset = '1') then
        err <= '0';
      elsif (update = '1') and (vji_ireg = addr_dmi_c) and (busy = '1') then
        err <= '1';
      end if;
      -- interface arbiter --
      dmi.op <= dmi_req_nop_c; -- default
      if (busy = '0') then -- idle: waiting for new request
        if (update = '1') and (vji_ireg = addr_dmi_c) then
          dmi.addr <= dreg(40 downto 34);
          dmi.data <= dreg(33 downto 2);
          dmi.op   <= dreg(1 downto 0);
          busy     <= or_reduce_f(dreg(1 downto 0));
        end if;
      elsif (dmi_rsp_i.ack = '1') or (dmihardreset = '1') then -- busy: wait for access termination
        dmi.data <= dmi_rsp_i.data;
        busy     <= '0';
      end if;
    end if;
  end process dmi_controller;

  -- DMI output --
  dmi_req_o <= dmi;


end neorv32_debug_dtm_altera_sld_rtl;
