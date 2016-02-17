library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.gencores_pkg.all;
use work.wb_scu_reg_pkg.all;
use work.remote_update_pkg.all;

entity housekeeping is
  generic ( 
            Base_addr:  unsigned(15 downto 0)
          );
  port (
        clk_sys:            in std_logic;
        clk_update:         in std_logic;
        clk_flash:          in std_logic;
        rstn_sys:           in std_logic;
        rstn_update:        in std_logic;
        rstn_flash:         in std_logic;

        ADR_from_SCUB_LA:   in std_logic_vector(15 downto 0);
        Data_from_SCUB_LA:  in std_logic_vector(15 downto 0);
        Ext_Adr_Val:        in std_logic;
        Ext_Rd_active:      in std_logic;
        Ext_Wr_active:      in std_logic;
        user_rd_active:     out std_logic;
        Data_to_SCUB:       out std_logic_vector(15 downto 0);
        Dtack_to_SCUB:      out std_logic;
        
        owr_pwren_o:        out std_logic_vector(1 downto 0);
        owr_en_o:           out std_logic_vector(1 downto 0);
        owr_i:              in std_logic_vector(1 downto 0);
  
        debug_serial_o:     out std_logic;
        debug_serial_i:     in  std_logic 
  );
end entity;


architecture housekeeping_arch of housekeeping is

 constant c_xwb_owm : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"00000000000000ff",
    product       => (
    vendor_id     => x"000000000000CE42", -- CERN
    device_id     => x"779c5443",
    version       => x"00000001",
    date          => x"20120603",
    name          => "WR-Periph-1Wire    ")));

  constant c_xwb_uart : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"00000000000000ff",
    product       => (
    vendor_id     => x"000000000000CE42", -- CERN
    device_id     => x"e2d13d04",
    version       => x"00000001",
    date          => x"20120603",
    name          => "WR-Periph-UART     ")));

  signal lm32_interrupt:   std_logic_vector(31 downto 0);
  signal lm32_rstn:        std_logic;

  -- Top crossbar layout
  constant c_slaves     : natural := 6;
  constant c_masters    : natural := 2;
  constant c_dpram_size : natural := 32768; -- in 32-bit words (64KB)
  constant c_layout_req     : t_sdb_record_array(c_slaves-1 downto 0) :=
   (0 => f_sdb_embed_device(f_xwb_dpram(c_dpram_size), x"00000000"),
    1 => f_sdb_embed_device(c_xwb_owm,                 x"00020000"),
    2 => f_sdb_embed_device(c_xwb_uart,                x"00020100"),
    3 => f_sdb_embed_device(c_xwb_scu_reg,             x"00020200"),
    4 => f_sdb_embed_device(c_wb_rem_upd_sdb,          x"00020500"),
    5 => f_sdb_embed_device(c_wb_asmi_sdb,             x"10000000"));
  
  constant c_top_layout  : t_sdb_record_array(c_slaves-1 downto 0) := f_sdb_auto_layout(c_layout_req);
  constant c_sdb_address : t_wishbone_address := x"3FFFE000";

  signal cbar_slave_i : t_wishbone_slave_in_array (c_masters-1 downto 0);
  signal cbar_slave_o : t_wishbone_slave_out_array(c_masters-1 downto 0);
  signal cbar_master_i : t_wishbone_master_in_array(c_slaves-1 downto 0);
  signal cbar_master_o : t_wishbone_master_out_array(c_slaves-1 downto 0);
  
  signal aru_i : t_wishbone_slave_in;
  signal aru_o : t_wishbone_slave_out;
  
  signal asmi_i : t_wishbone_slave_in;
  signal asmi_o : t_wishbone_slave_out;
  
  -- asmi interface, needed for pof check
  signal asmi_busy       :  std_logic;
  signal asmi_data_valid :  std_logic;
  signal asmi_dataout    :  std_logic_vector(7 downto 0);
  signal asmi_addr_ext   :  std_logic_vector(23 downto 0);
  signal asmi_rden_ext   :  std_logic;
  signal asmi_read_ext   :  std_logic;
  signal asmi_to_ext     :  std_logic;
  
  -- scu slave signals
  signal wb_reg_dtack:      std_logic;
  signal wb_reg_data:       std_logic_vector(15 downto 0);
  signal wb_reg_rd_active:  std_logic;
  
  signal info_rom_data:       std_logic_vector(15 downto 0);
  signal info_rom_dtack:      std_logic;
  signal info_rom_rd_active:  std_logic;
  

