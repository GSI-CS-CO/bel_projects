

LIBRARY ieee;
use ieee.std_logic_1164.all;
use IEEE.numeric_std.all;
use work.scu_bus_slave_pkg.all;
use work.scu_sio3_pkg.all;
use work.sio3_sys_clk_local_clk_switch_pkg.all;
use work.aux_functions_pkg.all;


--+-----------------------------------------------------------------------------------------------------------------+
--|  "scu_sio3"    Autor: K.Kaiser (based on W.Panschow SIO from 2012)                                               |
--|                                                                                                                  |
--|                                                                                                                  |
--|   Supports Device Bus Interface and Timing Input for Multi Mil as well as 4 LEMO I/Os                            |
--|   Introduced for enabling SCU as a substituion for Legacy (device-bus controlled) infrastructure.                |
--|   V.1 Initial Version from 2015-Jul-16                                                                           |
--+-----------------------------------------------------------------------------------------------------------------+




ENTITY scu_sio3 IS 
generic (
    CLK_in_Hz:      integer := 125000000
    );
port  (
    nCB_RESET:        in      std_logic;  --PIN_R3    EXTCON1
    clk_fpga:         in      std_logic;  --PIN_Y13   CLKFPGAp 125MHz
    F_PLL_5  :        in      std_logic;  --PIN_N4    F_PLL_5p EXTCON1: On SCU this Pin is an output 
    GP_Ext_Out_B:     in      std_logic;  --PIN_R7    EXTCON2

------------------------------------Device-Bus (Externer Manchester Decoder HD6408) ----------------------------------------------------
    A_ME_DSC:         in      std_logic;  --PIN_B19   Decoder Shift Clock
    A_ME_VW:          in      std_logic;  --PIN_D17   Valid Word when high
    A_ME_CDS:         in      std_logic;  --PIN_F16   Command Data Sync Low=DataSync, high=CommandSync was transmitted before data
    A_ME_TD:          in      std_logic;  --PIN_C18   Take Data high=valid data and 2 sync pulses received 
    A_ME_ESC:         in      std_logic;  --PIN_E16   Encoder Shift Clock
    A_ME_SD:          in      std_logic;  --PIN_AC13  Send Data high=external data source enabled
    A_ME_nBOO:        in      std_logic;  --PIN_AC12  Bip One Out active low drives a one
    A_ME_nBZO:        in      std_logic;  --PIN_AB13  Bip Zero Out active low drives a zero
    A_MIL1_BOI:       in      std_logic;  --PIN_AB12  Bip One In  
    A_MIL1_BZI:       in      std_logic;  --PIN_AA6   Bip Zero In
    A_UMIL15V:        in      std_logic;  --PIN_W8    Indicates 15V ok (using 26LS33 Line Receiver)
    A_UMIL5V:         in      std_logic;  --PIN_Y7    Indicates 3.3V is ok, not used internally
    A_nOPK_INL:       in      std_logic;  --PIN_A8    from Interlock line
    A_nOPK_DRDY:      in      std_logic;  --PIN_A7    from DRDY line
    A_nOPK_DRQ:       in      std_logic;  --PIN_D9    from DRQ Line
    A_ME_SDO:         in      std_logic;  --PIN_G16   Serial Data out NRZ
    A_ME_SDI:         out     std_logic;  --PIN_A18   Serial Data In clocked with A_ME_ESC
    A_ME_EE:          out     std_logic;  --PIN_F15   Encoder Enable high starts encode cycle
    A_ME_SS:          out     std_logic;  --PIN_A17   Sync Select High=CommandSync, Low=DataSync 
    A_ME_12MHZ:       out     std_logic;  --PIN_F8    12p5MHz to Encoder/Decoder Shift Clock of HD6408
    A_ME_BOI:         out     std_logic;  --PIN_C16   Data To HD6408 BOI
    A_ME_BZI:         out     std_logic;  --PIN_A16   Data To HD6408 BZI
    A_ME_UDI:         out     std_logic;  --PIN_B16   To HD6408 UDI, Unipolar Data when set to low
    A_MIL1_nBOO:      buffer  std_logic;  --PIN_A20   Data To Line Driver 75472
    A_MIL1_nBZO:      buffer  std_logic;  --PIN_E15   Data To Line Driver 75472
    A_MIL1_Out_Ena:   buffer  std_logic;  --PIN_C19   low enables Line Driver 75472 to forward activelow data
    A_MIL1_nIN_Ena:   out     std_logic;  --PIN_N7    low enables 26LS33, h puts 26LS33 Y outputs to Z
    clk_20:           in      std_logic;  --PIN_P4    20MHz onboard XCO
--------------------------------------- Galvanisch entkoppeltes LEMO-IO -------------------------------------------------------------------
    A_LEMO_1_IO:      inout   std_logic;  --PIN_AC6
    A_LEMO_1_EN_IN:   out     std_logic;  --PIN_B18   '1' => A_LEMO_1_IO ist Eingang; '0' A_LEMO_1_IO ist Ausgang
    A_nLEMO_1_LED:    out     std_logic;  --PIN_G9    für Aktivitätsanzeige von A_LEMO_1_IO vorgesehen
    A_LEMO_2_IO:      inout   std_logic;  --PIN_AA9
    A_LEMO_2_EN_IN:   out     std_logic;  --PIN_A19   '1' => A_LEMO_2_IO ist Eingang; '0' A_LEMO_2_IO ist Ausgang
    A_nLEMO_2_LED:    out     std_logic;  --PIN_H9    für Aktivitätsanzeige von A_LEMO_2_IO vorgesehen
    A_LEMO_3_IO:      inout   std_logic;  --PIN_AB6
    A_LEMO_3_EN_IN:   out     std_logic;  --PIN_C20   '1' => A_LEMO_3_IO ist Eingang; '0' A_LEMO_3_IO ist Ausgang
    A_nLEMO_3_LED:    out     std_logic;  --PIN_B4    für Aktivitätsanzeige von A_LEMO_3_IO vorgesehen
    A_LEMO_4_IO:      inout   std_logic;  --PIN_Y9
    A_LEMO_4_EN_IN:   out     std_logic;  --PIN_H16   '1' => A_LEMO_4_IO ist Eingang; '0' A_LEMO_4_IO ist Ausgang
    A_nLEMO_4_LED:    out     std_logic;  --PIN_C4    für Aktivitätsanzeige von A_LEMO_4_IO vorgesehen
  
---------------------------------------- OneWire --------------------------------------------------------------------------------------------------
    A_OneWire_EEPROM: inout   std_logic;  --PIN_R1    OneWire-EEPROM     DS28EC20
    A_OneWire:        inout   std_logic;  --PIN_T1    OneWire-ADC        DS2450 
    
 ---------------------------------------- Parallele SCU-Bus-Signale -------------------------------------------------------------------------------
    A_nReset:         in      std_logic;  --PIN_N6            Reset from SCU Bus
    A_A:              in      std_logic_vector(15 downto 0);  --SCU-Adressbus
    A_nADR_EN:        out     std_logic;  --PIN_AA8           '0' => externe Adresstreiber des Slaves aktiv
    A_nADR_SCUB:      out     std_logic;  --PIN_V9            '0' => externe Adresstreiber-Richtung: SCU-Bus nach Slave  
    A_D:              inout   std_logic_vector(15 downto 0);  --SCU-Datenbus
    A_nDS:            in      std_logic;  --PIN_U1            Data-Strobe vom Master gertieben
    A_RnW:            in      std_logic;  --PIN_Y1            Schreib/Lese-Signal vom Master getrieben, '0' => lesen
    A_nSel_Ext_Data_Drv:out   std_logic;  --PIN_AB8           '0' => externe Datentreiber des Slaves aktiv
    A_Ext_Data_RD:    out     std_logic;  --PIN_AA7           '0' => externe Datentreiber-Richtung: SCU-Bus nach Slave (besser default 0, oder Treiber A/B tauschen)
    A_nSEL_Ext_Signal_RDV:out std_logic;  --PIN_U9            '0' => Treiber für SCU-Bus-Steuer-Signale aktiv
    A_nExt_Signal_In: out     std_logic;  --PIN_AD6           '0' => Treiber für SCU-Bus-Steuer-Signale-Richtung: SCU-Bus nach Slave (besser default 0, oder Treiber A/B tauschen)
    A_nDtack:         out     std_logic;  --PIN_AD8           Data-Acknowlege null aktiv, '0' => aktiviert externen Opendrain-Treiber
    A_nSRQ:           out     std_logic;  --PIN_W9            Service-Request null aktiv, '0' => aktiviert externen Opendrain-Treiber
    A_nBoardSel:      in      std_logic;  --PIN_V1            '0' => Master aktiviert diesen Slave
    A_nEvent_Str:     in      std_logic;  --PIN_E9            '0' => Master sigalisiert Timing-Zyklus
    A_SysClock:       in      std_logic;  --PIN_AD13          SCU Bus Clock 12.5MHz vom Master getrieben.
    A_Spare0:         in      std_logic;  --PIN_U3            vom Master getrieben
    A_Spare1:         in      std_logic;  --PIN_U4            vom Master getrieben

----------------------------------------- Logikanalysator Ports ------------------------------------------------------------------------------------
    A_TA:             inout   std_logic_vector(15 downto 0);  --Logikanalysator Port A
    A_TB:             inout   std_logic_vector(15 downto 0);  --Logikanalysator Port B
    A_SEL:            in      std_logic_vector(3 downto 0);   --Hex-Schalter zur Auswahl von Signalgruppen des Logikanalysator Ports

----------------------------------------- Frontplatten-LEDs (2,5 Volt-IO) --------------------------------------------------------------------------
    A_nLED:           out     std_logic_vector(15 downto 0);
       
----------------------------------------- 3,3 Volt-IO zum Carrierboard des SCU-Bus-Masters ---------------------------------------------------------
    EIO:              inout   std_logic_vector(17 downto 0);
----------------------------------------- 2,5 Volt-IO zum Carrierboard des SCU-Bus-Masters (LVDS möglch) -------------------------------------------
    IO_2_5V:          inout   std_logic_vector(15 downto 0);
----------------------------------------- Altes GSI Timing -----------------------------------------------------------------------------------------
    A_Timing:         in      std_logic;                       -- From LEMO (Opto coupler)
    A_nLED_Timing:    out     std_logic;                       -- LED-Signalisierung, dass Timing empfangen wird

    GP_Ext_In:        out     std_logic;                       -- AD7 zu Ext. Connector
    nExtension_Res_Out:out    std_logic                        -- A87  steuert Reset Baustein TPS3307 an
    );
