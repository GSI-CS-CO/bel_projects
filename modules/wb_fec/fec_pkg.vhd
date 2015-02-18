library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wr_fabric_pkg.all;

package fec_pkg is 


  type t_pg_fsm is  (  IDLE,
                       GENERATING,
                       HALTING);

  type t_frame_fsm  is (  INIT_HDR,
                          ETH_HDR,
                          PAY_LOAD,
                          OOB,
                          IDLE);

  subtype t_wrf_bus    is std_logic_vector(15 downto 0);
  subtype t_mac_addr   is std_logic_vector(47 downto 0);
  subtype t_eth_type   is std_logic_vector(15 downto 0);
  subtype t_eth_hdr    is std_logic_vector(111 downto 0);

  -- Frame header
  type t_eth_frame_header is
    record
      eth_src_addr   :  t_mac_addr;
      eth_des_addr   :  t_mac_addr;
      eth_etherType  :  t_eth_type;
    --vlan_priority  :  std_logic_vector(2  downto 0);
    --valn_id        :  std_logic_vector(11 downto 0);
  end record;

  -- time code
  type t_time_code is
    record
      time_valid 	: std_logic;
      tai       	: std_logic_vector(39 downto 0);
      cycles     	: std_logic_vector(27 downto 0);
  end record;

  -- FEC header
  type t_fec_header is
    record
      fec_schema     :  integer;
      fec_id         :  std_logic_vector(2 downto 0);
  end record;

  type t_fec_latency_stat_reg is
    record
    fec_lat     : std_logic_vector(31 downto 0);
    fec_lat_max : std_logic_vector(31 downto 0);
    fec_lat_min : std_logic_vector(31 downto 0);
    fec_lat_acc : std_logic_vector(31 downto 0);
    fec_num_lat : std_logic_vector(31 downto 0);
  end record;

  -- WB FEC Register
  type t_fec_ctrl_reg is
    record
    -- encoder
    en_enc      : std_logic;
    -- decoder
    en_dec      : std_logic;
    time_code   : t_time_code;
  end record;
  
  type t_fec_enc_stat_reg is
    record
    frame_enc   : std_logic_vector(31 downto 0);
    latency_enc : t_fec_latency_stat_reg;
  end record;

  type t_fec_dec_stat_reg is
    record
    err_dec     : std_logic_vector(31 downto 0);
    latency_dec : t_fec_latency_stat_reg;
  end record;

   type t_fec_stat_reg is
    record
    stat_enc    : t_fec_enc_stat_reg;
    stat_dec    : t_fec_dec_stat_reg;
  end record;  

  type t_pg_ctrl_reg is
    record
      en_pg       : std_logic;
      --payload     : integer;
      payload     : std_logic_vector(15 downto 0);
      --rate        : integer;
      rate        : std_logic_vector(31 downto 0);
      eth_hdr     : t_eth_frame_header;
      time_code   : t_time_code;
  end record;

  type t_pg_stat_reg is
    record
    --frame_gen   :  integer;
    frame_gen   :  std_logic_vector(31 downto 0);
  end record;

  type t_pg_state is
    record
      gen_packet   : std_logic;
      cyc_ended    : std_logic;
      halt         : std_logic;
  end record;

  constant c_fec_reg_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"4", -- 32-bit port granularity
    sdb_component => (
      addr_first    => x"0000000000000000",
      addr_last     => x"000000000000ffff",
      product => (
      vendor_id     => x"0000000000000651", -- GSI
      device_id     => x"53bee0e2",
      version       => x"00000001",
      date          => x"20140422",
      name          => "WR_FEC_CTRL_STAT   ")));
  
  constant c_fec_dec_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"4", -- 32-bit port granularity
    sdb_component => (
      addr_first    => x"0000000000000000",
      addr_last     => x"000000000000ffff",
      product => (
      vendor_id     => x"0000000000000651", -- GSI
      device_id     => x"8b7ec5a3",
      version       => x"00000001",
      date          => x"20140422",
      name          => "WR_FEC_DECODER_UNIT")));

  constant c_fec_fabric2wb_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"4", -- 32-bit port granularity
    sdb_component => (
      addr_first    => x"0000000000000000",
      addr_last     => x"000000000000ffff",
      product => (
      vendor_id     => x"0000000000000651", -- GSI
      device_id     => x"c2983832",
      version       => x"00000001",
      date          => x"20140422",
      name          => "WR_FEC_FABRIC2WB   ")));
 
  constant c_fec_wb2fabric_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"4", -- 32-bit port granularity
    sdb_component => (
      addr_first    => x"0000000000000000",
      addr_last     => x"000000000000ffff",
      product => (
      vendor_id     => x"0000000000000651", -- GSI
      device_id     => x"CE979C10",
      version       => x"00000001",
      date          => x"20140422",
      name          => "WR_FEC_WB2FABRIC   ")));

  -- ethernet header ( without VLAN tag ) 112 bits / 16 bit/clk
  constant c_hdr_l   : integer := 7 - 1; -- the header needs 7 cycles in 125 Mhz

  constant c_eth_frame_header_default : t_eth_frame_header := (
    eth_src_addr   => x"abababababab",
    eth_des_addr   => x"ffffffffffff",
    eth_etherType  => x"0800");
    --vlan_priority  => x"000",
    --valn_id        => x"000");

  constant c_fec_header_default : t_fec_header    := (
    fec_schema  => 7,
    fec_id      => "111");

  constant c_time_code 		: t_time_code     := (
    time_valid  => '0',
    tai         => (others => '0'),
    cycles      => (others => '0'));
  constant c_fec_latency_stat_default : t_fec_latency_stat_reg := (
    fec_lat     => (others => '0'),
    fec_lat_max => (others => '0'),
    fec_lat_min => (others => '0'),
    fec_lat_acc => (others => '0'),
    fec_num_lat => (others => '0'));

  constant c_ctrl_reg_default   : t_fec_ctrl_reg  := (
    en_enc      => '1',
    en_dec      => '1',
    time_code   => c_time_code);
  
  constant c_stat_dec_reg_default   : t_fec_dec_stat_reg  := (
    err_dec     => (others => '0'),
    latency_dec => c_fec_latency_stat_default);

  constant c_stat_enc_reg_default   : t_fec_enc_stat_reg  := (
    frame_enc   => (others => '0'),
    latency_enc => c_fec_latency_stat_default);

  constant c_pg_stat_default    : t_pg_stat_reg   := (
    frame_gen   => (others => '0'));      
  
  constant c_pg_ctrl_default    : t_pg_ctrl_reg   := (
    en_pg       => '0',
    payload     => x"01f4",
    rate        => x"00019190",
    eth_hdr     => c_eth_frame_header_default,
    time_code   => c_time_code);

  constant c_pg_state_default   : t_pg_state      := (
    gen_packet  => '0',
    cyc_ended   => '0',
    halt        => '0');
  
  constant c_stat_reg_default   : t_fec_stat_reg  := (
    stat_enc    => c_stat_enc_reg_default,
    stat_dec    => c_stat_dec_reg_default);

  -- Component

  component packet_gen is
  port (
    clk_i       : in  std_logic;
    rst_n_i     : in  std_logic;
    ctrl_reg_i  : in  t_pg_ctrl_reg;
    stat_reg_o  : out t_pg_stat_reg;
    pg_src_i    : in  t_wrf_source_in;
    pg_src_o    : out t_wrf_source_out);
  end component;

  component xwb_fec is
    generic(
      g_fec_encoder     : boolean := true;
      g_fec_decoder     : boolean := false;
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
      time_code_i           : in  t_time_code);
 
  end component;

  component fec_encoder is
    port(
      clk_i             : in  std_logic;
      rst_n_i           : in  std_logic;
      ctrl_reg_i        : in  t_fec_ctrl_reg;
      stat_reg_o        : out t_fec_enc_stat_reg;
      enc_src_o         : out t_wrf_source_out;
      enc_src_i         : in  t_wrf_source_in;
      enc_snk_i         : in  t_wrf_sink_in;
      enc_snk_o         : out t_wrf_sink_out);
  end component;

  component fec_decoder is
    generic(
      g_dpram_size        : integer := 90112/4;
      g_init_file         : string  := "";
      g_linear_block_dec  : boolean  := false;
      g_upper_bridge_sdb: t_sdb_bridge);
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
  end component;

  component cross_fabric is
    generic (
      g_nodes     : integer := 2);
    port (
      clk_i       : in  std_logic;
      rst_n_i     : in  std_logic;
      mux_src_o   : out t_wrf_source_out;
      mux_src_i   : in  t_wrf_source_in;
      mux_snk_i   : in  t_wrf_sink_in_array(g_nodes-1 downto 0);
      mux_snk_o   : out t_wrf_sink_out_array(g_nodes-1 downto 0));
  end component;

  component wb_slave_fec is
    port (
      clk_i          : in  std_logic;
      rst_n_i        : in  std_logic;
      wb_slave_i     : in  t_wishbone_slave_in;
      wb_slave_o     : out t_wishbone_slave_out;
      fec_stat_reg_i : in  t_fec_stat_reg;
      fec_ctrl_reg_o : out t_fec_ctrl_reg;
      time_code_i    : in t_time_code;
      pg_stat_reg_i  : in  t_pg_stat_reg;
      pg_ctrl_reg_o  : out t_pg_ctrl_reg);
  end component;

  component fec_decoder_unit is
    port(
      clk_i          : in  std_logic;
      rst_n_i        : in  std_logic;
      dec_src_i      : in  t_wrf_source_in;
      dec_src_o      : out t_wrf_source_out;
      wb_dec_slave_i : in  t_wishbone_slave_in;
      wb_dec_slave_o : out t_wishbone_slave_out);
  end component;

  component wb2fabric is
    port(
      clk_i           : in  std_logic;
      rst_n_i         : in  std_logic;
      dec_src_i       : in  t_wrf_source_in;
      dec_src_o       : out t_wrf_source_out;
      wb_wbf_slave_i  : in  t_wishbone_slave_in;
      wb_wbf_slave_o  : out t_wishbone_slave_out);
  end component;

  component fabric2wb is
    generic(
      g_size_ram      : integer);
    port(
      clk_i           : in  std_logic;
      rst_n_i         : in  std_logic;
      dec_snk_i       : in  t_wrf_sink_in;
      dec_snk_o       : out t_wrf_sink_out;
      irq_fwb_o       : out std_logic;      
      wb_ram_master_i : in  t_wishbone_master_in;
      wb_ram_master_o : out t_wishbone_master_out;
      wb_fwb_slave_i  : in  t_wishbone_slave_in;
      wb_fwb_slave_o  : out t_wishbone_slave_out);
  end component;
  
  component fec_timekeeper is
    port(
      clk_i       : in  std_logic;
      rst_n_i     : in  std_logic;
      ctrl_reg_i  : in  t_fec_ctrl_reg;
      lat_stat_o  : out t_fec_latency_stat_reg;
      init_i	    : in  std_logic;
      halt_i	    : in  std_logic);
  end component;

  function f_eth_hdr ( eth_hdr : t_eth_frame_header)
    return t_eth_hdr;

  function fec_stat_pack ( stat_enc : t_fec_enc_stat_reg;
                           stat_dec : t_fec_dec_stat_reg)
    return t_fec_stat_reg;

end package fec_pkg;

package body fec_pkg is

  -- Functions
  function f_eth_hdr( eth_hdr : t_eth_frame_header)
     return t_eth_hdr is

  variable hdr  : t_eth_hdr := (others => '0');
    begin
      hdr :=  eth_hdr.eth_des_addr &
      eth_hdr.eth_src_addr &
      eth_hdr.eth_etherType;
    return hdr;
  end f_eth_hdr;

  function fec_stat_pack ( stat_enc : t_fec_enc_stat_reg;
                           stat_dec : t_fec_dec_stat_reg)
    return t_fec_stat_reg is

      variable fec_stat : t_fec_stat_reg;
    begin

      fec_stat.stat_dec := stat_dec;
      fec_stat.stat_enc := stat_enc;

    return fec_stat;
  end fec_stat_pack;
end fec_pkg;
