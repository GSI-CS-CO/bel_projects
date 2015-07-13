LIBRARY ieee;
use ieee.std_logic_1164.all;
use IEEE.numeric_std.all;
use work.aux_functions_pkg.all;
use work.dac714_pkg.all;

----------------------------------------------------------------------------------------------------------------------
--  Vers: 1 Revi: 0: erstellt am 25.04.2013, Autor: W.Panschow                                                      --
----------------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------
--  Vers: 2 Revi: 0: erstellt am 31.07.2013, Autor: W.Panschow                                                      --
--                                                                                                                  --
--    Der Source-File und der Entity-Name wurde von DAC_SPI auf dac714 umbenannt                                    --
--                                                                                                                  --
--    Wichtig in Vers. 2 wird vorausgesetzt, dass sowohl der Funktionsgenerator, als auch das SCU-Bus-Slave Makro   --
--    mit den gleichen Takt versorgt wird, wie das dac714 Makro!                                                    --
--                                                                                                                  --
--    Funktionsbeschreibung                                                                                         --
--                                                                                                                  --
--    Register-Layout                                                                                               --
--                                                                                                                  --
--      Base_addr   : Kontrollregister                                                                              --
--                    Lesen Bit:  15..5 | immer null                                                                --
--                                ------+-----------------------------------------------------------------------    --
--                                  4   | FG_mode;  1 = Funktiongenerator-Mode, DAC-Werte kommen von FG_Data und    --
--                                      |               werden mit FG_Strobe uebernommen. Kein externer Trigger!    --
--                                      |           0 = Software-Mode, DAC-Werte, kommen vom SCU-Bus-Slave.         --
--                                      |               Externe Triggerung mit pos. oder neg. Flanke, kann einge-   --
--                                      |               schaltet werden.                                            --
--                                ------+-----------------------------------------------------------------------    --
--                                  3   | dac_neg_edge_conv;  1 = neg. Flanke ist Trigger, wenn ext. Trig. selekt.  --
--                                      |                     0 = pos. Flanke ist Trigger, wenn ext. Trig. selekt.  --
--                                ------+-----------------------------------------------------------------------    --
--                                  2   | dac_conv_extern;    1 = externer Trigger ist selektiert                   --
--                                      |                     0 = direkt nach der seriellen Uebertragung, wird der  --
--                                      |                         DAC-Wert eingestellt.                             --
--                                ------+-----------------------------------------------------------------------    --
--                                  1   | CLR_DAC_active;   1 = der Reset des DACs ist noch nicht beendet (200ns).  --
--                                ------+-----------------------------------------------------------------------    --
--                                  0   | SPI_TRM;          1 = DAC-Wert wird seriell uebertragen.                  --
--                                ------+-----------------------------------------------------------------------    --
--                                                                                                                  --
--                Schreiben Bit:  15..5 | kein Einfluss                                                             --
--                                ------+-----------------------------------------------------------------------    --
--                                  4   | FG_mode;  1 = Funktiongenerator-Mode                                      --
--                                      |           0 = Software-Mode                                               --
--                                      |           Ein Wechsel dieses Bits hat immer einen Reset des gesamten      --
--                                      |           dac714-Makros zur Folge. D.h. beim Umschalten von FG-Mode auf   --
--                                      |           SW-Mode kann nicht der exterene Trigger und die entsprechende   --
--                                      |           Trigger-Flanke vorgegen werden, weil waehrend des Resets        --
--                                      |           diese Bits auf null gesetzt werden. Nachdem der Reset beendet   --
--                                      |           ist (CLR_DAC_active = 0), koennen die Bits entsprechend gesetzt --
--                                      |           werden.                                                         --
--                                ------+-----------------------------------------------------------------------    --
--                                  3   | dac_neg_edge_conv;  1 = neg. Flanke ist Trigger, wenn ext. Trig. selekt.  --
--                                      |                         Laesst sich nur im SW-Mode setzen.                --
--                                      |                     0 = pos. Flanke ist Trigger, wenn ext. Trig. selekt.  --
--                                ------+-----------------------------------------------------------------------    --
--                                  2   | dac_conv_extern;    1 = externer Trigger wird selektiert                  --
--                                      |                         Laesst sich nur im SW-Mode setzen.                --
--                                      |                     0 = direkt nach der seriellen Uebertragung, wird      --
--                                      |                         der DAC-Wert eingestellt.                         --
--                                ------+-----------------------------------------------------------------------    --
--                                  1   | CLR_DAC;    1 = ein Reset des DACs wird ausgefuehrt (200ns)               --
--                                ------+-----------------------------------------------------------------------    --
--                                  0   | SPI_TRM;  nicht setzbar, an anderer Stelle, abhaengig von der Betriebsart --
--                                      |           gesetzt (FG-Mode -> FG-Strobe, SW-Mode -> wr DAC-Wert), und     --
--                                      |           nach der seriellen Uebertragung zurueckgesetzt.                 --
--                                ------+-----------------------------------------------------------------------    --
--                                                                                                                  --
--      Base_addr +1: DAC-Wert 16 Bit, wird nur im Software-Mode genutzt, nur schreiben erlaubt.                    --
--      Base_addr +2: shift_err_cnt                                                                                 --
--                    Der Fehler-Zaehler wird inkrementiert, wenn der externe Trigger waerend der Uebertragung      --
--                    eines neuen DAC-Werts auftritt. Der Wert wird dann "verspaetet" uebernommen.                  --
--                    Ab 255 Fehlern wird der Fehler-Zaehler nicht mehr inkrementiert.                              --
--                    - Lesen, liefert den Stand des Zaehlers.                                                      --
--                    - Schreiben, loescht den Zaehler.                                                             --
--      Base_addr +3: old_data_err_cnt                                                                              --
--                    Der Fehler-Zaehler wird inkrementiert, wenn der externe Trigger kommt, aber kein neuer        --
--                    DAC-Wert geschrieben wurde.                                                                   --
--                    Ab 255 Fehlern wird der Fehler-Zaehler nicht mehr inkrementiert.                              --
--                    - Lesen, liefert den Stand des Zaehlers.                                                      --
--                    - Schreiben, loescht den Zaehler.                                                             --
--      Base_addr +4: trm_during_trm_active_err_cnt                                                                 --
--                    Der Fehler-Zaehler wird inkrementiert, wenn waehrend der seriellen Uebertragung eines DAC-    --
--                    Werts, schon ein weiterer Wert uebertragen werden soll.                                       --
--                    Ab 255 Fehlern wird der Fehler-Zaehler nicht mehr inkrementiert.                              --
--                    - Lesen, liefert den Stand des Zaehlers.                                                      --
--                    - Schreiben, loescht den Zaehler.                                                             --
--                                                                                                                  --
--  Aenderung 1)                                                                                                    --
--    Die Datenversorgung des DACs ist jetzt zwischen zwei verschiedenen Quellen umschaltbar:                       --
--      a)  Software getrieben. Diese Betriebsart war schon unter Vers. 1 realisiert, ist aber in seiner Fehler-    --
--          Diagnostik ueberarbeitet worden.                                                                        --
--      b)  Funktionsgenerator getrieben. Fuer diese neue Betriebsart wurde ein getrenntes Eingangsport definiert.  --
--          'FG_Data' muss mit den 16 hoechstwertigen Bits des Funktionsgenerators verbunden werden und ueber den   --
--          'FG_Strobe' muss der FG signalisieren, dass der DAC ein neues Datum uebernehmen soll.                   --
--                                                                                                                  --
--  Aenderung 2)                                                                                                    --
--    Das Control-Status-Register wurde uebearbeitet. Wenn der Funktionsgenerator-Mode selektiert ist, kann der     --
--    externe DAC-Trigger nicht eingeschaltet werden.                                                               --
--                                                                                                                  --
--  Aenderung 3)                                                                                                    --
--    Beim Wechsel der Betriebsart Software getrieben nach Funktionsgenerator getrieben und umgekehrt, wird ein     --
--    genereller Reset durchgefuehrt. Beim Umschalten vom FG-Mode zum SW-Mode können die Bits 'dac_conv_extern' und --
--    'dac_neg_edge_conv' nicht gleichzeitig gesetzt werden, da der Reset im asynchonen Pfad des Kontrollregisters  --
--    diese Bits auf null setzt. Wenn die Bits nach dem Umschalten gestzt werden sollen, muss das Kontrollregister  --
--    noch einmal geschrieben werden.                                                                               --
--                                                                                                                  --
--  Aenderung 4)                                                                                                    --
--    Fehlerstatistik-Zaehler eingebaut.                                                                            --
--      a)  Trig_DAC_during_shift_err_cnt: Der Fehler-Zaehler zaehlt externere Uebernahmesignale, die waehrend der  --
--          seriellen Uebertragung eines neuen DAC-Datum aufgetreten sind. Das Uebernahme-Signal wird gespeichert   --
--          und direkt nach der Beendigung der seriellen Uebtragung wird das Datum am DAC eingestellt.              --
--      b)  Trig_DAC_with_old_data_err_cnt: Der Fehler-Zaehler zaehlt externe Uebernahmesignale, die ein bereits    --
--          uebernommenes DAC-Datum retriggern, d.h. es wurde kein neues DAC-Datum geschrieben als der externe      --
--          DAC-Trigger dedektiert wurde.                                                                           --
--      c)  New_trm_during_trm_active_err_cnt: Dieser Fehler-Zaehler dedektiert Fehler, wenn waehrend eines         --
--          seriellen Transfers eines DAC-Wertes schon ein weiterer Wert uebertragen werden soll. Dies kann auch    --
--          ohne externe Triggerung der Fall sein, wenn                                                             --
--            i)  im Software-Mode das naechste DAC-Wort geschrieben wird, aber die vorherige Uebetragung noch      --
--                nicht abgeschlossen ist.                                                                          --
--            ii) im FG-Mode der naechste FG-Strobe akive wird, aber die vorherige Uebetragung noch nicht           --
--                abgeschlossen ist.                                                                                --
--          Fuer die Uebertragung eines DAC-Werts mit automatischem Ubernahme-Strobe baucht es 19 SPI-Takt-Zyklen.  --
--          Bei 10 MHz SPI-Takt darf ein neuer DAC-Wert erst nach 2 us geschrieben werden. Dies gilt sowohl fuer    --
--          den FG-Mode als fuer den SW-Mode.                                                                       --
--                                                                                                                  --
----------------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------
--  Vers: 2 Revi: 1: erstellt am 01.10.2013, Autor: W.Panschow                                                      --
--                                                                                                                  --
--  Aenderung 1)                                                                                                    --
--    Der Generic "Default_is_FG_mode" ist entfernt worden. Nach einem Reset oder Powerup ist immer der Software    --
--    gesteuerte Mode selektiert. Soll der Funktionsgenerator die Daten liefern, ist auf FG-Mode umzuschalten.      --
--    Der Funktionsgenerator muss sowieso entsprechend mit Daten versorgt werden, dazu gehoert eben auch die        --
--    Umschaltung in den FG-Mode.                                                                                   --
--                                                                                                                  --
--  Aenderung 2)                                                                                                    --
--    Bisher wurde bei einer Umschaltung der Betriebsart zwischen Software getrieben oder Funktionsgenerator        --
--    getrieben ein Automatisch generierter Reser erzeugt. Dieser Automatismus ist entfernt worden.                 --
--                                                                                                                  --
----------------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------
--  Vers: 2 Revi: 2: erstellt am 04.10.2013, Autor: W.Panschow                                                      --
--                                                                                                                  --
--  Aenderung 1)                                                                                                    --
--    Signal "New_trm_during_trm_active" ist jetzt nur eine Clockperiode aktiv, deshalb inkrementiert der Fehler-   --
--    Zaehler "New_trm_during_trm_active_err_cnt" jetzt richtig.                                                    --
--                                                                                                                  --
--  Aenderung 2)                                                                                                    --
--    Im Kontrollregister ist ein weiteres Bit hinzugekommen.                                                       --
--    Bit(5) enthaelt die Information, ob waehrend der Betriebsart "dac_conv_ext" noch auf den externeren Trigger   --
--    gewartet wird. Bit(5) => Ext_Trig_wait ist eins, wenn noch kein Trigger aufgetreten ist.                      --
--                                                                                                                  --
----------------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------
--  Vers: 3 Revi: 0: erstellt am 29.10.2013, Autor: W.Panschow                                                      --
--                                                                                                                  --
--  Aenderung 1)                                                                                                    --
--    Das Shift_Reg ist zu früh geschoben worden, deshalb ist das höchstwertige und niederwertigste Bit verloren    --
--    gegangen. Jetzt wird erst dem Schieben begonnen, wenn das erste bit in den DAC getakted wurde.                -- 
--                                                                                                                  --
----------------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------
--  Vers: 3 Revi: 1: erstellt am 06.12.2013, Autor: W.Panschow                                                      --
--                                                                                                                  --
--  Aenderung 1)                                                                                                    --
--    In der Betriebsart "Externe Triggerung" des DAC, sollen gültige Trigger-Ereignisse durch eine LED             --
--    signalisiert werden. Hierfür ist an der entity dac714 der Ausgang "ext_trig_valid" hinzugefuegt worden.       --
--    Die eigentliche Ansteuerung der Frontplatten-LED wird ausserhalb dieses Makros realisiert, da sich z.B. im    --
--    Projekt "scu_adda" zwei DACs einen Trigger-Eingang teilen muessen. Die Verknuepfung und die Opendrain-        --
--    Pulsverlaengerung wird deshalb eine Ebens hoeher realisiert.                                                  --
--                                                                                                                  --
--  Aenderung 2)                                                                                                    --
--    Die Address-Konstanten die fuer die Adressberechnung einer DAC-Instanz benoetigt werden (die gewuenschte      --
--    Basis-Adresse muss dazu addiert werden), waren zweifach definiert. Einmal hier in "dac_714.vhd" und ein       --
--    zweites mal in "dac_714_pkg.vhd".
----------------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------
--  Vers: 3 Revi: 2: erstellt am 07.02.2014, Autor: W.Panschow                                                      --
--                                                                                                                  --
--  Aenderung 1)                                                                                                    --
--    Die Entity dac714 hat ein weiteres Output-Port (DAC_convert_o) bekommen. Es wird bei jeder DAC-Konversion     --
--    fuer einen Takt aktiv Eins. Dabei spielt es keine Rolle ob die Konversion durch Software, den Funktions-      --
--    generator oder durch einen exteren Trigger ausgeloest wurde.                                                  -- 
----------------------------------------------------------------------------------------------------------------------