end scu_sio3;


ARCHITECTURE arch_scu_sio3 OF scu_sio3 IS


CONSTANT c_Firmware_Version:         Integer              := 5;         -- important: => Firmware_Version
CONSTANT c_Firmware_Release:         Integer              := 2;         -- important: => Firmware_Release

CONSTANT c_housekeeping_base:        unsigned(15 downto 0):=  x"0040";  -- housekeeping/LM32
CONSTANT c_test_usr_reg_Base_Addr:   integer              :=  16#0200#; -- Test Usr Reg
CONSTANT c_wb_mil_wrapper_Base_Addr: integer              :=  16#0400#; -- Mil Wrapper

constant  stretch_cnt:               integer := 5;

signal    ADR_from_SCUB_LA:       std_logic_vector(15 downto 0);
signal    clk:                    std_logic;
signal    Data_from_SCUB_LA:      std_logic_vector(15 downto 0);
signal    Data_to_SCUB:           std_logic_vector(15 downto 0);
signal    Dtack_to_SCUB:          std_logic;
signal    Ena_Every_100ns:        std_logic;
signal    Ena_Every_166ns:        std_logic;
signal    Ena_Every_20ms:         std_logic;
signal    Ext_Adr_Val:            std_logic;
signal    Ext_Rd_active:          std_logic;
signal    Ext_Rd_fin:             std_logic;
signal    Ext_Wr_active:          std_logic;
signal    Ext_Wr_fin:             std_logic;
signal    la_clk:                 std_logic;
signal    pll_1_locked:           std_logic;
signal    pll_2_locked:           std_logic;
signal    SysClock_pll_out:       std_logic;
signal    nPowerup_Res:           std_logic;
signal    SCU_Dtack:              std_logic;
signal    SCUB_SRQ:               std_logic;
signal    User1_Reg:              std_logic_vector(15 downto 0);

