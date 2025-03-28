library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.monster_pkg.all;
use work.ramsize_pkg.c_lm32_ramsizes;
use work.altera_lvds_pkg.all;
use work.altera_networks_pkg.all;

entity ftm10 is
  generic(
    g_quad_mode_psram : boolean := false; -- True: Connect all PSRAMs; False: connect only $g_default_psram PSRAM
    g_default_psram   : natural := 0 -- Possible values: 0, 1, 2, 3
  );
  port(
    ------------------------------------------------------------------------
    -- Input clocks
    ------------------------------------------------------------------------
    clk_20m_vcxo_i        : in std_logic; -- 20MHz VCXO clock
    clk_20m_vcxo_alt_i    : in std_logic; -- 20MHz VCXO clock alternative

    clk_125m_local_i      : in std_logic; -- Local clk from 125Mhz oszillator
    clk_125m_local_alt_i  : in std_logic; -- Local clk from 125Mhz oszillator alternative

    clk_125m_tcb_pllref_i : in std_logic; -- 125 MHz PLL reference at tranceiver bank
    clk_125m_tcb_local_i  : in std_logic; -- Local clk from 125Mhz oszillator at tranceiver bank
    clk_125m_tcb_sfpref_i : in std_logic; -- PLL/SFP reference clk from 125Mhz oszillator at tranceiver bank

    ------------------------------------------------------------------------
    -- PCI express pins
    ------------------------------------------------------------------------
    pcie_refclk_i : in    std_logic;
    pcie_rx_i     : in    std_logic_vector(3 downto 0);
    pcie_tx_o     : out   std_logic_vector(3 downto 0);
    nPCI_RESET_i  : in    std_logic;

    ------------------------------------------------------------------------
    -- WR DAC signals
    ------------------------------------------------------------------------
    wr_dac_sclk_o : out std_logic;
    wr_dac_din_o  : out std_logic;
    wr_ndac_cs_o  : out std_logic_vector(2 downto 1);

    -----------------------------------------------------------------------
    -- SPI Flash User Mode
    -----------------------------------------------------------------------
    --UM_AS_D           : inout std_logic_vector(3 downto 0) := (others => 'Z');
    --UM_nCSO           : out   std_logic := 'Z';
    --UM_DCLK           : out   std_logic := 'Z';

    -----------------------------------------------------------------------
    -- OneWire
    -----------------------------------------------------------------------
    OneWire_CB          : inout std_logic;
    OneWire_CB_splz     : out   std_logic; --Strong Pull-Up for Onewire
    OneWire_aux_CB      : inout std_logic;
    OneWire_aux_CB_splz : out   std_logic; --Strong Pull-Up for Onewire

    -----------------------------------------------------------------------
    -- Misc.
    -----------------------------------------------------------------------
    nres_out_o   : out std_logic; -- Reset MAX10
    a_nsys_reset : in  std_logic; -- Reset

    -----------------------------------------------------------------------
    -- I2C
    -----------------------------------------------------------------------
    i2c_scl_pad_io   : inout std_logic_vector(4 downto 1);
    i2c_sda_pad_io   : inout std_logic_vector(4 downto 1);

    -----------------------------------------------------------------------
    -- leds onboard
    -----------------------------------------------------------------------
    wr_leds_o                  : out std_logic_vector(3 downto 0) := (others => '1');
    wr_aux_leds_or_node_leds_o : out std_logic_vector(3 downto 0) := (others => '1');

    -----------------------------------------------------------------------
    -- Pseudo-SRAM (4x 256Mbit)
    -----------------------------------------------------------------------
    psram_a            : out   std_logic_vector(23 downto 0) := (others => 'Z');
    psram_dq           : inout std_logic_vector(15 downto 0) := (others => 'Z');
    psram_clk          : out   std_logic := 'Z';
    psram_advn         : out   std_logic_vector(3 downto 0) := (others => 'Z');
    psram_cre          : out   std_logic_vector(3 downto 0) := (others => 'Z');
    psram_cen          : out   std_logic_vector(3 downto 0) := (others => '1');
    psram_oen          : out   std_logic_vector(3 downto 0) := (others => 'Z');
    psram_ubn          : out   std_logic_vector(3 downto 0) := (others => 'Z');
    psram_wen          : out   std_logic_vector(3 downto 0) := (others => 'Z');
    psram_lbn          : out   std_logic_vector(3 downto 0) := (others => 'Z');
    psram_wait         : in    std_logic_vector(3 downto 0); -- DDR magic

    -----------------------------------------------------------------------
    -- usb
    -----------------------------------------------------------------------
    usb_slrd_o   : out   std_logic;
    usb_slwr_o   : out   std_logic;
    usb_fd_io    : inout std_logic_vector(7 downto 0);
    usb_pa_io    : inout std_logic_vector(7 downto 0) := (others => 'Z');
    usb_ctl_i    : in    std_logic_vector(2 downto 0);
    usb_ures_o   : out   std_logic;
    usb_clk_i    : in    std_logic;
    usb_uclk_i   : in    std_logic;

    -- ATXMega (F2F) previously CPLD
    -----------------------------------------------------------------------
    f2f_i2c_scl : inout std_logic;
    f2f_i2c_sda : inout std_logic;

    -----------------------------------------------------------------------
    -- SFP (main WR Interface)
    -----------------------------------------------------------------------
    sfp_tx_disable_o   : out   std_logic;                    -- Second SFP (ftm10 only)
    sfp_tx_fault_i     : in    std_logic;                    -- Second SFP (ftm10 only)
    sfp_los_i          : in    std_logic;                    -- Second SFP (ftm10 only)
    sfp_txp_o          : out   std_logic;                    -- Second SFP (ftm10 only)
    sfp_rxp_i          : in    std_logic;                    -- Second SFP (ftm10 only)
    sfp_mod0_i         : in    std_logic;                    -- Second SFP (ftm10 only)
    sfp_mod1_io        : inout std_logic;                    -- Second SFP (ftm10 only)
    sfp_mod2_io        : inout std_logic;                    -- Second SFP (ftm10 only)

    -----------------------------------------------------------------------
    -- SFP (auxiliary - only used on ftm10)
    -----------------------------------------------------------------------
    sfp_aux_tx_disable_o : out   std_logic := '0';
    sfp_aux_tx_fault_i   : in    std_logic;
    sfp_aux_los_i        : in    std_logic;
    sfp_aux_txp_o        : out   std_logic;
    sfp_aux_rxp_i        : in    std_logic;
    sfp_aux_mod0_i       : in    std_logic;
    sfp_aux_mod1_io      : inout std_logic;
    sfp_aux_mod2_io      : inout std_logic;

    -----------------------------------------------------------------------
    -- USBC no USB functionality only LVDS signals
    -----------------------------------------------------------------------
    usbc_tx1_en     : out std_logic_vector(5 downto 1);
    usbc_tx2_en     : out std_logic_vector(5 downto 1);
    usbc_tx3_en     : out std_logic_vector(5 downto 1);
    usbc_tx4_en     : out std_logic_vector(5 downto 1);
    --usbc_tx1_n     : out std_logic_vector(5 downto 1);
    usbc_tx1_p     : out std_logic_vector(5 downto 1);
    --usbc_tx2_n     : out std_logic_vector(5 downto 1);
    usbc_tx2_p     : out std_logic_vector(5 downto 1);
    --usbc_tx3_n     : out std_logic_vector(5 downto 1);
    usbc_tx3_p     : out std_logic_vector(5 downto 1);
    --usbc_tx4_n     : out std_logic_vector(5 downto 1);
    usbc_tx4_p     : out std_logic_vector(5 downto 1);
    usbc_rx1_n     : in std_logic_vector(5 downto 1);
    usbc_rx1_p     : in std_logic_vector(5 downto 1);
    usbc_rx2_n     : in std_logic_vector(5 downto 1);
    usbc_rx2_p     : in std_logic_vector(5 downto 1);
    usbc_rx3_n     : in std_logic_vector(5 downto 1);
    usbc_rx3_p     : in std_logic_vector(5 downto 1);
    usbc_rx4_n     : in std_logic_vector(5 downto 1);
    usbc_rx4_p     : in std_logic_vector(5 downto 1)
    );

