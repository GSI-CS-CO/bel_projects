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
  constant g_lm32_init_files   : string  := "virtual_tr_stub.mif;virtual_tr_stub.mif";
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
  constant c_top_slaves        : natural := 3;
  constant c_tops_mbox         : natural := 0;
  constant c_tops_minislave    : natural := 1;
  constant c_tops_ftm_cluster  : natural := 2;

  constant c_ftm_slaves : t_sdb_bridge := f_cluster_bridge(c_top_bridge_msi, g_lm32_cores, g_lm32_ramsizes, g_lm32_are_ftm, g_delay_diagnostics);

  constant c_top_layout_req_slaves : t_sdb_record_array(c_top_slaves-1 downto 0) :=
   (c_tops_mbox         => f_sdb_auto_device(c_mbox_sdb,      true),
    c_tops_minislave    => f_sdb_auto_device(c_minislave_sdb, true),
    c_tops_ftm_cluster  => f_sdb_auto_bridge(c_ftm_slaves,    true));

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
      master_i  => top_bus_slave_o(c_topm_usb),
      master_o  => top_bus_slave_i(c_topm_usb),
      msi_slave_i => top_msi_master_o(c_topm_usb),
      msi_slave_o => top_msi_master_i(c_topm_usb),
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


  mailbox : mbox
    port map(
      clk_i        => clk_sys,
      rst_n_i      => rstn_sys,
      bus_slave_i  => top_bus_master_o(c_tops_mbox),
      bus_slave_o  => top_bus_master_i(c_tops_mbox),
      msi_master_o => top_msi_slave_i (c_tops_mbox),
      msi_master_i => top_msi_slave_o (c_tops_mbox));


  minislave : entity work.wb_minislave
  port map (
    clk_i   => clk_sys,
    rst_n_i => rst_n,
    slave_i => top_bus_master_o(c_tops_minislave),
    slave_o => top_bus_master_i(c_tops_minislave),
    msi_master_o => top_msi_slave_i (c_tops_minislave),
    msi_master_i => top_msi_slave_o (c_tops_minislave)
  );

  lm32 : ftm_lm32_cluster
    generic map(
      g_is_dm               => g_lm32_are_ftm,
      g_delay_diagnostics   => g_delay_diagnostics,
      g_cores               => g_lm32_cores,
      g_ram_per_core        => g_lm32_ramsizes,
      g_world_bridge_sdb    => c_ftm_slaves,
      g_clu_msi_sdb         => c_top_bridge_msi,
      g_init_files          => g_lm32_init_files,
      g_profiles            => g_lm32_profiles)
    port map(
      clk_ref_i          => clk_sys,
      rst_ref_n_i        => rst_n,
      clk_sys_i          => clk_sys,
      rst_sys_n_i        => rst_n,
      rst_lm32_n_i       => s_lm32_rstn,
      tm_tai8ns_i        => s_time,
      wr_lock_i          => tm_valid,
      lm32_masters_o     => top_bus_slave_i(top_bus_slave_i'high downto c_top_my_masters),
      lm32_masters_i     => top_bus_slave_o(top_bus_slave_o'high downto c_top_my_masters),
      lm32_msi_slaves_o  => top_msi_master_i(top_msi_master_i'high downto c_top_my_masters),
      lm32_msi_slaves_i  => top_msi_master_o(top_msi_master_o'high downto c_top_my_masters),
      clu_slave_o        => top_bus_master_i(c_tops_ftm_cluster),
      clu_slave_i        => top_bus_master_o(c_tops_ftm_cluster),
      clu_msi_master_o   => top_msi_slave_i(c_tops_ftm_cluster),
      clu_msi_master_i   => top_msi_slave_o(c_tops_ftm_cluster),
      dm_prioq_master_o  => open);

  s_lm32_rstn <= (others => rst_n);

  tai_counter: process
  begin
    wait until rising_edge(clk_125);
    if rst_n = '0' then
    else
      s_time <= std_logic_vector(unsigned(s_time) + 1);
    end if;
  end process;

end architecture;