signal    user_reg1_Dtack:        std_logic;
signal    user_reg_rd_active:     std_logic;
signal    user_reg1_data_to_SCUB: std_logic_vector(15 downto 0);


signal    Timing_Pattern_RCV:     std_logic;
signal    Timing_Pattern_LA:      std_logic_vector(31 downto 0);

signal    test_port_in_0:         std_logic_vector(31 downto 0);

signal    Mil_Data_to_SCUB:       std_logic_vector(15 downto 0);
signal    mil_rd_active:          std_logic;
signal    mil_Dtack_to_SCUB:      std_logic;

signal    nSel_Mil_Drv:           std_logic;

signal    Deb_SCUB_Reset_out:     std_logic;

signal    Mil_Trm_Rdy:            std_logic;
signal    Mil_Rcv_Rdy:            std_logic;
signal    Mil_Rcv_Error:          std_logic;
signal    No_VW_Cnt:              std_logic_vector(15 downto 0);  
signal    Not_Equal_Cnt:          std_logic_vector(15 downto 0);
signal    Mil_Decoder_Diag_p:     std_logic_vector(15 downto 0);
signal    Mil_Decoder_Diag_n:     std_logic_vector(15 downto 0);


signal    ready:                  std_logic;

signal    Powerup_Res:            std_logic;  -- only for modelsim!
--signal    nMil_Trm_Rdy:         std_logic;  -- only for modelsim!
--signal    nMil_Rcv_Rdy:         std_logic;  -- only for modelsim!
signal    s_intr_in:              std_logic_vector(15 downto 1); 
signal    nMil_Rcv_Error:         std_logic;  -- only for modelsim!

signal    dly_intr_o:             std_logic;
signal    Interlock_Intr_o:       std_logic;
signal    Data_Rdy_Intr_o:        std_logic;
signal    Data_Req_Intr_o:        std_logic;
signal    nLed_Mil_Rcv_error:     std_logic;
signal    nLed_Mil_trm:           std_logic; 
signal    lemo_data_o:            std_logic_vector(4 downto 1);
signal    lemo_nled_o:            std_logic_vector(4 downto 1);
signal    lemo_out_en_o:          std_logic_vector(4 downto 1);
signal    lemo_data_i:            std_logic_vector(4 downto 1);
--signal    Sel_Mil_Drv:          std_logic;
signal    sys_clk_is_bad:         std_logic;
signal    sys_clk_is_bad_led_n:   std_logic;
signal    sys_clk_is_bad_la:      std_logic;
--signal    local_clk_is_bad:     std_logic;
signal    local_clk_is_running:   std_logic;
signal    local_clk_runs_led_n:   std_logic;

