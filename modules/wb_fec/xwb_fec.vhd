library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wr_fabric_pkg.all;
use work.fec_pkg.all;

entity xwb_fec is
  generic(
    g_fec_encoder     : boolean := true;
    g_fec_decoder     : boolean := true;
    g_packet_gen      : boolean := false;
    g_dpram_size      : integer := 90112/4;
    g_init_file       : string  := "";
    g_upper_bridge_sdb: t_sdb_bridge);

  port(
    clk_i                 : in std_logic;
    rst_n_i               : in std_logic;
    rst_lm32_n_i          : in std_logic;

    wr_snk_i              : in  t_wrf_sink_in;
    wr_snk_o              : out t_wrf_sink_out;
    wr_src_o              : out t_wrf_source_out;
    wr_src_i              : in  t_wrf_source_in;

    eb_snk_i              : in  t_wrf_sink_in;
    eb_snk_o              : out t_wrf_sink_out;
    eb_src_o              : out t_wrf_source_out;
    eb_src_i              : in  t_wrf_source_in;

    wb_ctrl_stat_slave_o  : out t_wishbone_slave_out;
    wb_ctrl_stat_slave_i  : in  t_wishbone_slave_in;

    wb_cross_master_o     : out t_wishbone_master_out;
    wb_cross_master_i     : in  t_wishbone_master_in;
   
    time_code_i		        : in  t_time_code);
end xwb_fec;


architecture rtl of xwb_fec is

  signal s_enc_snk_i      : t_wrf_sink_in;
  signal s_enc_snk_o      : t_wrf_sink_out;

  signal s_pg_snk_i       : t_wrf_sink_in;
  signal s_pg_snk_o       : t_wrf_sink_out;

  signal s_fec_ctrl       : t_fec_ctrl_reg;

  signal s_fec_stat       : t_fec_stat_reg;
  signal s_fec_dec_stat   : t_fec_dec_stat_reg;
  signal s_fec_enc_stat   : t_fec_enc_stat_reg;

  signal s_pg_ctrl        : t_pg_ctrl_reg;
  signal s_pg_stat        : t_pg_stat_reg;

  signal wb_cross_bar_master_i: t_wishbone_master_in;
  signal wb_cross_bar_master_o: t_wishbone_master_out;

begin

  packet_gen_y   : if (g_packet_gen and g_fec_encoder) generate
  crossing    : cross_fabric
    port  map (
      clk_i          => clk_i,
      rst_n_i        => rst_n_i,
      mux_src_i      => s_enc_snk_o,     -- encoder    / mux
      mux_src_o      => s_enc_snk_i,
      mux_snk_i(0)   => s_pg_snk_i,      -- packet gen / mux
      mux_snk_o(0)   => s_pg_snk_o,
      mux_snk_i(1)   => eb_snk_i,        -- etherbone  / mux
      mux_snk_o(1)   => eb_snk_o);

  pg : packet_gen
    port map (
      clk_i          => clk_i,
      rst_n_i        => rst_n_i,
      ctrl_reg_i     => s_pg_ctrl,
      stat_reg_o     => s_pg_stat,
      pg_src_i       => s_pg_snk_o,
      pg_src_o       => s_pg_snk_i);
  end generate;

  fec_encoder_y   : if g_fec_encoder generate
  fec_en       : fec_encoder
    port map (
      clk_i       => clk_i,
      rst_n_i     => rst_n_i,
      ctrl_reg_i  => s_fec_ctrl,
      stat_reg_o  => s_fec_enc_stat,
      enc_src_o   => wr_src_o,
      enc_src_i   => wr_src_i,
      enc_snk_i   => s_enc_snk_i,
      enc_snk_o   => s_enc_snk_o);
  end generate;

  fec_encoder_n   : if (not g_fec_encoder) generate
    eb_snk_o       <= wr_src_i;
    wr_src_o       <= eb_snk_i;
  end generate;

  fec_decoder_y   : if g_fec_decoder generate
  fec_de       : fec_decoder
    generic map (
      g_dpram_size        => 90112/4,
      g_init_file         => g_init_file,
      g_upper_bridge_sdb  => g_upper_bridge_sdb)
    port map (
      clk_i             => clk_i,
      rst_n_i           => rst_n_i,
      rst_lm32_n_i      => rst_lm32_n_i,
      ctrl_reg_i        => s_fec_ctrl,
      stat_reg_o        => s_fec_dec_stat,
      dec_snk_i         => wr_snk_i,
      dec_snk_o         => wr_snk_o,
      dec_src_i         => eb_src_i,
      dec_src_o         => eb_src_o,
      wb_cross_master_i => wb_cross_master_i,
      wb_cross_master_o => wb_cross_master_o);
  end generate;

  fec_decoder_n   : if (not g_fec_decoder) generate
    eb_src_o    <= wr_snk_i;
    wr_snk_o    <= eb_src_i;
  end generate;
  
  s_fec_stat <= fec_stat_pack(s_fec_enc_stat,s_fec_dec_stat);

  ctrl_stat_reg : wb_slave_fec
    port map (
      clk_i          => clk_i,
      rst_n_i        => rst_n_i,
      wb_slave_i     => wb_ctrl_stat_slave_i,
      wb_slave_o     => wb_ctrl_stat_slave_o,
      fec_stat_reg_i => s_fec_stat,
      fec_ctrl_reg_o => s_fec_ctrl,
      time_code_i    => time_code_i,
      pg_stat_reg_i  => s_pg_stat,
      pg_ctrl_reg_o  => s_pg_ctrl);

end rtl;
