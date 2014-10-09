--! @file monster_pkg.vhd
--! @brief Monster (all your top are belong to BEL) package
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
---------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;

package monster_pkg is

  function f_sub1(x : natural) return natural;
  function f_pick(x : boolean; y : integer; z : integer) return natural;

  component monster is
    generic(
      g_family      : string; -- "Arria II" or "Arria V"
      g_project     : string;
      g_flash_bits  : natural;
      g_ram_size    : natural := 131072;
      g_gpio_inout  : natural := 0;
      g_gpio_in     : natural := 0;
      g_gpio_out    : natural := 0;
      g_tlu_fifo_size : natural := 256;
      g_lvds_inout  : natural := 0;
      g_lvds_in     : natural := 0;
      g_lvds_out    : natural := 0;
      g_lvds_invert : boolean := false;
      g_en_pcie     : boolean := false;
      g_en_vme      : boolean := false;
      g_en_usb      : boolean := false;
      g_en_scubus   : boolean := false;
      g_en_mil      : boolean := false;
      g_en_oled     : boolean := false;
      g_en_lcd      : boolean := false;
      g_en_cfi      : boolean := false;
      g_en_ssd1325  : boolean := false;
      g_en_user_ow  : boolean := false;
      g_en_fg       : boolean := false;  
      g_lm32_cores           : natural := 1;
      g_lm32_MSIs            : natural := 1;
      g_lm32_ramsizes        : natural := 131072/4;
      g_lm32_shared_ramsize  : natural := 16384/4; -- will only be used if g_lm32_cores > 1
      g_lm32_are_ftm         : boolean := false
    );
    port(
      -- Required: core signals
      core_clk_20m_vcxo_i    : in    std_logic;
      core_clk_125m_sfpref_i : in    std_logic;
      core_clk_125m_pllref_i : in    std_logic;
      core_clk_125m_local_i  : in    std_logic;
      core_rstn_i            : in    std_logic := '1';
      -- Optional clock outputs
      core_clk_wr_ref_o      : out   std_logic;
      core_clk_butis_o       : out   std_logic;
      core_clk_butis_t0_o    : out   std_logic;
      core_rstn_wr_ref_o     : out   std_logic;
      core_rstn_butis_o      : out   std_logic;
      core_debug_o           : out   std_logic_vector(15 downto 0);
      -- Required: white rabbit pins
      wr_onewire_io          : inout std_logic;
      wr_sfp_sda_io          : inout std_logic;
      wr_sfp_scl_io          : inout std_logic;
      wr_sfp_det_i           : in    std_logic;
      wr_sfp_tx_o            : out   std_logic;
      wr_sfp_rx_i            : in    std_logic;
      wr_dac_sclk_o          : out   std_logic;
      wr_dac_din_o           : out   std_logic;
      wr_ndac_cs_o           : out   std_logic_vector(2 downto 1);
      -- Optional WR features
      wr_ext_clk_i           : in    std_logic := '0'; -- 10MHz
      wr_ext_pps_i           : in    std_logic := '0';
      wr_uart_o              : out   std_logic;
      wr_uart_i              : in    std_logic := '0';
      -- GPIO for the board (inouts start at 0, dedicated in/outs come after)
      gpio_i                 : in    std_logic_vector(f_sub1(g_gpio_inout+g_gpio_in)  downto 0) := (others => '0');
      gpio_o                 : out   std_logic_vector(f_sub1(g_gpio_inout+g_gpio_out) downto 0);
      gpio_oen_o             : out   std_logic_vector(f_sub1(g_gpio_inout)            downto 0);
      -- LVDS for the board (inouts start at 0, dedicated in/outs come after)
      lvds_p_i               : in    std_logic_vector(f_sub1(g_lvds_inout+g_lvds_in)  downto 0) := (others => '0');
      lvds_n_i               : in    std_logic_vector(f_sub1(g_lvds_inout+g_lvds_in)  downto 0) := (others => '0');
      lvds_i_led_o           : out   std_logic_vector(f_sub1(g_lvds_inout+g_lvds_in)  downto 0);
      lvds_p_o               : out   std_logic_vector(f_sub1(g_lvds_inout+g_lvds_out) downto 0);
      lvds_n_o               : out   std_logic_vector(f_sub1(g_lvds_inout+g_lvds_out) downto 0);
      lvds_o_led_o           : out   std_logic_vector(f_sub1(g_lvds_inout+g_lvds_out) downto 0);
      lvds_oen_o             : out   std_logic_vector(f_sub1(g_lvds_inout)            downto 0);
      -- Optional status LEDs
      led_link_up_o          : out   std_logic;
      led_link_act_o         : out   std_logic;
      led_track_o            : out   std_logic;
      led_pps_o              : out   std_logic;
      -- g_en_pcie
      pcie_refclk_i          : in    std_logic := '0';
      pcie_rstn_i            : in    std_logic := '0';
      pcie_rx_i              : in    std_logic_vector(3 downto 0) := (others => '0');
      pcie_tx_o              : out   std_logic_Vector(3 downto 0);
      -- g_en_vme
      vme_as_n_i             : in    std_logic := '0';
      vme_rst_n_i            : in    std_logic := '0';
      vme_write_n_i          : in    std_logic := '1';
      vme_am_i               : in    std_logic_vector(5 downto 0) := (others => '0');
      vme_ds_n_i             : in    std_logic_vector(1 downto 0) := (others => '1');
      vme_ga_i               : in    std_logic_vector(3 downto 0) := (others => '0');
      vme_addr_data_b        : inout std_logic_vector(31 downto 0) := (others => 'Z');
      vme_iack_n_i           : in    std_logic := '1';
      vme_iackin_n_i         : in    std_logic := '1';
      vme_iackout_n_o        : out   std_logic;
      vme_irq_n_o            : out   std_logic_vector(6 downto 0);
      vme_berr_o             : out   std_logic;
      vme_dtack_oe_o         : out   std_logic;
      vme_buffer_latch_o     : out   std_logic_vector(3 downto 0);
      vme_data_oe_ab_o       : out   std_logic;
      vme_data_oe_ba_o       : out   std_logic;
      vme_addr_oe_ab_o       : out   std_logic;
      vme_addr_oe_ba_o       : out   std_logic;
      -- g_en_usb
      usb_rstn_o             : out   std_logic;
      usb_ebcyc_i            : in    std_logic := '0';
      usb_speed_i            : in    std_logic := '0';
      usb_shift_i            : in    std_logic := '0';
      usb_readyn_io          : inout std_logic := 'Z';
      usb_fifoadr_o          : out   std_logic_vector(1 downto 0);
      usb_sloen_o            : out   std_logic;
      usb_fulln_i            : in    std_logic := '1';
      usb_emptyn_i           : in    std_logic := '0';
      usb_slrdn_o            : out   std_logic;
      usb_slwrn_o            : out   std_logic;
      usb_pktendn_o          : out   std_logic;
      usb_fd_io              : inout std_logic_vector(7 downto 0) := (others => 'Z');
      -- g_en_scubus
      scubus_a_a             : out   std_logic_vector(15 downto 0);
      scubus_a_d             : inout std_logic_vector(15 downto 0) := (others => 'Z');
      scubus_nsel_data_drv   : out   std_logic;
      scubus_a_nds           : out   std_logic;
      scubus_a_rnw           : out   std_logic;
      scubus_a_ndtack        : in    std_logic := '1';
      scubus_a_nsrq          : in    std_logic_vector(12 downto 1) := (others => '1');
      scubus_a_nsel          : out   std_logic_vector(12 downto 1);
      scubus_a_ntiming_cycle : out   std_logic;
      scubus_a_sysclock      : out   std_logic;
      -- g_en_mil
      mil_nme_boo_i          : in    std_logic := '0';
      mil_nme_bzo_i          : in    std_logic := '0';
      mil_me_sd_i            : in    std_logic := '0';
      mil_me_esc_i           : in    std_logic := '0';
      mil_me_sdi_o           : out   std_logic;
      mil_me_ee_o            : out   std_logic;
      mil_me_ss_o            : out   std_logic;
      mil_me_boi_o           : out   std_logic;
      mil_me_bzi_o           : out   std_logic;
      mil_me_udi_o           : out   std_logic;
      mil_me_cds_i           : in    std_logic := '0';
      mil_me_sdo_i           : in    std_logic := '0';
      mil_me_dsc_i           : in    std_logic := '0';
      mil_me_vw_i            : in    std_logic := '0';
      mil_me_td_i            : in    std_logic := '0';
      mil_me_12mhz_o         : out   std_logic;
      mil_boi_i              : in    std_logic := '0';
      mil_bzi_i              : in    std_logic := '0';
      mil_sel_drv_o          : out   std_logic;
      mil_nsel_rcv_o         : out   std_logic;
      mil_nboo_o             : out   std_logic;
      mil_nbzo_o             : out   std_logic;
      mil_nled_rcv_o         : out   std_logic;
      mil_nled_trm_o         : out   std_logic;
      mil_nled_err_o         : out   std_logic;
      mil_timing_i           : in    std_logic := '0';
      mil_nled_timing_o      : out   std_logic;
      mil_nled_fifo_ne_o     : out   std_logic;
      mil_interlock_intr_i   : in    std_logic := '0';
      mil_data_rdy_intr_i    : in    std_logic := '0';
      mil_data_req_intr_i    : in    std_logic := '0';
      mil_nled_interl_o      : out   std_logic;
      mil_nled_dry_o         : out   std_logic;
      mil_nled_drq_o         : out   std_logic;
      mil_io1_o              : out   std_logic;
      mil_io1_is_in_o        : out   std_logic;
      mil_nled_io1_o         : out   std_logic;
      mil_io2_o              : out   std_logic;
      mil_io2_is_in_o        : out   std_logic;
      mil_nled_io2_o         : out   std_logic;
      -- g_en_oled
      oled_rstn_o            : out   std_logic;
      oled_dc_o              : out   std_logic;
      oled_ss_o              : out   std_logic;
      oled_sck_o             : out   std_logic;
      oled_sd_o              : out   std_logic;
      oled_sh_vr_o           : out   std_logic;
      -- g_en_lcd
      lcd_scp_o              : out   std_logic;
      lcd_lp_o               : out   std_logic;
      lcd_flm_o              : out   std_logic;
      lcd_in_o               : out   std_logic;
	   -- g_en_ssd1325
	   ssd1325_rst_o          : out   std_logic;
	   ssd1325_dc_o           : out   std_logic;
	   ssd1325_ss_o           : out   std_logic;
	   ssd1325_sclk_o         : out   std_logic;
	   ssd1325_data_o         : out   std_logic;
		-- g_en_cfi
      cfi_ad                 : out   std_logic_vector(25 downto 1);
      cfi_df                 : inout std_logic_vector(15 downto 0) := (others => 'Z');
      cfi_adv_fsh            : out   std_logic ;
      cfi_nce_fsh            : out   std_logic ;
      cfi_clk_fsh            : out   std_logic ;
      cfi_nwe_fsh            : out   std_logic ;
      cfi_noe_fsh            : out   std_logic ;
      cfi_nrst_fsh           : out   std_logic ;
      cfi_wait_fsh           : in    std_logic := '0';

      -- g_en_user_ow
      ow_io                  : inout std_logic_vector(1 downto 0));
  end component;

  constant c_iodir_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"00",
    abi_ver_minor => x"00",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"000000000000000f",
    product => (
    vendor_id     => x"0000000000000651",
    device_id     => x"4d78adfd",
    version       => x"00000001",
    date          => x"20140516",
    name          => "GSI:IODIR_HACK     ")));
  
  component monster_iodir is
    generic(
      g_gpio_inout : natural := 0;
      g_lvds_inout : natural := 0);
    port(
      clk_i      : in  std_logic;
      rst_n_i    : in  std_logic;
      slave_i    : in  t_wishbone_slave_in;
      slave_o    : out t_wishbone_slave_out;
      gpio_oen_o : out std_logic_vector(f_sub1(g_gpio_inout) downto 0);
      lvds_oen_o : out std_logic_vector(f_sub1(g_lvds_inout) downto 0));
  end component;

end package;

package body monster_pkg is

  function f_sub1(x : natural) return natural is
  begin
    if x = 0
    then return 0;
    else return x-1;
    end if;
  end f_sub1;

  function f_pick(x : boolean; y : integer; z : integer) return natural is
  begin
    if x
    then return y;
    else return z;
    end if;
  end f_pick;

end monster_pkg;
