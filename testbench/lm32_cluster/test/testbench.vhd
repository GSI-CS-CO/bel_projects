library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use std.textio.all;

library work;
--use work.monster_pkg.all;
--use work.gencores_pkg.all;
--use work.wrcore_pkg.all;
--use work.pll_pkg.all;
--use work.wr_fabric_pkg.all;
use work.wishbone_pkg.all;
--use work.eca_pkg.all;
--use work.eca_internals_pkg.eca_wr_time;
--use work.eca_tap_pkg.all;
--use work.tlu_pkg.all;
--use work.pcie_wb_pkg.all;
--use work.wr_altera_pkg.all;
--use work.etherbone_pkg.all;
--use work.scu_bus_pkg.all;
--use work.altera_flash_pkg.all;
--use work.altera_networks_pkg.all;
--use work.altera_lvds_pkg.all;
--use work.build_id_pkg.all;
--use work.watchdog_pkg.all;
use work.mbox_pkg.all;
--use work.oled_display_pkg.all;
--use work.lpc_uart_pkg.all;
--use work.wb_irq_pkg.all;
use work.ftm_pkg.all;
use work.ez_usb_pkg.all;
use work.simbridge_pkg.all;
use work.wb_arria_reset_pkg.all;
--use work.xvme64x_pack.all;
--use work.VME_Buffer_pack.all;
--use work.wb_mil_scu_pkg.all;
--use work.wr_serialtimestamp_pkg.all;
--use work.wb_ssd1325_serial_driver_pkg.all;
--use work.wb_nau8811_audio_driver_pkg.all;
--use work.fg_quad_pkg.all;
--use work.cfi_flash_pkg.all;
--use work.psram_pkg.all;
--use work.wb_serdes_clk_gen_pkg.all;
--use work.io_control_pkg.all;
--use work.wb_pmc_host_bridge_pkg.all;
--use work.wb_temp_sense_pkg.all;
--use work.ddr3_wrapper_pkg.all;
--use work.endpoint_pkg.all;
--use work.cpri_phy_reconf_pkg.all;
--use work.beam_dump_pkg.all;


entity testbench is
  generic(
    g_lm32_cores : natural := 1;
    g_lm32_ramsizes : natural := 16#20000#;
    g_simulation : boolean := true;
    g_en_simbridge : boolean := true;
    g_lm32_are_ftm : boolean := false;
    g_delay_diagnostics : boolean := false
    );
end entity;

