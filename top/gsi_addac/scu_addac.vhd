library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.scu_bus_slave_pkg.all;
use work.aux_functions_pkg.all;
use work.adc_pkg.all;

entity scu_addac is
  port (
    -------------------------------------------------------------------------------------------------------------------
    CLK_FPGA:               in    std_logic;
    
    --------- Parallel SCU-Bus-Signale --------------------------------------------------------------------------------
    A_A:                  in    std_logic_vector(15 downto 0);  -- SCU-Adressbus
    A_nADR_EN:            out   std_logic := '0';               -- '0' => externe Adresstreiber des Slaves aktiv
    A_nADR_FROM_SCUB:     out   std_logic := '0';               -- '0' => externe Adresstreiber-Richtung: SCU-Bus nach Slave  
    A_D:                  inout std_logic_vector(15 downto 0);  -- SCU-Datenbus
    A_nDS:                in    std_logic;                      -- Data-Strobe vom Master gertieben
    A_RnW:                in    std_logic;                      -- Schreib/Lese-Signal vom Master getrieben, '0' => lesen
    A_nSel_Ext_Data_Drv:  out   std_logic;                      -- '0' => externe Datentreiber des Slaves aktiv
    A_Ext_Data_RD:        out   std_logic;                      -- '0' => externe Datentreiber-Richtung: SCU-Bus nach
                                                                -- Slave (besser default 0, oder Treiber A/B tauschen)
                                                                -- SCU-Bus nach Slave (besser default 0, oder Treiber A/B tauschen)
    A_nDtack:             out   std_logic;                      -- Data-Acknowlege null aktiv, '0' => aktiviert externen
                                                                -- Opendrain-Treiber
    A_nSRQ:               out   std_logic;                      -- Service-Request null aktiv, '0' => aktiviert externen
                                                                -- Opendrain-Treiber
    A_nBoardSel:          in    std_logic;                      -- '0' => Master aktiviert diesen Slave
    A_nEvent_Str:         in    std_logic;                      -- '0' => Master sigalisiert Timing-Zyklus
    A_SysClock:           in    std_logic;                      -- Clock vom Master getrieben.
    A_Spare0:             in    std_logic;                      -- vom Master getrieben
    A_Spare1:             in    std_logic;                      -- vom Master getrieben
    A_nReset:             in    std_logic;                      -- Reset (aktiv '0'), vom Master getrieben

    ------------ ADC Signals ------------------------------------------------------------------------------------------
    ADC_DB:               inout   std_logic_vector(15 downto 0);
    ADC_CONVST_A:         buffer  std_logic;
    ADC_CONVST_B:         buffer  std_logic;
    nADC_CS:              buffer  std_logic;
    nADC_RD_SCLK:         buffer  std_logic;
    ADC_BUSY:             in      std_logic;
    ADC_RESET:            buffer  std_logic;
    ADC_OS:               buffer  std_logic_vector(2 downto 0);
    nADC_PAR_SER_SEL:     buffer  std_logic := '0';
    ADC_Range:            buffer  std_logic;
    ADC_FRSTDATA:         in      std_logic;
    EXT_TRIG_ADC:         in      std_logic;
    ------------ ADC Diagnostic ---------------------------------------------------------------------------------------
    A_ADC_DAC_SEL:        in      std_logic_vector(3 downto 0);

    ------------ DAC Signals ------------------------------------------------------------------------------------------
    DAC1_SDI:             buffer  std_logic;                    -- is connected to DAC1-SDI
    DAC1_SDO:             buffer  std_logic;
    nDAC1_CLK:            buffer  std_logic;                    -- spi-clock of DAC1
    nDAC1_CLR:            buffer  std_logic;                    -- '0' set DAC1 to zero (pulse width min 200 ns)
    nDAC1_A0:             buffer  std_logic;                    -- '0' enable shift of internal shift register of DAC1
    nDAC1_A1:             buffer  std_logic;                    -- '0' copy shift register to output latch of DAC1
    DAC2_SDI:             buffer  std_logic;                    -- is connected to DAC2-SDI
    DAC2_SDO:             buffer  std_logic;
    nDAC2_CLK:            buffer  std_logic;                    -- spi-clock of DAC2
    nDAC2_CLR:            buffer  std_logic;                    -- '0' set DAC2 to zero (pulse width min 200 ns)
    nDAC2_A0:             buffer  std_logic;                    -- '0' enable shift of internal shift register of DAC2
    nDAC2_A1:             buffer  std_logic;                    -- '0' copy shift register to output latch of DAC2
    EXT_TRIG_DAC:         in      std_logic;
    
    ------------ IO-Port-Signale --------------------------------------------------------------------------------------
    a_io_7_0_tx:          out   std_logic;                      -- '1' = external io(7..0)-buffer set to output.
    a_io_15_8_tx:         out   std_logic;                      -- '1' = external io(15..8)-buffer set to output
    a_io_23_16_tx:        out   std_logic;                      -- '1' = external io(23..16)-buffer set to output
    a_io_31_24_tx:        out   std_logic;                      -- '1' = external io(31..24)-buffer set to output
    a_ext_io_7_0_dis:     out   std_logic;                      -- '1' = disable external io(7..0)-buffer.  
    a_ext_io_15_8_dis:    out   std_logic;                      -- '1' = disable external io(15..8)-buffer. 
    a_ext_io_23_16_dis:   out   std_logic;                      -- '1' = disable external io(23..16)-buffer.
    a_ext_io_31_24_dis:   out   std_logic;                      -- '1' = disable external io(31..24)-buffer.
    a_io:                 inout std_logic_vector(31 downto 0);  -- select and set direction only in 8-bit partitions
    
    ------------ Logic analyser Signals -------------------------------------------------------------------------------
    A_SEL:                in    std_logic_vector(3 downto 0);   -- use to select sources for the logic analyser ports
    A_TA:                 out   std_logic_vector(15 downto 0);  -- test port a
    A_TB:                 out   std_logic_vector(15 downto 0);  -- test port b
    TP:                   out   std_logic_vector(2 downto 1);   -- test points

    A_nState_LED:         out   std_logic_vector(2 downto 0);   -- ..LED(2) = R/W, ..LED(1) = Dtack, ..LED(0) = Sel
    A_nLED:               out   std_logic_vector(15 downto 0);
    A_NLED_TRIG_DAC:      out   std_logic;
    A_NLED_TRIG_ADC:      out   std_logic;
    
    HW_REV:               in    std_logic_vector(3 downto 0);
    A_MODE_SEL:           in    std_logic_vector(1 downto 0);
    A_OneWire:            inout std_logic;
    A_OneWire_EEPROM:     inout std_logic;
    
    NDIFF_IN_EN:          out   std_logic := '0'                -- enables diff driver for ADC channels 3-8
    
    
    );
