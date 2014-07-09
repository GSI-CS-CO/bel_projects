-- libraries and packages
-- ieee
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- wishbone/gsi/cern
library work;
use work.wishbone_pkg.all;

-- package declaration
package wb_ssd1325_serial_driver_pkg is
  component wb_ssd1325_serial_driver
    generic (
      g_address_size  : natural := 32;  -- in bit(s)
      g_data_size     : natural := 32;  -- in bit(s)
      g_spi_fifo_size : natural := 256; -- in g_spi_data_size(s)
      g_spi_data_size : natural := 8    -- in bit(s)
    );
    port (
      -- generic system interface
      clk_sys_i  : in  std_logic;
      rst_n_i    : in  std_logic;
      -- wishbone slave interface
      slave_i    : in  t_wishbone_slave_in;
      slave_o    : out t_wishbone_slave_out;
      -- ssd1325 interface
      ssd_rst_o  : out std_logic;
      ssd_dc_o   : out std_logic;
      ssd_ss_o   : out std_logic;
      ssd_sclk_o : out std_logic;
      ssd_data_o : out std_logic;
      -- optional system interface
      ssd_irq_o  : out std_logic
  );    
  end component;    

  constant c_ssd1325_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"4", -- 32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"000000000000000f",
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"55D1325D",
    version       => x"00000001",
    date          => x"20140627",
    name          => "SSD1325_SER_DRIVER ")));

end wb_ssd1325_serial_driver_pkg;	 