signal    sys_clk_deviation:      std_logic;
signal    sys_clk_deviation_la:   std_logic;
signal    sys_clk_deviation_led_n: std_logic;

signal    clk_switch_rd_data:     std_logic_vector(15 downto 0);
signal    clk_switch_rd_active:   std_logic;
signal    clk_switch_dtack:       std_logic;
signal    clk_switch_intr:        std_logic;
--signal  signal_tap_clk_250mhz:  std_logic;  
signal    owr_pwren_o:            std_logic_vector(1 downto 0);
signal    owr_en_o:               std_logic_vector(1 downto 0);
signal    owr_i:                  std_logic_vector(1 downto 0);
signal    wb_scu_rd_active:       std_logic;
signal    wb_scu_dtack:           std_logic;
signal    wb_scu_data_to_SCUB:    std_logic_vector(15 downto 0);
signal    io_port_rd_active:      std_logic;
signal    io_port_data_to_SCUB:   std_logic_vector(15 downto 0);
signal    tmr_rd_active:          std_logic;
signal    tmr_data_to_SCUB:       std_logic_vector(15 downto 0);
signal    nLED_MIL_Rcv_Rdy:       std_logic;
signal    ev_fifo_ne_intr_o:      std_logic;
signal    every_ms_intr_o:        std_logic;


BEGIN 

nExtension_Res_Out      <= '1';
A_nADR_EN               <= '0';
A_nADR_SCUB             <= '0';
A_nExt_Signal_In        <= '0';
A_nSEL_Ext_Signal_RDV   <= '0';

GP_Ext_In <= 'Z';

-- MIL  LEMO Control  
A_LEMO_1_IO      <= lemo_data_o(1) when lemo_out_en_o(1)='1' else 'Z';
A_LEMO_2_IO      <= lemo_data_o(2) when lemo_out_en_o(2)='1' else 'Z';
A_LEMO_3_IO      <= lemo_data_o(3) when lemo_out_en_o(3)='1' else 'Z';
A_LEMO_4_IO      <= lemo_data_o(4) when lemo_out_en_o(4)='1' else 'Z';

A_LEMO_1_EN_IN   <= not lemo_out_en_o(1); --A_LEMO_x_EN_IN = low = Lemo is output
A_LEMO_2_EN_IN   <= not lemo_out_en_o(2); 
A_LEMO_3_EN_IN   <= not lemo_out_en_o(3); 
A_LEMO_4_EN_IN   <= not lemo_out_en_o(4); 

A_nLEMO_1_LED    <= lemo_nled_o(1);
A_nLEMO_2_LED    <= lemo_nled_o(2);
A_nLEMO_3_LED    <= lemo_nled_o(3);
A_nLEMO_4_LED    <= lemo_nled_o(4);

lemo_data_i(1)   <= A_LEMO_1_IO;
lemo_data_i(2)   <= A_LEMO_2_IO;
lemo_data_i(3)   <= A_LEMO_3_IO;
lemo_data_i(4)   <= A_LEMO_4_IO;

A_ME_UDI <= '0';

Powerup_Res       <= not nPowerup_Res;  -- only for modelsim!
clk_switch_intr  <= sys_clk_is_bad_la or sys_clk_deviation_la;


user_reg1: sio3_Test_User_Reg  
  generic map(
    Base_addr  =>  c_test_usr_reg_Base_Addr
    )
  port map  (
    Adr_from_SCUB_LA      =>  ADR_from_SCUB_LA,                   -- latched address from SCU_Bus  
    Data_from_SCUB_LA     =>  Data_from_SCUB_LA,                  -- latched data from SCU_Bus 
    Ext_Adr_Val           =>  Ext_Adr_Val,                        -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active         =>  Ext_Rd_active,                      -- '1' => Rd-Cycle is active
    Ext_Rd_fin            =>  Ext_Rd_fin,                         -- marks end of read cycle,  active high for one clock period of sys_cl
    Ext_Wr_active         =>  Ext_Wr_active,                      -- '1' => Wr-Cycle is active
    Ext_Wr_fin            =>  Ext_Wr_fin,                         -- marks end of write cycle,  active high for one clock period of sys_cl
    clk                   =>  clk,                                -- should be the same clk as used by SCU_Bus_Slave
    nReset                =>  nPowerup_Res,                   
    User1_Reg             =>  User1_Reg,                          -- Daten-Reg. User1
    User2_Reg             =>  open,                                -- Daten-Reg. User2    -- User2_Reg not used
    User_Reg_rd_active    =>  User_Reg_rd_active,                 -- read data available at 'Dat
    Data_to_SCUB          =>  user_reg1_data_to_SCUB,             -- connect read sources to SCU
    Dtack_to_SCUB         =>  user_reg1_Dtack                     -- connect Dtack to SCUB-Macro
  );




-- open drain buffer for one wire

