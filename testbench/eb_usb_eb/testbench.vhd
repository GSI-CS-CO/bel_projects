library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;  

-- package with component to test on this testbench
--use work.pcie_tlp.all;
use work.wishbone_pkg.all;
use work.ez_usb_pkg.all;
use work.mbox_pkg.all;
use work.etherbone_pkg.all;
use work.wr_fabric_pkg.all;

entity testbench is
end entity;

architecture simulation of testbench is

  -- clock/reset generation
  signal rst              : std_logic := '1';
  signal rst_n            : std_logic := '0';
  signal rstn_sys         : std_logic := '0';
  constant clk_50_period  : time      := 20 ns;
  constant clk_125_period : time      :=  8 ns;
  constant clk_sys_period : time      := 16 ns;
  signal clk_50           : std_logic := '1';
  signal clk_125          : std_logic := '1';
  signal clk_sys          : std_logic := '1';


  -- wb signals
  signal wb_mosi : t_wishbone_master_out;
  signal wb_miso : t_wishbone_master_in;

  signal uart_usb     : std_logic := '0';
  signal uart_wrc     : std_logic := '0';
  signal usb_rstn     : std_logic := '0';
  signal usb_ebcyc    : std_logic := '0';
  signal usb_readyn   : std_logic := '0';
  signal usb_fifoadr  : std_logic_vector(1 downto 0) := (others => '0');
  signal usb_fulln    : std_logic := '0';
  signal usb_sloen    : std_logic := '0';
  signal usb_emptyn   : std_logic := '0';
  signal usb_slrdn    : std_logic := '0';
  signal usb_slwrn    : std_logic := '0';
  signal usb_pktendn  : std_logic := '0';
  signal usb_fd_io    : std_logic_vector(7 downto 0) := (others => 'Z');
  signal s_usb_fd     : std_logic_vector(7 downto 0) := (others => '0');
  signal s_usb_fd_oen : std_logic := '0';

  signal counter : integer := 0;

  -- SDB stuff
 constant c_minislave_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"00",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"000000000000000f",
    product => (
    vendor_id     => x"0000000000000651",
    device_id     => x"12345678",
    version       => x"00000001",
    date          => x"20100905",
    name          => "GSI:MSI_MINISLAVE  ")));

  constant c_zero_master : t_wishbone_master_out := (
    cyc => '0',
    stb => '0',
    adr => (others => '0'),
    sel => (others => '0'),
    we  => '0',
    dat => (others => '0'));


  ----------------------------------------------------------------------------------
  -- from USB to etherbone_master_slave_wrapper ------------------------------------
  ----------------------------------------------------------------------------------
  signal usb_wb_mosi : t_wishbone_master_out;
  signal usb_wb_miso : t_wishbone_master_in;
  signal wrf_loopback_out : t_wrf_source_out;
  signal wrf_loopback_in  : t_wrf_source_in;

  ----------------------------------------------------------------------------------
  -- GSI Top Crossbar Masters ------------------------------------------------------
  ----------------------------------------------------------------------------------

  constant c_top_masters : natural := 1;
  --constant c_topm_usb    : natural := 0;
  constant c_topm_ebs    : natural := 0;


  constant c_top_layout_req_masters : t_sdb_record_array(c_top_masters-1 downto 0) :=
   --(c_topm_usb     => f_sdb_auto_msi(c_usb_msi, true),
   (c_topm_ebs     => f_sdb_auto_msi(c_ebs_msi, false));

  constant c_top_layout_masters : t_sdb_record_array := f_sdb_auto_layout(c_top_layout_req_masters);
  constant c_top_bridge_msi     : t_sdb_msi          := f_xwb_msi_layout_sdb(c_top_layout_masters);

  signal top_bus_slave_i  : t_wishbone_slave_in_array  (c_top_masters-1 downto 0);
  signal top_bus_slave_o  : t_wishbone_slave_out_array (c_top_masters-1 downto 0);
  signal top_msi_master_i : t_wishbone_master_in_array (c_top_masters-1 downto 0);
  signal top_msi_master_o : t_wishbone_master_out_array(c_top_masters-1 downto 0);

  ----------------------------------------------------------------------------------
  -- GSI Top Crossbar Slaves -------------------------------------------------------
  ----------------------------------------------------------------------------------

  -- Only put a slave here if it has critical performance requirements!
  constant c_top_slaves        : natural := 1; -- number of slaves (only 1 for now)
  constant c_tops_minislave    : natural := 0; -- slave index 0
--constant c_tops_anotherslave : natural := 1; -- slave index 1
--           ...                 natural := 2: -- salve index 3

  constant c_top_layout_req_slaves : t_sdb_record_array(c_top_slaves-1 downto 0) :=
   (c_tops_minislave    => f_sdb_auto_device(c_minislave_sdb, true));

  constant c_top_layout      : t_sdb_record_array := f_sdb_auto_layout(c_top_layout_req_masters, c_top_layout_req_slaves);
  constant c_top_sdb_address : t_wishbone_address := f_sdb_auto_sdb   (c_top_layout_req_masters, c_top_layout_req_slaves);

  signal top_msi_slave_i  : t_wishbone_slave_in_array  (c_top_slaves-1 downto 0) := (others => c_zero_master);
  signal top_msi_slave_o  : t_wishbone_slave_out_array (c_top_slaves-1 downto 0);
  signal top_bus_master_i : t_wishbone_master_in_array (c_top_slaves-1 downto 0);
  signal top_bus_master_o : t_wishbone_master_out_array(c_top_slaves-1 downto 0);



begin


  ---- generate clock and reset signal -------
  clk_50  <= not clk_50  after clk_50_period/2;
  clk_125 <= not clk_125 after clk_125_period/2;
  clk_sys <= not clk_sys after clk_sys_period/2;
  rst     <= '0'         after clk_50_period*20;
  rst_n   <= not rst;
  rstn_sys<= not rst;
  --------------------------------------------

  ---- instance of EZUSB-chip 
  -- this simulates the physical chip that is connected to the FPGA
  chip : entity work.ez_usb_chip
    port map (
      rstn_i    => usb_rstn,
      ebcyc_o   => usb_ebcyc,
      readyn_o  => usb_readyn,
      fifoadr_i => usb_fifoadr,
      fulln_o   => usb_fulln,
      emptyn_o  => usb_emptyn,
      sloen_i   => usb_sloen,
      slrdn_i   => usb_slrdn,
      slwrn_i   => usb_slwrn,
      pktendn_i => usb_pktendn,
      fd_io     => usb_fd_io
      );

  ---- instance of ez_usb component
  --usb_readyn <= 'Z';
  usb_fd_io <= s_usb_fd when s_usb_fd_oen='1' else (others => 'Z');
  usb : ez_usb
    generic map(
      g_sdb_address => c_top_sdb_address,
      g_sys_freq => 10 -- this tells the component our frequency is only 10kHz. 
                        -- reason: so it doesn't wait too many clock tics until 
                        --  it releases the ez_usb chip from its reset
                        )
    port map(
      clk_sys_i => clk_sys,
      rstn_i    => rstn_sys,
      master_i  => usb_wb_miso,
      master_o  => usb_wb_mosi,
      uart_o    => uart_usb,
      uart_i    => uart_wrc,
      rstn_o    => usb_rstn,
      ebcyc_i   => usb_ebcyc,
      speed_i   => '0',
      shift_i   => '0',
      readyn_i  => usb_readyn,
      fifoadr_o => usb_fifoadr,
      fulln_i   => usb_fulln,
      sloen_o   => usb_sloen,
      emptyn_i  => usb_emptyn,
      slrdn_o   => usb_slrdn,
      slwrn_o   => usb_slwrn,
      pktendn_o => usb_pktendn,
      fd_i      => usb_fd_io,
      fd_o      => s_usb_fd,
      fd_oen_o  => s_usb_fd_oen); 

  --wr_uart_o <= uart_wrc;
  --uart_mux <= uart_usb and wr_uart_i;


  eb_msw: eb_master_slave_wrapper
    generic map (
      g_with_master     => true,
      g_ebs_sdb_address => (x"00000000" & c_top_sdb_address)
    )
    port map (
      clk_i           => clk_sys,                     -- : in  std_logic;
      nRst_i          => rstn_sys,                    -- : in  std_logic;
      snk_i           => wrf_loopback_out,            -- : in  t_wrf_sink_in;
      snk_o           => wrf_loopback_in,             -- : out t_wrf_sink_out;
      src_o           => wrf_loopback_out,            -- : out t_wrf_source_out;
      src_i           => wrf_loopback_in,             -- : in  t_wrf_source_in;
      ebs_cfg_slave_o => open,                        -- : out t_wishbone_slave_out;
      ebs_cfg_slave_i => c_zero_master,               -- : in  t_wishbone_slave_in;
      ebs_wb_master_o => top_bus_slave_i(c_topm_ebs), -- : out t_wishbone_master_out;
      ebs_wb_master_i => top_bus_slave_o(c_topm_ebs), -- : in  t_wishbone_master_in;
      ebm_wb_slave_i  => usb_wb_mosi,                 -- : in  t_wishbone_slave_in;
      ebm_wb_slave_o  => usb_wb_miso                  -- : out t_wishbone_slave_out       
    );



  top_bar : xwb_sdb_crossbar
    generic map(
      g_num_masters => c_top_masters,
      g_num_slaves  => c_top_slaves,
      g_registered  => true,
      g_wraparound  => true,
      g_layout      => c_top_layout,
      g_sdb_addr    => c_top_sdb_address)
    port map(
      clk_sys_i     => clk_sys,
      rst_n_i       => rstn_sys,
      slave_i       => top_bus_slave_i,
      slave_o       => top_bus_slave_o,
      msi_master_i  => top_msi_master_i,
      msi_master_o  => top_msi_master_o,
      master_i      => top_bus_master_i,
      master_o      => top_bus_master_o,
      msi_slave_i   => top_msi_slave_i,
      msi_slave_o   => top_msi_slave_o);




  minislave : entity work.wb_minislave
  port map (
    clk_i   => clk_sys,
    rst_n_i => rst_n,
    slave_i => top_bus_master_o(c_tops_minislave),
    slave_o => top_bus_master_i(c_tops_minislave)
  );



end architecture;



