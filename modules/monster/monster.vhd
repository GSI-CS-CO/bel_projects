--! @file monster_pkg.vhd
--! @brief Monster (all your top are belong to BEL) entity
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
use work.gencores_pkg.all;
use work.wrcore_pkg.all;
use work.pll_pkg.all;
use work.monster_pkg.all;
use work.wr_fabric_pkg.all;
use work.wishbone_pkg.all;
use work.eca_pkg.all;
use work.tlu_pkg.all;
use work.pcie_wb_pkg.all;
use work.wr_altera_pkg.all;
use work.etherbone_pkg.all;
use work.scu_bus_pkg.all;
use work.altera_flash_pkg.all;
use work.altera_networks_pkg.all;
use work.altera_lvds_pkg.all;
use work.build_id_pkg.all;
use work.watchdog_pkg.all;
use work.mbox_pkg.all;
use work.oled_display_pkg.all;
use work.lpc_uart_pkg.all;
use work.wb_irq_pkg.all;
use work.ftm_pkg.all;
use work.ez_usb_pkg.all;
use work.wb_arria_reset_pkg.all;
use work.xvme64x_pack.all;
use work.VME_Buffer_pack.all;
use work.wb_mil_scu_pkg.all;
use work.wr_serialtimestamp_pkg.all;
use work.wb_ssd1325_serial_driver_pkg.all;
use work.wb_nau8811_audio_driver_pkg.all;
use work.fg_quad_pkg.all;
use work.cfi_flash_pkg.all;
use work.psram_pkg.all;
use work.wb_serdes_clk_gen_pkg.all;
use work.io_control_pkg.all;
use work.wb_pmc_host_bridge_pkg.all;

entity monster is
  generic(
    g_family               : string; -- "Arria II" or "Arria V"
    g_project              : string;
    g_flash_bits           : natural;
    g_psram_bits           : natural;
    g_ram_size             : natural;
    g_gpio_inout           : natural;
    g_gpio_in              : natural;
    g_gpio_out             : natural;
    g_tlu_fifo_size        : natural;
    g_lvds_inout           : natural;
    g_lvds_in              : natural;
    g_lvds_out             : natural;
    g_fixed                : natural;
    g_lvds_invert          : boolean;
    g_en_pcie              : boolean;
    g_en_vme               : boolean;
    g_en_usb               : boolean;
    g_en_scubus            : boolean;
    g_en_mil               : boolean;
    g_en_oled              : boolean;
    g_en_lcd               : boolean;
    g_en_cfi               : boolean;
    g_en_ssd1325           : boolean;
    g_en_nau8811           : boolean;
    g_en_user_ow           : boolean;
    g_en_psram             : boolean;
    g_io_table             : t_io_mapping_table_arg_array(natural range <>);
    g_en_pmc               : boolean;
    g_lm32_cores           : natural;
    g_lm32_MSIs            : natural;
    g_lm32_ramsizes        : natural;
    g_lm32_init_files      : string;
	 g_lm32_profiles        : string;
    g_lm32_are_ftm         : boolean);
  port(
    -- Required: core signals
    core_clk_20m_vcxo_i    : in    std_logic;
    core_clk_125m_pllref_i : in    std_logic;
    core_clk_125m_sfpref_i : in    std_logic;
    core_clk_125m_local_i  : in    std_logic;
    core_rstn_i            : in    std_logic;
    -- Optional clock outputs
    core_clk_wr_ref_o      : out   std_logic;
    core_clk_butis_o       : out   std_logic;
    core_clk_butis_t0_o    : out   std_logic;
    core_rstn_wr_ref_o     : out   std_logic;
    core_rstn_butis_o      : out   std_logic;
    core_debug_o           : out   std_logic_vector(15 downto 0) := (others => 'Z');
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
    wr_ext_clk_i           : in    std_logic; -- 10MHz
    wr_ext_pps_i           : in    std_logic;
    wr_uart_o              : out   std_logic;
    wr_uart_i              : in    std_logic;
    -- GPIO for the board
    gpio_i                 : in    std_logic_vector(f_sub1(g_gpio_inout+g_gpio_in)  downto 0);
    gpio_o                 : out   std_logic_vector(f_sub1(g_gpio_inout+g_gpio_out) downto 0) := (others => 'Z');
    gpio_oen_o             : out   std_logic_vector(f_sub1(g_gpio_inout+g_gpio_out) downto 0) := (others => '0');
    gpio_term_o            : out   std_logic_vector(f_sub1(g_gpio_inout+g_gpio_in)  downto 0) := (others => '1');
    gpio_spec_in_o         : out   std_logic_vector(f_sub1(g_gpio_inout+g_gpio_in)  downto 0) := (others => '0');
    gpio_spec_out_o        : out   std_logic_vector(f_sub1(g_gpio_inout+g_gpio_out) downto 0) := (others => '0');
    -- LVDS for the board
    lvds_p_i               : in    std_logic_vector(f_sub1(g_lvds_inout+g_lvds_in)  downto 0);
    lvds_n_i               : in    std_logic_vector(f_sub1(g_lvds_inout+g_lvds_in)  downto 0);
    lvds_i_led_o           : out   std_logic_vector(f_sub1(g_lvds_inout+g_lvds_in)  downto 0) := (others => 'Z');
    lvds_p_o               : out   std_logic_vector(f_sub1(g_lvds_inout+g_lvds_out) downto 0) := (others => 'Z');
    lvds_n_o               : out   std_logic_vector(f_sub1(g_lvds_inout+g_lvds_out) downto 0) := (others => 'Z');
    lvds_o_led_o           : out   std_logic_vector(f_sub1(g_lvds_inout+g_lvds_out) downto 0) := (others => 'Z');
    lvds_oen_o             : out   std_logic_vector(f_sub1(g_lvds_inout+g_lvds_out) downto 0) := (others => '0');
    lvds_term_o            : out   std_logic_vector(f_sub1(g_lvds_inout+g_lvds_in)  downto 0) := (others => '1');
    lvds_spec_in_o         : out   std_logic_vector(f_sub1(g_lvds_inout+g_lvds_in)  downto 0) := (others => '0');
    lvds_spec_out_o        : out   std_logic_vector(f_sub1(g_lvds_inout+g_lvds_out) downto 0) := (others => '0');
    -- Optional status LEDs
    led_link_up_o          : out   std_logic;
    led_link_act_o         : out   std_logic;
    led_track_o            : out   std_logic;
    led_pps_o              : out   std_logic;
    -- g_en_pcie
    pcie_refclk_i          : in    std_logic;
    pcie_rstn_i            : in    std_logic;
    pcie_rx_i              : in    std_logic_vector(3 downto 0);
    pcie_tx_o              : out   std_logic_Vector(3 downto 0) := (others => 'Z');
    -- g_en_vme
    vme_as_n_i             : in    std_logic;
    vme_rst_n_i            : in    std_logic;
    vme_write_n_i          : in    std_logic;
    vme_am_i               : in    std_logic_vector(5 downto 0);
    vme_ds_n_i             : in    std_logic_vector(1 downto 0);
    vme_ga_i               : in    std_logic_vector(3 downto 0);
    vme_addr_data_b        : inout std_logic_vector(31 downto 0);
    vme_iack_n_i           : in    std_logic;
    vme_iackin_n_i         : in    std_logic;
    vme_iackout_n_o        : out   std_logic := 'Z';
    vme_irq_n_o            : out   std_logic_vector(6 downto 0) := (others => 'Z');
    vme_berr_o             : out   std_logic := 'Z';
    vme_dtack_oe_o         : out   std_logic := 'Z';
    vme_buffer_latch_o     : out   std_logic_vector(3 downto 0) := (others => 'Z');
    vme_data_oe_ab_o       : out   std_logic := 'Z';
    vme_data_oe_ba_o       : out   std_logic := 'Z';
    vme_addr_oe_ab_o       : out   std_logic := 'Z';
    vme_addr_oe_ba_o       : out   std_logic := 'Z';
    -- g_en_usb
    usb_rstn_o             : out   std_logic := 'Z';
    usb_ebcyc_i            : in    std_logic;
    usb_speed_i            : in    std_logic;
    usb_shift_i            : in    std_logic;
    usb_readyn_io          : inout std_logic;
    usb_fifoadr_o          : out   std_logic_vector(1 downto 0) := (others => 'Z');
    usb_sloen_o            : out   std_logic := 'Z';
    usb_fulln_i            : in    std_logic;
    usb_emptyn_i           : in    std_logic;
    usb_slrdn_o            : out   std_logic := 'Z';
    usb_slwrn_o            : out   std_logic := 'Z';
    usb_pktendn_o          : out   std_logic := 'Z';
    usb_fd_io              : inout std_logic_vector(7 downto 0);
    -- g_en_scubus
    scubus_a_a             : out   std_logic_vector(15 downto 0) := (others => 'Z');
    scubus_a_d             : inout std_logic_vector(15 downto 0);
    scubus_nsel_data_drv   : out   std_logic := 'Z';
    scubus_a_nds           : out   std_logic := 'Z';
    scubus_a_rnw           : out   std_logic := 'Z';
    scubus_a_ndtack        : in    std_logic;
    scubus_a_nsrq          : in    std_logic_vector(12 downto 1);
    scubus_a_nsel          : out   std_logic_vector(12 downto 1) := (others => 'Z');
    scubus_a_ntiming_cycle : out   std_logic := 'Z';
    scubus_a_sysclock      : out   std_logic := 'Z';
    -- g_en_mil
    mil_nme_boo_i          : in    std_logic;
    mil_nme_bzo_i          : in    std_logic;
    mil_me_sd_i            : in    std_logic;
    mil_me_esc_i           : in    std_logic;
    mil_me_sdi_o           : out   std_logic := 'Z';
    mil_me_ee_o            : out   std_logic := 'Z';
    mil_me_ss_o            : out   std_logic := 'Z';
    mil_me_boi_o           : out   std_logic := 'Z';
    mil_me_bzi_o           : out   std_logic := 'Z';
    mil_me_udi_o           : out   std_logic := 'Z';
    mil_me_cds_i           : in    std_logic;
    mil_me_sdo_i           : in    std_logic;
    mil_me_dsc_i           : in    std_logic;
    mil_me_vw_i            : in    std_logic;
    mil_me_td_i            : in    std_logic;
    mil_me_12mhz_o         : out   std_logic := 'Z';
    mil_boi_i              : in    std_logic;
    mil_bzi_i              : in    std_logic;
    mil_sel_drv_o          : out   std_logic := 'Z';
    mil_nsel_rcv_o         : out   std_logic := 'Z';
    mil_nboo_o             : out   std_logic := 'Z';
    mil_nbzo_o             : out   std_logic := 'Z';
    mil_nled_rcv_o         : out   std_logic := 'Z';
    mil_nled_trm_o         : out   std_logic := 'Z';
    mil_nled_err_o         : out   std_logic := 'Z';
    mil_timing_i           : in    std_logic;
    mil_nled_timing_o      : out   std_logic := 'Z';
    mil_nled_fifo_ne_o     : out   std_logic := 'Z';
    mil_interlock_intr_i   : in    std_logic;
    mil_data_rdy_intr_i    : in    std_logic;
    mil_data_req_intr_i    : in    std_logic;
    mil_nled_interl_o      : out   std_logic := 'Z';
    mil_nled_dry_o         : out   std_logic := 'Z';
    mil_nled_drq_o         : out   std_logic := 'Z';
	  mil_lemo_data_o        : out   std_logic_vector(4 downto 1);
    mil_lemo_nled_o        : out   std_logic_vector(4 downto 1);
    mil_lemo_out_en_o      : out   std_logic_vector(4 downto 1);
    mil_lemo_data_i        : in    std_logic_vector(4 downto 1):= (others => '0');
	 
--    mil_io1_o              : out   std_logic := 'Z';
--    mil_io1_is_in_o        : out   std_logic := 'Z';
--    mil_nled_io1_o         : out   std_logic := 'Z';
--    mil_io2_o              : out   std_logic := 'Z';
--    mil_io2_is_in_o        : out   std_logic := 'Z';
--    mil_nled_io2_o         : out   std_logic := 'Z';
    -- g_en_oled
    oled_rstn_o            : out   std_logic := 'Z';
    oled_dc_o              : out   std_logic := 'Z';
    oled_ss_o              : out   std_logic := 'Z';
    oled_sck_o             : out   std_logic := 'Z';
    oled_sd_o              : out   std_logic := 'Z';
    oled_sh_vr_o           : out   std_logic := 'Z';
    -- g_en_lcd
    lcd_scp_o              : out   std_logic := 'Z';
    lcd_lp_o               : out   std_logic := 'Z';
    lcd_flm_o              : out   std_logic := 'Z';
    lcd_in_o               : out   std_logic := 'Z';
    -- g_en_ssd1325
    ssd1325_rst_o          : out   std_logic := 'Z';
    ssd1325_dc_o           : out   std_logic := 'Z';
    ssd1325_ss_o           : out   std_logic := 'Z';
    ssd1325_sclk_o         : out   std_logic := 'Z';
    ssd1325_data_o         : out   std_logic := 'Z';
    -- g_en_nau8811
    nau8811_spi_csb_o      : out   std_logic := 'Z';
    nau8811_spi_sclk_o     : out   std_logic := 'Z';
    nau8811_spi_sdio_o     : out   std_logic := 'Z';
    nau8811_iis_fs_o       : out   std_logic := 'Z';
    nau8811_iis_bclk_o     : out   std_logic := 'Z';
    nau8811_iis_adcout_o   : out   std_logic := 'Z';
    nau8811_iis_dacin_i    : in    std_logic;
    -- g_en_cfi
    cfi_ad                 : out   std_logic_vector(25 downto 1) := (others => 'Z');
    cfi_df                 : inout std_logic_vector(15 downto 0);
    cfi_adv_fsh            : out   std_logic := 'Z';
    cfi_nce_fsh            : out   std_logic := 'Z';
    cfi_clk_fsh            : out   std_logic := 'Z';
    cfi_nwe_fsh            : out   std_logic := 'Z';
    cfi_noe_fsh            : out   std_logic := 'Z';
    cfi_nrst_fsh           : out   std_logic := 'Z';
    cfi_wait_fsh           : in    std_logic;
    -- g_en_psram
    ps_clk                 : out   std_logic := 'Z';
    ps_addr                : out   std_logic_vector(g_psram_bits-1 downto 0) := (others => 'Z');
    ps_data                : inout std_logic_vector(15 downto 0);
    ps_seln                : out   std_logic_vector(1 downto 0) := (others => 'Z');
    ps_cen                 : out   std_logic := 'Z';
    ps_oen                 : out   std_logic := 'Z';
    ps_wen                 : out   std_logic := 'Z';
    ps_cre                 : out   std_logic := 'Z';
    ps_advn                : out   std_logic := 'Z';
    ps_wait                : in    std_logic;
    
    -- g_en_pmc
    pmc_pci_clk_i          : in    std_logic;
    pmc_pci_rst_i          : in    std_logic;
    pmc_buf_oe_o           : out   std_logic := 'Z';
    pmc_busmode_io         : inout std_logic_vector(3 downto 0);
    pmc_ad_io              : inout std_logic_vector(31 downto 0);
    pmc_c_be_io            : inout std_logic_vector(3 downto 0);
    pmc_par_io             : inout std_logic;
    pmc_frame_io           : inout std_logic;
    pmc_trdy_io            : inout std_logic;
    pmc_irdy_io            : inout std_logic;
    pmc_stop_io            : inout std_logic;
    pmc_devsel_io          : inout std_logic;
    pmc_idsel_i            : in    std_logic;
    pmc_perr_io            : inout std_logic;
    pmc_serr_io            : inout std_logic;
    pmc_inta_o             : out   std_logic := 'Z';
    pmc_req_o              : out   std_logic;
    pmc_gnt_i              : in    std_logic;

    -- g_en_user_ow
    ow_io                  : inout std_logic_vector(1 downto 0);
    hw_version             : in    std_logic_vector(31 downto 0));
