library ieee;
use ieee.std_logic_1164.all;

library work;

package dummy_phy_pkg is

	component dummy_phy is
	port (
		pll_powerdown               : in  std_logic_vector(2 downto 0)   := (others => '0'); --               pll_powerdown.pll_powerdown
		tx_analogreset              : in  std_logic_vector(2 downto 0)   := (others => '0'); --              tx_analogreset.tx_analogreset
		tx_digitalreset             : in  std_logic_vector(2 downto 0)   := (others => '0'); --             tx_digitalreset.tx_digitalreset
		tx_pll_refclk               : in  std_logic_vector(0 downto 0)   := (others => '0'); --               tx_pll_refclk.tx_pll_refclk
		tx_std_clkout               : out std_logic_vector(2 downto 0);                      --               tx_pma_clkout.tx_pma_clkout
		tx_serial_data              : out std_logic_vector(2 downto 0);                      --              tx_serial_data.tx_serial_data
		pll_locked                  : out std_logic_vector(2 downto 0);                      --                  pll_locked.pll_locked
		rx_analogreset              : in  std_logic_vector(2 downto 0)   := (others => '0'); --              rx_analogreset.rx_analogreset
		rx_digitalreset             : in  std_logic_vector(2 downto 0)   := (others => '0'); --             rx_digitalreset.rx_digitalreset
		rx_cdr_refclk               : in  std_logic_vector(0 downto 0)   := (others => '0'); --               rx_cdr_refclk.rx_cdr_refclk
		rx_serial_data              : in  std_logic_vector(2 downto 0)   := (others => '0'); --              rx_serial_data.rx_serial_data
		rx_seriallpbken             : in  std_logic_vector(2 downto 0)   := (others => '0'); --             rx_seriallpbken.rx_seriallpbken
		tx_cal_busy                 : out std_logic_vector(2 downto 0);                      --                 tx_cal_busy.tx_cal_busy
		rx_cal_busy                 : out std_logic_vector(2 downto 0);                      --                 rx_cal_busy.rx_cal_busy
		reconfig_to_xcvr            : in  std_logic_vector(419 downto 0) := (others => '0'); --            reconfig_to_xcvr.reconfig_to_xcvr
		reconfig_from_xcvr          : out std_logic_vector(275 downto 0);                    --          reconfig_from_xcvr.reconfig_from_xcvr
		tx_parallel_data            : in  std_logic_vector(29 downto 0)  := (others => '0'); --        tx_pma_parallel_data.tx_pma_parallel_data
		unused_tx_parallel_data : in  std_logic_vector(101 downto 0) := (others => '0'); -- unused_tx_pma_parallel_data.unused_tx_pma_parallel_data
		rx_parallel_data            : out std_logic_vector(29 downto 0);                     --        rx_pma_parallel_data.rx_pma_parallel_data
		unused_rx_parallel_data : out std_logic_vector(161 downto 0)                     -- unused_rx_pma_parallel_data.unused_rx_pma_parallel_data
	);
end component dummy_phy;
  
  component dummy_phy_reset is
	port (
		clock              : in  std_logic                    := '0';             --              clock.clk
		reset              : in  std_logic                    := '0';             --              reset.reset
		pll_powerdown      : out std_logic_vector(0 downto 0);                    --      pll_powerdown.pll_powerdown
		tx_analogreset     : out std_logic_vector(2 downto 0);                    --     tx_analogreset.tx_analogreset
		tx_digitalreset    : out std_logic_vector(2 downto 0);                    --    tx_digitalreset.tx_digitalreset
		tx_ready           : out std_logic_vector(2 downto 0);                    --           tx_ready.tx_ready
		pll_locked         : in  std_logic_vector(0 downto 0) := (others => '0'); --         pll_locked.pll_locked
		pll_select         : in  std_logic_vector(0 downto 0) := (others => '0'); --         pll_select.pll_select
		tx_cal_busy        : in  std_logic_vector(2 downto 0) := (others => '0'); --        tx_cal_busy.tx_cal_busy
		rx_analogreset     : out std_logic_vector(2 downto 0);                    --     rx_analogreset.rx_analogreset
		rx_digitalreset    : out std_logic_vector(2 downto 0);                    --    rx_digitalreset.rx_digitalreset
		rx_ready           : out std_logic_vector(2 downto 0);                    --           rx_ready.rx_ready
		rx_is_lockedtodata : in  std_logic_vector(2 downto 0) := (others => '0'); -- rx_is_lockedtodata.rx_is_lockedtodata
		rx_cal_busy        : in  std_logic_vector(2 downto 0) := (others => '0')  --        rx_cal_busy.rx_cal_busy
	);
end component dummy_phy_reset;

component dummy_phy_reconf is
	port (
		reconfig_busy             : out std_logic;                                         --      reconfig_busy.reconfig_busy
		mgmt_clk_clk              : in  std_logic                      := '0';             --       mgmt_clk_clk.clk
		mgmt_rst_reset            : in  std_logic                      := '0';             --     mgmt_rst_reset.reset
		reconfig_mgmt_address     : in  std_logic_vector(6 downto 0)   := (others => '0'); --      reconfig_mgmt.address
		reconfig_mgmt_read        : in  std_logic                      := '0';             --                   .read
		reconfig_mgmt_readdata    : out std_logic_vector(31 downto 0);                     --                   .readdata
		reconfig_mgmt_waitrequest : out std_logic;                                         --                   .waitrequest
		reconfig_mgmt_write       : in  std_logic                      := '0';             --                   .write
		reconfig_mgmt_writedata   : in  std_logic_vector(31 downto 0)  := (others => '0'); --                   .writedata
		reconfig_to_xcvr          : out std_logic_vector(419 downto 0);                    --   reconfig_to_xcvr.reconfig_to_xcvr
		reconfig_from_xcvr        : in  std_logic_vector(275 downto 0) := (others => '0')  -- reconfig_from_xcvr.reconfig_from_xcvr
	);
end component dummy_phy_reconf;
  
end package;