entity dac714 is
  generic (
    Base_addr:        unsigned(15 downto 0) := X"0300";
    CLK_in_Hz:        integer := 100_000_000;
    SPI_CLK_in_Hz:    integer := 10_000_000
    );
  port
    (
    Adr_from_SCUB_LA:   in      std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
    Data_from_SCUB_LA:  in      std_logic_vector(15 downto 0);  -- latched data from SCU_Bus 
    Ext_Adr_Val:        in      std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:      in      std_logic;                      -- '1' => Rd-Cycle is active
    Ext_Wr_active:      in      std_logic;                      -- '1' => Wr-Cycle is active
    clk:                in      std_logic;                      -- should be the same clk, used by SCU_Bus_Slave
    nReset:             in      std_logic := '1';
    nExt_Trig_DAC:      in      std_logic;                      -- external trigger input over optocoupler,
                                                                -- led on -> nExt_Trig_DAC is low
    FG_Data:            in      std_logic_vector(15 downto 0) := (others => '0');  -- parallel dac data during FG-Mode
    FG_Strobe:          in      std_logic := '0';               -- strobe to start SPI transfer (if possible) during FG-Mode
    DAC_SI:             out     std_logic;                      -- connect to DAC-SDI
    nDAC_CLK:           out     std_logic;                      -- spi-clock of DAC
    nCS_DAC:            out     std_logic;                      -- '0' enable shift of internal shift register of DAC
    nLD_DAC:            out     std_logic;                      -- '0' copy shift register to output latch of DAC
    nCLR_DAC:           buffer  std_logic;                      -- '0' resets the DAC, Clear Pulsewidth min 200ns
                                                                -- resets both the input latch and the D/A latch to 0000H (midscale).
    ext_trig_valid:     out     std_logic;                      -- got an valid external trigger, during extern trigger mode.
    DAC_convert_o:      out     std_logic;                      -- '1' when DAC convert driven by software, functiongenerator or external trigger
    Rd_Port:            out     std_logic_vector(15 downto 0);  -- output for all read sources of this macro
    Rd_Activ:           out     std_logic;                      -- this acro has read data available at the Rd_Port.
    Dtack:              out     std_logic
    );