end entity;



architecture scu_addac_arch of scu_addac is


constant  scu_adda1_id:       integer range 16#0210# to 16#021F# := 16#0210#;

constant  clk_sys_in_Hz:      integer := 125_000_000;
constant  dac_spi_clk_in_hz:  integer := 10_000_000;


component DAC_SPI
  generic (
    Base_addr:        unsigned(15 downto 0);
    CLK_in_Hz:        integer := 100_000_000;
    SPI_CLK_in_Hz:    integer := 10_000_000 );
  port (
    Adr_from_SCUB_LA:   in    std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
    Data_from_SCUB_LA:  in    std_logic_vector(15 downto 0);  -- latched data from SCU_Bus 
    Ext_Adr_Val:        in    std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:      in    std_logic;                      -- '1' => Rd-Cycle is active
    Ext_Wr_active:      in    std_logic;                      -- '1' => Wr-Cycle is active
    Ext_Wr_fin:         in    std_logic;                      -- '1' => Wr-Cycle is finished
    clk:                in    std_logic;                      -- should be the same clk, used by SCU_Bus_Slave
    nReset:             in    std_logic := '1';
    DAC_SI:             out   std_logic;
    nDAC_CLK:           out   std_logic;
    nCS_DAC:            out   std_logic;
    nLD_DAC:            out   std_logic;
    nCLR_DAC:           out   std_logic;
    Rd_Port:            out   std_logic_vector(15 downto 0);
    Rd_Activ:           out   std_logic;
    Dtack:              out   std_logic
    );
end component DAC_SPI;