begin

  -- The top-most Wishbone B.4 crossbar
  interconnect : xwb_sdb_crossbar
   generic map(
     g_num_masters => c_masters,
     g_num_slaves => c_slaves,
     g_registered => true,
     g_wraparound => false, -- Should be true for nested buses
     g_layout => c_layout_req,
     g_sdb_addr => c_sdb_address)
   port map(
     clk_sys_i => clk_sys,
     rst_n_i => rstn_sys,
     -- Master connections (INTERCON is a slave)
     slave_i => cbar_slave_i,
     slave_o => cbar_slave_o,
     -- Slave connections (INTERCON is a master)
     master_i => cbar_master_i,
     master_o => cbar_master_o);

  -- The LM32 is master 0+1
  LM32 : xwb_lm32
    generic map(
      g_profile => "medium_icache_debug") -- Including JTAG and I-cache (no divide)
    port map(
      clk_sys_i => clk_sys,
      rst_n_i => rstn_sys,
      irq_i => lm32_interrupt,
      dwb_o => cbar_slave_i(0), -- Data bus
      dwb_i => cbar_slave_o(0),
      iwb_o => cbar_slave_i(1), -- Instruction bus
      iwb_i => cbar_slave_o(1));
  -- The other 31 interrupt pins are unconnected
  lm32_interrupt(31 downto 1) <= (others => '0');

  -- WB Slave 0 is the RAM
  ram : xwb_dpram
    generic map(
      g_size => c_dpram_size,
      g_slave1_interface_mode => PIPELINED,
      g_slave2_interface_mode => PIPELINED,
      g_slave1_granularity => BYTE,
      g_slave2_granularity => WORD,
      g_init_file => "housekeeping.mif")
    port map(
      clk_sys_i => clk_sys,
      rst_n_i => rstn_sys,
      -- First port connected to the crossbar
      slave1_i => cbar_master_o(0),
      slave1_o => cbar_master_i(0),
      -- Second port disconnected
      slave2_i => cc_dummy_slave_in, -- CYC always low
      slave2_o => open);
  
  --------------------------------------
  -- 1-WIRE
  --------------------------------------
  ONEWIRE : xwb_onewire_master
    generic map(
      g_interface_mode      => PIPELINED,
      g_address_granularity => BYTE,
      g_num_ports           => 2,
      g_ow_btp_normal       => "5.0",
      g_ow_btp_overdrive    => "1.0"
      )
    port map(
      clk_sys_i   => clk_sys,
      rst_n_i     => rstn_sys,

      -- Wishbone
      slave_i     => cbar_master_o(1),
      slave_o     => cbar_master_i(1),
      desc_o      => open,

      owr_pwren_o => owr_pwren_o,
      owr_en_o    => owr_en_o,
      owr_i       => owr_i
      );


  --------------------------------------
  -- UART
  --------------------------------------
  UART : xwb_simple_uart
    generic map(
      g_with_virtual_uart   => false,
      g_with_physical_uart  => true,
      g_interface_mode      => PIPELINED,
      g_address_granularity => BYTE
      )
    port map(
      clk_sys_i => clk_sys,
      rst_n_i   => rstn_sys,

      -- Wishbone
      slave_i => cbar_master_o(2),
      slave_o => cbar_master_i(2),
      desc_o  => open,

      uart_rxd_i => '0',
      uart_txd_o => debug_serial_o
      );

  -------------------------------------
  -- Interface to SCU Bus Slave
  -------------------------------------
  SCU_WB_Reg: wb_scu_reg
    generic map (
      Base_addr => Base_addr,
      size => 160,
      g_init_file => "")
    port map (
      clk_sys_i => clk_sys,
      rst_n_i => rstn_sys,

      -- Wishbone
      slave_i => cbar_master_o(3),
      slave_o => cbar_master_i(3),

      Adr_from_SCUB_LA  => ADR_from_SCUB_LA,
      Data_from_SCUB_LA => Data_from_SCUB_LA,
      Ext_Adr_Val       => Ext_Adr_Val,
      Ext_Rd_active     => Ext_Rd_active,
      Ext_Wr_active     => Ext_Wr_active,
      user_rd_active    => wb_reg_rd_active,
      Data_to_SCUB      => wb_reg_data,
      Dtack_to_SCUB     => wb_reg_dtack);
      
  -------------------------------------
  -- Interface to SCU Bus Slave
  -------------------------------------
  info_rom: wb_scu_reg
    generic map (
      Base_addr => Base_addr + 160,
      size => 256,
      g_init_file => "build_id.mif")
    port map (
      clk_sys_i => clk_sys,
      rst_n_i => rstn_sys,

      -- Wishbone
      slave_i => cc_dummy_slave_in,
      slave_o => open,

      Adr_from_SCUB_LA  => ADR_from_SCUB_LA,
      Data_from_SCUB_LA => Data_from_SCUB_LA,
      Ext_Adr_Val       => Ext_Adr_Val,
      Ext_Rd_active     => Ext_Rd_active,
      Ext_Wr_active     => Ext_Wr_active,
      user_rd_active    => info_rom_rd_active,
      Data_to_SCUB      => info_rom_data,
      Dtack_to_SCUB     => info_rom_dtack);
      
  --------------------------------------------
  -- clock crossing from sys clk to clk_10Mhz
  --------------------------------------------
   cross_systoaru : xwb_clock_crossing
    generic map ( g_size => 32)
    port map(
      -- Slave control port
      slave_clk_i    => clk_sys,
      slave_rst_n_i  => rstn_sys,
      slave_i        => cbar_master_o(4),
      slave_o        => cbar_master_i(4),
      -- Master reader port
      master_clk_i   => clk_update,
      master_rst_n_i => rstn_update,
      master_i       => aru_o,
      master_o       => aru_i);
  
  
  -----------------------------------------
  -- wb interface for altera remote update
  -----------------------------------------
  wb_aru: wb_remote_update
    port map (
      clk_sys_i => clk_update,
      rst_n_i   => rstn_update,
      
      slave_i      =>  aru_i,
      slave_o      =>  aru_o,
      
      -- asmi interface, needed for pof check
      asmi_busy       => asmi_busy,
      asmi_data_valid => asmi_data_valid,
      asmi_dataout    => asmi_dataout,
      asmi_addr       => asmi_addr_ext,
      asmi_rden       => asmi_rden_ext,
      asmi_read       => asmi_read_ext,
      asmi_to_aru     => asmi_to_ext);
      
  --------------------------------------------
  -- clock crossing from sys clk to clk_25Mhz
  --------------------------------------------
   cross_systoasmi : xwb_clock_crossing
    generic map ( g_size => 32)
    port map(
      -- Slave control port
      slave_clk_i    => clk_sys,
      slave_rst_n_i  => rstn_sys,
      slave_i        => cbar_master_o(5),
      slave_o        => cbar_master_i(5),
      -- Master reader port
      master_clk_i   => clk_flash,
      master_rst_n_i => rstn_flash,
      master_i       => asmi_o,
      master_o       => asmi_i);
  
  
  -----------------------------------------
  -- wb interface for altera remote update
  -----------------------------------------
  asmi: wb_asmi
    generic map ( PAGESIZE => 256 )
    port map (
      clk_flash_i => clk_flash,
      rst_n_i   => rstn_flash,
      
      slave_i      =>  asmi_i,
      slave_o      =>  asmi_o,
      
      -- asmi interface, needed for pof check
      asmi_busy         => asmi_busy,
      asmi_data_valid   => asmi_data_valid,
      asmi_dataout      => asmi_dataout,
      asmi_addr_ext     => asmi_addr_ext,
      asmi_rden_ext     => asmi_rden_ext,
      asmi_read_ext     => asmi_read_ext,
      -- needed for multiplexing
      asmi_to_ext       => asmi_to_ext);  
      
  
  Data_to_SCUB <= wb_reg_data when wb_reg_rd_active = '1' else
                  info_rom_data when info_rom_rd_active = '1' else
                  (others => '0');
                  
  user_rd_active <= wb_reg_rd_active or info_rom_rd_active;
  
  Dtack_to_SCUB <= wb_reg_dtack or info_rom_dtack;
  

end architecture;