end dac714;



architecture arch_dac714 OF dac714 IS


  constant  rw_dac_cntrl_addr:            unsigned(15 downto 0) := Base_addr + rw_dac_cntrl_addr_offset;
  constant  rw_dac_data_addr:             unsigned(15 downto 0) := Base_addr + wr_dac_addr_offset;
  constant  clr_rd_shift_err_cnt_addr:    unsigned(15 downto 0) := Base_addr + clr_rd_shift_err_cnt_addr_offset;
  constant  clr_rd_old_data_err_cnt_addr: unsigned(15 downto 0) := Base_addr + clr_rd_old_data_err_cnt_addr_offset;
  constant  clr_rd_trm_during_trm_active_err_cnt_addr:  unsigned(15 downto 0) := Base_addr + clr_rd_trm_during_trm_active_err_cnt_addr_offset;

  constant  c_spi_clk_ena_cnt:  integer := (clk_in_hz / spi_clk_in_hz) / 2;

  signal    spi_clk_ena:        std_logic;

  signal    Shift_Reg:        unsigned(15 downto 0);
  signal    Wr_Shift_Reg:     std_logic;
  signal    Wr_Shift_Reg_dly: std_logic;
  signal    Wr_DAC_Cntrl:     std_logic;
  signal    Rd_DAC_Cntrl:     std_logic;

  signal    FG_Strobe_dly:    std_logic;
  
  signal    S_Dtack:          std_logic;

  TYPE      T_SPI_SM  IS (
              Idle,
              Sel_On,   
              Clk_Lo, 
              Clk_Hi,
              Sel_Off,
              Load,
              load_Wait,
              Load_End
              );  

  signal    SPI_SM:           T_SPI_SM;

  signal    Bit_Cnt:          unsigned(4 DOWNTO 0);

  signal    spi_clk:          std_logic;  

  
  signal    build_edge:             std_logic_vector(2 downto 0);   -- shift_reg to detect external tirgger edge
  signal    Trig_DAC_with_old_data: std_logic;
  signal    Trig_DAC_during_shift:  std_logic;
  signal    wait_for_end_of_shift:  std_logic;
  signal    Trig_DAC:               std_logic;
  signal    New_trm_during_trm_active: std_logic;

  signal    SPI_TRM:          std_logic;
  