component IO_4x8
  generic (
      Base_addr:  unsigned(15 downto 0));
  port (
    Adr_from_SCUB_LA:   in    std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
    Data_from_SCUB_LA:  in    std_logic_vector(15 downto 0);  -- latched data from SCU_Bus 
    Ext_Adr_Val:        in    std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:      in    std_logic;                      -- '1' => Rd-Cycle is active
    Ext_Wr_active:      in    std_logic;                      -- '1' => Wr-Cycle is active
    clk:                in    std_logic;                      -- should be the same clk, used by SCU_Bus_Slave
    nReset:             in    std_logic := '1';
    io:                 inout std_logic_vector(31 downto 0);  -- select and set direction only in 8-bit partitions
    io_7_0_tx:          out   std_logic;                      -- '1' = external io(7..0)-buffer set to output.
    ext_io_7_0_dis:     out   std_logic;                      -- '1' = disable external io(7..0)-buffer.
    io_15_8_tx:         out   std_logic;                      -- '1' = external io(15..8)-buffer set to output
    ext_io_15_8_dis:    out   std_logic;                      -- '1' = disable external io(15..8)-buffer.
    io_23_16_tx:        out   std_logic;                      -- '1' = external io(23..16)-buffer set to output.
    ext_io_23_16_dis:   out   std_logic;                      -- '1' = disable external io(23..16)-buffer.
    io_31_24_tx:        out   std_logic;                      -- '1' = external io(31..24)-buffer set to output
    ext_io_31_24_dis:   out   std_logic;                      -- '1' = disable external io(31..24)-buffer.
    user_rd_active:     out   std_logic;                      -- '1' = read data available at 'Data_to_SCUB'-output
    Data_to_SCUB:       out   std_logic_vector(15 downto 0);  -- connect read sources to SCUB-Macro
    Dtack_to_SCUB:      out   std_logic                       -- connect Dtack to SCUB-Macro
    );  
  end component IO_4x8;

 
component flash_loader_v01
  port (
    noe_in: in  std_logic
    );
  end component;


component adda_pll
	port
	(
		inclk0: in  std_logic  := '0';
		c0:     out std_logic;
		c1:     out std_logic;
		locked: out std_logic 
	);
end component;

  
  signal clk_sys, clk_cal, locked : std_logic;
  
  signal SCUB_SRQ:    std_logic;
  signal SCUB_Dtack:  std_logic;
  signal convst:      std_logic;
  signal rst:         std_logic;
  
  signal  Dtack_to_SCUB:      std_logic;

  signal  io_port_Dtack_to_SCUB:  std_logic;
  signal  io_port_data_to_SCUB:   std_logic_vector(15 downto 0);
  signal  io_port_rd_active:      std_logic;
  
  signal  dac1_Dtack:         std_logic;
  signal  dac1_data_to_SCUB:  std_logic_vector(15 downto 0);
  signal  dac1_rd_active:     std_logic;
  
  signal  dac2_Dtack:         std_logic;
  signal  dac2_data_to_SCUB:  std_logic_vector(15 downto 0);
  signal  dac2_rd_active:     std_logic;
  
  signal  ADR_from_SCUB_LA:   std_logic_vector(15 downto 0);
  signal  Data_from_SCUB_LA:  std_logic_vector(15 downto 0);
  signal  Ext_Adr_Val:        std_logic;
  signal  Ext_Rd_active:      std_logic;
  signal  Ext_Wr_active:      std_logic;
  signal  Ext_Wr_fin_ovl:     std_logic;
  signal  nPowerup_Res:       std_logic;
  
  signal  adc_rd_active:      std_logic;
  signal  adc_data_to_SCUB:   std_logic_vector(15 downto 0);
  signal  adc_dtack:          std_logic;
  
  signal  rw_signal:          std_logic;
  signal  led_ena_cnt:        std_logic;

  signal  ADC_channel_1, ADC_channel_2, ADC_channel_3, ADC_channel_4: std_logic_vector(15 downto 0);
  signal  ADC_channel_5, ADC_channel_6, ADC_channel_7, ADC_channel_8: std_logic_vector(15 downto 0);

  signal  Data_to_SCUB:       std_logic_vector(15 downto 0);
  
  signal  modelsim_A_nBoardSel: std_logic;
  signal  modelsim_nPowerup_Res: std_logic;

  begin


fl : flash_loader_v01
  port map (noe_in	=>	'0');

  -- Obtain core clocking
adda_pll_1: adda_pll        -- Altera megafunction
	port map (
		inclk0 => CLK_FPGA,     -- 125Mhz oscillator from board
		c0     => clk_sys,      -- 125MHz system clk
    c1     => clk_cal,      -- 50Mhz calibration clock for Altera reconfig cores
		locked => locked);      -- '1' when the PLL has locked

  
