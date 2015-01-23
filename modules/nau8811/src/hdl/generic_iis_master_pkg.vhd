-- libraries and packages
-- ieee
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- wishbone/gsi/cern
library work;
use work.wishbone_pkg.all;

-- package declaration
package generic_iis_master_pkg is
  component generic_iis_master
    Generic ( g_word_width   : natural := 32 -- bit(s)
            );        
    Port ( clk_sys_i         : in  std_logic;
           rst_n_i           : in  std_logic;
           tx_data_i         : in  std_logic_vector ((g_word_width-1) downto 0);
           rx_data_o         : out std_logic_vector ((g_word_width-1) downto 0);
           iis_fs_o          : out std_logic;
           iis_bclk_o        : out std_logic;
           iis_adcout_o      : out std_logic;
           iis_dacin_i       : in  std_logic;
           iis_enable_clk_i  : in  std_logic;
           iis_transaction_i : in  std_logic;
           iis_mono_output_i : in  std_logic;
           iis_invert_fs_i   : in  std_logic;
           iis_padding_i     : in  std_logic;
           iis_heartbeat_i   : in  std_logic;
           iis_trigger_pol_i : in  std_logic;
           iis_trigger_i     : in  std_logic;
           tx_fifo_empty_i   : in  std_logic;
           rx_fifo_full_i    : in  std_logic;
           tx_read_o         : out std_logic;
           rx_write_o        : out std_logic
         );
  end component;    
end generic_iis_master_pkg;