----------------- bits of control register ------------------------------------
  signal  nCLR_DAC_dly:       std_logic;
  signal  dac_conv_extern:    std_logic;    -- '1' => enable external convert dac strobe, its bit(2) of DAC control register
  signal  dac_neg_edge_conv:  std_logic;    -- '1' => negative edge convert dac strobe,   its bit(3) of DAC control register
  signal  FG_mode:            std_logic;    -- '1' => enable fuction generator mode,      its bit(4) of DAC control register

  signal  clear_spi_clk:    std_logic;

  signal  nReset_ff:        std_logic;
  signal  nReset_sync:      std_logic_vector(1 downto 0); -- design assistant generates error msg, if the input nReset isn't sychronised

  signal  Trig_DAC_during_shift_err_cnt:    unsigned (7 downto 0);
  signal  Trig_DAC_during_shift_err_cnt_b:  unsigned (7 downto 0);  -- holds during read a copy of Trig_DAC_during_shift_err_cnt 
  signal  clr_shift_err_cnt:                std_logic;
  signal  rd_shift_err_cnt:                 std_logic;
  
  signal  Trig_DAC_with_old_data_err_cnt:   unsigned (7 downto 0);
  signal  Trig_DAC_with_old_data_err_cnt_b: unsigned (7 downto 0);  -- holds during read a copy of Trig_DAC_with_old_data_err_cnt
  signal  clr_old_data_err_cnt:             std_logic;
  signal  rd_old_data_err_cnt:              std_logic;
  
  signal  New_trm_during_trm_active_err_cnt:    unsigned (7 downto 0);
  signal  New_trm_during_trm_active_err_cnt_b:  unsigned (7 downto 0);  -- holds during read a copy of New_trm_during_trm_active_err_cnt 
  signal  clr_trm_during_trm_active_err_cnt:    std_logic;
  signal  rd_trm_during_trm_active_err_cnt:     std_logic;
  signal  Ext_Trig_wait:      std_logic;
  signal  DAC_convert:        std_logic;
  signal  dac_data:           unsigned(15 downto 0);
  signal  rd_dac_data:        std_logic;

