--TITLE "'SCU_Bus_Slave_Interface'
--

----------------------------------------------------------------------------------------------------------------------
--  Vers: 1 Revi: 0: erstellt am 23.07.2009, Autor: W.Panschow                                                      --
--                                                                                                                  --
--  Das SCU_Bus-Slave-Interface (FSI) soll Entwicklern von SCU_Bus-Slave-Karten ein standardisiertes und getestetes --
--  Interface zum SCU_Bus bereitstellen. Im wesentlichen übernimmt das FSI drei Funktionen:                          --
--    a)  Die Datenkommunikation mit dem SCU_Bus-Master.                                                            --
--    b)  Den Empfang, der vom SCU_Bus-Master über den SCU_Bus verteilten, Timing-Informationen.                   --
--    c)  Den Interrupt-Controller, der 16 mögliche Interruptquellen einer Slave-Karte auf ein                      --
--        Service-Request-Signal zum SCU_Bus-Master abbildet.                                                       --
----------------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------
--  Vers_1_Revi_1: erstellt am 19.01.2010, Autor: W.Panschow                                                        --
--                                                                                                                  --
--    Vorgenommene Änderungen:                                                                                     --
--      a) Das Interface des SCU_Bus_Slaves für nachgelagerte (externe) User-Funktionen war nicht optimal ausgelegt.--
--        a1) Der Lese-Zugriff auf User-Register musste durch eine UND-Verknüpfung des SCU_Bus_Slave-Ausgangs       --
--           "ADR_Val" mit dem externen SCU_Bus-Signal "SCUB_RDnWR" ausserhalb dieses Macros erzeugt werden.        --
--           Diese Verknüpfung wird jetzt  innerhalb des Makros vorgenommen und als neues Ausgangssignal             --
--           "Ext_Rd_active" bereitgestellt.                                                                        --
--        a2) Schreibzugriffe auf Useer-Register wurden mit dem SCU_Bus_Slave-Ausgang "DS_Val" signalisiert.        --
--            Zur Verdeutlichung, ist der Ausgang "DS_Val" in "Ext_Wr_active" umbenannt worden.                     --
--      b)  Es kann die Version und die Revision dieses Makros zurückgelesen werden.                               --
--          Unter der Adresse X"0006" kann im High Byte die Version und im Low Byte die Revision ausgelesen werden. --
--          Achtung das funktioniert nur, wenn bei Änderungen auch die Konstanten "C_Version" und "C_Revision"      --
--          geändert werden!                                                                                       --
----------------------------------------------------------------------------------------------------------------------
      
