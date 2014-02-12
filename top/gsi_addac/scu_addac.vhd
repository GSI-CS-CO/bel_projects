library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;

library work;
use work.wishbone_pkg.all;
use work.gencores_pkg.all;
use work.scu_bus_slave_pkg.all;
use work.aux_functions_pkg.all;
use work.adc_pkg.all;
use work.wb_scu_reg_pkg.all;
use work.dac714_pkg.all;
use work.fg_quad_pkg.all;

entity scu_addac is
  port (
    -------------------------------------------------------------------------------------------------------------------
    CLK_FPGA: in std_logic;
    
    --------- Parallel SCU-Bus-Signale --------------------------------------------------------------------------------
    A_A: in std_logic_vector(15 downto 0); -- SCU-Adressbus
    A_nADR_EN: out std_logic := '0'; -- '0' => externe Adresstreiber des Slaves aktiv
    A_nADR_FROM_SCUB: out std_logic := '0'; -- '0' => externe Adresstreiber-Richtung: SCU-Bus nach Slave
    A_D: inout std_logic_vector(15 downto 0); -- SCU-Datenbus
    A_nDS: in std_logic; -- Data-Strobe vom Master gertieben
    A_RnW: in std_logic; -- Schreib/Lese-Signal vom Master getrieben, '0' => lesen
    A_nSel_Ext_Data_Drv: out std_logic; -- '0' => externe Datentreiber des Slaves aktiv
    A_Ext_Data_RD: out std_logic; -- '0' => externe Datentreiber-Richtung: SCU-Bus nach
                                                                -- Slave (besser default 0, oder Treiber A/B tauschen)
                                                                -- SCU-Bus nach Slave (besser default 0, oder Treiber A/B tauschen)
    A_nDtack: out std_logic; -- Data-Acknowlege null aktiv, '0' => aktiviert externen
                                                                -- Opendrain-Treiber
    A_nSRQ: out std_logic; -- Service-Request null aktiv, '0' => aktiviert externen
                                                                -- Opendrain-Treiber
    A_nBoardSel: in std_logic; -- '0' => Master aktiviert diesen Slave
    A_nEvent_Str: in std_logic; -- '0' => Master sigalisiert Timing-Zyklus
    A_SysClock: in std_logic; -- Clock vom Master getrieben.
    A_Spare0: in std_logic; -- vom Master getrieben
    A_Spare1: in std_logic; -- vom Master getrieben
    A_nReset: in std_logic; -- Reset (aktiv '0'), vom Master getrieben

    ------------ ADC Signals ------------------------------------------------------------------------------------------
    ADC_DB: inout std_logic_vector(15 downto 0);
    ADC_CONVST_A: buffer std_logic;
    ADC_CONVST_B: buffer std_logic;
    nADC_CS: buffer std_logic;
    nADC_RD_SCLK: buffer std_logic;
    ADC_BUSY: in std_logic;
    ADC_RESET: buffer std_logic;
    ADC_OS: buffer std_logic_vector(2 downto 0);
    nADC_PAR_SER_SEL: buffer std_logic := '0';
    ADC_Range: buffer std_logic;
    ADC_FRSTDATA: in std_logic;
    EXT_TRIG_ADC: in std_logic;
    ------------ ADC Diagnostic ---------------------------------------------------------------------------------------
    A_ADC_DAC_SEL: in std_logic_vector(3 downto 0);

    ------------ DAC Signals ------------------------------------------------------------------------------------------
    DAC1_SDI: buffer std_logic; -- is connected to DAC1-SDI
    DAC1_SDO: buffer std_logic;
    nDAC1_CLK: buffer std_logic; -- spi-clock of DAC1
    nDAC1_CLR: buffer std_logic; -- '0' set DAC1 to zero (pulse width min 200 ns)
    nDAC1_A0: buffer std_logic; -- '0' enable shift of internal shift register of DAC1
    nDAC1_A1: buffer std_logic; -- '0' copy shift register to output latch of DAC1
    DAC2_SDI: buffer std_logic; -- is connected to DAC2-SDI
    DAC2_SDO: buffer std_logic;
    nDAC2_CLK: buffer std_logic; -- spi-clock of DAC2
    nDAC2_CLR: buffer std_logic; -- '0' set DAC2 to zero (pulse width min 200 ns)
    nDAC2_A0: buffer std_logic; -- '0' enable shift of internal shift register of DAC2
    nDAC2_A1: buffer std_logic; -- '0' copy shift register to output latch of DAC2
    EXT_TRIG_DAC: in std_logic;
    A_NLED_TRIG_DAC: out std_logic;
    
    ------------ IO-Port-Signale --------------------------------------------------------------------------------------
    a_io_7_0_tx: out std_logic; -- '1' = external io(7..0)-buffer set to output.
    a_io_15_8_tx: out std_logic; -- '1' = external io(15..8)-buffer set to output
    a_io_23_16_tx: out std_logic; -- '1' = external io(23..16)-buffer set to output
    a_io_31_24_tx: out std_logic; -- '1' = external io(31..24)-buffer set to output
    a_ext_io_7_0_dis: out std_logic; -- '1' = disable external io(7..0)-buffer.
    a_ext_io_15_8_dis: out std_logic; -- '1' = disable external io(15..8)-buffer.
    a_ext_io_23_16_dis: out std_logic; -- '1' = disable external io(23..16)-buffer.
    a_ext_io_31_24_dis: out std_logic; -- '1' = disable external io(31..24)-buffer.
    a_io: inout std_logic_vector(31 downto 0); -- select and set direction only in 8-bit partitions
    
    ------------ Logic analyser Signals -------------------------------------------------------------------------------
    A_SEL: in std_logic_vector(3 downto 0); -- use to select sources for the logic analyser ports
    A_TA: out std_logic_vector(15 downto 0); -- test port a
    A_TB: inout std_logic_vector(15 downto 0); -- test port b
    --A_TB0: out std_logic;
    --A_TB1: in std_logic;
    --A_TB2: out std_logic := '1';
    TP: out std_logic_vector(2 downto 1); -- test points

    A_nState_LED: out std_logic_vector(2 downto 0); -- ..LED(2) = R/W, ..LED(1) = Dtack, ..LED(0) = Sel
    A_nLED: out std_logic_vector(15 downto 0);
    A_NLED_TRIG_ADC: out std_logic;
    
    HW_REV: in std_logic_vector(3 downto 0);
    A_MODE_SEL: in std_logic_vector(1 downto 0);
    A_OneWire: inout std_logic;
    A_OneWire_EEPROM: inout std_logic;
    
    NDIFF_IN_EN: buffer std_logic -- enables diff driver for ADC channels 3-8
    
    
    );
