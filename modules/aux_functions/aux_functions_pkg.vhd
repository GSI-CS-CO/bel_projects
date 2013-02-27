library IEEE;
use IEEE.STD_LOGIC_1164.all;

library work;

package aux_functions_pkg is



component div_n
--+-----------------------------------------------------------------------------------------------------------------+
--| "div_n",    Autor: W.Panschow                                                                                   |
--|                                                                                                                 |
--| "div_n" soll nach 'n-1' Takten den Ausgang 'div_o' für eine Taktperiode akiv eins setzen.                       |
--|                                                                                                                 |
--| Die Funktion ist mit einem Abwärtszähler realisiert worden. Dadurch werden zwei Nachteile gegeüber einer Lösung |
--|  mit Aufwärtszähler vermieden.                                                                                  |
--|                                                                                                                 |
--| Die Nachteile des Aufwärtszählers:                                                                              |
--|  1) Zustätzlich zum eigentlichen Zähler braucht es noch einen Komparator der das Erreichen des gewünschten      |
--|     Untersetzungswerts abfragt. Bei größeren Zählerbreiten wird der Komparator mehrere Logoikgatter tief        |
--|     (für 8 Bit werden bei einer Vierer-LUT schon zwei Ebenen gebraucht).                                        |
--|  2) Der Komperator produziert Glitches die mit einem weiteren Register entfernt werden müssen.                  |
--|     Das vom Komparator zu testende Untersetzungverhältnis müsste um eins verringert werden.                     |
--|                                                                                                                 |
--| Der Abwärtszähler muss auch ein Bit breiter definiert werden, als eigentlich zur Darstellung des                |
--|     Untersetzungswerts benötigt wird. Dafür liegt mit dem 'Unterlaufbit' dem höchsten Bit des Zählers ein       |
--|     getaktetes (glitchfreies) Signal vor. Der Zähler braucht keinen Komparator! Das gewünsche                   |
--|     Untersetzungsverhältnis 'n' muss mit -2 korrigiert werden, da der Zähler zusätzlich die Null erreichen muss |
--|     und einen weiteren Takt braucht, um das 'Unterlauf'-Bit zu setzen. Diese Korrekur wird innerhalb des Makros |
--|     "div_n" vorgenommen, d.h. der generic "n" beschreibt das tatsaechliche Untersetznugsverhaeltnis.            |
--+-----------------------------------------------------------------------------------------------------------------+
generic
    (
    n:          integer range 2 to integer'high := 2;
    diag_on:    integer range 0 to 1 := 0
    );
port
    (
    res:    in  std_logic := '0';   -- '1' => set "div_n"-counter asynchron to generic-value "n"-2, so the 
                                    -- countdown is "n"-1 clocks to activate the "div_o"-output for one clock periode. 
    clk:    in  std_logic;          -- clk = clock
    ena:    in  std_logic := '1';   -- can be used for a reduction, signal should be generated from the same 
                                    -- clock domain and should be only one clock period active.
    div_o:  out    std_logic        -- div_o becomes '1' for one clock period, if "div_n" arrive n-1 clocks
                                    -- (if ena is permanent '1').
    );
end component;


component led_n
--+-----------------------------------------------------------------------------------------------------------------+
--| "led_n",    Autor: W.Panschow                                                                                   |
--|                                                                                                                 |
--| "led_n" provides puls streching of the signal "Sig_In" to the active zero outputs "nLED" and "nLED_opdrn"       |
--| "Sig_In"  = '1' resets puls streching counter asynchron. If "Sig_In" change to '0' counter hold the outputs     |
--| "nLED" and "nLED_opdrn" at active zero, while the "stretch_cnt" isn't reached.                                  |
--+-----------------------------------------------------------------------------------------------------------------+ 
generic (
        stretch_cnt:    integer := 3    -- default stretch_cnt is 3
        );
port    (
        ena:        in  std_logic;      -- if you use ena for a reduction, signal should be generated from the same
                                        -- clock domain and should be only one clock period active.
        clk:        in  std_logic;      -- clk = clock.
        Sig_In:     in  std_logic;      -- '1' holds "nLED" and "nLED_opdrn" on active zero. "Sig_in" changeing to '0'
                                        -- "nLED" and "nLED_opdrn" change to inactive State after stretch_cnt clock 
                                        -- periodes.
        nLED:       out std_logic;      -- Push-Pull output, active low, inactive high.
        nLED_opdrn: out std_logic       -- open drain output, active low, inactive tristate.
        );