----------------------------------------------------------------------------------------------------------------------
--  Vers_1_Revi_2: erstellt am 21.05.2010, Autor: W.Panschow                                                        --
--                                                                                                                  --
--    Vorgenommene Änderungen:                                                                                     --
--      a) Zwei neue Signale sollen das Interface zu externen Registern (oder Fifos) erleichtern.                   --
--         "Ext_Wr_fin" steht für extern-write-finished. Es signalisiert für eine Clockperiode von "clk" mit einem  --
--         aktiv-Eins-Pegel das Ende eines Schreibzyklusses vom SCU-Bus. Das Signal ist nur für ausserhalb           --
--         dieses Makros liegende (externe) Registerzugriffe aktiv.                                                 --
--         "Ext_Rd_fin" steht für extern-read-finished. Es signalisiert für eine Clockperiode von "clk" mit einem   --
--         Aktiv-Eins-Pegel das Ende eines Lesezyklusses vom SCU-Bus. Das Signal ist nur für ausserhalb              --
--         dieses Makros liegende (externe) Registerzugriffe aktiv.                                                 --
--                                                                                                                  --
--      b) Generic-Parameter "Ext_Fin_Sig_overlap" ist hinzugekommen.                                               --
--                                                                                                                  --
--      "Ext_Fin_Sig_overlap" = 1 => Die Signale überlappen sich am Ende des exterenen Buszyklusses für einen Takt. --
--      "Ext_Rd_Fin" oder "Ext_Wr_Fin"      _________________________|===|___________                               --
--      "Ext_Rd_active oder "Ext_Wr_active  _________|===================|___________                               --
--      "Clk"                               _|=|_|=|_|=|_|=|_|=|_|=|_|=|_|=|_|=|_|=|_                               --
--                                                                                                                  --
--      "Ext_Fin_Sig_overlap" = 0 => Die Signale folgen einen Takt nach dem Ende des externen Buszyklusses.         --
--      "Ext_Rd_Fin" oder "Ext_Wr_Fin"      _____________________________|===|_______                               --
--      "Ext_Rd_active oder "Ext_Wr_active  _________|===================|___________                               --
--      "Clk"                               _|=|_|=|_|=|_|=|_|=|_|=|_|=|_|=|_|=|_|=|_                               --
--                                                                                                                  --
--      c) Anpassungen vorgenommen, damit die Standard-Bibliothek "IEEE.numeric_std.all" verwendet werden kann.     --
--                                                                                                                  --
--      d) Die Adressen X"0000" bis X"0026" sind jetzt durchgängig für Resourcen innerhalb dieses Makros reserviert.--
--         D.h. externe (user) Register können erst ab der Adresse X"0027" angesprochen werden.                      -- 
----------------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------
--  Vers_2_Revi_0: erstellt am 26.07.2010, Autor: W.Panschow                                                        --
--                                                                                                                  --
--    Vorgenommene Änderungen:                                                                                     --
--      a)Die unter Vers_1_Revi_2 hinzugefügten Signale "Ext_Wr_Fin" und "Ext_Rd_Fin" konnten mit dem Generic-     --
--        Parameter "Ext_Fin_Sig_overlap" so eingestellt werden, dass sie gleichzeitig mit dem Ende des jeweiligen  --
--        Schreib- oder Lesezyklusses aktiv werden, oder erst nach dem Abschluss des Zugriffs.                      --
--        Dies hat den Nachteil, dass die Signale für alle externen Makros nur in der einen oder anderen Form zur  --
--        Verfügung stehen. Besser ist es, beide Formen parallel anzubieten.                                       --
--        Deshalb sind zwei neue Signale hinzugekommen, "Ext_Wr_Fin_ovl" und "Ext_Rd_Fin_ovl".                      --
--        Beide Signale beenden den jeweiligen Zyklus mit einen Takt Überlappung.                                  --
--        "Ext_Wr_Fin" und "Ext_Rd_Fin" werden jetzt immer erst nach dem Ende des jeweiligen Zyklusses für         --
--        einen Takt aktiv sein (keine  Überlappung).                                                                --
--        Der Generic-Parameter "Ext_Fin_Sig_overlap" wird aus obigen Gründen nicht mehr beötigt.                   --
--                                                                                                                  --
--      b)  Auf Vorschlag von S. Schäfer ist der Lesezyklus des SCU_Busses überarbeitet worden.                     --
--      b1) Zu Beginn des Lesezugriffes wurde immer für einen Takt das Datum des vorherigen Lesezugriffes          --
--          auf den SCU_Bus-Datenbus angelegt bevor das gewünschte Datum nachfolgte. Verursacht unnötige Störungen.  --
--      b2) Bei einem Lesezugriff auf eine  n i c h t  existierende extene Resource wurde auf den SCU_Bus-Datenbus  --
--          das Datum des zuvor ausgeführten Lesezugriffes ausgegeben. Es wurde zwar kein Dtack generiert, aber    --
--          der Datenbus-Einschwing-Vorgang ist unnötig.                                                           --
----------------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------
--  Vers_2_Revi_1: erstellt am 02.08.2010, Autor: W.Panschow                                                        --
--                                                                                                                  --
--    Vorgenommene Änderungen:                                                                                     --
--      a)  Mit der unter Vers_2_Revi_0 -b) vorgenommenen Änderung in der Aktivierung des SCU_Bus-Datentreibers,     --
--          wird das Dtack der User-Register um ca. 20 ns verzögert.                                               --
----------------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------
--  Vers_2_Revi_2: erstellt am 12.11.2010, Autor: W.Panschow                                                        --
--                                                                                                                  --
--    Vorgenommene Änderungen:                                                                                     --
--      a)Das SCU_Bus_Signal "nReset" ist der null-aktive Reset des SCU_Bus_Masters. Hier wurden beim Einsatz der   --
--        Interfacekarte als SCU_Bus_Master Störungen auf dieser Leitung festgestellt. Diese konnten leicht zur    -- 
--        Störungen im SCU_Bus_Slave führen, da das Signal "nReset" mit den asynchronen Teil der Prozesse           --
--        verbunden war. Zur Abhilfe wird "nReset" mit dem Makro debounce.vhd entprellt.                            --
----------------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------
--  Vers_2_Revi_3: erstellt am 26.01.2012, Autor: W.Panschow                                                        --
--                                                                                                                  --
--    Vorgenommene Änderungen:                                                                                     --
--      Die Signale "Ext_Rd_active" und "Ext_Wr_active" werden zum Ansteuern von externen Makros verwendet.         --
--      Durch das getaktete Signal "S_Adr_Val" kann auf den  "Ext_Rd_active"- oder "Ext_Wr_active"-Signal ein       --
--      ungültiger Puls  entstehen, wenn der Masterzugriff mit einem Timeout abgegrochen wurde.                      --
--      Durch das "Abklemmen" mit S_nSync_Board_Sel = "00" wird dies korrigiert.                                    --
--      Bis Vers_2_Revi_2:                                                                                          --
--        Ext_Rd_active <= '1' when (S_Adr_Val = '1' and SCUB_RDnWR = '1') else '0';                                --
--        Ext_Wr_active <= '1' when (S_Adr_Val = '1' and SCUB_RDnWR = '0') else '0';                                --
--      Ab Vers_2_Revi_3:                                                                                           --
--        Ext_Rd_active <= '1' when (S_DS_Val = '1' and S_nSync_Board_Sel = "00" and SCUB_RDnWR = '1') else '0';    --
--        Ext_Wr_active <= '1' when (S_DS_Val = '1' and S_nSync_Board_Sel = "00" and SCUB_RDnWR = '0') else '0';    --
----------------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------
--  Vers_2_Revi_4: erstellt am 26.11.2012, Autor: W.Panschow                                                        --
--                                                                                                                  --
--    Vorgenommene Änderungen:                                                                                     --
--      a)  Um zu verdeutlichen das 'nReset' ein SCU-Bus-Signal ist, wurde es in 'nSCUB_Reset_in' umbenannt.        --
--      b)  Damit externe Makros das entprellte 'nSCUB_Reset_in' als Reset verwenden können, wurde der             --
--        Ausgang 'Deb_SCUB_Reset_out' hinzugefügt.                                                                  --
----------------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------
--  Vers_2_Revi_5: erstellt am 31.01.2013, Autor: W.Panschow                                                        --
--                                                                                                                  --
--    Vorgenommene Änderungen:                                                                                     --
--      Wird ein Zugriff auf ausserhalb dieses Makros befindliche Resourcen (Anwender-Funktionen) mit einem         --
--      Timeout beendet (die angesprochene Adresse liefert kein Dtack), dann wurden die Signale "Ext_Rd_fin",       --
--      "Ext_Rd_fin_ovl", "Ext_Wr_fin" und "Ext_Wr_fin_ovl" akiviert. Dies wird jetzt verhindert indem die Signale  --
--      nur noch erzeugt werden, wenn die Anwender-Funktionen ein Dtack liefern.                                    --
--                                                                                                                  --
--  Bis Vers_2_Revi_4:                                                                                              --
--    Ext_Rd_fin_ovl  <= '1' when (S_Adr_Val = '1' and SCUB_RDnWR = '1' and S_nSync_Board_Sel = "01") else '0';     --
--    Ext_Wr_fin_ovl  <= '1' when (S_DS_Val = '1' and SCUB_RDnWR = '0' and S_nSync_Board_Sel = "01") else '0';      --
--  Ab Vers_2_Revi_5:                                                                                               --
--    Ext_Rd_fin_ovl  <= '1' when (S_Adr_Val = '1' and SCUB_RDnWR = '1' and S_nSync_Board_Sel = "01"                --
--                                 and S_SCUB_Dtack = '1') else '0';                                                --
--    Ext_Wr_fin_ovl  <= '1' when (S_DS_Val = '1' and SCUB_RDnWR = '0' and S_nSync_Board_Sel = "01"                 --
--                                 and S_SCUB_Dtack = '1') else '0';                                                --
--                                                                                                                  --
--  Bis Vers_2_Revi_4:                                                                                              --
--    P_no_fin_sig_overap: process (clk)                                                                            --
--      begin                                                                                                       --
--        if rising_edge(clk) then                                                                                  --
--          if (S_Adr_Val = '1' and SCUB_RDnWR = '1' and S_nSync_Board_Sel = "01") then                             --
--            Ext_Rd_fin <= '1';                                                                                    --
--          else                                                                                                    --
--            Ext_Rd_fin <= '0';                                                                                    --
--          end if;                                                                                                 --
--          if (S_DS_Val = '1' and SCUB_RDnWR = '0' and S_nSync_Board_Sel = "01") then                              --
--            Ext_Wr_fin <= '1';                                                                                    --
--          else                                                                                                    --
--            Ext_Wr_fin <= '0';                                                                                    --
--          end if;                                                                                                 --
--        end if;                                                                                                   --
--      end process P_no_fin_sig_overap;                                                                            --
--                                                                                                                  --
--  Ab Vers_2_Revi_5:                                                                                               --
--    P_no_fin_sig_overap: process (clk)                                                                            --
--      begin                                                                                                       --
--        if rising_edge(clk) then                                                                                  --
--          if (S_Adr_Val = '1' and SCUB_RDnWR = '1' and S_nSync_Board_Sel = "01" and S_SCUB_Dtack = '1') then      --
--            Ext_Rd_fin <= '1';                                                                                    --
--          else                                                                                                    --
--            Ext_Rd_fin <= '0';                                                                                    --
--          end if;                                                                                                 --
--          if (S_DS_Val = '1' and SCUB_RDnWR = '0' and S_nSync_Board_Sel = "01" and S_SCUB_Dtack = '1') then       --
--            Ext_Wr_fin <= '1';                                                                                    --
--          else                                                                                                    --
--            Ext_Wr_fin <= '0';                                                                                    --
--          end if;                                                                                                 --
--        end if;                                                                                                   --
--      end process P_no_fin_sig_overap;                                                                            --
----------------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------
--  Vers_3_Revi_0: erstellt am 06.02.2013, Autor: W.Panschow                                                        --
--    Die Version und Revision dieses Makros soll nicht mehr aus dem Namen ersichtlich sein. Die letzte Version die --
--    diesen Schema folgte, lautete "SCU_Bus_Slave_V2R5".                                                           --
--    Ab der Vers_3_Revi_0, wird der Makro nur noch "SCU_Bus_Slave" heissen.                                        --
--    Wenn der "SCU_Bus_Slave" in einem Block-Design-File als Symbol eingebunden werden soll, wird über die Generics--
--      "This_macro_vers_dont_change_from_outside" die Version und                                                  --
--      "This_macro_revi_dont_change_from_outside" die Revision angezeigt.                                          --
--    Das setzt vorraus, das vom aktuell eingebundenen SCU_Bus_Slave.vhd-File ein neues BSF generiert wird.         --
--    Wird der SCU_Bus_Slave als Vhdl-Componente in ein in der Hirachie höheren Vhdl-File eingefügt, muesste die    --
--    Componenten-Deklaration haendisch aktualisiert werden. Deshalb soll dieser Makro als Package in das           --
--    übergeordnete Vhdl-File eingebunden werden.                                                                   --
----------------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------
--  Vers_3_Revi_1: erstellt am 22.03.2013, Autor: W.Panschow                                                        --
--    Die Generics "Hardware_Version" und "Hardware_Release" sollen Elemente der CID-Nummern repräsentiern.         --
--    Da diese Nummern für alle Baugruppen die im Fair-Projekt eingesetzt werden sollen, eindeutig sein müssen,     --
--    ist es wichtig, dass nicht jeder Entwickler eine Nummer nach gutdünken vergibt. Um zu verdeutlichen, dass     --
--    über diese Generics nur die CID-Elemente "System" und "Gruppe" referiert werden dürfen, wurde sie umbenannt:  --
--                                                                                                                  --
--      "Hardware_Version" in "CID_System".                                                                         --
--            "CSCOHW" hat z.B. die "CID_System"-Kennung dezimal 55. Baugruppen anderer Gewerke-Hersteller sollten  --
--            verbindlich ihre "CID_System"-Kennung eintragen. Falls keine vorhanden ist, darf der                  --
--            Defaultwert 0 nicht verändert werden.                                                                 --
--                                                                                                                  --
--      "Hardware_Revision" in "CID_Group".                                                                         --
--            Jede Baugruppe die diesen Macro verwendet sollte durch die "CID-Group" zusammen mit "CID-System"      --
--            eine eindeutige Identifizierug der Hardware-Funktion und deren Revision ermöglichen.                  --
--            Die "CID-Group"-Nummer wird bei jeder neuen Karte oder jeder neuen Revision hochgezählt.              --
--            Z.B. "CSCOHW" hat die Karte "FG900160_SCU_ADDAC1" entwickelt, für die die "CID_Group"-Nummer          --
--            0003 dezimal vergeben wurde.                                                                          --
--            Eine neue Version der Baugruppe, z.B. "FG900161_SCU_ADDAC2" könnte die "CID-Group"-Nummer 0011        --
--            haben, da zwischenzeitlich "CID-Group"-Nummern für andere Projekte (Funktionen) vergeben wurden,      --
--            und die "CID-Group"-Nummer kontinuierlich hochgezählt werden soll.                                    --
--            Falls keine verbindliche "CID-Group"-Nummer vorliegt, muss der Defaultwert 0 stehen bleiben!          --
----------------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------
--  Vers_3_Revi_2: erstellt am 30.04.2013, Autor: W.Panschow                                                        --
--    Eventuell auftretendes Latch von "S_SCUB_Dtack" und "S_DS_Val" entfernt.                                      --
----------------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------
--  Vers_3_Revi_3: erstellt am 21.05.2013, Autor: W.Panschow                                                        --
--    Process "P_Intr" ueberarbeitet.                                                                               --
----------------------------------------------------------------------------------------------------------------------