end entity;



architecture scu_addac_arch of scu_addac is


constant scu_adda1_id: integer range 16#0210# to 16#021F# := 16#0210#;

constant clk_sys_in_Hz: integer := 125_000_000;
constant dac_spi_clk_in_hz: integer := 10_000_000;


component IO_4x8
  generic (
      Base_addr: unsigned(15 downto 0));
  port (
    Adr_from_SCUB_LA: in std_logic_vector(15 downto 0); -- latched address from SCU_Bus
    Data_from_SCUB_LA: in std_logic_vector(15 downto 0); -- latched data from SCU_Bus
    Ext_Adr_Val: in std_logic; -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active: in std_logic; -- '1' => Rd-Cycle is active
    Ext_Wr_active: in std_logic; -- '1' => Wr-Cycle is active
    clk: in std_logic; -- should be the same clk, used by SCU_Bus_Slave
    nReset: in std_logic := '1';
    io: inout std_logic_vector(31 downto 0); -- select and set direction only in 8-bit partitions
    io_7_0_tx: out std_logic; -- '1' = external io(7..0)-buffer set to output.
    ext_io_7_0_dis: out std_logic; -- '1' = disable external io(7..0)-buffer.
    io_15_8_tx: out std_logic; -- '1' = external io(15..8)-buffer set to output
    ext_io_15_8_dis: out std_logic; -- '1' = disable external io(15..8)-buffer.
    io_23_16_tx: out std_logic; -- '1' = external io(23..16)-buffer set to output.
    ext_io_23_16_dis: out std_logic; -- '1' = disable external io(23..16)-buffer.
    io_31_24_tx: out std_logic; -- '1' = external io(31..24)-buffer set to output
    ext_io_31_24_dis: out std_logic; -- '1' = disable external io(31..24)-buffer.
    user_rd_active: out std_logic; -- '1' = read data available at 'Data_to_SCUB'-output
    Data_to_SCUB: out std_logic_vector(15 downto 0); -- connect read sources to SCUB-Macro
    Dtack_to_SCUB: out std_logic -- connect Dtack to SCUB-Macro
    );
  end component IO_4x8;

 
component flash_loader_v01
  port (
    noe_in: in std_logic
    );
  end component;


component adda_pll
  PORT
  (
    inclk0: in std_logic := '0';
    c0: out std_logic;
    c1: out std_logic;
    locked: out std_logic
  );
end component;

constant c_xwb_owm : t_sdb_device := (
    abi_class => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian => c_sdb_endian_big,
    wbd_width => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first => x"0000000000000000",
    addr_last => x"00000000000000ff",
    product => (
    vendor_id => x"000000000000CE42", -- CERN
    device_id => x"779c5443",
    version => x"00000001",
    date => x"20120603",
    name => "WR-Periph-1Wire    ")));
        
