library ieee;  
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.gencores_pkg.all;
use work.scu_bus_slave_pkg.all;
use work.scu_diob_pkg.all;
use work.pll_pkg.all;
use work.monster_pkg.all;
use work.aux_functions_pkg.all;


entity scu_diob_v2 is
generic (
    CLK_sys_in_Hz:      integer := 125000000;
    g_card_type:        string := "diob"
        );

port (
  ------------------------------ Clocks -------------------------------------------------------------------------
  CLK_20MHz_A:            in    std_logic;                          -- Clock_A
  CLK_20MHz_B:            in    std_logic;                          -- Clock_B
  CLK_20MHz_C:            in    std_logic;                          -- Clock_C
  CLK_20MHz_D:            in    std_logic;                          -- Clock_D
   
  --------- Parallel SCU-Bus-Signale ----------------------------------------------------------------------------
  A_A:                    in    std_logic_vector(15 downto 0);      -- SCU-Adressbus
  A_nADR_EN:              out   std_logic := '0';                   -- '0' => externe Adresstreiber des Slaves aktiv
  A_nADR_FROM_SCUB:       out   std_logic := '0';                   -- '0' => externe Adresstreiber-Richtung: SCU-Bus nach Slave
  A_D:                    inout std_logic_vector(15 downto 0);      -- SCU-Datenbus
  A_nDS:                  in    std_logic;                          -- Data-Strobe vom Master gertieben
  A_RnW:                  in    std_logic;                          -- Schreib/Lese-Signal vom Master getrieben, '0' => lesen
  A_nSel_Ext_Data_Drv:    out   std_logic;                          -- '0' => externe Datentreiber des Slaves aktiv
  A_Ext_Data_RD:          out   std_logic;                          -- '0' => externe Datentreiber-Richtung: SCU-Bus nach
                                                                    -- Slave (besser default 0, oder Treiber A/B tauschen)
                                                                    -- SCU-Bus nach Slave (besser default 0, oder Treiber A/B tauschen)
  A_nDtack:               out   std_logic;                          -- Data-Acknowlege null aktiv, '0' => aktiviert externen
                                                                    -- Opendrain-Treiber
  A_nSRQ:                 out   std_logic;                          -- Service-Request null aktiv, '0' => aktiviert externen
                                                                    -- Opendrain-Treiber
  A_nBoardSel:            in    std_logic;                          -- '0' => Master aktiviert diesen Slave
  A_nEvent_Str:           in    std_logic;                          -- '0' => Master sigalisiert Timing-Zyklus
  A_SysClock:             in    std_logic;                          -- Clock vom Master getrieben.
  A_Spare0:               in    std_logic;                          -- vom Master getrieben
  A_Spare1:               in    std_logic;                          -- vom Master getrieben
  A_nReset:               in    std_logic;                          -- Reset (aktiv '0'), vom Master getrieben

  A_nSEL_Ext_Signal_DRV:  out   std_logic;                          -- '0' => Treiber fr SCU-Bus-Steuer-Signale aktiv
  A_nExt_Signal_in:       out   std_logic;                          -- '0' => Treiber fr SCU-Bus-Steuer-Signale-Richtung:
                                                                    -- SCU-Bus nach Slave (besser default 0, oder Treiber A/B tauschen)

  ----------------- OneWire ----------------------------------------------------------------------------------------
  A_OneWire:              inout std_logic;                          -- Temp.-OneWire auf dem Slave
   
  ------------ Logic analyser Signals -------------------------------------------------------------------------------
  A_SEL:                  in    std_logic_vector(3 downto 0);       -- use to select sources for the logic analyser ports
  A_Tclk:                 out   std_logic;                          -- Clock  for Logikanalysator Port A
  A_TA:                   out   std_logic_vector(15 downto 0);      -- test port a

  ---------------------------------- Diagnose-LED's -----------------------------------------------------------------
  A_nLED_D2:              out   std_logic;                          -- Diagnose-LED_D2 auf dem Basis-Board
  A_nLED_D3:              out   std_logic;                          -- Diagnose-LED_D3 auf dem Basis-Board

  ------------ User I/O zur VG-Leiste -------------------------------------------------------------------------------
  A_nUser_EN:             out   std_logic;                          -- Enable User-I/O
  UIO:                    inout std_logic_vector(15 downto 0);      -- User I/O VG-Leiste
  
  ---------------- Parallel IO to user module -----------------------------------------------------------------------
  CLK_IO:                 in    std_logic;                          -- Clock vom Anwender_I/0
  PIO:                    inout std_logic_vector(142 downto 16);    -- Dig. User I/0 to Piggy
  module_id:              in    std_logic_vector(7 downto 0));      -- id code of plugged module