owr_i(0)          <= A_OneWire_EEPROM;
owr_i(1)          <= A_OneWire;
A_OneWire_EEPROM  <= owr_pwren_o(0) when (owr_pwren_o(0) = '1' or owr_en_o(0) = '1') else 'Z';
A_OneWire         <= owr_pwren_o(1) when (owr_pwren_o(1) = '1' or owr_en_o(1) = '1') else 'Z';

lm32_ow: housekeeping
  generic map (
    Base_Addr => c_housekeeping_base)
  port map (
    clk_sys             => clk,
    n_rst               => nPowerup_Res,

    ADR_from_SCUB_LA    => ADR_from_SCUB_LA,
    Data_from_SCUB_LA   => Data_from_SCUB_LA,
    Ext_Adr_Val         => Ext_Adr_Val,
    Ext_Rd_active       => Ext_Rd_active,
    Ext_Wr_active       => Ext_Wr_active,
    user_rd_active      => wb_scu_rd_active,
    Data_to_SCUB        => wb_scu_data_to_SCUB,
    Dtack_to_SCUB       => wb_scu_dtack,

    owr_pwren_o         => owr_pwren_o,
    owr_en_o            => owr_en_o,
    owr_i               => owr_i,

    debug_serial_o      => open, --A_TB(0),
    debug_serial_i      => '0'
);
  
 

  
sio3_clk_sw: sio3_sys_clk_local_clk_switch
  port map(
    local_clk_i             => CLK_FPGA,              --125MHz XTAL
    sys_clk_i               => A_SysClock,            --12p5MHz SCU Bus
    nReset                  => nPowerup_Res,
    master_clk_o            => clk,                   --SysClk 125MHz
    pll_locked              => pll_1_locked,          --open,
    A_ME_12MHz              => A_ME_12MHz,            --12p5MHz to Pin
    sys_clk_is_bad          => sys_clk_is_bad,        
    sys_clk_is_bad_la       => sys_clk_is_bad_la,
    local_clk_is_bad        => open,                  --local_clk_is_bad,not used
    local_clk_is_running    => local_clk_is_running, 
    sys_clk_deviation       => sys_clk_deviation,     
    sys_clk_deviation_la    => sys_clk_deviation_la, 
    Adr_from_SCUB_LA        => ADR_from_SCUB_LA,      -- in, latched address from SCU_Bus
    Data_from_SCUB_LA       => Data_from_SCUB_LA,     -- in, latched data from SCU_Bus
    Ext_Adr_Val             => Ext_Adr_Val,           -- in, '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active           => Ext_Rd_active,         -- in, '1' => Rd-Cycle is active
    Ext_Wr_active           => Ext_Wr_active,         -- in, '1' => Wr-Cycle is active
    Rd_Port                 => clk_switch_rd_data,    -- output for all read sources of this macro
    Rd_Activ                => clk_switch_rd_active,  -- this macro has read data available at the Rd_Port.
    Dtack                   => clk_switch_dtack,
    signal_tap_clk_250mhz   => open                   -- signal_tap_clk_250mhz
  );
      