constant c_xwb_uart : t_sdb_device := (
    abi_class => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian => c_sdb_endian_big,
    wbd_width => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first => x"0000000000000000",
    addr_last => x"00000000000000ff",
    product => (
    vendor_id => x"000000000000CE42", -- CERN
    device_id => x"e2d13d04",
    version => x"00000001",
    date => x"20120603",
    name => "WR-Periph-UART     ")));
  
  signal clk_sys, clk_cal, locked : std_logic;
  
  constant clk_in_hz: integer := 125_000_000;
  
  signal SCUB_SRQ:    std_logic;
  signal SCUB_Dtack:  std_logic;
  signal convst:      std_logic;
  signal rst:         std_logic;
  
  signal Dtack_to_SCUB: std_logic;

  signal io_port_Dtack_to_SCUB: std_logic;
  signal io_port_data_to_SCUB:  std_logic_vector(15 downto 0);
  signal io_port_rd_active:     std_logic;
  
  signal dac1_Dtack:        std_logic;
  signal dac1_data_to_SCUB: std_logic_vector(15 downto 0);
  signal dac1_rd_active:    std_logic;
  
  signal dac2_Dtack:        std_logic;
  signal dac2_data_to_SCUB: std_logic_vector(15 downto 0);
  signal dac2_rd_active:    std_logic;
  
  signal ADR_from_SCUB_LA:  std_logic_vector(15 downto 0);
  signal Data_from_SCUB_LA: std_logic_vector(15 downto 0);
  signal Ext_Adr_Val:       std_logic;
  signal Ext_Rd_active:     std_logic;
  signal Ext_Wr_active:     std_logic;
  signal Ext_Wr_fin_ovl:    std_logic;
  signal nPowerup_Res:      std_logic;
  
  signal adc_rd_active:     std_logic;
  signal adc_data_to_SCUB:  std_logic_vector(15 downto 0);
  signal adc_dtack:         std_logic;
  
  signal tmr_rd_active:     std_logic;
  signal tmr_data_to_SCUB:  std_logic_vector(15 downto 0);
  signal tmr_dtack:         std_logic;
  
  signal fg_1_dtack:        std_logic;
  signal fg_1_data_to_SCUB: std_logic_vector(15 downto 0);
  signal fg_1_rd_active:    std_logic;
  signal fg_1_sw:           std_logic_vector(31 downto 0);
  signal fg_1_strobe:       std_logic;
  signal fg_1_dreq:         std_logic;
  
  signal fg_2_dtack:        std_logic;
  signal fg_2_data_to_SCUB: std_logic_vector(15 downto 0);
  signal fg_2_rd_active:    std_logic;
  signal fg_2_sw:           std_logic_vector(31 downto 0);
  signal fg_2_strobe:       std_logic;
  signal fg_2_dreq:         std_logic;

  signal led_ena_cnt: std_logic;

  signal ADC_channel_1, ADC_channel_2, ADC_channel_3, ADC_channel_4: std_logic_vector(15 downto 0);
  signal ADC_channel_5, ADC_channel_6, ADC_channel_7, ADC_channel_8: std_logic_vector(15 downto 0);

  signal Data_to_SCUB: std_logic_vector(15 downto 0);
  
  signal reset_clks:       std_logic_vector(0 downto 0);
  signal reset_rstn:       std_logic_vector(0 downto 0);
  signal clk_sys_rstn:     std_logic;
  signal lm32_interrupt:   std_logic_vector(31 downto 0);
  signal lm32_rstn:        std_logic;
  
  -- Top crossbar layout
  constant c_slaves     : natural := 4;
  constant c_masters    : natural := 2;
  constant c_dpram_size : natural := 32768; -- in 32-bit words (64KB)
  constant c_layout     : t_sdb_record_array(c_slaves-1 downto 0) :=
   (0 => f_sdb_embed_device(f_xwb_dpram(c_dpram_size),  x"00000000"),
    1 => f_sdb_embed_device(c_xwb_owm,                  x"00100600"),
    2 => f_sdb_embed_device(c_xwb_uart,                 x"00100700"),
    3 => f_sdb_embed_device(c_xwb_scu_reg,              x"00100800"));
  constant c_sdb_address : t_wishbone_address := x"00100000";

  signal cbar_slave_i : t_wishbone_slave_in_array (c_masters-1 downto 0);
  signal cbar_slave_o : t_wishbone_slave_out_array(c_masters-1 downto 0);
  signal cbar_master_i : t_wishbone_master_in_array(c_slaves-1 downto 0);
  signal cbar_master_o : t_wishbone_master_out_array(c_slaves-1 downto 0);
  
  signal owr_pwren_o:   std_logic_vector(1 downto 0);
  signal owr_en_o:      std_logic_vector(1 downto 0);
  signal owr_i:         std_logic_vector(1 downto 0);
  
  signal wb_scu_rd_active:    std_logic;
  signal wb_scu_dtack:        std_logic;
  signal wb_scu_data_to_SCUB: std_logic_vector(15 downto 0);
  
  --signal irqcnt:  unsigned(12 downto 0);
  signal tmr_irq: std_logic;
  
  signal dac_1_ext_trig, dac_2_ext_trig: std_logic;
  
  constant c_led_cnt:				integer := integer(ceil(real(clk_sys_in_hz) / real(1_000_000_000) * real(125000000)));
	constant c_led_cnt_width:	integer := integer(floor(log2(real(c_led_cnt)))) + 2;
	signal  s_led_cnt:				unsigned( c_led_cnt_width-1 downto 0) := (others => '0');
  
  signal s_led_en:      std_logic;
  signal s_test_vector: std_logic_vector(15 downto 0) := x"8000";

  begin
    
fl : flash_loader_v01
  port map (noe_in => '0');

  -- Obtain core clocking
