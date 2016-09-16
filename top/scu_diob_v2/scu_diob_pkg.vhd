library ieee;  
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.gencores_pkg.all;
use work.scu_bus_slave_pkg.all;

package scu_diob_pkg is

  TYPE    t_IO_Reg_1_to_7_Array     is array (1 to 7)  of std_logic_vector(15 downto 0);
  TYPE    t_IO_Reg_0_to_7_Array     is array (0 to 7)  of std_logic_vector(15 downto 0);
  
  component diob_pio_tristate is
    generic (
      pio_size:   natural
    );
    port (
      pio:            inout std_logic_vector(pio_size-1 downto 0);
      bit_vector_in:  in    std_logic_vector(pio_size-1 downto 0);
      bit_vector_out: in    std_logic_vector(pio_size-1 downto 0);
      bit_vector_en:  in    std_logic_vector(pio_size-1 downto 0);
      module_io_i:    out   std_logic_vector(pio_size-1 downto 0)); 
  end component diob_pio_tristate;
    
end scu_diob_pkg;
