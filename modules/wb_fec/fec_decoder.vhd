library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wr_fabric_pkg.all;
use work.genram_pkg.all;

use work.fec_pkg.all;

entity fec_decoder is
  generic(
    g_dpram_size         : integer  := 90112/4;
    g_init_file          : string   :="";
    g_linear_block_dec   : boolean  := false;
    g_upper_bridge_sdb   : t_sdb_bridge);
  port(
    clk_i             : in  std_logic;
    rst_n_i           : in  std_logic;
    rst_lm32_n_i      : in  std_logic;
    ctrl_reg_i        : in  t_fec_ctrl_reg;
    stat_reg_o        : out t_fec_dec_stat_reg;
    dec_src_o         : out t_wrf_source_out;
    dec_src_i         : in  t_wrf_source_in;
    dec_snk_i         : in  t_wrf_sink_in;
    dec_snk_o         : out t_wrf_sink_out;
    wb_cross_master_i : in  t_wishbone_master_in;
    wb_cross_master_o : out t_wishbone_master_out);
end fec_decoder;

architecture rtl of fec_decoder is
  --constant c_dpram_frame_size : integer := 3056; -- 12224/4
  constant c_dpram_frame_size : integer := 3200;

  constant c_master       : natural := 2;

  constant c_lm32_data    : natural := 0;
  constant c_lm32_add     : natural := 1;

  constant c_slave              : natural := 5;

  constant c_upper_bridge       : natural := 4;
  constant c_wb2f               : natural := 3;
  constant c_f2wb_lc            : natural := 2;
  constant c_frame_dpram        : natural := 1;
  constant c_lm32_dpram         : natural := 0;

  -----------------------------------------------------------------------------
  --WB intercon
  -----------------------------------------------------------------------------  
  constant c_layout_req : t_sdb_record_array(c_slave-1 downto 0) :=
     (c_lm32_dpram        => f_sdb_embed_device((f_xwb_dpram(g_dpram_size)),      x"00000000"),
   c_frame_dpram       => f_sdb_embed_device((f_xwb_dpram(c_dpram_frame_size)),   x"00100000"),
   c_f2wb_lc           => f_sdb_embed_device(c_fec_fabric2wb_sdb,                 x"00200000"),
   c_wb2f              => f_sdb_embed_device(c_fec_wb2fabric_sdb,                 x"00300000"),
   c_upper_bridge      => f_sdb_embed_bridge(g_upper_bridge_sdb,                  x"80000000"));
 
   constant c_sdb_address : t_wishbone_address := x"000F0000";

   signal cbar_slave_i  : t_wishbone_slave_in_array  (c_master-1 downto 0);
   signal cbar_slave_o  : t_wishbone_slave_out_array (c_master-1 downto 0);
   signal cbar_master_i : t_wishbone_master_in_array (c_slave-1 downto 0);
   signal cbar_master_o : t_wishbone_master_out_array(c_slave-1 downto 0);

   signal s_wb_m_f2wb_lc_i : t_wishbone_master_in;
   signal s_wb_m_f2wb_lc_o : t_wishbone_master_out;

   -- irq fabric2wb FIFO
   signal s_lm32_irq    : std_logic_vector(31 downto 0) := (others => '0');
   signal s_rst_lm32_n  : std_logic;

   signal s_init  : std_logic;
   signal s_halt  : std_logic;
    
   signal s_dec_src_o : t_wrf_source_out;
   signal s_dec_src_i : t_wrf_source_in;

begin

  -----------------------------------------------------------------------------
  -- Fabric intercon
  -----------------------------------------------------------------------------

  dec_src_o    <= s_dec_src_o;
  s_dec_src_i  <= dec_src_i;

  -----------------------------------------------------------------------------
  -- FEC Decoder Unit
  -----------------------------------------------------------------------------

  linear_block_dec_y : if g_linear_block_dec generate