adda_pll_1: adda_pll -- Altera megafunction
  port map (
    inclk0  => CLK_FPGA,  -- 125Mhz oscillator from board
    c0      => clk_sys,   -- 125MHz system clk
    c1      => clk_cal,   -- 50Mhz calibration clock for Altera reconfig cores
    locked  => locked);   -- '1' when the PLL has locked

    reset : gc_reset
    port map(
      free_clk_i => clk_sys,
      locked_i => locked,
      clks_i => reset_clks,
      rstn_o => reset_rstn);
    reset_clks(0) <= clk_sys;
    clk_sys_rstn <= reset_rstn(0);

  -- open drain buffer for one wire
    owr_i(0) <= A_OneWire;
    A_OneWire <= owr_pwren_o(0) when (owr_pwren_o(0) = '1' or owr_en_o(0) = '1') else 'Z';
    owr_i(1) <= A_OneWire_EEPROM;
    A_OneWire_EEPROM <= owr_pwren_o(1) when (owr_pwren_o(1) = '1' or owr_en_o(1) = '1') else 'Z';
  
  -- The top-most Wishbone B.4 crossbar
  interconnect : xwb_sdb_crossbar
   generic map(
     g_num_masters => c_masters,
     g_num_slaves => c_slaves,
     g_registered => true,
     g_wraparound => false, -- Should be true for nested buses
     g_layout => c_layout,
     g_sdb_addr => c_sdb_address)
   port map(
     clk_sys_i => clk_sys,
     rst_n_i => clk_sys_rstn,
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
      rst_n_i => clk_sys_rstn,
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
      g_init_file => "scu_addac.mif")
    port map(
      clk_sys_i => clk_sys,
      rst_n_i => clk_sys_rstn,
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
      rst_n_i     => clk_sys_rstn,

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
      g_with_virtual_uart => false,
      g_with_physical_uart => true,
      g_interface_mode => PIPELINED,
      g_address_granularity => BYTE
      )
    port map(
      clk_sys_i => clk_sys,
      rst_n_i => clk_sys_rstn,

      -- Wishbone
      slave_i => cbar_master_o(2),
      slave_o => cbar_master_i(2),
      desc_o => open,

      uart_rxd_i => '0',
      uart_txd_o => A_TB(0)
      );
  
  SCU_WB_Reg: wb_scu_reg
    generic map (
      Base_addr => x"0040",
      register_cnt => 16 )
    port map (
      clk_sys_i => clk_sys,
      rst_n_i => clk_sys_rstn,

      -- Wishbone
      slave_i => cbar_master_o(3),
      slave_o => cbar_master_i(3),
      
      Adr_from_SCUB_LA => ADR_from_SCUB_LA,
      Data_from_SCUB_LA => Data_from_SCUB_LA,
      Ext_Adr_Val => Ext_Adr_Val,
      Ext_Rd_active => Ext_Rd_active,
      Ext_Wr_active => Ext_Wr_active,
      user_rd_active => wb_scu_rd_active,
      Data_to_SCUB => wb_scu_data_to_SCUB,
      Dtack_to_SCUB => wb_scu_dtack );

    

Dtack_to_SCUB <= io_port_Dtack_to_SCUB or dac1_dtack or dac2_dtack or adc_dtack
                or wb_scu_dtack or fg_1_dtack or fg_2_dtack or tmr_dtack;

SCU_Slave: SCU_Bus_Slave
generic map (
    CLK_in_Hz               => clk_sys_in_Hz,
    Firmware_Release        => 0,
    Firmware_Version        => 0,
    CID_System              => 55,  -- important: 55 => CSCOHW
    CID_Group               => 3,   -- important: 3 => "FG900160_SCU_ADDAC1"
    Intr_Enable             => b"0000_0000_0000_0001",
    Slave_ID                => scu_adda1_id)
port map (
    SCUB_Addr               => A_A,                 -- in,        SCU_Bus: address bus
    nSCUB_Timing_Cyc        => A_nEvent_Str,        -- in,        SCU_Bus signal: low active SCU_Bus runs timing cycle
    SCUB_Data               => A_D,                 -- inout,        SCU_Bus: data bus (FPGA tri state buffer)
    nSCUB_Slave_Sel         => A_nBoardSel,         -- in, SCU_Bus: '0' => SCU master select slave
    nSCUB_DS                => A_nDS,               -- in,        SCU_Bus: '0' => SCU master activate data strobe
    SCUB_RDnWR              => A_RnW,               -- in, SCU_Bus: '1' => SCU master read slave
    clk                     => clk_sys,
    nSCUB_Reset_in          => A_nReset,            -- in,        SCU_Bus-Signal: '0' => 'nSCUB_Reset_In' is active
    Data_to_SCUB            => Data_to_SCUB,        -- in,        connect read sources from external user functions
    Dtack_to_SCUB           => Dtack_to_SCUB,       -- in,        connect Dtack from from external user functions
    Intr_In                 => "000000000000" & tmr_irq & fg_2_dreq & fg_1_dreq, -- in,        interrupt(15 downto 1)
    User_Ready              => '1',
    Data_from_SCUB_LA       => Data_from_SCUB_LA,   -- out,        latched data from SCU_Bus for external user functions
    ADR_from_SCUB_LA        => ADR_from_SCUB_LA,    -- out,        latched address from SCU_Bus for external user functions
    Timing_Pattern_LA       => open,                -- out,        latched timing pattern from SCU_Bus for external user functions
    Timing_Pattern_RCV      => open,                -- out,        timing pattern received
    nSCUB_Dtack_Opdrn       => open,                -- out,        for direct connect to SCU_Bus opendrain signal
                                                    -- '0' => slave give dtack to SCU master
    SCUB_Dtack              => SCUB_Dtack,          -- out,        for connect via ext. open collector driver
                                                    -- '1' => slave give dtack to SCU master
    nSCUB_SRQ_Opdrn         => open,                -- out,        for direct connect to SCU_Bus opendrain signal
                                                    -- '0' => slave service request to SCU ma
    SCUB_SRQ                => SCUB_SRQ,            -- out,        for connect via ext. open collector driver
                                                    -- '1' => slave service request to SCU master
    nSel_Ext_Data_Drv       => A_nSel_Ext_Data_Drv, -- out,        '0' => select the external data driver on the SCU_Bus slave
    Ext_Data_Drv_Rd         => A_Ext_Data_RD,       -- out,        '1' => direction of the external data driver on the
                                                    -- SCU_Bus slave is to the SCU_Bus
    Standard_Reg_Acc        => open,                -- out,        '1' => mark the access to register of this macro
    Ext_Adr_Val             => Ext_Adr_Val,         -- out,        for external user functions: '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active           => Ext_Rd_active,       -- out,        '1' => Rd-Cycle to external user register is active
    Ext_Rd_fin              => open,                -- out,        marks end of read cycle, active one for one clock period
                                                    -- of clk past cycle end (no overlap)
    Ext_Rd_Fin_ovl          => open,                -- out,        marks end of read cycle, active one for one clock period
                                                    -- of clk during cycle end (overlap)
    Ext_Wr_active           => Ext_Wr_active,       -- out,        '1' => Wr-Cycle to external user register is active
    Ext_Wr_fin              => open,                -- out,        marks end of write cycle, active high for one clock period
                                                    -- of clk past cycle end (no overlap)
    Ext_Wr_fin_ovl          => Ext_Wr_fin_ovl,      -- out, marks end of write cycle, active high for one clock period
                                                    -- of clk before write cycle finished (with overlap)
    Deb_SCUB_Reset_out      => open,                -- out,        the debounced 'nSCUB_Reset_In'-signal, is active high,
                                                    -- can be used to reset
                                                    -- external macros, when 'nSCUB_Reset_In' is '0'
    nPowerup_Res            => nPowerup_Res,        -- out,        this macro generated a power up reset
    Powerup_Done            => open                 -- out  this memory is set to one if an Powerup is done. Only the SCUB-Master can clear this bit.
    );

