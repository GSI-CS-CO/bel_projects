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
use work.wr_altera_pkg.all;
use work.etherbone_pkg.all;
use work.altera_flash_pkg.all;
use work.ez_usb_pkg.all;
use work.xvme64x_pack.all;
use work.VME_Buffer_pack.all;
use work.build_id_pkg.all;
use work.altera_networks_pkg.all;

entity vetar_top is
  port(
    clk_20m_vcxo_i    	: in std_logic;  -- 20MHz VCXO clock
    clk_125m_pllref_i 	: in std_logic;  -- 125 MHz PLL reference
    clk_125m_local_i  	: in std_logic;  -- local clk from 125Mhz oscillator
    clk_pll_o           : out std_logic; -- clock pll output

    
    ------------------------------------------------------------------------
    -- WR DAC signals
    ------------------------------------------------------------------------
    dac_sclk         : out std_logic;
    dac_din          : out std_logic;
    ndac_cs          : out std_logic_vector(2 downto 1);
    
    -----------------------------------------
    -- LEMO on front panel NIM/TTL
    -----------------------------------------
    lemo_i           	: in std_logic;
    lemo_o           	: out std_logic;
    lemo_o_en_o      	: out std_logic;

    -----------------------------------------------------------------------
    -- LEDs on VETAR carrier
    -----------------------------------------------------------------------
    leds_o           : out std_logic_vector(15 downto 0);
    
    -----------------------------------------------------------------------
    -- OneWire
    -----------------------------------------------------------------------
    OneWire_CB       : inout std_logic;
    
    -----------------------------------------------------------------------
    -- Timing SFP 
    -----------------------------------------------------------------------
    sfp_ref_clk_i    : in  std_logic;
    
    sfp_tx_disable_o : out std_logic := '0';
    sfp_txp_o        : out std_logic;
    sfp_rxp_i        : in  std_logic;
    
    sfp_mod0         : in    std_logic; -- grounded by module
    sfp_mod1         : inout std_logic; -- SCL
    sfp_mod2         : inout std_logic; -- SDA

	 -----------------------------------------------------------------------
     -- VME bus
	 -----------------------------------------------------------------------
    vme_as_n_i          : in    std_logic;
    vme_rst_n_i         : in    std_logic;
    vme_write_n_i       : in    std_logic;
    vme_am_i            : in    std_logic_vector(5 downto 0);
    vme_ds_n_i          : in    std_logic_vector(1 downto 0);
    -- the vetar card doesn't support geographical addressing
    -- it does it with the switches for convention with the core
    vme_ga_i            : in    std_logic_vector(3 downto 0);
    vme_addr_data_b     : inout std_logic_vector(31 downto 0);
    vme_iackin_n_i      : in    std_logic;
    vme_iackout_n_o     : out   std_logic;
    vme_iack_n_i        : in    std_logic;  
    vme_irq_n_o         : out   std_logic_vector(6 downto 0);
    vme_berr_o          : out   std_logic;
    vme_dtack_oe_o      : out   std_logic;
	 vme_buffer_latch_o  : out   std_logic_vector(3 downto 0);
    vme_data_oe_ab_o    : out   std_logic;
    vme_data_oe_ba_o    : out   std_logic;
    vme_addr_oe_ab_o    : out   std_logic;
    vme_addr_oe_ba_o    : out   std_logic;

    -----------------------------------------------------------------------
    -- LA port
    -----------------------------------------------------------------------
    hplw             : inout std_logic_vector(15 downto 0);
    
    -----------------------------------------
    -- USB micro controller 3.3V
    -----------------------------------------
    --pres_o  must be '0', it is by design 
    sres_o    : out   std_logic; -- AB8   active low reset#
    slrdn_o   : out   std_logic; -- AC10   read strobe
    slwrn_o   : out   std_logic; -- AB9    write strobe
    speed_i   : in    std_logic; -- PA0 = AF8
    shift_i   : in    std_logic; -- PA1 = AE8
    sloen_o   : out   std_logic; -- PA2 = W11
    ebcyc_i   : in    std_logic; -- PA3 = W12
    fifoadr_o : out   std_logic_vector(1 downto 0); -- 0=PA4=AC12, 1=PA5=W13
    pktendn_o : out   std_logic; -- PA6 = Y12
    readyn_io : inout std_logic; -- PA7 = AD12
    fulln_i   : in    std_logic; -- CTL1 = AA9
    emptyn_i  : in    std_logic; -- CTL2 = AB10
    fd_io     : inout std_logic_vector(7 downto 0); -- FIFO bus
                                                    -- AH2,AA10,AC6,AH3,
                                                    -- Y10,AD6,W10,Y1
	 -----------------------------------------------------------------------
    -- RAM
    -----------------------------------------------------------------------
--    ram_gw	   :	in		std_logic;					      -- Synchronous Global Write Enable
--    ram_bwe	   :	out	std_logic;							-- Synchronous Byte Write Enable
--    ram_bwx	   :	out	std_logic_vector(3 downto 0);	-- Synchronous Byte Write Enable
--    ram_oe	   :	out 	std_logic;					      -- Output Enable
--    ram_ce	   :	out	std_logic_vector(1 downto 0);	-- Synchronous Chip Enable
--    ram_adv	   :	out	std_logic;							-- Synchronous Burst Write Enable
--    ram_adsc   :	out	std_logic;							-- Synchronous Controller Address Status
--    ram_adsp   :	out	std_logic;							-- Synchronous Processor Address Status
--    ram_address:	in		std_logic_vector(18 downto 0);
--    ram_data	:	inout	std_logic_vector(31 downto 0);
--    ram_clk    :  out   std_logic;

	 -----------------------------------------------------------------------
    -- Display
    -----------------------------------------------------------------------	
 	 di              : out std_logic_vector(3 downto 0);
	 color				: out std_logic_vector(2 downto 0);                                        
	
    -----------------------------------------
    -- VETAR1DB1 ADD-ON Board 
    -----------------------------------------
	 
    -- LVDS
	 lvds_in_i				: out std_logic_vector(1 downto 0);
	 lvds_out_o				: out std_logic_vector(1 downto 0);
	 leds_lvds_out_o	: out std_logic_vector(1 downto 0);

    -- CLKs
	 --clk_conn_i				: in std_logic;
	 --clk_con_i				: in std_logic;

	 -- HDMI
	 hdmi_o					: out std_logic_vector(1 downto 0);
	 hdmi_i					: out std_logic;
	 
	 -- Output LEMOs and LEDS
	 lemo_addOn_o			: out std_logic_vector(5 downto 0);
	 leds_lemo_addOn_o	: out std_logic_vector(5 downto 0);
	 
    -- NIM/TTL LEMOs
	 lemo_nim_ttl_i		: inout std_logic_vector(1  downto 0));
       
