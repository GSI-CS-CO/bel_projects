--==============================================================================
-- GSI Helmholz center for Heavy Ion Research GmbH
-- Package file for Wishbone SERDES clock generator
--==============================================================================
--
-- author: Theodor Stana (t.stana@gsi.de)
--
-- date of creation: 2015-03-25
--
-- version: 1.0
--
-- description:
--    Package file for the Wishbone SERDES clock generator, declares components
--    and the SDB descriptor for the module.
--
--==============================================================================
-- GNU LESSER GENERAL PUBLIC LICENSE
--==============================================================================
-- This source file is free software; you can redistribute it and/or modify it
-- under the terms of the GNU Lesser General Public License as published by the
-- Free Software Foundation; either version 2.1 of the License, or (at your
-- option) any later version. This source is distributed in the hope that it
-- will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
-- of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
-- See the GNU Lesser General Public License for more details. You should have
-- received a copy of the GNU Lesser General Public License along with this
-- source; if not, download it from http://www.gnu.org/licenses/lgpl-2.1.html
--==============================================================================
-- last changes:
--    2015-03-25   Theodor Stana     File created
--==============================================================================
-- TODO: -
--==============================================================================

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.genram_pkg.all;
use work.altera_lvds_pkg.all;


package wb_serdes_clk_gen_pkg is

  --============================================================================
  -- Constant declarations
  --============================================================================
  constant c_wb_serdes_clk_gen_sdb : t_sdb_device := (
    abi_class     => x"0000",              -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"00",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7",                 -- 8/16/32-bit port granularity
    sdb_component => (
      addr_first  => x"0000000000000000",
      addr_last   => x"00000000000000ff",
      product     => (
        vendor_id => x"0000000000000651",  -- GSI
        device_id => x"5f3eaf43",          -- echo "wb_serdes_clk_gen  " | md5sum | cut -c1-8
        version   => x"00000001",
        date      => x"20150325",
        name      => "wb_serdes_clk_gen  "))
   );

  --============================================================================
  -- Component declarations
  --============================================================================
  ------------------------------------------------------------------------------
  -- WB clock generator, detailed interface
  ------------------------------------------------------------------------------
  component wb_serdes_clk_gen is
    generic
    (
      -- Number of SERDES outputs to connect to
      g_num_outputs           : natural;

      -- Number of bits for the SERDES channels (all g_num_outputs of them)
      g_num_serdes_bits       : natural;

      -- Enable fractional division ratios
      g_with_frac_counter     : boolean := false;

      -- Enable selectable duty cycles. If set to FALSE, the period at the SERDES
      -- output will be double the period set in the registers and the duty cycle
      -- will be 50/50.
      g_selectable_duty_cycle : boolean := false;

      -- Enable synchronization of clocks of the same frequency. Needs White
      -- Rabbit connection.
      -- FIXME: FALSE DOESN'T WORK.
      g_with_sync             : boolean := true
    );
    port
    (
      ---------------------------------------------------------------------------
      -- Ports in clk_sys_i domain
      ---------------------------------------------------------------------------
      clk_sys_i    : in  std_logic;
      rst_sys_n_i  : in  std_logic;

      wb_adr_i     : in  std_logic_vector( 2 downto 0);
      wb_dat_i     : in  std_logic_vector(31 downto 0);
      wb_dat_o     : out std_logic_vector(31 downto 0);
      wb_cyc_i     : in  std_logic;
      wb_sel_i     : in  std_logic_vector(3 downto 0);
      wb_stb_i     : in  std_logic;
      wb_we_i      : in  std_logic;
      wb_ack_o     : out std_logic;
      wb_stall_o   : out std_logic;

      ---------------------------------------------------------------------------
      -- Ports in clk_ref_i domain
      ---------------------------------------------------------------------------
      clk_ref_i    : in  std_logic;
      rst_ref_n_i  : in  std_logic;

      eca_time_i   : in std_logic_vector(63 downto 0);

      serdes_dat_o : out t_lvds_byte_array
    );
  end component wb_serdes_clk_gen;

  ------------------------------------------------------------------------------
  -- WB clock generator, interface with VHDL record
  ------------------------------------------------------------------------------
  component xwb_serdes_clk_gen is
    generic
    (
      -- Number of SERDES outputs to connect to
      g_num_outputs           : natural;

      -- Number of bits for the SERDES channels (all g_num_outputs of them)
      g_num_serdes_bits       : natural;

      -- Enable fractional division ratios
      g_with_frac_counter     : boolean := false;

      -- Enable selectable duty cycles. If set to FALSE, the period at the SERDES
      -- output will be double the period set in the registers and the duty cycle
      -- will be 50/50.
      g_selectable_duty_cycle : boolean := false;

      -- Enable synchronization of clocks of the same frequency. Needs White
      -- Rabbit connection.
      -- FIXME: FALSE DOESN'T WORK.
      g_with_sync             : boolean := true
    );
    port
    (
      --------------------------------------------------------------------------
      -- Ports in clk_sys_i domain
      --------------------------------------------------------------------------
      clk_sys_i    : in  std_logic;
      rst_sys_n_i  : in  std_logic;

      wbs_i        : in  t_wishbone_slave_in;
      wbs_o        : out t_wishbone_slave_out;

      --------------------------------------------------------------------------
      -- Ports in clk_ref_i domain
      --------------------------------------------------------------------------
      clk_ref_i    : in  std_logic;
      rst_ref_n_i  : in  std_logic;

      eca_time_i   : in std_logic_vector(63 downto 0);

      serdes_dat_o : out t_lvds_byte_array
    );
  end component xwb_serdes_clk_gen;

end package wb_serdes_clk_gen_pkg;