end component;


component debounce
--+-----------------------------------------------------------------------------------------------------------------+
--| "debounce",    Autor: W.Panschow                                                                                |
--|                                                                                                                 |
--| "debounce" stellt am Ausgang "DB_Out" das entprellte Eingangssignal "DB_In" bereit.                             |
--| Die Entprellung wird mit einem Zaehler realisiert. Eine Aenderung an "DB_In" von '0' auf '1' laesst den Zaehler |
--| hochzaehlen. Aber erst wenn "DB_Cnt" erreicht wurde, wird der Ausgang "DB_Out" auch '1'. Wenn dieser Zustand    |
--| erreicht wurde, bleibt der Zaehler auf diesem Wert stehen. Wechselt der Pegel am Eingang "DB_In" von '1'        |
--| auf '0', beginnt der Zeahler runter zu zaehlen, aber erst mit erreichen der Null wechselt auch der              |
--| Ausgang "DB_Out" auf Null.                                                                                      |
--+-----------------------------------------------------------------------------------------------------------------+
generic
    (
    DB_Cnt:     integer := 3;   -- "DB_Cnt" = debounce count gibt die Anzahl von Taktperioden vor, die das
                                -- Signal "DB_In" mindestens '1' oder '0' sein muss, damit der Ausgang
                                -- "DB_Out" diesem Pegel folgt.     
    DB_Tst_Cnt: integer := 3;   -- "DB_Tst_Cnt" = debounce test count wird verwendet wenn generic "Test" = 1
                                -- ist. "DB_Tst_Cnt" soll im Testbench eine kuerzere Simmulationszeit
                                -- ermoeglichen. Zur Berechnung der Zaehlergroesse wird immer "DB_Cnt" 
                                -- verwendet, damit bei der Schaltungssynthese immer die entsprechenden
                                -- Resourcen benutzt werden. Deshalb darf "DB_Tst_Cnt" nie groesser als
                                -- "DB_Cnt" sein. 
    Test:   integer range 0 TO 1 := 0    -- Im Testbench wird bei "Test" = 1 der kleinere "DB_Tst_Cnt" verwendet.
    );
port
    (
    DB_In:      in  std_logic;  -- Das zu entprellende Signal
    Reset:      in  std_logic;  -- Asynchroner reset. Achtung der sollte nicht Stoerungsbehaftet sein.
    Clk:        in  std_logic;
    DB_Out:     out std_logic   -- Das entprellte Signal von "DB_In".
    );
end component;


component lemo_io
--+-----------------------------------------------------------------------------------------------------------------+
--| "lemo_io",    Autor: W.Panschow                                                                                 |
--|                                                                                                                 |
--| "lemon_io" ist fuer einen Icoupler-Baustein geschrieben, der in seiner Richtung umgeschaltet werden kann.       |
--| Die Schaltung ist zur Zeit im "FG900111_SCU2" und "FG900151_SCU_SIO2" und weiteren Projekten implementiert,     |
--| deshalb macht es Sinn, das Interface als allgemeine Funktion bereitzustellen.                                   |
--+-----------------------------------------------------------------------------------------------------------------+
generic
    (
    stretch_cnt:    integer := 3    -- Anzahl der Taktperioden von led_clk, um die die activity_led_n nach 
                                    -- einer Aktivität (Flanke) an lemo_io gestretcht wird.
    );
port
    (
    reset:                  in    std_logic;
    clk:                    in    std_logic;
    led_clk:                in    std_logic;  
    lemo_io_is_output:      in    std_logic;  -- '0' => lemo_io ist Eingang; '1' => lemo_io ist Ausgang
    stretch_led_off:        in    std_logic;  -- '0' => activity_led_n ist gestretcht; 
                                              -- '1' => activity_led_n nicht gestretcht
    to_lemo:                in    std_logic;  -- wenn lemo_io ein Ausgang ist, wird das Signal to_lemo ausgegeben
    lemo_io:                inout std_logic;  -- hier ist der ICoupler angeschlossen
    lemo_en_in:             out   std_logic;  -- '1' => lemo_io ist Eingang; '0' => lemo_io ist Ausgang
    activity_led_n:         out   std_logic;  -- für Aktivitätsanzeige von lemo_io vorgesehen (push-pull, aktiv low)
    activity_led_n_opdrn:   out   std_logic   -- für Aktivitätsanzeige von lemo_io vorgesehen    (opendrain, aktiv low)
    );
end component lemo_io;

end package aux_functions_pkg;