begin


spi_clk_gen:  div_n
  generic map (
    n       => c_spi_clk_ena_cnt,
    diag_on => 0
    )
  port map (
    res     => clear_spi_clk,   -- in, '1' => set "div_n"-counter asynchron to generic-value "n"-2, so the 
                                --     countdown is "n"-1 clocks to activate the "div_o"-output for one clock periode. 
    clk     => clk,             -- clk = clock
    ena     => '1',             -- in, can be used for a reduction, signal should be generated from the same 
                                --     clock domain and should be only one clock period active.
    div_o   => spi_clk_ena      -- out, div_o becomes '1' for one clock period, if "div_n" arrive n-1 clocks
                                --      (if ena is permanent '1').
    );


P_dac714_Adr_Deco: process (clk, nReset_sync(1))
  begin
    if nReset_sync(1) = '0' then
      Wr_DAC_Cntrl                      <= '0';
      Rd_DAC_Cntrl                      <= '0';
      Wr_Shift_Reg                      <= '0';
      Wr_Shift_Reg_dly                  <= '0';
      clr_shift_err_cnt                 <= '0';
      rd_shift_err_cnt                  <= '0';
      clr_old_data_err_cnt              <= '0';
      rd_old_data_err_cnt               <= '0';
      clr_trm_during_trm_active_err_cnt <= '0';
      rd_trm_during_trm_active_err_cnt  <= '0';
      rd_dac_data                       <= '0';
      S_Dtack                           <= '0';

    elsif rising_edge(clk) then
    
      Wr_DAC_Cntrl                      <= '0';
      Rd_DAC_Cntrl                      <= '0';
      Wr_Shift_Reg                      <= '0';
      clr_shift_err_cnt                 <= '0';
      rd_shift_err_cnt                  <= '0';
      clr_old_data_err_cnt              <= '0';
      rd_old_data_err_cnt               <= '0';
      clr_trm_during_trm_active_err_cnt <= '0';
      rd_trm_during_trm_active_err_cnt  <= '0';
      rd_dac_data                       <= '0';
      S_Dtack                           <= '0';
      
      Wr_Shift_Reg_dly <= Wr_Shift_Reg;

      if Ext_Adr_Val = '1' then

        case unsigned(Adr_from_SCUB_LA) IS

          when rw_dac_cntrl_addr =>
            if Ext_Wr_active = '1' then
              Wr_DAC_Cntrl  <= '1';
              S_Dtack       <= '1';
            end if;
            if Ext_Rd_active = '1' then
              Rd_DAC_Cntrl  <= '1';
              S_Dtack       <= '1';
            end if;
            
          when rw_dac_data_addr =>
            if Ext_Wr_active = '1' and FG_mode = '0' then
              Wr_Shift_Reg  <= '1';
              S_Dtack       <= '1';
            elsif Ext_Rd_active = '1' then
              rd_dac_data <= '1';
              S_Dtack     <= '1';
            end if;

          when clr_rd_shift_err_cnt_addr =>
            if Ext_Wr_active = '1' then
              clr_shift_err_cnt <= '1';
              S_Dtack           <= '1';
            end if;
            if Ext_Rd_active = '1' then
              rd_shift_err_cnt  <= '1';
              S_Dtack           <= '1';
            end if;

          when clr_rd_old_data_err_cnt_addr =>
            if Ext_Wr_active = '1' then
              clr_old_data_err_cnt  <= '1';
              S_Dtack               <= '1';
            end if;
            if Ext_Rd_active = '1' then
              rd_old_data_err_cnt <= '1';
              S_Dtack             <= '1';
            end if;

          when clr_rd_trm_during_trm_active_err_cnt_addr =>
            if Ext_Wr_active = '1' then
              clr_trm_during_trm_active_err_cnt <= '1';
              S_Dtack               <= '1';
            end if;
            if Ext_Rd_active = '1' then
              rd_trm_during_trm_active_err_cnt  <= '1';
              S_Dtack             <= '1';
            end if;

          when others =>
            Wr_DAC_Cntrl                        <= '0';
            Rd_DAC_Cntrl                        <= '0';
            Wr_Shift_Reg                        <= '0';
            clr_shift_err_cnt                   <= '0';
            rd_shift_err_cnt                    <= '0';
            clr_old_data_err_cnt                <= '0';
            rd_old_data_err_cnt                 <= '0';
            clr_trm_during_trm_active_err_cnt   <= '0';
            rd_trm_during_trm_active_err_cnt    <= '0';
            rd_dac_data                         <= '0';
            S_Dtack                             <= '0';

        end case;
      end if;
    end if;
  end process P_dac714_Adr_Deco;
  
