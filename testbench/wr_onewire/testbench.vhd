library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;  

-- package with component to test on this testbench
--use work.pcie_tlp.all;
use work.wishbone_pkg.all;
use work.ez_usb_pkg.all;
use work.mbox_pkg.all;

use work.ftm_pkg.all;
use work.eca_internals_pkg.all;
use work.wb_arria_reset_pkg.all;


-- use with socat pseudo terminals:
--   socat -d -d pty,raw,echo=0 pty,raw,echo=0  # creates /dev/pts/40 and /dev/pts/39
--   socat -u -d -d file:/dev/pts/40 pty,raw,echo=0 # creates /dev/pts/42
--   socat -U -d -d file:/dev/pts/40 pty,raw,echo=0 # creates /dev/pts/44
-- then start simulation and call:
--   eb-read -p dev/pts/39 0x01000000/4
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

  signal s_time : t_time := (others => '0');
  signal tm_valid : std_logic := '1';

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
  -- FTM constants and signals -----------------------------------------------------
  ----------------------------------------------------------------------------------
  constant g_lm32_are_ftm      : boolean := false;
  constant g_delay_diagnostics : boolean := false;
  constant g_lm32_ramsizes     : natural := 32768/4;
  constant g_lm32_profiles     : string  := "medium_icache_debug";
  constant g_lm32_init_files   : string  := ";";
  constant g_lm32_cores : integer := 2;

  signal s_lm32_rstn : std_logic_vector(g_lm32_cores-1 downto 0) := (others => '0');
  ----------------------------------------------------------------------------------
  -- GSI Top Crossbar Masters ------------------------------------------------------
  ----------------------------------------------------------------------------------

  constant c_top_my_masters : natural := 1;
  constant c_topm_usb    : natural := 0;

  constant c_top_layout_my_masters : t_sdb_record_array(c_top_my_masters-1 downto 0) :=
   (c_topm_usb     => f_sdb_auto_msi(c_usb_msi, true));


  -- The FTM adds a bunch of masters to this crossbar
  constant c_ftm_masters : t_sdb_record_array := f_lm32_masters_bridge_msis(g_lm32_cores);
  constant c_top_masters : natural := c_ftm_masters'length + c_top_my_masters;
  constant c_top_layout_req_masters : t_sdb_record_array(c_top_masters-1 downto 0) :=
    c_ftm_masters & c_top_layout_my_masters;

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
  constant c_top_slaves        : natural := 4;
  constant c_tops_mbox         : natural := 0;
  constant c_tops_minislave    : natural := 1;
  constant c_tops_reset        : natural := 2;
  constant c_tops_ftm_cluster  : natural := 3;

  constant c_ftm_slaves : t_sdb_bridge := f_cluster_bridge(c_top_bridge_msi, g_lm32_cores, g_lm32_ramsizes, g_lm32_are_ftm, g_delay_diagnostics);

  constant c_top_layout_req_slaves : t_sdb_record_array(c_top_slaves-1 downto 0) :=
   (c_tops_mbox         => f_sdb_auto_device(c_mbox_sdb,      true),
    c_tops_minislave    => f_sdb_auto_device(c_minislave_sdb, true),
    c_tops_reset        => f_sdb_auto_device(c_arria_reset,   true),
    c_tops_ftm_cluster  => f_sdb_auto_bridge(c_ftm_slaves,    true));

  constant c_top_layout      : t_sdb_record_array := f_sdb_auto_layout(c_top_layout_req_masters, c_top_layout_req_slaves);
  constant c_top_sdb_address : t_wishbone_address := f_sdb_auto_sdb   (c_top_layout_req_masters, c_top_layout_req_slaves);

  signal top_msi_slave_i  : t_wishbone_slave_in_array  (c_top_slaves-1 downto 0) := (others => c_zero_master);
  signal top_msi_slave_o  : t_wishbone_slave_out_array (c_top_slaves-1 downto 0);
  signal top_bus_master_i : t_wishbone_master_in_array (c_top_slaves-1 downto 0);
  signal top_bus_master_o : t_wishbone_master_out_array(c_top_slaves-1 downto 0);

  signal owire     : std_logic_vector(0 downto 0);
  signal owr_pwren : std_logic_vector(0 downto 0);
  signal owr_en    : std_logic_vector(0 downto 0);


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
      master_i  => wb_miso,
      master_o  => wb_mosi,
      msi_slave_i => c_dummy_slave_in,
      msi_slave_o => open,
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


  owire <= owr_pwren when owr_en="1" else "Z";
  owm : entity work.xwb_onewire_master
  port map(
    clk_sys_i => clk_sys,
    rst_n_i   => rstn_sys,

    -- Wishbone
    slave_i   => wb_mosi,
    slave_o   => wb_miso,
    desc_o    => open,

    owr_pwren_o => owr_pwren,
    owr_en_o    => owr_en,
    owr_i       => owire
    );



end architecture;