end ftm10;

architecture rtl of ftm10 is

  signal s_led_link_up  : std_logic;
  signal s_led_link_act : std_logic;
  signal s_led_track    : std_logic;
  signal s_led_pps      : std_logic;

  signal s_led_aux_link_up  : std_logic;
  signal s_led_aux_link_act : std_logic;
  signal s_led_aux_track    : std_logic;
  signal s_led_aux_pps      : std_logic;

  signal s_sfp_disable : std_logic;

  signal s_gpio_o   : std_logic_vector(19 downto 0);
  signal s_gpio_i   : std_logic_vector(19 downto 0);
  signal s_lvds_p_i : std_logic_vector(19 downto 0);
  signal s_lvds_n_i : std_logic_vector(19 downto 0);
  signal s_lvds_p_o : std_logic_vector(19 downto 0);
  signal s_lvds_n_o : std_logic_vector(19 downto 0);

  signal s_i2c_scl_pad_out  : std_logic_vector(6 downto 1);
  signal s_i2c_scl_pad_in   : std_logic_vector(6 downto 1);
  signal s_i2c_scl_padoen   : std_logic_vector(6 downto 1);
  signal s_i2c_sda_pad_out  : std_logic_vector(6 downto 1);
  signal s_i2c_sda_pad_in   : std_logic_vector(6 downto 1);
  signal s_i2c_sda_padoen   : std_logic_vector(6 downto 1);

  signal s_clk_20m_vcxo_i       : std_logic;
  signal s_clk_125m_pllref_i    : std_logic;
  signal s_clk_125m_local_i     : std_logic;
  signal s_clk_sfp_i            : std_logic;
  signal s_stub_pll_reset       : std_logic;
  signal s_stub_pll_locked      : std_logic;
  signal s_stub_pll_locked_prev : std_logic;

  signal s_psram_ubn     : std_logic;
  signal s_psram_lbn     : std_logic;
  signal s_psram_cen     : std_logic;
  signal s_psram_oen     : std_logic;
  signal s_psram_wen     : std_logic;
  signal s_psram_cre     : std_logic;
  signal s_psram_advn    : std_logic;
  signal s_psram_wait    : std_logic;
  signal s_psram_wait_or : std_logic; -- Remove this later

  constant io_mapping_table : t_io_mapping_table_arg_array(0 to 19) :=
  (
  -- TBD: LEDs are missing, how to implement I2C-controlled IOs? Use spec. out and in?
  -- Name[12 Bytes], Special Purpose, SpecOut, SpecIn, Index, Direction,   Channel,  OutputEnable, Termination, Logic Level
    ("USBC1_IO1  ",  IO_I2C_USB_C,    false,   false,  0,     IO_INOUTPUT, IO_GPIO,  true,         false,       IO_TTL),
    ("USBC1_IO2  ",  IO_I2C_USB_C,    false,   false,  1,     IO_INOUTPUT, IO_GPIO,  true,         false,       IO_TTL),
    ("USBC1_IO3  ",  IO_I2C_USB_C,    false,   false,  2,     IO_INOUTPUT, IO_GPIO,  true,         false,       IO_TTL),
    ("USBC1_IO4  ",  IO_I2C_USB_C,    false,   false,  3,     IO_INOUTPUT, IO_GPIO,  true,         false,       IO_TTL),
    ("USBC1_IO5  ",  IO_I2C_USB_C,    false,   false,  4,     IO_INOUTPUT, IO_GPIO,  true,         false,       IO_TTL),
    ("USBC2_IO1  ",  IO_I2C_USB_C,    false,   false,  5,     IO_INOUTPUT, IO_GPIO,  true,         false,       IO_TTL),
    ("USBC2_IO2  ",  IO_I2C_USB_C,    false,   false,  6,     IO_INOUTPUT, IO_GPIO,  true,         false,       IO_TTL),
    ("USBC2_IO3  ",  IO_I2C_USB_C,    false,   false,  7,     IO_INOUTPUT, IO_GPIO,  true,         false,       IO_TTL),
    ("USBC2_IO4  ",  IO_I2C_USB_C,    false,   false,  8,     IO_INOUTPUT, IO_GPIO,  true,         false,       IO_TTL),
    ("USBC2_IO5  ",  IO_I2C_USB_C,    false,   false,  9,     IO_INOUTPUT, IO_GPIO,  true,         false,       IO_TTL),
    ("USBC3_IO1  ",  IO_I2C_USB_C,    false,   false, 10,     IO_INOUTPUT, IO_GPIO,  true,         false,       IO_TTL),
    ("USBC3_IO2  ",  IO_I2C_USB_C,    false,   false, 11,     IO_INOUTPUT, IO_GPIO,  true,         false,       IO_TTL),
    ("USBC3_IO3  ",  IO_I2C_USB_C,    false,   false, 12,     IO_INOUTPUT, IO_GPIO,  true,         false,       IO_TTL),
    ("USBC3_IO4  ",  IO_I2C_USB_C,    false,   false, 13,     IO_INOUTPUT, IO_GPIO,  true,         false,       IO_TTL),
    ("USBC3_IO5  ",  IO_I2C_USB_C,    false,   false, 14,     IO_INOUTPUT, IO_GPIO,  true,         false,       IO_TTL),
    ("USBC4_IO1  ",  IO_I2C_USB_C,    false,   false, 15,     IO_INOUTPUT, IO_GPIO,  true,         false,       IO_TTL),
    ("USBC4_IO2  ",  IO_I2C_USB_C,    false,   false, 16,     IO_INOUTPUT, IO_GPIO,  true,         false,       IO_TTL),
    ("USBC4_IO3  ",  IO_I2C_USB_C,    false,   false, 17,     IO_INOUTPUT, IO_GPIO,  true,         false,       IO_TTL),
    ("USBC4_IO4  ",  IO_I2C_USB_C,    false,   false, 18,     IO_INOUTPUT, IO_GPIO,  true,         false,       IO_TTL),
    ("USBC4_IO5  ",  IO_I2C_USB_C,    false,   false, 19,     IO_INOUTPUT, IO_GPIO,  true,         false,       IO_TTL)
  );

  constant c_family       : string  := "Arria 10 GX FTM10";
  constant c_project      : string  := "ftm10";
  constant c_initf_name   : string  := c_project & "_stub.mif";
  constant c_profile_name : string  := "medium_icache_debug";
  constant c_psram_bits   : natural := 24;
  constant c_cores        : natural := 4;

