library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wr_fabric_pkg.all;
use work.endpoint_pkg.all;

package packet_pkg is 


  type t_pg_fsm is  (  IDLE,
                       CONTINUOUS,
                       DISCRETE,
                       CON_HALTING,
                       DIS_HALTING);

  type t_frame_fsm  is (  INIT_HDR,
                          ETH_HDR,
                          PAY_LOAD,
                          OOB,
                          IDLE);   

  subtype t_wrf_bus    is std_logic_vector(15 downto 0);
  subtype t_mac_addr   is std_logic_vector(47 downto 0);
  subtype t_eth_type   is std_logic_vector(15 downto 0);
  subtype t_eth_hdr    is std_logic_vector(111 downto 0);

  -- Packet generator-
  --------------------
  -- Frame header
  type t_eth_frame_header is
    record
      eth_src_addr   :  t_mac_addr;
      eth_des_addr   :  t_mac_addr;
      eth_etherType  :  t_eth_type;
  end record;

  type t_pg_ctrl_reg is
    record
      en_pg       : std_logic;
      mode        : std_logic_vector(1 downto 0);
      payload     : std_logic_vector(15 downto 0);
      rate        : std_logic_vector(31 downto 0);
      udp         : std_logic_vector(7 downto 0);
      eth_hdr     : t_eth_frame_header;
  end record;

  type t_pg_stat_reg is
    record
    frame_gen   : std_logic_vector(31 downto 0);
    count       : std_logic_vector(31 downto 0);
  end record;

  type t_pg_state is
    record
      gen_con_packet   : std_logic;
      gen_dis_packet   : std_logic;
      cyc_ended        : std_logic;
      new_start        : std_logic;
      halt             : std_logic;
  end record;

  constant c_packet_gen_sdb : t_sdb_device := (
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
      date          => x"20150615",
      name          => "GSI:PKT_GENERATOR  ")));
  
  constant c_hdr_l   : integer := 7 - 1; -- the header needs 7 cycles in 125 Mhz

  constant c_eth_frame_header_default : t_eth_frame_header := (
    eth_src_addr   => x"abababababab",
    eth_des_addr   => x"ffffffffffff",
    eth_etherType  => x"0800");

  constant c_pg_stat_default    : t_pg_stat_reg   := (
    frame_gen       => (others => '0'),      
    count           => x"00000000");

  constant c_pg_ctrl_default    : t_pg_ctrl_reg   := (
    en_pg       => '0',
    mode        => "00",
    payload     => x"01f4",
    rate        => x"00000404",
    udp         => x"11",
    eth_hdr     => c_eth_frame_header_default);

  constant c_pg_state_default   : t_pg_state      := (
    gen_con_packet  => '0',
	  gen_dis_packet  => '0',
    cyc_ended       => '0',
    new_start       => '0',
    halt            => '0');

  -- Component
  component packet_gen is
    port (
      clk_i       : in  std_logic;
      rst_n_i     : in  std_logic;
      ctrl_reg_i  : in  t_pg_ctrl_reg;
      stat_reg_o  : out t_pg_stat_reg;
      pg_timestamps_i     : in t_txtsu_timestamp;
      pg_tm_tai_i   : in std_logic_vector(39 downto 0); 
      pg_tm_cycle_i : in std_logic_vector(27 downto 0);
      pg_src_i    : in  t_wrf_source_in;
      pg_src_o    : out t_wrf_source_out);
  end component;

  component xwb_packet is
    port(
      clk_i                 : in std_logic;
      rst_n_i               : in std_logic;
      wr_src_o              : out t_wrf_source_out;
      wr_src_i              : in  t_wrf_source_in;
      timestamps_i      : in  t_txtsu_timestamp; 
      tm_tai_i          : in std_logic_vector(39 downto 0);
      tm_cycle_i        : in std_logic_vector(27 downto 0);
      wb_ctrl_stat_slave_o  : out t_wishbone_slave_out;
      wb_ctrl_stat_slave_i  : in  t_wishbone_slave_in);
  end component;

  component wb_slave_packet is
    port (
      clk_i          : in  std_logic;
      rst_n_i        : in  std_logic;
      wb_slave_i     : in  t_wishbone_slave_in;
      wb_slave_o     : out t_wishbone_slave_out;
      pg_stat_reg_i  : in  t_pg_stat_reg;
      pg_ctrl_reg_o  : out t_pg_ctrl_reg);
  end component;

  function f_eth_hdr ( eth_hdr : t_eth_frame_header)
    return t_eth_hdr;

end package packet_pkg;

package body packet_pkg is

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

end packet_pkg;