mil_slave_1: wb_mil_wrapper 
  generic map(
    Clk_in_Hz  =>  clk_in_hz,  -- Um  1Mb/s Manchester genau genug auszumessen (kürzester Flankenabstand 500 ns), mindestens mit 20 Mhz takten!
                               -- "Mil_Clk" im Generic "Mil_clk_in_Hz" richtig definieren, sonst ist "Baudrate" des Manchester-I/O Datenstroms falsch.
    Base_Addr  =>  c_wb_mil_wrapper_Base_Addr
  )
  port map(
    Adr_from_SCUB_LA    =>  ADR_from_SCUB_LA,   -- in,    latched address from SCU_Bus
    Data_from_SCUB_LA   =>  Data_from_SCUB_LA,  -- in,   latched data from SCU_Bus 
    Ext_Adr_Val         =>  Ext_Adr_Val,        -- in,   '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active       =>  Ext_Rd_active,      -- in,   '1' => Rd-Cycle is active
    Ext_Rd_fin          =>  Ext_Rd_fin,         -- in,    marks end of read cycle, active one for one clock period of sys_clk
    Ext_Wr_active       =>  Ext_Wr_active,      -- in,   '1' => Wr-Cycle is active
    Ext_Wr_fin          =>  Ext_Wr_fin,         -- in,    marks end of write cycle, active one for one clock period of sys_clk
    clk                 =>  clk,                -- in,    should be the same clk, used by SCU_Bus_Slave
    Data_to_SCUB        =>  Mil_Data_to_SCUB,   -- out,   connect read sources to SCUB-Macro
    Data_for_SCUB       =>  mil_rd_active,      -- out,   this macro has data for the SCU-Bus
    Dtack_to_SCUB       =>  mil_Dtack_to_SCUB,  -- out,   connect Dtack to SCUB-Macro
    nME_BZO             =>  A_ME_nBZO,          -- in
    nME_BOO             =>  A_ME_nBOO,          -- in
    Reset_Puls          =>  Powerup_Res,        -- in
    ME_SD               =>  A_ME_SD,            -- in
    ME_ESC              =>  A_ME_ESC,           -- in
    ME_CDS              =>  A_ME_CDS,           -- in     kk not used
    ME_SDO              =>  A_ME_SDO,           -- in
    ME_DSC              =>  A_ME_DSC,           -- in
    ME_VW               =>  A_ME_VW,            -- in
    ME_TD               =>  A_ME_TD,            -- in
    ME_SDI              =>  A_ME_SDI,           -- out
    ME_SS               =>  A_ME_SS,            -- out
    ME_EE               =>  A_ME_EE,            -- out
    Mil_In_Pos          =>  A_Mil1_BOI,         -- in
    Mil_In_Neg          =>  A_Mil1_BZI,         -- in
    ME_BOI              =>  A_ME_BOI,
    ME_BZI              =>  A_ME_BZI,
    nSel_Mil_Drv        =>  nSel_Mil_Drv,       -- out
    nSel_Mil_Rcv        =>  A_Mil1_nIN_Ena,     -- out
    nMil_Out_Pos        =>  A_MIL1_nBZO,        -- out
    nMil_Out_Neg        =>  A_MIL1_nBOO,        -- out
    Mil_Trm_Rdy         =>  Mil_Trm_Rdy,        -- out
    Mil_Rcv_Rdy         =>  Mil_Rcv_Rdy,        -- out
    nLED_Mil_Rcv_Error  =>  nLED_Mil_Rcv_Error,  -- out
    nLed_Mil_Trm        =>  nLed_Mil_Trm,
    error_limit_reached =>  open,                -- out
    No_VW_Cnt           =>  No_VW_Cnt,           -- out   zur Diagnose des EPLD-Manchester-Decoders: Bit[15..8] Fehlerzähler für No Valid Word des positiven Decoders "No_VW_p", Bit[7..0] Fehlerzähler für No Valid Word des negativen Decoders "No_VM_n"
    Not_Equal_Cnt       =>  Not_Equal_Cnt,       -- out   zur Diagnose des EPLD-Manchester-Decoders: Bit[15..8] Fehlerzähler für Data_not_equal, Bit[7..0] Fehlerzähler für unterschiedliche Komando-Daten-Kennung (CMD_not_equal).
    Mil_Decoder_Diag_p  =>  Mil_Decoder_Diag_p,  -- out   zur Diagnose des EPLD-Manchester-Decoders: des Positiven Signalpfades
    Mil_Decoder_Diag_n  =>  Mil_Decoder_Diag_n,  -- out   zur Diagnose des EPLD-Manchester-Decoders: des Negativen Signalpfades
    timing              => not A_Timing,
    nLed_Timing         => A_nLED_Timing,
    dly_intr_o          => dly_intr_o,
    nLed_Fifo_ne        => open,
    ev_fifo_ne_intr_o   => ev_fifo_ne_intr_o,
    Interlock_Intr_i    => not A_nOPK_INL,
    Data_Rdy_Intr_i     => not A_nOPK_DRDY,
    Data_Req_Intr_i     => not A_nOPK_DRQ,
    Interlock_Intr_o    => Interlock_Intr_o,
    Data_Rdy_Intr_o     => Data_Rdy_Intr_o,
    Data_Req_Intr_o     => Data_Req_Intr_o,
    nLed_Interl         => A_NLED(2),
    nLed_Dry            => A_nLED(0),
    nLed_Drq            => A_nLED(1),
    every_ms_intr_o     => every_ms_intr_o, 
    -- lemo I/F        
    lemo_data_o         => lemo_data_o,
    lemo_nled_o         => lemo_nled_o,
    lemo_out_en_o       => lemo_out_en_o,
    lemo_data_i         => lemo_data_i
  );
    
A_MIL1_Out_Ena  <= not nSel_Mil_Drv;
A_ME_UDI        <= '0';


-- KK  now from wb_mil_scu A_ME_BOI  <= A_MIL1_BOI;            --  A_ME_BOI  <= A_MIL1_nBOO;
-- KK  now from wb_mil_scu A_ME_BZI  <= A_MIL1_BZI;            --  A_ME_BZI  <= A_MIL1_nBZO;


testport_mux:  process  (A_SEL, test_port_in_0, Timing_Pattern_LA, Mil_Decoder_Diag_p, Mil_Decoder_Diag_n, No_VW_Cnt, Not_Equal_Cnt)
variable test_out: std_logic_vector(31 downto 0);
begin
  case (A_SEL) is
    when X"0"    => test_out := test_port_in_0;
    when X"1"    => test_out := x"CAFEBABE";
    when X"2"    => test_out := Timing_Pattern_LA;
    when X"3"    => test_out := Mil_Decoder_Diag_n & Mil_Decoder_Diag_p;
    when X"4"    => test_out := No_VW_Cnt & Not_Equal_Cnt;
    when others  => test_out := (others => '0');
  end case;
  
  A_TB <= test_out(31 downto 16);
  A_TA <= test_out(15 downto 0);
end process testport_mux;


