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
use work.wr_fabric_pkg.all;
use work.wishbone_pkg.all;
use work.eca_pkg.all;
use work.wb_cores_pkg_gsi.all;
use work.pcie_wb_pkg.all;
use work.wr_altera_pkg.all;
use work.etherbone_pkg.all;
use work.scu_bus_pkg.all;
use work.altera_flash_pkg.all;
use work.altera_networks_pkg.all;
use work.build_id_pkg.all;
use work.oled_display_pkg.all;
use work.lpc_uart_pkg.all;
use work.wb_irq_pkg.all;
use work.ftm_pkg.all;
use work.ez_usb_pkg.all;
use work.wb_arria_reset_pkg.all;
use work.xvme64x_pack.all;
use work.VME_Buffer_pack.all;
use work.wb_mil_scu_pkg.all;

entity monster is
  generic(
    g_family      : string; -- "Arria II" or "Arria V"
    g_project     : string;
    g_inputs      : natural;
    g_outputs     : natural;
    g_flash_bits  : natural;
    g_ram_size    : natural;
    g_pll_skew    : natural; -- (ref-tx) in ps
    g_en_pcie     : boolean;
    g_en_vme      : boolean;
    g_en_usb      : boolean;
    g_en_scubus   : boolean;
    g_en_mil      : boolean;
    g_en_oled     : boolean;
    g_en_lcd      : boolean);
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
    core_rstn_wr_ref_o     : out   std_logic;
    core_rstn_butis_o      : out   std_logic;
    core_debug_o           : out   std_logic_vector(15 downto 0) := (others => 'Z');
    -- GPIO for the board
    gpio_o                 : out   std_logic_vector(g_outputs-1 downto 0);
    gpio_i                 : in    std_logic_vector(g_inputs-1  downto 0);
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
    mil_io1_o              : out   std_logic := 'Z';
    mil_io1_is_in_o        : out   std_logic := 'Z';
    mil_nled_io1_o         : out   std_logic := 'Z';
    mil_io2_o              : out   std_logic := 'Z';
    mil_io2_is_in_o        : out   std_logic := 'Z';
    mil_nled_io2_o         : out   std_logic := 'Z';
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
    lcd_in_o               : out   std_logic := 'Z');
end monster;