Dtack_to_SCUB <= io_port_Dtack_to_SCUB or dac1_dtack or dac2_dtack or adc_dtack;

SCU_Slave: SCU_Bus_Slave
generic map (
    CLK_in_Hz         =>  clk_sys_in_Hz,
    Firmware_Release	=>  0,
    Firmware_Version	=>  0,
    Hardware_Release	=>  0,
    Hardware_Version	=>  0,
    Intr_Edge_Trig    =>  "111111111111111",
    Intr_Enable   	  =>  "000000000000000",
    Intr_Level_Neg    =>  "000000000000000",
    Slave_ID          =>  scu_adda1_id)
port map (
    SCUB_Addr           =>  A_A,                    -- in,	SCU_Bus: address bus
    nSCUB_Timing_Cyc  	=>  A_nEvent_Str,           -- in,	SCU_Bus signal: low active SCU_Bus runs timing cycle
    SCUB_Data           =>  A_D,                    -- inout,	SCU_Bus: data bus (FPGA tri state buffer)
    nSCUB_Slave_Sel     =>  A_nBoardSel,            -- in,      SCU_Bus: '0' => SCU master select slave
    nSCUB_DS            =>  A_nDS,                  -- in,	SCU_Bus: '0' => SCU master activate data strobe
    SCUB_RDnWR          =>  A_RnW,                  -- in,      SCU_Bus: '1' => SCU master read slave
    clk                 =>  clk_sys,  
    nSCUB_Reset_in      =>  A_nReset,               -- in,	SCU_Bus-Signal: '0' => 'nSCUB_Reset_In' is active
    Data_to_SCUB        =>  Data_to_SCUB,           -- in,	connect read sources from external user functions
    Dtack_to_SCUB       =>  Dtack_to_SCUB,          -- in,	connect Dtack from from external user functions
    Intr_In             =>  "000000000000000",      -- in,	interrupt(15 downro 1)
    User_Ready          =>  '1',
    Data_from_SCUB_LA   =>  Data_from_SCUB_LA,      -- out,	latched data from SCU_Bus for external user functions 
    ADR_from_SCUB_LA    =>  ADR_from_SCUB_LA,       -- out,	latched address from SCU_Bus for external user functions
    Timing_Pattern_LA   =>  open,                   -- out,	latched timing pattern from SCU_Bus for external user functions
    Timing_Pattern_RCV  =>  open,                   -- out,	timing pattern received
    nSCUB_Dtack_Opdrn   =>  open,                   -- out,	for direct connect to SCU_Bus opendrain signal
                                                    --          '0' => slave give dtack to SCU master
    SCUB_Dtack          =>  SCUB_Dtack,             -- out,	for connect via ext. open collector driver
                                                    --          '1' => slave give dtack to SCU master
    nSCUB_SRQ_Opdrn     =>  open,                   -- out,	for direct connect to SCU_Bus opendrain signal
                                                    --          '0' => slave service request to SCU ma
    SCUB_SRQ            =>  SCUB_SRQ,               -- out,	for connect via ext. open collector driver
                                                    --          '1' => slave service request to SCU master
    nSel_Ext_Data_Drv   =>  A_nSel_Ext_Data_Drv,    -- out,	'0' => select the external data driver on the SCU_Bus slave
    Ext_Data_Drv_Rd	    =>  A_Ext_Data_RD,          -- out,	'1' => direction of the external data driver on the
                                                    --          SCU_Bus slave is to the SCU_Bus
    Standard_Reg_Acc    =>  open,                   -- out,	'1' => mark the access to register of this macro
    Ext_Adr_Val         =>  Ext_Adr_Val,            -- out,	for external user functions: '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active       =>  Ext_Rd_active,          -- out,	'1' => Rd-Cycle to external user register is active
    Ext_Rd_fin          =>  open,                   -- out,	marks end of read cycle, active one for one clock period
                                                    --          of clk past cycle end (no overlap)
    Ext_Rd_Fin_ovl      =>  open,                   -- out,	marks end of read cycle, active one for one clock period
                                                    --          of clk during cycle end (overlap)
    Ext_Wr_active       =>  Ext_Wr_active,          -- out,	'1' => Wr-Cycle to external user register is active
    Ext_Wr_fin          =>  open,                   -- out,	marks end of write cycle, active high for one clock period
                                                    --          of clk past cycle end (no overlap)
    Ext_Wr_fin_ovl      =>  Ext_Wr_fin_ovl,         -- out, marks end of write cycle, active high for one clock period
                                                    --          of clk before write cycle finished (with overlap) 
    Deb_SCUB_Reset_out	=>  open,                   -- out,	the debounced 'nSCUB_Reset_In'-signal, is active high,
                                                    --          can be used to reset
                                                    --          external macros, when 'nSCUB_Reset_In' is '0'
    nPowerup_Res        =>  nPowerup_Res);          -- out,	this macro generated a power up reset


