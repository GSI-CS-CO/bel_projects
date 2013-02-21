library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wr_altera_pkg.all;
use work.scu_bus_slave_pkg.all;
use work.aux_functions_pkg.all;

entity scu_addac is
  port
    (
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
                                                                --  Slave (besser default 0, oder Treiber A/B tauschen)
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
          
    A_nState_LED:         out   std_logic_vector(2 downto 0)    -- ..LED(2) = R/W, ..LED(1) = Dtack, ..LED(0) = Sel
  );
end entity;



architecture scu_addac_arch of scu_addac is

constant  scu_adda1_id:   integer range 16#0210# to 16#021F# := 16#0210#;
constant  clk_sys_in_Hz:  integer := 65_500_000;


component IO_4x8
  generic
    (
    Base_addr:  integer range 1 to 16#ffff# := 16#200#
    );
    
  port
    (
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
  

  signal clk_sys, clk_cal, rstn, locked : std_logic;
  
  signal  SCUB_SRQ:           std_logic;
  signal  SCUB_Dtack:         std_logic;
  
  signal  user_rd_active:     std_logic;     
  signal  Data_to_SCUB:       std_logic;       
  signal  Dtack_to_SCUB:      std_logic;

  signal  io_port_Dtack_to_SCUB:  std_logic;
  signal  io_port_data_to_SCUB:   std_logic_vector(15 downto 0);
  signal  io_port_rd_active:      std_logic;
  
  signal  ADR_from_SCUB_LA:   std_logic_vector(15 downto 0);
  signal  Data_from_SCUB_LA:  std_logic_vector(15 downto 0);
  signal  Ext_Adr_Val:        std_logic;
  signal  Ext_Rd_active:      std_logic;
  signal  Ext_Wr_active:      std_logic;
  signal  nPowerup_Res:       std_logic;
  
  signal  led_ena_cnt:        std_logic;

  begin

  

-- Obtain core clocking
sys_pll_inst: sys_pll      -- Altera megafunction
  port map
    (
    inclk0 => CLK_FPGA,     -- 125Mhz oscillator from board
    c0     => clk_sys,      -- 62.5MHz system clk (cannot use external pin as clock for RAM blocks)
    c1     => clk_cal,      -- 50Mhz calibration clock for Altera reconfig cores
    locked => locked        -- '1' when the PLL has locked
    );



SCU_Slave: SCU_Bus_Slave
generic map
    (
    CLK_in_Hz         =>  clk_sys_in_Hz,
    Firmware_Release  =>  0,
    Firmware_Version  =>  0,
    Hardware_Release  =>  0,
    Hardware_Version  =>  0,
    Intr_Edge_Trig    =>  "111111111111111",
    Intr_Enable       =>  "000000000000000",
    Intr_Level_Neg    =>  "000000000000000",
    Slave_ID          =>  scu_adda1_id
    )
port map
    (
    SCUB_Addr           =>  A_A,                    -- in,    SCU_Bus: address bus
    nSCUB_Timing_Cyc    =>  A_nEvent_Str,           -- in,    SCU_Bus signal: low active SCU_Bus runs timing cycle
    SCUB_Data           =>  A_D,                    -- inout, SCU_Bus: data bus (FPGA tri state buffer)
    nSCUB_Slave_Sel     =>  A_nBoardSel,            -- in,    SCU_Bus: '0' => SCU master select slave
    nSCUB_DS            =>  A_nDS,                  -- in,    SCU_Bus: '0' => SCU master activate data strobe
    SCUB_RDnWR          =>  A_RnW,                  -- in,    SCU_Bus: '1' => SCU master read slave
    clk                 =>  clk_sys,
    nSCUB_Reset_in      =>  A_nReset,               -- in,    SCU_Bus-Signal: '0' => 'nSCUB_Reset_In' is active
    Data_to_SCUB        =>  io_port_data_to_SCUB,   -- in,    connect read sources from external user functions
    Dtack_to_SCUB       =>  io_port_Dtack_to_SCUB,  -- in,    connect Dtack from from external user functions
    Intr_In             =>  "000000000000000",      -- in,    interrupt(15 downro 1)
    User_Ready          =>  '1',
    Data_from_SCUB_LA   =>  Data_from_SCUB_LA,      -- out,   latched data from SCU_Bus for external user functions 
    ADR_from_SCUB_LA    =>  ADR_from_SCUB_LA,       -- out,   latched address from SCU_Bus for external user functions
    Timing_Pattern_LA   =>  open,                   -- out,   latched timing pattern from SCU_Bus for external user functions
    Timing_Pattern_RCV  =>  open,                   -- out,   timing pattern received
    nSCUB_Dtack_Opdrn   =>  open,                   -- out,   for direct connect to SCU_Bus opendrain signal
                                                    --        '0' => slave give dtack to SCU master
    SCUB_Dtack          =>  SCUB_Dtack,             -- out,   for connect via ext. open collector driver
                                                    --        '1' => slave give dtack to SCU master
    nSCUB_SRQ_Opdrn     =>  open,                   -- out,   for direct connect to SCU_Bus opendrain signal
                                                    --        '0' => slave service request to SCU ma
    SCUB_SRQ            =>  SCUB_SRQ,               -- out,   for connect via ext. open collector driver
                                                    --        '1' => slave service request to SCU master
    nSel_Ext_Data_Drv   =>  A_nSel_Ext_Data_Drv,    -- out,   '0' => select the external data driver on the SCU_Bus slave
    Ext_Data_Drv_Rd     =>  A_Ext_Data_RD,          -- out,   '1' => direction of the external data driver on the
                                                    --        SCU_Bus slave is to the SCU_Bus
    Standard_Reg_Acc    =>  open,                   -- out,   '1' => mark the access to register of this macro
    Ext_Adr_Val         =>  Ext_Adr_Val,            -- out,   for external user functions: '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active       =>  Ext_Rd_active,          -- out,   '1' => Rd-Cycle to external user register is active
    Ext_Rd_fin          =>  open,                   -- out,   marks end of read cycle, active one for one clock period
                                                    --        of clk past cycle end (no overlap)
    Ext_Rd_Fin_ovl      =>  open,                   -- out,   marks end of read cycle, active one for one clock period
                                                    --        of clk during cycle end (overlap)
    Ext_Wr_active       =>  Ext_Wr_active,          -- out,   '1' => Wr-Cycle to external user register is active
    Ext_Wr_fin          =>  open,                   -- out,   marks end of write cycle, active one for one clock period
                                                    --        of clk past cycle end (no overlap)
    Ext_Wr_fin_ovl      =>  open,
    Deb_SCUB_Reset_out  =>  open,                   -- out,   the debounced 'nSCUB_Reset_In'-signal, is active high,
                                                    --        can be used to reset
                                                    --        external macros, when 'nSCUB_Reset_In' is '0'
    nPowerup_Res        =>  nPowerup_Res            -- out,   this macro generated a power up reset
    );



io_port:  IO_4x8
  generic map
    (
    Base_addr   => 16#200#
    )
  port map
    (
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
    Dtack_to_SCUB       =>  io_port_Dtack_to_SCUB   -- out, connect Dtack to SCUB-Macro
    );  


p_led_ena:  div_n
  generic map
    (
    n       => clk_sys_in_Hz / 100, -- div_o is every 10 ms for one clock period active
    diag_on => 0
    )
  port map
    (
    res     => not nPowerup_Res,    -- in, '1' => set "div_n"-counter asynchron to generic-value "n"-2, so the 
                                    --     countdown is "n"-1 clocks to activate the "div_o"-output for one clock periode. 
    clk     => clk_sys,             -- clk = clock
    ena     => '1',                 -- in, can be used for a reduction, signal should be generated from the same 
                                    --     clock domain and should be only one clock period active.
    div_o   => led_ena_cnt          -- out, div_o becomes '1' for one clock period, if "div_n" arrive n-1 clocks
                                    --      (if ena is permanent '1').
    );

  
sel_led: led_n
  generic map
    (
    stretch_cnt => 3
    )
  port map
    (
    ena         => led_ena_cnt,     -- is every 10 ms for one clock period active
    clk         => clk_sys,
    Sig_in      => not A_nBoardSel,
    nLED        => open,
    nLED_opdrn  => A_nState_LED(0)
    );
    
dtack_led: led_n
  generic map
    (
    stretch_cnt => 3
    )
  port map
    (
    ena         => led_ena_cnt,     -- is every 10 ms for one clock period active
    clk         => clk_sys,
    Sig_in      => SCUB_Dtack,
    nLED        => open,
    nLED_opdrn  => A_nState_LED(1)
    );
    
rw_led: led_n
  generic map
    (
    stretch_cnt => 3
    )
  port map
    (
    ena         => led_ena_cnt,     -- is every 10 ms for one clock period active
    clk         => clk_sys,
    Sig_in      => not A_RnW,
    nLED        => open,
    nLED_opdrn  => A_nState_LED(2)
    );
    
  A_nDtack <= not SCUB_Dtack;
  A_nSRQ <= not SCUB_SRQ;


end architecture;