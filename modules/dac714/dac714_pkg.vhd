LIBRARY ieee;
use ieee.std_logic_1164.all;
use IEEE.numeric_std.all;
use work.aux_functions_pkg.all;

library work;

package dac714_pkg is

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

component dac714 is
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
end component dac714;

-- address offsets, used during instantiation of the component. The real address is calulated by adding the offsets to the base address
  constant  rw_dac_Cntrl_addr_offset:             unsigned(15 downto 0) := X"0000";
  constant  wr_dac_addr_offset:                   unsigned(15 downto 0) := X"0001";
  constant  clr_rd_shift_err_cnt_addr_offset:     unsigned(15 downto 0) := X"0002";
  constant  clr_rd_old_data_err_cnt_addr_offset:  unsigned(15 downto 0) := X"0003";
  constant  clr_rd_trm_during_trm_active_err_cnt_addr_offset: unsigned(15 downto 0) := X"0004";
  constant  rd_fw_version_offset:                 unsigned(15 downto 0) := X"0005";


end package dac714_pkg;
