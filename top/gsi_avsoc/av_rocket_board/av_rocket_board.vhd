library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.monster_pkg.all;
use work.ramsize_pkg.c_lm32_ramsizes;
use work.ghrd_5astfd5k3_pkg.all;

entity av_rocket_board is
  port(
    -- Monster ports
	 sfp234_ref_clk_i       : in  std_logic;
    sfp4_tx_disable_o      : out   std_logic;
    sfp4_tx_fault          : in    std_logic;
    sfp4_los               : in    std_logic;
    sfp4_txp_o             : out   std_logic;
    sfp4_rxp_i             : in    std_logic;
    sfp4_mod0              : in    std_logic; -- grounded by module
    sfp4_mod1              : inout std_logic; -- SCL
    sfp4_mod2              : inout std_logic; -- SDA
    rom_data               : inout std_logic; -- OneWire
    dac_sclk               : out   std_logic; -- WR
    dac_din                : out   std_logic; -- WR
    ndac_cs                : out   std_logic_vector(2 downto 1); -- WR
    pcie_refclk_i          : in    std_logic; -- PCIe
    pcie_rx_i              : in    std_logic_vector(3 downto 0); -- PCIe
    pcie_tx_o              : out   std_logic_vector(3 downto 0); -- PCIe
    pcie_resetn_i          : in    std_logic; -- PCIe
    -- FPGA peripherals ports
    fpga_dipsw_pio         : in    std_logic_vector(3 downto 0);
    fpga_led_pio           : out   std_logic_vector(3 downto 0);
    fpga_button_pio        : in    std_logic_vector(3 downto 0);
    -- HPS memory controller ports
    hps_memory_mem_a       : out   std_logic_vector(14 downto 0);
    hps_memory_mem_ba      : out   std_logic_vector(2 downto 0);
    hps_memory_mem_ck      : out   std_logic;
    hps_memory_mem_ck_n    : out   std_logic;
    hps_memory_mem_cke     : out   std_logic;
    hps_memory_mem_cs_n    : out   std_logic;
    hps_memory_mem_ras_n   : out   std_logic;
    hps_memory_mem_cas_n   : out   std_logic;
    hps_memory_mem_we_n    : out   std_logic;
    hps_memory_mem_reset_n : out   std_logic;
    hps_memory_mem_dq      : inout std_logic_vector(39 downto 0);
    hps_memory_mem_dqs     : inout std_logic_vector(4 downto 0);
    hps_memory_mem_dqs_n   : inout std_logic_vector(4 downto 0);
    hps_memory_mem_odt     : out   std_logic;
    hps_memory_mem_dm      : out   std_logic_vector(4 downto 0);
    hps_memory_oct_rzqin   : in    std_logic;
    -- HPS peripherals
    hps_emac1_TX_CLK       : out   std_logic;
    hps_emac1_TXD0         : out   std_logic;
    hps_emac1_TXD1         : out   std_logic;
    hps_emac1_TXD2         : out   std_logic;
    hps_emac1_TXD3         : out   std_logic;
    hps_emac1_RXD0         : in    std_logic;
    hps_emac1_MDIO         : inout std_logic;
    hps_emac1_MDC          : out   std_logic;
    hps_emac1_RX_CTL       : in    std_logic;
    hps_emac1_TX_CTL       : out   std_logic;
    hps_emac1_RX_CLK       : in    std_logic;
    hps_emac1_RXD1         : in    std_logic;
    hps_emac1_RXD2         : in    std_logic;
    hps_emac1_RXD3         : in    std_logic;
    hps_qspi_IO0           : inout std_logic;
    hps_qspi_IO1           : inout std_logic;
    hps_qspi_IO2           : inout std_logic;
    hps_qspi_IO3           : inout std_logic;
    hps_qspi_SS0           : out   std_logic;
    hps_qspi_CLK           : out   std_logic;
    hps_sdio_CMD           : inout std_logic;
    hps_sdio_D0            : inout std_logic;
    hps_sdio_D1            : inout std_logic;
    hps_sdio_CLK           : out   std_logic;
    hps_sdio_D2            : inout std_logic;
    hps_sdio_D3            : inout std_logic;
    hps_usb1_D0            : inout std_logic;
    hps_usb1_D1            : inout std_logic;
    hps_usb1_D2            : inout std_logic;
    hps_usb1_D3            : inout std_logic;
    hps_usb1_D4            : inout std_logic;
    hps_usb1_D5            : inout std_logic;
    hps_usb1_D6            : inout std_logic;
    hps_usb1_D7            : inout std_logic;
    hps_usb1_CLK           : in    std_logic;
    hps_usb1_STP           : out   std_logic;
    hps_usb1_DIR           : in    std_logic;
    hps_usb1_NXT           : in    std_logic;
    hps_uart0_RX           : in    std_logic;
    hps_uart0_TX           : out   std_logic;
    hps_uart1_RX           : in    std_logic;
    hps_uart1_TX           : out   std_logic;
    hps_i2c0_SDA           : inout std_logic;
    hps_i2c0_SCL           : inout std_logic;
    hps_trace_CLK          : out   std_logic;
    hps_trace_D0           : out   std_logic;
    hps_trace_D1           : out   std_logic;
    hps_trace_D2           : out   std_logic;
    hps_trace_D3           : out   std_logic;
    hps_trace_D4           : out   std_logic;
    hps_trace_D5           : out   std_logic;
    hps_trace_D6           : out   std_logic;
    hps_trace_D7           : out   std_logic;
    hps_gpio_GPIO00        : inout std_logic;
    hps_gpio_GPIO17        : inout std_logic;
    hps_gpio_GPIO18        : inout std_logic;
    hps_gpio_GPIO22        : inout std_logic;
    hps_gpio_GPIO24        : inout std_logic;
    hps_gpio_GPIO26        : inout std_logic;
    hps_gpio_GPIO27        : inout std_logic;
    hps_gpio_GPIO35        : inout std_logic;
    hps_gpio_GPIO40        : inout std_logic;
    hps_gpio_GPIO41        : inout std_logic;
    hps_gpio_GPIO42        : inout std_logic;
    hps_gpio_GPIO43        : inout std_logic;
    -- FPGA clock and reset
    fpga_clk_50            : in    std_logic
  );
