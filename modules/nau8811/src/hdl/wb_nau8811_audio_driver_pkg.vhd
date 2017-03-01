-- libraries and packages
-- ieee
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- wishbone/gsi/cern
library work;
use work.wishbone_pkg.all;
use work.genram_pkg.all;

-- package declaration
package wb_nau8811_audio_driver_pkg is
  component wb_nau8811_audio_driver
    generic (
      g_address_size     : natural := 32;   -- in bit(s)
      g_data_size        : natural := 32;   -- in bit(s)
      g_spi_fifo_size    : natural := 32;   -- in g_spi_data_size(s)
      g_spi_data_size    : natural := 16;   -- in bit(s)
      g_iis_tx_fifo_size : natural := 1024; -- in g_spi_data_size(s)
      g_iis_rx_fifo_size : natural := 1024; -- in g_spi_data_size(s)
      g_iis_word_size    : natural := 32;   -- in bit(s)
      g_use_external_pll : boolean := true  -- true for real synthesis or false for simulation purpose
    );
    port (
      -- generic system interface
      clk_sys_i    : in  std_logic;
      rst_n_i      : in  std_logic;
      -- pll reference
      pll_ref_i    : in  std_logic;
      -- heartbeat trigger
      trigger_i    : in  std_logic;
      -- wishbone slave interface
      slave_i      : in  t_wishbone_slave_in;
      slave_o      : out t_wishbone_slave_out;
      -- nau8811 serial interface
      spi_csb_o    : out std_logic; -- Chip/Slave select
      spi_sclk_o   : out std_logic; -- Serial clock
      spi_sdio_o   : out std_logic; -- Serial data in/out
      -- nau8811 inter-ic sound interface
      iis_fs_o     : out std_logic; -- Frame sync
      iis_bclk_o   : out std_logic; -- Clock
      iis_adcout_o : out std_logic; -- Data out
      iis_dacin_i  : in  std_logic  -- Data in
  );    
  end component;

  -- external pll
  component audio_pll is
    port (
      refclk   : in  std_logic := 'X'; -- clk
      rst      : in  std_logic := 'X'; -- reset
      outclk_0 : out std_logic;        -- clk
      locked   : out std_logic         -- export
    );
  end component audio_pll;

  constant c_nau8811_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"4", -- 32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"00000000000000ff",
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"7AE8811D",
    version       => x"00000001",
    date          => x"20140922",
    name          => "NAU8811_AUD_DRIVER ")));

end wb_nau8811_audio_driver_pkg;