rd_port_mux:  process  ( 
  mil_rd_active, Mil_Data_to_SCUB, 
  User_Reg_rd_active,user_reg1_data_to_SCUB,
  clk_switch_rd_active,clk_switch_rd_data,
  wb_scu_rd_active,wb_scu_data_to_SCUB )
begin
  if mil_rd_active = '1' then
    Data_to_SCUB <= Mil_Data_to_SCUB;
  elsif User_Reg_rd_active = '1' then 
    Data_to_SCUB <= user_reg1_data_to_SCUB;
  elsif clk_switch_rd_active = '1' then
    Data_to_SCUB <= clk_switch_rd_data;
  elsif wb_scu_rd_active = '1' then
    Data_to_SCUB <= wb_scu_data_to_SCUB;
  else
    Data_to_SCUB <= (others => '-');
  end if;
end process rd_port_mux;



p_ready:  process(clk, nPowerup_Res, Deb_SCUB_Reset_out)
begin
  if (nPowerup_Res = '0') or (Deb_SCUB_Reset_out = '1') then
    ready <= '0';
  elsif rising_edge(clk) then
    ready <= '1';
  end if;
end process;


test_port_in_0 <=
   X"000"             & '0'                 & Mil_Trm_Rdy      & Mil_Rcv_Rdy      & nLed_Mil_Rcv_Error &  -- bit31..16
   nPowerup_Res       & clk                 & Ena_Every_100ns  & Ena_Every_166ns  &                       -- bit15..12
   '0'                & '0'                 & pll_1_locked     &  pll_2_locked    &                       -- bit11..8
   '0'                & SysClock_pll_out    & A_RnW            & A_nDS            &                       -- bit7..4
  Timing_Pattern_RCV  & '0'                 & '0'              & SCU_Dtack                                -- bit3..0
  ;


fl:flash_loader_v01
  port map  (
    noe_in  =>  '0'
  );



--s_intr_in <= '0'& clk_switch_intr & "000000000"& dly_intr_o & Interlock_Intr_o & Data_Rdy_Intr_o & Data_Req_Intr_o;
s_intr_in <= '0'& clk_switch_intr & "0000000"& Interlock_Intr_o & Data_Rdy_Intr_o & Data_Req_Intr_o & dly_intr_o & ev_fifo_ne_intr_o & every_ms_intr_o;


SCU_Slave:scu_bus_slave
  generic map  (
    CLK_in_Hz           =>  CLK_in_Hz,
    Slave_ID            =>  SCU_SIO2_ID,
    Firmware_Version    =>  c_Firmware_Version,
    Firmware_Release    =>  c_Firmware_Release,
    CID_SYSTEM          =>  55,
    CID_GROUP           =>  26,
    Intr_Enable         =>  "0000000000000001"
  )
  port map  (
    SCUB_Addr           =>  A_A,                -- in     SCU_Bus: address bus
    nSCUB_Timing_Cyc    =>  A_nEvent_Str,       -- in     SCU_Bus signal: low active SCU_Bus runs timing cycle
    SCUB_Data           =>  A_D,                -- inout  SCU_Bus: data bus (FPGA tri state buffer)
    nSCUB_Slave_Sel     =>  A_nBoardSel,        -- in,    SCU_Bus: '0' => SCU master select slave
    nSCUB_DS            =>  A_nDS,              -- in,    SCU_Bus: '0' => SCU master activate data strobe
    SCUB_RDnWR          =>  A_RnW,              -- in,    SCU_Bus: '1' => SCU master read slave
    clk                 =>  clk,              
    nSCUB_Reset_in      =>  A_nReset,           -- in,    SCU_Bus-Signal: '0' => 'nSCUB_Reset_In' is active
    Data_to_SCUB        =>  Data_to_SCUB,       -- in,    connect read sources from external user functions
    Dtack_to_SCUB       =>  Dtack_to_SCUB,      -- in,    connect Dtack from from external user functions  
    Intr_In             =>  s_intr_in,          -- in     
    User_Ready          =>  ready,
    Data_from_SCUB_LA   =>  Data_from_SCUB_LA,  -- out    latched data from SCU_Bus for external user functions 
    ADR_from_SCUB_LA    =>  ADR_from_SCUB_LA,   -- out    latched address from SCU_Bus for external user functions
    Timing_Pattern_LA   =>  Timing_Pattern_LA,  -- out    latched timing pattern from SCU_Bus for external user functions
    Timing_Pattern_RCV  =>  Timing_Pattern_RCV, -- out    timing pattern received
    nSCUB_Dtack_Opdrn   =>  open,               -- out    for direct connect to SCU_Bus opendrain signal - '0' => slave give dtack to SCU master
    SCUB_Dtack          =>  SCU_Dtack,          -- out    for connect via ext. open collector driver - '1' => slave give dtack to SCU master
    nSCUB_SRQ_Opdrn     =>  open,               -- out    for direct connect to SCU_Bus opendrain signal - '0' => slave service request to SCU ma
    SCUB_SRQ            =>  SCUB_SRQ,           -- out    for connect via ext. open collector driver - '1' => slave service request to SCU master
    nSel_Ext_Data_Drv   =>  A_nSel_Ext_Data_Drv,-- out    '0' => select the external data driver on the SCU_Bus slave
    Ext_Data_Drv_Rd     =>  A_Ext_Data_RD,      -- out    '1' => direction of the external data driver on the SCU_Bus slave is to the SCU_Bus
    Standard_Reg_Acc    =>  open,               -- out    '1' => mark the access to register of this macro
    Ext_Adr_Val         =>  Ext_Adr_Val,        -- out    for external user functions: '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active       =>  Ext_Rd_active,      -- out    '1' => Rd-Cycle to external user register is active
    Ext_Rd_fin          =>  Ext_Rd_fin,         -- out    marks end of read cycle, active one for one clock period of clk past cycle end (no overlap)
    Ext_Rd_Fin_ovl      =>  open,               -- out    marks end of read cycle, active one for one clock period of clk during cycle end (overlap)
    Ext_Wr_active       =>  Ext_Wr_active,      -- out    '1' => Wr-Cycle to external user register is active
    Ext_Wr_fin          =>  Ext_Wr_fin,         -- out    marks end of write cycle, active one for one clock period of clk past cycle end (no overlap)
    Ext_Wr_fin_ovl      =>  open,
    Deb_SCUB_Reset_out  =>  Deb_SCUB_Reset_out, -- out    Debounced 'nSCUB_Reset_In' (active high) for reset of external macros, when 'nSCUB_Reset_In' is '0'
    nPowerup_Res        =>  nPowerup_Res,       -- out    this macro generated a power up reset
    Powerup_Done        =>  open
  );  

