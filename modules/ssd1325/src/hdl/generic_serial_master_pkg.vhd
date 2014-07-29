-- libraries and packages
-- ieee
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- wishbone/gsi/cern
library work;
use work.wishbone_pkg.all;

-- package declaration
package generic_serial_master_pkg is
  component generic_serial_master
    Generic ( g_system_clock : natural := 62500000; -- Hz
              g_serial_clock : natural := 2000000;  -- Hz
              g_data_width   : natural := 8         -- bit(s)
            );           
    Port ( clk_sys_i  : in  std_logic;
           rst_n_i    : in  std_logic;
           tx_data_i  : in  std_logic_vector(g_data_width-1 downto 0);
           mosi_o     : out std_logic;                           
           sclk_o     : out std_logic;
           ss_o       : out std_logic;
           tx_start_i : in  std_logic;
           tx_done_o  : out std_logic;
           rx_read_o  : out std_logic
         );
  end component;    
end generic_serial_master_pkg;