library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use ieee.math_real.all;

library work;
use work.aux_functions_pkg.debounce;

entity SCU_Bus_Slave is

generic
    (
    CLK_in_Hz:        integer := 100_000_000;             -- frequency of the "SCU_Bus_Slave" clock in Hz,
                                                          -- should be higher then 100 Mhz
    Slave_ID:         integer range 0 TO 16#FFFF# := 0;   -- ID of the realisied slave board function
    Firmware_Version: integer range 0 to 16#FFFF# := 0;
    Firmware_Release: integer range 0 to 16#FFFF# := 0;

    -- "CSCOHW" hat z.B. die "CID_System"-Kennung dezimal 55. Baugruppen anderer Gewerke-Hersteller sollten verbindlich
    -- ihre "CID_System"-Kennung eintragen. Falls keine vorhanden ist, darf der Defaultwert 0 nicht verändert werden.
    CID_System:       integer range 0 to 16#FFFF# := 0;

    -- Jede Baugruppe die diesen Macro verwendet sollte durch die "CID-Group" zusammen mit "CID-System" eine eindeutige
    -- Identifizierug der Hardware-Funktion und deren Revision ermöglichen. Die "CID-Group"-Nummer wird bei jeder neuen Karte
    -- oder jeder neuen Revision hochgezählt.Z.B. "CSCOHW" hat die Karte "FG900160_SCU_ADDAC1" entwickelt,
    -- für die die "CID_Group"-Nummer 0003 dezimal vergeben wurde.
    -- Eine neue Version der Baugruppe, z.B. "FG900161_SCU_ADDAC2" könnte die "CID-Group"-Nummer 0011 haben, da
    -- zwischenzeitlich  "CID-Group"-Nummern für andere Projekte (Funktionen) vergeben wurden, und die "CID-Group"-Nummer
    -- kontinuierlich hochgezählt werden soll.                                  --
    -- Falls keine verbindliche "CID-Group"-Nummer vorliegt, muss der Defaultwert 0 stehen bleiben!
    CID_Group: integer range 0 to 16#FFFF# := 0;

    -- the bit positions are corresponding to Intr_In.
    -- A '1' set default level of this Intr_In(n) to neg. level or neg. edge
    Intr_Level_Neg:   std_logic_vector(15 DOWNTO 1) := B"0000_0000_0000_000";

    -- the bit positions are corresponding to Intr_In.
    -- A '1' set default of this Intr_In(n) to edge triggered, '0' is level triggered
    Intr_Edge_Trig:   std_logic_vector(15 DOWNTO 1) := B"1111_1111_1111_110";
    
    -- the bit positions are corresponding to Intr_In.
    -- A '1' enable Intr_In(n), '0' disable Intr_In(n)
    Intr_Enable:      std_logic_vector(15 DOWNTO 1) := B"0000_0000_0000_001";
                                                                              
    -- change only here! increment by major changes of this macro
    This_macro_vers_dont_change_from_outside: integer range 0 to 16#FF# := 3;
    
    -- change only here! increment by minor changes of this macro
    This_macro_revi_dont_change_from_outside: integer range 0 to 16#FF# := 3
    );