architecture simulation of testbench is

  constant c_initf_name   : string := "firmware/firmware.mif";
  constant c_profile_name  : string := "medium_icache_debug";


  constant c_uart_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"000000000000000f",
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"1ac4ca35",
    version       => x"00000001",
    date          => x"20221201",
    name          => "uart-output        ")));

  constant c_zero_master : t_wishbone_master_out := (
    cyc => '0',
    stb => '0',
    adr => (others => '0'),
    sel => (others => '0'),
    we  => '0',
    dat => (others => '0'));

    signal clk_20m_vcxo_i    : std_logic := '1';  -- 20MHz VCXO clock
    signal clk_125m_pllref_i : std_logic := '1';  -- 125 MHz PLL reference
    signal clk_125m_local_i  : std_logic := '1';  -- local clk from 125Mhz oszillator

    signal clk_sys : std_logic := '1';
    signal rstn_sys: std_logic := '0';

    signal clk_ref : std_logic := '1';
    signal rstn_ref: std_logic := '0';

    signal rst_lm32 : std_logic_vector(g_lm32_cores-1 downto 0) := (others => '0');

    signal s_time : unsigned(63 downto 0) := (others => '0');


  ----------------------------------------------------------------------------------
  -- GSI Top Crossbar Masters ------------------------------------------------------
  ----------------------------------------------------------------------------------

  type top_my_masters is (
      --topm_usb
      topm_simbridge
    );
  constant c_top_my_masters : natural := top_my_masters'pos(top_my_masters'right)+1;

  constant c_top_layout_my_masters : t_sdb_record_array(c_top_my_masters-1 downto 0) :=
   (
    --top_my_masters'pos(topm_usb)     => f_sdb_auto_msi(c_usb_msi,     g_en_usb)
    top_my_masters'pos(topm_simbridge)=>f_sdb_auto_msi(c_simbridge_msi, g_simulation and g_en_simbridge)
    );

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
  -- GSI Dev Crossbar Masters ------------------------------------------------------
  ----------------------------------------------------------------------------------
  constant c_dev_masters         : natural := 1;
  constant c_devm_top            : natural := 0;

  constant c_dev_layout_req_masters : t_sdb_record_array(c_dev_masters-1 downto 0) :=
    (c_devm_top => f_sdb_auto_msi(c_top_bridge_msi, true));
  constant c_dev_layout_masters : t_sdb_record_array := f_sdb_auto_layout(c_dev_layout_req_masters);
  constant c_dev_bridge_msi : t_sdb_msi := f_xwb_msi_layout_sdb(c_dev_layout_masters);

  signal dev_bus_slave_i  : t_wishbone_slave_in_array  (c_dev_masters-1 downto 0);
  signal dev_bus_slave_o  : t_wishbone_slave_out_array (c_dev_masters-1 downto 0);
  signal dev_msi_master_i : t_wishbone_master_in_array (c_dev_masters-1 downto 0);
  signal dev_msi_master_o : t_wishbone_master_out_array(c_dev_masters-1 downto 0);

  attribute keep                  : boolean;
  signal sdb_dummy_top            : std_logic := '0';
  signal sdb_dummy_dev            : std_logic := '0';
  attribute keep of sdb_dummy_top : signal is true;
  attribute keep of sdb_dummy_dev : signal is true;
 
  ----------------------------------------------------------------------------------
  -- GSI Dev Crossbar Slaves -------------------------------------------------------
  ----------------------------------------------------------------------------------

  type dev_slaves is (
    devs_reset,
    devs_ftm_cluster,
    devs_uart_output
  );
  constant c_dev_slaves          : natural := dev_slaves'pos(dev_slaves'right)+1;
  
  constant c_wrcore_bridge_sdb : t_sdb_bridge := f_xwb_bridge_manual_sdb(x"0003ffff", x"00030000");
  constant c_ftm_slaves : t_sdb_bridge := f_cluster_bridge(c_dev_bridge_msi, g_lm32_cores, g_lm32_ramsizes/4, g_lm32_are_ftm, g_delay_diagnostics);

  constant c_dev_layout_req_slaves : t_sdb_record_array(c_dev_slaves-1 downto 0) :=
   (
    dev_slaves'pos(devs_reset)       => f_sdb_auto_device(c_arria_reset, true),
    dev_slaves'pos(devs_uart_output) => f_sdb_auto_device(c_uart_sdb,    true),
    dev_slaves'pos(devs_ftm_cluster) => f_sdb_auto_bridge(c_ftm_slaves,  true)
  );
  constant c_dev_layout      : t_sdb_record_array := f_sdb_auto_layout(c_dev_layout_req_masters, c_dev_layout_req_slaves);
  constant c_dev_sdb_address : t_wishbone_address := f_sdb_auto_sdb   (c_dev_layout_req_masters, c_dev_layout_req_slaves);
  constant c_dev_bridge_sdb  : t_sdb_bridge       := f_xwb_bridge_layout_sdb(true, c_dev_layout, c_dev_sdb_address);

  signal dev_msi_slave_i  : t_wishbone_slave_in_array  (c_dev_slaves-1 downto 0) := (others => c_zero_master);
  signal dev_msi_slave_o  : t_wishbone_slave_out_array (c_dev_slaves-1 downto 0);
  signal dev_bus_master_i : t_wishbone_master_in_array (c_dev_slaves-1 downto 0);
  signal dev_bus_master_o : t_wishbone_master_out_array(c_dev_slaves-1 downto 0);


  ----------------------------------------------------------------------------------
  -- GSI Top Crossbar Slaves -------------------------------------------------------
  ----------------------------------------------------------------------------------

  -- Only put a slave here if it has critical performance requirements!
  type top_slaves is (
    tops_mbox,
    tops_dev
    );
  constant c_top_slaves        : natural := top_slaves'pos(top_slaves'right)+1;

  constant c_top_layout_req_slaves : t_sdb_record_array(c_top_slaves-1 downto 0) :=
   (
    top_slaves'pos(tops_mbox)         => f_sdb_auto_device(c_mbox_sdb,                        true),
    top_slaves'pos(tops_dev)          => f_sdb_auto_bridge(c_dev_bridge_sdb,                  true)
   );

  constant c_top_layout      : t_sdb_record_array := f_sdb_auto_layout(c_top_layout_req_masters, c_top_layout_req_slaves);
  constant c_top_sdb_address : t_wishbone_address := f_sdb_auto_sdb   (c_top_layout_req_masters, c_top_layout_req_slaves);
  constant c_top_bridge_sdb  : t_sdb_bridge       := f_xwb_bridge_layout_sdb(true, c_top_layout, c_top_sdb_address);

  signal top_msi_slave_i  : t_wishbone_slave_in_array  (c_top_slaves-1 downto 0) := (others => c_zero_master);
  signal top_msi_slave_o  : t_wishbone_slave_out_array (c_top_slaves-1 downto 0);
  signal top_bus_master_i : t_wishbone_master_in_array (c_top_slaves-1 downto 0);
  signal top_bus_master_o : t_wishbone_master_out_array(c_top_slaves-1 downto 0);

  ----------------------------------------------------------------------------------


  function f_string_list_repeat(s : string; times : natural)
  return string is
    variable i : natural := 0;
   constant delimeter : string := ";";
   constant str : string := s & delimeter;
    variable res : string(1 to str'length*times);
  begin
    for i in 0 to times-1 loop
      res(i*str'length+1 to (i+1)*str'length) := str;
    end loop;
    return res;
  end f_string_list_repeat;

  signal fifoadr : std_logic_vector(1 downto 0) := (others => '0');
  signal fulln   : std_logic := '0';
  signal sloen   : std_logic := '0';
  signal emptyn  : std_logic := '0';
  signal slrdn   : std_logic := '0';
  signal slwrn   : std_logic := '0';
  signal pktendn : std_logic := '0';
  signal ebcyc   : std_logic := '0';
  signal readyn  : std_logic := '0';
  signal fd_io   : std_logic_vector(7 downto 0);
  signal fd_o    : std_logic_vector(7 downto 0);
  signal fd_oen   : std_logic := '0';
  signal usb_rstn: std_logic := '0';
begin
  
  clk_sys <= not clk_sys after 8 ns; -- 62.5 MHz
  rstn_sys <= '1' after 100 ns;

  clk_ref <= not clk_ref after 4 ns; -- 125 MHz
  rstn_ref <= '1' after 100 ns;

  process
  begin

    wait until rising_edge(clk_ref);
    s_time <= s_time + 8;
  end process;

  simbridge : entity work.simbridge_chopped
    generic map(
        g_sdb_address => c_top_sdb_address
      )
    port map( 
      clk_i     => clk_sys,
      rstn_i    => rstn_sys,
      master_i  => top_bus_slave_o(top_my_masters'pos(topm_simbridge)),
      master_o  => top_bus_slave_i(top_my_masters'pos(topm_simbridge)),
      msi_slave_i => top_msi_master_o(top_my_masters'pos(topm_simbridge)),
      msi_slave_o => top_msi_master_i(top_my_masters'pos(topm_simbridge))
      );


  top_bar : xwb_sdb_crossbar
    generic map(
      g_num_masters => c_top_masters,
      g_num_slaves  => c_top_slaves,
      g_registered  => true,
      g_wraparound  => true,
      g_sdb_wb_mode => PIPELINED,
      g_verbose     => true,
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


  dev_bar : xwb_sdb_crossbar
    generic map(
      g_num_masters => c_dev_masters,
      g_num_slaves  => c_dev_slaves,
      g_registered  => true,
      g_wraparound  => true,
      g_sdb_wb_mode => PIPELINED,
      g_verbose     => true,
      g_layout      => c_dev_layout,
      g_sdb_addr    => c_dev_sdb_address)
    port map(
      clk_sys_i     => clk_sys,
      rst_n_i       => rstn_sys,
      slave_i       => dev_bus_slave_i,
      slave_o       => dev_bus_slave_o,
      msi_master_i  => dev_msi_master_i,
      msi_master_o  => dev_msi_master_o,
      master_i      => dev_bus_master_i,
      master_o      => dev_bus_master_o,
      msi_slave_i   => dev_msi_slave_i,
      msi_slave_o   => dev_msi_slave_o);

  top2dev_bus : xwb_register_link
    generic map(
      g_wb_adapter  => false)
    port map(
      clk_sys_i     => clk_sys,
      rst_n_i       => rstn_sys,
      slave_i       => top_bus_master_o(top_slaves'pos(tops_dev)),
      slave_o       => top_bus_master_i(top_slaves'pos(tops_dev)),
      master_i      => dev_bus_slave_o (c_devm_top),
      master_o      => dev_bus_slave_i (c_devm_top));

  dev2top_msi : xwb_register_link
    generic map(
      g_wb_adapter  => false)
    port map(
      clk_sys_i     => clk_sys,
      rst_n_i       => rstn_sys,
      slave_i       => dev_msi_master_o(c_devm_top),
      slave_o       => dev_msi_master_i(c_devm_top),
      master_i      => top_msi_slave_o (top_slaves'pos(tops_dev)),
      master_o      => top_msi_slave_i (top_slaves'pos(tops_dev)));



  mailbox : mbox
    port map(
      clk_i        => clk_sys,
      rst_n_i      => rstn_sys,
      bus_slave_i  => top_bus_master_o(top_slaves'pos(tops_mbox)),
      bus_slave_o  => top_bus_master_i(top_slaves'pos(tops_mbox)),
      msi_master_o => top_msi_slave_i (top_slaves'pos(tops_mbox)),
      msi_master_i => top_msi_slave_o (top_slaves'pos(tops_mbox)));


  wb_reset : wb_arria_reset
    generic map(
      arria_family => "Arria II",
      rst_channels => g_lm32_cores,
      clk_in_hz    => 62_500_000,
      en_wd_tmr    => false)
    port map(
      clk_sys_i  => clk_sys,
      rstn_sys_i => rstn_sys,
      clk_upd_i  => '0',
      rstn_upd_i => '0',
      hw_version => (others => '0'),
      slave_o    => dev_bus_master_i(dev_slaves'pos(devs_reset)),
      slave_i    => dev_bus_master_o(dev_slaves'pos(devs_reset)),
      rstn_o     => rst_lm32);

  lm32 : ftm_lm32_cluster
    generic map(
      g_is_dm               => false,
      g_delay_diagnostics   => false,
      g_cores               => g_lm32_cores,
      g_ram_per_core        => g_lm32_ramsizes/4,
      g_world_bridge_sdb    => c_top_bridge_sdb,
      g_clu_msi_sdb         => c_top_bridge_msi,
      g_init_files          => f_string_list_repeat(c_initf_name, g_lm32_cores),
      g_en_timer            => true,
      g_profiles            => f_string_list_repeat(c_profile_name, g_lm32_cores))
    port map(
      clk_ref_i          => clk_ref,
      rst_ref_n_i        => rstn_ref,
      clk_sys_i          => clk_sys,
      rst_sys_n_i        => rstn_sys,
      rst_lm32_n_i       => rst_lm32,
      tm_tai8ns_i        => std_logic_vector(s_time),
      wr_lock_i          => '1',
      lm32_masters_o     => top_bus_slave_i(top_bus_slave_i'high downto c_top_my_masters),
      lm32_masters_i     => top_bus_slave_o(top_bus_slave_o'high downto c_top_my_masters),
      lm32_msi_slaves_o  => top_msi_master_i(top_msi_master_i'high downto c_top_my_masters),
      lm32_msi_slaves_i  => top_msi_master_o(top_msi_master_o'high downto c_top_my_masters),
      clu_slave_o        => dev_bus_master_i(dev_slaves'pos(devs_ftm_cluster)),
      clu_slave_i        => dev_bus_master_o(dev_slaves'pos(devs_ftm_cluster)),
      clu_msi_master_o   => dev_msi_slave_i(dev_slaves'pos(devs_ftm_cluster)),
      clu_msi_master_i   => dev_msi_slave_o(dev_slaves'pos(devs_ftm_cluster)),
      dm_prioq_master_o  => open,
      dm_prioq_master_i  => (ack => '0', err => '0', stall => '0', rty => '0', dat => (others=>'0')));


    uart_output: process
      FILE stdout : text;-- is "cpu_output.txt";
      variable cpu_output : line;
    begin
      file_open(stdout, "uart_output.txt", write_mode);
      while true loop
        wait until rising_edge(clk_sys);

        dev_bus_master_i(dev_slaves'pos(devs_uart_output)).ack <= '0';
        dev_bus_master_i(dev_slaves'pos(devs_uart_output)).err <= '0';
        dev_bus_master_i(dev_slaves'pos(devs_uart_output)).rty <= '0';
        dev_bus_master_i(dev_slaves'pos(devs_uart_output)).stall <= '0';

        if  dev_bus_master_o(dev_slaves'pos(devs_uart_output)).cyc = '1' and
            dev_bus_master_o(dev_slaves'pos(devs_uart_output)).stb = '1' and
            dev_bus_master_o(dev_slaves'pos(devs_uart_output)).we  = '1' then
          dev_bus_master_i(dev_slaves'pos(devs_uart_output)).ack <= '1';
          --report "output => " & character'val(to_integer(unsigned(
          --  dev_bus_master_o(dev_slaves'pos(devs_uart_output)).dat(7 downto 0)
          --))); 

          if to_integer(unsigned(dev_bus_master_o(dev_slaves'pos(devs_uart_output)).dat(7 downto 0))) = 10 then
            writeline(stdout,cpu_output);
            file_close(stdout);
            file_open(stdout, "uart_output.txt", append_mode);      
            --flush(stdout);
          else
            write(cpu_output, character'val(to_integer(unsigned(dev_bus_master_o(dev_slaves'pos(devs_uart_output)).dat(7 downto 0)))));
            file_close(stdout);
            file_open(stdout, "uart_output.txt", append_mode);      
            --flush(stdout);
          end if;


        end if;
      end loop;
    end process;

end architecture;