--  LINEAR_DECODE : xxx
--    port map(
--      clk_i           => clk_i,
--      rst_n_i         => rst_n_i,
--      dec_snk_i       => dec_snk_i,  
--      dec_snk_o       => dec_snk_o,
--      irq_fwb_o       => s_lm32_irq(0),
--      wb_ram_master_i => s_wb_m_f2wb_lc_i,
--      wb_ram_master_o => s_wb_m_f2wb_lc_o,
--      wb_fwb_slave_i  => cbar_m_o(c_f2wb_lc),
--      wb_fwb_slave_o  => cbar_m_i(c_f2wb_lc));
  end generate; 


  linear_block_dec_n : if (not g_linear_block_dec) generate
  F2W : fabric2wb
    generic map( g_size_ram => c_dpram_frame_size)
    port map(
      clk_i           => clk_i,
      rst_n_i         => rst_n_i,
      dec_snk_i       => dec_snk_i,  
      dec_snk_o       => dec_snk_o,
      irq_fwb_o       => s_lm32_irq(0),
      wb_ram_master_i => s_wb_m_f2wb_lc_i,
      wb_ram_master_o => s_wb_m_f2wb_lc_o,
      wb_fwb_slave_i  => cbar_master_o(c_f2wb_lc),
      wb_fwb_slave_o  => cbar_master_i(c_f2wb_lc));      

  end generate;

  -----------------------------------------------------------------------------
  -- Wishbone to Frabic dapater
  -----------------------------------------------------------------------------
  
  W2B : wb2fabric
    port map(
      clk_i           => clk_i,
      rst_n_i         => rst_n_i,
      dec_src_i       => s_dec_src_i,  
      dec_src_o       => s_dec_src_o,
      wb_wbf_slave_i  => cbar_master_o(c_wb2f),
      wb_wbf_slave_o  => cbar_master_i(c_wb2f));

  -----------------------------------------------------------------------------
  -- LM32 softCPU
  -----------------------------------------------------------------------------
  
  LM32_CORE : xwb_lm32
    generic map(g_profile => "medium_icache_debug",
                g_sdb_address => c_sdb_address)
    port map(
      clk_sys_i => clk_i,
      rst_n_i   => s_rst_lm32_n,
      irq_i     => s_lm32_irq,
      dwb_o     => cbar_slave_i(c_lm32_data), -- Data bus
      dwb_i     => cbar_slave_o(c_lm32_data),
      iwb_o     => cbar_slave_i(c_lm32_add), -- Instruction bus
      iwb_i     => cbar_slave_o(c_lm32_add));

  s_rst_lm32_n <= rst_n_i and rst_lm32_n_i;
  
  DPRAM_LM32 : xwb_dpram
    generic map(
      g_size                  => g_dpram_size,
      g_init_file             => g_init_file,
      g_must_have_init_file   => true,
      g_slave1_interface_mode => PIPELINED,
      g_slave2_interface_mode => PIPELINED,
      g_slave1_granularity    => BYTE,
      g_slave2_granularity    => BYTE)  
    port map(
      clk_sys_i => clk_i,
      rst_n_i   => rst_n_i,
      slave1_i  => cbar_master_o(c_lm32_dpram),
      slave1_o  => cbar_master_i(c_lm32_dpram),
      slave2_i  => cc_dummy_slave_in,
      slave2_o  => open);


  -----------------------------------------------------------------------------
  -- Frame DPRAM 1600 bytes payload frame => 12000 bits x 4 frames = 48000 bits
  -- size of the ram = port_a_width * n_words
  -- port_a_width=16bit, for 48000 bits, 3200
  -----------------------------------------------------------------------------
  
  DPRAM_FRAME : xwb_dpram_mixed
    generic map(
      g_size                   => c_dpram_frame_size,
      g_must_have_init_file    => false,
      g_slave1_interface_mode  => PIPELINED,
      g_slave2_interface_mode  => PIPELINED,
      g_dpram_port_a_width     => 32, --slave1
      g_dpram_port_b_width     => 16, --slave2
      g_slave1_granularity     => WORD,
      g_slave2_granularity     => WORD)
    port map(
      clk_slave1_i  => clk_i,
      clk_slave2_i  => clk_i,
      rst_n_i       => rst_n_i,
      slave1_i      => cbar_master_o(c_frame_dpram),
      slave1_o      => cbar_master_i(c_frame_dpram),
      slave2_i      => s_wb_m_f2wb_lc_o,
      slave2_o      => s_wb_m_f2wb_lc_i);

  -----------------------------------------------------------------------------
  -- WB gateway to the next WB intercon
  -----------------------------------------------------------------------------

  cbar_master_i(c_upper_bridge)	<= wb_cross_master_i;
  wb_cross_master_o  				    <= cbar_master_o(c_upper_bridge);

  -----------------------------------------------------------------------------
  -- WB intercon
  -----------------------------------------------------------------------------
  WB_CON : xwb_sdb_crossbar
    generic map(
      g_num_masters => c_master,
      g_num_slaves  => c_slave,
      g_registered  => true,
      g_wraparound  => true,
      g_layout      => c_layout_req,
      g_sdb_addr    => c_sdb_address)  
    port map(
      clk_sys_i => clk_i,
      rst_n_i   => rst_n_i,
      -- Master connections (INTERCON is a slave)
      slave_i   => cbar_slave_i,
      slave_o   => cbar_slave_o,
      -- Slave connections (INTERCON is a master)
      master_i  => cbar_master_i,
      master_o  => cbar_master_o);
  -----------------------------------------------------------------------------
  -- Latency Module measurement
  -----------------------------------------------------------------------------
  -- the frame arrives in the frabic2wb module
  s_init  <= dec_snk_i.cyc and dec_snk_i.stb;
  -- the frame leaves the FEC module
  s_halt  <= s_dec_src_o.cyc and s_dec_src_o.stb;

  LATENCY : fec_timekeeper 
    port map(
      clk_i       => clk_i,
      rst_n_i     => rst_n_i,
      ctrl_reg_i  => ctrl_reg_i,
      lat_stat_o  => stat_reg_o.latency_dec,
      init_i	    => s_init,
      halt_i	    => s_halt);

end rtl;