dac_1: dac714
  generic map(
    Base_addr         => x"0200",
    CLK_in_Hz         => clk_sys_in_Hz,
    SPI_CLK_in_Hz     => dac_spi_clk_in_hz)
  port map(
    Adr_from_SCUB_LA  => ADR_from_SCUB_LA,      -- in, latched address from SCU_Bus
    Data_from_SCUB_LA => Data_from_SCUB_LA,     -- in, latched data from SCU_Bus
    Ext_Adr_Val       => Ext_Adr_Val,           -- in, '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active     => Ext_Rd_active,         -- in, '1' => Rd-Cycle is active
    Ext_Wr_active     => Ext_Wr_active,         -- in, '1' => Wr-Cycle is active
    clk               => clk_sys,               -- in, should be the same clk, used by SCU_Bus_Slave
    nReset            => nPowerup_Res,          -- in, '0' => resets the DAC_1
    nExt_Trig_DAC     => EXT_TRIG_DAC,          -- external trigger input over optocoupler,
                                                -- led on -> nExt_Trig_DAC is low
    FG_Data           => fg_1_sw(23 downto 8),  -- parallel dac data during FG-Mode
    FG_Strobe         => fg_1_strobe,           -- strobe to start SPI transfer (if possible) during FG-Mode
    DAC_SI            => DAC1_SDI,              -- out, is connected to DAC1-SDI
    nDAC_CLK          => nDAC1_CLK,             -- out, spi-clock of DAC1
    nCS_DAC           => nDAC1_A0,              -- out, '0' enable shift of internal shift register of DAC1
    nLD_DAC           => nDAC1_A1,              -- out, '0' copy shift register to output latch of DAC1
    nCLR_DAC          => nDAC1_CLR,             -- out, '0' set DAC1 to zero (pulse width min 200 ns)
    ext_trig_valid    => dac_1_ext_trig,        -- out, '1' got an valid external trigger, during extern trigger mode.
    Rd_Port           => dac1_data_to_SCUB,     -- out, connect read sources (over multiplexer) to SCUB-Macro
    Rd_Activ          => dac1_rd_active,        -- out, '1' = read data available at 'Data_to_SCUB'-output
    Dtack             => dac1_dtack
    );

    
dac_2: dac714
  generic map(
    Base_addr         => x"0210",
    CLK_in_Hz         => clk_sys_in_Hz,
    SPI_CLK_in_Hz     => dac_spi_clk_in_hz)
  port map(
    Adr_from_SCUB_LA  => ADR_from_SCUB_LA,      -- in, latched address from SCU_Bus
    Data_from_SCUB_LA => Data_from_SCUB_LA,     -- in, latched data from SCU_Bus
    Ext_Adr_Val       => Ext_Adr_Val,           -- in, '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active     => Ext_Rd_active,         -- in, '1' => Rd-Cycle is active
    Ext_Wr_active     => Ext_Wr_active,         -- in, '1' => Wr-Cycle is active
    clk               => clk_sys,               -- in, should be the same clk, used by SCU_Bus_Slave
    nReset            => nPowerup_Res,          -- in, '0' => resets the DAC_2
    nExt_Trig_DAC     => EXT_TRIG_DAC,          -- external trigger input over optocoupler,
                                                -- led on -> nExt_Trig_DAC is low
    FG_Data           => fg_2_sw(23 downto 8),  -- parallel dac data during FG-Mode
    FG_Strobe         => fg_2_strobe,           -- strobe to start SPI transfer (if possible) during FG-Mode
    DAC_SI            => DAC2_SDI,              -- out, is connected to DAC2-SDI
    nDAC_CLK          => nDAC2_CLK,             -- out, spi-clock of DAC2
    nCS_DAC           => nDAC2_A0,              -- out, '0' enable shift of internal shift register of DAC2
    nLD_DAC           => nDAC2_A1,              -- out, '0' copy shift register to output latch of DAC2
    nCLR_DAC          => nDAC2_CLR,             -- out, '0' set DAC2 to zero (pulse width min 200 ns)
    ext_trig_valid    => dac_2_ext_trig,        -- out, '1' got an valid external trigger, during extern trigger mode.
    Rd_Port           => dac2_data_to_SCUB,     -- out, connect read sources (over multiplexer) to SCUB-Macro
    Rd_Activ          => dac2_rd_active,        -- out, '1' = read data available at 'Data_to_SCUB'-output
    Dtack             => dac2_dtack
    );


