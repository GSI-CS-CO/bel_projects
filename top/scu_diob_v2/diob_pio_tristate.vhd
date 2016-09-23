library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.scu_diob_pkg.all;

entity diob_pio_tristate is
  generic (
    pio_size:     natural;
    module_count: natural;
    module_list:  t_module_array
  );
  port (
    module_id:          std_logic_vector(7 downto 0);
    pio:                inout std_logic_vector(pio_max downto pio_min);
    bit_vector_config:  in    t_pio_bit_vectors_array;
    module_io_i:        out   std_logic_vector(pio_max downto pio_min);
    irq_o:              out   std_logic_vector(15 downto 1));
end entity diob_pio_tristate;

architecture arch of diob_pio_tristate is
  signal sel_bit_vector_in:     std_logic_vector(pio_max downto pio_min);
  signal sel_bit_vector_out:    std_logic_vector(pio_max downto pio_min);
  signal sel_bit_vector_en:     std_logic_vector(pio_max downto pio_min);
  

begin
  vector_mux:
  for i in 0 to module_count-1 generate
  --for i in 5 to 5 generate
    sel_bit_vector_in  <= bit_vector_config(i).vect_in  when module_list(i).id = module_id else (others => 'Z');
    sel_bit_vector_out <= bit_vector_config(i).vect_out when module_list(i).id = module_id else (others => 'Z');
    sel_bit_vector_en  <= bit_vector_config(i).vect_en  when module_list(i).id = module_id else (others => 'Z');
    irq_o              <= bit_vector_config(i).irq      when module_list(i).id = module_id else (others => 'Z');
  end generate vector_mux; 

  module_io_i <= pio;
  
  pio_out:
  for i in pio_min to pio_max generate
    pio(i) <= sel_bit_vector_en(i) when sel_bit_vector_out(i) = '1' else 'Z';
  end generate pio_out; 


end architecture;