---------------------- generate reset ---------------------------------
P_reset: process (clk)
  
  begin
    if rising_edge(clk) then

      nReset_sync <= nReset_sync(0) & nReset;       -- synchronise nReset

      nReset_ff <= '1';                             -- set nReset_ff default value to inactive

      if nReset_sync(1) = '0'
        or (nCLR_DAC = '0' and nCLR_DAC_dly = '1')  -- occurs when nCLR_DAC is set to zero, during cntrl_reg_wr
      then
        nReset_ff <= '0';
      end if;
    end if;
  end process P_reset;

-- data register for readback of set value
data_reg: process (clk, nReset)
begin
  if rising_edge(clk) then
    if nReset = '0' then
      dac_data <= (others => '0');
    elsif Wr_Shift_Reg = '1' then
      dac_data <= unsigned(Data_from_SCUB_LA);
    end if;
  end if;
end process;


P_SPI_SM: process (clk, nReset_ff)
  
  begin
    if nReset_ff = '0' then
      SPI_SM <= Idle;
      Bit_Cnt <= (others => '0');
      nCS_DAC <= '1';
      spi_clk <= '1';
      nLD_DAC <= '1';
      SPI_TRM <= '0';
      clear_spi_clk <= '1';
      Shift_Reg <= (others => '0');
      New_trm_during_trm_active <= '0';
      ext_trig_valid <= '0';
  
    elsif rising_edge(clk) then
    
      FG_Strobe_dly <= FG_Strobe;
      
      clear_spi_clk <= '0';
      New_trm_during_trm_active <= '0';
      ext_trig_valid <= '0';
      
      if SPI_SM = Load_End then
        DAC_convert <= '1';    -- '1' when DAC convert driven by software, functiongenerator or external trigger
      else
        DAC_convert <= '0';
      end if;

      if FG_mode = '0' then
        -- software mode is selected
        if Wr_Shift_Reg = '1' and Wr_Shift_Reg_dly = '0' then
          if SPI_SM = Idle then
            Shift_Reg <= unsigned(Data_from_SCUB_LA);     -- in SW mode and Idle state, load of Shift_Reg is always alowed, source scub data
            clear_spi_clk <= '1';
            SPI_TRM <= '1';
          elsif dac_conv_extern = '1' and ((SPI_SM = Load) or (SPI_SM = Load_wait)) then
            Shift_Reg <= unsigned(Data_from_SCUB_LA);     -- in SW-Mode and wait for external Trigger, load of Shift_Reg is alowed, source scub data
            clear_spi_clk <= '1';
            SPI_TRM <= '1';
          else
            New_trm_during_trm_active <= '1';
          end if;
        end if;
      else
        -- funktiongenerator mode is selected
        if FG_Strobe = '1' and FG_Strobe_dly = '0' then
          if SPI_SM = Idle then
            Shift_Reg <= unsigned(FG_Data);               -- in FG mode load Shift_Reg only in Idle state alowed, source FG_Data
            clear_spi_clk <= '1';
            SPI_TRM <= '1';
         else
            New_trm_during_trm_active <= '1';
          end if;
        end if;
      end if;

      if spi_clk_ena = '1' then
      
        case SPI_SM IS

          when Idle =>
            Bit_Cnt <= (others => '0');
            nCS_DAC <= '1';
            spi_clk <= '1';
            nLD_DAC <= '1';
            if SPI_TRM = '1' then
              SPI_SM <= Sel_On;
            end if;

          when Sel_On =>
            nCS_DAC <= '0';
            SPI_SM <= CLK_Lo;

          when CLK_Lo =>
            if Bit_Cnt <= Shift_Reg'length-1 then
              if Bit_Cnt > 0 then          -- V3R0: shift after first bit of Shift_Reg is clocked to dac.
                Shift_Reg <= (Shift_Reg(Shift_Reg'high-1 DOWNTO 0) & '0');
              end if;
              spi_clk <= '0';
              SPI_SM <= CLK_Hi;
            ELSE
              spi_clk <= '0';
              nCS_DAC <= '1';
              SPI_SM <= Sel_Off;
            end if;

          when CLK_Hi =>
            spi_clk <= '1';
            Bit_Cnt <= Bit_Cnt + 1;
            SPI_SM <= CLK_Lo;

          when Sel_Off =>
            spi_clk <= '1';
            if dac_conv_extern = '1' then
              SPI_TRM <= '0';               -- during extern trigger wait, reset the SPI_TRM,
                                            -- so you can recognize an new SPI_TRM during wait for external Trigger
            end if;
            SPI_SM <= Load;

          when Load =>
            spi_clk <= '0';
            if dac_conv_extern = '0' then -- software driven so load directly
              nLD_DAC <= '0';            
              SPI_TRM <= '0';
              SPI_SM <= Load_End;
            else                          -- extenal driven load, so wait on Trig_DAC
              if Trig_DAC = '1' then
                ext_trig_valid <= '1';
                nLD_DAC <= '0';
                SPI_TRM <= '0';
                SPI_SM <= Load_End;
              else
                SPI_SM <= Load_wait;
              end if;
            end if;

          when Load_wait =>               -- wait external triggered load
            spi_clk <= '1';
            if SPI_TRM = '1' then         -- new data during wait for Trig_DAC
              SPI_SM <= Idle;
            else
              SPI_SM <= Load;
            end if;

          when Load_End =>
            spi_clk <= '1';
            nLD_DAC <= '0';
            SPI_SM <= Idle;

        end case;
      end if;
    end if;
  end process P_SPI_SM;


P_Ext_Trig:   process (clk, nReset_ff)

  begin
    if nReset_ff = '0' then
      Trig_DAC_with_old_data <= '0';
      Trig_DAC_during_shift <= '0';
      build_edge <= "000";
      Trig_DAC <= '0';
      wait_for_end_of_shift <= '0';
    
    elsif rising_edge(clk) then
    
      build_edge <= build_edge(build_edge'high-1 downto 0) & (dac_neg_edge_conv xor nExt_Trig_DAC);
    
      Trig_DAC_with_old_data <= '0';
      Trig_DAC_during_shift <= '0';
      
      if dac_conv_extern = '1' then
        -- shift_reg to detect external tirgger
        if build_edge(build_edge'high) = '0' and build_edge(build_edge'high-1) = '1' then   
          -- external trigger (edge) detected
          if SPI_SM = Idle then
            -- external trigger during idle state, so no new DAC data available
            Trig_DAC_with_old_data <= '1';
          elsif (SPI_SM = Sel_On) or (SPI_SM = Clk_Lo) or (SPI_SM = Clk_Hi) then
            -- external trigger during DAC data spi shift operation, so its to early
            wait_for_end_of_shift <= '1';
            Trig_DAC_during_shift <= '1';
          elsif (SPI_SM = Sel_Off) or (SPI_SM = Load) or (SPI_SM = Load_wait) then
            -- external trigger during Wait for external trigger, so it's okay
            Trig_DAC <= '1';    -- the event from build_edge is only on clock active, so the event must be stored in Trig_DAC,
                                -- because the state machine SPI_SM is only every spi_clk_ena active.
          elsif wait_for_end_of_shift = '1' and (SPI_SM = Sel_OFF) then
            -- trigger gots during spi shift operation, now its finished, so trigger DAC
            wait_for_end_of_shift <= '0';
            Trig_DAC <= '1';    -- the event from build_edge is only on clock active, so the event must be stored in Trig_DAC,
                                -- because the state machine SPI_SM is only every spi_clk_ena active.
          elsif (SPI_SM = Load_End) then
            Trig_DAC <= '0';    -- DAC load is finished, so clear the trigger event storage 
          end if;
        end if;
      else
        Trig_DAC_with_old_data <= '0';
        Trig_DAC_during_shift <= '0';
        build_edge <= "000";
        Trig_DAC <= '0';
      end if;
    end if;
  end process P_Ext_Trig;


P_DAC_Cntrl:  process (clk, nReset_ff)

  variable  clr_cnt:  unsigned(1 downto 0) := "00";

  begin
    if nReset_ff = '0' then
      nCLR_DAC          <= '0';
      nCLR_DAC_dly      <= '0';
      dac_conv_extern   <= '0';
      dac_neg_edge_conv <= '0';
      clr_cnt           := "00";
      FG_mode <= '0';

    elsif rising_edge(clk) then
    
      nCLR_DAC_dly <= nCLR_DAC;             -- nCLR_DAC_dly is used to detect the reason off activating nCLR_DAC.
                                            -- In the asynchronous path nCLR_DAC and nCLR_DAC_dly are set to '0'
                                            -- driven by wr_DAC_Cntrl you can detec an edge with both signals.

      if Wr_DAC_Cntrl = '1' then
        if Data_from_SCUB_LA(1) = '1' then  -- arm clear signal of DAC
          nCLR_DAC <= '0';
          clr_cnt := "00";                  -- reset the counter for generating the correct length of clear signal
        end if;
        dac_conv_extern   <= Data_from_SCUB_LA(2) and not FG_mode;  -- '1' => enable external convert dac strobe, allowed only when FG_mode is off
        dac_neg_edge_conv <= Data_from_SCUB_LA(3) and not FG_mode;  -- '1' => negative edge convert dac strobe, allowed only when FG_mode is off
        FG_mode           <= Data_from_SCUB_LA(4);                  -- '1' => enable fuction generator mode
      else
        if nCLR_DAC = '0' then
          if spi_clk_ena = '1' then 
            if clr_cnt < 3 then
              clr_cnt := clr_cnt + 1;
            else
              nCLR_DAC <= '1';
            end if;
          end if;
        end if;
      end if;
    end if;
  end process P_DAC_Cntrl;


P_error_stat: process (clk, nReset_ff)

  begin
    if nReset_ff = '0' then
      Trig_DAC_during_shift_err_cnt <= to_unsigned(0, Trig_DAC_during_shift_err_cnt'length);
      Trig_DAC_with_old_data_err_cnt <= to_unsigned(0, Trig_DAC_with_old_data_err_cnt'length);
      New_trm_during_trm_active_err_cnt <= to_unsigned(0, New_trm_during_trm_active_err_cnt'length);
    
    elsif rising_edge(clk) then
    
      if clr_shift_err_cnt = '1' then
        Trig_DAC_during_shift_err_cnt <= to_unsigned(0, Trig_DAC_during_shift_err_cnt'length);
      elsif Trig_DAC_during_shift = '1' and (Trig_DAC_during_shift_err_cnt < (2**Trig_DAC_during_shift_err_cnt'length)-1) then
        Trig_DAC_during_shift_err_cnt <= Trig_DAC_during_shift_err_cnt + 1;
      end if;
      if rd_shift_err_cnt = '0' then                  -- during rd_shift_err_cnt Trig_DAC_during_shift_err_cnt_b can't change
        Trig_DAC_during_shift_err_cnt_b <= Trig_DAC_during_shift_err_cnt;
      end if;
    
      if clr_old_data_err_cnt = '1' then
        Trig_DAC_with_old_data_err_cnt <= to_unsigned(0, Trig_DAC_with_old_data_err_cnt'length);
      elsif Trig_DAC_with_old_data = '1' and (Trig_DAC_with_old_data_err_cnt < (2**Trig_DAC_with_old_data_err_cnt'length)-1) then
        Trig_DAC_with_old_data_err_cnt <= Trig_DAC_with_old_data_err_cnt + 1;
      end if;
      if rd_old_data_err_cnt = '0' then               -- during rd_old_data_err_cnt Trig_DAC_with_old_data_err_cnt_b can't change
        Trig_DAC_with_old_data_err_cnt_b <= Trig_DAC_with_old_data_err_cnt;
      end if;
      
      if clr_trm_during_trm_active_err_cnt = '1' then
        New_trm_during_trm_active_err_cnt <= to_unsigned(0, New_trm_during_trm_active_err_cnt'length);
      elsif New_trm_during_trm_active = '1' and (New_trm_during_trm_active_err_cnt < (2**New_trm_during_trm_active_err_cnt'length)-1) then
        New_trm_during_trm_active_err_cnt <= New_trm_during_trm_active_err_cnt + 1;
      end if;
      if rd_trm_during_trm_active_err_cnt = '0' then  -- during rd_trm_during_trm_active_err_cnt New_trm_during_trm_active_err_cnt_b can't change
        New_trm_during_trm_active_err_cnt_b <= New_trm_during_trm_active_err_cnt;
      end if;
    end if;
  end process P_error_stat;

  
DAC_SI  <= Shift_Reg(Shift_Reg'high);

P_Ext_Trig_wait: process (dac_conv_extern, SPI_SM)
  begin
    if dac_conv_extern = '1' and ((SPI_SM = Load) or (SPI_SM = Load_wait)) then
      Ext_Trig_wait <= '1';
    else
      Ext_Trig_wait <= '0';
    end if;
  end process P_Ext_Trig_wait;


nDAC_CLK <= not spi_clk;

DAC_convert_o <= DAC_convert; -- '1' when DAC convert driven by software, functiongenerator or external trigger

P_read_mux: process (rd_trm_during_trm_active_err_cnt, rd_old_data_err_cnt, rd_shift_err_cnt, Rd_DAC_Cntrl,
                     Ext_Trig_wait, FG_mode, dac_neg_edge_conv, dac_conv_extern, nCLR_DAC, SPI_TRM, 
                     Trig_DAC_during_shift_err_cnt_b, Trig_DAC_with_old_data_err_cnt_b, New_trm_during_trm_active_err_cnt_b,
                     rd_dac_data, dac_data)
  variable  sel_mux:  std_logic_vector(5 downto 0);
  begin
    sel_mux := ('0' , rd_dac_data, rd_trm_during_trm_active_err_cnt, rd_old_data_err_cnt, rd_shift_err_cnt, Rd_DAC_Cntrl);
    case sel_mux is
      when "000001" => Rd_Port <= (X"00" & "00" & Ext_Trig_wait & FG_mode & dac_neg_edge_conv & dac_conv_extern & not nCLR_DAC & SPI_TRM);
      when "000010" => Rd_Port <= (X"00" & std_logic_vector(Trig_DAC_during_shift_err_cnt_b));
      when "000100" => Rd_Port <= (X"00" & std_logic_vector(Trig_DAC_with_old_data_err_cnt_b));
      when "001000" => Rd_Port <= (X"00" & std_logic_vector(New_trm_during_trm_active_err_cnt_b));
      when "010000" => Rd_port <= std_logic_vector(dac_data);
      when others   => Rd_Port <= (others => '0');
    end case;
  end process P_read_mux;
  
Dtack <= S_Dtack;

Rd_Activ <= rd_dac_data or rd_trm_during_trm_active_err_cnt or rd_old_data_err_cnt or rd_shift_err_cnt or Rd_DAC_Cntrl;

end Arch_dac714;
