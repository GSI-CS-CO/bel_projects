library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library work;
use work.wishbone_pkg.all;
use work.wr_fabric_pkg.all;
use work.fec_pkg.all;

entity fec_encoder is
  port(
    clk_i       : in  std_logic;
    rst_n_i     : in  std_logic;
    ctrl_reg_i  : in  t_fec_ctrl_reg;
    stat_reg_o  : out t_fec_enc_stat_reg;
    enc_src_o   : out t_wrf_source_out;
    enc_src_i   : in  t_wrf_source_in;
    enc_snk_i   : in  t_wrf_sink_in;
    enc_snk_o   : out t_wrf_sink_out);
end fec_encoder;

architecture rtl of fec_encoder is

begin

  enc_src_o   <= enc_snk_i;
  enc_snk_o   <= enc_src_i;

  stat_reg_o <= c_stat_enc_reg_default;
end rtl;