io_port: IO_4x8
  generic map (
    Base_addr => x"0220")
  port map (
    Adr_from_SCUB_LA => ADR_from_SCUB_LA, -- in, latched address from SCU_Bus
    Data_from_SCUB_LA => Data_from_SCUB_LA, -- in, latched data from SCU_Bus
    Ext_Adr_Val => Ext_Adr_Val, -- in, '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active => Ext_Rd_active, -- in, '1' => Rd-Cycle is active
    Ext_Wr_active => Ext_Wr_active, -- in, '1' => Wr-Cycle is active
    clk => clk_sys, -- in, should be the same clk, used by SCU_Bus_Slave
    nReset => nPowerup_Res, -- in, '0' => resets the IO_4x8
    io => a_io, -- inout, select and set direction only in 8-bit partitions
    io_7_0_tx => a_io_7_0_tx, -- out, '1' = external io(7..0)-buffer set to output.
    ext_io_7_0_dis => a_ext_io_7_0_dis, -- out, '1' = disable external io(7..0)-buffer.
    io_15_8_tx => a_io_15_8_tx, -- out, '1' = external io(15..8)-buffer set to output
    ext_io_15_8_dis => a_ext_io_15_8_dis, -- out, '1' = disable external io(15..8)-buffer.
    io_23_16_tx => a_io_23_16_tx, -- out, '1' = external io(23..16)-buffer set to output.
    ext_io_23_16_dis => a_ext_io_23_16_dis, -- out, '1' = disable external io(23..16)-buffer.
    io_31_24_tx => a_io_31_24_tx, -- out, '1' = external io(31..24)-buffer set to output
    ext_io_31_24_dis => a_ext_io_31_24_dis, -- out, '1' = disable external io(31..24)-buffer.
    user_rd_active => io_port_rd_active, -- out, '1' = read data available at 'Data_to_SCUB'-output
    Data_to_SCUB => io_port_data_to_SCUB, -- out, connect read sources to SCUB-Macro
    Dtack_to_SCUB => io_port_Dtack_to_SCUB); -- out, connect Dtack to SCUB-Macro

    
adc: adc_scu_bus
  generic map (
    Base_addr     => x"0230",
    clk_in_Hz     => clk_sys_in_Hz,
    diag_on_is_1  => 1)
  port map (
    clk           => clk_sys,
    nrst          => nPowerup_Res,
    
    db            => ADC_DB(13 downto 0),
    db14_hben     => ADC_DB(14),
    db15_byte_sel => ADC_DB(15),
    convst_a      => ADC_CONVST_A,
    convst_b      => ADC_CONVST_B,
    n_cs          => nADC_CS,
    n_rd_sclk     => nADC_RD_SCLK,
    busy          => ADC_BUSY,
    adc_reset     => ADC_RESET,
    os            => ADC_OS,
    par_ser_sel   => nADC_PAR_SER_SEL,
    adc_range     => ADC_Range,
    firstdata     => ADC_FRSTDATA,
    nDiff_In_En   => NDIFF_IN_EN,
    
    Adr_from_SCUB_LA  => ADR_from_SCUB_LA,
    Data_from_SCUB_LA => Data_from_SCUB_LA,
    Ext_Adr_Val       => Ext_Adr_Val,
    Ext_Rd_active     => Ext_Rd_active,
    Ext_Wr_active     => Ext_Wr_active,
    user_rd_active    => adc_rd_active,
    Data_to_SCUB      => adc_data_to_SCUB,
    Dtack_to_SCUB     => adc_dtack,

    channel_1 => ADC_channel_1,
    channel_2 => ADC_channel_2,
    channel_3 => ADC_channel_3,
    channel_4 => ADC_channel_4,
    channel_5 => ADC_channel_5,
    channel_6 => ADC_channel_6,
    channel_7 => ADC_channel_7,
    channel_8 => ADC_channel_8);
   
    
fg_1: fg_quad_scu_bus
  generic map (
    Base_addr     => x"0300",
    clk_in_hz     => clk_sys_in_Hz,
    diag_on_is_1  => 0 -- if 1 then diagnosic information is generated during compilation
    )
  port map (

    -- SCUB interface
    Adr_from_SCUB_LA  => ADR_from_SCUB_LA,      -- in, latched address from SCU_Bus
    Data_from_SCUB_LA => Data_from_SCUB_LA,     -- in, latched data from SCU_Bus
    Ext_Adr_Val       => Ext_Adr_Val,           -- in, '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active     => Ext_Rd_active,         -- in, '1' => Rd-Cycle is active
    Ext_Wr_active     => Ext_Wr_active,         -- in, '1' => Wr-Cycle is active
    clk               => clk_sys,               -- in, should be the same clk, used by SCU_Bus_Slave
    nReset            => nPowerup_Res,          -- in, '0' => resets the fg_1
    Rd_Port           => fg_1_data_to_SCUB,     -- out, connect read sources (over multiplexer) to SCUB-Macro
    user_rd_active    => fg_1_rd_active,        -- '1' = read data available at 'Rd_Port'-output
    Dtack             => fg_1_dtack,            -- connect Dtack to SCUB-Macro
    dreq              => fg_1_dreq,             -- request of new parameter set

    -- fg output
    sw_out            => fg_1_sw,               -- 24bit output from fg
    sw_strobe         => fg_1_strobe            -- signals new output data
  );