begin

  main : monster
    generic map(
      g_family             => c_family,
      g_project            => c_project,
      g_flash_bits         => 25, -- !!! TODO: Check this
      g_psram_bits         => c_psram_bits,
      g_gpio_inout         => 20,
      g_en_i2c_wrapper     => true,
      g_num_i2c_interfaces => 6,
      g_en_pcie            => true,
      g_en_tlu             => false,
      g_en_usb             => true,
      g_en_psram           => true,
      g_io_table           => io_mapping_table,
      g_en_a10ts           => true,
      g_a10_use_sys_fpll   => false,
      g_a10_use_ref_fpll   => false,
      g_dual_port_wr       => true,
      g_en_eca             => false,
      g_delay_diagnostics  => true,
      g_lm32_are_ftm       => true,
      g_lm32_MSIs          => 1,
      g_lm32_cores         => c_cores,
      g_lm32_ramsizes      => c_lm32_ramsizes/4,
      g_lm32_init_files    => f_string_list_repeat(c_initf_name, c_cores),
      g_lm32_profiles      => f_string_list_repeat(c_profile_name, c_cores),
      g_en_asmi            => true,
      g_en_a10vs           => true
    )
    port map(
      core_clk_20m_vcxo_i     => clk_20m_vcxo_i,
      aux_clk_20m_vcxo_i      => clk_20m_vcxo_i,
      core_clk_125m_pllref_i  => clk_125m_tcb_pllref_i,
      core_clk_125m_sfpref_i  => clk_125m_tcb_pllref_i,
      aux_clk_125m_pllref_i   => clk_125m_tcb_pllref_i,
      aux_clk_125m_sfpref_i   => clk_125m_tcb_pllref_i,
      core_clk_125m_local_i   => clk_125m_tcb_local_i,
      wr_dac_sclk_o           => wr_dac_sclk_o,
      wr_dac_din_o            => wr_dac_din_o,
      wr_ndac_cs_o            => wr_ndac_cs_o,
      wr_onewire_io           => OneWire_CB,
      wr_aux_onewire_io       => OneWire_aux_CB,
      wr_sfp_sda_io           => sfp_mod2_io,
      wr_sfp_scl_io           => sfp_mod1_io,
      wr_sfp_det_i            => sfp_mod0_i,
      wr_sfp_tx_o             => sfp_txp_o,
      wr_sfp_rx_i             => sfp_rxp_i,
      wr_aux_sfp_sda_io       => sfp_aux_mod2_io,
      wr_aux_sfp_scl_io       => sfp_aux_mod1_io,
      wr_aux_sfp_det_i        => sfp_aux_mod0_i,
      wr_aux_sfp_tx_o         => sfp_aux_txp_o,
      wr_aux_sfp_rx_i         => sfp_aux_rxp_i,
      sfp_tx_disable_o        => open,
      sfp_tx_fault_i          => sfp_tx_fault_i,
      sfp_los_i               => sfp_los_i,
      sfp_aux_tx_disable_o    => open,
      sfp_aux_tx_fault_i      => sfp_aux_tx_fault_i,
      sfp_aux_los_i           => sfp_aux_los_i,
      wbar_phy_dis_o          => s_sfp_disable,
      i2c_scl_pad_i           => s_i2c_scl_pad_in,
      i2c_scl_pad_o           => s_i2c_scl_pad_out,
      i2c_scl_padoen_o        => s_i2c_scl_padoen,
      i2c_sda_pad_i           => s_i2c_sda_pad_in,
      i2c_sda_pad_o           => s_i2c_sda_pad_out,
      i2c_sda_padoen_o        => s_i2c_sda_padoen,
      gpio_o                  => s_gpio_o,
      gpio_i                  => s_gpio_i,
      gpio_oen_o(4 downto 0)   => usbc_tx1_en,
      gpio_oen_o(9 downto 5)   => usbc_tx2_en,
      gpio_oen_o(14 downto 10) => usbc_tx3_en,
      gpio_oen_o(19 downto 15) => usbc_tx4_en,
      usb_rstn_o              => usb_ures_o,
      usb_ebcyc_i             => usb_pa_io(3),
      usb_speed_i             => usb_pa_io(0),
      usb_shift_i             => usb_pa_io(1),
      usb_readyn_io           => usb_pa_io(7),
      usb_fifoadr_o           => usb_pa_io(5 downto 4),
      usb_sloen_o             => usb_pa_io(2),
      usb_fulln_i             => usb_ctl_i(1),
      usb_emptyn_i            => usb_ctl_i(2),
      usb_slrdn_o             => usb_slrd_o,
      usb_slwrn_o             => usb_slwr_o,
      usb_pktendn_o           => usb_pa_io(6),
      usb_fd_io               => usb_fd_io,
      led_link_up_o           => s_led_link_up,
      led_link_act_o          => s_led_link_act,
      led_track_o             => s_led_track,
      led_pps_o               => s_led_pps,
      led_aux_link_up_o       => s_led_aux_link_up,
      led_aux_link_act_o      => s_led_aux_link_act,
      led_aux_track_o         => s_led_aux_track,
      led_aux_pps_o           => s_led_aux_pps,
      pcie_refclk_i           => pcie_refclk_i,
      pcie_rstn_i             => nPCI_RESET_i,
      pcie_rx_i               => pcie_rx_i,
      pcie_tx_o               => pcie_tx_o,
      --PSRAM TODO: Multi Chip
      ps_clk                  => psram_clk,
      ps_addr                 => psram_a,
      ps_data                 => psram_dq,
      ps_seln(0)              => s_psram_ubn,
      ps_seln(1)              => s_psram_lbn,
      ps_cen                  => s_psram_cen,
      ps_oen                  => s_psram_oen,
      ps_wen                  => s_psram_wen,
      ps_cre                  => s_psram_cre,
      ps_advn                 => s_psram_advn,
      ps_wait                 => s_psram_wait_or);

  -- SFP management
  sfp_tx_disable_o     <= s_sfp_disable;
  sfp_aux_tx_disable_o <= s_sfp_disable;

  -- Use only one PSRAM (TBD: Multichip support)
  psram_single : if not(g_quad_mode_psram) generate
    psram_gen : for i in 0 to 3 generate
      psram_enable : if (g_default_psram = i) generate
        s_psram_wait_or <= psram_wait(i);
        psram_advn(i)   <= s_psram_advn;
        psram_cre(i)    <= s_psram_cre;
        psram_cen(i)    <= s_psram_cen;
        psram_oen(i)    <= s_psram_oen;
        psram_ubn(i)    <= s_psram_ubn;
        psram_wen(i)    <= s_psram_wen;
        psram_lbn(i)    <= s_psram_lbn;
      end generate; -- psram_enable
      psram_disable : if not(g_default_psram = i) generate
        psram_advn(i) <= '0';
        psram_cre(i)  <= '0';
        psram_cen(i)  <= '1';
        psram_oen(i)  <= '1';
        psram_ubn(i)  <= '0';
        psram_wen(i)  <= '1';
        psram_lbn(i)  <= '0';
      end generate; -- psram_disable
    end generate; -- psram_gen
  end generate; -- psram_single

  psram_quad : if (g_quad_mode_psram) generate
    s_psram_wait_or <= psram_wait(0) or psram_wait(1) or psram_wait(2) or psram_wait(3);
    psram_quad_gen : for i in 0 to 3 generate
      psram_advn(i)   <= s_psram_advn;
      psram_cre(i)    <= s_psram_cre;
      psram_cen(i)    <= s_psram_cen;
      psram_oen(i)    <= s_psram_oen;
      psram_ubn(i)    <= s_psram_ubn;
      psram_wen(i)    <= s_psram_wen;
      psram_lbn(i)    <= s_psram_lbn;
    end generate; -- psram_quad_gen
  end generate; -- psram_quad

  -- LEDs
  wr_leds_o(0)                  <= not (s_led_link_act and s_led_link_up);          -- red   = traffic/no-link
  wr_leds_o(1)                  <= not s_led_track;                                 -- green = timing valid
  wr_leds_o(2)                  <= not (s_led_link_up and not(s_led_track));        -- blue  = link
  wr_leds_o(3)                  <= not s_led_pps;                                   -- white = PPS
  wr_aux_leds_or_node_leds_o(0) <= not (s_led_aux_link_act and s_led_aux_link_up);  -- red   = traffic/no-link
  wr_aux_leds_or_node_leds_o(1) <= not s_led_aux_track;                             -- green = timing valid
  wr_aux_leds_or_node_leds_o(2) <= not (s_led_aux_link_up and not(s_led_aux_track));-- blue  = link
  wr_aux_leds_or_node_leds_o(3) <= not s_led_aux_pps;                               -- white = PPS

  -------------------------------------------------
  -- LVDS USBC mapping
  -------------------------------------------------
  -- USBC TX LVDS output
  usbc_tx : for i in 0 to 4 generate
    usbc_tx1_p(i+1) <= s_gpio_o(i);
    usbc_tx2_p(i+1) <= s_gpio_o(i+5);
    usbc_tx3_p(i+1) <= s_gpio_o(i+10);
    usbc_tx4_p(i+1) <= s_gpio_o(i+15);

