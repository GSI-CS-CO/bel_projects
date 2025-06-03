library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.gencores_pkg.all;
use work.monster_pkg.all;
use work.ramsize_pkg.c_lm32_ramsizes;
use work.altera_lvds_pkg.all;
use work.altera_networks_pkg.all;

entity scu_control is
  port(
    ------------------------------------------------------------------------
    -- Input clocks
    ------------------------------------------------------------------------
    clk1_20m_vcxo_i     : in std_logic; -- 20MHz VCXO clock, WR#1
    clk1_20m_vcxo_alt_i : in std_logic; -- 20MHz VCXO clock, WR#1, alternative clock
    clk2_20m_vcxo_i     : in std_logic; -- 20MHz VCXO clock, WR#2
    clk2_20m_vcxo_alt_i : in std_logic; -- 20MHz VCXO clock, WR#2, alternative clock

    clk_125m_local_aa18 : in std_logic; -- Local clk from 125Mhz oszillator
    clk_125m_local_d15  : in std_logic; -- Local clk from 125Mhz oszillator alternative

    clk_gxbl1c_u24      : in std_logic; -- 125 MHz WR1, 125 MHz WR2, 125 MHz local, 100 MHz real or fake PCIe, set by AVR
    clk_gxbl1c_w24      : in std_logic; -- ""
    clk_gxbl1d_n24      : in std_logic; -- "", PCIe option #1
    clk_gxbl1d_r24      : in std_logic; -- "", PCIe option #2

    clk1_wr_125m_ab16   : in std_logic; -- Optional Serdes/Deserdes clocks (based on WR#1)
    clk1_wr_125m_ac20   : in std_logic; -- ""

    ------------------------------------------------------------------------
    -- PCI express pins
    ------------------------------------------------------------------------
    nPCI_RESET_i  : in  std_logic;
    pcie_rdy_o    : out std_logic;

    ------------------------------------------------------------------------
    -- WR DAC signals
    ------------------------------------------------------------------------
    wr_dac_sclk_o : out std_logic;
    wr_dac_din_o  : out std_logic;
    wr_ndac_cs_o  : out std_logic_vector(2 downto 1); -- NDAC_CS1/2?

    -----------------------------------------------------------------------
    -- OneWire
    -----------------------------------------------------------------------
    OneWire_CB       : inout std_logic;
    OneWire_CB_splz  : out   std_logic; -- strong Pull-Up for Onewire

    onewire_ext      : inout std_logic; -- to extension board

    -----------------------------------------------------------------------
    -- ComExpress signals
    -----------------------------------------------------------------------
    ser0_rxd          : out std_logic; -- RX/TX view from ComX
    ser0_txd          : in  std_logic; -- RX/TX view from ComX
    ser1_rxd          : out std_logic; -- RX/TX view from ComX
    ser1_txd          : in  std_logic; -- RX/TX view from ComX
    nTHRMTRIP         : in  std_logic;
    WDT               : in  std_logic;
    fpga_res_i        : in  std_logic; -- NCB_RESET?
    nSys_Reset        : in  std_logic; -- Reset From ComX

    -----------------------------------------------------------------------
    -- SCU Bus
    -----------------------------------------------------------------------
    A_D               : inout std_logic_vector(15 downto 0);
    A_A               : out   std_logic_vector(15 downto 0);
    A_nTiming_Cycle   : out   std_logic;
    A_nDS             : out   std_logic;
    A_nReset          : out   std_logic;
    nSel_Ext_Data_DRV : out   std_logic;
    A_RnW             : out   std_logic;
    A_Spare           : out   std_logic_vector(1 downto 0);
    A_nSEL            : out   std_logic_vector(12 downto 1);
    A_nDtack          : in    std_logic;
    A_nSRQ            : in    std_logic_vector(12 downto 1);
    A_SysClock        : out   std_logic;
    ADR_TO_SCUB       : out   std_logic;
    nADR_EN           : out   std_logic;
    A_OneWire         : inout std_logic;
    A_OneWire_stpz    : out   std_logic;

    -----------------------------------------------------------------------
    -- Misc.
    -----------------------------------------------------------------------
    nFPGA_Res_Out : out   std_logic;                     -- Reset  Output
    user_btn      : in    std_logic;                     -- User Button (NUSER_PB?)
    avr_sda       : inout std_logic;                     -- I2C Connection to AVR MCU (F2F-I2C-ADA)
    avr_scl       : inout std_logic;                     -- I2C Connection to AVR MCU (F2F-I2C-SCL)
    serial_cb_out : out   std_logic_vector (1 downto 0); -- Serial to Backplane
    serial_cb_in  : in    std_logic_vector (1 downto 0); -- Serial to Backplane
    rear_in       : in    std_logic_vector (1 downto 0); -- GPIO to Backplane
    rear_out      : out   std_logic_vector (1 downto 0); -- GPIO to Backplane

    -----------------------------------------------------------------------
    -- SCU-CB Version
    -----------------------------------------------------------------------
    scu_cb_version : in  std_logic_vector(3 downto 0); -- must be assigned with weak pull ups
                                                       -- PIN_P2 VERSION_2_0
                                                       -- PIN_J2 VERSION_2_1
                                                       -- PIN_P4 VERSION_2_2 (ground, no pull up)
                                                       -- PIN_N2 VERSION_2_3

    -----------------------------------------------------------------------
    -- LVTTL IOs
    -----------------------------------------------------------------------
    fastIO_p_i : in  std_logic_vector(2 downto 0);
    fastIO_n_i : in  std_logic_vector(2 downto 0);
    fastIO_p_o : out std_logic_vector(2 downto 0); -- Negativ Pin assigned by Quartus 18.1, manually assignment causes issues
    --fastIO_n_o : out std_logic_vector(2 downto 0); -- Now possible? If not: Delete this
    lemo_out   : out std_logic_vector(3 downto 0); -- Isolated Onboard TTL OUT
    lemo_in    : in  std_logic_vector(1 downto 0); -- Isolated OnBoard TTL IN

    -----------------------------------------------------------------------
    -- Extension Connector
    -----------------------------------------------------------------------
    ext_ch : inout std_logic_vector(21 downto 0); -- See EIO
    ext_id : in    std_logic_vector(3 downto 0);

    -----------------------------------------------------------------------
    -- usb
    -----------------------------------------------------------------------
    usb_slrd : out   std_logic;
    usb_slwr : out   std_logic;
    usb_fd   : inout std_logic_vector(7 downto 0) := (others => 'Z');
    usb_pa   : inout std_logic_vector(7 downto 0) := (others => 'Z');
    usb_ctl  : in    std_logic_vector(2 downto 0);
    usb_uclk : in    std_logic; -- T0OUT/IFCLK
    usb_clk  : in    std_logic; -- T1OUT/CLKOUT - unused
    usb_ures : out   std_logic; -- NFX2_RESET

    -----------------------------------------------------------------------
    -- leds onboard
    -----------------------------------------------------------------------
    wr_led_pps : out std_logic := '1';
    --user_led_0 : out std_logic_vector(2 downto 0) := (others => '1'); -- USER_LED0?
    user_led0_g : out std_logic;
    user_led0_b : out std_logic;
    user_led0_r : out std_logic;
    wr_rgb_led : out std_logic_vector(2 downto 0) := (others => '1'); -- LDWR0_B/G/R?
    lemo_led   : out std_logic_vector(5 downto 0) := (others => '1'); -- LEMO_LED?

    -----------------------------------------------------------------------
    -- Pseudo-SRAM (4x 256Mbit)
    -----------------------------------------------------------------------
    psram_a    : out   std_logic_vector(23 downto 0) := (others => 'Z');
    psram_dq   : inout std_logic_vector(15 downto 0) := (others => 'Z');
    psram_clk  : out   std_logic := 'Z';
    psram_advn : out   std_logic_vector(3 downto 0) := (others => '1');
    psram_cre  : out   std_logic_vector(3 downto 0) := (others => '0');
    psram_cen  : out   std_logic_vector(3 downto 0) := (others => '1');
    psram_oen  : out   std_logic_vector(3 downto 0) := (others => '1');
    psram_wen  : out   std_logic_vector(3 downto 0) := (others => '1');
    psram_ubn  : out   std_logic_vector(3 downto 0) := (others => '0');
    psram_lbn  : out   std_logic_vector(3 downto 0) := (others => '0');
    psram_wait : in    std_logic_vector(3 downto 0);

    -----------------------------------------------------------------------
    -- Debugging
    -----------------------------------------------------------------------
    debug_leds : out std_logic_vector(7 downto 0);

    -----------------------------------------------------------------------
    -- Transceiver bank 1C
    -----------------------------------------------------------------------
    gxbl1c_rx_ch0p_af26         : in std_logic;
    --gxbl1c_rx_ch0n_af25         : in std_logic;
    gxbl1c_rx_ch1p_ad26         : in std_logic;
    --gxbl1c_rx_ch1n_ad25         : in std_logic;
    gxbl1c_rx_ch2p_ab26         : in std_logic;
    --gxbl1c_rx_ch2n_ab25         : in std_logic;
    gxbl1c_rx_ch2p_y26          : in std_logic;
    --gxbl1c_rx_ch2n_y25          : in std_logic;
    gxbl1c_rx_ch3p_v26_pcie0_rx : in std_logic;
    --gxbl1c_rx_ch3n_v25_pcie0_rx : in std_logic;
    gxbl1c_rx_ch4p_t26_pcie1_rx : in std_logic;
    --gxbl1c_rx_ch4n_t25_pcie1_rx : in std_logic;

    gxbl1c_tx_ch0p_ag28         : out std_logic;
    --gxbl1c_tx_ch0n_ag27         : in std_logic;
    gxbl1c_tx_ch1p_ae28         : out std_logic;
    --gxbl1c_tx_ch1n_ae27         : in std_logic;
    gxbl1c_tx_ch2p_ac28         : out std_logic;
    --gxbl1c_tx_ch2n_ac27         : in std_logic;
    gxbl1c_tx_ch3p_aa28         : out std_logic;
    --gxbl1c_tx_ch3n_aa27         : in std_logic;
    gxbl1c_tx_ch4p_w28_pcie0_tx : out std_logic;
    --gxbl1c_tx_ch4n_w28_pcie0_tx : in std_logic;
    gxbl1c_tx_ch5p_u28_pcie1_tx : out std_logic;
    --gxbl1c_tx_ch5n_u28_pcie1_tx : in std_logic;

    -----------------------------------------------------------------------
    -- Transceiver bank 1D
    -----------------------------------------------------------------------
    gxbl1d_rx_ch0p_p26_pcie2_rx : in std_logic;
    --gxbl1d_rx_ch0n_p25_pcie2_rx : in std_logic;
    gxbl1d_rx_ch1p_m26_pcie3_rx : in std_logic;
    --gxbl1d_rx_ch1n_m25_pcie3_rx : in std_logic;
    gxbl1d_rx_ch2p_k26_pciex_rx : in std_logic;
    --gxbl1d_rx_ch2n_k25_pciex_rx : in std_logic;
    gxbl1d_rx_ch3p_h26_pciex_rx : in std_logic;
    --gxbl1d_rx_ch3n_h25_pciex_rx : in std_logic;
    gxbl1d_rx_ch4p_f26_pciex_rx : in std_logic;
    --gxbl1d_rx_ch4n_f25_pciex_rx : in std_logic;
    gxbl1d_rx_ch5p_d26_pciex_rx : in std_logic;
    --gxbl1d_rx_ch5n_d25_pciex_rx : in std_logic;

    gxbl1d_tx_ch0p_r28_pcie2_tx : out std_logic;
    --gxbl1d_tx_ch0n_r27_pcie2_tx : in std_logic;
    gxbl1d_tx_ch1p_n28_pcie3_tx : out std_logic;
    --gxbl1d_tx_ch1n_n27_pcie3_tx : in std_logic;
    gxbl1d_tx_ch2p_l28_pciex_tx : out std_logic;
    --gxbl1d_tx_ch2n_l27_pciex_tx : in std_logic;
    gxbl1d_tx_ch3p_j28_pciex_tx : out std_logic;
    --gxbl1d_tx_ch3n_j27_pciex_tx : in std_logic;
    gxbl1d_tx_ch4p_g28_pciex_tx : out std_logic;
    --gxbl1d_tx_ch4n_g27_pciex_tx : in std_logic;
    gxbl1d_tx_ch5p_e28_pciex_tx : out std_logic;
    --gxbl1d_tx_ch5n_e27_pciex_tx : in std_logic;

    -----------------------------------------------------------------------
    -- SFP
    -----------------------------------------------------------------------
    sfp_rate_sel_o   : out   std_logic;
    sfp_tx_disable_o : out   std_logic;
    sfp_tx_fault_i   : in    std_logic;
    sfp_los_i        : in    std_logic;
    sfp_mod0_i       : in    std_logic;
    sfp_mod1_io      : inout std_logic;
    sfp_mod2_io      : inout std_logic);

end scu_control;

architecture rtl of scu_control is

  signal s_led_link_up  : std_logic;
  signal s_led_link_act : std_logic;
  signal s_led_track    : std_logic;
  signal s_led_pps      : std_logic;
  signal s_lemo_led     : std_logic_vector (5 downto 0);

  signal s_gpio_o    : std_logic_vector(9 downto 0);
  signal s_gpio_i    : std_logic_vector(2 downto 0);
  --signal s_lvds_p_i  : std_logic_vector(2 downto 0);
  --signal s_lvds_n_i  : std_logic_vector(2 downto 0);
  --signal s_lvds_p_o  : std_logic_vector(2 downto 0);
  --signal s_lvds_term : std_logic_vector(2 downto 0);

  signal s_clk_20m_vcxo_i       : std_logic;
  signal s_clk_125m_pllref_i    : std_logic;
  signal s_clk_125m_local_i     : std_logic;
  signal s_clk_sfp_i            : std_logic;
  signal s_stub_pll_reset       : std_logic;
  signal s_stub_pll_locked      : std_logic;
  signal s_stub_pll_locked_prev : std_logic;

  signal s_i2c_scl_pad_out  : std_logic_vector(1 downto 1);
  signal s_i2c_scl_pad_in   : std_logic_vector(1 downto 1);
  signal s_i2c_scl_padoen   : std_logic_vector(1 downto 1);
  signal s_i2c_sda_pad_out  : std_logic_vector(1 downto 1);
  signal s_i2c_sda_pad_in   : std_logic_vector(1 downto 1);
  signal s_i2c_sda_padoen   : std_logic_vector(1 downto 1);

  signal s_core_clk_25m     : std_logic;

  signal s_psram_advn       : std_logic_vector(3 downto 0);
  signal s_psram_cre        : std_logic_vector(3 downto 0);
  signal s_psram_cen        : std_logic_vector(3 downto 0);
  signal s_psram_oen        : std_logic_vector(3 downto 0);
  signal s_psram_wen        : std_logic_vector(3 downto 0);
  signal s_psram_wait       : std_logic_vector(3 downto 0);
  signal s_psram_ubn        : std_logic_vector(3 downto 0);
  signal s_psram_lbn        : std_logic_vector(3 downto 0);

  signal rstn_ref           : std_logic;
  signal clk_ref            : std_logic;

  constant io_mapping_table : t_io_mapping_table_arg_array(0 to 14) :=
  (
  -- Name[12 Bytes], Special Purpose, SpecOut, SpecIn, Index, Direction,   Channel,  OutputEnable, Termination, Logic Level
    ("LEMO_IN_0  ",  IO_NONE,         false,   false,  0,     IO_INPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("LEMO_IN_1  ",  IO_NONE,         false,   false,  1,     IO_INPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("FAST_IN_0  ",  IO_NONE,         false,   false,  2,     IO_INPUT,    IO_GPIO,  false,        false,       IO_LVDS),
    ("FAST_IN_1  ",  IO_NONE,         false,   false,  3,     IO_INPUT,    IO_GPIO,  false,        false,       IO_LVDS),
    ("FAST_IN_2  ",  IO_NONE,         false,   false,  4,     IO_INPUT,    IO_GPIO,  false,        false,       IO_LVDS),
    ("USER_LED0_R",  IO_NONE,         false,   false,  0,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("USER_LED0_G",  IO_NONE,         false,   false,  1,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("USER_LED0_B",  IO_NONE,         false,   false,  2,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LEMO_OUT_0 ",  IO_NONE,         false,   false,  3,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LEMO_OUT_1 ",  IO_NONE,         false,   false,  4,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LEMO_OUT_2 ",  IO_NONE,         false,   false,  5,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("LEMO_OUT_3 ",  IO_NONE,         false,   false,  6,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_TTL),
    ("FAST_OUT_0 ",  IO_NONE,         false,   false,  7,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_LVDS),
    ("FAST_OUT_1 ",  IO_NONE,         false,   false,  8,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_LVDS),
    ("FAST_OUT_2 ",  IO_NONE,         false,   false,  9,     IO_OUTPUT,   IO_GPIO,  false,        false,       IO_LVDS)
  );

  constant c_family       : string := "Arria 10 GX SCU4";
  constant c_project      : string := "scu_control";
  constant c_cores        : natural:= 1;
  constant c_initf_name   : string := c_project & "_stub.mif";
  constant c_profile_name : string := "medium_icache_debug";
  constant c_cr_bits      : natural := 24;

begin

  main : monster
    generic map(
      g_family             => c_family,
      g_project            => c_project,
      g_flash_bits         => 25, -- !!! TODO: Check this
      g_cr_bits            => c_cr_bits,
      g_gpio_in            => 5,
      g_gpio_out           => 10,
      --g_lvds_in            => 3,
      --g_lvds_out           => 3,
      --g_lvds_invert        => true,
      g_en_user_ow         => true,
      g_en_ddr3            => false,
      g_en_cfi             => false,
      g_en_i2c_wrapper     => true,
      g_num_i2c_interfaces => 1,
      g_en_scubus          => true,
      g_en_pcie            => true,
      g_en_tlu             => false,
      g_en_usb             => true,
      g_en_cellular_ram    => true,
      g_rams               => 4,
      g_io_table           => io_mapping_table,
      g_en_enc_err_counter => true,
      g_en_tempsens        => false,
      g_en_a10ts           => true,
      g_a10_use_sys_fpll   => false,
      g_a10_use_ref_fpll   => false,
      g_lm32_cores         => c_cores,
      g_lm32_ramsizes      => c_lm32_ramsizes/4,
      g_lm32_init_files    => f_string_list_repeat(c_initf_name, c_cores),
      g_lm32_profiles      => f_string_list_repeat(c_profile_name, c_cores),
      g_en_asmi            => true,
      g_en_a10vs           => true
    )
    port map(
      core_clk_20m_vcxo_i     => clk1_20m_vcxo_i,
      core_clk_125m_pllref_i  => clk_gxbl1c_u24,
      core_clk_125m_local_i   => clk_125m_local_aa18,
      core_clk_125m_sfpref_i  => clk_gxbl1c_u24,
      core_clk_25m_o          => s_core_clk_25m,
      core_clk_wr_ref_o       => clk_ref,
      core_rstn_wr_ref_o      => rstn_ref,
      wr_onewire_io           => OneWire_CB,
      wr_sfp_sda_io           => sfp_mod2_io,
      wr_sfp_scl_io           => sfp_mod1_io,
      wr_sfp_det_i            => sfp_mod0_i,
      wr_sfp_tx_o             => gxbl1c_tx_ch0p_ag28,
      wr_sfp_rx_i             => gxbl1c_rx_ch0p_af26,
      wr_dac_sclk_o           => wr_dac_sclk_o,
      wr_dac_din_o            => wr_dac_din_o,
      wr_ndac_cs_o            => wr_ndac_cs_o,
      wr_uart_o               => ser1_rxd,
      wr_uart_i               => ser1_txd,
      wbar_phy_dis_o          => sfp_tx_disable_o,
      sfp_tx_fault_i          => sfp_tx_fault_i,
      sfp_los_i               => sfp_los_i,
      gpio_i(1 downto 0)      => lemo_in,
      gpio_i(4 downto 2)      => s_gpio_i,
      gpio_o(9 downto 0)      => s_gpio_o(9 downto 0),
      --lvds_p_i                => s_lvds_p_i,
      --lvds_n_i                => s_lvds_n_i,
      --lvds_p_o                => s_lvds_p_o,
      led_link_up_o           => s_led_link_up,
      led_link_act_o          => s_led_link_act,
      led_track_o             => s_led_track,
      led_pps_o               => s_led_pps,
      debug_sys_locked_o      => debug_leds(0),
      debug_ge_85_c_o         => debug_leds(1),
      debug_ref1_locked_o     => debug_leds(2),
      debug_dmtd1_locked_o    => debug_leds(3),
      debug_ref2_locked_o     => debug_leds(4),
      debug_dmtd2_locked_o    => debug_leds(5),
      pcie_ready_o            => debug_leds(6),
      scubus_a_a              => A_A,
      scubus_a_d              => A_D,
      scubus_nsel_data_drv    => nSel_Ext_Data_DRV,
      scubus_a_nds            => A_nDS,
      scubus_a_rnw            => A_RnW,
      scubus_a_ndtack         => A_nDtack,
      scubus_a_nsrq           => A_nSRQ,
      scubus_a_nsel           => A_nSEL,
      scubus_a_ntiming_cycle  => A_nTiming_Cycle,
      scubus_a_sysclock       => A_SysClock,
      ow_io(0)                => onewire_ext,
      ow_io(1)                => A_OneWire,
      pcie_refclk_i           => clk_gxbl1d_n24,
      pcie_rstn_i             => nPCI_RESET_i,
      pcie_rx_i(0)            => gxbl1c_rx_ch3p_v26_pcie0_rx,
      pcie_rx_i(1)            => gxbl1c_rx_ch4p_t26_pcie1_rx,
      pcie_rx_i(2)            => gxbl1d_rx_ch0p_p26_pcie2_rx,
      pcie_rx_i(3)            => gxbl1d_rx_ch1p_m26_pcie3_rx,
      pcie_tx_o(0)            => gxbl1c_tx_ch4p_w28_pcie0_tx,
      pcie_tx_o(1)            => gxbl1c_tx_ch5p_u28_pcie1_tx,
      pcie_tx_o(2)            => gxbl1d_tx_ch0p_r28_pcie2_tx,
      pcie_tx_o(3)            => gxbl1d_tx_ch1p_n28_pcie3_tx,
      -- I2C
      i2c_scl_pad_i           => s_i2c_scl_pad_in,
      i2c_scl_pad_o           => s_i2c_scl_pad_out,
      i2c_scl_padoen_o        => s_i2c_scl_padoen,
      i2c_sda_pad_i           => s_i2c_sda_pad_in,
      i2c_sda_pad_o           => s_i2c_sda_pad_out,
      i2c_sda_padoen_o        => s_i2c_sda_padoen,
      -- FX2 USB
      usb_rstn_o              => usb_ures,
      usb_ebcyc_i             => usb_pa(3),
      usb_speed_i             => usb_pa(0),
      usb_shift_i             => usb_pa(1),
      usb_readyn_io           => usb_pa(7),
      usb_fifoadr_o           => usb_pa(5 downto 4),
      usb_sloen_o             => usb_pa(2),
      usb_fulln_i             => usb_ctl(1),
      usb_emptyn_i            => usb_ctl(2),
      usb_slrdn_o             => usb_slrd,
      usb_slwrn_o             => usb_slwr,
      usb_pktendn_o           => usb_pa(6),
      usb_fd_io               => usb_fd,
      -- PSRAM
      cr_clk_o                => psram_clk,
      cr_addr_o               => psram_a,
      cr_data_io              => psram_dq,
      cr_lbn_o(3 downto 0)    => s_psram_lbn,
      cr_ubn_o(3 downto 0)    => s_psram_ubn,
      cr_cen_o(3 downto 0)    => s_psram_cen,
      cr_oen_o(3 downto 0)    => s_psram_oen,
      cr_wen_o(3 downto 0)    => s_psram_wen,
      cr_cre_o(3 downto 0)    => s_psram_cre,
      cr_advn_o(3 downto 0)   => s_psram_advn,
      cr_wait_i(3 downto 0)   => s_psram_wait,
      hw_version              => x"0000000" & not scu_cb_version);

  -- Quad PSRAM
  quad_ram : for i in 0 to 3 generate
    psram_cen(i)    <= s_psram_cen(i);
    psram_cre(i)    <= s_psram_cre(i);
    psram_oen(i)    <= s_psram_oen(i);
    psram_wen(i)    <= s_psram_wen(i);
    psram_lbn(i)    <= s_psram_lbn(i);
    psram_ubn(i)    <= s_psram_ubn(i);
    psram_advn(i)   <= s_psram_advn(i);
    s_psram_wait(i) <= psram_wait(i);
  end generate;

  --user_led_0   <= s_gpio_o(2 downto 0) or s_psram_wait(3 downto 1); -- Keep unused WAIT in pins used, there this laster
  user_led0_r <= s_gpio_o(0);
  user_led0_g <= s_gpio_o(1);
  user_led0_b <= s_gpio_o(2);

  -- LEDs
  wr_led_pps    <= s_led_pps;                                             -- white = PPS
  wr_rgb_led(0) <= s_led_link_act;                                        -- WR-RGB Red
  wr_rgb_led(1) <= s_led_track;                                           -- WR-RGB Green
  wr_rgb_led(2) <= '1' when (not s_led_track and s_led_link_up) else '0'; -- WR-RGB Blue
  --user_led_0    <= s_gpio_o(2 downto 0); -> See PSRAM

  -- LEMOs
  lemos : for i in 0 to 2 generate
    --s_lvds_p_i(i) <= fastIO_p_i(i);
    --s_lvds_n_i(i) <= fastIO_n_i(i);
    --fastIO_p_o(i) <= s_lvds_p_o(i);
    fastIO_p_o <= s_gpio_o(9 downto 7); -- !!!

    --single_gpio_to_lvds : altera_lvds_obuf
    --  generic map(
    --    g_family  => c_family)
    --  port map(
    --    dataout_b  => fastIO_n_o(i),
    --    dataout    => fastIO_p_o(i),
    --    datain     => s_gpio_o(7+i)
    --  );

    lvds_to_single_gpio : altera_lvds_ibuf
      generic map(
        g_family  => c_family)
      port map(
        datain_b  => fastIO_n_i(i),
        datain    => fastIO_p_i(i),
        dataout   => s_gpio_i(i)
      );
  end generate;
  lemo_out <= s_gpio_o(6 downto 3);

  -- Lemo LEDs
  s_lemo_led (3 downto 0) <= s_gpio_o(6 downto 3);
  s_lemo_led (5 downto 4) <= lemo_in;

  -- Extend LEMO input/outputs to LEDs at 20Hz
  lemo_leds : for i in 0 to 5 generate
    lemo_ledx : gc_extend_pulse
      generic map(
        g_width => 125_000_000/20) -- 20 Hz
      port map(
        clk_i      => clk_ref,
        rst_n_i    => rstn_ref,
        pulse_i    => s_lemo_led(i),
        extended_o => lemo_led(i));
  end generate;

  -- OneWire
  OneWire_CB_splz   <= '1';  --Strong Pull-Up disabled

  --Extension Piggy
  ext_ch(0) <= s_led_pps;
  ext_ch(1) <= s_core_clk_25m;
  ext_ch(21 downto 2) <= (others => 'Z');

  -- I2C to ATXMEGA
  avr_scl             <= s_i2c_scl_pad_out(1) when (s_i2c_scl_padoen(1) = '0') else 'Z';
  avr_sda             <= s_i2c_sda_pad_out(1) when (s_i2c_sda_padoen(1) = '0') else 'Z';
  s_i2c_scl_pad_in(1) <= avr_scl;
  s_i2c_sda_pad_in(1) <= avr_sda;

  -- Resets
  A_nReset      <= rstn_ref;
  nFPGA_Res_Out <= rstn_ref; -- To ComExpress

  -- fixed scubus signals
  ADR_TO_SCUB <= '1';
  nADR_EN     <= '0';
  A_Spare     <= (others => 'Z');
  --A_OneWire   <= 'Z';

  -- other fixed signals
  sfp_rate_sel_o <= '0';
  A_OneWire_stpz <= '0';
  debug_leds(7)  <= nPCI_RESET_i;

end rtl;