end vetar_top;

architecture rtl of vetar_top is
  
  -- WR core layout
  constant c_wrcore_bridge_sdb : t_sdb_bridge := f_xwb_bridge_manual_sdb(x"0003ffff", x"00030000");
  
  -- Top crossbar layout
  constant c_slaves  : natural := 8;
  constant c_masters : natural := 3;
  constant c_layout : t_sdb_record_array(c_slaves-1 downto 0) :=
   (0 => f_sdb_embed_bridge(c_wrcore_bridge_sdb,          x"00000000"),
    1 => f_sdb_embed_device(c_xwr_wb_timestamp_latch_sdb, x"00100000"),
    2 => f_sdb_embed_device(c_eca_sdb,                    x"00100800"),
    3 => f_sdb_embed_device(c_eca_event_sdb,              x"00100C00"),
    4 => f_sdb_embed_device(c_wb_serial_lcd_sdb,          x"00100D00"),
    5 => f_sdb_embed_device(c_build_id_sdb,               x"00200000"),
    6 => f_sdb_embed_device(f_wb_spi_flash_sdb(24),       x"04000000"),
    7 => f_sdb_embed_device(c_vme_msi_sdb,                x"00100900"));

  constant c_sdb_address : t_wishbone_address := x"00300000";

  signal cbar_slave_i  : t_wishbone_slave_in_array (c_masters-1 downto 0);
  signal cbar_slave_o  : t_wishbone_slave_out_array(c_masters-1 downto 0);
  signal cbar_master_i : t_wishbone_master_in_array(c_slaves-1 downto 0);
  signal cbar_master_o : t_wishbone_master_out_array(c_slaves-1 downto 0);

  signal vme_slave_i : t_wishbone_slave_in;
  signal vme_slave_o : t_wishbone_slave_out;
  
  -- Sys PLL from clk_125m_local_i
  signal rstn_sys         : std_logic;
  signal pll_arst         : std_logic;
  
  -- logical clock names
  signal clk_reconf       : std_logic;
  signal clk_flash        : std_logic;
  signal clk_lcd          : std_logic;
  
  -- RX PLL
  signal gxb_locked       : std_logic;
  signal clk_ref0         : std_logic;
  signal clk_ref1         : std_logic;
  signal clk_ref2         : std_logic;
  
  -- Ref PLL from clk_125m_pllref_i
  signal ref_locked       : std_logic;
  signal clk_ref          : std_logic;
  signal clk_butis        : std_logic;
  signal clk_25m          : std_logic;
  
  signal sys_locked       : std_logic;  
  signal clk_sys0         : std_logic;
  signal clk_sys1         : std_logic;
  signal clk_sys2         : std_logic;
  signal clk_sys          : std_logic;

  signal phase_done       : std_logic;
  signal phase_step       : std_logic;
  signal phase_sel        : std_logic_vector(3 downto 0);
  signal phase_butis      : phase_offset;

  -- DMTD PLL from clk_20m_vcxo_i
  signal dmtd_locked      : std_logic;
  signal clk_dmtd         : std_logic;
  signal clk_dmtd0        : std_logic;
  signal clk_free         : std_logic;
  signal rstn_free        : std_logic;
  signal clk_phase        : std_logic;
  signal rstn_ref         : std_logic;
  signal rstn_butis       : std_logic;

  signal dac_hpll_load_p1 : std_logic;
  signal dac_dpll_load_p1 : std_logic;
  signal dac_hpll_data    : std_logic_vector(15 downto 0);
  signal dac_dpll_data    : std_logic_vector(15 downto 0);

  signal ext_pps : std_logic;
  signal pps : std_logic;

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

  signal wrc_master_i  : t_wishbone_master_in;
  signal wrc_master_o  : t_wishbone_master_out;

  signal mb_src_out    : t_wrf_source_out;
  signal mb_src_in     : t_wrf_source_in;
  signal mb_snk_out    : t_wrf_sink_out;
  signal mb_snk_in     : t_wrf_sink_in;
  
  signal tm_up     : std_logic;
  signal tm_valid  : std_logic;  
  signal tm_tai    : std_logic_vector(39 downto 0);
  signal tm_cycles : std_logic_vector(27 downto 0);

  signal channels : t_channel_array(1 downto 0);
  
  signal owr_pwren_o : std_logic_vector(1 downto 0);
  signal owr_en_o: std_logic_vector(1 downto 0);
  signal owr_i:	std_logic_vector(1 downto 0);
  
  signal sda_i:	std_logic;
  signal sda_o:	std_logic;
  signal scl_i:	std_logic;
  signal scl_o:	std_logic;
  
  signal sfp_scl_o:	std_logic;
  signal sfp_scl_i:	std_logic;
  signal sfp_sda_o:	std_logic;
  signal sfp_sda_i:	std_logic;
  signal sfp_det_i: std_logic;
  
  signal di_scp : std_logic;
  signal di_lp  : std_logic;
  signal di_flm : std_logic;
  signal di_dat : std_logic;
  signal di_bll : std_logic;
  
  constant black : std_logic_vector 	:= "111";
  constant red : std_logic_vector 		:= "101";
  constant green   : std_logic_vector 	:= "110";
  constant blue  : std_logic_vector 	:= "011";
  
  signal eca_gpio :  std_logic_vector(15 downto 0);

  signal s_uart_rxd_i   : std_logic;
  signal s_uart_txd_o   : std_logic;
  signal s_link_act 		: std_logic;
  signal s_link_up		  : std_logic;

  signal fd_oen : std_logic;
  signal fd_o   : std_logic_vector(7 downto 0);

  -- vme
  signal s_int_ack        : std_logic;

  signal s_vme_buffer     : t_vme_buffer;
  signal s_buffer_latch   : std_logic;
  signal s_vme_dtack_oe_o : std_logic;
  signal s_vme_dtack_n_o  : std_logic;
  signal s_vme_lword_n_o  : std_logic;
  signal s_vme_lword_n_i  : std_logic;
  signal s_vme_berr_o 	  : std_logic;
  --mux
  signal s_vme_data_o     : std_logic_vector(31 downto 0);
  signal s_vme_addr_o     : std_logic_vector(31 downto 1);

  signal s_trigger        : std_logic;

  -- test flash
  signal clk_50_delayed_72		: std_logic;

  --irq test
  signal     irq_test : std_logic :='0';
  signal     width    : std_logic :='0';