fg_2: fg_quad_scu_bus
  generic map (
    Base_addr     => x"0340",
    clk_in_hz     => clk_sys_in_Hz,
    diag_on_is_1  => 0 -- if 1 then diagnosic information is generated during compilation
    )
  port map (

    -- SCUB interface
    Adr_from_SCUB_LA  => ADR_from_SCUB_LA,      -- in, latched address from SCU_Bus
    Data_from_SCUB_LA => Data_from_SCUB_LA,     -- in, latched data from SCU_Bus
    Ext_Adr_Val       => Ext_Adr_Val,           -- in, '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active     => Ext_Rd_active,         -- in, '1' => Rd-Cycle is active
    Ext_Wr_active     => Ext_Wr_active,         -- in, '1' => Wr-Cycle is active
    clk               => clk_sys,               -- in, should be the same clk, used by SCU_Bus_Slave
    nReset            => nPowerup_Res,          -- in, '0' => resets the fg_1
    Rd_Port           => fg_2_data_to_SCUB,     -- out, connect read sources (over multiplexer) to SCUB-Macro
    user_rd_active    => fg_2_rd_active,        -- '1' = read data available at 'Rd_Port'-output
    Dtack             => fg_2_dtack,            -- connect Dtack to SCUB-Macro
    dreq              => fg_2_dreq,             -- request of new parameter set

    -- fg output
    sw_out            => fg_2_sw,               -- 24bit output from fg
    sw_strobe         => fg_2_strobe            -- signals new output data
  );

  tmr: tmr_scu_bus
  generic map (
    Base_addr     => x"0330",
    diag_on_is_1  => 1)
  port map (
    clk           => clk_sys,
    nrst          => nPowerup_Res,
    tmr_irq       => tmr_irq,
    
    Adr_from_SCUB_LA  => ADR_from_SCUB_LA,
    Data_from_SCUB_LA => Data_from_SCUB_LA,
    Ext_Adr_Val       => Ext_Adr_Val,
    Ext_Rd_active     => Ext_Rd_active,
    Ext_Wr_active     => Ext_Wr_active,
    user_rd_active    => tmr_rd_active,
    Data_to_SCUB      => tmr_data_to_SCUB,
    Dtack_to_SCUB     => tmr_dtack);

p_led_ena: div_n
  generic map (
    n => clk_sys_in_Hz / 100, -- div_o is every 10 ms for one clock period active
    diag_on => 0)
  port map (
    res => not nPowerup_Res, -- in, '1' => set "div_n"-counter asynchron to generic-value "n"-2, so the
                                    -- countdown is "n"-1 clocks to activate the "div_o"-output for one clock periode.
    clk => clk_sys, -- clk = clock
    ena => '1', -- in, can be used for a reduction, signal should be generated from the same
                                    -- clock domain and should be only one clock period active.
    div_o => led_ena_cnt); -- out, div_o becomes '1' for one clock period, if "div_n" arrive n-1 clocks
                                    -- (if ena is permanent '1').


--p_test_port_mux: process (
-- DAC1_SDI, nDAC1_CLK, nDAC1_A0, nDAC1_A1, nDAC1_CLR, dac1_rd_active, dac1_dtack,
-- DAC2_SDI, nDAC2_CLK, nDAC2_A0, nDAC2_A1, nDAC2_CLR, dac2_rd_active, dac2_dtack,
-- ADC_Range, ADC_FRSTDATA,
-- ADC_CONVST_A, ADC_CONVST_B, nADC_CS, nADC_RD_SCLK, ADC_BUSY, ADC_RESET, ADC_OS, nADC_PAR_SER_SEL, ADC_DB(15 downto 0),
-- A_SEL(3 downto 0)
-- )
-- begin
-- case not A_SEL IS
--
-- when X"0" =>
-- A_TA <= '0' & DAC2_SDI & nDAC2_CLK & nDAC2_A0 & nDAC2_A1 & nDAC2_CLR & dac2_rd_active & dac2_dtack &
-- '0' & DAC1_SDI & nDAC1_CLK & nDAC1_A0 & nDAC1_A1 & nDAC1_CLR & dac1_rd_active & dac1_dtack;
-- A_TB <= X"0000";
--
-- when X"1" =>
-- A_TA <= X"0" & ADC_Range & ADC_FRSTDATA & ADC_CONVST_A & ADC_CONVST_B &
-- nADC_CS & nADC_RD_SCLK & ADC_BUSY & ADC_RESET & ADC_OS & nADC_PAR_SER_SEL;
-- A_TB <= ADC_DB(15 downto 0);
--
-- when others =>
-- A_TA <= (others => '0');
-- A_TB <= (others => '0');
-- end case;
--
-- end process p_test_port_mux;

  -------------------------------------------------------------------------------
	-- precsaler for the led test vector
	-------------------------------------------------------------------------------
	sec_prescale:	process(clk_sys, nPowerup_Res)
	begin
		if nPowerup_Res = '0' then
			s_led_en <= '0';
		elsif rising_edge(clk_sys) then
			if s_led_cnt = to_unsigned(c_led_cnt, c_led_cnt_width) then
				s_led_en <= '1';
				s_led_cnt <= (others => '0');
			else
				s_led_en <= '0';
				s_led_cnt <= s_led_cnt  + 1;
			end if;
		end if;
	end process;
	
	-------------------------------------------------------------------------------
	-- rotating bit as a test vector for led testing
	-------------------------------------------------------------------------------
	test_signal: process(clk_sys, nPowerup_Res, s_led_en)
	begin
		if nPowerup_Res = '0' then
			s_test_vector <= ('1', others => '0');
		elsif rising_edge(clk_sys) and s_led_en = '1' then
			s_test_vector <= s_test_vector(s_test_vector'high-1 downto 0) & s_test_vector(s_test_vector'high);
		end if;
	end process;

