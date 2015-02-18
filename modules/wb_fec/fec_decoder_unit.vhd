library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wr_fabric_pkg.all;

entity fec_decoder_unit is
   port(
      clk_i          : in  std_logic;
      rst_n_i        : in  std_logic;
      dec_src_i      : in  t_wrf_source_in;
      dec_src_o      : out t_wrf_source_out;
      wb_dec_slave_i : in  t_wishbone_slave_in;
      wb_dec_slave_o : out t_wishbone_slave_out
      );
end fec_decoder_unit;

architecture rtl of fec_decoder_unit is

begin
  

  wb_dec_slave_o <= cc_dummy_slave_out;



end rtl;