begin

  -- one wire
  owr_i(0) <= OneWire_CB;
  owr_i(1) <= '0';
  OneWire_CB <= owr_pwren_o(0) when (owr_pwren_o(0) = '1' or owr_en_o(0) = '1') else 'Z';
  
  -- open drain buffer for SFP i2c
  sfp_scl_i <= sfp_mod1;
  sfp_sda_i <= sfp_mod2;
  
  sfp_det_i <= sfp_mod0;
  sfp_mod1 <= '0' when sfp_scl_o = '0' else 'Z';
  sfp_mod2 <= '0' when sfp_sda_o = '0' else 'Z';
  
  sfp_tx_disable_o <= '0';				-- enable SFP

  -- We need at least one off-chip free running clock to setup PLLs
  clk_free <= clk_20m_vcxo_i;
  
  reset : altera_reset
    generic map(
      g_clocks      => 3)
    port map(
      clk_free_i    => clk_free,
      rstn_i        => '1',
      pll_lock_i(0) => dmtd_locked,
      pll_lock_i(1) => ref_locked,
      pll_lock_i(2) => sys_locked,
      pll_lock_i(3) => gxb_locked,
      pll_arst_o    => pll_arst,
      clocks_i(0)   => clk_sys,
      clocks_i(1)   => clk_free,
      clocks_i(2)   => clk_ref,
      rstn_o(0)     => rstn_sys,
      rstn_o(1)     => rstn_free,
      rstn_o(2)     => rstn_ref);

  -- PLLs

  dmtd_inst : dmtd_pll port map(
    areset => pll_arst,
    inclk0 => clk_20m_vcxo_i,    --  20  Mhz 
    c0     => clk_dmtd0,         --  62.5MHz
    locked => dmtd_locked);
  
  dmtd_clk : single_region port map(
    inclk  => clk_dmtd0,
    outclk => clk_dmtd);
 
  sys_inst : sys_pll port map(
    areset => pll_arst,
    inclk0 => clk_125m_local_i, -- 125  Mhz 
    c0     => clk_sys0,         --  62.5 MHz
    c1     => clk_sys1,         --  50  Mhz
    c2     => clk_sys2,         --  20  MHz
    locked => sys_locked);

  sys_clk : global_region port map(
    inclk  => clk_sys0,
    outclk => clk_sys);

  reconf_clk : global_region port map(
    inclk  => clk_sys1,
    outclk => clk_reconf);

  clk_flash <= clk_reconf;

  lcd_clk : single_region port map(
    inclk  => clk_sys2,
    outclk => clk_lcd);

  ref_inst : ref_pll port map( -- see "Phase Counter Select Mapping" table for arria2gx
    areset => pll_arst,
    inclk0 => clk_125m_pllref_i, -- 125 MHz
    c0     => clk_ref0,          -- 125 MHz, counter: 0010 - #2
    c1     => clk_ref1,          -- 200 MHz, counter: 0011 = #3
    c2     => clk_ref2,          --  25 MHz, counter: 0100 = #4
    locked => ref_locked,
    scanclk            => clk_free,
    phasedone          => phase_done,
    phasecounterselect => phase_sel,
    phasestep          => phase_step,
    phaseupdown        => '1');


  ref_clk : global_region port map(
    inclk  => clk_ref0,
    outclk => clk_ref);

  butis_clk : global_region port map(
    inclk  => clk_ref1,
    outclk => clk_butis);

  clk_pll_o  <= clk_ref1; -- dedicated output clock pin

  phase_clk : single_region port map(
    inclk  => clk_ref2,
    outclk => clk_phase);

  phase : altera_phase
    generic map(
      g_select_bits   => 4,
      g_outputs       => 1,
      g_base          => 0,
      g_vco_freq      => 1000, -- 1GHz
      g_output_freq   => (0 => 200),
      g_output_select => (0 =>   3))
    port map(
      clk_i       => clk_free,
      rstn_i      => rstn_free,
      clks_i(0)   => clk_butis,
      rstn_o(0)   => rstn_butis,
      offset_i(0) => phase_butis,
      phasedone_i => phase_done,
      phasesel_o  => phase_sel,
      phasestep_o => phase_step);

  butis : altera_butis
    port map(
      clk_ref_i => clk_ref,
      clk_25m_i => clk_phase,
      pps_i     => pps,
      phase_o   => phase_butis);

  -- ID
  id : build_id
    port map(
      clk_i   => clk_sys,
      rst_n_i => rstn_sys,
      slave_i => cbar_master_o(5),
      slave_o => cbar_master_i(5)); 

  -- FLASH
  flash : flash_top
    generic map(
      g_family                 => "Arria II GX",
      g_port_width             => 1,   -- single-lane SPI bus
      g_addr_width             => 24,  -- 3 byte addressed chip
      g_dummy_time             => 8,
      g_input_latch_edge       => '1', -- 30ns at 50MHz (10+20) after falling edge sets up SPI output
      g_output_latch_edge      => '0', -- falling edge to meet SPI setup times
      g_input_to_output_cycles => 2)   -- delayed to work-around unconstrained design
    port map(
      clk_i     => clk_sys,
      rstn_i    => rstn_sys,
      slave_i   => cbar_master_o(6),
      slave_o   => cbar_master_i(6),
      clk_out_i => clk_flash,
      clk_ext_i => clk_flash,
      clk_in_i  => clk_flash); -- no need to phase shift at 50MHz
  
  
  U_WR_CORE : xwr_core
    generic map (
      g_simulation                => 0,
      g_phys_uart                 => true,
      g_virtual_uart              => false,
      g_with_external_clock_input => true,
      g_aux_clks                  => 1,
      g_ep_rxbuf_size             => 1024,
      g_dpram_initf               => "wrc.mif",
		--g_dpram_initf               => "",
      g_dpram_size                => 131072/4,
      g_interface_mode            => PIPELINED,
      g_address_granularity       => BYTE,
      g_aux_sdb                   => c_etherbone_sdb)
    port map (
      clk_sys_i  => clk_sys,
      clk_dmtd_i => clk_dmtd,
      clk_ref_i  => clk_ref,
      clk_aux_i  => (others => '0'),
      clk_ext_i  => '0', -- g_with_external_clock_input controls usage
      pps_ext_i  => '0',
      rst_n_i    => rstn_sys,
		
      dac_hpll_load_p1_o => dac_hpll_load_p1,
      dac_hpll_data_o    => dac_hpll_data,
      dac_dpll_load_p1_o => dac_dpll_load_p1,
      dac_dpll_data_o    => dac_dpll_data,
		
      phy_ref_clk_i      => clk_ref,
      phy_tx_data_o      => phy_tx_data,
      phy_tx_k_o         => phy_tx_k,
      phy_tx_disparity_i => phy_tx_disparity,
      phy_tx_enc_err_i   => phy_tx_enc_err,
      phy_rx_data_i      => phy_rx_data,
      phy_rx_rbclk_i     => phy_rx_rbclk,
      phy_rx_k_i         => phy_rx_k,
      phy_rx_enc_err_i   => phy_rx_enc_err,
      phy_rx_bitslide_i  => phy_rx_bitslide,
      phy_rst_o          => phy_rst,
      phy_loopen_o       => phy_loopen,
      
      led_act_o   => s_link_act,
      led_link_o  => s_link_up,
      scl_o       => scl_o,
      scl_i       => scl_i,
      sda_i       => sda_i,
      sda_o       => sda_o,
      sfp_scl_i   => sfp_scl_i,
      sfp_sda_i   => sfp_sda_i,
      sfp_scl_o   => sfp_scl_o,
      sfp_sda_o   => sfp_sda_o,
      sfp_det_i   => sfp_det_i,
      btn1_i      => '0',
      btn2_i      => '0',

      uart_rxd_i => s_uart_rxd_i,
      uart_txd_o => s_uart_txd_o,
      
      owr_pwren_o => owr_pwren_o,
      owr_en_o    => owr_en_o,
      owr_i       => owr_i,
      slave_i => cbar_master_o(0),
      slave_o => cbar_master_i(0),

      wrf_src_o => mb_snk_in,
      wrf_src_i => mb_snk_out,
      wrf_snk_o => mb_src_in,
      wrf_snk_i => mb_src_out,

      aux_master_o => wrc_master_o,
      aux_master_i => wrc_master_i,
 
      tm_link_up_o         => tm_up,
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

  wr_gxb_phy_arriaii_1 : wr_arria2_phy
    port map (
      clk_reconf_i   => clk_reconf,
      clk_pll_i      => clk_ref0,
      clk_cru_i      => sfp_ref_clk_i,
      clk_free_i     => clk_free,
      rst_i          => pll_arst,
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
      pad_txp_o      => sfp_txp_o,
      pad_rxp_i      => sfp_rxp_i);

  -- VME bus interface
  --------------------
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
      g_base_addr		  => MECHANICALLY,
      g_irq_src        => MSI)

     port map(
        -- VME
      clk_i           => clk_sys,
      vme_as_n_i      => vme_as_n_i,
      vme_rst_n_i     => vme_rst_n_i,
      vme_write_n_i   => vme_write_n_i,
      vme_am_i        => vme_am_i,
      vme_ds_n_i      => vme_ds_n_i,
      vme_ga_i        => b"00" & vme_ga_i ,
      vme_berr_o      => s_vme_berr_o,
      vme_dtack_n_o   => s_vme_dtack_n_o,
		vme_retry_n_o   => open,		       -- no retry pin in VETAR  
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
      -- buffer
      --vme_dtack_oe_o  => s_vme_dtack_oe_o,
      vme_buffer_o    => s_vme_buffer,
      --vme_retry_oe_o  => vme_retry_oe_o,
		vme_retry_oe_o  => open,            -- no retry in VETAR
      --IRQ Generator    
      irq_i           => '0', 
      int_ack_o       => open, -- => s_int_ack,
      reset_o         => open, -- => s_rst,
      -- WB Interface
      master_o        => cbar_slave_i(2),
      master_i        => cbar_slave_o(2),

      slave_o         => cbar_master_i(7),
      slave_i         => cbar_master_o(7),
      debug           => open);

  U_BUFFER_CTRL :   VME_Buffer_ctrl
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
      latch_buff_o     =>  s_buffer_latch);

  vme_buffer_latch_o  <=  (others => s_buffer_latch);

  -- DATA & ADDR BUS MUX
  vme_addr_data_b <=  s_vme_data_o                       when  s_vme_buffer.s_buffer_eo = data_buff and 
																               s_vme_buffer.s_datadir = fpga2vme         else
						    (s_vme_addr_o & s_vme_lword_n_o)	when  s_vme_buffer.s_buffer_eo = addr_buff and 
																				   s_vme_buffer.s_addrdir = fpga2vme         else
						    (others => 'Z');

  s_vme_lword_n_i	<= vme_addr_data_b(0);	

  -- completely redundant. There is already a signal for controlling the oe 
  -- of the buffer. To be removed and tested.
  vme_dtack_oe_o  <=  s_vme_dtack_n_o	when s_vme_dtack_oe_o = '1' else 
                      '1';
	
  vme_berr_o			<=	not s_vme_berr_o; -- the logic in the core is inversed because their buffers invert the signal..
  
  --Rst <= VME_RST_n_i and Reset;
  --rst	<= vme_rst_n_i;

 ----------------------------------------

  U_DAC_ARB : spec_serial_dac_arb
    generic map (
      g_invert_sclk    => false,
      g_num_extra_bits => 8) -- AD DACs with 24bit interface
    port map (
      clk_i   => clk_sys,
      rst_n_i => rstn_sys,

      val1_i  => dac_dpll_data,
      load1_i => dac_dpll_load_p1,

      val2_i  => dac_hpll_data,
      load2_i => dac_hpll_load_p1,

      dac_cs_n_o(0) => ndac_cs(1),
      dac_cs_n_o(1) => ndac_cs(2),
      dac_clr_n_o   => open,
      dac_sclk_o    => dac_sclk,
      dac_din_o     => dac_din);

  U_Extend_PPS : gc_extend_pulse
    generic map (
      g_width => 10000000)
    port map (
      clk_i      => clk_ref,
      rst_n_i    => rstn_ref,
      pulse_i    => pps,
      extended_o => ext_pps);
  
   U_ebone : eb_ethernet_slave
     generic map(
       g_sdb_address => x"00000000" & c_sdb_address)
     port map(
       clk_i       => clk_sys,
       nRst_i      => rstn_sys,
       snk_i       => mb_snk_in,
       snk_o       => mb_snk_out,
       src_o       => mb_src_out,
       src_i       => mb_src_in,
       cfg_slave_o => wrc_master_i,
       cfg_slave_i => wrc_master_o,
       master_o    => cbar_slave_i(0),
       master_i    => cbar_slave_o(0));
  
  TLU : wb_timestamp_latch
    generic map (
      g_num_triggers => 1,
      g_fifo_depth   => 10)
    port map (
      ref_clk_i       => clk_ref,
      ref_rstn_i      => rstn_ref,
      sys_clk_i       => clk_sys,
      sys_rstn_i      => rstn_sys,
      triggers_i(0)   => s_trigger,
      tm_time_valid_i => '0',
      tm_tai_i        => tm_tai,
      tm_cycles_i     => tm_cycles,
      wb_slave_i      => cbar_master_o(1),
      wb_slave_o      => cbar_master_i(1));

  ECA0 : wr_eca
    generic map(
      g_eca_name       => f_name("VETAR top"),
      g_channel_names  => (f_name("GPIO: LEMOs(0-5)"),
                           --f_name("VME: Interrupt generator"), 
                           f_name("WB:   timing bus")),
      g_log_table_size => 7,
      g_log_queue_len  => 8,
      g_num_channels   => 2,
      g_num_streams    => 1)
    port map(
      e_clk_i  (0)=> clk_sys,
      e_rst_n_i(0)=> rstn_sys,
      e_slave_i(0)=> cbar_master_o(3),
      e_slave_o(0)=> cbar_master_i(3),
      c_clk_i     => clk_sys,
      c_rst_n_i   => rstn_sys,
      c_slave_i   => cbar_master_o(2),
      c_slave_o   => cbar_master_i(2),
      a_clk_i     => clk_ref,
      a_rst_n_i   => rstn_ref,
      a_tai_i     => tm_tai,
      a_cycles_i  => tm_cycles,
      a_channel_o => channels,
      i_clk_i     => clk_sys,
      i_rst_n_i   => rstn_sys,
      i_master_i  => cc_dummy_master_in, -- !!! FIXME: connect somewhere
      i_master_o  => open);
  
  C0 : eca_gpio_channel
    port map(
      clk_i     => clk_ref,
      rst_n_i   => rstn_ref,
      channel_i => channels(0),
      gpio_o    => eca_gpio);
  
 -- C1 : eca_wb_channel
 --   port map(
 --     clk_i     => clk_ref,
 --     rst_n_i   => rstn_ref,
 --     channel_i => channels(0),
 --     master_o  => vme_slave_i,
 --     master_i  => vme_slave_o);
  
  GSI_CON : xwb_sdb_crossbar
   generic map(
     g_num_masters => c_masters,
     g_num_slaves  => c_slaves,
     g_registered  => true,
     g_wraparound  => true,
     g_layout      => c_layout,
     g_sdb_addr    => c_sdb_address)
   port map(
     clk_sys_i     => clk_sys,
     rst_n_i       => rstn_sys,
     -- Master connections (INTERCON is a slave)
     slave_i       => cbar_slave_i,
     slave_o       => cbar_slave_o,
     -- Slave connections (INTERCON is a master)
     master_i      => cbar_master_i,
     master_o      => cbar_master_o);

  -- USB micro controller
  -----------------
  fd_io <= fd_o when fd_oen='1' else (others => 'Z');
  readyn_io <= 'Z'; -- weak pull-up
  
  EZUSB : ez_usb
    generic map(
      g_sdb_address => c_sdb_address)
    port map(
      clk_sys_i => clk_sys,
      rstn_i    => rstn_sys,
      master_i  => cbar_slave_o(1),
      master_o  => cbar_slave_i(1),
      uart_o    => s_uart_rxd_i,
      uart_i    => s_uart_txd_o,
      
      rstn_o    => sres_o,
      ebcyc_i   => ebcyc_i,
      speed_i   => speed_i,
      shift_i   => shift_i,
      fifoadr_o => fifoadr_o,
      readyn_i  => readyn_io,
      fulln_i   => fulln_i,
      emptyn_i  => emptyn_i,
      sloen_o   => sloen_o,
      slrdn_o   => slrdn_o,
      slwrn_o   => slwrn_o,
      pktendn_o => pktendn_o,
      fd_i      => fd_io,
      fd_o      => fd_o,
      fd_oen_o  => fd_oen);
  
  -- Display
  ----------------
   display : wb_serial_lcd
   generic map(
      g_wait => 1,
      g_hold => 15)
   port map(
         slave_clk_i  => clk_sys,
         slave_rstn_i => rstn_sys,
         slave_i      => cbar_master_o(4),
         slave_o      => cbar_master_i(4),
         di_clk_i     => clk_lcd,
         di_scp_o     => di_scp,
         di_lp_o      => di_lp,
         di_flm_o     => di_flm,
         di_dat_o     => di_dat);
  
  di(3) <= '0' when (di_scp = '0') else 'Z'; -- clock (run at 2MHz)                            
  di(2) <= '0' when (di_flm = '0') else 'Z'; -- first-line marker
  di(1) <= '0' when (di_lp  = '0') else 'Z'; -- latch pulse (end-of-40-bit-row)  
  di(0) <= '0' when (di_dat = '0') else 'Z'; -- shift register in
  
  -- red=nolink, blue=link+notrack, green=track
  color <= red 	when (not tm_up) 							else
			  blue  	when (    tm_up and not tm_valid)  	else
			  green  when (    tm_up and     tm_valid)   else
			  black;          

  -- On board leds
  -----------------
   -- Link Activity
	--!!! DON'T USE leds_o(0) leds_o(1) are too close to
	-- a clock and they can be harmfull
  leds_o(15)		<= not (s_link_act and s_link_up); -- Link active
  leds_o(14)		<= not s_link_up;						  -- Link up
  leds_o(13)		<=	not tm_valid;						  -- Timing Valid
  leds_o(12)	   <= not ext_pps;
  
  -- not assigned leds
  leds_o(11 downto 0)	<= (others => '1'); -- power off
  
  -- On board lemo
  ---------------- 
  -- PPS output
  lemo_o_en_o 	<= '0';
  lemo_o		  	<= pps;

  -- Latch input
  --lemo_i, see component map  wb_timestamp_latch 
  s_trigger    <= lemo_i;
 
 -- Add-on Board
 ----------------
  -- Input 
 
  -- ECA output lemo
  lemo_addOn_o(0)	<= eca_gpio(0);
  leds_lemo_addOn_o(0)	<= not eca_gpio(0);
  
  lemo_addOn_o(1)	<= eca_gpio(1);
  leds_lemo_addOn_o(1)	<= not eca_gpio(1);
  
  lemo_addOn_o(2)	<= eca_gpio(2);
  leds_lemo_addOn_o(2)	<= not eca_gpio(2);
  
  lemo_addOn_o(3)	<= eca_gpio(3);
  leds_lemo_addOn_o(3)	<= not eca_gpio(3);
  
  lemo_addOn_o(4)	<= eca_gpio(4);
  leds_lemo_addOn_o(4)	<= not eca_gpio(4);
  
  lemo_addOn_o(5)	<= eca_gpio(5);
  leds_lemo_addOn_o(5)	<= not eca_gpio(5);
  
  lvds_out_o(0)		<=	eca_gpio(0);
  lvds_out_o(1)		<=	eca_gpio(1);
  leds_lvds_out_o(0)	<= not eca_gpio(0);
  leds_lvds_out_o(1)	<= not eca_gpio(1);
   
  -- HDMI
  hdmi_o(0) <= pps;
  hdmi_o(1) <= clk_ref;  
 
  -- LA
  ---------------------  
  hplw(0) <= '0';
  hplw(1) <= '0';
  hplw(2) <= '0';
  hplw(3) <= '0';
  
  hplw(7) <= clk_lcd;
  hplw(8) <= di_scp;
  hplw(9) <= di_lp;
  hplw(10)<= di_flm;
  hplw(11)<= di_dat;
  
  hplw(15 downto 12) <= (others => '0');
  
end rtl;