end scu_diob_v2;



architecture arch of scu_diob_v2 is


--  +============================================================================================================================+
--  |                                 Firmware_Version/Firmware_Release und Basis-Adressen                                       |
--  +============================================================================================================================+

  constant c_Firmware_Version:    integer := 0;   ---- Firmware_Version
--  constant c_Firmware_Release:    integer := 9;   ---- Firmware_Release
--  constant c_Firmware_Release:    integer := 10;  ---- Firmware_Release Stand 03.08.2015 ('740 nur Standard- kein DAC/FG-Mode)
--  constant c_Firmware_Release:    integer := 11;  ---- Firmware_Release Stand 04.09.2015 ('Fehler LED-Lemo-Out, IO-Modul-Backplane-Test)
--  constant c_Firmware_Release:    integer := 12;  ---- Firmware_Release Stand 10.09.2015 ('IO-Modul-Backplane-Test Fortsetzung)
--  constant c_Firmware_Release:    integer := 13;  ---- Firmware_Release Stand 09.10.2015 ('+ 16In, 16Out, CID SPSIOI1 60 geändert in 68)
--  constant c_Firmware_Release:    integer := 14;  ---- Firmware_Release Stand 12.10.2015 ( +'731 und Fehlerkorrektur)
--  constant c_Firmware_Release:    integer := 15;  ---- Firmware_Release Stand 21.10.2015 ( +'731 und Fehlerkorrektur)
--  constant c_Firmware_Release:    integer := 16;  ---- Firmware_Release Stand 19.11.2015 ( Update Reg.-Belegung '700' u. '710')
--  constant c_Firmware_Release:    integer := 17;  ---- Firmware_Release Stand 02.12.2015 ( Strobe und Trigger (ECC) auf '710' geändert)
--  constant c_Firmware_Release:    integer := 18;  ---- Firmware_Release Stand 28.01.2016 ( Error, Tag-Steuerung: überlappende Outputs im gleichen Register)
--  constant c_Firmware_Release:    integer := 19;  ---- Firmware_Release Stand 08.02.2016 ( + Tag-Steuerung: Level für jedes Bit im Outp.-Register ist einstellbar)
  constant c_Firmware_Release:    integer := 20;  ---- Firmware_release Stand 17.02.2016 ( INFO ROM in Housekeeping Modul eingefügt)

    
  constant clk_switch_status_cntrl_addr:  unsigned := x"0030";
  constant c_lm32_ow_Base_Addr:           unsigned(15 downto 0):=  x"0040";   -- housekeeping/LM32

  constant c_ADDAC_Base_addr:             integer := 16#0200#;                -- ADDAC (DAC = x"0200", ADC = x"0230")
  constant c_io_port_Base_Addr:           unsigned(15 downto 0):=  x"0220";   -- 4x8 Bit (ADDAC FG900.161)
  constant c_fg_1_Base_Addr:              unsigned(15 downto 0):=  x"0300";   -- FG1
  constant c_tmr_Base_Addr:               unsigned(15 downto 0):=  x"0330";   -- Timer
  constant c_fg_2_Base_Addr:              unsigned(15 downto 0):=  x"0340";   -- FG2

  constant c_Conf_Sts1_Base_Addr:         integer := 16#0500#;                -- Status-Config-Register
  constant c_AW_Port1_Base_Addr:          integer := 16#0510#;                -- Anwender I/O-Register
  constant c_INL_xor1_Base_Addr:          integer := 16#0530#;                -- Interlock-Pegel-Register
  constant c_INL_msk1_Base_Addr:          integer := 16#0540#;                -- Interlock-Masken-Register
  constant c_Tag_Ctrl1_Base_Addr:         integer := 16#0580#;                -- Tag-Steuerung
  constant c_IOBP_Masken_Base_Addr:       integer := 16#0600#;                -- IO-Backplane Masken-Register
   
  
  
--  +============================================================================================================================+
--  |                                                 constant                                                                   |
--  +============================================================================================================================+


  constant c_cid_system:     integer range 0 to 16#FFFF#:= 55;     -- extension card: cid_system, CSCOHW=55


  type ID_CID is record
    ID   : std_logic_vector(7 downto 0);
    CID  : integer range 0 to 16#FFFF#;
  end record;
--                                        +--------------- Piggy-ID(Codierung)
--                                        |     +--------- CID(extension card: cid_system)
--                                        |     |     
  constant c_AW_P37IO:      ID_CID:= (x"01", 27);   ---- Piggy-ID(Codierung), B"0000_0001", FG900_700
  constant c_AW_P25IO:      ID_CID:= (x"02", 28);   ---- Piggy-ID(Codierung), B"0000_0010", FG900_710
  constant c_AW_OCin:       ID_CID:= (x"03", 29);   ---- Piggy-ID(Codierung), B"0000_0011", FG900_720
  constant c_AW_OCIO1:      ID_CID:= (x"04", 30);   ---- Piggy-ID(Codierung), B"0000_0100", FG900_730
  constant c_AW_UIO:        ID_CID:= (x"05", 31);   ---- Piggy-ID(Codierung), B"0000_0101", FG900_740
  constant c_AW_DA:         ID_CID:= (x"06", 32);   ---- Piggy-ID(Codierung), B"0000_0110", FG900_750
  constant c_AW_Frei:       ID_CID:= (x"07", 00);   ---- Piggy-ID(Codierung), B"0000_0111", FG900_760
  constant c_AW_SPSIO1:     ID_CID:= (x"08", 33);   ---- Piggy-ID(Codierung), B"0000_1000", FG900_770 -- Ausgänge schalten nach 24V
  constant c_AW_HFIO:       ID_CID:= (x"09", 34);   ---- Piggy-ID(Codierung), B"0000_1001", FG900_780
  constant c_AW_SPSIOI1:    ID_CID:= (x"0A", 68);   ---- Piggy-ID(Codierung), B"0000_1010", FG901_770 -- Ausgänge schalten nach GND
  constant c_AW_INLB12S:    ID_CID:= (x"0B", 67);   ---- Piggy-ID(Codierung), B"0000_1011", FG902_050 -- IO-Modul-Backplane mit 12 Steckplätzen
  constant c_AW_16Out2:     ID_CID:= (x"0C", 70);   ---- Piggy-ID(Codierung), B"0000_1100", FG901_010 -- Output 16 Bit
  constant c_AW_16In2:      ID_CID:= (x"0D", 71);   ---- Piggy-ID(Codierung), B"0000_1101", FG901_020 -- Input 16 Bit
  constant c_AW_OCIO2:      ID_CID:= (x"0E", 61);   ---- Piggy-ID(Codierung), B"0000_1110", FG900_731
--  constant c_AW:            ID_CID:= (x"0F", 00);   ---- Piggy-ID(Codierung), B"0000_1111", 

  
  constant  stretch_cnt:    integer := 5;                               -- für LED's
  
      
  constant  Clk_in_ns:      integer  :=  1000000000 /  clk_sys_in_Hz;          -- (=8ns,    bei 125MHz)
  constant  CLK_sys_in_ps:  integer  := (1000000000 / (CLK_sys_in_Hz / 1000));  -- muss eigentlich clk-halbe sein
      
  constant  C_Strobe_1us:   integer := 1000 / Clk_in_ns;                       -- Anzahl der Clocks für 1us
  constant  C_Strobe_2us:   integer := 2000 / Clk_in_ns;                       -- Anzahl der Clocks für 2us
  constant  C_Strobe_3us:   integer := 003000 * 1000 / CLK_sys_in_ps;          -- Anzahl der Clock's für die Debounce-Zeit von   3uS 
  constant  C_Strobe_7us:   integer := 007000 * 1000 / CLK_sys_in_ps;          -- Anzahl der Clock's für die Debounce-Zeit von   7uS 
--  constant  C_Strobe_3us:   integer := 000300 * 1000 / CLK_sys_in_ps;          -- Anzahl der Clock's für die Debounce-Zeit von   300nS (Test)
--  constant  C_Strobe_7us:   integer := 000700 * 1000 / CLK_sys_in_ps;          -- Anzahl der Clock's für die Debounce-Zeit von   700nS (Test) 
  
  
  type      t_integer_Array  is array (0 to 7) of integer range 0 to 16383;
   
  --------------- Array für die Anzahl der Clock's für die B1dddebounce-Zeiten von 1,2,4,8,16,32,64,128 us ---------------


  constant  Wert_2_Hoch_n:   t_integer_Array := (001000 * 1000 / CLK_sys_in_ps,   -- Anzahl der Clock's für die Debounce-Zeit von   1uS 
                                                 002000 * 1000 / CLK_sys_in_ps,   -- Anzahl der Clock's für die Debounce-Zeit von   2uS 
                                                 004000 * 1000 / CLK_sys_in_ps,   -- Anzahl der Clock's für die Debounce-Zeit von   4uS 
                                                 008000 * 1000 / CLK_sys_in_ps,   -- Anzahl der Clock's für die Debounce-Zeit von   8uS 
                                                 016000 * 1000 / CLK_sys_in_ps,   -- Anzahl der Clock's für die Debounce-Zeit von  16uS 
                                                 032000 * 1000 / CLK_sys_in_ps,   -- Anzahl der Clock's für die Debounce-Zeit von  32uS 
                                                 064000 * 1000 / CLK_sys_in_ps,   -- Anzahl der Clock's für die Debounce-Zeit von  64uS 
                                                 128000 * 1000 / CLK_sys_in_ps);  -- Anzahl der Clock's für die Debounce-Zeit von 128uS 
                                                 
                                                 
  constant  c_cnt_Syn_min:  integer := 3;   -- Anzahl der Clock's, für die Synchonnisation der Eingänge bei abgeschalter Entprellung 
  constant  C_Strobe_100ns:  integer range 0 to 16383:= (000100 * 1000 / CLK_sys_in_ps);   -- Anzahl der Clock's für den Strobe 100ns 
  type      t_integer_Strobe_Array     is array (0 to 7) of integer range 0 to 65535;
  constant Wert_Strobe_2_Hoch_n : t_integer_Strobe_Array := (00001, 00002, 00004, 00008, 00016, 00032, 00064, 00128);
            
--  +============================================================================================================================+
--  |                                                    Component                                                               |
--  +============================================================================================================================+


  component pu_reset
    generic (
      PU_Reset_in_clks : integer);
    port (
      Clk:      in    std_logic;
      PU_Res:   out   std_logic);
  end component;

--  +============================================================================================================================+
--  |                                                         signal                                                             |
--  +============================================================================================================================+

  signal clk_sys, clk_cal, locked : std_logic;

  signal SCUB_SRQ:                  std_logic;
  signal SCUB_Dtack:                std_logic;
  signal convst:                    std_logic;
  signal rst:                       std_logic;
  
  signal Dtack_to_SCUB:             std_logic;
  
  signal ADR_from_SCUB_LA:          std_logic_vector(15 downto 0);
  signal Data_from_SCUB_LA:         std_logic_vector(15 downto 0);
  signal Ext_Adr_Val:               std_logic;
  signal Ext_Rd_active:             std_logic;
  signal Ext_Wr_active:             std_logic;
  signal Ext_Wr_fin_ovl:            std_logic;
  signal Ext_RD_fin_ovl:            std_logic;
  signal SCU_Ext_Wr_fin:            std_logic;
  signal nPowerup_Res:              std_logic;
  signal Timing_Pattern_LA:         std_logic_vector(31 downto 0);--  latched timing pattern from SCU_Bus for external user functions
  signal Timing_Pattern_RCV:        std_logic;----------------------  timing pattern received
  
  signal extension_cid_system:      integer range 0 to 16#FFFF#;  -- in,  extension card: cid_system
  signal extension_cid_group:       integer range 0 to 16#FFFF#;  --in, extension card: cid_group
  
  signal led_ena_cnt:               std_logic;

  signal Data_to_SCUB:              std_logic_vector(15 downto 0);
  
  signal reset_clks :               std_logic_vector(0 downto 0);
  signal reset_rstn :               std_logic_vector(0 downto 0);
  signal clk_sys_rstn :             std_logic;
  
  signal owr_pwren_o:               std_logic_vector(1 downto 0);
  signal owr_en_o:                  std_logic_vector(1 downto 0);
  signal owr_i:                     std_logic_vector(1 downto 0);
  
  signal wb_scu_rd_active:          std_logic;
  signal wb_scu_dtack:              std_logic;
  signal wb_scu_data_to_SCUB:       std_logic_vector(15 downto 0);
   

  signal Powerup_Res:               std_logic;  -- only for modelsim!
  signal Powerup_Done:              std_logic;  -- this memory is set to one if an Powerup is done. Only the SCUB-Master can clear this bit.
  signal WRnRD:                     std_logic;  -- only for modelsim!

  signal Deb_SCUB_Reset_out:        std_logic;
  signal Standard_Reg_Acc:          std_logic;
  signal Ext_Rd_fin:                std_logic;

  signal test_out:                  std_logic_vector(15 downto 0);
 
  signal Ena_Every_100ns:           std_logic;
  signal Ena_Every_166ns:           std_logic;
  signal Ena_Every_250ns:           std_logic;
  signal Ena_Every_500ns:           std_logic;
  signal Ena_Every_20ms:            std_logic;
  signal Ena_Every_1us:             std_logic;
  signal Ena_Every_250ms:           std_logic;
  signal Ena_Every_500ms:           std_logic;
 
  signal F_12p5_MHz:                std_logic;
   
  signal test_port_in_0:            std_logic_vector(15 downto 0);
  signal test_clocks:               std_logic_vector(15 downto 0);
  
  signal s_nLED_Sel:                std_logic;   -- LED = Sel
  signal s_nLED_Dtack:              std_logic;   -- LED = Dtack
  signal s_nLED_inR:                std_logic;   -- LED = interrupt
   
  signal s_nLED:                    std_logic_vector(7 downto 0); -- LED's
  signal s_nLED_Out:                std_logic_vector(7 downto 0); -- LED's
  signal AW_ID:                     std_logic_vector(7 downto 0); -- Anwender_ID
   

  signal FG_1_dreq:                 std_logic;
  signal FG_2_dreq:                 std_logic;
  signal tmr_irq:                   std_logic;

  signal clk_blink:                 std_logic;
  
  signal sys_clk_is_bad:            std_logic;
  signal sys_clk_is_bad_led_n:      std_logic;
  signal sys_clk_is_bad_la:         std_logic;
  signal local_clk_is_bad:          std_logic;
  signal local_clk_is_running:      std_logic;
  signal local_clk_runs_led_n:      std_logic;
  signal sys_clk_failed:            std_logic;
  signal sys_clk_deviation:         std_logic;
  signal sys_clk_deviation_la:      std_logic;
  signal sys_clk_deviation_led_n:   std_logic;
  signal clk_switch_rd_data:        std_logic_vector(15 downto 0);
  signal clk_switch_rd_active:      std_logic;
  signal clk_switch_dtack:          std_logic;
  signal pll_locked:                std_logic;
  signal clk_switch_intr:           std_logic;
  
  signal signal_tap_clk_250mhz:     std_logic;
  signal clk_update:                std_logic;
  signal clk_flash:                 std_logic;
  
  
  signal rstn_sys:                  std_logic;
  signal rstn_update:               std_logic;
  signal rstn_flash:                std_logic;
  signal rstn_stc:                  std_logic;

  signal uart_txd_out:              std_logic; 
  
  constant c_is_arria5:             boolean := false;
  signal scu_master_o:              t_scu_local_master_o;
  signal scu_master_i:              t_scu_local_master_i;
  
  constant module_count:            natural := 2;
  type bit_vector_array is array  (0 to module_count-1) of std_logic_vector(PIO'length-1 downto 0);
  signal bit_vector_in:             bit_vector_array;
  signal bit_vector_out:            bit_vector_array;
  signal bit_vector_en:             bit_vector_array;
  signal pio_bit_vector_in:         std_logic_vector(PIO'length-1 downto 0);
  signal pio_bit_vector_out:        std_logic_vector(PIO'length-1 downto 0);
  signal pio_bit_vector_en:         std_logic_vector(PIO'length-1 downto 0);

  function or_reduce(a : bit_vector_array; vector_size : integer) return std_logic_vector is
    variable ret : std_logic_vector(vector_size-1 downto 0);
  begin
    for i in a'range loop
        ret := ret or a(i);
    end loop;

    return ret;
  end function or_reduce;

  
--  ###############################################################################################################################
--  ###############################################################################################################################
--  #####                                                                                                                     #####
--  #####                                                 BEGIN                                                               #####
--  #####                                                                                                                     #####
--  ###############################################################################################################################
--  ###############################################################################################################################


  begin


  A_nADR_EN             <= '0';
  A_nADR_FROM_SCUB      <= '0';
  A_nExt_Signal_in      <= '0';
  A_nSEL_Ext_Signal_DRV <= '0';
  A_nUser_EN            <= '0';
  

  pio_mux: diob_pio_tristate
  generic map ( pio_size => PIO'length)
  port map (
    pio             => PIO,
    bit_vector_in   => pio_bit_vector_in,
    bit_vector_out  => pio_bit_vector_out,
    bit_vector_en   => pio_bit_vector_en,
    module_io_i     => open);
  
    pio_bit_vector_in   <= or_reduce(bit_vector_in, PIO'length);
    pio_bit_vector_out  <= or_reduce(bit_vector_out, PIO'length);
    pio_bit_vector_en   <= or_reduce(bit_vector_en, PIO'length);


  module1: entity work.diob_module(diob_module1_arch)
    generic map (
      size      => PIO'length,
      module_id => c_AW_P37IO.ID
    )
    port map (
          scu_slave_i => scu_master_o,
          scu_slave_o => scu_master_i,
          tag_i       => Timing_Pattern_LA,
          tag_valid   => Timing_Pattern_RCV,
          en_i        => '0',
          vect_i      => bit_vector_in(0),
          vect_o      => bit_vector_out(0),
          vect_en     => bit_vector_en(0));
  
  module2: entity work.diob_module(diob_module2_arch)
    generic map (
      size      => PIO'length,
      module_id => c_AW_P25IO.ID
    )
    port map (
          scu_slave_i => scu_master_o,
          scu_slave_o => scu_master_i,
          tag_i       => Timing_Pattern_LA,
          tag_valid   => Timing_Pattern_RCV,
          en_i        => '1',
          vect_i      => bit_vector_in(1),
          vect_o      => bit_vector_out(1),
          vect_en     => bit_vector_en(1));


  


  
  Powerup_Res <= not nPowerup_Res;  -- only for modelsim!
  WRnRD       <= not A_RnW;         -- only for modelsim!

  diob_clk_switch: slave_clk_switch
    generic map (
      Base_Addr => clk_switch_status_cntrl_addr,
      card_type => g_card_type
    )
    port map(
      local_clk_i             => CLK_20MHz_D,
      sys_clk_i               => A_SysClock,
      nReset                  => nPowerup_Res,
      master_clk_o            => clk_sys,               -- core clocking
      pll_locked              => pll_locked,
      sys_clk_is_bad          => sys_clk_is_bad,
      sys_clk_is_bad_la       => sys_clk_is_bad_la,
      local_clk_is_bad        => local_clk_is_bad,
      local_clk_is_running    => local_clk_is_running,
      sys_clk_deviation       => sys_clk_deviation,
      sys_clk_deviation_la    => sys_clk_deviation_la,
      Adr_from_SCUB_LA        => ADR_from_SCUB_LA,      -- in, latched address from SCU_Bus
      Data_from_SCUB_LA       => Data_from_SCUB_LA,     -- in, latched data from SCU_Bus
      Ext_Adr_Val             => Ext_Adr_Val,           -- in, '1' => "ADR_from_SCUB_LA" is valid
      Ext_Rd_active           => Ext_Rd_active,         -- in, '1' => Rd-Cycle is active
      Ext_Wr_active           => Ext_Wr_active,         -- in, '1' => Wr-Cycle is active
      Rd_Port                 => clk_switch_rd_data,    -- output for all read sources of this macro
      Rd_Activ                => clk_switch_rd_active,  -- this acro has read data available at the Rd_Port.
      Dtack                   => clk_switch_dtack,
      signal_tap_clk_250mhz   => signal_tap_clk_250mhz,
      clk_update              => clk_update,
      clk_flash               => clk_flash,
      clk_encdec              => open
      );
   
   reset : altera_reset
    generic map(
      g_plls   => 1,
      g_clocks => 4,
      g_areset => f_pick(c_is_arria5, 100, 1)*1024,
      g_stable => f_pick(c_is_arria5, 100, 1)*1024)
    port map(
      clk_free_i    => clk_sys,
      rstn_i        => A_nReset,
      pll_lock_i(0) => pll_locked,
      pll_arst_o    => open,
      clocks_i(0)   => clk_sys,
      clocks_i(1)   => signal_tap_clk_250mhz,
      clocks_i(2)   => clk_update,
      clocks_i(3)   => clk_flash,
      rstn_o(0)     => rstn_sys,
      rstn_o(1)     => rstn_stc,
      rstn_o(2)     => rstn_update,
      rstn_o(3)     => rstn_flash);

  clk_switch_intr <= local_clk_is_running or sys_clk_deviation_la;

  SCU_Slave: SCU_Bus_Slave
  generic map (
    CLK_in_Hz               => clk_sys_in_Hz,
    Firmware_Release        => c_Firmware_Release,  -------------------- important: => Firmware_Release
    Firmware_Version        => c_Firmware_Version,  -------------------- important: => Firmware_Version
    CID_System              => 55, ------------------------------------- important: => CSCOHW
    intr_Enable             => b"0000_0000_0000_0001")
  port map (
    SCUB_Addr               => A_A,                                   -- in, SCU_Bus: address bus
    nSCUB_Timing_Cyc        => A_nEvent_Str,                          -- in, SCU_Bus signal: low active SCU_Bus runs timing cycle
    SCUB_Data               => A_D,                                   -- inout, SCU_Bus: data bus (FPGA tri state buffer)
    nSCUB_Slave_Sel         => A_nBoardSel,                           -- in, SCU_Bus: '0' => SCU master select slave
    nSCUB_DS                => A_nDS,                                 -- in, SCU_Bus: '0' => SCU master activate data strobe
    SCUB_RDnWR              => A_RnW,                                 -- in, SCU_Bus: '1' => SCU master read slave
    clk                     => clk_sys,
    nSCUB_Reset_in          => A_nReset,                              -- in, SCU_Bus-Signal: '0' => 'nSCUB_Reset_in' is active
    Data_to_SCUB            => Data_to_SCUB,                          -- in, connect read sources from external user functions
    Dtack_to_SCUB           => Dtack_to_SCUB,                         -- in, connect Dtack from from external user functions
    intr_in                 => FG_1_dreq & FG_2_dreq & tmr_irq & '0'  -- bit 15..12
                              & x"0"                                  -- bit 11..8
                              & x"0"                                  -- bit 7..4
                              & '0' & '0' & clk_switch_intr,          -- bit 3..1
    User_Ready              => '1',
    CID_GROUP               => 26,                                    -- important: => "FG900500_SCU_Diob1"
    extension_cid_system    => extension_cid_system,                  -- in, extension card: cid_system
    extension_cid_group     => extension_cid_group,                   -- in, extension card: cid_group
    Data_from_SCUB_LA       => Data_from_SCUB_LA,                     -- out, latched data from SCU_Bus for external user functions
    ADR_from_SCUB_LA        => ADR_from_SCUB_LA,                      -- out, latched address from SCU_Bus for external user functions
    Timing_Pattern_LA       => Timing_Pattern_LA,                     -- out, latched timing pattern from SCU_Bus for external user functions
    Timing_Pattern_RCV      => Timing_Pattern_RCV,                    -- out, timing pattern received
    nSCUB_Dtack_Opdrn       => open,                                  -- out, for direct connect to SCU_Bus opendrain signal
                                                                      -- '0' => slave give dtack to SCU master
    SCUB_Dtack              => SCUB_Dtack,                            -- out, for connect via ext. open collector driver
                                                                      -- '1' => slave give dtack to SCU master
    nSCUB_SRQ_Opdrn         => open,                                  -- out, for direct connect to SCU_Bus opendrain signal
                                                                      -- '0' => slave service request to SCU ma
    SCUB_SRQ                => SCUB_SRQ,                              -- out, for connect via ext. open collector driver
                                                                      -- '1' => slave service request to SCU master
    nSel_Ext_Data_Drv       => A_nSel_Ext_Data_Drv,                   -- out, '0' => select the external data driver on the SCU_Bus slave
    Ext_Data_Drv_Rd         => A_Ext_Data_RD,                         -- out, '1' => direction of the external data driver on the
                                                                      -- SCU_Bus slave is to the SCU_Bus
    Standard_Reg_Acc        => Standard_Reg_Acc,                      -- out, '1' => mark the access to register of this macro
    Ext_Adr_Val             => Ext_Adr_Val,                           -- out, for external user functions: '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active           => Ext_Rd_active,                         -- out, '1' => Rd-Cycle to external user register is active
    Ext_Rd_fin              => Ext_Rd_fin,                            -- out, marks end of read cycle, active one for one clock period
                                                                      -- of clk past cycle end (no overlap)
    Ext_Rd_Fin_ovl          => Ext_Rd_Fin_ovl,                        -- out, marks end of read cycle, active one for one clock period
                                                                      -- of clk during cycle end (overlap)
    Ext_Wr_active           => Ext_Wr_active,                         -- out, '1' => Wr-Cycle to external user register is active
    Ext_Wr_fin              => SCU_Ext_Wr_fin,                        -- out, marks end of write cycle, active high for one clock period
                                                                      -- of clk past cycle end (no overlap)
    Ext_Wr_fin_ovl          => Ext_Wr_fin_ovl,                        -- out, marks end of write cycle, active high for one clock period
                                                                      -- of clk before write cycle finished (with overlap)
    Deb_SCUB_Reset_out      => Deb_SCUB_Reset_out,                    -- out, the debounced 'nSCUB_Reset_in'-signal, is active high,
                                                                      -- can be used to reset
                                                                      -- external macros, when 'nSCUB_Reset_in' is '0'
    nPowerup_Res            => nPowerup_Res,                          -- out, this macro generates a power up reset
    Powerup_Done            => Powerup_Done);                         -- out, this signal is set after powerup. Only the SCUB-Master can clear this bit.

  lm32_ow: housekeeping
    generic map (
      Base_addr => c_lm32_ow_Base_Addr)
    port map (
      clk_sys           => clk_sys,
      clk_update        => clk_update,
      clk_flash         => clk_flash,
      rstn_sys          => rstn_sys,
      rstn_update       => rstn_update,
      rstn_flash        => rstn_flash,
      ADR_from_SCUB_LA  => ADR_from_SCUB_LA,
      Data_from_SCUB_LA => Data_from_SCUB_LA,
      Ext_Adr_Val       => Ext_Adr_Val,
      Ext_Rd_active     => Ext_Rd_active,
      Ext_Wr_active     => Ext_Wr_active,
      user_rd_active    => wb_scu_rd_active,
      Data_to_SCUB      => wb_scu_data_to_SCUB,
      Dtack_to_SCUB     => wb_scu_dtack,

      owr_pwren_o       => owr_pwren_o,
      owr_en_o          => owr_en_o,
      owr_i             => owr_i,

      debug_serial_o    => uart_txd_out,
      debug_serial_i    => '0');
    
end architecture arch;
