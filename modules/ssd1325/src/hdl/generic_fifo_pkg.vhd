-- libraries and packages
-- ieee
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- package declaration
package generic_fifo_pkg is
  component generic_fifo
    Generic (
      g_data_width : natural := 8;  -- bit(s)
      g_fifo_depth : natural := 256 -- g_data_width*bit(s)
    );
    Port ( 
      clk_sys_i    : in  std_logic;
      rst_n_i      : in  std_logic;
      write_en_i   : in  std_logic;
      data_i       : in  std_logic_vector(g_data_width-1 downto 0);
      read_en_i    : in  std_logic;
      data_o       : out std_logic_vector(g_data_width-1 downto 0);
      flag_empty_o : out std_logic;
      flag_full_o  : out std_logic;
      fill_level_o : out std_logic_vector(g_data_width-1 downto 0)
    );
  end component;    
end generic_fifo_pkg;