end av_rocket_board;

architecture rtl of av_rocket_board is

  -- PLL stub signals
  signal clk_20m_vcxo    : std_logic;
  signal clk_125m_pllref : std_logic;
  signal clk_125m_sfpref : std_logic;
  signal clk_125m_local  : std_logic;
  
  -- Monster signals
  constant c_family       : string := "Arria V"; 
  constant c_project      : string := "av_rocket_board";
  constant c_cores        : natural:= 1;
  constant c_initf_name   : string := c_project & ".mif";
  constant c_profile_name : string := "medium_icache_debug";
  
  constant io_mapping_table : t_io_mapping_table_arg_array(0 to 3) := 
  (
  -- Name[12 Bytes], Special Purpose, SpecOut, SpecIn, Index, Direction,   Channel,  OutputEnable, Termination, Logic Level
    ("LED1       ",  IO_NONE,         false,   false,  0,     IO_INOUTPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("LED2       ",  IO_NONE,         false,   false,  1,     IO_INOUTPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("LED3       ",  IO_NONE,         false,   false,  2,     IO_INOUTPUT,    IO_GPIO,  false,        false,       IO_TTL),
    ("LED4       ",  IO_NONE,         false,   false,  3,     IO_INOUTPUT,    IO_GPIO,  false,        false,       IO_TTL)
  );

  -- HPS signals
  signal stm_hw_events          : std_logic_vector(27 downto 0);
  signal fpga_led_internal      : std_logic_vector(3 downto 0);
  signal fpga_debounced_buttons : std_logic_vector(3 downto 0);
  signal hps_fpga_reset_n       : std_logic;
  signal hps_cold_reset         : std_logic;
  signal hps_warm_reset         : std_logic;
  signal hps_debug_reset        : std_logic;
  signal hps_reset_req          : std_logic_vector(2 downto 0);
  
  signal led_blinking: std_logic := '0';
  signal led_counter : std_logic_vector(31 downto 0) := (others => '0');
  
  signal s_button_clk : std_logic;
  signal s_led_button_clk : std_logic;
  signal s_button : std_logic;
  signal s_led_button : std_logic;

begin

  -- PLL stub
  pll_inst : stub_pll
    port map(
      refclk   => fpga_clk_50,
      rst      => hps_fpga_reset_n,
      outclk_0 => clk_125m_pllref,
      outclk_1 => clk_125m_sfpref,
      outclk_2 => clk_125m_local,
      outclk_3 => clk_20m_vcxo
    );
	 
  proc_led_button: process( clk_125m_pllref )
  begin
    if rising_edge(clk_125m_pllref) then
	   if unsigned(led_counter) = 0 then
		  led_counter <= std_logic_vector(to_unsigned(125000000, 32));
	     led_blinking <= not led_blinking;
		else
		  led_counter <= std_logic_vector(unsigned(led_counter) - 1);
		end if;
		
		s_led_button_clk <= s_button_clk;
		
	 end if;
  end process;
  
  s_button_clk <= fpga_button_pio(0);
  s_button <= fpga_button_pio(1);
  s_led_button <= s_button;
  
  fpga_led_pio(0) <=        led_blinking;
  fpga_led_pio(1) <=        s_led_button_clk;
  fpga_led_pio(2) <=        s_led_button;
  fpga_led_pio(3) <=        not led_blinking;

  
  -- Monster ...
  monster_inst : monster
    generic map(
      g_family          => c_family,
      g_project         => c_project,
      g_flash_bits      => 25,
      g_gpio_inout      => io_mapping_table'LENGTH,
	   g_en_pcie         => true,
      g_io_table        => io_mapping_table,
      g_lm32_cores      => c_cores,
      g_lm32_ramsizes   => c_lm32_ramsizes/4,
      g_lm32_init_files => f_string_list_repeat(c_initf_name & ".mif", c_cores),
      g_lm32_profiles   => f_string_list_repeat(c_profile_name, c_cores)
    )  
    port map(
      core_clk_20m_vcxo_i    => clk_20m_vcxo,
      core_clk_125m_pllref_i => clk_125m_pllref,
      core_clk_125m_sfpref_i => sfp234_ref_clk_i,
      core_clk_125m_local_i  => clk_125m_local,
      wr_onewire_io          => rom_data,
      wr_sfp_sda_io          => sfp4_mod2,
      wr_sfp_scl_io          => sfp4_mod1,
      wr_sfp_det_i           => sfp4_mod0,
      wr_sfp_tx_o            => sfp4_txp_o,
      wr_sfp_rx_i            => sfp4_rxp_i,
      wr_dac_sclk_o          => dac_sclk,
      wr_dac_din_o           => dac_din,
      wr_ndac_cs_o           => ndac_cs,
      pcie_refclk_i          => pcie_refclk_i,
      pcie_rstn_i            => pcie_resetn_i,
      pcie_rx_i              => pcie_rx_i,
      pcie_tx_o              => pcie_tx_o
    );
  
  -- Monster connections
  sfp4_tx_disable_o <= '0';
  
  -- SoC sub-system module
  soc_inst : ghrd_5astfd5k3
    port map(
      hps_0_f2h_stm_hw_events_stm_hwevents  => stm_hw_events,
      memory_mem_a                          => hps_memory_mem_a,
      memory_mem_ba                         => hps_memory_mem_ba,
      memory_mem_ck                         => hps_memory_mem_ck,
      memory_mem_ck_n                       => hps_memory_mem_ck_n,
      memory_mem_cke                        => hps_memory_mem_cke,
      memory_mem_cs_n                       => hps_memory_mem_cs_n,
      memory_mem_ras_n                      => hps_memory_mem_ras_n,
      memory_mem_cas_n                      => hps_memory_mem_cas_n,
      memory_mem_we_n                       => hps_memory_mem_we_n,
      memory_mem_reset_n                    => hps_memory_mem_reset_n,
      memory_mem_dq                         => hps_memory_mem_dq,
      memory_mem_dqs                        => hps_memory_mem_dqs,
      memory_mem_dqs_n                      => hps_memory_mem_dqs_n,
      memory_mem_odt                        => hps_memory_mem_odt,
      memory_mem_dm                         => hps_memory_mem_dm,
      memory_oct_rzqin                      => hps_memory_oct_rzqin,
      dipsw_pio_external_connection_export  => fpga_dipsw_pio,
      led_pio_external_connection_in_port   => fpga_led_internal,
      led_pio_external_connection_out_port  => fpga_led_internal,
      button_pio_external_connection_export => fpga_debounced_buttons,
      hps_io_hps_io_emac1_inst_TX_CLK       => hps_emac1_TX_CLK, 
      hps_io_hps_io_emac1_inst_TXD0         => hps_emac1_TXD0,
      hps_io_hps_io_emac1_inst_TXD1         => hps_emac1_TXD1,
      hps_io_hps_io_emac1_inst_TXD2         => hps_emac1_TXD2,
      hps_io_hps_io_emac1_inst_TXD3         => hps_emac1_TXD3,
      hps_io_hps_io_emac1_inst_RXD0         => hps_emac1_RXD0,
      hps_io_hps_io_emac1_inst_MDIO         => hps_emac1_MDIO,
      hps_io_hps_io_emac1_inst_MDC          => hps_emac1_MDC,
      hps_io_hps_io_emac1_inst_RX_CTL       => hps_emac1_RX_CTL,
      hps_io_hps_io_emac1_inst_TX_CTL       => hps_emac1_TX_CTL,
      hps_io_hps_io_emac1_inst_RX_CLK       => hps_emac1_RX_CLK,
      hps_io_hps_io_emac1_inst_RXD1         => hps_emac1_RXD1,
      hps_io_hps_io_emac1_inst_RXD2         => hps_emac1_RXD2,
      hps_io_hps_io_emac1_inst_RXD3         => hps_emac1_RXD3,
      hps_io_hps_io_qspi_inst_IO0           => hps_qspi_IO0,
      hps_io_hps_io_qspi_inst_IO1           => hps_qspi_IO1,
      hps_io_hps_io_qspi_inst_IO2           => hps_qspi_IO2,
      hps_io_hps_io_qspi_inst_IO3           => hps_qspi_IO3,
      hps_io_hps_io_qspi_inst_SS0           => hps_qspi_SS0,
      hps_io_hps_io_qspi_inst_CLK           => hps_qspi_CLK,
      hps_io_hps_io_sdio_inst_CMD           => hps_sdio_CMD,
      hps_io_hps_io_sdio_inst_D0            => hps_sdio_D0,
      hps_io_hps_io_sdio_inst_D1            => hps_sdio_D1,
      hps_io_hps_io_sdio_inst_CLK           => hps_sdio_CLK,
      hps_io_hps_io_sdio_inst_D2            => hps_sdio_D2,
      hps_io_hps_io_sdio_inst_D3            => hps_sdio_D3,
      hps_io_hps_io_usb1_inst_D0            => hps_usb1_D0,
      hps_io_hps_io_usb1_inst_D1            => hps_usb1_D1,
      hps_io_hps_io_usb1_inst_D2            => hps_usb1_D2,
      hps_io_hps_io_usb1_inst_D3            => hps_usb1_D3,
      hps_io_hps_io_usb1_inst_D4            => hps_usb1_D4,
      hps_io_hps_io_usb1_inst_D5            => hps_usb1_D5,
      hps_io_hps_io_usb1_inst_D6            => hps_usb1_D6,
      hps_io_hps_io_usb1_inst_D7            => hps_usb1_D7,
      hps_io_hps_io_usb1_inst_CLK           => hps_usb1_CLK,
      hps_io_hps_io_usb1_inst_STP           => hps_usb1_STP,
      hps_io_hps_io_usb1_inst_DIR           => hps_usb1_DIR,
      hps_io_hps_io_usb1_inst_NXT           => hps_usb1_NXT,
      hps_io_hps_io_uart0_inst_RX           => hps_uart0_RX,
      hps_io_hps_io_uart0_inst_TX           => hps_uart0_TX,
      hps_io_hps_io_uart1_inst_RX           => hps_uart1_RX,
      hps_io_hps_io_uart1_inst_TX           => hps_uart1_TX,
      hps_io_hps_io_i2c0_inst_SDA           => hps_i2c0_SDA,
      hps_io_hps_io_i2c0_inst_SCL           => hps_i2c0_SCL,
      hps_io_hps_io_trace_inst_CLK          => hps_trace_CLK,
      hps_io_hps_io_trace_inst_D0           => hps_trace_D0,
      hps_io_hps_io_trace_inst_D1           => hps_trace_D1,
      hps_io_hps_io_trace_inst_D2           => hps_trace_D2,
      hps_io_hps_io_trace_inst_D3           => hps_trace_D3,
      hps_io_hps_io_trace_inst_D4           => hps_trace_D4,
      hps_io_hps_io_trace_inst_D5           => hps_trace_D5,
      hps_io_hps_io_trace_inst_D6           => hps_trace_D6,
      hps_io_hps_io_trace_inst_D7           => hps_trace_D7,
      hps_io_hps_io_gpio_inst_GPIO00        => hps_gpio_GPIO00,
      hps_io_hps_io_gpio_inst_GPIO17        => hps_gpio_GPIO17,
      hps_io_hps_io_gpio_inst_GPIO18        => hps_gpio_GPIO18,
      hps_io_hps_io_gpio_inst_GPIO22        => hps_gpio_GPIO22,
      hps_io_hps_io_gpio_inst_GPIO24        => hps_gpio_GPIO24,
      hps_io_hps_io_gpio_inst_GPIO26        => hps_gpio_GPIO26,
      hps_io_hps_io_gpio_inst_GPIO27        => hps_gpio_GPIO27,
      hps_io_hps_io_gpio_inst_GPIO35        => hps_gpio_GPIO35,
      hps_io_hps_io_gpio_inst_GPIO40        => hps_gpio_GPIO40,
      hps_io_hps_io_gpio_inst_GPIO41        => hps_gpio_GPIO41,
      hps_io_hps_io_gpio_inst_GPIO42        => hps_gpio_GPIO42,
      hps_io_hps_io_gpio_inst_GPIO43        => hps_gpio_GPIO43,
      clk_clk                               => fpga_clk_50,
      hps_0_h2f_reset_reset_n               => hps_fpga_reset_n,
      reset_reset_n                         => hps_fpga_reset_n,
      hps_0_f2h_cold_reset_req_reset_n      => not(hps_cold_reset),
      hps_0_f2h_warm_reset_req_reset_n      => not(hps_warm_reset),
      hps_0_f2h_debug_reset_req_reset_n     => not(hps_debug_reset)
    );
  
  -- SoC sub-system module, debounce logic to clean out glitches within 1ms
  debounce_inst : altera_wrapper_debounce
    port map(
      clk      => fpga_clk_50,
      reset_n  => hps_fpga_reset_n,
      data_in  => fpga_button_pio,
      data_out => fpga_debounced_buttons
    );
  
  -- SoC sub-system module, source/probe megawizard instance
  hps_reset_inst : hps_reset
    port map(
      source_clk => fpga_clk_50,
      source     => hps_reset_req
    );
  
  -- SoC sub-system reset handling (cold reset)
  pulse_cold_reset_inst : altera_wrapper_pcr
    port map(
      clk       => fpga_clk_50,
      rst_n     => hps_fpga_reset_n,
      signal_in => hps_reset_req(0),
      pulse_out => hps_cold_reset
    );
  
  -- SoC sub-system reset handling (warm reset)
  pulse_warm_reset_inst : altera_wrapper_pwr
    port map(
      clk       => fpga_clk_50,
      rst_n     => hps_fpga_reset_n,
      signal_in => hps_reset_req(1),
      pulse_out => hps_warm_reset
    );
  
  -- SoC sub-system reset handling (debug reset)
  pulse_debug_reset_inst : altera_wrapper_pdr
    port map(
      clk       => fpga_clk_50,
      rst_n     => hps_fpga_reset_n,
      signal_in => hps_reset_req(2),
      pulse_out => hps_debug_reset
    );
  
  -- SoC connections
  --fpga_led_pio                <= not(fpga_led_internal); -- !!! Inverted to verify the correct bitstream
  stm_hw_events(3  downto  0) <= fpga_debounced_buttons;
  stm_hw_events(7  downto  4) <= fpga_led_internal;
  stm_hw_events(11 downto  8) <= fpga_dipsw_pio;
  stm_hw_events(27 downto 12) <= (others => '0');
  
end rtl;