architecture rtl of monster is

  ----------------------------------------------------------------------------------
  -- Platform dependant parameters -------------------------------------------------
  ----------------------------------------------------------------------------------
  
  function f_pll_select return natural_vector is
    -- Note: for arria5 you need to set location assignments to make this true!
    constant c_a2_pll_select : natural_vector := (0 =>  2, 1=>  3, 2 =>  4);
    constant c_a5_pll_select : natural_vector := (0 => 12, 1=> 13, 2 => 14);
    constant c_error         : natural_vector := (0 => 0);
  begin
    if    g_family = "Arria II" then return c_a2_pll_select;
    elsif g_family = "Arria V"  then return c_a5_pll_select;
    else                             return c_error;
    end if;
  end f_pll_select;
  
  ----------------------------------------------------------------------------------
  -- MSI IRQ Crossbar --------------------------------------------------------------
  ----------------------------------------------------------------------------------
  constant c_irq_masters : natural := 4;
  constant c_irqm_top    : natural := 0;
  constant c_irqm_eca    : natural := 1;
  constant c_irqm_aq     : natural := 2;
  constant c_irqm_scubus : natural := 3;
  
  constant c_irq_slaves  : natural := 3;
  constant c_irqs_lm32   : natural := 0;
  constant c_irqs_pcie   : natural := 1;
  constant c_irqs_vme    : natural := 2;
  
  constant c_irq_layout_req : t_sdb_record_array(c_irq_slaves-1 downto 0) :=
   (c_irqs_lm32 => f_sdb_auto_device(c_irq_ep_sdb,   true),
    c_irqs_pcie => f_sdb_auto_device(c_msi_pcie_sdb, g_en_pcie),
    c_irqs_vme  => f_sdb_auto_device(c_vme_msi_sdb,  g_en_vme));
  
  constant c_irq_layout      : t_sdb_record_array(c_irq_slaves-1 downto 0) 
                                                  := f_sdb_auto_layout(c_irq_layout_req);
  constant c_irq_sdb_address : t_wishbone_address := f_sdb_auto_sdb(c_irq_layout_req);
  constant c_irq_bridge_sdb  : t_sdb_bridge       := f_xwb_bridge_layout_sdb(true, c_irq_layout, c_irq_sdb_address);

  signal irq_cbar_slave_i  : t_wishbone_slave_in_array  (c_irq_masters-1 downto 0);
  signal irq_cbar_slave_o  : t_wishbone_slave_out_array (c_irq_masters-1 downto 0);
  signal irq_cbar_master_i : t_wishbone_master_in_array (c_irq_slaves -1 downto 0);
  signal irq_cbar_master_o : t_wishbone_master_out_array(c_irq_slaves -1 downto 0);

  -- END OF MSI IRQ Crossbar
  ----------------------------------------------------------------------------------
  
  ----------------------------------------------------------------------------------
  -- GSI Top Crossbar --------------------------------------------------------------
  ----------------------------------------------------------------------------------
  constant c_top_masters    : natural := 5;
  constant c_topm_ebs       : natural := 0;
  constant c_topm_lm32      : natural := 1;
  constant c_topm_pcie      : natural := 2;
  constant c_topm_vme       : natural := 3;
  constant c_topm_usb       : natural := 4;
  
  -- required slaves
  constant c_top_slaves     : natural := 15;
  constant c_tops_irq       : natural := 0;
  constant c_tops_wrc       : natural := 1;
  constant c_tops_lm32_ram  : natural := 2;
  constant c_tops_build_id  : natural := 3;
  constant c_tops_flash     : natural := 4;
  constant c_tops_reset     : natural := 5;
  constant c_tops_ebm       : natural := 6;
  constant c_tops_tlu       : natural := 7;
  constant c_tops_eca_ctl   : natural := 8;
  constant c_tops_eca_event : natural := 9;
  constant c_tops_eca_aq    : natural := 10;
  -- !!! missing IO configuration slave
  -- optional slaves:
  constant c_tops_lcd       : natural := 11;
  constant c_tops_oled      : natural := 12;
  constant c_tops_scubus    : natural := 13;
  constant c_tops_mil       : natural := 14;
  
  -- We have to specify the values for WRC as there is no generic out in vhdl
  constant c_wrcore_bridge_sdb : t_sdb_bridge := f_xwb_bridge_manual_sdb(x"0003ffff", x"00030000");
  
  constant c_top_layout_req : t_sdb_record_array(c_top_slaves-1 downto 0) :=
   (c_tops_irq       => f_sdb_auto_bridge(c_irq_bridge_sdb,                 true),
    c_tops_wrc       => f_sdb_auto_bridge(c_wrcore_bridge_sdb,              true),
    c_tops_lm32_ram  => f_sdb_auto_device(f_xwb_dpram(g_ram_size/4),        true),
    c_tops_build_id  => f_sdb_auto_device(c_build_id_sdb,                   true),
    c_tops_flash     => f_sdb_auto_device(f_wb_spi_flash_sdb(g_flash_bits), true),
    c_tops_reset     => f_sdb_auto_device(c_arria_reset,                    true),
    c_tops_ebm       => f_sdb_auto_device(c_ebm_sdb,                        true),
    c_tops_tlu       => f_sdb_auto_device(c_xwr_wb_timestamp_latch_sdb,     true),
    c_tops_eca_ctl   => f_sdb_auto_device(c_eca_sdb,                        true),
    c_tops_eca_event => f_sdb_embed_device(c_eca_event_sdb, x"7FFFFFF0"), -- must be located at fixed address
    c_tops_eca_aq    => f_sdb_auto_device(c_eca_queue_sdb,                  true),
    c_tops_lcd       => f_sdb_auto_device(c_wb_serial_lcd_sdb,              g_en_lcd),
    c_tops_oled      => f_sdb_auto_device(c_oled_display,                   g_en_oled),
    c_tops_scubus    => f_sdb_auto_device(c_scu_bus_master,                 g_en_scubus),
    c_tops_mil       => f_sdb_auto_device(c_xwb_gsi_mil_scu,                g_en_mil));
    
  constant c_top_layout      : t_sdb_record_array(c_top_slaves-1 downto 0) 
                                                  := f_sdb_auto_layout(c_top_layout_req);
  constant c_top_sdb_address : t_wishbone_address := f_sdb_auto_sdb(c_top_layout_req);
  constant c_top_bridge_sdb  : t_sdb_bridge       := f_xwb_bridge_layout_sdb(true, c_top_layout, c_top_sdb_address);
  
  signal top_cbar_slave_i  : t_wishbone_slave_in_array (c_top_masters-1 downto 0);
  signal top_cbar_slave_o  : t_wishbone_slave_out_array(c_top_masters-1 downto 0);
  signal top_cbar_master_i : t_wishbone_master_in_array(c_top_slaves-1 downto 0);
  signal top_cbar_master_o : t_wishbone_master_out_array(c_top_slaves-1 downto 0);  
  
  -- END OF GSI Top Crossbar
  ----------------------------------------------------------------------------------     

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
  
  signal clk_sys          : std_logic;
  signal clk_reconf       : std_logic; -- 50MHz on arrai2, 100MHz on arria5
  signal clk_flash        : std_logic; -- for now, the same as clk_reconf
  signal clk_20m          : std_logic;
  signal clk_update       : std_logic;
  signal rstn_sys         : std_logic;
  signal rstn_update      : std_logic;
  
  -- Ref PLL from clk_125m_pllref_i
  signal ref_locked       : std_logic;
  signal clk_ref0         : std_logic;
  signal clk_ref1         : std_logic;
  signal clk_ref2         : std_logic;
  
  signal clk_ref          : std_logic;
  signal clk_butis        : std_logic;
  signal clk_phase        : std_logic;
  signal clk_12_5         : std_logic;
  signal rstn_ref         : std_logic;
  signal rstn_butis       : std_logic;
  signal rstn_phase       : std_logic;
  
  signal phase_done       : std_logic;
  signal phase_step       : std_logic;
  signal phase_sel        : std_logic_vector(4 downto 0);
  
  signal phase_butis      : phase_offset;
  
  -- DMTD PLL from clk_20m_vcxo_i
  signal dmtd_locked      : std_logic;
  signal clk_dmtd0        : std_logic;
  signal clk_dmtd         : std_logic;
  
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
  
  signal s_lm32_rstn : std_logic;

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
  
  signal sys_tai8ns : t_time;
  signal ref_tai8ns : t_time;

  signal owr_pwren : std_logic_vector(1 downto 0);
  signal owr_en    : std_logic_vector(1 downto 0);
  
  signal sfp_scl_o : std_logic;
  signal sfp_sda_o : std_logic;
  
  signal channels : t_channel_array(1 downto 0);
  
  -- END OF White Rabbit
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
  
  signal lcd_scp : std_logic;
  signal lcd_lp  : std_logic;
  signal lcd_flm : std_logic;
  signal lcd_in  : std_logic;
  signal gpio    : std_logic_vector(15 downto 0);
  
