library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.gencores_pkg.all;
use work.scu_bus_slave_pkg.all;
use work.scu_diob_pkg.all;

entity diob_module is
    generic ( 
      size:       natural;
      module_id:  std_logic_vector(7 downto 0)
      );
    port (
      scu_slave_i:  in  t_scu_local_slave_i;
      scu_slave_o:  out t_scu_local_slave_o;
      tag_i:        in  std_logic_vector(31 downto 0);
      tag_valid:    in  std_logic;
      en_i:         in  std_logic;
      vect_i:       out std_logic_vector(size-1 downto 0);
      vect_o:       out std_logic_vector(size-1 downto 0);
      vect_en:      out std_logic_vector(size-1 downto 0));
  end entity diob_module;