--    lvds_to_single_gpio_tx1 : altera_lvds_obuf
--      generic map(
--        g_family  => c_family)
--      port map(
--        datain    => s_gpio_o(i),
--        dataout   => usbc_tx1_p(i+1),
--        dataout_b => usbc_tx1_n(i+1)
--      );

--    lvds_to_single_gpio_tx2 : altera_lvds_obuf
--      generic map(
--        g_family  => c_family)
--      port map(
--        datain    => s_gpio_o(i+5),
--        dataout   => usbc_tx2_p(i+1),
--        dataout_b => usbc_tx2_n(i+1)
--      );

--    lvds_to_single_gpio_tx3 : altera_lvds_obuf
--      generic map(
--        g_family  => c_family)
--      port map(
--        datain    => s_gpio_o(i+10),
--        dataout   => usbc_tx3_p(i+1),
--        dataout_b => usbc_tx3_n(i+1)
--      );

--    lvds_to_single_gpio_tx4 : altera_lvds_obuf
--      generic map(
--        g_family  => c_family)
--      port map(
--        datain    => s_gpio_o(i+15),
--        dataout   => usbc_tx4_p(i+1),
--        dataout_b => usbc_tx4_n(i+1)
--      );
  end generate;

  -- USBC RX LVDS input
  usbc_rx : for i in 0 to 4 generate
    lvds_to_single_gpio_rx1 : altera_lvds_ibuf
      generic map(
        g_family  => c_family)
      port map(
        datain_b  => usbc_rx1_n(i+1),
        datain    => usbc_rx1_p(i+1),
        dataout   => s_gpio_i(i)
      );

    lvds_to_single_gpio_rx2 : altera_lvds_ibuf
      generic map(
        g_family  => c_family)
      port map(
        datain_b  => usbc_rx2_n(i+1),
        datain    => usbc_rx2_p(i+1),
        dataout   => s_gpio_i(i+5)
      );

    lvds_to_single_gpio_rx3 : altera_lvds_ibuf
      generic map(
        g_family  => c_family)
      port map(
        datain_b  => usbc_rx3_n(i+1),
        datain    => usbc_rx3_p(i+1),
        dataout   => s_gpio_i(i+10)
      );

    lvds_to_single_gpio_rx4 : altera_lvds_ibuf
      generic map(
        g_family  => c_family)
      port map(
        datain_b  => usbc_rx4_n(i+1),
        datain    => usbc_rx4_p(i+1),
        dataout   => s_gpio_i(i+15)
      );
  end generate;

  -- I2C
  interfaces : for i in 2 to 5 generate
    i2c_scl_pad_io(i-1) <= s_i2c_scl_pad_out(i) when (s_i2c_scl_padoen(i) = '0') else 'Z';
    i2c_sda_pad_io(i-1) <= s_i2c_sda_pad_out(i) when (s_i2c_sda_padoen(i) = '0') else 'Z';
    s_i2c_scl_pad_in(i) <= i2c_scl_pad_io(i-1);
    s_i2c_sda_pad_in(i) <= i2c_sda_pad_io(i-1);
  end generate;

  -- CPLD
  -- ATXMega (F2F) previously CPLD
  --s_gpio_i(5 downto 0) <= cpld_io(5 downto 0);
  --cpld_con : for i in 0 to 5 generate
    --cpld_io(i) <= s_gpio_o(i) when s_gpio_o(i)='0' else 'Z';
    --cpld_io(i) <= 'Z';
  --end generate;

  -- I2C to ATXMega
  f2f_i2c_scl         <= s_i2c_scl_pad_out(1) when (s_i2c_scl_padoen(1) = '0') else 'Z';
  f2f_i2c_sda         <= s_i2c_sda_pad_out(1) when (s_i2c_sda_padoen(1) = '0') else 'Z';
  s_i2c_scl_pad_in(1) <= f2f_i2c_scl ;
  s_i2c_sda_pad_in(1) <= f2f_i2c_sda;

  -- OneWire
  OneWire_CB_splz     <= '1'; -- Strong Pull-Up disabled
  OneWire_aux_CB_splz <= '1'; -- Strong Pull-Up disabled

end rtl;