Dtack_to_SCUB <=  user_reg1_Dtack  or mil_Dtack_to_SCUB or clk_switch_dtack or wb_scu_dtack;

A_nDtack  <= NOT(SCU_Dtack);
A_nSRQ    <= NOT(SCUB_SRQ);


zeit1 : zeitbasis
  generic map  (
    CLK_in_Hz  =>  CLK_in_Hz,
    diag_on    =>  1
  )
  port map  (
    Res                =>  Powerup_Res,
    Clk                =>  clk,
    Ena_every_100ns    =>  Ena_Every_100ns,
    Ena_every_166ns    =>  Ena_Every_166ns,
    Ena_every_250ns    =>  open,
    Ena_every_500ns    =>  open,
    Ena_every_1us      =>  open,
    Ena_Every_20ms     =>  Ena_Every_20ms
  );

---------------------------------------------------LEDs----------------------------------------------
p_led_sel: led_n
  generic  map (
    stretch_cnt => stretch_cnt)
  port map (
    ena         => Ena_Every_20ms,
    CLK         => clk,
    Sig_In      => SCU_Dtack,
    nLED        => A_nLED(15)
  );

clk_failed_led: led_n
  generic  map (
    stretch_cnt => stretch_cnt)
  port map (
    ena         => Ena_Every_20ms,
    CLK         => clk,
    Sig_In      => sys_clk_is_bad,
    nLED        => sys_clk_is_bad_led_n
  );
      
local_clk_led: led_n
  generic map (
    stretch_cnt  => stretch_cnt)
  port map  (
    ena         => Ena_Every_20ms,
    CLK         => clk,
    Sig_In      => local_clk_is_running,
    nLED        => local_clk_runs_led_n
  );
       
clk_deviation_led: led_n
  generic map (
    stretch_cnt   => stretch_cnt)
  port map  (
    ena           => Ena_Every_20ms,
    CLK           => clk,
    Sig_In        => sys_clk_deviation,
    nLED          => sys_clk_deviation_led_n
  );
  
p_led_rcv: led_n
  generic map (
    stretch_cnt   => stretch_cnt)
  port map  (
    ena           => Ena_Every_20ms,
    CLK           => clk,
    Sig_In        => Mil_Rcv_Rdy,
    nLED          => nLED_MIL_Rcv_Rdy
  );

A_nLED(14 downto 11)  <=  not User1_Reg(14 downto 11);
A_nLED(10)            <=  local_clk_runs_led_n;
A_nLED(9)             <=  sys_clk_deviation_led_n;
A_nLED(8)             <=  sys_clk_is_bad_led_n;
A_nLED(7)             <=  not User1_Reg(7);
A_nLED(6)             <=  nLED_Mil_Rcv_Error;
A_nLED(5)             <=  nLED_MIL_Trm;
A_nLED(4)             <=  nLED_MIL_Rcv_Rdy;
A_nLED(3)             <=  not A_UMIL15V;

--nMil_Trm_Rdy    <= not Mil_Trm_Rdy;   -- only for modelsim --not used -> Quartus Warning
--Sel_Mil_Drv     <= nSel_Mil_Drv;      -- not used -> Quartus Warning       
--nMil_Rcv_Rdy    <= not Mil_Rcv_Rdy;   -- only for modelsim! --not used -> Quartus Warning

end arch_scu_sio3;
