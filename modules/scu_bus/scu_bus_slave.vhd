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

----------------------------------------------------------------------------------------------------------------------
--  Vers_3_Revi_4: erstellt am 22.05.2013, Autor: W.Panschow                                                        --
--    Die Signale "Ext_Wr_fin", "Ext_Rd_Fin", "Ext_Wr_fin_ovl" and "Ext_Rd_fin_ovl" wurden ueberarbeitet.           --
----------------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------
--  Vers_4_Revi_0: erstellt am 05.12.2013, Autor: W.Panschow                                                        --
--  A) Die Interrupt-Funktion der Intr_In-Eingaenge wurde ueberarbeitet.                                            --
--    Die Aenderungen betreffen:                                                                                    --
--    1) Die Pegel bzw. Flanken zum generieren eines Interrupts. Sie koennen nicht mehr durch Generics oder         --
--      Register vorgegeben werden. Nur ein positiver Pegel oder eine positive Flanke loest einen Interrupt aus,    --
--      vorausgesetzt der Intr_In-Eingang ist enabled. Deshalb sind die Generics "Intr_Level_Neg" und               --
--      "Intr_Edge_Trig" entfernt worden. Auch die Moeglichkeit im laufenden Betrieb die Vorgaben zu aendern ist    --
--      entfernt worden. Deshalb sind die Adressen "C_Intr_Level_Adr = X"0025" und "C_Intr_Edge_Adr = X0026" nicht  --
--      mehr ansprechbar.                                                                                           --
--    2) Es kann nicht mehr zwischen Pegel- und Flanken-Triggerung gewaehlt werden. Der Interrupt muss fuer         --
--      mindestens eine Taktperiode aktiv sein, dann bleibt er im "S_Intr_Active"-Register gespeichert, bis er dort --
--      geloescht wird. Das Generic "Intr_Level_Neg" und das Register "S_Intr_Edge_Trig" wurde entfernt.            --
--    3) Das interrupt mask register wurde entfernt, deshalb ist die Adresse "C_Intr_Mask_Adr = X"0023" nicht mehr  --
--      erlaubt. Die Interrupts werden nur noch ueber das Generic "Intr_Enable" oder das Register "S_Intr_Enable"   --
--      armiert. Ist ein Interrupt schon waerend des Enable-Vorgangs aktiv,wird eine Flanke erzeugt, d.h. der       --
--      Interrupt wird im "S_Intr_Active"-Register gesetzt.                                                         --
--    4) Das interrupt pending register wurde entfernt, deshalb ist Adresse "C_Intr_Pending_Adr = X"0022" nicht     --
--      mehr erlaubt.                                                                                               --
--  B) Das Signal "S_Powerup_Done" ist auf einen Entiy-Ausgang gelegt worden. Mit diesem Signal koennen die         --
--    Firmware-Entwickler von Slave-Karten signalisieren, dass ein Powerup durchgefuehrt wurde. Dieses Signal ist   --
--    nur so lange aktiv, bis es der SCU-Bus-Master quittiert hat. Da dies im Allgemeinen schnell passieren wird,   --
--    sollte das Signal entsprechend gestrecht werden, wenn es durch eine LED treiben soll.                         --
----------------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------
--  Vers_5_Revi_0: erstellt am 17.01.2014, Autor: W.Panschow                                                        --
--  A) Falls Slave-Karten die Moeglichkeit bieten, mit Erweiterungs-Karten bestueckt zu werden, ist es sinnvoll     --
--    den bestueckten Erweiterungs-Karte-Typ, direkt ueber den Uebergabe-Stecker kodiert zu kennzeichnen. Bei der   --
--    DIOB-Karte sind hierfuer z.B. 8 Bit vorgesehen. Firmware-Entwickler dieser Karte muessen fuer den jeweils     --
--    bestueckten Erweiterungs-Karten-Typ entsprechende Funktionen auf Slave-Karte bereitstellen. Bei der           --
--    Implementierung dieser Funktionen kann der Entwickler bei eindeutiger Erweiterungs-Karten-Kennung eine        --
--    Tabelle pflegen, die ueber diese Kennung die entsprechende CID-System- und CID-Group-Kennung an die neuen     --
--    SCU-Bus-Slave-Eingangsports anlegt.                                                                           --
--    1) Es sind zwei neue Eingangs-Ports definiert worden:                                                         --
--      "extension_cid_system" und "extension_cid_group". Beide sind 16 Bit breit. Wenn die Ports nicht beschaltet  --
--       sind, werden sie mit "0000"hex vorbelegt.                                                                  --
--    2) Es wurde die Auslese des "extension_cid_system"-Ports eingebaut.                                           --
--      Die Adresse lautet "c_extension_cid_system_adr" und hat die Adresse X"0007" (hex).                          --
--    3) Es wurde die Auslese des "extension_cid_group"-Ports eingebaut.                                            --
--      Die Adresse lautet "c_extension_cid_group_adr" und hat die Adresse X"0008" (hex).                           --
--  B) Fuer betimmte Funktionen ist es interessant zu wissen mit welcher Frequenz, dieses Makro, und eventuell      --
--    nachgelagerte Anwenderfunktionen getaktet werden. Deshalb kann jetzt der Generic "CLK_in_Hz" ausgelesen       --
--    werden. Um den Wert auf 16 Bit Breite zu beschraenken wird der Wert "Clk_in_Hz" in "ClK_in_10kHz" um-         --
--    rechnet.                                                                                                      --
--    1) Die Adresse zum Auslesen des "Clk_in_10kHz"-Werts lautet "c_clk_in_10khz_adr" und hat die                  --
--     Adresse x"0009" (hex).                                                                                       --
----------------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------
--  Vers_5_Revi_1: erstellt am 07.02.2014, Autor: W.Panschow                                                        --
--  A) Die Adressdekodierung der Makro-Internen Adressen wurde ueberarbeitet. Die Range-Definitionen z.B:           --
--    when C_Free_Intern_A_12 to C_Free_Intern_A_1F, wurden in ein Oder-Konstrukt geaendert. Da die Adress-         --
--    dekodierung nur fehlerfrei funktioniert, wenn alle Adressen aufsteigend sortiert waren. Bei der Oder-         --
--    Implementierung musste darauf nicht geachtet werden.                                                          --
----------------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------
--  Vers_5_Revi_2: erstellt am 13.02.2014, Autor: W.Panschow                                                        --
--  A) Die beiden Ports CID-System und CID-Group_Kennung wurden von Type Std_logic_vector(15 downto 0) auf          --
--    den Type integer range 0 to 16#FFFF# umgestellt. Die Vorgabe kann dann genauso direkt (ohne Typkonversion)    --
--    vorgnommen werden, wie bei den Generics CID_System und CID_Group.                                             --
----------------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------
--  Vers_5_Revi_3: erstellt am 29.10.2015, Autor: K.Kaiser                                                          --
--  A) Die CID_Group Kennung (Type integer range 0 to 16#FFFF#) wurde aus dem generic map in den port map verschoben--
--     Hiermit ist es moeglich, für SIO, ADDAC und DIOB entsprechend dem Hex Dial bei Auslieferung eine Variante    --
--     einzustellen ( z.B. SIO1, SIO2, SIO3). Damit kann die SW nicht nur über 1Wire ID die Variante bestimmen      --
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
    -- CID_Group: integer range 0 to 16#FFFF# := 0;

    -- the bit positions are corresponding to Intr_In. A '1' enable Intr_In(n), '0' disable Intr_In(n)
    -- The least significant bit don't care, because it represent the powerup interrupt. This interrupt is always enabled.
    Intr_Enable:      std_logic_vector(15 DOWNTO 0) := B"0000_0000_0000_0000";
                                                                              
    -- change only here! increment by major changes of this macro
    This_macro_vers_dont_change_from_outside: integer range 0 to 16#FF# := 5;
    
    -- change only here! increment by minor changes of this macro
    This_macro_revi_dont_change_from_outside: integer range 0 to 16#FF# := 2
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
    Intr_In:            in    std_logic_vector(15 DOWNTO 1);
    
    -- '1' => the user function(s), device, is ready to work with the control system
    User_Ready:         in    std_logic;
    
    -- CID Group has default as 0x0000, or mapped to actual parameter as defined by hex dial in top level.
    CID_Group:            in    integer range 0 to 16#FFFF# := 0;
    
    -- if an extension card is connected to the slave card, than you can map cid_system of this extension
    -- (vorausgesetzt der Typ der Extension-Card ist über diese Verbindung eindeutig bestimmbar).
    extension_cid_system: in  integer range 0 to 16#FFFF# := 0;
    
    -- if an extension card is connected to the slave card, than you can map cid_group of this extension
    -- (vorausgesetzt der Typ der Extension-Card ist über diese Verbindung eindeutig bestimmbar).
    extension_cid_group:  in    integer range 0 to 16#FFFF# := 0;
  
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
    nPowerup_Res:       out   std_logic;    -- '0' => the FPGA make a powerup
    Powerup_Done:       out   std_logic     -- this memory is set to one if an Powerup is done. Only the SCUB-Master can clear this bit.
    );
  
  constant  Clk_in_ps: integer  := 1000000000 / (Clk_in_Hz / 1000);
  constant  Clk_in_ns: integer  := 1000000000 / Clk_in_Hz;
  constant  clk_in_10kHz: integer := Clk_in_Hz / 10_000;
  
  constant  C_Version:  unsigned(7 downto 0) :=  to_unsigned(This_macro_vers_dont_change_from_outside, 8);
  constant  C_Revision: unsigned(7 downto 0) :=  to_unsigned(This_macro_revi_dont_change_from_outside, 8);
  
  constant  C_Slave_ID_Adr:     integer := 16#0001#;   -- address of slave ident code (rd)
  constant  C_FW_Version_Adr:   integer := 16#0002#;   -- address of firmware version (rd)
  constant  C_FW_Release_Adr:   integer := 16#0003#;   -- address of firmware release (rd)
  constant  C_CID_System_Adr:   integer := 16#0004#;   -- addresses CID system number of slave card, source is generic CID_System (rd)
  constant  C_CID_Group_Adr:    integer := 16#0005#;   -- addresses CID group number of slave card, source is generic CID_Group (rd)

  constant  C_Vers_Revi_of_this_Macro:  integer := 16#0006#;  -- address of version and revision register of this macro (rd)
  constant  c_extension_cid_system_adr: integer := 16#0007#;  -- adresses cid system number of a connected extension card,
                                                              -- source is the port "extension_cid_system" of this macro
  constant  c_extension_cid_group_adr:  integer := 16#0008#;  -- adresses cid system group of a connected extension card,
                                                              -- source is the port "extension_cid_group" of this macro
  constant  c_clk_in_10khz_adr: integer := 16#0009#;   -- adresse unter der die Frequenz des Generics "CLK_in_Hz" in 10khz-Aufloesung
                                                       -- umgerechnet ausgelesen werden kann.
  
  constant  C_Free_Intern_A_0A: integer := 16#000A#;   -- reserved internal address 0Ahex
  constant  C_Free_Intern_A_0B: integer := 16#000B#;   -- reserved internal address 0Bhex
  constant  C_Free_Intern_A_0C: integer := 16#000C#;   -- reserved internal address 0Chex
  constant  C_Free_Intern_A_0D: integer := 16#000D#;   -- reserved internal address 0Dhex
  constant  C_Free_Intern_A_0E: integer := 16#000E#;   -- reserved internal address 0Ehex
  constant  C_Free_Intern_A_0F: integer := 16#000F#;   -- reserved internal address 0Fhex
  
  constant  C_Echo_Reg_Adr:     integer := 16#0010#;   -- address of echo register (rd/wr)
  constant  C_Status_Reg_Adr:   integer := 16#0011#;   -- address of status register (rd)
  

  constant  C_Free_Intern_A_12: integer := 16#0012#;   -- reserved internal address 12hex
  constant  C_Free_Intern_A_13: integer := 16#0013#;   -- reserved internal address 13hex
  constant  C_Free_Intern_A_14: integer := 16#0014#;   -- reserved internal address 14hex
  constant  C_Free_Intern_A_15: integer := 16#0015#;   -- reserved internal address 15hex
  constant  C_Free_Intern_A_16: integer := 16#0016#;   -- reserved internal address 16hex
  constant  C_Free_Intern_A_17: integer := 16#0017#;   -- reserved internal address 17hex
  constant  C_Free_Intern_A_18: integer := 16#0018#;   -- reserved internal address 18hex
  constant  C_Free_Intern_A_19: integer := 16#0019#;   -- reserved internal address 19hex
  constant  C_Free_Intern_A_1A: integer := 16#001A#;   -- reserved internal address 1Ahex
  constant  C_Free_Intern_A_1B: integer := 16#001B#;   -- reserved internal address 1Bhex
  constant  C_Free_Intern_A_1C: integer := 16#001C#;   -- reserved internal address 1Chex
  constant  C_Free_Intern_A_1D: integer := 16#001D#;   -- reserved internal address 1Dhex
  constant  C_Free_Intern_A_1E: integer := 16#001E#;   -- reserved internal address 1Ehex
  constant  C_Free_Intern_A_1F: integer := 16#001F#;   -- reserved internal address 1Fhex

  constant  C_Intr_In_Adr:      integer := 16#0020#;   -- address of interrupt In register (rd)
  constant  C_Intr_Ena_Adr:     integer := 16#0021#;   -- address of interrupt enable register (rd/wr)
  constant  C_Free_Intern_A_22: integer := 16#0022#;   -- reserved internal address 22hex
  constant  C_Free_Intern_A_23: integer := 16#0023#;   -- reserved internal address 23hex
  constant  C_Intr_Active_Adr:  integer := 16#0024#;   -- address of interrupt active register (rd)

  constant  C_Free_Intern_A_25: integer := 16#0025#;   -- reserved internal address 25hex
  constant  C_Free_Intern_A_26: integer := 16#0026#;   -- reserved internal address 26hex
  constant  C_Free_Intern_A_27: integer := 16#0027#;   -- reserved internal address 27hex
  constant  C_Free_Intern_A_28: integer := 16#0028#;   -- reserved internal address 28hex
  constant  C_Free_Intern_A_29: integer := 16#0029#;   -- reserved internal address 29hex
  constant  C_Free_Intern_A_2A: integer := 16#002A#;   -- reserved internal address 2Ahex
  constant  C_Free_Intern_A_2B: integer := 16#002B#;   -- reserved internal address 2Bhex
  constant  C_Free_Intern_A_2C: integer := 16#002C#;   -- reserved internal address 2Chex
  constant  C_Free_Intern_A_2D: integer := 16#002D#;   -- reserved internal address 2Dhex
  constant  C_Free_Intern_A_2E: integer := 16#002E#;   -- reserved internal address 2Ehex
  constant  C_Free_Intern_A_2F: integer := 16#002F#;   -- reserved internal address 2Fhex

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
  signal    intr_enable_previous: std_logic_vector(Intr_In'range);
  signal    intr_reactivate:      std_logic_vector(Intr_In'range);
  signal    S_Intr_Active:        std_logic_vector(Intr_In'range);
  signal    S_Wr_Intr_Active:     std_logic_vector(1 downto 0);
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
  
  signal    is_ext_wr_cycle:      std_logic;
  signal    is_ext_rd_cycle:      std_logic;
  signal    dt_caught:            std_logic;
  
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
      S_Intr_Active   <= (others => '0');
      intr_reactivate <= (others => '0');
      intr_enable_previous <= (others => '0');

    elsif rising_edge(clk) then

      S_Intr_In_Sync0 <= Intr_In;             -- 'Intr_In' first synchronisation
      S_Intr_In_Sync1 <= S_Intr_In_Sync0;     -- second synchronisation used for edge detection 
      S_Intr_In_Sync2 <= S_Intr_In_Sync1;     -- third synchronisation used for edge detection
      intr_enable_previous <= S_Intr_Enable;  -- used to detect transition frorm disable to enable

      FOR i in Intr_In'low TO Intr_In'high LOOP

        if S_Intr_Enable(i) = '0' then
        -- specific interrupt is disabled, so clear the specific interrupt active bit
          S_Intr_Active(i) <= '0';                

        elsif S_Intr_Enable(i) = '1' and intr_enable_previous(i) = '0' and S_Intr_In_Sync2(i) = '1' then
        -- specific interrupt changed from disabled to enabled, if the specific interrupt is active, set the the specific interrupt active bit without edge detect.
          S_Intr_Active(i)  <= '1';
        
        elsif S_Intr_Enable(i) = '1' and S_Intr_In_Sync1(i) = '1' and S_Intr_In_Sync2(i) = '0' then
        -- specific interrupt is enabled and an edge is detected.
          if S_Intr_Active(i) = '1' then
            S_Intr_Active(i)  <= '0';     -- it is allready active, so clear it for one clk period. So we generate a new edge on S_Intr_Active(i). 
            intr_reactivate(i) <= '1';    -- set an memo to ractivate the S_Intr_Active(i) one clk later
          else
            S_Intr_Active(i)  <= '1';
          end if;
 
          if S_Wr_Intr_Active = "01" and S_Data_from_SCUB_LA(i) = '1' then
          -- edge detection  and clear action occurs at same time
            S_Intr_Active(i) <= '0';      -- so, first clear the S_Intr_Active bit
            intr_reactivate(i) <= '1';    -- and set an memo to ractivate the S_Intr_Active(i) one clk later
          end if;
          
        elsif S_Wr_Intr_Active = "01" and S_Data_from_SCUB_LA(i) = '1' then
        -- clear interrupt active bit, even it's not enabled
          S_Intr_Active(i) <= '0';
        
        elsif intr_reactivate(i) = '1' then
        -- ractivate the S_Intr_Active bit
          S_Intr_Active(i) <= '1';
          intr_reactivate(i) <= '0';
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
  

P_no_fin_sig_overap: process (clk, S_nReset)
  begin
  
    if S_nReset = '0' then
      is_ext_rd_cycle <= '0';
      is_ext_wr_cycle <= '0';
      dt_caught <= '0';
      Ext_Rd_fin <= '0';
      Ext_Wr_fin <= '0';
      
    elsif rising_edge(clk) then
    
      if S_Adr_Val = '1' and S_nSync_Board_Sel = "00" and SCUB_RDnWR = '1' then
        is_ext_rd_cycle <= '1';     -- store that the next cycle is external write access
        if S_SCUB_Dtack = '1' then
          dt_caught <= '1';
        end if;
      elsif  S_Adr_Val = '1' and S_nSync_Board_Sel = "00" and SCUB_RDnWR = '0' then
        is_ext_wr_cycle <= '1';     -- store that the next cycle is external read access
        if S_SCUB_Dtack = '1' then
          dt_caught <= '1';
        end if;
      end if;
      
      if S_nSync_Board_Sel = "01" and is_ext_rd_cycle = '1' then
        -- the external read becomes inactive
        is_ext_rd_cycle <= '0';     -- so clear the cycle type storage
        if dt_caught = '1' then
        -- the external read becomes inactive with data acknowlege
          Ext_Rd_fin <= '1';        -- one clock cyce active
        end if;
      elsif S_nSync_Board_Sel = "11" then
        -- the external read is finished
        dt_caught <= '0';           -- so clear dt_caught-latch
        Ext_Rd_fin <= '0';          -- so clear Ext_Rd_fin-latch
      end if;
      
      if S_nSync_Board_Sel = "01" and is_ext_wr_cycle = '1' then
        -- the external write becomes inactive
        is_ext_wr_cycle <= '0';     -- so clear the cycle type storage
        if dt_caught = '1' then
        -- the external write becomes inactive with data acknowlege
          Ext_Wr_fin <= '1';        -- one clock cyce active
        end if;
      elsif S_nSync_Board_Sel = "11" then
        -- the external write is finished
        dt_caught <= '0';           -- so clear dt_caught-latch
        Ext_Wr_fin <= '0';          -- so clear Ext_Wr_fin-latch
      end if;
      
    end if;
  end process P_no_fin_sig_overap;
  

Ext_Rd_fin_ovl  <= '1' when (is_ext_rd_cycle = '1' and S_nSync_Board_Sel = "01" and dt_caught = '1') 
                       else '0';
Ext_Wr_fin_ovl  <= '1' when (is_ext_wr_cycle = '1' and S_nSync_Board_Sel = "01" and dt_caught = '1')
                       else '0';


P_Standard_Reg: process (clk, S_nReset)
  begin
    if S_nReset = '0' then
      S_Echo_Reg <= (others => '0');
      S_Intr_Enable <= Intr_Enable(Intr_In'range);
      S_Adr_Val <= '0';
      S_DS_Val <= '0';
      S_SCUB_Dtack <= '0';
      S_Standard_Reg_Acc <= '0';
      S_Wr_Intr_Active <= "00";
      S_Read_Out <= (others => 'Z');    -- Vers_2 Revi_0: Vorschlag S. Schäfer,
                                        -- vermeidet unötige Datenübergänge auf dem SCU-Datenbus

    elsif rising_edge(clk) then
      S_Adr_Val <= '0';
      S_DS_Val <= '0';
      S_SCUB_Dtack <= '0';
      S_Wr_Intr_Active <= "00";
      S_Standard_Reg_Acc <= '0';
      S_Dtack_to_SCUB_Dly <= (others => '0');
      S_Read_Out <= (others => 'Z');    -- Vers_2 Revi_0: Vorschlag S. Schäfer, vermeidet
                                        --unötige Datenübergänge auf dem SCU-Datenbus 
      if S_nSync_Board_Sel = "00" and S_nSync_Timing_Cyc = "11" then
        case to_integer(unsigned(S_ADR_from_SCUB_LA)) IS
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
          when C_Vers_Revi_of_this_Macro =>
            S_Standard_Reg_Acc <= '1';
            if SCUB_RDnWR = '1' then
              S_Read_Out <= std_logic_vector(C_Version) & std_logic_vector(C_Revision);
              S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
            end if;
          when c_extension_cid_system_adr =>
            S_Standard_Reg_Acc <= '1';
            if SCUB_RDnWR = '1' then
              S_Read_Out <= std_logic_vector(to_unsigned(extension_cid_system, 16));
              S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
            end if;
          when c_extension_cid_group_adr =>
            S_Standard_Reg_Acc <= '1';
            if SCUB_RDnWR = '1' then
              S_Read_Out <= std_logic_vector(to_unsigned(extension_cid_group, 16));
              S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
            end if;
          when c_clk_in_10khz_adr =>
            S_Standard_Reg_Acc <= '1';
            if SCUB_RDnWR = '1' then
              S_Read_Out <= std_logic_vector(to_unsigned(clk_in_Hz / 10_000, S_Read_Out'length));
              S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
            end if;
          when C_Free_Intern_A_0A
             | C_Free_Intern_A_0B
             | C_Free_Intern_A_0C
             | C_Free_Intern_A_0D
             | C_Free_Intern_A_0E
             | C_Free_Intern_A_0F => S_Standard_Reg_Acc <= '1';
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
          when C_Free_Intern_A_12
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
             | C_Free_Intern_A_1F => S_Standard_Reg_Acc <= '1';
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
          when C_Free_Intern_A_22
             | C_Free_Intern_A_23 => S_Standard_Reg_Acc <= '1';
          when C_Intr_Active_Adr =>
            S_Standard_Reg_Acc <= '1';
            if SCUB_RDnWR = '1' then
              S_Read_Out <= S_Intr_Active & S_Powerup_Done; -- register latchen ???
              S_SCUB_Dtack <= NOT (S_nSync_DS(1) OR S_nSync_DS(0));
            elsif S_nSync_DS = "00" then
              S_Wr_Intr_Active <= S_Wr_Intr_Active(0) & '1';
              S_SCUB_Dtack <= '1';  -- ??
            end if;
          when C_Free_Intern_A_25
             | C_Free_Intern_A_26
             | C_Free_Intern_A_27
             | C_Free_Intern_A_28
             | C_Free_Intern_A_29
             | C_Free_Intern_A_2A
             | C_Free_Intern_A_2B
             | C_Free_Intern_A_2C
             | C_Free_Intern_A_2D
             | C_Free_Intern_A_2E
             | C_Free_Intern_A_2F => S_Standard_Reg_Acc <= '1';
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

Ext_Rd_active <= is_ext_rd_cycle;

Ext_Wr_active <= is_ext_wr_cycle and S_DS_Val;

nPowerup_Res <= not S_Powerup_Res;

Ext_Data_Drv_Rd <= SCUB_RDnWR;

Standard_Reg_Acc <= S_Standard_Reg_Acc;

Deb_SCUB_Reset_out <= S_Deb_Reset;  -- Vers_2_Revi_4: das Reset-Signal des SCU-Busses 'nSCUB_Reset_In' wird entprellt
                                    -- und für andere Macros zur Verfügung gestellt
Powerup_Done <= S_Powerup_Done;

end Arch_SCU_Bus_Slave;