dac_1: DAC_SPI
  generic map(
    Base_addr       => x"0200",
    CLK_in_Hz       => clk_sys_in_Hz,
    SPI_CLK_in_Hz   => dac_spi_clk_in_hz )
  port map(
    Adr_from_SCUB_LA    =>  ADR_from_SCUB_LA,       -- in, latched address from SCU_Bus
    Data_from_SCUB_LA   =>  Data_from_SCUB_LA,      -- in, latched data from SCU_Bus 
    Ext_Adr_Val         =>  Ext_Adr_Val,            -- in, '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active       =>  Ext_Rd_active,          -- in, '1' => Rd-Cycle is active
    Ext_Wr_active       =>  Ext_Wr_active,          -- in, '1' => Wr-Cycle is active
    Ext_Wr_fin          =>  Ext_Wr_fin_ovl,         -- in, '1' => Wr-Cycle is finished
    clk                 =>  clk_sys,                -- in, should be the same clk, used by SCU_Bus_Slave
    nReset              =>  nPowerup_Res,           -- in, '0' => resets the DAC_1
    DAC_SI              =>  DAC1_SDI,               -- out, is connected to DAC1-SDI
    nDAC_CLK            =>  nDAC1_CLK,              -- out, spi-clock of DAC1
    nCS_DAC             =>  nDAC1_A0,               -- out, '0' enable shift of internal shift register of DAC1
    nLD_DAC             =>  nDAC1_A1,               -- out, '0' copy shift register to output latch of DAC1
    nCLR_DAC            =>  nDAC1_CLR,              -- out, '0' set DAC1 to zero (pulse width min 200 ns)
    Rd_Port             =>  dac1_data_to_SCUB,      -- out, connect read sources (over multiplexer) to SCUB-Macro
    Rd_Activ            =>  dac1_rd_active,         -- out, '1' = read data available at 'Data_to_SCUB'-output
    Dtack               =>  dac1_dtack
    );

    
dac_2: DAC_SPI
  generic map(
    Base_addr       => x"0210",
    CLK_in_Hz       => clk_sys_in_Hz,
    SPI_CLK_in_Hz   => dac_spi_clk_in_hz )
  port map(
    Adr_from_SCUB_LA    =>  ADR_from_SCUB_LA,       -- in, latched address from SCU_Bus
    Data_from_SCUB_LA   =>  Data_from_SCUB_LA,      -- in, latched data from SCU_Bus 
    Ext_Adr_Val         =>  Ext_Adr_Val,            -- in, '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active       =>  Ext_Rd_active,          -- in, '1' => Rd-Cycle is active
    Ext_Wr_active       =>  Ext_Wr_active,          -- in, '1' => Wr-Cycle is active
    Ext_Wr_fin          =>  Ext_Wr_fin_ovl,         -- in, '1' => Wr-Cycle is finished
    clk                 =>  clk_sys,                -- in, should be the same clk, used by SCU_Bus_Slave
    nReset              =>  nPowerup_Res,           -- in, '0' => resets the DAC_2
    DAC_SI              =>  DAC2_SDI,               -- out, is connected to DAC2-SDI
    nDAC_CLK            =>  nDAC2_CLK,              -- out, spi-clock of DAC2
    nCS_DAC             =>  nDAC2_A0,               -- out, '0' enable shift of internal shift register of DAC2
    nLD_DAC             =>  nDAC2_A1,               -- out, '0' copy shift register to output latch of DAC2
    nCLR_DAC            =>  nDAC2_CLR,              -- out, '0' set DAC2 to zero (pulse width min 200 ns)
    Rd_Port             =>  dac2_data_to_SCUB,      -- out, connect read sources (over multiplexer) to SCUB-Macro
    Rd_Activ            =>  dac2_rd_active,         -- out, '1' = read data available at 'Data_to_SCUB'-output
    Dtack               =>  dac2_dtack
    );


