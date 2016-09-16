library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.scu_diob_pkg.all;

entity diob_pio_tristate is
  generic (
    pio_size:   natural
  );
  port (
    pio:            inout std_logic_vector(pio_size-1 downto 0);
    bit_vector_in:  in    std_logic_vector(pio_size-1 downto 0);
    bit_vector_out: in    std_logic_vector(pio_size-1 downto 0);
    bit_vector_en:  in    std_logic_vector(pio_size-1 downto 0);
    module_io_i:    out   std_logic_vector(pio_size-1 downto 0));
end entity diob_pio_tristate;

architecture arch of diob_pio_tristate is
begin
  module_io_i <= pio and bit_vector_in;
  
  pio_out:
  for i in 0 to pio_size-1 generate
    pio(i) <= bit_vector_en(i) when bit_vector_out(i) = '1' else 'Z';
  end generate pio_out; 
 

end architecture;