end monster;

architecture rtl of monster is

  constant c_is_arria5 : boolean := g_family = "Arria V";
  constant c_is_arria2 : boolean := g_family = "Arria II";
  
  constant c_zero_master : t_wishbone_master_out := (
    cyc => '0',
    stb => '0',
    adr => (others => '0'),
    sel => (others => '0'),
    we  => '0',
    dat => (others => '0'));

  ----------------------------------------------------------------------------------
  -- GSI Top Crossbar Masters ------------------------------------------------------
  ----------------------------------------------------------------------------------
  
  constant c_top_my_masters : natural := 7;
  constant c_topm_ebs       : natural := 0;
  constant c_topm_eca_wbm   : natural := 1;
  constant c_topm_pcie      : natural := 2;
  constant c_topm_vme       : natural := 3;
  constant c_topm_usb       : natural := 4;
  constant c_topm_prioq     : natural := 5;
  constant c_topm_pmc       : natural := 6;
  
  constant c_top_layout_my_masters : t_sdb_record_array(c_top_my_masters-1 downto 0) :=
   (c_topm_ebs     => f_sdb_auto_msi(c_ebs_msi,     false),   -- Need to add MSI support !!!
    c_topm_eca_wbm => f_sdb_auto_msi(c_null_msi,    false),   -- no MSIs for ECA=>WB macro player
    c_topm_pcie    => f_sdb_auto_msi(c_pcie_msi,    g_en_pcie),
    c_topm_vme     => f_sdb_auto_msi(c_vme_msi,     g_en_vme),
    c_topm_usb     => f_sdb_auto_msi(c_usb_msi,     false), -- Need to add MSI support !!!
    c_topm_prioq   => f_sdb_auto_msi(c_null_msi,    false),
    c_topm_pmc     => f_sdb_auto_msi(c_pmc_msi,    g_en_pmc));  
  
  -- The FTM adds a bunch of masters to this crossbar
  constant c_ftm_masters : t_sdb_record_array := f_lm32_masters_bridge_msis(g_lm32_cores);
  constant c_top_masters : natural := c_ftm_masters'length + c_top_my_masters;
  constant c_top_layout_req_masters : t_sdb_record_array(c_top_masters-1 downto 0) :=
    c_ftm_masters & c_top_layout_my_masters;
    
  constant c_top_layout_masters : t_sdb_record_array := f_sdb_auto_layout(c_top_layout_req_masters);
  constant c_top_bridge_msi     : t_sdb_msi          := f_xwb_msi_layout_sdb(c_top_layout_masters);
  
  signal top_bus_slave_i  : t_wishbone_slave_in_array  (c_top_masters-1 downto 0);
  signal top_bus_slave_o  : t_wishbone_slave_out_array (c_top_masters-1 downto 0);
  signal top_msi_master_i : t_wishbone_master_in_array (c_top_masters-1 downto 0);
  signal top_msi_master_o : t_wishbone_master_out_array(c_top_masters-1 downto 0);
  
  ----------------------------------------------------------------------------------
  -- GSI Dev Crossbar Masters ------------------------------------------------------
  ----------------------------------------------------------------------------------
  constant c_dev_masters         : natural := 1;
  constant c_devm_top            : natural := 0;
  
  constant c_dev_layout_req_masters : t_sdb_record_array(c_dev_masters-1 downto 0) :=
    (c_devm_top => f_sdb_auto_msi(c_top_bridge_msi, true));
  constant c_dev_layout_masters : t_sdb_record_array := f_sdb_auto_layout(c_dev_layout_req_masters);
  constant c_dev_bridge_msi : t_sdb_msi := f_xwb_msi_layout_sdb(c_dev_layout_masters);
  
  signal dev_bus_slave_i  : t_wishbone_slave_in_array  (c_dev_masters-1 downto 0);
  signal dev_bus_slave_o  : t_wishbone_slave_out_array (c_dev_masters-1 downto 0);
  signal dev_msi_master_i : t_wishbone_master_in_array (c_dev_masters-1 downto 0);
  signal dev_msi_master_o : t_wishbone_master_out_array(c_dev_masters-1 downto 0);
  
  ----------------------------------------------------------------------------------
  -- GSI Dev Crossbar Slaves -------------------------------------------------------
  ----------------------------------------------------------------------------------
  
  -- required slaves
  constant c_dev_slaves          : natural := 28;
  constant c_devs_build_id       : natural := 0;
  constant c_devs_watchdog       : natural := 1;
  constant c_devs_mbox           : natural := 2;
  constant c_devs_flash          : natural := 3;
  constant c_devs_reset          : natural := 4;
  constant c_devs_wrc            : natural := 5;
  constant c_devs_ebm            : natural := 6;
  constant c_devs_tlu            : natural := 7;
  constant c_devs_eca_ctl        : natural := 8;
  constant c_devs_eca_aq         : natural := 9;
  constant c_devs_eca_tlu        : natural := 10;
  constant c_devs_eca_wbm        : natural := 11;
  constant c_devs_emb_cpu        : natural := 12;
  constant c_devs_serdes_clk_gen : natural := 13;
  constant c_devs_control        : natural := 14;
  constant c_devs_ftm_cluster    : natural := 15;
  
  -- optional slaves:
  constant c_devs_lcd            : natural := 16;
  constant c_devs_oled           : natural := 17;
  constant c_devs_scubus         : natural := 18;
  constant c_devs_scubirq        : natural := 19;
  constant c_devs_mil            : natural := 20;
  constant c_devs_mil_ctrl       : natural := 21;
  constant c_devs_ow             : natural := 22;
  constant c_devs_ssd1325        : natural := 23;
  constant c_devs_vme_info       : natural := 24;
  constant c_devs_CfiPFlash      : natural := 25;
  constant c_devs_nau8811        : natural := 26;
  constant c_devs_psram          : natural := 27;

  -- We have to specify the values for WRC as they provide no function for this
  constant c_wrcore_bridge_sdb : t_sdb_bridge := f_xwb_bridge_manual_sdb(x"0003ffff", x"00030000");
  constant c_ftm_slaves : t_sdb_bridge := f_cluster_bridge(c_dev_bridge_msi, g_lm32_cores, g_lm32_ramsizes, g_lm32_are_ftm);
  
  constant c_dev_layout_req_slaves : t_sdb_record_array(c_dev_slaves-1 downto 0) :=
   (c_devs_build_id       => f_sdb_auto_device(c_build_id_sdb,                   true),
    c_devs_watchdog       => f_sdb_auto_device(c_watchdog_sdb,                   true),
    c_devs_mbox           => f_sdb_auto_device(c_mbox_sdb,                       true),
    c_devs_flash          => f_sdb_auto_device(f_wb_spi_flash_sdb(g_flash_bits), true),
    c_devs_reset          => f_sdb_auto_device(c_arria_reset,                    true),
    c_devs_wrc            => f_sdb_auto_bridge(c_wrcore_bridge_sdb,              true),
    c_devs_ebm            => f_sdb_auto_device(c_ebm_sdb,                        true),
    c_devs_tlu            => f_sdb_auto_device(c_tlu_sdb,                        true),
    c_devs_eca_ctl        => f_sdb_auto_device(c_eca_slave_sdb,                  true),
    c_devs_eca_aq         => f_sdb_auto_device(c_eca_queue_slave_sdb,            true),
    c_devs_eca_tlu        => f_sdb_auto_device(c_eca_tlu_slave_sdb,              true),
    c_devs_eca_wbm        => f_sdb_auto_device(c_eca_ac_wbm_slave_sdb,           true),
    c_devs_emb_cpu        => f_sdb_auto_device(c_eca_queue_slave_sdb,            true),
    c_devs_serdes_clk_gen => f_sdb_auto_device(c_wb_serdes_clk_gen_sdb,          true),
    c_devs_control        => f_sdb_auto_device(c_io_control_sdb,                 true),
    c_devs_ftm_cluster    => f_sdb_auto_bridge(c_ftm_slaves,                     true),
    c_devs_lcd            => f_sdb_auto_device(c_wb_serial_lcd_sdb,              g_en_lcd),
    c_devs_oled           => f_sdb_auto_device(c_oled_display,                   g_en_oled),
    c_devs_scubus         => f_sdb_auto_device(c_scu_bus_master,                 g_en_scubus),
    c_devs_scubirq        => f_sdb_auto_device(c_scu_irq_ctrl_sdb,               g_en_scubus),
    c_devs_mil            => f_sdb_auto_device(c_xwb_gsi_mil_scu,                g_en_mil),
    c_devs_mil_ctrl       => f_sdb_auto_device(c_irq_master_ctrl_sdb,            g_en_mil),
    c_devs_ow             => f_sdb_auto_device(c_wrc_periph2_sdb,                g_en_user_ow),
    c_devs_nau8811        => f_sdb_auto_device(c_nau8811_sdb,                    g_en_nau8811),
    c_devs_vme_info       => f_sdb_auto_device(c_vme_info_sdb,                   g_en_vme),
    c_devs_psram          => f_sdb_auto_device(f_psram_sdb(g_psram_bits),        g_en_psram),
    c_devs_CfiPFlash      => f_sdb_auto_device(c_wb_CfiPFlash_sdb,               g_en_cfi),
    c_devs_ssd1325        => f_sdb_auto_device(c_ssd1325_sdb,                    g_en_ssd1325));
    
  constant c_dev_layout      : t_sdb_record_array := f_sdb_auto_layout(c_dev_layout_req_masters, c_dev_layout_req_slaves);
  constant c_dev_sdb_address : t_wishbone_address := f_sdb_auto_sdb   (c_dev_layout_req_masters, c_dev_layout_req_slaves);
  constant c_dev_bridge_sdb  : t_sdb_bridge       := f_xwb_bridge_layout_sdb(true, c_dev_layout, c_dev_sdb_address);
  
  signal dev_msi_slave_i  : t_wishbone_slave_in_array  (c_dev_slaves-1 downto 0) := (others => c_zero_master);
  signal dev_msi_slave_o  : t_wishbone_slave_out_array (c_dev_slaves-1 downto 0);
  signal dev_bus_master_i : t_wishbone_master_in_array (c_dev_slaves-1 downto 0);
  signal dev_bus_master_o : t_wishbone_master_out_array(c_dev_slaves-1 downto 0);

  ----------------------------------------------------------------------------------
  -- GSI Top Crossbar Slaves -------------------------------------------------------
  ----------------------------------------------------------------------------------
  
  -- Only put a slave here if it has critical performance requirements!
  constant c_top_slaves     : natural := 2;
  constant c_tops_eca_event : natural := 0;
  constant c_tops_dev       : natural := 1;
  
  constant c_top_layout_req_slaves : t_sdb_record_array(c_top_slaves-1 downto 0) :=
   (c_tops_eca_event  => f_sdb_embed_device(c_eca_event_sdb, x"7FFFFFF0"), -- must be located at fixed address
    c_tops_dev        => f_sdb_auto_bridge (c_dev_bridge_sdb));
  
  constant c_top_layout      : t_sdb_record_array := f_sdb_auto_layout(c_top_layout_req_masters, c_top_layout_req_slaves);
  constant c_top_sdb_address : t_wishbone_address := f_sdb_auto_sdb   (c_top_layout_req_masters, c_top_layout_req_slaves);
  constant c_top_bridge_sdb  : t_sdb_bridge       := f_xwb_bridge_layout_sdb(true, c_top_layout, c_top_sdb_address);
  
  signal top_msi_slave_i  : t_wishbone_slave_in_array  (c_top_slaves-1 downto 0) := (others => c_zero_master);
  signal top_msi_slave_o  : t_wishbone_slave_out_array (c_top_slaves-1 downto 0);
  signal top_bus_master_i : t_wishbone_master_in_array (c_top_slaves-1 downto 0);
  signal top_bus_master_o : t_wishbone_master_out_array(c_top_slaves-1 downto 0);

  ----------------------------------------------------------------------------------
  -- Clock networks ----------------------------------------------------------------
  ----------------------------------------------------------------------------------
  
  -- Non-PLL reset stuff
  signal clk_free         : std_logic;
  signal rstn_free        : std_logic;
  signal gxb_locked       : std_logic;
  signal pll_rst          : std_logic;
  
  -- Sys PLL from clk_125m_local_i
  signal sys_locked       : std_logic;
  signal clk_sys0         : std_logic;
  signal clk_sys1         : std_logic;
  signal clk_sys2         : std_logic;
  signal clk_sys3         : std_logic;
  signal clk_sys4         : std_logic;
  signal clk_sys5         : std_logic;
  
  signal clk_sys          : std_logic;
  signal clk_reconf       : std_logic; -- 50MHz on arrai2, 100MHz on arria5
  signal clk_flash_ext    : std_logic;
  signal clk_flash_out    : std_logic;
  signal clk_flash_in     : std_logic;
  signal clk_20m          : std_logic;
  signal clk_update       : std_logic;
  signal rstn_sys         : std_logic;
  signal rstn_update      : std_logic;
  
  -- Ref PLL from clk_125m_pllref_i
  signal ref_locked       : std_logic;
  signal clk_ref0         : std_logic;
  signal clk_ref1         : std_logic;
  signal clk_ref2         : std_logic;
  signal clk_ref3         : std_logic;
  signal clk_ref4         : std_logic;
  
  signal clk_ref          : std_logic;
  signal clk_butis        : std_logic;
  signal clk_phase        : std_logic;
  signal clk_lvds         : std_logic;
  signal clk_enable       : std_logic;
  signal clk_12_5         : std_logic;
  signal rstn_ref         : std_logic;
  signal rstn_butis       : std_logic;
  
  signal phase_done       : std_logic;
  signal phase_step       : std_logic;
  signal phase_sel        : std_logic_vector(4 downto 0);
  
  signal phase_butis      : phase_offset;
  
  -- DMTD PLL from clk_20m_vcxo_i
  signal dmtd_locked      : std_logic;
  signal clk_dmtd0        : std_logic;
  signal clk_dmtd         : std_logic;
  
  -- BuTiS T0 clocks
  signal clk_butis_t0     : std_logic; -- 100KHz
  signal clk_butis_t0_ts  : std_logic; -- 100KHz + timestamp
  
  -- END OF Clock networks
  ----------------------------------------------------------------------------------
  
  ----------------------------------------------------------------------------------
  -- Master signals ----------------------------------------------------------------
  ----------------------------------------------------------------------------------
  signal wrc_slave_i   : t_wishbone_slave_in;
  signal wrc_slave_o   : t_wishbone_slave_out;
  signal wrc_master_i  : t_wishbone_master_in;
  signal wrc_master_o  : t_wishbone_master_out;
  signal eb_src_out    : t_wrf_source_out;
  signal eb_src_in     : t_wrf_source_in;
  signal eb_snk_out    : t_wrf_sink_out;
  signal eb_snk_in     : t_wrf_sink_in;
  
  signal uart_usb : std_logic; -- from usb
  signal uart_mux : std_logic; -- either usb or external
  signal uart_wrc : std_logic; -- from wrc
  
  signal s_usb_fd_o   : std_logic_vector(7 downto 0);
  signal s_usb_fd_oen : std_logic;
  
  signal s_lm32_rstn : std_logic_vector(g_lm32_cores-1 downto 0);

  -- END OF Master signals
  ----------------------------------------------------------------------------------

  ----------------------------------------------------------------------------------
  -- White Rabbit signals ----------------------------------------------------------
  ----------------------------------------------------------------------------------
  
  signal dac_hpll_load_p1 : std_logic;
  signal dac_dpll_load_p1 : std_logic;
  signal dac_hpll_data    : std_logic_vector(15 downto 0);
  signal dac_dpll_data    : std_logic_vector(15 downto 0);
  
  signal phy_tx_data      : std_logic_vector(7 downto 0);
  signal phy_tx_k         : std_logic;
  signal phy_tx_disparity : std_logic;
  signal phy_tx_enc_err   : std_logic;
  signal phy_rx_data      : std_logic_vector(7 downto 0);
  signal phy_rx_rbclk     : std_logic;
  signal phy_rx_k         : std_logic;
  signal phy_rx_enc_err   : std_logic;
  signal phy_rx_bitslide  : std_logic_vector(3 downto 0);
  signal phy_rst          : std_logic;
  signal phy_loopen       : std_logic;

  signal link_act : std_logic;
  signal link_up  : std_logic;
  signal pps      : std_logic;
  signal ext_pps  : std_logic;

  signal tm_valid  : std_logic;
  signal tm_tai    : std_logic_vector(39 downto 0);
  signal tm_cycles : std_logic_vector(27 downto 0);
  
  signal ref_tai8ns : std_logic_vector(63 downto 0);

  signal owr_pwren : std_logic_vector(1 downto 0);
  signal owr_en    : std_logic_vector(1 downto 0);
  
  signal sfp_scl_o : std_logic;
  signal sfp_sda_o : std_logic;
  
  constant c_loc_linux        : natural := 0;
  constant c_loc_wb_master    : natural := 1;
  constant c_loc_embedded_cpu : natural := 2;
  constant c_loc_scubus_tag   : natural := 3;
  
  function f_channel_types return t_nat_array is
    constant c_scu_channel_types : t_nat_array(3 downto 0) := (
      0 => c_loc_linux, 
      1 => c_loc_wb_master, 
      2 => c_loc_embedded_cpu,
      3 => c_loc_scubus_tag);
    constant c_channel_types : t_nat_array(2 downto 0) := c_scu_channel_types(2 downto 0);
  begin
    if g_en_scubus then
      return c_scu_channel_types;
    else
      return c_channel_types;
    end if;
  end f_channel_types;
  
  constant c_channel_types : t_nat_array := f_channel_types;
  
  signal s_stall_i   : std_logic_vector(c_channel_types'range) := (others => '0');
  signal s_channel_o : t_channel_array(c_channel_types'range);
  signal s_time      : t_time;
  
  constant c_num_streams : natural := 2;
  signal s_stream_i : t_stream_array(c_num_streams-1 downto 0);
  signal s_stall_o  : std_logic_vector(c_num_streams-1 downto 0);
  
  -- END OF White Rabbit
  ----------------------------------------------------------------------------------

  ----------------------------------------------------------------------------------
  -- Mil-Extension signals ---------------------------------------------------------
  ----------------------------------------------------------------------------------
  
  signal  mil_interlock_intr_o:   std_logic;
  signal  mil_data_rdy_intr_o:    std_logic;
  signal  mil_data_req_intr_o:    std_logic;
  signal  mil_dly_intr_o:         std_logic;
  signal  mil_ev_fifo_ne_intr_o:  std_logic;
  signal  mil_every_ms_intr_o:  std_logic;
  
  -- Mil-Extension signals
  ----------------------------------------------------------------------------------
  
  ----------------------------------------------------------------------------------
  -- SCU bus signals ---------------------------------------------------------
  ----------------------------------------------------------------------------------
  
  signal  tag        : std_logic_vector(31 downto 0);
  signal  tag_valid  : std_logic;
  
  -- SCU bus signals
  ----------------------------------------------------------------------------------
  
  
  ----------------------------------------------------------------------------------
  -- VME signals -------------------------------------------------------------------
  ----------------------------------------------------------------------------------
  
  signal s_vme_lword_n_o    : std_logic;
  signal s_vme_lword_n_i    : std_logic;
  signal s_vme_berr_o       : std_logic;
  signal s_vme_dtack_n_o    : std_logic;
  signal s_vme_dtack_oe_o   : std_logic;
  signal s_vme_data_o       : std_logic_vector(31 downto 0);
  signal s_vme_addr_o       : std_logic_vector(31 downto 1);
  signal s_vme_buffer       : t_vme_buffer;
  signal s_vme_buffer_latch : std_logic;
  
  -- END OF VME signals
  ----------------------------------------------------------------------------------
  
  signal lcd_scp       : std_logic;
  signal lcd_lp        : std_logic;
  signal lcd_flm       : std_logic;
  signal lcd_in        : std_logic;
  signal user_ow_pwren : std_logic_vector(1 downto 0);
  signal user_ow_en    : std_logic_vector(1 downto 0);
  
  constant c_eca_lvds : natural := g_lvds_inout + g_lvds_out;
  constant c_eca_gpio : natural := g_gpio_inout + g_gpio_out;
  constant c_eca_io   : natural := c_eca_lvds + c_eca_gpio;
  
  constant c_tlu_lvds : natural := g_lvds_inout + g_lvds_in;
  constant c_tlu_gpio : natural := g_gpio_inout + g_gpio_in;
  constant c_tlu_io   : natural := c_tlu_lvds + c_tlu_gpio;

  signal s_eca_io   : t_gpio_array(c_eca_io-1 downto 0);
  signal s_tlu_io   : t_gpio_array(c_tlu_io-1 downto 0);

  signal s_gpio_out          : std_logic_vector(f_sub1(c_eca_gpio) downto 0);
  signal s_gpio_src_eca      : std_logic_vector(f_sub1(c_eca_gpio) downto 0);
  signal s_gpio_src_ioc      : std_logic_vector(f_sub1(c_eca_gpio) downto 0);
  signal s_gpio_src_wr_pps   : std_logic_vector(f_sub1(c_eca_gpio) downto 0);
  signal s_gpio_src_butis_t0 : std_logic_vector(f_sub1(c_eca_gpio) downto 0);
  
  signal s_gpio_mux      : std_logic_vector(f_sub1(c_eca_gpio) downto 0);
  signal s_lvds_mux      : std_logic_vector(f_sub1(c_eca_lvds) downto 0);
  signal s_gpio_pps_mux  : std_logic_vector(f_sub1(c_eca_gpio) downto 0);
  signal s_lvds_pps_mux  : std_logic_vector(f_sub1(c_eca_lvds) downto 0);
  signal s_lvds_vec_i    : t_lvds_byte_array(f_sub1(g_lvds_inout+g_lvds_in) downto 0);
  
  signal lvds_dat_fr_butis_t0 : t_lvds_byte_array(f_sub1(c_eca_lvds) downto 0);
  signal lvds_dat_fr_ioc      : t_lvds_byte_array(f_sub1(c_eca_lvds) downto 0);
  signal lvds_dat_fr_eca_chan : t_lvds_byte_array(f_sub1(c_eca_lvds) downto 0);
  signal lvds_dat_fr_clk_gen  : t_lvds_byte_array(f_sub1(c_eca_lvds) downto 0);
  signal lvds_dat_fr_wr_pps   : t_lvds_byte_array(f_sub1(c_eca_lvds) downto 0);
  signal lvds_dat             : t_lvds_byte_array(f_sub1(c_eca_lvds) downto 0);
  signal lvds_i               : t_lvds_byte_array(15 downto 0);
  
  signal s_triggers : t_trigger_array(g_gpio_in + g_gpio_inout + g_lvds_inout + g_lvds_in -1 downto 0);
  
  function f_lvds_array_to_trigger_array(lvds : t_lvds_byte_array) return t_trigger_array is
    variable i : natural := 0;
    variable result : t_trigger_array(lvds'left downto 0);
  begin
    for i in 0 to lvds'left loop
      result(i) := lvds(i);
    end loop;
    return result;
  end f_lvds_array_to_trigger_array; 
  
begin

  ----------------------------------------------------------------------------------
  -- Reset and PLLs ----------------------------------------------------------------
  ----------------------------------------------------------------------------------
  
  -- We need at least one off-chip free running clock to setup PLLs
  free_a5 : if c_is_arria5 generate
    clk_free <= core_clk_125m_local_i;
  end generate;
  free_a2 : if c_is_arria2 generate
    clk_free <= core_clk_20m_vcxo_i; -- (125MHz is too fast)
  end generate;
  
  reset : altera_reset
    generic map(
      g_plls   => 4,
      g_clocks => 4,
      g_areset => f_pick(c_is_arria5, 100, 1)*1024,
      g_stable => f_pick(c_is_arria5, 100, 1)*1024)
    port map(
      clk_free_i    => clk_free,
      rstn_i        => core_rstn_i,
      pll_lock_i(0) => dmtd_locked,
      pll_lock_i(1) => ref_locked,
      pll_lock_i(2) => sys_locked,
      pll_lock_i(3) => gxb_locked,
      pll_arst_o    => pll_rst,
      clocks_i(0)   => clk_free,
      clocks_i(1)   => clk_sys,
      clocks_i(2)   => clk_update,
      clocks_i(3)   => clk_ref,
      rstn_o(0)     => rstn_free,
      rstn_o(1)     => rstn_sys,
      rstn_o(2)     => rstn_update,
      rstn_o(3)     => rstn_ref);

  dmtd_a2 : if c_is_arria2 generate
    dmtd_inst : dmtd_pll port map(
      areset   => pll_rst,
      inclk0   => core_clk_20m_vcxo_i,    --  20  Mhz 
      c0       => clk_dmtd0,              --  62.5MHz
      locked   => dmtd_locked);
  end generate;
  dmtd_a5 : if c_is_arria5 generate
    dmtd_inst : dmtd_pll5 port map(
      rst      => pll_rst,
      refclk   => core_clk_20m_vcxo_i,    --  20  MHz
      outclk_0 => clk_dmtd0,              --  62.5MHz
      locked   => dmtd_locked);
  end generate;
  
  dmtd_clk : single_region port map(
    inclk  => clk_dmtd0,
    outclk => clk_dmtd);
  
  sys_a2 : if c_is_arria2 generate
    sys_inst : sys_pll port map(
      areset => pll_rst,
      inclk0 => core_clk_125m_local_i, -- 125  Mhz 
      c0     => clk_sys0,         --  62.5 MHz
      c1     => clk_sys1,         --  50  Mhz
      c2     => clk_sys2,         --  20  MHz
      c3     => clk_sys3,         --  10  MHz
      locked => sys_locked);
    clk_sys4 <= clk_sys1;
  end generate;
  sys_a5 : if c_is_arria5 generate
    sys_inst : sys_pll5 port map(
      rst      => pll_rst,
      refclk   => core_clk_125m_local_i, -- 125  Mhz 
      outclk_0 => clk_sys0,           --  62.5MHz
      outclk_1 => clk_sys1,           -- 100  MHz +0   ns
      outclk_2 => clk_sys2,           --  20  MHz
      outclk_3 => clk_sys3,           --  10  MHz
      outclk_4 => clk_sys4,           --  20  MHz
      locked   => sys_locked);
  end generate;
  
  sys_clk : global_region port map(
    inclk  => clk_sys0,
    outclk => clk_sys);
  
  reconf_clk : global_region port map(
    inclk  => clk_sys1,
    outclk => clk_reconf);
  
  c20m_clk : single_region port map(
    inclk  => clk_sys2,
    outclk => clk_20m);
  
  update_clk : single_region port map(
    inclk  => clk_sys3,
    outclk => clk_update);
  
  flash_out : global_region port map(
    inclk  => clk_sys4,
    outclk => clk_flash_ext);
  
  clk_flash_in  <= clk_flash_ext;
  clk_flash_out <= clk_reconf;
  
  ref_a2 : if c_is_arria2 generate
    ref_inst : ref_pll port map( -- see "Phase Counter Select Mapping" table for arria2gx
      areset => pll_rst,
      inclk0 => core_clk_125m_pllref_i, -- 125 MHz
      c0     => clk_ref0,          -- 125 MHz, counter: 0010 - #2
      c1     => clk_ref1,          -- 200 MHz, counter: 0011 = #3
      c2     => clk_ref2,          --  25 MHz, counter: 0100 = #4
      locked => ref_locked,
      scanclk            => clk_free,
      phasedone          => phase_done,
      phasecounterselect => phase_sel(3 downto 0),
      phasestep          => phase_step,
      phaseupdown        => '1');
  end generate;

  ref_a5 : if c_is_arria5 generate
    ref_inst : ref_pll5 port map(
      rst        => pll_rst,
      refclk     => core_clk_125m_pllref_i, -- 125 MHz
      outclk_0   => clk_ref0,         -- 125 MHz
      outclk_1   => clk_ref1,         -- 200 MHz
      outclk_2   => clk_ref2,         --  25 MHz
      outclk_3   => clk_ref3,         --1000 MHz
      outclk_4   => clk_ref4,         -- 125 MHz, 1/8 duty, -1.5ns phase
      locked     => ref_locked,
      scanclk    => clk_free,  
      cntsel     => phase_sel, 
      phase_en   => phase_step,
      updn       => '1',              -- positive phase shift (widen period)
      phase_done => phase_done);
  end generate;
  
  phase : altera_phase
    generic map(
      g_select_bits   => 5,
      g_outputs       => 1,
      g_base          => 0,
      g_vco_freq      => 1000, -- 1GHz
      g_output_freq   => (0 => 200),
      g_output_select => (0 => f_pick(c_is_arria5, 4, 3)))
    port map(
      clk_i       => clk_free,
      rstn_i      => rstn_free,
      clks_i(0)   => clk_butis,
      rstn_o(0)   => rstn_butis,
      offset_i(0) => phase_butis,
      phasedone_i => phase_done,
      phasesel_o  => phase_sel,
      phasestep_o => phase_step);
  
  ref_clk : global_region port map(
    inclk  => clk_ref0,
    outclk => clk_ref);

  --butis_clk : global_region port map(
  --  inclk  => clk_ref1,
  -- outclk => clk_butis);
  clk_butis <= clk_ref1;
  
  clk_div: process(clk_ref0)
    variable cnt: integer := 0;
  begin
    if rising_edge(clk_ref0) then
      if cnt < 4 then
        cnt := cnt + 1;
      else
        cnt := 0;
        clk_12_5 <= not clk_12_5;
      end if;
    end if;
  end process;
  
  phase_clk : global_region port map( -- skew must match ref_clk
    inclk  => clk_ref2,
    outclk => clk_phase);
  
  clk_lvds   <= clk_ref3;
  clk_enable <= clk_ref4;

  butis : altera_butis
    port map(
      clk_ref_i => clk_ref,
      clk_25m_i => clk_phase,
      pps_i     => pps,
      phase_o   => phase_butis);
  
  butis_t0 : BuTiS_T0_generator
    port map(
      wr_clock_i               => clk_ref,
      wr_rst_n_i               => rstn_ref,
      wr_PPSpulse_i            => pps,
      BuTis_rst_n_i            => rstn_butis,
      timestamp_i              => s_time,
      BuTis_C2_i               => clk_butis,
      BuTis_T0_o               => clk_butis_t0,
      BuTis_T0_timestamp_o     => clk_butis_t0_ts,
      error_o                  => open);
  
  core_clk_wr_ref_o  <= clk_ref;
  core_clk_butis_o   <= clk_butis;
  core_clk_butis_t0_o<= clk_butis_t0_ts;
  core_rstn_wr_ref_o <= rstn_ref;
  core_rstn_butis_o  <= rstn_butis;
  
  -- END OF Reset and PLLs
  ----------------------------------------------------------------------------------
  
  ----------------------------------------------------------------------------------
  -- Wishbone crossbars ------------------------------------------------------------
  ----------------------------------------------------------------------------------
  
  top_bar : xwb_sdb_crossbar
    generic map(
      g_num_masters => c_top_masters,
      g_num_slaves  => c_top_slaves,
      g_registered  => true,
      g_wraparound  => true,
      g_layout      => c_top_layout,
      g_sdb_addr    => c_top_sdb_address)
    port map(
      clk_sys_i     => clk_sys,
      rst_n_i       => rstn_sys,
      slave_i       => top_bus_slave_i,
      slave_o       => top_bus_slave_o,
      msi_master_i  => top_msi_master_i,
      msi_master_o  => top_msi_master_o,
      master_i      => top_bus_master_i,
      master_o      => top_bus_master_o,
      msi_slave_i   => top_msi_slave_i,
      msi_slave_o   => top_msi_slave_o);
  
  dev_bar : xwb_sdb_crossbar
    generic map(
      g_num_masters => c_dev_masters,
      g_num_slaves  => c_dev_slaves,
      g_registered  => true,
      g_wraparound  => true,
      g_layout      => c_dev_layout,
      g_sdb_addr    => c_dev_sdb_address)
    port map(
      clk_sys_i     => clk_sys,
      rst_n_i       => rstn_sys,
      slave_i       => dev_bus_slave_i,
      slave_o       => dev_bus_slave_o,
      msi_master_i  => dev_msi_master_i,
      msi_master_o  => dev_msi_master_o,
      master_i      => dev_bus_master_i,
      master_o      => dev_bus_master_o,
      msi_slave_i   => dev_msi_slave_i,
      msi_slave_o   => dev_msi_slave_o);
  
  top2dev_bus : xwb_register_link
    port map(
      clk_sys_i     => clk_sys,
      rst_n_i       => rstn_sys,
      slave_i       => top_bus_master_o(c_tops_dev),
      slave_o       => top_bus_master_i(c_tops_dev),
      master_i      => dev_bus_slave_o (c_devm_top),
      master_o      => dev_bus_slave_i (c_devm_top));
  
  dev2top_msi : xwb_register_link
    port map(
      clk_sys_i     => clk_sys,
      rst_n_i       => rstn_sys,
      slave_i       => dev_msi_master_o(c_devm_top),
      slave_o       => dev_msi_master_i(c_devm_top),
      master_i      => top_msi_slave_o (c_tops_dev),
      master_o      => top_msi_slave_i (c_tops_dev));
  
  top2wrc_bus : xwb_register_link
    port map(
      clk_sys_i     => clk_sys,
      rst_n_i       => rstn_sys,
      slave_i       => dev_bus_master_o(c_devs_wrc),
      slave_o       => dev_bus_master_i(c_devs_wrc),
      master_i      => wrc_slave_o,
      master_o      => wrc_slave_i);
  
  -- END OF Wishbone crossbars
  ----------------------------------------------------------------------------------

  ----------------------------------------------------------------------------------
  -- Top Wishbone masters ----------------------------------------------------------
  
  top_msi_master_i(c_topm_ebs) <= cc_dummy_slave_out; -- Etherbone does not accept MSI !!!
  eb : eb_master_slave_wrapper
    generic map(
      g_with_master     => true,
      g_ebs_sdb_address => (x"00000000" & c_top_sdb_address))
    port map(
      clk_i           => clk_sys,
      nRst_i          => rstn_sys,
      snk_i           => eb_snk_in,
      snk_o           => eb_snk_out,
      src_o           => eb_src_out,
      src_i           => eb_src_in,
      ebs_cfg_slave_o => wrc_master_i,
      ebs_cfg_slave_i => wrc_master_o,
      ebs_wb_master_o => top_bus_slave_i (c_topm_ebs),
      ebs_wb_master_i => top_bus_slave_o (c_topm_ebs),
      ebm_wb_slave_i  => dev_bus_master_o(c_devs_ebm),
      ebm_wb_slave_o  => dev_bus_master_i(c_devs_ebm));
   

  lm32 : ftm_lm32_cluster 
    generic map(
      g_is_dm            => g_lm32_are_ftm,	
      g_cores            => g_lm32_cores,
      g_ram_per_core     => g_lm32_ramsizes,
      g_world_bridge_sdb => c_top_bridge_sdb,
      g_clu_msi_sdb      => c_dev_bridge_msi,
      g_init_files       => g_lm32_init_files,
		g_profiles         => g_lm32_profiles)
    port map(
      clk_ref_i          => clk_ref,
      rst_ref_n_i        => rstn_ref,
      clk_sys_i          => clk_sys,
      rst_sys_n_i        => rstn_sys,
      rst_lm32_n_i       => s_lm32_rstn,
      tm_tai8ns_i        => s_time,
      lm32_masters_o     => top_bus_slave_i(top_bus_slave_i'high downto c_top_my_masters),
      lm32_masters_i     => top_bus_slave_o(top_bus_slave_o'high downto c_top_my_masters),
      lm32_msi_slaves_o  => top_msi_master_i(top_msi_master_i'high downto c_top_my_masters), 
      lm32_msi_slaves_i  => top_msi_master_o(top_msi_master_o'high downto c_top_my_masters), 
      clu_slave_o        => dev_bus_master_i(c_devs_ftm_cluster),
      clu_slave_i        => dev_bus_master_o(c_devs_ftm_cluster),
      clu_msi_master_o   => dev_msi_slave_i(c_devs_ftm_cluster),
      clu_msi_master_i   => dev_msi_slave_o(c_devs_ftm_cluster),
      dm_prioq_master_o  => top_bus_slave_i(c_topm_prioq),
      dm_prioq_master_i  => top_bus_slave_o(c_topm_prioq));
  
  pcie_n : if not g_en_pcie generate
    top_bus_slave_i (c_topm_pcie) <= cc_dummy_master_out;
    top_msi_master_i(c_topm_pcie) <= cc_dummy_slave_out;
  end generate;
  pcie_y : if g_en_pcie generate
    pcie : pcie_wb
      generic map(
        g_family => g_family,
        sdb_addr => c_top_sdb_address)
      port map(
        clk125_i      => core_clk_125m_local_i,
        cal_clk50_i   => clk_reconf,
        pcie_refclk_i => pcie_refclk_i,
        pcie_rstn_i   => pcie_rstn_i,
        pcie_rx_i     => pcie_rx_i,
        pcie_tx_o     => pcie_tx_o,
        master_clk_i  => clk_sys,
        master_rstn_i => rstn_sys,
        master_o      => top_bus_slave_i (c_topm_pcie),
        master_i      => top_bus_slave_o (c_topm_pcie),
        slave_clk_i   => clk_sys,
        slave_rstn_i  => rstn_sys,
        slave_i       => top_msi_master_o(c_topm_pcie),
        slave_o       => top_msi_master_i(c_topm_pcie));
  end generate;

  pmc_n : if not g_en_pmc generate
    top_bus_slave_i (c_topm_pmc) <= cc_dummy_master_out;
    top_msi_master_i(c_topm_pmc) <= cc_dummy_slave_out;
  end generate;
 pmc_y : if g_en_pmc generate
    signal s_pmc_debug_in   : std_logic_vector(7 downto 0);
    signal s_pmc_debug_out  : std_logic_vector(7 downto 0);  
 begin 
    pmc : wb_pmc_host_bridge
    generic map(
      g_family      => g_family,
      g_sdb_addr    => c_top_sdb_address
    ) 
    port map(
      clk_sys_i     => clk_sys,
      rst_n_i       => rstn_sys,

      master_clk_i  => clk_sys,
      master_rstn_i => rstn_sys,
      slave_clk_i   => clk_sys,
      slave_rstn_i  => rstn_sys,
      master_o      => top_bus_slave_i (c_topm_pmc),
      master_i      => top_bus_slave_o (c_topm_pmc),
      slave_i       => top_msi_master_o(c_topm_pmc),
      slave_o       => top_msi_master_i(c_topm_pmc), 
      pci_clk_i     => pmc_pci_clk_i,
      pci_rst_i     => pmc_pci_rst_i,
      buf_oe_o      => pmc_buf_oe_o,
      busmode_io    => pmc_busmode_io,
      ad_io         => pmc_ad_io,
      c_be_io       => pmc_c_be_io,
      par_io        => pmc_par_io,
      frame_io      => pmc_frame_io,
      trdy_io       => pmc_trdy_io,
      irdy_io       => pmc_irdy_io,
      stop_io       => pmc_stop_io,
      devsel_io     => pmc_devsel_io,
      idsel_i       => pmc_idsel_i,
      perr_io       => pmc_perr_io,
      serr_io       => pmc_serr_io,
      inta_o        => pmc_inta_o,
      req_o         => pmc_req_o,
      gnt_i         => pmc_gnt_i,
      debug_i       => s_pmc_debug_in,
      debug_o       => s_pmc_debug_out
    );

    s_pmc_debug_in(0)          <= gpio_i(0);      -- FPGA push button used to trigger INTx IRQ
    s_pmc_debug_in(1)          <= gpio_i(1); -- CPLD push button used to trigger MSI IRQ
    s_pmc_debug_in(7 downto 2) <= (others => '0');

  end generate;

  
  vme_n : if not g_en_vme generate
    top_bus_slave_i (c_topm_vme) <= cc_dummy_master_out;
    top_msi_master_i(c_topm_vme) <= cc_dummy_slave_out;
    dev_bus_master_i(c_devs_vme_info) <= cc_dummy_slave_out;
    vme_addr_data_b <= (others => 'Z');
  end generate;
  vme_y : if g_en_vme generate
    
    U_VME64 : xVME64xCore_Top
      generic map(
        g_clock          => 62,
        g_wb_data_width  => 32,
        g_wb_addr_width  => 32,
        g_cram_size      => c_CRAM_SIZE,  -- 1024
        g_BoardID        => c_VETAR_ID,   -- 0x00000199 
        g_ManufacturerID => c_GSI_ID,     -- 0x080031
        g_RevisionID     => c_RevisionID, -- 0x1
        g_ProgramID      => 96,           -- 0x60
        g_base_addr      => MECHANICALLY,
        g_sdb_addr       => c_top_sdb_address,
        g_irq_src        => MSI)
       port map(
        clk_i           => clk_sys,
        rst_n_i         => rstn_sys,
        vme_as_n_i      => vme_as_n_i,
        vme_rst_n_i     => vme_rst_n_i,
        vme_write_n_i   => vme_write_n_i,
        vme_am_i        => vme_am_i,
        vme_ds_n_i      => vme_ds_n_i,
        vme_ga_i        => b"00" & vme_ga_i,
        vme_berr_o      => s_vme_berr_o,
        vme_dtack_n_o   => s_vme_dtack_n_o,
        vme_retry_n_o   => open,		
        vme_lword_n_i   => s_vme_lword_n_i,
        vme_lword_n_o   => s_vme_lword_n_o,
        vme_addr_i      => vme_addr_data_b(31 downto 1),
        vme_addr_o      => s_vme_addr_o,
        vme_data_i      => vme_addr_data_b,
        vme_data_o      => s_vme_data_o,
        vme_irq_o       => vme_irq_n_o,
        vme_iackin_n_i  => vme_iackin_n_i,
        vme_iack_n_i    => vme_iack_n_i,
        vme_iackout_n_o => vme_iackout_n_o,
        vme_buffer_o    => s_vme_buffer,
        vme_retry_oe_o  => open,
        irq_i           => '0',  -- => wbirq_i,  
        int_ack_o       => open, -- => s_int_ack,
        --reset_o         => open, -- => s_rst,
        master_o        => top_bus_slave_i (c_topm_vme),
        master_i        => top_bus_slave_o (c_topm_vme),
        slave_o         => top_msi_master_i(c_topm_vme),
        slave_i         => top_msi_master_o(c_topm_vme), 
        info_slave_i    => dev_bus_master_o(c_devs_vme_info),
        info_slave_o    => dev_bus_master_i(c_devs_vme_info),
        debug           => open);
    
    U_BUFFER_CTRL : VME_Buffer_ctrl
      generic map(
        g_bus_mode  =>  LATCHED)
      port map( 
        clk_i            =>  clk_sys,
        rst_i            =>  vme_rst_n_i, 
        buffer_stat_i    =>  s_vme_buffer,
        buffer_clk_o     =>  open,
        data_buff_v2f_o  =>  vme_data_oe_ab_o,
        data_buff_f2v_o  =>  vme_data_oe_ba_o,
        addr_buff_v2f_o  =>  vme_addr_oe_ab_o,
        addr_buff_f2v_o  =>  vme_addr_oe_ba_o,
        dtack_oe_o       =>  s_vme_dtack_oe_o,
        latch_buff_o     =>  s_vme_buffer_latch);
    
    vme_addr_data_b <= 
      s_vme_data_o                     when s_vme_buffer.s_buffer_eo = data_buff and s_vme_buffer.s_datadir = fpga2vme else
      (s_vme_addr_o & s_vme_lword_n_o) when s_vme_buffer.s_buffer_eo = addr_buff and s_vme_buffer.s_addrdir = fpga2vme else 
      (others => 'Z');
    
    vme_buffer_latch_o <= (others => s_vme_buffer_latch);
    s_vme_lword_n_i    <= vme_addr_data_b(0);
    vme_dtack_oe_o     <= s_vme_dtack_n_o when s_vme_dtack_oe_o = '1' else '1';
    vme_berr_o         <= not s_vme_berr_o;
    
  end generate;
  
  top_msi_master_i(c_topm_usb) <= cc_dummy_slave_out; -- USB does not accept MSI !!!
  usb_n : if not g_en_usb generate
    top_bus_slave_i(c_topm_usb) <= cc_dummy_master_out;
    uart_usb <= '1';
    usb_readyn_io <= 'Z';
    usb_fd_io <= (others => 'Z');
  end generate;
  usb_y : if g_en_usb generate
    usb_readyn_io <= 'Z';
    usb_fd_io <= s_usb_fd_o when s_usb_fd_oen='1' else (others => 'Z');
    usb : ez_usb
      generic map(
        g_sdb_address => c_top_sdb_address)
      port map(
        clk_sys_i => clk_sys,
        rstn_i    => rstn_sys,
        master_i  => top_bus_slave_o(c_topm_usb),
        master_o  => top_bus_slave_i(c_topm_usb),
        uart_o    => uart_usb,
        uart_i    => uart_wrc,
        rstn_o    => usb_rstn_o,
        ebcyc_i   => usb_ebcyc_i,
        speed_i   => usb_speed_i,
        shift_i   => usb_shift_i,
        readyn_i  => usb_readyn_io,
        fifoadr_o => usb_fifoadr_o,
        fulln_i   => usb_fulln_i,
        sloen_o   => usb_sloen_o,
        emptyn_i  => usb_emptyn_i,
        slrdn_o   => usb_slrdn_o,
        slwrn_o   => usb_slwrn_o,
        pktendn_o => usb_pktendn_o,
        fd_i      => usb_fd_io,
        fd_o      => s_usb_fd_o,
        fd_oen_o  => s_usb_fd_oen);
  end generate;
  
  wr_uart_o <= uart_wrc;
  uart_mux <= uart_usb and wr_uart_i;
  
  -- END OF Wishbone masters
  ----------------------------------------------------------------------------------
  
  ----------------------------------------------------------------------------------
  -- White Rabbit ------------------------------------------------------------------
  ----------------------------------------------------------------------------------
  
  U_WR_CORE : xwr_core
    generic map (
      g_simulation                => 0,
      g_phys_uart                 => true,
      g_virtual_uart              => true,
      g_with_external_clock_input => true,
      g_aux_clks                  => 1,
      g_ep_rxbuf_size             => 1024,
      g_dpram_initf               => "../../../ip_cores/wrpc-sw/wrc.mif",
      g_dpram_size                => 131072/4,
      g_interface_mode            => PIPELINED,
      g_address_granularity       => BYTE,
      g_aux_sdb                   => c_etherbone_sdb)
    port map (
      clk_sys_i            => clk_sys,
      clk_dmtd_i           => clk_dmtd,
      clk_ref_i            => clk_ref,
      clk_aux_i            => (others => '0'),
      clk_ext_i            => wr_ext_clk_i,
      pps_ext_i            => wr_ext_pps_i,
      rst_n_i              => rstn_sys,
      dac_hpll_load_p1_o   => dac_hpll_load_p1,
      dac_hpll_data_o      => dac_hpll_data,
      dac_dpll_load_p1_o   => dac_dpll_load_p1,
      dac_dpll_data_o      => dac_dpll_data,
      phy_ref_clk_i        => clk_ref,
      phy_tx_data_o        => phy_tx_data,
      phy_tx_k_o(0)        => phy_tx_k,
      phy_tx_disparity_i   => phy_tx_disparity,
      phy_tx_enc_err_i     => phy_tx_enc_err,
      phy_rx_data_i        => phy_rx_data,
      phy_rx_rbclk_i       => phy_rx_rbclk,
      phy_rx_k_i(0)        => phy_rx_k,
      phy_rx_enc_err_i     => phy_rx_enc_err,
      phy_rx_bitslide_i    => phy_rx_bitslide,
      phy_rst_o            => phy_rst,
      phy_loopen_o         => phy_loopen,
      led_act_o            => link_act,
      led_link_o           => link_up,
      scl_o                => open, -- Our ROM is on onewire, not i2c
      scl_i                => '0',
      sda_i                => '0',
      sda_o                => open,
      sfp_scl_i            => wr_sfp_scl_io,
      sfp_sda_i            => wr_sfp_sda_io,
      sfp_scl_o            => sfp_scl_o,
      sfp_sda_o            => sfp_sda_o,
      sfp_det_i            => wr_sfp_det_i,
      btn1_i               => '0',
      btn2_i               => '0',
      uart_rxd_i           => uart_mux,
      uart_txd_o           => uart_wrc,
      owr_pwren_o          => owr_pwren,
      owr_en_o             => owr_en,
      owr_i(0)             => wr_onewire_io,
      owr_i(1)             => '0',
      slave_i              => wrc_slave_i,
      slave_o              => wrc_slave_o,
      aux_master_o         => wrc_master_o,
      aux_master_i         => wrc_master_i,
      wrf_src_o            => eb_snk_in,
      wrf_src_i            => eb_snk_out,
      wrf_snk_o            => eb_src_in,
      wrf_snk_i            => eb_src_out,
      tm_link_up_o         => open,
      tm_dac_value_o       => open,
      tm_dac_wr_o          => open,
      tm_clk_aux_lock_en_i => (others => '0'),
      tm_clk_aux_locked_o  => open,
      tm_time_valid_o      => tm_valid,
      tm_tai_o             => tm_tai,
      tm_cycles_o          => tm_cycles,
      pps_p_o              => pps,
      dio_o                => open,
      rst_aux_n_o          => open,
      link_ok_o            => open);
  
  U_DAC_ARB : spec_serial_dac_arb
    generic map (
      g_invert_sclk    => false,
      g_num_extra_bits => 8) -- AD DACs with 24bit interface
    port map (
      clk_i         => clk_sys,
      rst_n_i       => rstn_sys,
      val1_i        => dac_dpll_data,
      load1_i       => dac_dpll_load_p1,
      val2_i        => dac_hpll_data,
      load2_i       => dac_hpll_load_p1,
      dac_cs_n_o(0) => wr_ndac_cs_o(1),
      dac_cs_n_o(1) => wr_ndac_cs_o(2),
      dac_clr_n_o   => open,
      dac_sclk_o    => wr_dac_sclk_o,
      dac_din_o     => wr_dac_din_o);

  phy_a2 : if c_is_arria2 generate
    phy : wr_arria2_phy
      port map (
        clk_reconf_i   => clk_reconf,
        clk_pll_i      => clk_ref0, -- PLL cascade
        clk_cru_i      => core_clk_125m_sfpref_i,
        clk_free_i     => clk_free,
        rst_i          => pll_rst,
        locked_o       => gxb_locked,
        loopen_i       => phy_loopen,
        drop_link_i    => phy_rst,
        tx_clk_i       => clk_ref,
        tx_data_i      => phy_tx_data,
        tx_k_i         => phy_tx_k,
        tx_disparity_o => phy_tx_disparity,
        tx_enc_err_o   => phy_tx_enc_err,
        rx_rbclk_o     => phy_rx_rbclk,
        rx_data_o      => phy_rx_data,
        rx_k_o         => phy_rx_k,
        rx_enc_err_o   => phy_rx_enc_err,
        rx_bitslide_o  => phy_rx_bitslide,
        pad_txp_o      => wr_sfp_tx_o,
        pad_rxp_i      => wr_sfp_rx_i);    
  end generate;
  
  phy_a5 : if c_is_arria5 generate
    phy : wr_arria5_phy
      port map (
        clk_reconf_i   => clk_reconf,
        clk_phy_i      => core_clk_125m_sfpref_i,
        locked_o       => gxb_locked,
        loopen_i       => phy_loopen,
        drop_link_i    => phy_rst,
        tx_clk_i       => clk_ref,
        tx_data_i      => phy_tx_data,
        tx_k_i         => phy_tx_k,
        tx_disparity_o => phy_tx_disparity,
        tx_enc_err_o   => phy_tx_enc_err,
        rx_rbclk_o     => phy_rx_rbclk,
        rx_data_o      => phy_rx_data,
        rx_k_o         => phy_rx_k,
        rx_enc_err_o   => phy_rx_enc_err,
        rx_bitslide_o  => phy_rx_bitslide,
        pad_txp_o      => wr_sfp_tx_o,
        pad_rxp_i      => wr_sfp_rx_i);    
  end generate;
  
  pps_ext : gc_extend_pulse
    generic map(
      g_width => 10000000)
    port map(
      clk_i      => clk_ref,
      rst_n_i    => rstn_ref,
      pulse_i    => pps,
      extended_o => ext_pps);
  
  wr_onewire_io <= owr_pwren(0) when (owr_pwren(0) = '1' or owr_en(0) = '1') else 'Z';
  wr_sfp_scl_io <= '0' when sfp_scl_o = '0' else 'Z';
  wr_sfp_sda_io <= '0' when sfp_sda_o = '0' else 'Z';
  
  led_link_up_o  <= link_up;
  led_link_act_o <= link_act;
  led_track_o    <= tm_valid;
  led_pps_o      <= ext_pps;
  
  -- END OF White Rabbit
  ----------------------------------------------------------------------------------

  ----------------------------------------------------------------------------------
  -- Wishbone slaves ---------------------------------------------------------------
  ----------------------------------------------------------------------------------
  
  id : build_id
    port map(
      clk_i   => clk_sys,
      rst_n_i => rstn_sys,
      slave_i => dev_bus_master_o(c_devs_build_id),
      slave_o => dev_bus_master_i(c_devs_build_id));
  
  dog : watchdog
    port map(
      clk_i   => clk_sys,
      rst_n_i => rstn_sys,
      slave_i => dev_bus_master_o(c_devs_watchdog),
      slave_o => dev_bus_master_i(c_devs_watchdog));
  
  mailbox : mbox
    port map(
      clk_i        => clk_sys,
      rst_n_i      => rstn_sys,
      bus_slave_i  => dev_bus_master_o(c_devs_mbox),
      bus_slave_o  => dev_bus_master_i(c_devs_mbox),
      msi_master_o => dev_msi_slave_i (c_devs_mbox),
      msi_master_i => dev_msi_slave_o (c_devs_mbox));
  
  flash_a2 : if c_is_arria2 generate
    flash : flash_top
      generic map(
        g_family                 => "Arria II GX",
        g_port_width             => 1,   -- single-lane SPI bus
        g_addr_width             => g_flash_bits,
        g_dummy_time             => 8,   -- 8 cycles between address and data
        g_input_latch_edge       => '0', -- 30ns at 50MHz (10+20) after falling edge sets up SPI output
        g_output_latch_edge      => '1', -- falling edge to meet SPI setup times
        g_input_to_output_cycles => 4)   -- delayed to work-around unconstrained design
      port map(
        clk_i     => clk_sys,
        rstn_i    => rstn_sys,
        slave_i   => dev_bus_master_o(c_devs_flash),
        slave_o   => dev_bus_master_i(c_devs_flash),
        clk_ext_i => clk_flash_ext,
        clk_out_i => clk_flash_out,
        clk_in_i  => clk_flash_in);
  end generate;
  
  flash_a5 : if c_is_arria5 generate
    flash : flash_top
      generic map(
        g_family                 => "Arria V",
        g_port_width             => 4,  -- quad-lane SPI bus
        g_addr_width             => g_flash_bits,
        g_dummy_time             => 10,
        g_input_latch_edge       => '0',
        g_output_latch_edge      => '1',
        g_input_to_output_cycles => 4)
      port map(
        clk_i     => clk_sys,
        rstn_i    => rstn_sys,
        slave_i   => dev_bus_master_o(c_devs_flash),
        slave_o   => dev_bus_master_i(c_devs_flash),
        clk_ext_i => clk_flash_ext,
        clk_out_i => clk_flash_ext,
        clk_in_i  => clk_flash_ext);
  end generate;
  
  wb_reset : wb_arria_reset
    generic map(
      arria_family => g_family,
      rst_channels => g_lm32_cores)
    port map(
      clk_sys_i  => clk_sys,
      rstn_sys_i => rstn_sys,
      clk_upd_i  => clk_update,
      rstn_upd_i => rstn_update,
      hw_version => hw_version,
      slave_o    => dev_bus_master_i(c_devs_reset),
      slave_i    => dev_bus_master_o(c_devs_reset),
      rstn_o     => s_lm32_rstn);
  
  iocontrol : io_control
    generic map(
      g_project    => g_project,
      g_syn_target => g_family,
      g_gpio_in    => g_gpio_in,
      g_gpio_out   => g_gpio_out,
      g_gpio_inout => g_gpio_inout,
      g_lvds_in    => g_lvds_in,
      g_lvds_out   => g_lvds_out,
      g_lvds_inout => g_lvds_inout,
      g_fixed      => g_fixed,
      g_io_table   => g_io_table)
    port map(
      clk_i           => clk_sys,
      rst_n_i         => rstn_sys,
      gpio_input_i    => gpio_i(f_sub1(g_gpio_in+g_gpio_inout) downto 0),
      gpio_output_i   => s_gpio_out,
      gpio_output_o   => s_gpio_src_ioc,
      lvds_input_i    => s_lvds_vec_i(f_sub1(g_lvds_in+g_lvds_inout) downto 0),
      lvds_output_i   => lvds_dat,
      lvds_output_o   => lvds_dat_fr_ioc,
      slave_i         => dev_bus_master_o(c_devs_control),
      slave_o         => dev_bus_master_i(c_devs_control),
      gpio_oe_o       => gpio_oen_o,
      gpio_term_o     => gpio_term_o,
      gpio_spec_out_o => gpio_spec_out_o,
      gpio_spec_in_o  => gpio_spec_in_o,
      gpio_mux_o      => s_gpio_mux,
      gpio_pps_mux_o  => s_gpio_pps_mux,
      lvds_oe_o       => lvds_oen_o,
      lvds_term_o     => lvds_term_o,
      lvds_spec_out_o => lvds_spec_out_o,
      lvds_spec_in_o  => lvds_spec_in_o,
      lvds_mux_o      => s_lvds_mux,
      lvds_pps_mux_o  => s_lvds_pps_mux);
  
  lvds_vec_in_zero : if (g_lvds_inout + g_lvds_in = 0) generate
    s_lvds_vec_i <= (others => (others => '0'));
  end generate;
  
  lvds_vec_in : if (g_lvds_inout + g_lvds_in > 0) generate
    s_lvds_vec_i <= lvds_i(f_sub1(g_lvds_in+g_lvds_inout) downto 0);
  end generate;
  
  gpio_out_selector : for i in 0 to f_sub1(c_eca_gpio) generate
    s_gpio_src_butis_t0(i) <= '0' when s_gpio_mux(i)='0' else clk_butis_t0_ts;
  end generate;
  
  gpio_pps_selector : for i in 0 to f_sub1(c_eca_gpio) generate
    s_gpio_src_wr_pps(i) <= '0' when s_gpio_pps_mux(i)='0' else ext_pps;
  end generate;
  
  s_gpio_out <= s_gpio_src_eca or s_gpio_src_ioc or s_gpio_src_butis_t0 or s_gpio_src_wr_pps;
  gpio_o     <= s_gpio_out;
  
  lvds_out_selector : for i in 0 to f_sub1(c_eca_lvds) generate
    lvds_dat_fr_butis_t0(i) <= (others => clk_butis_t0_ts and s_lvds_mux(i)); -- !!! This is just a STUB and UNSAFE -> Clock domain crossing 1bit 20MHz <-> 8bit 125MHz
  end generate;
  
  lvds_pps_selector : for i in 0 to f_sub1(c_eca_lvds) generate
    lvds_dat_fr_wr_pps(i) <= (others => ext_pps and s_lvds_pps_mux(i));
  end generate;
  
  -- Instantiate SERDES clock generator
  cmp_serdes_clk_gen : xwb_serdes_clk_gen
    generic map(
      g_num_serdes_bits       => 8,
      g_selectable_duty_cycle => true,
      g_with_frac_counter     => true,
      g_num_outputs           => f_sub1(c_eca_lvds)+1)
    port map(
      clk_sys_i    => clk_sys,
      rst_sys_n_i  => rstn_sys,
      wbs_i        => dev_bus_master_o(c_devs_serdes_clk_gen),
      wbs_o        => dev_bus_master_i(c_devs_serdes_clk_gen),
      clk_ref_i    => clk_ref,
      rst_ref_n_i  => rstn_ref,
      eca_time_i   => ref_tai8ns,
      serdes_dat_o => lvds_dat_fr_clk_gen);

  -- LVDS component data input is OR between ECA chan output and SERDES clk. gen.
  gen_lvds_dat : for i in lvds_dat'range generate
    lvds_dat(i) <= lvds_dat_fr_eca_chan(i) or lvds_dat_fr_clk_gen(i) or lvds_dat_fr_ioc(i) or lvds_dat_fr_butis_t0(i) or lvds_dat_fr_wr_pps(i);
  end generate gen_lvds_dat;
  
  tlu_gpio : if (g_gpio_in + g_gpio_inout > 0) generate
   s_triggers(g_gpio_in + g_gpio_inout -1 downto 0) <= f_gpio_to_trigger_array(gpio_i);
  end generate;
   
  tlu_lvds : if (g_lvds_inout + g_lvds_in > 0) generate
   s_triggers(g_gpio_in + g_gpio_inout + g_lvds_inout + g_lvds_in -1 downto g_gpio_in + g_gpio_inout) <= f_lvds_array_to_trigger_array(lvds_i(f_sub1(g_lvds_inout+g_lvds_in) downto 0));
  end generate;
  
  tlu : wr_tlu
    generic map(
      g_num_triggers => g_gpio_in + g_gpio_inout + g_lvds_inout + g_lvds_in,
      g_fifo_depth   => g_tlu_fifo_size)
    port map(
      clk_ref_i      => clk_ref,
      rst_ref_n_i    => rstn_ref,
      clk_sys_i      => clk_sys,
      rst_sys_n_i    => rstn_sys,
      triggers_i     => s_triggers,
      tm_tai_cyc_i   => ref_tai8ns,
      ctrl_slave_i   => dev_bus_master_o(c_devs_tlu),
      ctrl_slave_o   => dev_bus_master_i(c_devs_tlu),
      irq_master_o   => dev_msi_slave_i (c_devs_tlu),
      irq_master_i   => dev_msi_slave_o (c_devs_tlu));
  
  ecawb : eca_wb_event
    port map(
      w_clk_i    => clk_sys,
      w_rst_n_i  => rstn_sys,
      w_slave_i  => top_bus_master_o(c_tops_eca_event),
      w_slave_o  => top_bus_master_i(c_tops_eca_event),
      e_clk_i    => clk_ref,
      e_rst_n_i  => rstn_ref,
      e_stream_o => s_stream_i(0),
      e_stall_i  => s_stall_o(0));
  
  ecatlu : eca_tlu
    generic map(
      g_inputs => c_tlu_io)
    port map(
      c_clk_i    => clk_sys,
      c_rst_n_i  => rstn_sys,
      c_slave_i  => dev_bus_master_o(c_devs_eca_tlu),
      c_slave_o  => dev_bus_master_i(c_devs_eca_tlu),
      a_clk_i    => clk_ref,
      a_rst_n_i  => rstn_ref,
      a_time_i   => s_time,
      a_gpio_i   => s_tlu_io,
      a_stream_o => s_stream_i(1),
      a_stall_i  => s_stall_o(1));

  eca : wr_eca
    generic map(
      g_channel_types  => c_channel_types,
      g_num_streams    => c_num_streams,
      g_num_ios        => c_eca_io,
      g_log_table_size => 8,
      g_log_queue_size => 8) -- any smaller and g_log_latency must be decreased
    port map(
      c_clk_i     => clk_sys,
      c_rst_n_i   => rstn_sys,
      c_slave_i   => dev_bus_master_o(c_devs_eca_ctl),
      c_slave_o   => dev_bus_master_i(c_devs_eca_ctl),
      a_clk_i     => clk_ref,
      a_rst_n_i   => rstn_ref,
      a_tai_i     => tm_tai,  
      a_cycles_i  => tm_cycles,
      a_time_o    => s_time,
      a_stream_i  => s_stream_i,
      a_stall_o   => s_stall_o,
      a_stall_i   => s_stall_i,
      a_channel_o => s_channel_o,
      a_io_o      => s_eca_io,
      i_clk_i     => clk_sys,  
      i_rst_n_i   => rstn_sys, 
      i_master_i  => dev_msi_slave_o(c_devs_eca_ctl),
      i_master_o  => dev_msi_slave_i(c_devs_eca_ctl));

  -- Legacy 8ns time
  ref_tai8ns <= "000" & s_time(63 downto 3);
  
  -- GPIO output from the ECA
  gpio1 : if c_eca_gpio > 0 generate
    gpio : for i in 0 to c_eca_gpio-1 generate
      s_gpio_src_eca(i) <= s_eca_io(i)(0);
    end generate;
  end generate;
  
  -- LVDS output from the ECA
  lvds1 : if c_eca_lvds > 0 generate
    lvds : for i in 0 to c_eca_lvds-1 generate
      bits : for b in 0 to 7 generate -- 0 goes first for ECA, 7 goes first for serdes
        lvds_dat_fr_eca_chan(i)(b) <= s_eca_io(i+c_eca_gpio)(7-b);
      end generate;
    end generate;
  end generate;
  
  -- GPIO input to the TLU
  gpi1 : if c_tlu_gpio > 0 generate
    gpio : for i in 0 to c_tlu_gpio-1 generate
      s_tlu_io(i) <= (others => gpio_i(i));
    end generate;
  end generate;
  
  -- LVDS input to the TLU
  lvd1 : if c_tlu_lvds > 0 generate
    lvds : for i in 0 to c_tlu_lvds-1 generate
      bits : for b in 0 to 7 generate -- 0 goes first for ECA
        s_tlu_io(i+c_tlu_gpio)(b) <= lvds_i(i)(7-b);
      end generate;
    end generate;
  end generate;
  
  c0 : eca_queue
    generic map(
      g_queue_id  => 0)
    port map(   
      a_clk_i     => clk_ref,
      a_rst_n_i   => rstn_ref,
      a_stall_o   => s_stall_i(0),
      a_channel_i => s_channel_o(0),
      q_clk_i     => clk_sys,
      q_rst_n_i   => rstn_sys,  
      q_slave_i   => dev_bus_master_o(c_devs_eca_aq),
      q_slave_o   => dev_bus_master_i(c_devs_eca_aq)); 
  
  top_msi_master_i(c_topm_eca_wbm) <= cc_dummy_slave_out; -- does not accept MSIs
  c1: eca_ac_wbm
    generic map(
      g_entries  => 16,
      g_ram_size => 128)
    port map(
      clk_ref_i   => clk_ref,
      rst_ref_n_i => rstn_ref,
      channel_i   => s_channel_o(1),
      clk_sys_i   => clk_sys,
      rst_sys_n_i => rstn_sys,
      slave_i     => dev_bus_master_o(c_devs_eca_wbm),
      slave_o     => dev_bus_master_i(c_devs_eca_wbm),
      master_o    => top_bus_slave_i(c_topm_eca_wbm),
      master_i    => top_bus_slave_o(c_topm_eca_wbm));

  c2 : eca_queue
    generic map(
      g_queue_id  => 2)
    port map(   
      a_clk_i     => clk_ref,
      a_rst_n_i   => rstn_ref,
      a_stall_o   => s_stall_i(2),
      a_channel_i => s_channel_o(2),
      q_clk_i     => clk_sys,
      q_rst_n_i   => rstn_sys,  
      q_slave_i   => dev_bus_master_o(c_devs_emb_cpu),
      q_slave_o   => dev_bus_master_i(c_devs_emb_cpu)); 
  
  eca_scu : if g_en_scubus generate
    c3 : eca_scubus_channel
      port map(
        clk_i     => clk_ref,
        rst_n_i   => rstn_ref,
        channel_i => s_channel_o(3),
        tag_valid => tag_valid,
        tag       => tag);
  end generate;
        
  lvds_pins : altera_lvds
    generic map(
      g_family  => g_family,
      g_inputs  => f_sub1(g_lvds_inout+g_lvds_in) +1,
      g_outputs => f_sub1(g_lvds_inout+g_lvds_out)+1,
      g_invert  => g_lvds_invert)
    port map(
      clk_ref_i    => clk_ref,
      rstn_ref_i   => rstn_ref,
      clk_lvds_i   => clk_lvds,
      clk_enable_i => clk_enable,
      dat_o        => lvds_i(f_sub1(g_lvds_inout+g_lvds_in) downto 0),
      lvds_p_i     => lvds_p_i,
      lvds_n_i     => lvds_n_i,
      lvds_i_led_o => lvds_i_led_o,
      dat_i        => lvds_dat(f_sub1(g_lvds_inout+g_lvds_out) downto 0),
      lvds_p_o     => lvds_p_o,
      lvds_n_o     => lvds_n_o,
      lvds_o_led_o => lvds_o_led_o);

  CfiPFlash_n : if not g_en_cfi generate
    dev_bus_master_i(c_devs_CfiPFlash) <= cc_dummy_slave_out;
  end generate;
  CfiPFlash_y : if g_en_cfi generate
    CfiPFlash: XWB_CFI_WRAPPER 
      port map(
        clk_i          => clk_sys,
        rst_n_i        => rstn_sys,
        slave_i        => dev_bus_master_o(c_devs_CfiPFlash),    -- to Slave
        slave_o        => dev_bus_master_i(c_devs_CfiPFlash),    -- to WB
        AD             => cfi_ad,
        DF             => cfi_df,
        ADV_FSH        => cfi_adv_fsh,
        nCE_FSH        => cfi_nce_fsh,
        CLK_FSH        => cfi_clk_fsh,
        nWE_FSH        => cfi_nwe_fsh,
        nOE_FSH        => cfi_noe_fsh,
        nRST_FSH       => cfi_nrst_fsh,
        WAIT_FSH       => cfi_wait_fsh);
  end generate;

  lcd_n : if not g_en_lcd generate
    dev_bus_master_i(c_devs_lcd) <= cc_dummy_slave_out;
  end generate;
  lcd_y : if g_en_lcd generate
    lcd : wb_serial_lcd
      generic map(
        g_wait => 1,
        g_hold => 15)
      port map(
        slave_clk_i  => clk_sys,
        slave_rstn_i => rstn_sys,
        slave_i      => dev_bus_master_o(c_devs_lcd),
        slave_o      => dev_bus_master_i(c_devs_lcd),
        di_clk_i     => clk_20m,
        di_scp_o     => lcd_scp,
        di_lp_o      => lcd_lp,
        di_flm_o     => lcd_flm,
        di_dat_o     => lcd_in);
    
    lcd_scp_o <= '0' when lcd_scp = '0' else 'Z';
    lcd_lp_o  <= '0' when lcd_lp  = '0' else 'Z';
    lcd_flm_o <= '0' when lcd_flm = '0' else 'Z';
    lcd_in_o  <= '0' when lcd_in  = '0' else 'Z';
  end generate;
  
  oled_n : if not g_en_oled generate
    dev_bus_master_i(c_devs_oled) <= cc_dummy_slave_out;
  end generate;
  oled_y : if g_en_oled generate
    oled : display_console
      port map(
        clk_i      => clk_sys,
        nRst_i     => rstn_sys,
        slave_i    => dev_bus_master_o(c_devs_oled),
        slave_o    => dev_bus_master_i(c_devs_oled),
        RST_DISP_o => oled_rstn_o,
        DC_SPI_o   => oled_dc_o,
        SS_SPI_o   => oled_ss_o,
        SCK_SPI_o  => oled_sck_o, 
        SD_SPI_o   => oled_sd_o,
        SH_VR_o    => oled_sh_vr_o);
  end generate;
  
  ssd1325_n : if not g_en_ssd1325 generate
    dev_bus_master_i(c_devs_ssd1325) <= cc_dummy_slave_out;
  end generate;
  ssd1325_y : if g_en_ssd1325 generate
    ssd1325_display : wb_ssd1325_serial_driver
      port map (
        clk_sys_i  => clk_sys,
        rst_n_i    => rstn_sys,
        slave_i    => dev_bus_master_o(c_devs_ssd1325),
        slave_o    => dev_bus_master_i(c_devs_ssd1325),
        ssd_rst_o  => ssd1325_rst_o,
        ssd_dc_o   => ssd1325_dc_o,    
        ssd_ss_o   => ssd1325_ss_o,    
        ssd_sclk_o => ssd1325_sclk_o,   
        ssd_data_o => ssd1325_data_o);
  end generate;

  nau8811_n : if not g_en_nau8811 generate
    dev_bus_master_i(c_devs_nau8811) <= cc_dummy_slave_out;
  end generate;
  nau8811_y : if g_en_nau8811 generate
    nau8811_audio : wb_nau8811_audio_driver
      generic map (
        g_use_external_pll => true)
      port map (
        clk_sys_i    => clk_sys,
        rst_n_i      => rstn_sys,
        pll_ref_i    => core_clk_125m_local_i,
        trigger_i    => ext_pps,
        slave_i      => dev_bus_master_o(c_devs_nau8811),
        slave_o      => dev_bus_master_i(c_devs_nau8811),
        spi_csb_o    => nau8811_spi_csb_o,
        spi_sclk_o   => nau8811_spi_sclk_o,
        spi_sdio_o   => nau8811_spi_sdio_o, 
        iis_fs_o     => nau8811_iis_fs_o,
        iis_bclk_o   => nau8811_iis_bclk_o,
        iis_adcout_o => nau8811_iis_adcout_o,
        iis_dacin_i  => nau8811_iis_dacin_i);
  end generate;
  
  scub_n : if not g_en_scubus generate
    dev_bus_master_i(c_devs_scubus)  <= cc_dummy_slave_out;
    dev_bus_master_i(c_devs_scubirq) <= cc_dummy_slave_out;
    dev_msi_slave_i (c_devs_scubirq) <= cc_dummy_master_out;
    scubus_a_d <= (others => 'Z');
  end generate;
  scub_y : if g_en_scubus generate
    scubus_a_sysclock <= clk_12_5;
    scub : wb_irq_scu_bus
      generic map(
        g_interface_mode      => PIPELINED,
        g_address_granularity => BYTE,
        clk_in_hz             => 62_500_000,
        Test                  => 0,  
        Time_Out_in_ns        => 350)
      port map(
        clk_i              => clk_sys,
        rst_n_i            => rstn_sys,
        tag                => tag,
        tag_valid          => tag_valid,
        irq_master_o       => dev_msi_slave_i (c_devs_scubirq),
        irq_master_i       => dev_msi_slave_o (c_devs_scubirq),
        ctrl_irq_o         => dev_bus_master_i(c_devs_scubirq),
        ctrl_irq_i         => dev_bus_master_o(c_devs_scubirq),
        scu_slave_o        => dev_bus_master_i(c_devs_scubus),
        scu_slave_i        => dev_bus_master_o(c_devs_scubus),
        scub_data          => scubus_a_d,
        nscub_ds           => scubus_a_nds,
        nscub_dtack        => scubus_a_ndtack,
        scub_addr          => scubus_a_a,
        scub_rdnwr         => scubus_a_rnw,
        nscub_srq_slaves   => scubus_a_nsrq,
        nscub_slave_sel    => scubus_a_nsel,
        nscub_timing_cycle => scubus_a_ntiming_cycle,
        nsel_ext_data_drv  => scubus_nsel_data_drv);
  end generate;
  
  mil_n : if not g_en_mil generate
    dev_bus_master_i(c_devs_mil)      <= cc_dummy_slave_out;
    dev_bus_master_i(c_devs_mil_ctrl) <= cc_dummy_slave_out;
    dev_msi_slave_i (c_devs_mil_ctrl) <= cc_dummy_master_out;
  end generate;
  
  mil_y : if g_en_mil generate
  
    milp : mil_pll
      port map(
        inclk0 => clk_sys1,
        c0     => mil_me_12mhz_o);

   mil_irq_inst:  wb_irq_master
    generic map(
      g_channels     => 6,        -- number of interrupt lines
      g_round_rb     => true,     -- scheduler       true: round robin,                         false: prioritised 
      g_det_edge     => true,     -- edge detection. true: trigger on rising edge of irq lines, false: trigger on high level
      g_has_dev_id   => false,    -- if set, dst adr bits 11..7 hold g_dev_id as device identifier
      g_dev_id       => (others => '0'), -- device identifier
      g_has_ch_id    => false,           -- if set, dst adr bits  6..2 hold g_ch_id  as device identifier         
      g_default_msg  => true             -- initialises msgs to a default value in order to detect uninitialised irq master
      )
    port map(
      clk_i           => clk_sys,
      rst_n_i         => rstn_sys,
      --msi if
      irq_master_o    => dev_msi_slave_i (c_devs_mil_ctrl),
      irq_master_i    => dev_msi_slave_o (c_devs_mil_ctrl),
      -- ctrl interface  
      ctrl_slave_o    => dev_bus_master_i(c_devs_mil_ctrl),       
      ctrl_slave_i    => dev_bus_master_o(c_devs_mil_ctrl),
      --irq lines
      irq_i           => (mil_every_ms_intr_o,
                          mil_ev_fifo_ne_intr_o,
                          mil_dly_intr_o,
                          mil_data_req_intr_o,
                          mil_data_rdy_intr_o,
                          mil_interlock_intr_o)
      );
     
    mil : wb_mil_scu
      generic map(
        Clk_in_Hz     => 62_500_000)
      port map(
        clk_i         => clk_sys,
        nRst_i        => rstn_sys,
        slave_i       => dev_bus_master_o(c_devs_mil),
        slave_o       => dev_bus_master_i(c_devs_mil),
        nME_BOO       => mil_nme_boo_i,
        nME_BZO       => mil_nme_bzo_i,
        ME_SD         => mil_me_sd_i,
        ME_ESC        => mil_me_esc_i,
        ME_SDI        => mil_me_sdi_o,
        ME_EE         => mil_me_ee_o,
        ME_SS         => mil_me_ss_o,
        ME_BOI        => mil_me_boi_o,
        ME_BZI        => mil_me_bzi_o,
        ME_UDI        => mil_me_udi_o,
        ME_CDS        => mil_me_cds_i,
        ME_SDO        => mil_me_sdo_i,
        ME_DSC        => mil_me_dsc_i,
        ME_VW         => mil_me_vw_i,
        ME_TD         => mil_me_td_i,
        Mil_BOI       => mil_boi_i,
        Mil_BZI       => mil_bzi_i,
        Sel_Mil_Drv   => mil_sel_drv_o,
        nSel_Mil_Rcv  => mil_nsel_rcv_o,
        Mil_nBOO      => mil_nboo_o,
        Mil_nBZO      => mil_nbzo_o,
        nLed_Mil_Rcv  => mil_nled_rcv_o,
        nLed_Mil_Trm  => mil_nled_trm_o,
        nLed_Mil_Err  => mil_nled_err_o,
        error_limit_reached => open,
        Mil_Decoder_Diag_p  => open,
        Mil_Decoder_Diag_n  => open,
        timing         => mil_timing_i,
        dly_intr_o     => mil_dly_intr_o,
        nLed_Timing    => mil_nled_timing_o,
        nLed_Fifo_ne   => mil_nled_fifo_ne_o,
        ev_fifo_ne_intr_o => mil_ev_fifo_ne_intr_o,
        Interlock_Intr_i => mil_interlock_intr_i,
        Data_Rdy_Intr_i  => mil_data_rdy_intr_i,
        Data_Req_Intr_i  => mil_data_req_intr_i,
        Interlock_Intr_o => mil_interlock_intr_o,
        Data_Rdy_Intr_o  => mil_data_rdy_intr_o,
        Data_Req_Intr_o  => mil_data_req_intr_o,
        nLed_Interl    => mil_nled_interl_o,
        nLed_drq       => mil_nled_drq_o,
        nLed_dry       => mil_nled_dry_o,
        every_ms_intr_o => mil_every_ms_intr_o,
        lemo_data_o     => mil_lemo_data_o,
        lemo_nled_o     => mil_lemo_nled_o, 
	     lemo_out_en_o   => mil_lemo_out_en_o,     
        lemo_data_i     => mil_lemo_data_i, 
--        io_1           => mil_io1_o,
--        io_1_is_in     => mil_io1_is_in_o,
--        nLed_io_1      => mil_nled_io1_o,
--        io_2           => mil_io2_o,
--        io_2_is_in     => mil_io2_is_in_o,
--        nLed_io_2      => mil_nled_io2_o,
        nsig_wb_err    => open);
  end generate;
  
  
  ow_n : if not g_en_user_ow generate
    dev_bus_master_i(c_devs_ow) <= cc_dummy_slave_out;
  end generate;
  ow_y : if g_en_user_ow generate
    ow_io(0) <= user_ow_pwren(0) when (user_ow_pwren(0) = '1' or user_ow_en(0) = '1') else 'Z';
    ow_io(1) <= user_ow_pwren(1) when (user_ow_pwren(1) = '1' or user_ow_en(1) = '1') else 'Z';
    ONEWIRE : xwb_onewire_master
      generic map(
        g_interface_mode      => PIPELINED,
        g_address_granularity => BYTE,
        g_num_ports           => 2,
        g_ow_btp_normal       => "5.0",
        g_ow_btp_overdrive    => "1.0")
      port map(
        clk_sys_i   => clk_sys,
        rst_n_i     => rstn_sys,
        slave_i     => dev_bus_master_o(c_devs_ow),
        slave_o     => dev_bus_master_i(c_devs_ow),
        desc_o      => open,
        owr_pwren_o => user_ow_pwren,
        owr_en_o    => user_ow_en,
        owr_i       => ow_io);
  end generate;
  
  psram_n : if not g_en_psram generate
    dev_bus_master_i(c_devs_psram) <= cc_dummy_slave_out;
  end generate;
  psram_y : if g_en_psram generate
    ram : psram
      generic map(
        g_bits => g_psram_bits)
      port map(
      clk_i     => clk_sys,
      rstn_i    => rstn_sys,
      slave_i   => dev_bus_master_o(c_devs_psram),
      slave_o   => dev_bus_master_i(c_devs_psram),
      ps_clk    => ps_clk,
      ps_addr   => ps_addr,
      ps_data   => ps_data,
      ps_seln   => ps_seln,
      ps_cen    => ps_cen,
      ps_oen    => ps_oen,
      ps_wen    => ps_wen,
      ps_cre    => ps_cre,
      ps_advn   => ps_advn,
      ps_wait   => ps_wait);
  end generate;
 
  -- END OF Wishbone slaves
  ----------------------------------------------------------------------------------
  
end rtl;
