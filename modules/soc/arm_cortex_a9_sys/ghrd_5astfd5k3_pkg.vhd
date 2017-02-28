-- libraries and packages
-- ieee
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- package declaration
package ghrd_5astfd5k3_pkg is
  component ghrd_5astfd5k3 is
    port (
      button_pio_external_connection_export : in    std_logic_vector(3 downto 0)  := (others => 'X'); -- export
      clk_clk                               : in    std_logic                     := 'X';             -- clk
      dipsw_pio_external_connection_export  : in    std_logic_vector(3 downto 0)  := (others => 'X'); -- export
      hps_0_f2h_cold_reset_req_reset_n      : in    std_logic                     := 'X';             -- reset_n
      hps_0_f2h_debug_reset_req_reset_n     : in    std_logic                     := 'X';             -- reset_n
      hps_0_f2h_stm_hw_events_stm_hwevents  : in    std_logic_vector(27 downto 0) := (others => 'X'); -- stm_hwevents
      hps_0_f2h_warm_reset_req_reset_n      : in    std_logic                     := 'X';             -- reset_n
      hps_0_h2f_reset_reset_n               : out   std_logic;                                        -- reset_n
      hps_io_hps_io_emac1_inst_TX_CLK       : out   std_logic;                                        -- hps_io_emac1_inst_TX_CLK
      hps_io_hps_io_emac1_inst_TXD0         : out   std_logic;                                        -- hps_io_emac1_inst_TXD0
      hps_io_hps_io_emac1_inst_TXD1         : out   std_logic;                                        -- hps_io_emac1_inst_TXD1
      hps_io_hps_io_emac1_inst_TX_CTL       : out   std_logic;                                        -- hps_io_emac1_inst_TX_CTL
      hps_io_hps_io_emac1_inst_RXD0         : in    std_logic                     := 'X';             -- hps_io_emac1_inst_RXD0
      hps_io_hps_io_emac1_inst_RXD1         : in    std_logic                     := 'X';             -- hps_io_emac1_inst_RXD1
      hps_io_hps_io_emac1_inst_TXD2         : out   std_logic;                                        -- hps_io_emac1_inst_TXD2
      hps_io_hps_io_emac1_inst_TXD3         : out   std_logic;                                        -- hps_io_emac1_inst_TXD3
      hps_io_hps_io_emac1_inst_MDIO         : inout std_logic                     := 'X';             -- hps_io_emac1_inst_MDIO
      hps_io_hps_io_emac1_inst_MDC          : out   std_logic;                                        -- hps_io_emac1_inst_MDC
      hps_io_hps_io_emac1_inst_RX_CTL       : in    std_logic                     := 'X';             -- hps_io_emac1_inst_RX_CTL
      hps_io_hps_io_emac1_inst_RX_CLK       : in    std_logic                     := 'X';             -- hps_io_emac1_inst_RX_CLK
      hps_io_hps_io_emac1_inst_RXD2         : in    std_logic                     := 'X';             -- hps_io_emac1_inst_RXD2
      hps_io_hps_io_emac1_inst_RXD3         : in    std_logic                     := 'X';             -- hps_io_emac1_inst_RXD3
      hps_io_hps_io_qspi_inst_IO0           : inout std_logic                     := 'X';             -- hps_io_qspi_inst_IO0
      hps_io_hps_io_qspi_inst_IO1           : inout std_logic                     := 'X';             -- hps_io_qspi_inst_IO1
      hps_io_hps_io_qspi_inst_IO2           : inout std_logic                     := 'X';             -- hps_io_qspi_inst_IO2
      hps_io_hps_io_qspi_inst_IO3           : inout std_logic                     := 'X';             -- hps_io_qspi_inst_IO3
      hps_io_hps_io_qspi_inst_SS0           : out   std_logic;                                        -- hps_io_qspi_inst_SS0
      hps_io_hps_io_qspi_inst_CLK           : out   std_logic;                                        -- hps_io_qspi_inst_CLK
      hps_io_hps_io_sdio_inst_CMD           : inout std_logic                     := 'X';             -- hps_io_sdio_inst_CMD
      hps_io_hps_io_sdio_inst_D0            : inout std_logic                     := 'X';             -- hps_io_sdio_inst_D0
      hps_io_hps_io_sdio_inst_D1            : inout std_logic                     := 'X';             -- hps_io_sdio_inst_D1
      hps_io_hps_io_sdio_inst_CLK           : out   std_logic;                                        -- hps_io_sdio_inst_CLK
      hps_io_hps_io_sdio_inst_D2            : inout std_logic                     := 'X';             -- hps_io_sdio_inst_D2
      hps_io_hps_io_sdio_inst_D3            : inout std_logic                     := 'X';             -- hps_io_sdio_inst_D3
      hps_io_hps_io_usb1_inst_D0            : inout std_logic                     := 'X';             -- hps_io_usb1_inst_D0
      hps_io_hps_io_usb1_inst_D1            : inout std_logic                     := 'X';             -- hps_io_usb1_inst_D1
      hps_io_hps_io_usb1_inst_D2            : inout std_logic                     := 'X';             -- hps_io_usb1_inst_D2
      hps_io_hps_io_usb1_inst_D3            : inout std_logic                     := 'X';             -- hps_io_usb1_inst_D3
      hps_io_hps_io_usb1_inst_D4            : inout std_logic                     := 'X';             -- hps_io_usb1_inst_D4
      hps_io_hps_io_usb1_inst_D5            : inout std_logic                     := 'X';             -- hps_io_usb1_inst_D5
      hps_io_hps_io_usb1_inst_D6            : inout std_logic                     := 'X';             -- hps_io_usb1_inst_D6
      hps_io_hps_io_usb1_inst_D7            : inout std_logic                     := 'X';             -- hps_io_usb1_inst_D7
      hps_io_hps_io_usb1_inst_CLK           : in    std_logic                     := 'X';             -- hps_io_usb1_inst_CLK
      hps_io_hps_io_usb1_inst_STP           : out   std_logic;                                        -- hps_io_usb1_inst_STP
      hps_io_hps_io_usb1_inst_DIR           : in    std_logic                     := 'X';             -- hps_io_usb1_inst_DIR
      hps_io_hps_io_usb1_inst_NXT           : in    std_logic                     := 'X';             -- hps_io_usb1_inst_NXT
      hps_io_hps_io_uart0_inst_RX           : in    std_logic                     := 'X';             -- hps_io_uart0_inst_RX
      hps_io_hps_io_uart0_inst_TX           : out   std_logic;                                        -- hps_io_uart0_inst_TX
      hps_io_hps_io_uart1_inst_RX           : in    std_logic                     := 'X';             -- hps_io_uart1_inst_RX
      hps_io_hps_io_uart1_inst_TX           : out   std_logic;                                        -- hps_io_uart1_inst_TX
      hps_io_hps_io_i2c0_inst_SDA           : inout std_logic                     := 'X';             -- hps_io_i2c0_inst_SDA
      hps_io_hps_io_i2c0_inst_SCL           : inout std_logic                     := 'X';             -- hps_io_i2c0_inst_SCL
      hps_io_hps_io_trace_inst_CLK          : out   std_logic;                                        -- hps_io_trace_inst_CLK
      hps_io_hps_io_trace_inst_D0           : out   std_logic;                                        -- hps_io_trace_inst_D0
      hps_io_hps_io_trace_inst_D1           : out   std_logic;                                        -- hps_io_trace_inst_D1
      hps_io_hps_io_trace_inst_D2           : out   std_logic;                                        -- hps_io_trace_inst_D2
      hps_io_hps_io_trace_inst_D3           : out   std_logic;                                        -- hps_io_trace_inst_D3
      hps_io_hps_io_trace_inst_D4           : out   std_logic;                                        -- hps_io_trace_inst_D4
      hps_io_hps_io_trace_inst_D5           : out   std_logic;                                        -- hps_io_trace_inst_D5
      hps_io_hps_io_trace_inst_D6           : out   std_logic;                                        -- hps_io_trace_inst_D6
      hps_io_hps_io_trace_inst_D7           : out   std_logic;                                        -- hps_io_trace_inst_D7
      hps_io_hps_io_gpio_inst_GPIO00        : inout std_logic                     := 'X';             -- hps_io_gpio_inst_GPIO00
      hps_io_hps_io_gpio_inst_GPIO17        : inout std_logic                     := 'X';             -- hps_io_gpio_inst_GPIO17
      hps_io_hps_io_gpio_inst_GPIO18        : inout std_logic                     := 'X';             -- hps_io_gpio_inst_GPIO18
      hps_io_hps_io_gpio_inst_GPIO22        : inout std_logic                     := 'X';             -- hps_io_gpio_inst_GPIO22
      hps_io_hps_io_gpio_inst_GPIO24        : inout std_logic                     := 'X';             -- hps_io_gpio_inst_GPIO24
      hps_io_hps_io_gpio_inst_GPIO26        : inout std_logic                     := 'X';             -- hps_io_gpio_inst_GPIO26
      hps_io_hps_io_gpio_inst_GPIO27        : inout std_logic                     := 'X';             -- hps_io_gpio_inst_GPIO27
      hps_io_hps_io_gpio_inst_GPIO35        : inout std_logic                     := 'X';             -- hps_io_gpio_inst_GPIO35
      hps_io_hps_io_gpio_inst_GPIO40        : inout std_logic                     := 'X';             -- hps_io_gpio_inst_GPIO40
      hps_io_hps_io_gpio_inst_GPIO41        : inout std_logic                     := 'X';             -- hps_io_gpio_inst_GPIO41
      hps_io_hps_io_gpio_inst_GPIO42        : inout std_logic                     := 'X';             -- hps_io_gpio_inst_GPIO42
      hps_io_hps_io_gpio_inst_GPIO43        : inout std_logic                     := 'X';             -- hps_io_gpio_inst_GPIO43
      led_pio_external_connection_in_port   : in    std_logic_vector(3 downto 0)  := (others => 'X'); -- in_port
      led_pio_external_connection_out_port  : out   std_logic_vector(3 downto 0);                     -- out_port
      memory_mem_a                          : out   std_logic_vector(14 downto 0);                    -- mem_a
      memory_mem_ba                         : out   std_logic_vector(2 downto 0);                     -- mem_ba
      memory_mem_ck                         : out   std_logic;                                        -- mem_ck
      memory_mem_ck_n                       : out   std_logic;                                        -- mem_ck_n
      memory_mem_cke                        : out   std_logic;                                        -- mem_cke
      memory_mem_cs_n                       : out   std_logic;                                        -- mem_cs_n
      memory_mem_ras_n                      : out   std_logic;                                        -- mem_ras_n
      memory_mem_cas_n                      : out   std_logic;                                        -- mem_cas_n
      memory_mem_we_n                       : out   std_logic;                                        -- mem_we_n
      memory_mem_reset_n                    : out   std_logic;                                        -- mem_reset_n
      memory_mem_dq                         : inout std_logic_vector(39 downto 0) := (others => 'X'); -- mem_dq
      memory_mem_dqs                        : inout std_logic_vector(4 downto 0)  := (others => 'X'); -- mem_dqs
      memory_mem_dqs_n                      : inout std_logic_vector(4 downto 0)  := (others => 'X'); -- mem_dqs_n
      memory_mem_odt                        : out   std_logic;                                        -- mem_odt
      memory_mem_dm                         : out   std_logic_vector(4 downto 0);                     -- mem_dm
      memory_oct_rzqin                      : in    std_logic                     := 'X';             -- oct_rzqin
      reset_reset_n                         : in    std_logic                     := 'X'              -- reset_n
    );
  end component;
  
  component altera_wrapper_debounce is
    port (
      clk      : in  std_logic;
      reset_n  : in  std_logic;
      data_in  : in  std_logic_vector(3 downto 0);
      data_out : out std_logic_vector(3 downto 0)
    );
  end component;
  
  component hps_reset is
    port (
      probe      : in  std_logic                    := '0';
      source_clk : in  std_logic                    := '0';
      source     : out std_logic_vector(2 downto 0)
    );
  end component;
  
  component altera_wrapper_pcr is
    port (
      clk       : in  std_logic;
      rst_n     : in  std_logic;
      signal_in : in  std_logic;
      pulse_out : out std_logic
    );
  end component;
  
  component altera_wrapper_pwr is
    port (
      clk       : in  std_logic;
      rst_n     : in  std_logic;
      signal_in : in  std_logic;
      pulse_out : out std_logic
    );
  end component;
  
  component altera_wrapper_pdr is
    port (
      clk       : in  std_logic;
      rst_n     : in  std_logic;
      signal_in : in  std_logic;
      pulse_out : out std_logic
    );
  end component;
  
	component ghrd_5astfd5k3_sysid_qsys is
		port (
			clock    : in  std_logic;
			reset_n  : in  std_logic;
			readdata : out std_logic_vector(31 downto 0);
			address  : in  std_logic 
		);
	end component ghrd_5astfd5k3_sysid_qsys;

	component ghrd_5astfd5k3_f2sdram_only_master is
		generic (
			USE_PLI     : integer := 0;
			PLI_PORT    : integer := 50000;
			FIFO_DEPTHS : integer := 2
		);
		port (
			clk_clk              : in  std_logic                     := 'X';             -- clk
			clk_reset_reset      : in  std_logic                     := 'X';             -- reset
			master_address       : out std_logic_vector(31 downto 0);                    -- address
			master_readdata      : in  std_logic_vector(31 downto 0) := (others => 'X'); -- readdata
			master_read          : out std_logic;                                        -- read
			master_write         : out std_logic;                                        -- write
			master_writedata     : out std_logic_vector(31 downto 0);                    -- writedata
			master_waitrequest   : in  std_logic                     := 'X';             -- waitrequest
			master_readdatavalid : in  std_logic                     := 'X';             -- readdatavalid
			master_byteenable    : out std_logic_vector(3 downto 0);                     -- byteenable
			master_reset_reset   : out std_logic                                         -- reset
		);
	end component ghrd_5astfd5k3_f2sdram_only_master;