port
    (
    SCUB_Addr:          in    std_logic_vector(15 DOWNTO 0);  -- SCU_Bus: address bus
    nSCUB_Timing_Cyc:   in    std_logic;                      -- SCU_Bus signal: low active SCU_Bus runs timing cycle
    SCUB_Data:          inout std_logic_vector(15 DOWNTO 0);  -- SCU_Bus: data bus (FPGA tri state buffer)
    nSCUB_Slave_Sel:    in    std_logic;                      -- SCU_Bus: '0' => SCU master select slave
    nSCUB_DS:           in    std_logic;                      -- SCU_Bus: '0' => SCU master activate data strobe
    SCUB_RDnWR:         in    std_logic;                      -- SCU_Bus: '1' => SCU master read slave
    clk:                in    std_logic;                      -- clock of "SCU_Bus_Slave"
    nSCUB_Reset_in:     in    std_logic;                      -- SCU_Bus-Signal: '0' => 'nSCUB_Reset_In' is active
    Data_to_SCUB:       in    std_logic_vector(15 DOWNTO 0);  -- connect read sources from external user functions
    Dtack_to_SCUB:      in    std_logic;                      -- connect Dtack from from external user functions

    -- 15 interrupts from external user functions
    Intr_In:            in    std_logic_vector(15 DOWNTO 1) := NOT Intr_Level_Neg(15 Downto 1);
    
    -- '1' => the user function(s), device, is ready to work with the control system
    User_Ready:         in    std_logic;

    -- latched data from SCU_Bus for external user functions
    Data_from_SCUB_LA:  out   std_logic_vector(15 DOWNTO 0);

    -- latched address from SCU_Bus for external user functions
    ADR_from_SCUB_LA:   out   std_logic_vector(15 DOWNTO 0);

    -- latched timing pattern from SCU_Bus for external user functions
    Timing_Pattern_LA:  out   std_logic_vector(31 DOWNTO 0);

    Timing_Pattern_RCV: out   std_logic;    -- timing pattern received
    nSCUB_Dtack_Opdrn:  out   std_logic;    -- for direct connect to SCU_Bus opendrain signal - '0' => slave give
                                            -- dtack to SCU master
    SCUB_Dtack:         out   std_logic;    -- for connect via ext. open collector driver - '1' => slave give
                                            -- dtack to SCU master
    nSCUB_SRQ_Opdrn:    out   std_logic;    -- for direct connect to SCU_Bus opendrain signal - '0' => slave
                                            -- service request to SCU master
    SCUB_SRQ:           out   std_logic;    -- for connect via ext. open collector driver - '1' => slave
                                            -- service request to SCU master
    nSel_Ext_Data_Drv:  out   std_logic;    -- '0' => select the external data driver on the SCU_Bus slave
    Ext_Data_Drv_Rd:    out   std_logic;    -- '1' => direction of the external data driver on the SCU_Bus slave
                                            -- is to the SCU_Bus
    Standard_Reg_Acc:   out   std_logic;    -- '1' => mark the access to register of this macro
    Ext_Adr_Val:        out   std_logic;    -- for external user functions: '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:      out   std_logic;    -- '1' => Rd-Cycle to external user register is active
    Ext_Rd_fin:         out   std_logic;    -- marks end of read cycle, active one for one clock period
                                            -- of clk past cycle end (no overlap)
    Ext_Rd_Fin_ovl:     out   std_logic;    -- marks end of read cycle, active one for one clock period
                                            -- of clk during cycle end (overlap)
    Ext_Wr_active:      out   std_logic;    -- '1' => Wr-Cycle to external user register is active
    Ext_Wr_fin:         out   std_logic;    -- marks end of write cycle, active one for one clock period
                                            -- of clk past cycle end (no overlap)
    Ext_Wr_fin_ovl:     out   std_logic;    -- marks end of write cycle, active one for one clock period
                                            -- of clk during cycle end (overlap)
    Deb_SCUB_Reset_out: out   std_logic;    -- the debounced SCU-Bus signal 'nSCUB_Reset_In'. Use for other macros.
    nPowerup_Res:       out   std_logic     -- '0' => the FPGA make a powerup
    );
  
  constant  Clk_in_ps: integer  := 1000000000 / (Clk_in_Hz / 1000);
  constant  Clk_in_ns: integer  := 1000000000 / Clk_in_Hz;
  
  constant  C_Version:  unsigned(7 downto 0) :=  to_unsigned(This_macro_vers_dont_change_from_outside, 8);
  constant  C_Revision: unsigned(7 downto 0) :=  to_unsigned(This_macro_revi_dont_change_from_outside, 8);
  
  constant  C_Slave_ID_Adr:   unsigned(15 downto 0) := X"0001";   -- address of slave ident code (rd)
  constant  C_FW_Version_Adr: unsigned(15 downto 0) := X"0002";   -- address of firmware version (rd)
  constant  C_FW_Release_Adr: unsigned(15 downto 0) := X"0003";   -- address of firmware release (rd)
  constant  C_CID_System_Adr: unsigned(15 downto 0) := X"0004";   -- address of hardware version (rd)
  constant  C_CID_Group_Adr:  unsigned(15 downto 0) := X"0005";   -- address of hardware release (rd)
  -- address of version and revision register of this macro (rd)
  constant  C_Vers_Revi_of_this_Macro: unsigned(15 downto 0) := X"0006";
  constant  C_Echo_Reg_Adr:   unsigned(15 downto 0) := X"0010";   -- address of echo register (rd/wr)
  constant  C_Status_Reg_Adr: unsigned(15 downto 0) := X"0011";   -- address of status register (rd)
  
  constant  C_Free_Intern_A_12: unsigned(15 downto 0) := X"0012";   -- reserved internal address 12hex
  constant  C_Free_Intern_A_13: unsigned(15 downto 0) := X"0013";   -- reserved internal address 13hex
  constant  C_Free_Intern_A_14: unsigned(15 downto 0) := X"0014";   -- reserved internal address 14hex
  constant  C_Free_Intern_A_15: unsigned(15 downto 0) := X"0015";   -- reserved internal address 15hex
  constant  C_Free_Intern_A_16: unsigned(15 downto 0) := X"0016";   -- reserved internal address 16hex
  constant  C_Free_Intern_A_17: unsigned(15 downto 0) := X"0017";   -- reserved internal address 17hex
  constant  C_Free_Intern_A_18: unsigned(15 downto 0) := X"0018";   -- reserved internal address 18hex
  constant  C_Free_Intern_A_19: unsigned(15 downto 0) := X"0019";   -- reserved internal address 19hex
  constant  C_Free_Intern_A_1A: unsigned(15 downto 0) := X"001A";   -- reserved internal address 1Ahex
  constant  C_Free_Intern_A_1B: unsigned(15 downto 0) := X"001B";   -- reserved internal address 1Bhex
  constant  C_Free_Intern_A_1C: unsigned(15 downto 0) := X"001C";   -- reserved internal address 1Chex
  constant  C_Free_Intern_A_1D: unsigned(15 downto 0) := X"001D";   -- reserved internal address 1Dhex
  constant  C_Free_Intern_A_1E: unsigned(15 downto 0) := X"001E";   -- reserved internal address 1Ehex
  constant  C_Free_Intern_A_1F: unsigned(15 downto 0) := X"001F";   -- reserved internal address 1Fhex

  constant  C_Intr_In_Adr:      unsigned(15 downto 0) := X"0020";   -- address of interrupt In register (rd)
  constant  C_Intr_Ena_Adr:     unsigned(15 downto 0) := X"0021";   -- address of interrupt enable register (rd/wr)
  constant  C_Intr_Pending_Adr: unsigned(15 downto 0) := X"0022";   -- address of interrupt pending register (rd/wr)
  constant  C_Intr_Mask_Adr:    unsigned(15 downto 0) := X"0023";   -- address of interrupt mask register (rd/wr)
  constant  C_Intr_Active_Adr:  unsigned(15 downto 0) := X"0024";   -- address of interrupt active register (rd)
  constant  C_Intr_Level_Adr:   unsigned(15 downto 0) := X"0025";   -- address of interrupt level register (rd/wr)
  constant  C_Intr_Edge_Adr:    unsigned(15 downto 0) := X"0026";   -- address of interrupt edge register (rd/wr)

  signal    S_nReset:           std_logic;                        -- '0' => S_nReset is active

  signal    S_ADR_from_SCUB_LA:   std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
  signal    S_Adr_Val:            std_logic;                      -- for external address decoding "ADR_from_SCUB_LA" is valid
  signal    S_Data_from_SCUB_LA:  std_logic_vector(15 downto 0);  -- latched data from SCU_Bus
  signal    S_Timing_Pattern_LA : std_logic_vector(31 downto 0);  -- latched timing pattern from SCU_Bus
  signal    S_Timing_Pat_RCV:     std_logic;                      -- generate pulse if a new timing pattern latched
  signal    S_Timing_Pat_RCV_Dly: std_logic;                      -- generate delayed pulse if a new timing pattern latched

  signal    S_nSync_Board_Sel:    std_logic_vector(1 downto 0);   -- synchronize nSCUB_Slave_Sel and generate pulse on neg edge
  signal    S_nSync_DS:           std_logic_vector(1 downto 0);   -- synchronize nSCUB_DS and generate pulse on neg edge
  signal    S_nSync_Timing_Cyc:   std_logic_vector(1 downto 0);   -- synchronize nSCUB_Timing_Cyc and generate pulse on neg edge

  signal    S_DS_Val:             std_logic;

  signal    S_SCUB_Dtack:         std_logic;

  signal    S_Echo_Reg:           std_logic_vector(15 downto 0);
  
  signal    S_Read_Out:           std_logic_vector(15 downto 0);

  signal    S_Intr_In_Sync0:      std_logic_vector(Intr_In'range);
  signal    S_Intr_In_Sync1:      std_logic_vector(Intr_In'range);
  signal    S_Intr_In_Sync2:      std_logic_vector(Intr_In'range);
  signal    S_Intr_Enable:        std_logic_vector(Intr_In'range);
  signal    S_Intr_Mask:          std_logic_vector(Intr_In'range);
  signal    S_Intr_Pending:       std_logic_vector(Intr_In'range);
  signal    S_Wr_Intr_Pending:    std_logic_vector(1 downto 0);
  signal    S_Intr_Active:        std_logic_vector(Intr_In'range);
  signal    S_Wr_Intr_Active:     std_logic_vector(1 downto 0);
  signal    S_Intr_Level_Neg:     std_logic_vector(Intr_In'range);
  signal    S_Intr_Edge_Trig:     std_logic_vector(Intr_In'range);
  signal    S_SRQ:                std_logic;
  
  signal    S_Standard_Reg_Acc  : std_logic;

  constant  C_Powerup_time_in_ns: integer := 500;
  constant  C_Powerup_Res_Cnt:    integer := (C_Powerup_time_in_ns * 1000 / Clk_in_ps);
  constant  C_Powerup_Res_Cnt_len:  integer := integer(ceil(log2(real(C_Powerup_Res_Cnt))));
  signal    S_Powerup_Res_Cnt:    unsigned(C_Powerup_Res_Cnt_len downto 0) := (others => '0');
  
  signal    S_Powerup_Res:        std_logic;
  signal    S_Powerup_Done:       std_logic;
  
  constant  C_Dtack_dly_in_ns:    integer := 20;
  constant  C_Dtack_dly_cnt:      integer := integer(ceil(real(clk_in_hz) / real(1_000_000_000) * real(C_Dtack_dly_in_ns)));
  signal    S_Dtack_to_SCUB_Dly:  std_logic_vector(C_Dtack_dly_cnt downto 0);   -- als Schieberegister
  
  constant  C_Debounce_nReset_in_ns:  integer := 500;   -- Vers_2_Revi_2
  signal    S_Deb_Reset:          std_logic;            -- Vers_2_Revi_2
  signal    Res:                  std_logic;
  
  end SCU_Bus_Slave;


ARCHITECTURE Arch_SCU_Bus_Slave OF SCU_Bus_Slave IS


begin

ASSERT NOT (Clk_in_Hz < 100000000)
  REPORT "Achtung Generic Clk_in_Hz ist auf " & integer'image(Clk_in_Hz)
      & " gesetzt. Mit der Periodendauer von " & integer'image(Clk_in_ns)
      & " ns laesst sich kein schnelles Slaveinterface realisieren"
SEVERITY Warning;

ASSERT false
  REPORT "Das SCUB-Signal nSCUB_Reset_in wird mit "
      & integer'image(C_Debounce_nReset_in_ns) & " ns entprellt."   -- Vers_2_Revi_2
SEVERITY NOTE;

ASSERT false
  REPORT "'Der SCU_Bus_Slave'-Makro hat die Version: "
      & integer'image(This_macro_vers_dont_change_from_outside)
      & " und die Revision: "
      & integer'image(This_macro_revi_dont_change_from_outside)     -- ab Vers_3_Revi_0
SEVERITY NOTE;

ASSERT  false
  REPORT "S_Dtack_to_SCUB_Dly length:  " & integer'image(S_Dtack_to_SCUB_Dly'length)
SEVERITY note;


Res <= not nSCUB_Reset_in; -- Modelsim kann kein not in der Signalzuweisung

Deb_nReset: Debounce
GENERIC MAP
    (
    -- Vers_2_Revi_2: Count (DB_Cnt) für die Entprellung (C_Debounce_nReset_in_ns / clk_in_ns)
    DB_Cnt => C_Debounce_nReset_in_ns / clk_in_ns
    )
PORT MAP
    (                         -- Vers_2_Revi_2
    DB_In   => Res,
    Reset   => S_Powerup_Res,
    clk     => clk,
    DB_Out  => S_Deb_Reset
    );
                
P_Reset:  process (clk, S_Deb_Reset, S_Powerup_Res)         -- Vers_2_Revi_2
  begin
    if rising_edge(clk) then
      if S_Deb_Reset = '1' OR S_Powerup_Res = '1' then      -- Vers_2_Revi_2
        S_nReset <= '0';
      else
        S_nReset <= '1';
      end if;
    end if;
  end process P_Reset;
  

P_Powerup:  process (clk)
  begin
    if rising_edge(clk) then
      if S_Powerup_Res_Cnt <= C_Powerup_Res_Cnt - 2 then
        S_Powerup_Res_Cnt <= S_Powerup_Res_Cnt + 1;
        S_Powerup_Res <= '1';
      else
        S_Powerup_Res <= '0';
      end if;
      if S_Powerup_Res_Cnt = C_Powerup_Res_Cnt - 2 then
        S_Powerup_Done <= '1';
      end if;
      if S_Wr_Intr_Active(0) = '1' and S_Data_from_SCUB_LA(0) = '1' then
        S_Powerup_Done <= '0';
      end if;
    end if;
  end process P_Powerup;


P_Adr_LA: process (clk, S_nReset)
  begin
    if S_nReset = '0' then
      S_ADR_from_SCUB_LA <= (others => '0');
      S_nSync_Board_Sel <= (others => '1');
    elsif rising_edge(clk) then
      S_nSync_Board_Sel <= (S_nSync_Board_Sel(S_nSync_Board_Sel'high-1 downto 0) & nSCUB_Slave_Sel);
      if S_nSync_Board_Sel = "10" then
        S_ADR_from_SCUB_LA <= SCUB_Addr;
      end if;
    end if;
  end process P_Adr_LA;


P_Data_LA:  process (clk, S_nReset)
  begin
    if S_nReset = '0' then
      S_Data_from_SCUB_LA <= (others => '0');
      S_nSync_DS <= (others => '1');
    elsif rising_edge(clk) then
      S_nSync_DS <= (others => '1');
      if S_nSync_Board_Sel = "00" then
        S_nSync_DS <= (S_nSync_DS(S_nSync_DS'high-1 downto 0) & nSCUB_DS);
        if S_nSync_DS = "10" and SCUB_RDnWR = '0' then
          S_Data_from_SCUB_LA <= SCUB_Data;
        end if;
      end if;
    end if;
  end process P_Data_LA;


P_Timing_LA:  process (clk, S_nReset)
  begin
    if S_nReset = '0' then
      S_Timing_Pattern_LA <= (others => '0');
      S_nSync_Timing_Cyc <= (others => '1');
      S_Timing_Pat_RCV <= '0';
      S_Timing_Pat_RCV_Dly <= '0';
    elsif rising_edge(clk) then
      S_Timing_Pat_RCV <= '0';
      S_Timing_Pat_RCV_Dly <= S_Timing_Pat_RCV;
      S_nSync_Timing_Cyc <= (S_nSync_Timing_Cyc(S_nSync_Timing_Cyc'high-1 downto 0) & nSCUB_Timing_Cyc);
      if S_nSync_Timing_Cyc = "10" and SCUB_RDnWR = '0' then
        S_Timing_Pattern_LA <= (SCUB_Addr & SCUB_Data);
        S_Timing_Pat_RCV <= '1';
      end if;
    end if;
  end process P_Timing_LA;


P_Intr: process (clk, S_nReset, S_Powerup_Done)
  begin
    if S_nReset = '0' then
      S_Intr_In_Sync0 <= (others => '0');
      S_Intr_In_Sync1 <= (others => '0');
      S_Intr_In_Sync2 <= (others => '0');
      S_Intr_Pending  <= (others => '0');
      S_Intr_Active   <= (others => '0');

    elsif rising_edge(clk) then
      FOR i in Intr_In'low TO Intr_In'high LOOP
        -- convert 'Intr_In' to positive level and made first synchronisation
        S_Intr_In_Sync0(i) <= Intr_In(i) XOR S_Intr_Level_Neg(i);
        -- second synchronisation usefull for edge detection        
        S_Intr_In_Sync1(i) <= S_Intr_In_Sync0(i);
        -- third synchronisation usefull for edge detection        
        S_Intr_In_Sync2(i) <= S_Intr_In_Sync1(i);
        if S_Intr_Enable(i) = '1' then
          -- specific interrupt is enabled
          if S_Intr_Edge_Trig(i) = '1' then
            -- specific interrupt is edge triggered
            if S_Intr_In_Sync1(i) = '1' and S_Intr_In_Sync2(i) = '0' then
              -- specific edge detected, only one clock pulse active
              if S_Intr_Mask(i) = '0' then
                -- specific interrupt not masked
                S_Intr_Pending(i) <= '1';             -- so, store the specific edge in the interrupt pending register

              elsif S_Intr_Active(i) = '1' then
                -- the specific interrupt is active
                S_Intr_Pending(i) <= '0';             -- so, clear the specific interrupt pending bit
              else
              -- the specific interrupt is not active
                S_Intr_Active(i) <= '1';              -- so, set the specific interrupt active bit
              end if;
            elsif S_Wr_Intr_Active = "01" and S_Data_from_SCUB_LA(i) = '1' then
              S_Intr_Active(i) <= '0';
            elsif S_Wr_Intr_Pending = "01" and S_Data_from_SCUB_LA(i) = '1' then
              S_Intr_Pending(i) <= '0';
            elsif S_Intr_Pending(i) = '1' and S_Intr_Active(i) = '0' and S_Intr_Mask(i) = '0' then
              S_Intr_Pending(i) <= '0';
              S_Intr_Active(i) <= '1';
            end if;
          else
            -- specific interrupt is level triggered
            S_Intr_Pending(i) <= S_Intr_In_Sync2(i);  -- follows synchronized Intr_IN(i) level
            if S_Intr_Pending(i) = '1' and S_Intr_Mask(i) = '0' then
              -- specific interrupt active and not masked
              S_Intr_Active(i) <= '1';                -- so, set the specific interrupt active bit
            else
              S_Intr_Active(i) <= '0';                -- so, clear the specific interrupt active bit
            end if;
          end if;
        else
          -- specific interrupt is disabled, so clear specific ...Pending- and Active-Bit 
          S_Intr_Pending(i) <= '0';
          S_Intr_Active(i) <= '0';
        end if;
      end LOOP;
      if unsigned(S_Intr_Active) /= 0 OR S_Powerup_Done = '1' then
        S_SRQ <= '1';
      else
        S_SRQ <= '0';
      end if;
    end if;
  end process P_Intr;


P_Tri_Buff: process (S_nReset, SCUB_RDnWR, nSCUB_DS, nSCUB_Slave_Sel, S_Read_Out)
  begin
    if S_nReset = '0' then
      SCUB_Data <= (others => 'Z');
      nSel_Ext_Data_Drv <= '1';
    elsif nSCUB_Slave_Sel = '0' then  -- setzt voraus, dass der SCU_BusMaster während eines Timing-Cycles
                                      -- die nSCUB_Slave_Sel bedient.
      nSel_Ext_Data_Drv <= '0';
      if SCUB_RDnWR = '1' and nSCUB_DS = '0' then
        SCUB_Data <= S_Read_Out;
      else
        SCUB_Data <= (others => 'Z');
      end if;
    else
      nSel_Ext_Data_Drv <= '1';
      SCUB_Data <= (others => 'Z');
    end if;
  end process P_Tri_Buff;
  

P_no_fin_sig_overap: process (clk)
  begin
    if rising_edge(clk) then
      if (S_Adr_Val = '1' and SCUB_RDnWR = '1' and S_nSync_Board_Sel = "01" and S_SCUB_Dtack = '1') then
        Ext_Rd_fin <= '1';
      else
        Ext_Rd_fin <= '0';
      end if;
      if (S_DS_Val = '1' and SCUB_RDnWR = '0' and S_nSync_Board_Sel = "01" and S_SCUB_Dtack = '1') then
        Ext_Wr_fin <= '1';
      else
        Ext_Wr_fin <= '0';
      end if;
    end if;
  end process P_no_fin_sig_overap;
  

Ext_Rd_fin_ovl  <= '1' when (S_Adr_Val = '1' and SCUB_RDnWR = '1' and S_nSync_Board_Sel = "01" and S_SCUB_Dtack = '1') 
                       else '0';
Ext_Wr_fin_ovl  <= '1' when (S_DS_Val = '1' and SCUB_RDnWR = '0' and S_nSync_Board_Sel = "01" and S_SCUB_Dtack = '1')
                       else '0';


P_Standard_Reg: process (clk, S_nReset)
  begin
    if S_nReset = '0' then
      S_Echo_Reg <= (others => '0');
      S_Intr_Mask <= (others => '0');
      S_Intr_Level_Neg <= Intr_Level_Neg(Intr_In'range);
      S_Intr_Edge_Trig <= Intr_Edge_Trig(Intr_In'range);
      S_Intr_Enable <= Intr_Enable;
      S_Adr_Val <= '0';
      S_DS_Val <= '0';
      S_SCUB_Dtack <= '0';
      S_Standard_Reg_Acc <= '0';
      S_Wr_Intr_Pending <= "00";
      S_Wr_Intr_Active <= "00";
      S_Read_Out <= (others => 'Z');    -- Vers_2 Revi_0: Vorschlag S. Schäfer,
                                        -- vermeidet unötige Datenübergänge auf dem SCU-Datenbus

    elsif rising_edge(clk) then
      S_Adr_Val <= '0';
      S_DS_Val <= '0';
      S_SCUB_Dtack <= '0';
      S_Wr_Intr_Pending <= "00";
      S_Wr_Intr_Active <= "00";
      S_Standard_Reg_Acc <= '0';
      S_Dtack_to_SCUB_Dly <= (others => '0');
      S_Read_Out <= (others => 'Z');    -- Vers_2 Revi_0: Vorschlag S. Schäfer, vermeidet
                                        --unötige Datenübergänge auf dem SCU-Datenbus 
      if S_nSync_Board_Sel = "00" and S_nSync_Timing_Cyc = "11" then
        case unsigned(S_ADR_from_SCUB_LA) IS
          when C_Slave_ID_Adr =>
            S_Standard_Reg_Acc <= '1';
            if SCUB_RDnWR = '1' then
              S_Read_Out <= std_logic_vector(to_unsigned(Slave_ID, S_Read_Out'length));
              S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
            end if;
          when C_FW_Version_Adr =>
            S_Standard_Reg_Acc <= '1';
            if SCUB_RDnWR = '1' then
              S_Read_Out <= std_logic_vector(to_unsigned(Firmware_Version, S_Read_Out'length));
              S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
            end if;
          when C_FW_Release_Adr =>
            S_Standard_Reg_Acc <= '1';
            if SCUB_RDnWR = '1' then
              S_Read_Out <= std_logic_vector(to_unsigned(Firmware_Release, S_Read_Out'length));
              S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
            end if;
          when C_CID_System_Adr =>
            S_Standard_Reg_Acc <= '1';
            if SCUB_RDnWR = '1' then
              S_Read_Out <= std_logic_vector(to_unsigned(CID_System, S_Read_Out'length));
              S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
            end if;
          when C_CID_Group_Adr =>
            S_Standard_Reg_Acc <= '1';
            if SCUB_RDnWR = '1' then
              S_Read_Out <= std_logic_vector(to_unsigned(CID_Group, S_Read_Out'length));
              S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
            end if;
          when C_Echo_Reg_Adr =>
            S_Standard_Reg_Acc <= '1';
            if SCUB_RDnWR = '1' then
              S_Read_Out <= S_Echo_Reg;
              S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
            elsif S_nSync_DS = "00" then
              S_Echo_Reg <= S_Data_from_SCUB_LA;
              S_SCUB_Dtack <= '1';
            else
              S_SCUB_Dtack <= '0';
            end if;
          when C_Status_Reg_Adr =>
            S_Standard_Reg_Acc <= '1';
            if SCUB_RDnWR = '1' then
              S_Read_Out <= (X"000" & '0' & '0' & User_Ready & S_Powerup_Done); -- User_Ready must be synchron with clk
              S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
            end if;
          when C_Intr_In_Adr =>
            S_Standard_Reg_Acc <= '1';
            if SCUB_RDnWR = '1' then
              S_Read_Out <= (S_Intr_In_Sync2 & '0');
              S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
            end if;
          when C_Intr_Ena_Adr =>
            S_Standard_Reg_Acc <= '1';
            if SCUB_RDnWR = '1' then
              S_Read_Out <= (S_Intr_Enable & '1');
              S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
            elsif S_nSync_DS = "00" then
              S_Intr_Enable <= S_Data_from_SCUB_LA(Intr_In'range);
              S_SCUB_Dtack <= '1';
            end if;
          when C_Intr_Pending_Adr =>
            S_Standard_Reg_Acc <= '1';
            if SCUB_RDnWR = '1' then
              S_Read_Out <= (S_Intr_Pending & '0'); -- register latchen ???
              S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
            elsif S_nSync_DS = "00" then
              S_Wr_Intr_Pending <= S_Wr_Intr_Pending(0) & '1';
              S_SCUB_Dtack <= '1';  -- ???
            end if;
          when C_Intr_Mask_Adr =>
            S_Standard_Reg_Acc <= '1';
            if SCUB_RDnWR = '1' then
              S_Read_Out <= (S_Intr_Mask & '0');
              S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
            elsif S_nSync_DS = "00" then
              S_Intr_Mask <= S_Data_from_SCUB_LA(Intr_In'range);
              S_SCUB_Dtack <= '1';
            end if;
          when C_Intr_Active_Adr =>
            S_Standard_Reg_Acc <= '1';
            if SCUB_RDnWR = '1' then
              S_Read_Out <= S_Intr_Active & S_Powerup_Done; -- register latchen ???
              S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
            elsif S_nSync_DS = "00" then
              S_Wr_Intr_Active <= S_Wr_Intr_Active(0) & '1';
              S_SCUB_Dtack <= '1';  -- ??
            end if;
          when C_Intr_Level_Adr =>
            S_Standard_Reg_Acc <= '1';
            if SCUB_RDnWR = '1' then
              S_Read_Out <= (S_Intr_Level_Neg & '0');
              S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
            elsif S_nSync_DS = "00" then
              S_Intr_Level_Neg <= S_Data_from_SCUB_LA(Intr_In'range);
              S_SCUB_Dtack <= '1';
            end if;
          when C_Intr_Edge_Adr =>
            S_Standard_Reg_Acc <= '1';
            if SCUB_RDnWR = '1' then
              S_Read_Out <= (S_Intr_Edge_Trig & '0');
              S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
            elsif S_nSync_DS = "00" then
              S_Intr_Edge_Trig <= S_Data_from_SCUB_LA(Intr_In'range);
              S_SCUB_Dtack <= '1';
            end if;
          when C_Vers_Revi_of_this_Macro =>
            S_Standard_Reg_Acc <= '1';
            if SCUB_RDnWR = '1' then
              S_Read_Out <= std_logic_vector(C_Version) & std_logic_vector(C_Revision);
              S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
            end if;
          when   C_Free_Intern_A_12
             | C_Free_Intern_A_13
             | C_Free_Intern_A_14
             | C_Free_Intern_A_15
             | C_Free_Intern_A_16
             | C_Free_Intern_A_17
             | C_Free_Intern_A_18
             | C_Free_Intern_A_19
             | C_Free_Intern_A_1A
             | C_Free_Intern_A_1B
             | C_Free_Intern_A_1C
             | C_Free_Intern_A_1D
             | C_Free_Intern_A_1E
             | C_Free_Intern_A_1F  =>
            S_Standard_Reg_Acc <= '1';
          when others =>                -- der Zugriff soll ausserhalb dieses Makros erfolgen (externe User-Register)
            S_Adr_Val <= '1';

            S_Dtack_to_SCUB_Dly <= S_Dtack_to_SCUB_Dly(S_Dtack_to_SCUB_Dly'high-1 downto 0) & Dtack_to_SCUB;  -- Vers_2 Revi_1

            if SCUB_RDnWR = '1' and Dtack_to_SCUB = '1' then  -- Vers_2 Revi_1: Vorschlag S. Schäfer, nur wenn Resource  --
              S_Read_Out <= Data_to_SCUB;                     -- existiert,wird das Lesedatum auf den SCU_Bus gelegt.   --
              S_SCUB_Dtack <= S_Dtack_to_SCUB_Dly(S_Dtack_to_SCUB_Dly'high);    -- Vers_2 Revi_1
            elsif SCUB_RDnWR = '0' and S_nSync_DS = "00" then                   -- Vers_2 Revi_1
              S_DS_Val <= '1';
              S_SCUB_Dtack <= S_Dtack_to_SCUB_Dly(S_Dtack_to_SCUB_Dly'high);    -- Vers_2 Revi_1
            else
              S_SCUB_Dtack <= '0';                                              -- Vers_3_Revi_2, eigentlich sollte der Default (nach rising_edge)
              S_DS_Val <= '0';                                                  --                für das Rücksetzen sorgen. 
            end if;
        end case;
      end if;

    end if;
  end process P_Standard_Reg;
  

nSCUB_Dtack_Opdrn <= '0' when (S_SCUB_Dtack = '1' and nSCUB_Slave_Sel = '0') else 'Z';
SCUB_Dtack      <= '1' when (S_SCUB_Dtack = '1' and nSCUB_Slave_Sel = '0') else '0';

ADR_from_SCUB_LA <= S_ADR_from_SCUB_LA;

Data_from_SCUB_LA <= S_Data_from_SCUB_LA;

Timing_Pattern_LA <= S_Timing_Pattern_LA;
Timing_Pattern_RCV <= S_Timing_Pat_RCV_Dly;

nSCUB_SRQ_Opdrn <= '0' when (S_SRQ = '1') else 'Z';
SCUB_SRQ    <= '1' when (S_SRQ = '1') else '0';

Ext_Adr_Val <= S_Adr_Val;

Ext_Rd_active <= '1' when (S_Adr_Val = '1' and S_nSync_Board_Sel = "00" and SCUB_RDnWR = '1') else '0';

Ext_Wr_active <= '1' when (S_DS_Val = '1' and S_nSync_Board_Sel = "00"  and SCUB_RDnWR = '0') else '0';

nPowerup_Res <= not S_Powerup_Res;

Ext_Data_Drv_Rd <= SCUB_RDnWR;

Standard_Reg_Acc <= S_Standard_Reg_Acc;

Deb_SCUB_Reset_out <= S_Deb_Reset;  -- Vers_2_Revi_4: das Reset-Signal des SCU-Busses 'nSCUB_Reset_In' wird entprellt
                                    -- und für andere Macros zur Verfügung gestellt

end Arch_SCU_Bus_Slave;
