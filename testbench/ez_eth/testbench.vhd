library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;  
use ieee.std_logic_textio.all; 

-- package with component to test on this testbench
--use work.pcie_tlp.all;

use work.wishbone_pkg.all;
use work.mbox_pkg.all;
use work.etherbone_pkg.all;
use work.wr_fabric_pkg.all;
use work.ez_eth_pkg.all;

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


  
  signal wrc_slave_i   : t_wishbone_slave_in;
  signal wrc_slave_o   : t_wishbone_slave_out;
  signal wrc_master_i  : t_wishbone_master_in;
  signal wrc_master_o  : t_wishbone_master_out;
  signal eb_src_out    : t_wrf_source_out;
  signal eb_src_in     : t_wrf_source_in;
  signal eb_snk_out    : t_wrf_sink_out;
  signal eb_snk_in     : t_wrf_sink_in;

  ---
  
  -- wb signals
  signal wb_mosi : t_wishbone_master_out;
  signal wb_miso : t_wishbone_master_in;


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
    addr_last     => x"0000000000000fff",
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
  -- GSI Top Crossbar Masters ------------------------------------------------------
  ----------------------------------------------------------------------------------

  constant c_top_masters : natural := 1;
  constant c_topm_ebs       : natural := 0;


    constant c_top_layout_req_masters : t_sdb_record_array(c_top_masters-1 downto 0) :=
   (c_topm_ebs     => f_sdb_auto_msi(c_ebs_msi,     false));   -- Need to add MSI support !!!
    ---
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
  constant c_top_slaves        : natural := 2; -- number of slaves (only 1 for now)
  constant c_tops_minislave    : natural := 0; -- slave index 0
  constant c_tops_ebcfgs       : natural := 1; -- slave index 0
--constant c_tops_anotherslave : natural := 1; -- slave index 1
--           ...                 natural := 2: -- salve index 3

  constant c_top_layout_req_slaves : t_sdb_record_array(c_top_slaves-1 downto 0) :=
   (c_tops_minislave    => f_sdb_auto_device(c_minislave_sdb, true),
    c_tops_ebcfgs       => f_sdb_auto_device(c_etherbone_sdb, false));

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

  ---- instance of EZETH-chip 
  -- virtual NIC
  chip : ez_eth_chip
    port map (
      rstn_i    => rstn_sys,
      clk_i   => clk_sys,
      rx_src_o => eb_snk_in,
      rx_src_i => eb_snk_out,
      tx_snk_i => eb_src_out,
      tx_snk_o => eb_src_in


      );

  debug : process
  begin
    wait until rst_n = '1';
    report "Base SDB @ "  & f_bits2string(c_top_sdb_address)  severity Note;
    while true loop
      wait until rising_edge(clk_sys); 
    end loop;  
  end process;


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


--EB SLAVE HERE

  top_msi_master_i(c_topm_ebs) <= cc_dummy_slave_out; -- Etherbone does not accept MSI !!!

  eb : eb_master_slave_wrapper
    generic map(
      g_with_master     => false,
      g_ebs_sdb_address => (x"00000000" & c_top_sdb_address))
    port map(
      clk_i           => clk_sys,
      nRst_i          => rstn_sys,
      snk_i           => eb_snk_in,
      snk_o           => eb_snk_out,
      src_o           => eb_src_out,
      src_i           => eb_src_in,
      ebs_cfg_slave_o => top_bus_master_i (c_tops_ebcfgs),
      ebs_cfg_slave_i => top_bus_master_o (c_tops_ebcfgs),
      ebs_wb_master_o => top_bus_slave_i (c_topm_ebs),
      ebs_wb_master_i => top_bus_slave_o (c_topm_ebs));


  minislave : entity work.wb_minislave
  port map (
    clk_i   => clk_sys,
    rst_n_i => rst_n,
    slave_i => top_bus_master_o(c_tops_minislave),
    slave_o => top_bus_master_i(c_tops_minislave)
  );



end architecture;