--	component ghrd_5astfd5k3_hps_0 is
--		generic (
--			F2S_Width : integer := 2;
--			S2F_Width : integer := 2
--		);
--		port (
--			f2h_cold_rst_req_n       : in    std_logic                      := 'X';             -- reset_n
--			f2h_dbg_rst_req_n        : in    std_logic                      := 'X';             -- reset_n
--			f2h_warm_rst_req_n       : in    std_logic                      := 'X';             -- reset_n
--			f2h_stm_hwevents         : in    std_logic_vector(27 downto 0)  := (others => 'X'); -- stm_hwevents
--			mem_a                    : out   std_logic_vector(14 downto 0);                     -- mem_a
--			mem_ba                   : out   std_logic_vector(2 downto 0);                      -- mem_ba
--			mem_ck                   : out   std_logic;                                         -- mem_ck
--			mem_ck_n                 : out   std_logic;                                         -- mem_ck_n
--			mem_cke                  : out   std_logic;                                         -- mem_cke
--			mem_cs_n                 : out   std_logic;                                         -- mem_cs_n
--			mem_ras_n                : out   std_logic;                                         -- mem_ras_n
--			mem_cas_n                : out   std_logic;                                         -- mem_cas_n
--			mem_we_n                 : out   std_logic;                                         -- mem_we_n
--			mem_reset_n              : out   std_logic;                                         -- mem_reset_n
--			mem_dq                   : inout std_logic_vector(39 downto 0)  := (others => 'X'); -- mem_dq
--			mem_dqs                  : inout std_logic_vector(4 downto 0)   := (others => 'X'); -- mem_dqs
--			mem_dqs_n                : inout std_logic_vector(4 downto 0)   := (others => 'X'); -- mem_dqs_n
--			mem_odt                  : out   std_logic;                                         -- mem_odt
--			mem_dm                   : out   std_logic_vector(4 downto 0);                      -- mem_dm
--			oct_rzqin                : in    std_logic                      := 'X';             -- oct_rzqin
--			hps_io_emac1_inst_TX_CLK : out   std_logic;                                         -- hps_io_emac1_inst_TX_CLK
--			hps_io_emac1_inst_TXD0   : out   std_logic;                                         -- hps_io_emac1_inst_TXD0
--			hps_io_emac1_inst_TXD1   : out   std_logic;                                         -- hps_io_emac1_inst_TXD1
--			hps_io_emac1_inst_TX_CTL : out   std_logic;                                         -- hps_io_emac1_inst_TX_CTL
--			hps_io_emac1_inst_RXD0   : in    std_logic                      := 'X';             -- hps_io_emac1_inst_RXD0
--			hps_io_emac1_inst_RXD1   : in    std_logic                      := 'X';             -- hps_io_emac1_inst_RXD1
--			hps_io_emac1_inst_TXD2   : out   std_logic;                                         -- hps_io_emac1_inst_TXD2
--			hps_io_emac1_inst_TXD3   : out   std_logic;                                         -- hps_io_emac1_inst_TXD3
--			hps_io_emac1_inst_MDIO   : inout std_logic                      := 'X';             -- hps_io_emac1_inst_MDIO
--			hps_io_emac1_inst_MDC    : out   std_logic;                                         -- hps_io_emac1_inst_MDC
--			hps_io_emac1_inst_RX_CTL : in    std_logic                      := 'X';             -- hps_io_emac1_inst_RX_CTL
--			hps_io_emac1_inst_RX_CLK : in    std_logic                      := 'X';             -- hps_io_emac1_inst_RX_CLK
--			hps_io_emac1_inst_RXD2   : in    std_logic                      := 'X';             -- hps_io_emac1_inst_RXD2
--			hps_io_emac1_inst_RXD3   : in    std_logic                      := 'X';             -- hps_io_emac1_inst_RXD3
--			hps_io_qspi_inst_IO0     : inout std_logic                      := 'X';             -- hps_io_qspi_inst_IO0
--			hps_io_qspi_inst_IO1     : inout std_logic                      := 'X';             -- hps_io_qspi_inst_IO1
--			hps_io_qspi_inst_IO2     : inout std_logic                      := 'X';             -- hps_io_qspi_inst_IO2
--			hps_io_qspi_inst_IO3     : inout std_logic                      := 'X';             -- hps_io_qspi_inst_IO3
--			hps_io_qspi_inst_SS0     : out   std_logic;                                         -- hps_io_qspi_inst_SS0
--			hps_io_qspi_inst_CLK     : out   std_logic;                                         -- hps_io_qspi_inst_CLK
--			hps_io_sdio_inst_CMD     : inout std_logic                      := 'X';             -- hps_io_sdio_inst_CMD
--			hps_io_sdio_inst_D0      : inout std_logic                      := 'X';             -- hps_io_sdio_inst_D0
--			hps_io_sdio_inst_D1      : inout std_logic                      := 'X';             -- hps_io_sdio_inst_D1
--			hps_io_sdio_inst_CLK     : out   std_logic;                                         -- hps_io_sdio_inst_CLK
--			hps_io_sdio_inst_D2      : inout std_logic                      := 'X';             -- hps_io_sdio_inst_D2
--			hps_io_sdio_inst_D3      : inout std_logic                      := 'X';             -- hps_io_sdio_inst_D3
--			hps_io_usb1_inst_D0      : inout std_logic                      := 'X';             -- hps_io_usb1_inst_D0
--			hps_io_usb1_inst_D1      : inout std_logic                      := 'X';             -- hps_io_usb1_inst_D1
--			hps_io_usb1_inst_D2      : inout std_logic                      := 'X';             -- hps_io_usb1_inst_D2
--			hps_io_usb1_inst_D3      : inout std_logic                      := 'X';             -- hps_io_usb1_inst_D3
--			hps_io_usb1_inst_D4      : inout std_logic                      := 'X';             -- hps_io_usb1_inst_D4
--			hps_io_usb1_inst_D5      : inout std_logic                      := 'X';             -- hps_io_usb1_inst_D5
--			hps_io_usb1_inst_D6      : inout std_logic                      := 'X';             -- hps_io_usb1_inst_D6
--			hps_io_usb1_inst_D7      : inout std_logic                      := 'X';             -- hps_io_usb1_inst_D7
--			hps_io_usb1_inst_CLK     : in    std_logic                      := 'X';             -- hps_io_usb1_inst_CLK
--			hps_io_usb1_inst_STP     : out   std_logic;                                         -- hps_io_usb1_inst_STP
--			hps_io_usb1_inst_DIR     : in    std_logic                      := 'X';             -- hps_io_usb1_inst_DIR
--			hps_io_usb1_inst_NXT     : in    std_logic                      := 'X';             -- hps_io_usb1_inst_NXT
--			hps_io_uart0_inst_RX     : in    std_logic                      := 'X';             -- hps_io_uart0_inst_RX
--			hps_io_uart0_inst_TX     : out   std_logic;                                         -- hps_io_uart0_inst_TX
--			hps_io_uart1_inst_RX     : in    std_logic                      := 'X';             -- hps_io_uart1_inst_RX
--			hps_io_uart1_inst_TX     : out   std_logic;                                         -- hps_io_uart1_inst_TX
--			hps_io_i2c0_inst_SDA     : inout std_logic                      := 'X';             -- hps_io_i2c0_inst_SDA
--			hps_io_i2c0_inst_SCL     : inout std_logic                      := 'X';             -- hps_io_i2c0_inst_SCL
--			hps_io_trace_inst_CLK    : out   std_logic;                                         -- hps_io_trace_inst_CLK
--			hps_io_trace_inst_D0     : out   std_logic;                                         -- hps_io_trace_inst_D0
--			hps_io_trace_inst_D1     : out   std_logic;                                         -- hps_io_trace_inst_D1
--			hps_io_trace_inst_D2     : out   std_logic;                                         -- hps_io_trace_inst_D2
--			hps_io_trace_inst_D3     : out   std_logic;                                         -- hps_io_trace_inst_D3
--			hps_io_trace_inst_D4     : out   std_logic;                                         -- hps_io_trace_inst_D4
--			hps_io_trace_inst_D5     : out   std_logic;                                         -- hps_io_trace_inst_D5
--			hps_io_trace_inst_D6     : out   std_logic;                                         -- hps_io_trace_inst_D6
--			hps_io_trace_inst_D7     : out   std_logic;                                         -- hps_io_trace_inst_D7
--			hps_io_gpio_inst_GPIO00  : inout std_logic                      := 'X';             -- hps_io_gpio_inst_GPIO00
--			hps_io_gpio_inst_GPIO17  : inout std_logic                      := 'X';             -- hps_io_gpio_inst_GPIO17
--			hps_io_gpio_inst_GPIO18  : inout std_logic                      := 'X';             -- hps_io_gpio_inst_GPIO18
--			hps_io_gpio_inst_GPIO22  : inout std_logic                      := 'X';             -- hps_io_gpio_inst_GPIO22
--			hps_io_gpio_inst_GPIO24  : inout std_logic                      := 'X';             -- hps_io_gpio_inst_GPIO24
--			hps_io_gpio_inst_GPIO26  : inout std_logic                      := 'X';             -- hps_io_gpio_inst_GPIO26
--			hps_io_gpio_inst_GPIO27  : inout std_logic                      := 'X';             -- hps_io_gpio_inst_GPIO27
--			hps_io_gpio_inst_GPIO35  : inout std_logic                      := 'X';             -- hps_io_gpio_inst_GPIO35
--			hps_io_gpio_inst_GPIO40  : inout std_logic                      := 'X';             -- hps_io_gpio_inst_GPIO40
--			hps_io_gpio_inst_GPIO41  : inout std_logic                      := 'X';             -- hps_io_gpio_inst_GPIO41
--			hps_io_gpio_inst_GPIO42  : inout std_logic                      := 'X';             -- hps_io_gpio_inst_GPIO42
--			hps_io_gpio_inst_GPIO43  : inout std_logic                      := 'X';             -- hps_io_gpio_inst_GPIO43
--			h2f_rst_n                : out   std_logic;                                         -- reset_n
--			f2h_sdram0_clk           : in    std_logic                      := 'X';             -- clk
--			f2h_sdram0_ADDRESS       : in    std_logic_vector(26 downto 0)  := (others => 'X'); -- address
--			f2h_sdram0_BURSTCOUNT    : in    std_logic_vector(7 downto 0)   := (others => 'X'); -- burstcount
--			f2h_sdram0_WAITREQUEST   : out   std_logic;                                         -- waitrequest
--			f2h_sdram0_READDATA      : out   std_logic_vector(255 downto 0);                    -- readdata
--			f2h_sdram0_READDATAVALID : out   std_logic;                                         -- readdatavalid
--			f2h_sdram0_READ          : in    std_logic                      := 'X';             -- read
--			f2h_sdram0_WRITEDATA     : in    std_logic_vector(255 downto 0) := (others => 'X'); -- writedata
--			f2h_sdram0_BYTEENABLE    : in    std_logic_vector(31 downto 0)  := (others => 'X'); -- byteenable
--			f2h_sdram0_WRITE         : in    std_logic                      := 'X';             -- write
--			h2f_axi_clk              : in    std_logic                      := 'X';             -- clk
--			h2f_AWID                 : out   std_logic_vector(11 downto 0);                     -- awid
--			h2f_AWADDR               : out   std_logic_vector(29 downto 0);                     -- awaddr
--			h2f_AWLEN                : out   std_logic_vector(3 downto 0);                      -- awlen
--			h2f_AWSIZE               : out   std_logic_vector(2 downto 0);                      -- awsize
--			h2f_AWBURST              : out   std_logic_vector(1 downto 0);                      -- awburst
--			h2f_AWLOCK               : out   std_logic_vector(1 downto 0);                      -- awlock
--			h2f_AWCACHE              : out   std_logic_vector(3 downto 0);                      -- awcache
--			h2f_AWPROT               : out   std_logic_vector(2 downto 0);                      -- awprot
--			h2f_AWVALID              : out   std_logic;                                         -- awvalid
--			h2f_AWREADY              : in    std_logic                      := 'X';             -- awready
--			h2f_WID                  : out   std_logic_vector(11 downto 0);                     -- wid
--			h2f_WDATA                : out   std_logic_vector(63 downto 0);                     -- wdata
--			h2f_WSTRB                : out   std_logic_vector(7 downto 0);                      -- wstrb
--			h2f_WLAST                : out   std_logic;                                         -- wlast
--			h2f_WVALID               : out   std_logic;                                         -- wvalid
--			h2f_WREADY               : in    std_logic                      := 'X';             -- wready
--			h2f_BID                  : in    std_logic_vector(11 downto 0)  := (others => 'X'); -- bid
--			h2f_BRESP                : in    std_logic_vector(1 downto 0)   := (others => 'X'); -- bresp
--			h2f_BVALID               : in    std_logic                      := 'X';             -- bvalid
--			h2f_BREADY               : out   std_logic;                                         -- bready
--			h2f_ARID                 : out   std_logic_vector(11 downto 0);                     -- arid
--			h2f_ARADDR               : out   std_logic_vector(29 downto 0);                     -- araddr
--			h2f_ARLEN                : out   std_logic_vector(3 downto 0);                      -- arlen
--			h2f_ARSIZE               : out   std_logic_vector(2 downto 0);                      -- arsize
--			h2f_ARBURST              : out   std_logic_vector(1 downto 0);                      -- arburst
--			h2f_ARLOCK               : out   std_logic_vector(1 downto 0);                      -- arlock
--			h2f_ARCACHE              : out   std_logic_vector(3 downto 0);                      -- arcache
--			h2f_ARPROT               : out   std_logic_vector(2 downto 0);                      -- arprot
--			h2f_ARVALID              : out   std_logic;                                         -- arvalid
--			h2f_ARREADY              : in    std_logic                      := 'X';             -- arready
--			h2f_RID                  : in    std_logic_vector(11 downto 0)  := (others => 'X'); -- rid
--			h2f_RDATA                : in    std_logic_vector(63 downto 0)  := (others => 'X'); -- rdata
--			h2f_RRESP                : in    std_logic_vector(1 downto 0)   := (others => 'X'); -- rresp
--			h2f_RLAST                : in    std_logic                      := 'X';             -- rlast
--			h2f_RVALID               : in    std_logic                      := 'X';             -- rvalid
--			h2f_RREADY               : out   std_logic;                                         -- rready
--			f2h_axi_clk              : in    std_logic                      := 'X';             -- clk
--			f2h_AWID                 : in    std_logic_vector(7 downto 0)   := (others => 'X'); -- awid
--			f2h_AWADDR               : in    std_logic_vector(31 downto 0)  := (others => 'X'); -- awaddr
--			f2h_AWLEN                : in    std_logic_vector(3 downto 0)   := (others => 'X'); -- awlen
--			f2h_AWSIZE               : in    std_logic_vector(2 downto 0)   := (others => 'X'); -- awsize
--			f2h_AWBURST              : in    std_logic_vector(1 downto 0)   := (others => 'X'); -- awburst
--			f2h_AWLOCK               : in    std_logic_vector(1 downto 0)   := (others => 'X'); -- awlock
--			f2h_AWCACHE              : in    std_logic_vector(3 downto 0)   := (others => 'X'); -- awcache
--			f2h_AWPROT               : in    std_logic_vector(2 downto 0)   := (others => 'X'); -- awprot
--			f2h_AWVALID              : in    std_logic                      := 'X';             -- awvalid
--			f2h_AWREADY              : out   std_logic;                                         -- awready
--			f2h_AWUSER               : in    std_logic_vector(4 downto 0)   := (others => 'X'); -- awuser
--			f2h_WID                  : in    std_logic_vector(7 downto 0)   := (others => 'X'); -- wid
--			f2h_WDATA                : in    std_logic_vector(63 downto 0)  := (others => 'X'); -- wdata
--			f2h_WSTRB                : in    std_logic_vector(7 downto 0)   := (others => 'X'); -- wstrb
--			f2h_WLAST                : in    std_logic                      := 'X';             -- wlast
--			f2h_WVALID               : in    std_logic                      := 'X';             -- wvalid
--			f2h_WREADY               : out   std_logic;                                         -- wready
--			f2h_BID                  : out   std_logic_vector(7 downto 0);                      -- bid
--			f2h_BRESP                : out   std_logic_vector(1 downto 0);                      -- bresp
--			f2h_BVALID               : out   std_logic;                                         -- bvalid
--			f2h_BREADY               : in    std_logic                      := 'X';             -- bready
--			f2h_ARID                 : in    std_logic_vector(7 downto 0)   := (others => 'X'); -- arid
--			f2h_ARADDR               : in    std_logic_vector(31 downto 0)  := (others => 'X'); -- araddr
--			f2h_ARLEN                : in    std_logic_vector(3 downto 0)   := (others => 'X'); -- arlen
--			f2h_ARSIZE               : in    std_logic_vector(2 downto 0)   := (others => 'X'); -- arsize
--			f2h_ARBURST              : in    std_logic_vector(1 downto 0)   := (others => 'X'); -- arburst
--			f2h_ARLOCK               : in    std_logic_vector(1 downto 0)   := (others => 'X'); -- arlock
--			f2h_ARCACHE              : in    std_logic_vector(3 downto 0)   := (others => 'X'); -- arcache
--			f2h_ARPROT               : in    std_logic_vector(2 downto 0)   := (others => 'X'); -- arprot
--			f2h_ARVALID              : in    std_logic                      := 'X';             -- arvalid
--			f2h_ARREADY              : out   std_logic;                                         -- arready
--			f2h_ARUSER               : in    std_logic_vector(4 downto 0)   := (others => 'X'); -- aruser
--			f2h_RID                  : out   std_logic_vector(7 downto 0);                      -- rid
--			f2h_RDATA                : out   std_logic_vector(63 downto 0);                     -- rdata
--			f2h_RRESP                : out   std_logic_vector(1 downto 0);                      -- rresp
--			f2h_RLAST                : out   std_logic;                                         -- rlast
--			f2h_RVALID               : out   std_logic;                                         -- rvalid
--			f2h_RREADY               : in    std_logic                      := 'X';             -- rready
--			h2f_lw_axi_clk           : in    std_logic                      := 'X';             -- clk
--			h2f_lw_AWID              : out   std_logic_vector(11 downto 0);                     -- awid
--			h2f_lw_AWADDR            : out   std_logic_vector(20 downto 0);                     -- awaddr
--			h2f_lw_AWLEN             : out   std_logic_vector(3 downto 0);                      -- awlen
--			h2f_lw_AWSIZE            : out   std_logic_vector(2 downto 0);                      -- awsize
--			h2f_lw_AWBURST           : out   std_logic_vector(1 downto 0);                      -- awburst
--			h2f_lw_AWLOCK            : out   std_logic_vector(1 downto 0);                      -- awlock
--			h2f_lw_AWCACHE           : out   std_logic_vector(3 downto 0);                      -- awcache
--			h2f_lw_AWPROT            : out   std_logic_vector(2 downto 0);                      -- awprot
--			h2f_lw_AWVALID           : out   std_logic;                                         -- awvalid
--			h2f_lw_AWREADY           : in    std_logic                      := 'X';             -- awready
--			h2f_lw_WID               : out   std_logic_vector(11 downto 0);                     -- wid
--			h2f_lw_WDATA             : out   std_logic_vector(31 downto 0);                     -- wdata
--			h2f_lw_WSTRB             : out   std_logic_vector(3 downto 0);                      -- wstrb
--			h2f_lw_WLAST             : out   std_logic;                                         -- wlast
--			h2f_lw_WVALID            : out   std_logic;                                         -- wvalid
--			h2f_lw_WREADY            : in    std_logic                      := 'X';             -- wready
--			h2f_lw_BID               : in    std_logic_vector(11 downto 0)  := (others => 'X'); -- bid
--			h2f_lw_BRESP             : in    std_logic_vector(1 downto 0)   := (others => 'X'); -- bresp
--			h2f_lw_BVALID            : in    std_logic                      := 'X';             -- bvalid
--			h2f_lw_BREADY            : out   std_logic;                                         -- bready
--			h2f_lw_ARID              : out   std_logic_vector(11 downto 0);                     -- arid
--			h2f_lw_ARADDR            : out   std_logic_vector(20 downto 0);                     -- araddr
--			h2f_lw_ARLEN             : out   std_logic_vector(3 downto 0);                      -- arlen
--			h2f_lw_ARSIZE            : out   std_logic_vector(2 downto 0);                      -- arsize
--			h2f_lw_ARBURST           : out   std_logic_vector(1 downto 0);                      -- arburst
--			h2f_lw_ARLOCK            : out   std_logic_vector(1 downto 0);                      -- arlock
--			h2f_lw_ARCACHE           : out   std_logic_vector(3 downto 0);                      -- arcache
--			h2f_lw_ARPROT            : out   std_logic_vector(2 downto 0);                      -- arprot
--			h2f_lw_ARVALID           : out   std_logic;                                         -- arvalid
--			h2f_lw_ARREADY           : in    std_logic                      := 'X';             -- arready
--			h2f_lw_RID               : in    std_logic_vector(11 downto 0)  := (others => 'X'); -- rid
--			h2f_lw_RDATA             : in    std_logic_vector(31 downto 0)  := (others => 'X'); -- rdata
--			h2f_lw_RRESP             : in    std_logic_vector(1 downto 0)   := (others => 'X'); -- rresp
--			h2f_lw_RLAST             : in    std_logic                      := 'X';             -- rlast
--			h2f_lw_RVALID            : in    std_logic                      := 'X';             -- rvalid
--			h2f_lw_RREADY            : out   std_logic;                                         -- rready
--			f2h_irq_p0               : in    std_logic_vector(31 downto 0)  := (others => 'X'); -- irq
--			f2h_irq_p1               : in    std_logic_vector(31 downto 0)  := (others => 'X')  -- irq
--		);
--	end component ghrd_5astfd5k3_hps_0;

	component ghrd_5astfd5k3_rst_controller is
		generic (
			NUM_RESET_INPUTS          : integer := 6;
			OUTPUT_RESET_SYNC_EDGES   : string  := "deassert";
			SYNC_DEPTH                : integer := 2;
			RESET_REQUEST_PRESENT     : integer := 0;
			RESET_REQ_WAIT_TIME       : integer := 1;
			MIN_RST_ASSERTION_TIME    : integer := 3;
			RESET_REQ_EARLY_DSRT_TIME : integer := 1;
			USE_RESET_REQUEST_IN0     : integer := 0;
			USE_RESET_REQUEST_IN1     : integer := 0;
			USE_RESET_REQUEST_IN2     : integer := 0;
			USE_RESET_REQUEST_IN3     : integer := 0;
			USE_RESET_REQUEST_IN4     : integer := 0;
			USE_RESET_REQUEST_IN5     : integer := 0;
			USE_RESET_REQUEST_IN6     : integer := 0;
			USE_RESET_REQUEST_IN7     : integer := 0;
			USE_RESET_REQUEST_IN8     : integer := 0;
			USE_RESET_REQUEST_IN9     : integer := 0;
			USE_RESET_REQUEST_IN10    : integer := 0;
			USE_RESET_REQUEST_IN11    : integer := 0;
			USE_RESET_REQUEST_IN12    : integer := 0;
			USE_RESET_REQUEST_IN13    : integer := 0;
			USE_RESET_REQUEST_IN14    : integer := 0;
			USE_RESET_REQUEST_IN15    : integer := 0;
			ADAPT_RESET_REQUEST       : integer := 0
		);
		port (
			reset_in0      : in  std_logic := 'X'; -- reset
			clk            : in  std_logic := 'X'; -- clk
			reset_out      : out std_logic;        -- reset
			reset_req      : out std_logic;        -- reset_req
			reset_req_in0  : in  std_logic := 'X'; -- reset_req
			reset_in1      : in  std_logic := 'X'; -- reset
			reset_req_in1  : in  std_logic := 'X'; -- reset_req
			reset_in2      : in  std_logic := 'X'; -- reset
			reset_req_in2  : in  std_logic := 'X'; -- reset_req
			reset_in3      : in  std_logic := 'X'; -- reset
			reset_req_in3  : in  std_logic := 'X'; -- reset_req
			reset_in4      : in  std_logic := 'X'; -- reset
			reset_req_in4  : in  std_logic := 'X'; -- reset_req
			reset_in5      : in  std_logic := 'X'; -- reset
			reset_req_in5  : in  std_logic := 'X'; -- reset_req
			reset_in6      : in  std_logic := 'X'; -- reset
			reset_req_in6  : in  std_logic := 'X'; -- reset_req
			reset_in7      : in  std_logic := 'X'; -- reset
			reset_req_in7  : in  std_logic := 'X'; -- reset_req
			reset_in8      : in  std_logic := 'X'; -- reset
			reset_req_in8  : in  std_logic := 'X'; -- reset_req
			reset_in9      : in  std_logic := 'X'; -- reset
			reset_req_in9  : in  std_logic := 'X'; -- reset_req
			reset_in10     : in  std_logic := 'X'; -- reset
			reset_req_in10 : in  std_logic := 'X'; -- reset_req
			reset_in11     : in  std_logic := 'X'; -- reset
			reset_req_in11 : in  std_logic := 'X'; -- reset_req
			reset_in12     : in  std_logic := 'X'; -- reset
			reset_req_in12 : in  std_logic := 'X'; -- reset_req
			reset_in13     : in  std_logic := 'X'; -- reset
			reset_req_in13 : in  std_logic := 'X'; -- reset_req
			reset_in14     : in  std_logic := 'X'; -- reset
			reset_req_in14 : in  std_logic := 'X'; -- reset_req
			reset_in15     : in  std_logic := 'X'; -- reset
			reset_req_in15 : in  std_logic := 'X'  -- reset_req
		);
	end component ghrd_5astfd5k3_rst_controller;
  
  component stub_pll is
    port (
      refclk   : in  std_logic := 'X'; -- clk
      rst      : in  std_logic := 'X'; -- reset
      outclk_0 : out std_logic;        -- clk
      outclk_1 : out std_logic;        -- clk
      outclk_2 : out std_logic;        -- clk
      outclk_3 : out std_logic;        -- clk
      locked   : out std_logic         -- export
    );
  end component stub_pll;
  
end ghrd_5astfd5k3_pkg;
