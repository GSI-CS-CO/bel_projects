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
    irq_o:              out   std_logic_vector(15 downto 1);
    mod_en:             out   std_logic_vector(module_count-1 downto 0));
end entity diob_pio_tristate;

architecture arch of diob_pio_tristate is
  signal sel_bit_vector_in:     std_logic_vector(pio_max downto pio_min);
  signal sel_bit_vector_out:    std_logic_vector(pio_max downto pio_min);
  signal sel_bit_vector_en:     std_logic_vector(pio_max downto pio_min);
  

begin
  vector_mux: process (module_id, bit_vector_config)
    variable sel_bit_vector_in_tmp:   std_logic_vector(pio_max downto pio_min);
    variable sel_bit_vector_out_tmp:  std_logic_vector(pio_max downto pio_min);
    variable sel_bit_vector_en_tmp:   std_logic_vector(pio_max downto pio_min);
    variable irq_tmp:                 std_logic_vector(15 downto 1);
  begin
    sel_bit_vector_in_tmp   := (others => '0');
    sel_bit_vector_out_tmp  := (others => '0');
    sel_bit_vector_en_tmp   := (others => '0');
    irq_tmp                 := (others => '0');
    for i in 0 to module_count-1 loop
      if module_list(i).id = module_id then
        sel_bit_vector_in_tmp   := sel_bit_vector_in_tmp or bit_vector_config(i).vect_in;
        sel_bit_vector_out_tmp  := sel_bit_vector_out_tmp or bit_vector_config(i).vect_out;
        sel_bit_vector_en_tmp   := sel_bit_vector_en_tmp or bit_vector_config(i).vect_en;
        irq_tmp                 := irq_tmp or bit_vector_config(i).irq;
      end if;
    end loop;
    irq_o               <= irq_tmp;
    sel_bit_vector_in   <= sel_bit_vector_in_tmp;
    sel_bit_vector_out  <= sel_bit_vector_out_tmp;
    sel_bit_vector_en   <= sel_bit_vector_en_tmp;
    
  end process;
  
  module_io_i <= pio;
  
  pio_out:
  for i in pio_min to pio_max generate
    pio(i) <= sel_bit_vector_en(i) when sel_bit_vector_out(i) = '1' else 'Z';
  end generate pio_out; 
  
  en:
  for i in 0 to module_count-1 generate
    mod_en(i) <= '1' when module_list(i).id = module_id else '0';
  end generate en;


end architecture;