io_port:  IO_4x8
  generic map (
    Base_addr   => x"0220")
  port map (
    Adr_from_SCUB_LA    =>  ADR_from_SCUB_LA,       -- in, latched address from SCU_Bus
    Data_from_SCUB_LA   =>  Data_from_SCUB_LA,      -- in, latched data from SCU_Bus 
    Ext_Adr_Val         =>  Ext_Adr_Val,            -- in, '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active       =>  Ext_Rd_active,          -- in, '1' => Rd-Cycle is active
    Ext_Wr_active       =>  Ext_Wr_active,          -- in, '1' => Wr-Cycle is active
    clk                 =>  clk_sys,                -- in, should be the same clk, used by SCU_Bus_Slave
    nReset              =>  nPowerup_Res,           -- in, '0' => resets the IO_4x8
    io                  =>  a_io,                   -- inout, select and set direction only in 8-bit partitions
    io_7_0_tx           =>  a_io_7_0_tx,            -- out, '1' = external io(7..0)-buffer set to output.
    ext_io_7_0_dis      =>  a_ext_io_7_0_dis,       -- out, '1' = disable external io(7..0)-buffer.
    io_15_8_tx          =>  a_io_15_8_tx,           -- out, '1' = external io(15..8)-buffer set to output
    ext_io_15_8_dis     =>  a_ext_io_15_8_dis,      -- out, '1' = disable external io(15..8)-buffer.
    io_23_16_tx         =>  a_io_23_16_tx,          -- out, '1' = external io(23..16)-buffer set to output.
    ext_io_23_16_dis    =>  a_ext_io_23_16_dis,     -- out, '1' = disable external io(23..16)-buffer.
    io_31_24_tx         =>  a_io_31_24_tx,          -- out, '1' = external io(31..24)-buffer set to output
    ext_io_31_24_dis    =>  a_ext_io_31_24_dis,     -- out, '1' = disable external io(31..24)-buffer.
    user_rd_active      =>  io_port_rd_active,      -- out, '1' = read data available at 'Data_to_SCUB'-output
    Data_to_SCUB        =>  io_port_data_to_SCUB,   -- out, connect read sources to SCUB-Macro
    Dtack_to_SCUB       =>  io_port_Dtack_to_SCUB); -- out, connect Dtack to SCUB-Macro

    
adc: adc_scu_bus
  generic map (
    Base_addr     => x"0230",
    clk_in_Hz     => clk_sys_in_Hz,
    diag_on_is_1  => 0)
  port map (
    clk           =>  clk_sys,
    nrst          =>  nPowerup_Res,
    
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
    
    Adr_from_SCUB_LA  => ADR_from_SCUB_LA,
    Data_from_SCUB_LA => Data_from_SCUB_LA,
    Ext_Adr_Val       => Ext_Adr_Val,
    Ext_Rd_active     => Ext_Rd_active,
    Ext_Wr_active     => Ext_Wr_active,
    user_rd_active    => adc_rd_active,
    Data_to_SCUB      => adc_data_to_SCUB,
    Dtack_to_SCUB     => adc_dtack);


modelsim_nPowerup_Res <= not nPowerup_Res;

p_led_ena:  div_n
  generic map (
    n       => clk_sys_in_Hz / 100, -- div_o is every 10 ms for one clock period active
    diag_on => 0)
  port map (
    res     => modelsim_nPowerup_Res, -- in, '1' => set "div_n"-counter asynchron to generic-value "n"-2, so the 
                                    --     countdown is "n"-1 clocks to activate the "div_o"-output for one clock periode. 
    clk     => clk_sys,             -- clk = clock
    ena     => '1',                 -- in, can be used for a reduction, signal should be generated from the same 
                                    --     clock domain and should be only one clock period active.
    div_o   => led_ena_cnt);        -- out, div_o becomes '1' for one clock period, if "div_n" arrive n-1 clocks
                                    --      (if ena is permanent '1').