begin

  ----------------------------------------------------------------------------------
  -- Reset and PLLs ----------------------------------------------------------------
  ----------------------------------------------------------------------------------
  
  -- We need at least one off-chip free running clock to setup PLLs
  free_a5 : if g_family = "Arria V" generate
    clk_free <= core_clk_125m_local_i;
  end generate;
  free_a2 : if g_family = "Arria II" generate
    clk_free <= core_clk_20m_vcxo_i; -- (125MHz is too fast)
  end generate;
  
  reset : altera_reset
    generic map(
      g_clocks => 3)
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
      rstn_o(0)     => rstn_free,
      rstn_o(1)     => rstn_sys,
      rstn_o(2)     => rstn_update);

  dmtd_a2 : if g_family = "Arria II" generate
    dmtd_inst : dmtd_pll port map(
      areset   => pll_rst,
      inclk0   => core_clk_20m_vcxo_i,    --  20  Mhz 
      c0       => clk_dmtd0,              --  62.5MHz
      locked   => dmtd_locked);
  end generate;
  dmtd_a5 : if g_family = "Arria V" generate
    dmtd_inst : dmtd_pll5 port map(
      rst      => pll_rst,
      refclk   => core_clk_20m_vcxo_i,    --  20  MHz
      outclk_0 => clk_dmtd0,              --  62.5MHz
      locked   => dmtd_locked);
  end generate;
  
  dmtd_clk : single_region port map(
    inclk  => clk_dmtd0,
    outclk => clk_dmtd);
  
  sys_a2 : if g_family = "Arria II" generate
    sys_inst : sys_pll port map(
      areset => pll_rst,
      inclk0 => core_clk_125m_local_i, -- 125  Mhz 
      c0     => clk_sys0,         --  62.5 MHz
      c1     => clk_sys1,         --  50  Mhz
      c2     => clk_sys2,         --  20  MHz
      c3     => clk_sys3,         --  10  MHz
      locked => sys_locked);
  end generate;
  sys_a5 : if g_family = "Arria V" generate
    sys_inst : sys_pll5 port map(
      rst      => pll_rst,
      refclk   => core_clk_125m_local_i, -- 125  Mhz 
      outclk_0 => clk_sys0,           --  62.5MHz
      outclk_1 => clk_sys1,           -- 100  MHz
      outclk_2 => clk_sys2,           --  20  MHz
      outclk_3 => clk_sys3,           --  10  MHz
      locked   => sys_locked);
  end generate;
  
  sys_clk : global_region port map(
    inclk  => clk_sys0,
    outclk => clk_sys);
  
  reconf_clk : global_region port map(
    inclk  => clk_sys1,
    outclk => clk_reconf);
  
  clk_flash <= clk_reconf;
  
  c20m_clk : single_region port map(
    inclk  => clk_sys2,
    outclk => clk_20m);
  
  update_clk : single_region port map(
    inclk  => clk_sys3,
    outclk => clk_update);
  
  ref_a2 : if g_family = "Arria II" generate
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
  ref_a5 : if g_family = "Arria V" generate
    ref_inst : ref_pll5 port map(
      rst        => pll_rst,
      refclk     => core_clk_125m_pllref_i, -- 125 MHz
      outclk_0   => clk_ref0,         -- 125 MHz
      outclk_1   => clk_ref1,         -- 200 MHz
      outclk_2   => clk_ref2,         --  25 MHz
      locked     => ref_locked,
      scanclk    => clk_free,  
      cntsel     => phase_sel, 
      phase_en   => phase_step,
      updn       => '1',              -- positive phase shift (widen period)
      phase_done => phase_done);
  end generate;
  
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

  phase : altera_phase
    generic map(
      g_select_bits   => 5,
      g_outputs       => 3,
      g_base          => g_pll_skew,
      g_vco_freq      => 1000, -- 1GHz
      g_output_freq   => (0 => 125, 1 => 200, 2 => 25),
      g_output_select => f_pll_select)
    port map(
      clk_i       => clk_free,
      rstn_i      => rstn_free,
      clks_i(0)   => clk_ref,
      clks_i(1)   => clk_butis,
      clks_i(2)   => clk_phase,
      rstn_o(0)   => rstn_ref,
      rstn_o(1)   => rstn_butis,
      rstn_o(2)   => rstn_phase,
      offset_i(0) => (others => '0'),
      offset_i(1) => phase_butis,
      offset_i(2) => (others => '0'),
      phasedone_i => phase_done,
      phasesel_o  => phase_sel,
      phasestep_o => phase_step);
  
  butis : altera_butis
    port map(
      clk_ref_i => clk_ref,
      clk_25m_i => clk_phase,
      pps_i     => pps,
      phase_o   => phase_butis);
  
  core_clk_wr_ref_o  <= clk_ref;
  core_clk_butis_o   <= clk_butis;
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
      slave_i       => top_cbar_slave_i,
      slave_o       => top_cbar_slave_o,
      master_i      => top_cbar_master_i,
      master_o      => top_cbar_master_o);
  
  irq_bar : xwb_sdb_crossbar
    generic map(
      g_num_masters => c_irq_masters,
      g_num_slaves  => c_irq_slaves,
      g_registered  => true,
      g_wraparound  => true,
      g_layout      => c_irq_layout,
      g_sdb_addr    => c_irq_sdb_address)
    port map(
      clk_sys_i     => clk_sys,
      rst_n_i       => rstn_sys,
      slave_i       => irq_cbar_slave_i,
      slave_o       => irq_cbar_slave_o,
      master_i      => irq_cbar_master_i,
      master_o      => irq_cbar_master_o);
  
  top2irq : xwb_register_link
    port map(
      clk_sys_i     => clk_sys,
      rst_n_i       => rstn_sys,
      slave_i       => top_cbar_master_o(c_tops_irq),
      slave_o       => top_cbar_master_i(c_tops_irq),
      master_i      => irq_cbar_slave_o (c_irqm_top),
      master_o      => irq_cbar_slave_i (c_irqm_top));
  
  top2wrc : xwb_register_link
    port map(
      clk_sys_i     => clk_sys,
      rst_n_i       => rstn_sys,
      slave_i       => top_cbar_master_o(c_tops_wrc),
      slave_o       => top_cbar_master_i(c_tops_wrc),
      master_i      => wrc_slave_o,
      master_o      => wrc_slave_i);
  
  -- END OF Wishbone crossbars
  ----------------------------------------------------------------------------------

  ----------------------------------------------------------------------------------
  -- Wishbone masters --------------------------------------------------------------
  
  eb : eb_master_slave_wrapper
    generic map(
      g_with_master     => true,
      g_ebs_sdb_address => (x"00000000" & c_top_sdb_address),
      g_ebm_adr_bits_hi => 10)
    port map(
      clk_i           => clk_sys,
      nRst_i          => rstn_sys,
      snk_i           => eb_snk_in,
      snk_o           => eb_snk_out,
      src_o           => eb_src_out,
      src_i           => eb_src_in,
      ebs_cfg_slave_o => wrc_master_i,
      ebs_cfg_slave_i => wrc_master_o,
      ebs_wb_master_o => top_cbar_slave_i (c_topm_ebs),
      ebs_wb_master_i => top_cbar_slave_o (c_topm_ebs),
      ebm_wb_slave_i  => top_cbar_master_o(c_tops_ebm),
      ebm_wb_slave_o  => top_cbar_master_i(c_tops_ebm));
  
  lm32 : ftm_lm32
    generic map(
      g_size       => g_ram_size/4,
      g_bridge_sdb => c_top_bridge_sdb,
      g_init_file  => g_project & ".mif",
      g_msi_queues => 1)
    port map(
      clk_sys_i       => clk_sys,
      rst_n_i         => rstn_sys,
      rst_lm32_n_i    => s_lm32_rstn,
      tm_tai8ns_i     => sys_tai8ns,
      lm32_master_o   => top_cbar_slave_i (c_topm_lm32),
      lm32_master_i   => top_cbar_slave_o (c_topm_lm32),
      irq_slaves_o(0) => irq_cbar_master_i(c_irqs_lm32),
      irq_slaves_i(0) => irq_cbar_master_o(c_irqs_lm32),
      ram_slave_o     => top_cbar_master_i(c_tops_lm32_ram),
      ram_slave_i     => top_cbar_master_o(c_tops_lm32_ram));
  
  pcie_n : if not g_en_pcie generate
    top_cbar_slave_i (c_topm_pcie) <= cc_dummy_master_out;
    irq_cbar_master_i(c_irqs_pcie) <= cc_dummy_slave_out;
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
        master_o      => top_cbar_slave_i (c_topm_pcie),
        master_i      => top_cbar_slave_o (c_topm_pcie),
        slave_clk_i   => clk_sys,
        slave_rstn_i  => rstn_sys,
        slave_i       => irq_cbar_master_o(c_irqs_pcie),
        slave_o       => irq_cbar_master_i(c_irqs_pcie));
  end generate;
  
  vme_n : if not g_en_vme generate
    top_cbar_slave_i (c_topm_vme) <= cc_dummy_master_out;
    irq_cbar_master_i(c_irqs_vme) <= cc_dummy_slave_out;
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
        g_sdb_addr 		 => c_top_sdb_address,
        g_irq_src        => MSI)
       port map(
        clk_i           => clk_sys,
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
        reset_o         => open, -- => s_rst,
        master_o        => top_cbar_slave_i(c_topm_vme),
        master_i        => top_cbar_slave_o(c_topm_vme),
        slave_o         => irq_cbar_master_i(c_irqs_vme),
        slave_i         => irq_cbar_master_o(c_irqs_vme), 
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
  
  usb_n : if not g_en_usb generate
    top_cbar_slave_i (c_topm_usb) <= cc_dummy_master_out;
    uart_usb <= '0';
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
        master_i  => top_cbar_slave_o(c_topm_usb),
        master_o  => top_cbar_slave_i(c_topm_usb),
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
  uart_mux <= uart_usb or wr_uart_i;
  
  -- END OF Wishbone masters
  ----------------------------------------------------------------------------------
  
  ----------------------------------------------------------------------------------
  -- White Rabbit ------------------------------------------------------------------
  ----------------------------------------------------------------------------------
  
  U_WR_CORE : xwr_core
    generic map (
      g_simulation                => 0,
      g_phys_uart                 => true,
      g_virtual_uart              => false,
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
      phy_tx_k_o           => phy_tx_k,
      phy_tx_disparity_i   => phy_tx_disparity,
      phy_tx_enc_err_i     => phy_tx_enc_err,
      phy_rx_data_i        => phy_rx_data,
      phy_rx_rbclk_i       => phy_rx_rbclk,
      phy_rx_k_i           => phy_rx_k,
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

  phy_a2 : if g_family = "Arria II" generate
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
  
  phy_a5 : if g_family = "Arria V" generate
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
  
  sys_time : time_clk_cross
    port map(
      clk_ref_i           => clk_ref,
      rst_ref_n_i         => rstn_ref,
      clk_sys_i           => clk_sys,
      rst_sys_n_i         => rstn_sys,
      tm_time_valid_i     => tm_valid,
      tm_tai_i            => tm_tai,
      tm_cycles_i         => tm_cycles,
      tm_ref_tai_cycles_o => ref_tai8ns,
      tm_sys_tai_cycles_o => sys_tai8ns);

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
      slave_i => top_cbar_master_o(c_tops_build_id),
      slave_o => top_cbar_master_i(c_tops_build_id));
  
  flash_a2 : if g_family = "Arria II" generate
    flash : flash_top
      generic map(
        g_family                 => "Arria II GX",
        g_port_width             => 1,   -- single-lane SPI bus
        g_addr_width             => g_flash_bits,
        g_dummy_time             => 8,   -- 8 cycles between address and data
        g_input_latch_edge       => '0', -- 30ns at 50MHz (10+20) after falling edge sets up SPI output
        g_output_latch_edge      => '1', -- falling edge to meet SPI setup times
        g_input_to_output_cycles => 2)   -- delayed to work-around unconstrained design
      port map(
        clk_i     => clk_sys,
        rstn_i    => rstn_sys,
        slave_i   => top_cbar_master_o(c_tops_flash),
        slave_o   => top_cbar_master_i(c_tops_flash),
        clk_ext_i => clk_flash,
        clk_out_i => clk_flash,
        clk_in_i  => clk_flash);
  end generate;
  flash_a5 : if g_family = "Arria V" generate
    flash : flash_top
      generic map(
        g_family                 => "Arria V",
        g_port_width             => 4,  -- quad-lane SPI bus
        g_addr_width             => g_flash_bits,
        g_dummy_time             => 10,
        g_config                 => true,
        g_input_latch_edge       => '1',
        g_output_latch_edge      => '1',
        g_input_to_output_cycles => 3)
      port map(
        clk_i     => clk_sys,
        rstn_i    => rstn_sys,
        slave_i   => top_cbar_master_o(c_tops_flash),
        slave_o   => top_cbar_master_i(c_tops_flash),
        clk_ext_i => clk_flash,
        clk_out_i => clk_flash,
        clk_in_i  => clk_flash);
  end generate;
  
  wb_reset : wb_arria_reset
    generic map(
      arria_family => g_family,
      rst_channels => 1)
    port map(
      clk_sys_i  => clk_sys,
      rstn_sys_i => rstn_sys,
      clk_upd_i  => clk_update,
      rstn_upd_i => rstn_update,
      slave_o    => top_cbar_master_i(c_tops_reset),
      slave_i    => top_cbar_master_o(c_tops_reset),
      rstn_o(0)  => s_lm32_rstn);
  
  tlu : wb_timestamp_latch
    generic map(
      g_num_triggers => g_inputs,
      g_fifo_depth   => 10)
    port map(
      ref_clk_i       => clk_ref,
      ref_rstn_i      => rstn_ref,
      sys_clk_i       => clk_sys,
      sys_rstn_i      => rstn_sys,
      triggers_i      => gpio_i,
      tm_time_valid_i => tm_valid, 
      tm_tai_i        => tm_tai,  
      tm_cycles_i     => tm_cycles,
      wb_slave_i      => top_cbar_master_o(c_tops_tlu),
      wb_slave_o      => top_cbar_master_i(c_tops_tlu));
  
  eca : wr_eca
    generic map(
      g_eca_name      => f_name(g_project & " top"),
      g_channel_names => (f_name("GPIO: output triggers"),
                          f_name("RTOS: Action Queue")),
      g_log_table_size => 7,
      g_log_queue_len  => 8,
      g_num_channels   => 2,
      g_num_streams    => 1)
    port map(
      e_clk_i  (0)=> clk_sys,
      e_rst_n_i(0)=> rstn_sys,
      e_slave_i(0)=> top_cbar_master_o(c_tops_eca_event),
      e_slave_o(0)=> top_cbar_master_i(c_tops_eca_event),
      c_clk_i     => clk_sys,
      c_rst_n_i   => rstn_sys,
      c_slave_i   => top_cbar_master_o(c_tops_eca_ctl),
      c_slave_o   => top_cbar_master_i(c_tops_eca_ctl),
      a_clk_i     => clk_ref,
      a_rst_n_i   => rstn_ref,
      a_tai_i     => tm_tai,  
      a_cycles_i  => tm_cycles,
      a_channel_o => channels, 
      i_clk_i     => clk_sys,  
      i_rst_n_i   => rstn_sys, 
      i_master_i  => irq_cbar_slave_o(c_irqm_eca),
      i_master_o  => irq_cbar_slave_i(c_irqm_eca));
    
  c0 : eca_gpio_channel
    port map(
      clk_i     => clk_ref,
      rst_n_i   => rstn_ref,
      channel_i => channels(0),
      gpio_o    => gpio);
  gpio_o <= gpio(gpio_o'range);
  
  c1 : eca_queue_channel
    port map(   
      a_clk_i     => clk_ref,
      a_rst_n_i   => rstn_ref,
      a_channel_i => channels(1),
      i_clk_i     => clk_sys,
      i_rst_n_i   => rstn_sys,
      i_master_o  => irq_cbar_slave_i(c_irqm_aq),
      i_master_i  => irq_cbar_slave_o(c_irqm_aq),
      q_clk_i     => clk_sys,
      q_rst_n_i   => rstn_sys,  
      q_slave_i   => top_cbar_master_o(c_tops_eca_aq),
      q_slave_o   => top_cbar_master_i(c_tops_eca_aq)); 
  
  lcd_n : if not g_en_lcd generate
    top_cbar_master_i(c_tops_lcd) <= cc_dummy_slave_out;
  end generate;
  lcd_y : if g_en_lcd generate
    lcd : wb_serial_lcd
      generic map(
        g_wait => 1,
        g_hold => 15)
      port map(
        slave_clk_i  => clk_sys,
        slave_rstn_i => rstn_sys,
        slave_i      => top_cbar_master_o(c_tops_lcd),
        slave_o      => top_cbar_master_i(c_tops_lcd),
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
    top_cbar_master_i(c_tops_oled) <= cc_dummy_slave_out;
  end generate;
  oled_y : if g_en_oled generate
    oled : display_console
      port map(
        clk_i      => clk_sys,
        nRst_i     => rstn_sys,
        slave_i    => top_cbar_master_o(c_tops_oled),
        slave_o    => top_cbar_master_i(c_tops_oled),
        RST_DISP_o => oled_rstn_o,
        DC_SPI_o   => oled_dc_o,
        SS_SPI_o   => oled_ss_o,
        SCK_SPI_o  => oled_sck_o, 
        SD_SPI_o   => oled_sd_o,
        SH_VR_o    => oled_sh_vr_o);
  end generate;
  
  scub_n : if not g_en_scubus generate
    top_cbar_master_i(c_tops_scubus) <= cc_dummy_slave_out;
    irq_cbar_slave_i (c_irqm_scubus) <= cc_dummy_master_out;
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
        clk_i    => clk_sys,
        rst_n_i  => rstn_sys,
        irq_master_o       => irq_cbar_slave_i (c_irqm_scubus),
        irq_master_i       => irq_cbar_slave_o (c_irqm_scubus),
        scu_slave_o        => top_cbar_master_i(c_tops_scubus),
        scu_slave_i        => top_cbar_master_o(c_tops_scubus),
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
    top_cbar_master_i(c_tops_mil) <= cc_dummy_slave_out;
  end generate;
  mil_y : if g_en_mil generate
    milp : mil_pll
      port map(
        inclk0 => clk_sys1,
        c0     => mil_me_12mhz_o);
      
    mil : wb_mil_scu
      generic map(
        Clk_in_Hz     => 62_500_000)
      port map(
        clk_i         => clk_sys,
        nRst_i        => rstn_sys,
        slave_i       => top_cbar_master_o(c_tops_mil),
        slave_o       => top_cbar_master_i(c_tops_mil),
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
        nLed_Timing    => mil_nled_timing_o,
        nLed_Fifo_ne   => mil_nled_fifo_ne_o,
        Interlock_Intr => mil_interlock_intr_i,
        Data_Rdy_Intr  => mil_data_rdy_intr_i,
        Data_Req_Intr  => mil_data_req_intr_i,
        nLed_Interl    => mil_nled_interl_o,
        nLed_drq       => mil_nled_drq_o,
        nLed_dry       => mil_nled_dry_o,
        io_1           => mil_io1_o,
        io_1_is_in     => mil_io1_is_in_o,
        nLed_io_1      => mil_nled_io1_o,
        io_2           => mil_io2_o,
        io_2_is_in     => mil_io2_is_in_o,
        nLed_io_2      => mil_nled_io2_o,
        nsig_wb_err    => open);
  end generate;
  
  -- END OF Wishbone slaves
  ----------------------------------------------------------------------------------
  
end rtl;