p_led_mux: process (
    ADC_channel_1, ADC_channel_2, ADC_channel_3, ADC_channel_4,
    ADC_channel_5, ADC_channel_6, ADC_channel_7, ADC_channel_8,
    A_ADC_DAC_SEL(3 downto 0), A_MODE_SEL(1 downto 0),
    nADC_PAR_SER_SEL, NDIFF_IN_EN
    )
  begin
    if A_MODE_SEL = "11" then
      A_nLED <= not nADC_PAR_SER_SEL & nADC_PAR_SER_SEL & NDIFF_IN_EN & not dac1_dtack & not dac2_dtack & "111" & x"FF";
    elsif A_MODE_SEL = "01" then
      case not A_ADC_DAC_SEL IS
        when X"1" => A_nLED <= not ADC_channel_1;
        when X"2" => A_nLED <= not ADC_channel_2;
        when X"3" => A_nLED <= not ADC_channel_3;
        when X"4" => A_nLED <= not ADC_channel_4;
        when X"5" => A_nLED <= not ADC_channel_5;
        when X"6" => A_nLED <= not ADC_channel_6;
        when X"7" => A_nLED <= not ADC_channel_7;
        when X"8" => A_nLED <= not ADC_channel_8;
        when others =>
          A_nLED <= (others => '1');
      end case;
    elsif A_MODE_SEL = "10" then
      A_nLED <= not s_test_vector;
    else
      A_nLED <= (others => '1');
    end if;
  end process p_led_mux;
 

p_read_mux: process (
    io_port_rd_active, io_port_data_to_SCUB,
    dac1_rd_active, dac1_data_to_SCUB,
    dac2_rd_active, dac2_data_to_SCUB,
    adc_rd_active, adc_data_to_SCUB,
    fg_1_rd_active, fg_1_data_to_SCUB,
    fg_2_rd_active, fg_2_data_to_SCUB,
    tmr_rd_active, tmr_data_to_SCUB,
    wb_scu_rd_active, wb_scu_data_to_SCUB
    )
  variable sel: unsigned(7 downto 0);
  begin
    sel :=  wb_scu_rd_active & tmr_rd_active & fg_2_rd_active & fg_1_rd_active & adc_rd_active & dac2_rd_active & dac1_rd_active & io_port_rd_active;
    case sel IS
      when "00000001" => Data_to_SCUB <= io_port_data_to_SCUB;
      when "00000010" => Data_to_SCUB <= dac1_data_to_SCUB;
      when "00000100" => Data_to_SCUB <= dac2_data_to_SCUB;
      when "00001000" => Data_to_SCUB <= adc_data_to_SCUB;
      when "00010000" => Data_to_SCUB <= fg_1_data_to_SCUB;
      when "00100000" => Data_to_SCUB <= fg_2_data_to_SCUB;
      when "01000000" => Data_to_SCUB <= tmr_data_to_SCUB;
      when "10000000" => Data_to_SCUB <= wb_scu_data_to_SCUB;
      when others =>
        Data_to_SCUB <= X"0000";
    end case;
  end process p_read_mux;
  

sel_led: led_n
  generic map (
    stretch_cnt => 5)
  port map (
    ena => led_ena_cnt, -- is every 10 ms for one clock period active
    clk => clk_sys,
    Sig_in => not A_nBoardSel,
    nLED => open,
    nLED_opdrn => A_nState_LED(0));

    
dtack_led: led_n
  generic map (
    stretch_cnt => 5)
  port map
    (
    ena => led_ena_cnt, -- is every 10 ms for one clock period active
    clk => clk_sys,
    Sig_in => SCUB_Dtack,
    nLED => open,
    nLED_opdrn => A_nState_LED(1));
    

rw_led: led_n
  generic map (
    stretch_cnt => 5)
  port map (
    ena => led_ena_cnt, -- is every 10 ms for one clock period active
    clk => clk_sys,
    Sig_in => not A_RnW and not A_nBoardSel,
    nLED => open,
    nLED_opdrn => A_nState_LED(2));

ext_trig_led: led_n
  generic map (
    stretch_cnt => 3)
  port map (
    ena => led_ena_cnt, -- is every 10 ms for one clock period active
    clk => clk_sys,
    Sig_in => dac_1_ext_trig or dac_2_ext_trig,
    nLED => open,
    nLED_opdrn => A_nLED_Trig_DAC);

    
  A_nDtack <= not SCUB_Dtack;
  A_nSRQ <= not SCUB_SRQ;
  A_TB(1) <= '1';
  --A_TA <= fg_1_sw(23 downto 8);
  --
  --A_TB <= fg_1_sw(7 downto 0) & fg_1_strobe & "0000000";
end architecture;