p_test_port_mux: process (
    DAC1_SDI, nDAC1_CLK, nDAC1_A0, nDAC1_A1, nDAC1_CLR, dac1_rd_active, dac1_dtack,
    DAC2_SDI, nDAC2_CLK, nDAC2_A0, nDAC2_A1, nDAC2_CLR, dac2_rd_active, dac2_dtack,
    ADC_Range, ADC_FRSTDATA,
    ADC_CONVST_A, ADC_CONVST_B, nADC_CS, nADC_RD_SCLK, ADC_BUSY, ADC_RESET, ADC_OS, nADC_PAR_SER_SEL, ADC_DB(15 downto 0),
    A_SEL(3 downto 0)
    )
  begin
    case not A_SEL IS

      when X"0" =>
        A_TA <= '0' & DAC2_SDI & nDAC2_CLK & nDAC2_A0 & nDAC2_A1 & nDAC2_CLR & dac2_rd_active & dac2_dtack &
                '0' & DAC1_SDI & nDAC1_CLK & nDAC1_A0 & nDAC1_A1 & nDAC1_CLR & dac1_rd_active & dac1_dtack;
        A_TB <= X"0000";

      when X"1" =>
        A_TA <= X"0" & ADC_Range & ADC_FRSTDATA & ADC_CONVST_A & ADC_CONVST_B &
                nADC_CS & nADC_RD_SCLK & ADC_BUSY & ADC_RESET & ADC_OS & nADC_PAR_SER_SEL;
        A_TB <= ADC_DB(15 downto 0);

      when others =>
        A_TA <= (others => '0');
        A_TB <= (others => '0');
    end case;

  end process p_test_port_mux;


p_led_mux: process (
    ADC_channel_1, ADC_channel_2, ADC_channel_3, ADC_channel_4,
    ADC_channel_5, ADC_channel_6, ADC_channel_7, ADC_channel_8,
    A_ADC_DAC_SEL(3 downto 0)
    )
  begin
    case not A_ADC_DAC_SEL IS
      when X"0" => A_nLED <= not ADC_channel_1;
      when X"1" => A_nLED <= not ADC_channel_2;
      when X"2" => A_nLED <= not ADC_channel_3;
      when X"3" => A_nLED <= not ADC_channel_4;
      when X"4" => A_nLED <= not ADC_channel_5;
      when X"5" => A_nLED <= not ADC_channel_6;
      when X"6" => A_nLED <= not ADC_channel_7;
      when X"7" => A_nLED <= not ADC_channel_8;
      when others =>
        A_nLED <= (others => '1');
    end case;
  end process p_led_mux;
 

p_read_mux: process (
    io_port_rd_active, io_port_data_to_SCUB,
    dac1_rd_active, dac1_data_to_SCUB,
    dac2_rd_active, dac2_data_to_SCUB,
    adc_rd_active, adc_data_to_SCUB
    )
  variable  sel:  unsigned(3 downto 0);
  begin
    sel := adc_rd_active & dac2_rd_active & dac1_rd_active & io_port_rd_active;
    case sel IS
      when "0001" => Data_to_SCUB <= io_port_data_to_SCUB;
      when "0010" => Data_to_SCUB <= dac1_data_to_SCUB;
      when "0100" => Data_to_SCUB <= dac2_data_to_SCUB;
      when "1000" => Data_to_SCUB <= adc_data_to_SCUB;
      when others =>
        Data_to_SCUB <= X"0000";
    end case;
  end process p_read_mux;
  

modelsim_A_nBoardSel <= not A_nBoardSel; -- modelsim can't use not ...;
  
sel_led: led_n
  generic map (
    stretch_cnt => 3)
  port map (
    ena         => led_ena_cnt,     -- is every 10 ms for one clock period active
    clk         => clk_sys,
    Sig_in      => modelsim_A_nBoardSel,
    nLED        => open,
    nLED_opdrn  => A_nState_LED(0));

    
dtack_led: led_n
  generic map (
    stretch_cnt => 3)
  port map
    (
    ena         => led_ena_cnt,     -- is every 10 ms for one clock period active
    clk         => clk_sys,
    Sig_in      => SCUB_Dtack,
    nLED        => open,
    nLED_opdrn  => A_nState_LED(1));
    
rw_signal <= not A_RnW and not A_nBoardSel;

rw_led: led_n
  generic map (
    stretch_cnt => 3)
  port map (
    ena         => led_ena_cnt,     -- is every 10 ms for one clock period active
    clk         => clk_sys,
    Sig_in      => rw_signal,
    nLED        => open,
    nLED_opdrn  => A_nState_LED(2));
    
  A_nDtack  <= not SCUB_Dtack;
  A_nSRQ    <= not SCUB_SRQ;
end architecture;
