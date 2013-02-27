--+-----------------------------------------------------------------------------------------------------+
--| SCU_Slave_FG_V1_R1 ist ein linearer Funktionsgenerator. Er ist in seiner Funktionsweise             |
--| weitgehend kompatibel mit:                                                                          |
--|     - der Fg-Generator-Tocherkarte für die Intefacekarte.                                           |
--|     - der FG-Makro-Funktion, die dierekt in das FPGA der Interfacekarte implementiert werden kann.  |
--| Der Funktionsgenerator SCU_Slave_FG_V1_R1 hat ein angepasstes Interface zur SCU-Bus-Slave-Makro-    |
--| Funktion und kann deshalb direkt in das FPGA einer SCU-Bus-Slave-Karte implementiert werden.        |
--| Die hauptsächlichen Änderungen betreffen:                                                           |
--|     - die Umstellung des "langsamen" Funktionscode gesteuerten Interfaces auf die schnelle SCU-Bus- |
--|       Adressierung.                                                                                 |
--|     - Aufwendige Sychronisierung zwischen der schnellen Clock-Domain, der SCU-Bus-Slave-Funktion,   |
--|       und der langsamen Clock-Domain des Funktionsgenerators.                                       |
--+-----------------------------------------------------------------------------------------------------+

--+-----------------------------------------------------------------------------------------------------+
--| Stand:  02.07.2010  Version 1, Revision 1 (V1R1)                                                    |
--| Autor:  W. Panschow / S. Rauch                                                                      |
--| V1R1:   alle wesentlichen Funktionen des Funktionsgenertors sind implementiert.                     |
--|         Nicht implementiert ist die Umschaltung des FG's auf den Betrieb mit einem externen Takt.   |
--|         Die Dokumentation dieses Makros ist noch sehr rudimentär.                                   |
--+-----------------------------------------------------------------------------------------------------+

--+-----------------------------------------------------------------------------------------------------+
--| Stand:  28.07.2010  Version 2, Revision 0 (V2R0)                                                    |
--| Autor:  W. Panschow / S. Rauch                                                                      |
--| V2R0:   Es gibt nur noch eine Clock-Domain "sys_clk".                                               |
--|         Der wesentlich niedrigere Takt "fg_clk" oder "ext_clk" wird mit "sys_clk" abgetastet und    |
--|         eine Flankenerkennung durchgeführt. Aufwendige und Zeitkostende Aussynchronisierung auf die |
--|         "fg_clk" oder "ext_clk" entfallen. Die Umschaltung zwischen "fg_clk" und "ext_clk" verein-  |
--|         facht sich ebenfalls, da der Takt "sys_clk" kontinuierlich mit allen Prozessen verbunden    |
--|         bleibt.                                                                                     |
--+-----------------------------------------------------------------------------------------------------+

--+-----------------------------------------------------------------------------------------------------+
--| Stand:  28.07.2010  Version 2, Revision 1 (V2R1)                                                    |
--| Autor:  W. Panschow / S. Rauch                                                                      |
--| V2R1:   "rd_dtack" und "wr_dtack" waren nur einen sys_clk-Takt lang aktiv. Diese Zeit ist für den   |
--|         SCUB-Master nicht lang genug um die Daten zu übernehmen. Da der SCUB_Slave den Datenbus nur |
--|         solange treibt, wie das Dtack der externen Makrofunktionen (also auch von diesem Makro)     |
--|         aktiv ist. Deshalb werden die Signale solange aktiv gehalten bis das Ende des jeweiligen    |
--|         erreicht ist ("ext_wr_fin", "ext_rd_fin).                                                   |
--+-----------------------------------------------------------------------------------------------------+

--+-----------------------------------------------------------------------------------------------------+
--| Stand:  05.11.2012  Version 2, Revision 2 (V2R2)                                                    |
--| Autor:  W. Panschow / S. Rauch                                                                      |
--| V2R2:   Der FG kann wieder mit einem externen Signal gestartet werden. Mit broadcast_en = '1' wird  |
--|         der FG armiert. Dann startet er entweder mit einem Schreibzugriff auf die                   |
--|         broadcast_wr_fg-Adresse oder wenn A_Spare0-Signal des SCUBusses auf '0' geht.               |
--|         Das A_Spare0-Signal wird vom SCU-Busmaster getrieben.                                       |
--+-----------------------------------------------------------------------------------------------------+

--+-----------------------------------------------------------------------------------------------------+
--| Stand:  16.01.2013  Version 2, Revision 3 (V2R3)                                                    |
--| Autor:  W. Panschow                                                                                 |
--| V2R3:   Durch den Einbau des externen Startsignales (V2R2) kann ein ungewollter Betriebszustand     |
--|         im Broadcastmode eintreten. Bis (V2R1) musste der Broadcastmode aktiviert werden und durch  |
--|         schreiben auf die Broadcast-Start-Adresse wurde die FG-Interpolation gestartet.             |
--|         Mit schreiben eines Sollwert1/2 wurde der Interpolationsmode des FG gestoppt und der FG     |
--|         wurde erst durch erneuten Broadcast-Start wieder gestartet (das Broadcast-Enable blieb      |
--|         aktiv!). Bei der bisherigen Startmethode durch schreiben auf die Broadcast-Start-Adresse,   |
--|         hat das nicht gestört, da der Fg auf diese Aktion gewartet hat.                             |
--|         Beim Betrieb mit externem Startsignal funktioniert dies nur eingechränkt. Sobald das Signal |
--|         aktiv wird würde der FG sofort wieder mit dem Interpolieren starten, weil das Broadcast-    |
--|         Enable immer noch gesetzt blieb, obwohl Sollwert1/2 zum stoppen der FG-Interpolation        |
--|         geschrieben wurde.                                                                          |
--|         Um dies zu vermeiden wird beim schreiben von Sollwert1/2 auch das Broadcast-Enable gelöscht.|
--|         Vor jedem neuen Start der FG-Interpolation muss das Broadcast-Enable wieder gesetzt werden. |
--|         Realistiert ist das Rücksetzen des "Broadcast_en"-Bits im Prozess "status_1_2".             |
--|         Bei der Überarbeitung des "Broadcast_en" wurde auch das "Mapping" des SW4- und SW5-Register |
--|         von internen Signalen (s.u.) korrigiert.                                                    |
--|                                                                                                     |
--|         s_add_sel           <= s_status1_reg(2 downto 0);   -- Auswahl Addierschritte               |
--|         s_freq_sel          <= s_status1_reg(5 downto 3);   -- Auswahl der Addierfreqeuenz          |
--|         s_is_slave          <= s_status1_reg(6);            -- Master/Slave                         |
--|         s_freq_ext          <= s_status1_reg(7);            -- Benutze externen Takt                |
--|                                                                                                     |
--|         only_data_points    <= s_status2_reg(0);            -- Interpolation ein/aus                |
--|         sw3_mul_by_32       <= s_status2_reg(1);            -- Shift um 1 oder 5 Bit                |
--|         broadcast_en        <= s_status2_reg(2);            -- Start bei Broadcast                  |
--|         s_twos_comp         <= s_status2_reg(3);            -- 1er oder 2er Komplement              |
--|                                                                                                     |
--|         auf Mapping mittels Alias-Deklaration umgestellt (s.u.)                                     |
--|                                                                                                     |
--|         alias   s_add_sel:          std_logic_vector(2 downto 0)    is  s_status1_reg(2 downto 0);  |
--|         alias   s_freq_sel:         std_logic_vector(2 downto 0)    is  s_status1_reg(5 downto 3);  |
--|         alias   s_is_slave:         std_logic                       is  s_status1_reg(6);           |
--|         alias   s_freq_ext:         std_logic                       is  s_status1_reg(7);           |
--|                                                                                                     |
--|         alias   only_data_points:   std_logic                       is  s_status2_reg(0);           |
--|         alias   sw3_mul_by_32:      std_logic                       is  s_status2_reg(1);           |
--|         alias   broadcast_en:       std_logic                       is  s_status2_reg(2);           |
--|         alias   s_twos_comp:        std_logic                       is  s_status2_reg(3);           |
--|                                                                                                     |
--|         Das Layout von "s_status1_out" wurde korrigiert. An Bit 7 ist jetzt das Bit 7 des           |
--|         s_status1_reg angeschlossen (s_freq_ext).                                                   |
--|         alt:    s_status1_out <=    s_gate_out_active                                               |
--|                                 &   s_gate_in_nactive                                               |
--|                                 &   s_ext_freq_active                                               |
--|                                 &   only_data_points                                                |
--|                                 &   broadcast_en                                                    |
--|                                 &   ramp_modus                                                      |
--|                                 &   internal_error                                                  |
--|                                 &   sw3_timeout                                                     |
--|                                 &   sw3_mul_by_32               (jetzt nur in s_status2_out)        |
--|                                 &   s_status1_reg(6 downto 0);                                      |
--|                                                                                                     |
--|         neu:    s_status1_out <=    s_gate_out_active                                               |
--|                                 &   s_gate_in_nactive                                               |
--|                                 &   s_ext_freq_active                                               |
--|                                 &   only_data_points                                                |
--|                                 &   broadcast_en                                                    |
--|                                 &   ramp_modus                                                      |
--|                                 &   internal_error                                                  |
--|                                 &   sw3_timeout                                                     |
--|                                 &   s_status1_reg(7 downto 0);                                      |
--+-----------------------------------------------------------------------------------------------------+

--+-----------------------------------------------------------------------------------------------------+
--| Stand:  07.02.2013  Version 3, Revision 0 (V3R0)                                                    |
--|     Die Version und Revision dieses Makros soll nicht mehr aus dem Namen ersichtlich sein.          |
--|     Die letzte Version die diesen Schema folgte, lautete "SCU_Slave_FG_V2R3".                       |
--|     Ab der Vers_3_Revi_0, wird der Makro nur noch "SCU_Slave_FG" heissen.                           |
--|     Wenn der "SCU_Slave_FG" in einem Block-Design-File als Symbol eingebunden werden soll, wird     |
--|    über die Generics                                                                                |
--|         "This_macro_vers_dont_change_from_outside" die Version und                                  |
--|         "This_macro_vers_dont_change_from_outside" die Revision angezeigt.                          |
--|     Das setzt vorraus, das vom aktuell eingebudenen SCU_Bus_Slave.vhd-File ein neues BSF generiert  |
--|     wird.                                                                                           |
--|     Wird der SCU_Bus_Slave als Vhdl-Componente in ein in der Hirachie höheren Vhdl-File eingefügt,  |
--|     muesste die Componenten-Deklaration haendisch aktualisiert werden. Deshalb soll dieser Makro    |
--|      als Package in das übergeordnete Vhdl-File eingebunden werden.                                 |
--+-----------------------------------------------------------------------------------------------------+


library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;

entity SCU_Slave_FG is
  generic
    (
    sys_clk_in_hz:          natural := 100_000_000;
    broadcast_start_addr:   natural := 16#1030#;
    base_addr:              natural := 16#100#;
    This_macro_vers_dont_change_from_outside: integer   range 0 to 2**3-1 := 3; -- change version only here!
                                                                                -- increment by major changes of this macro
    This_macro_revi_dont_change_from_outside: integer   range 0 to 2**4-1 := 0  -- change revision only here!
                                                                                -- increment by minor changes of this macro
    );
  port
   (
   fg_clk:           in    std_logic;                      -- attention, fg_clk is 8.192 Mhz
   ext_clk:          in    std_logic;                      --
   sys_clk:          in    std_logic;                      -- should be the same clk, used by SCU_Bus_Slave
   ADR_from_SCUB_LA: in    std_logic_vector(15 DOWNTO 0);  -- latched address from SCU_Bus
   Ext_Adr_Val:      in    std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
   Ext_Rd_active:    in    std_logic;                      -- '1' => Rd-Cycle is active
   Ext_Rd_fin:       in    std_logic;                      -- marks end of read cycle, active one for one
                                                           -- clock period of sys_clk
   Ext_Wr_active:    in    std_logic;                      -- '1' => Wr-Cycle is active
   Ext_Wr_fin:       in    std_logic;                      -- marks end of write cycle, active one for one
                                                           -- clock period of sys_clk
   Data_from_SCUB_LA:in    std_logic_vector(15 DOWNTO 0);  -- latched data from SCU_Bus 
   nPowerup_Res:     in    std_logic;                      -- '0' => the FPGA make a powerup
   nStartSignal:     in    std_logic;                      -- '0' => started FG if broadcast_en = '1'
   Data_to_SCUB:     out   std_logic_vector(15 DOWNTO 0);  -- connect read sources to SCUB-Macro
   Dtack_to_SCUB:    out   std_logic;                      -- connect Dtack to SCUB-Macro
   dreq:             out   std_logic;                      -- data request interrupt for new SW3
   read_cycle_act:   out   std_logic;                      -- this macro has data for the SCU-Bus
   sw_out:           out   std_logic_vector(31 downto 0);  -- 
   new_data_strobe:  out   std_logic;                      -- sw_out changed
   data_point_strobe:out   std_logic                       -- data point reached
   );                     

--!!!! ext frequenz noch nicht eingebaut.
                          
                          
end entity SCU_Slave_FG;

architecture SCU_Slave_FG_arch of SCU_Slave_FG is

  constant  c_fg_vers:      unsigned(2 downto 0) :=  to_unsigned(This_macro_vers_dont_change_from_outside, 3);
  constant  c_fg_revi:      unsigned(3 downto 0) :=  to_unsigned(This_macro_revi_dont_change_from_outside, 4);
  constant  c_fg_version:   std_logic_vector(6 downto 0) := std_logic_vector(c_fg_vers) & std_logic_vector(c_fg_revi);

  -- Definition der Konstanten für Frequenz und Addierschritte

  type int_array is array(7 DOWNTO 0) of integer;
  
  constant c_freq_cnt: int_array := (   
                                    2**2 -2,    -- FG_clk divide by 4 => 2.048 MHz
                                    2**3 -2,    -- FG_clk divide by 8 => 1.024 MHz
                                    2**4 -2,    -- FG_clk divide by 16 => 512 kHz   
                                    2**5 -2,    -- FG_clk divide by 32 => 256 kHz
                                    2**6 -2,    -- FG_clk divide by 64 => 128 khz
                                    2**7 -2,    -- FG_clk divide by 128 => 64 kHz
                                    2**8 -2,    -- FG_clk divide by 256 => 32 kHz
                                    2**9 -2     -- FG_clk divide by 512 => 16 kHz
                                    );

  constant c_add_cnt: int_array := (
                                   2**8 -1,     -- 256 Addierschritte bis zum nächsten Stützpunkt
                                   2**9 -1,     -- 512 Addierschritte bis zum nächsten Stützpunkt
                                   2**10 -1,    -- 1024 Addierschritte bis zum nächsten Stützpunkt
                                   2**11 -1,    -- 2048 Addierschritte bis zum nächsten Stützpunkt
                                   2**12 -1,    -- 4096 Addierschritte bis zum nächsten Stützpunkt
                                   2**13 -1,    -- 8192 Addierschritte bis zum nächsten Stützpunkt
                                   2**14 -1,    -- 16384 Addierschritte bis zum nächsten Stützpunkt
                                   2**15 -1     -- 32768 Addierschritte bis zum nächsten Stützpunkt
                                   );

  -- Bitbreite nach dem größten Wert bestimmen
  constant  c_freq_cnt_width:       integer := integer(ceil(log2(real(c_freq_cnt(0)))));
  constant  c_add_cnt_width:        integer := integer(ceil(log2(real(c_add_cnt(0)))));

  constant  addr_width:             integer := 16;

  constant  sw1_addr_offset:            integer := 1;   -- Setzen oder rücklesen von SW_Out(31..16)
  constant  sw2_addr_offset:            integer := 2;   -- Setzen oder rücklesen von SW_Out(15..0)
  constant  sw3_addr_offset:            integer := 3;   -- Setzen des Addierwertes, wird noch, in Abhängikeit von Status-Register2, mit 2 oder 32 multipliziert
  constant  sw4_addr_offset:            integer := 4;   -- Vorgabe: Addierschrittanzahl, Addierfrequenz, externe/lokale Frequenz
  constant  sw5_addr_offset:            integer := 5;   -- Interpolation ein/aus, sw3 um 1 oder 5 geschiftet, Starte mit Broadcast, sw3 im 1er oder 2er Komplement
  constant  status1_addr_offset:        integer := 6;
  constant  status2_addr_offset:        integer := 7;
  constant  reset_wr_addr_offset:       integer := 8;   -- FG rücksetzen
  constant  hw_data_point_addr_offset:  integer := 9;   -- speichert den jeweils letzten Stützpunkwert(31..16)
  constant  lw_data_point_addr_offset:  integer := 10;  -- speichert den jeweils letzten Stützpunkwert(15..0)
  
  -- Falls mehrere Funktionsgeneratoren in einem SCU-Slave eingesetzt werden, sollen sie gleichzeitig zu starten sein.        --
  -- Deshalb liegt die Broadcast-Adresse fest auf 100 hex. Mit  e i n e m  Schreibzugriff auf diese Adresse lassen sich       --
  -- m e h r e r e Funktionsgeneratoren starten, sofern sie individuell dafür freigegeben wurden.                                --
  constant  c_broadcast_addr:       unsigned(addr_width-1 downto 0) := to_unsigned(broadcast_start_addr, addr_width);

  -- Alle nachfolgen Adressen sind individuell und errechnen sich aus dem Generic "base_addr" + dem entsprechenden Offset.    --
  -- Achtung! Bei der Instanzierung von mehreren FGs ist darauf zu achten, dass sich die Adressbereiche der einzelnen FGs     --
  -- nicht überlappen.                                                                                                       --
  constant  c_reset_wr_addr:        unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + reset_wr_addr_offset), addr_width);
  constant  c_sw1_addr:             unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + sw1_addr_offset), addr_width);
  constant  c_sw2_addr:             unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + sw2_addr_offset), addr_width);
  constant  c_sw3_addr:             unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + sw3_addr_offset), addr_width);
  constant  c_sw4_addr:             unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + sw4_addr_offset), addr_width);
  constant  c_sw5_addr:             unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + sw5_addr_offset), addr_width);
  constant  c_status1_addr:         unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + status1_addr_offset), addr_width);
  constant  c_status2_addr:         unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + status2_addr_offset), addr_width);
  constant  c_hw_data_point_addr:   unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + hw_data_point_addr_offset), addr_width);
  constant  c_lw_data_point_addr:   unsigned(addr_width-1 downto 0) := to_unsigned((base_addr + lw_data_point_addr_offset), addr_width);

  -- Vereinbarungen zur Überwachung der Exteren Clock
  constant  sys_clk_in_ps:              integer := 1_000_000_000 / (sys_clk_in_Hz / 1000);
  constant  test_time_ext_clk_in_us:    integer := 100;     -- 0,1 ms   --
  constant  test_time_ext_clk_cnt_val:  integer := (test_time_ext_clk_in_us * 1_000_000 / sys_clk_in_ps) - 2;
  constant  test_time_ext_clk_cnt_width:integer := integer(ceil(log2(real(test_time_ext_clk_cnt_val))));
  signal    test_time_ext_clk_cnt:      unsigned(test_time_ext_clk_cnt_width downto 0);
  signal    ext_clk_edge_detection:     std_logic_vector(2 downto 0);
  signal    ext_clk_edge_cnt:           integer range 0 to 1024;
  signal    ext_clk_edge_cnt_mem:       integer range 0 to 1024;
  signal    ext_clk_ok:                 std_logic;

  
  signal    s_freq_en:                  std_logic;
  signal    s_freq_cnt:                 unsigned(c_freq_cnt_width downto 0);
  signal    data_point_reached:         std_logic;
  
  
  signal    s_add_cnt:                  unsigned(c_add_cnt_width downto 0);
    
  signal    s_status1_reg:              std_logic_vector(15 downto 0);
  signal    s_status2_reg:              std_logic_vector(15 downto 0);
  
  signal    s_status1_out:              std_logic_vector(15 downto 0); --Bus für Status lesen
  signal    s_status2_out:              std_logic_vector(15 downto 0); --Bus für Status lesen
  
  -- Signale aus dem Statusregister
  signal    sw3_timeout:                std_logic;
  signal    internal_error:             std_logic;
  signal    ramp_modus:                 std_logic;
  signal    s_ext_freq_active:          std_logic;
  signal    s_gate_in_nactive:          std_logic;
  signal    s_gate_out_active:          std_logic;


  alias     s_add_sel:          std_logic_vector(2 downto 0)    is  s_status1_reg(2 downto 0);
  alias     s_freq_sel:         std_logic_vector(2 downto 0)    is  s_status1_reg(5 downto 3);
  alias     s_is_slave:         std_logic                       is  s_status1_reg(6);
  alias     s_freq_ext:         std_logic                       is  s_status1_reg(7);
  
  alias     only_data_points:   std_logic                       is  s_status2_reg(0);
  alias     sw3_mul_by_32:      std_logic                       is  s_status2_reg(1);
  alias     broadcast_en:       std_logic                       is  s_status2_reg(2);
  alias     s_twos_comp:        std_logic                       is  s_status2_reg(3);
  
  -- Signale für FG_SM
  type  fg_sm_type is   (
                        idle,
                        drq,
                        run,
                        err
                        );      

  signal    fg_sm:      fg_sm_type;

  signal  next_step_value_en:         std_logic;
  
  -- Signale für Datenpfad
  signal    sw3_reg:                signed(15 downto 0);
  signal    raw_step_value:         signed(15 downto 0);
  signal    step_value:             signed(31 downto 0);
  signal    accu_reg, adder_result: signed(31 downto 0);
  signal    sw_out_reg:             signed(31 downto 0);
  signal    data_point_mem:         signed(31 downto 0);
  
  signal    wr_req_to_fg:           std_logic;
--signal  wr_req_to_fg_activ: std_logic;

  signal  broadcast_wr:             std_logic;
  signal  broadcast_wr_fg:          std_logic;
  signal  reset_wr:                 std_logic;
  signal  sw1_wr:                   std_logic;
  signal  sw1_wr_fg:                std_logic;
  signal  sw1_rd:                   std_logic;
  signal  sw2_wr:                   std_logic;
  signal  sw2_wr_fg:                std_logic;
  signal  sw2_rd:                   std_logic;
  signal  sw3_wr:                   std_logic;
  signal  sw3_wr_fg:                std_logic;
  signal  sw4_wr:                   std_logic;
  signal  sw4_wr_fg:                std_logic;
  signal  sw5_wr:                   std_logic;
  signal  sw5_wr_fg:                std_logic;
  signal  status1_rd:               std_logic;
  signal  status1_rd_fg:            std_logic;
  signal  status2_rd:               std_logic;
  signal  hw_data_point_rd:         std_logic;
  signal  lw_data_point_rd:         std_logic;

  signal  sync_to_sys_clk:          std_logic_vector(2 downto 0);
  signal  fg_clk_edge_detect:       std_logic;

  signal  rd_dtack:                 std_logic;
  signal  wr_dtack:                 std_logic;

  signal  rd_port:                  std_logic_vector(15 downto 0);
  signal  s_read_cycle_act:         std_logic;
  
  signal  first_data_point:         std_logic;
  signal  dly_data_point:           std_logic_vector(2 downto 0);
  signal  dly_new_data:             std_logic_vector(1 downto 0);
  signal  sw_reg_en:                std_logic;
  signal  adder_active:             std_logic;
  
  signal  s_dreq:                   std_logic;
  signal  s_interlock:              std_logic;
    
  signal  res_sys_clk_sync:         std_logic;
  signal  sys_clk_reset_cnt:        std_logic_vector(2 downto 0) := (OTHERS => '0');
  signal  reset_wr_dtack:           std_logic;


begin

p_ext_clk_cntrl: process (sys_clk, nPowerup_Res)
  begin
    if nPowerup_res = '0' then
      ext_clk_edge_cnt <= 0;
      test_time_ext_clk_cnt <= to_unsigned(0, test_time_ext_clk_cnt'length);
    elsif rising_edge(sys_clk) then
      ext_clk_edge_detection <= ext_clk_edge_detection(1 downto 0) & ext_clk;
      -- Zähler bestimmt das Messintervall
      if test_time_ext_clk_cnt(test_time_ext_clk_cnt'high) = '1' then
        test_time_ext_clk_cnt <= to_unsigned(test_time_ext_clk_cnt_val, test_time_ext_clk_cnt'length);
        ext_clk_edge_cnt_mem <= ext_clk_edge_cnt;
        ext_clk_edge_cnt <= 0;
      else
        test_time_ext_clk_cnt <= test_time_ext_clk_cnt - 1;
        -- Zähler für die während des Messintervalles dedektierten Flanken der ext_clk
        if ext_clk_edge_detection(2) = '0' and ext_clk_edge_detection(1) = '1' then
          if ext_clk_edge_cnt < 1024 then
            ext_clk_edge_cnt <= ext_clk_edge_cnt + 1;
          end if;
        end if;
      end if;
      if (800 < ext_clk_edge_cnt_mem) and (ext_clk_edge_cnt_mem < 838) then
         ext_clk_ok <= '1';
      else
         ext_clk_ok <= '0';
      end if;
    end if;
  end process p_ext_clk_cntrl;


p_sys_clk_sync_res: process (sys_clk)
  begin
    if rising_edge(sys_clk) then
      if nPowerup_Res = '0' then
        res_sys_clk_sync <= '1';              
      elsif (reset_wr = '1' and res_sys_clk_sync = '0' and reset_wr_dtack = '0') then
        res_sys_clk_sync <= '1';
        sys_clk_reset_cnt <= std_logic_vector(to_unsigned(2**(sys_clk_reset_cnt'length-1)-1, sys_clk_reset_cnt'length));
      elsif  sys_clk_reset_cnt(sys_clk_reset_cnt'high) = '0' then 
        sys_clk_reset_cnt <= std_logic_vector(unsigned(sys_clk_reset_cnt) - 1);
      else
        reset_wr_dtack <= reset_wr;
        res_sys_clk_sync <= '0';
      end if;
    end if;
  end process;

    
-- Downcounter für den Frequenzteiler

freq_cnt: process(sys_clk, res_sys_clk_sync)
  begin
    -- wichtig für synchronen Start 
    if res_sys_clk_sync = '1' then
      s_freq_cnt <= (others => '0');
    elsif rising_edge(sys_clk) then
      s_freq_en <= '0';
      -- Bei Überlauf des Downcounters wird der Couter neu geladen
      if fg_clk_edge_detect = '1' THEN
        s_freq_en <= s_freq_cnt(s_freq_cnt'high);
        if s_freq_cnt(s_freq_cnt'high) = '1' or ramp_modus = '0' then
           -- Konstante aus Array auswählen
           s_freq_cnt <= to_unsigned(c_freq_cnt(to_integer(unsigned(s_freq_sel))), s_freq_cnt'length);
        elsif ramp_modus = '1' then
           s_freq_cnt <= s_freq_cnt - 1;
        end if;
      end if;
    end if;
  end process freq_cnt;   
    

-- Downcounter für die Addierschritte  
data_point_reached <= s_add_cnt(s_add_cnt'high);

add_cnt: process(sys_clk, res_sys_clk_sync)
  begin
    if res_sys_clk_sync = '1' then
      s_add_cnt <= (others => '0');
    elsif rising_edge(sys_clk) then
      -- Bei Unterlauf oder bei nicht gestartetem FG wird mit der Anzahl der Addierschritte geladen.  --
      -- Die Addierschritt-Anzahl kann aus acht konstanten Vorgaben ausgewählt werden.               --
      if data_point_reached = '1' or ramp_modus = '0' then
        -- Addierschritt-Anzahl aus Array, durch Sollwert4 gesteuert, auswählen.                       --
        s_add_cnt <= to_unsigned(c_add_cnt(to_integer(unsigned(s_add_sel))), s_add_cnt'length);
      elsif s_freq_en = '1' and ramp_modus = '1' then
        s_add_cnt <= s_add_cnt - 1;
      end if;
    end if;
  end process add_cnt;
    
    
wr_req_to_fg <=    broadcast_wr
                or sw1_wr
                or sw2_wr
                or sw3_wr
                or sw4_wr
                or sw5_wr
                or status1_rd;  -- muss im Timing als Schreibzugriff abgearbeitet werden, da beim Status1-Lesen Fehlerbits zurückgesetzt werden. 


broadcast_wr_fg <= broadcast_wr and fg_clk_edge_detect;
sw1_wr_fg       <= sw1_wr and fg_clk_edge_detect;
sw2_wr_fg       <= sw2_wr and fg_clk_edge_detect;
sw3_wr_fg       <= sw3_wr and fg_clk_edge_detect;
sw4_wr_fg       <= sw4_wr and fg_clk_edge_detect;
sw5_wr_fg       <= sw5_wr and fg_clk_edge_detect;
status1_rd_fg   <= status1_rd and  fg_clk_edge_detect;  -- muss im Timing als Schreibzugriff abgearbeitet werden, da beim Status1-Lesen Fehlerbits zurückgesetzt werden. 


p_fg_clk_edge_detect: process (sys_clk, res_sys_clk_sync)
    begin
        if res_sys_clk_sync = '1' then
            sync_to_sys_clk <= (OTHERS => '0');
            fg_clk_edge_detect <= '0';
        elsif rising_edge(sys_clk) THEN
            sync_to_sys_clk <= sync_to_sys_clk(sync_to_sys_clk'high-1 downto 0) & fg_clk;
            fg_clk_edge_detect <= not sync_to_sys_clk(sync_to_sys_clk'high) and sync_to_sys_clk(sync_to_sys_clk'high-1);
        end if;
    end process p_fg_clk_edge_detect;


p_dtack: process (sys_clk, res_sys_clk_sync)
  begin
    if res_sys_clk_sync = '1' then
      rd_dtack <= '0';
      wr_dtack <= '0';
    elsif rising_edge(sys_clk) then
      if wr_req_to_fg = '1' and fg_clk_edge_detect = '1' then
        wr_dtack <= '1';
      elsif ext_wr_fin = '1' then
        wr_dtack <= '0';
      end if;
      
      if (    sw1_rd = '1'
          or  sw2_rd = '1'
          or  status2_rd = '1'
          or  hw_data_point_rd = '1'
          or  lw_data_point_rd = '1'
          ) and fg_clk_edge_detect = '1'  -- nur einen Takt aktiv!
      then
        rd_dtack <= '1';
      elsif ext_rd_fin = '1' then     -- V1R1: rd_dtack steuert indirekt den SCU-Daten-Bus im SCUB_Slave_Makro! Der Bus   --
        rd_dtack <= '0';            -- muss so lange aktiv bleiben, bis der SCU_Master die Daten übernommen hat.       --
      end if;
    end if;
  end process p_dtack;

                    
Dtack_to_SCUB <= wr_dtack or rd_dtack or reset_wr_dtack;

    
p_adr_deco: process (sys_clk, nPowerup_Res)
  begin
    if nPowerup_Res = '0' then
      broadcast_wr <= '0';
      reset_wr <= '0';
      sw1_wr <= '0';
      sw1_rd <= '0';
      sw2_wr <= '0';
      sw2_rd <= '0';
      sw3_wr <= '0';
      sw4_wr <= '0';
      sw5_wr <= '0';
      status1_rd <= '0';
      status2_rd <= '0';
      hw_data_point_rd <= '0';
      lw_data_point_rd <= '0';
      s_read_cycle_act <= '0';
    elsif rising_edge(Sys_clk) then
      broadcast_wr <= '0';
      reset_wr <= '0';
      sw1_wr <= '0';
      sw1_rd <= '0';
      sw2_wr <= '0';
      sw2_rd <= '0';
      sw3_wr <= '0';
      sw4_wr <= '0';
      sw5_wr <= '0';
      status1_rd <= '0';
      status2_rd <= '0';
      hw_data_point_rd <= '0';
      lw_data_point_rd <= '0';
      s_read_cycle_act <= '0';
      if Ext_Adr_Val = '1' then
        case unsigned(ADR_from_SCUB_LA) is
          when c_broadcast_addr =>
            if Ext_Wr_active = '1' then
              broadcast_wr <= '1';
            end if;
          when c_reset_wr_addr =>
            if Ext_Wr_active = '1' then
              reset_wr <= '1';
            end if;
          when c_sw1_addr =>
            if Ext_Wr_active = '1' then
              sw1_wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              sw1_rd <= '1';
              s_read_cycle_act <= '1';
            end if;
          when c_sw2_addr =>
            if Ext_Wr_active = '1' then
              sw2_wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              sw2_rd <= '1';
              s_read_cycle_act <= '1';
            end if;
          when c_sw3_addr =>
            if Ext_Wr_active = '1' then
              sw3_wr <= '1';
            end if;
          when c_sw4_addr =>
            if Ext_Wr_active = '1' then
              sw4_wr <= '1';
            end if;
          when c_sw5_addr =>
             if Ext_Wr_active = '1' then
               sw5_wr <= '1';
            end if;
          when c_status1_addr =>
            if Ext_Rd_active = '1' then
              status1_rd <= '1';
              s_read_cycle_act <= '1';
            end if;
          when c_status2_addr =>
            if Ext_Rd_active = '1' then
              status2_rd <= '1';
              s_read_cycle_act <= '1';
            end if;
          when c_hw_data_point_addr =>
            if Ext_Rd_active = '1' then
              hw_data_point_rd <= '1';
              s_read_cycle_act <= '1';
            end if;
          when c_lw_data_point_addr =>
            if Ext_Rd_active = '1' then
              lw_data_point_rd <= '1';
              s_read_cycle_act <= '1';
            end if;
          when others =>  
            broadcast_wr <= '0';
            reset_wr <= '0';
            sw1_wr <= '0';
            sw1_rd <= '0';
            sw2_wr <= '0';
            sw2_rd <= '0';
            sw3_wr <= '0';
            sw4_wr <= '0';
            sw5_wr <= '0';
            status1_rd <= '0';
            status2_rd <= '0';
            hw_data_point_rd <= '0';
            lw_data_point_rd <= '0';
            s_read_cycle_act <= '0';
            -- null; nicht verwenden!!!!!!! Gibt in der Synthese von Quartus fehlerhafte Resultate. Modelsim funktioniert richtig!!!!!
        end case;
      end if;
    end if;
  end process p_adr_deco;
    

read_mux: process (sys_clk)
  begin
    if rising_edge(sys_clk) then
      if sw1_rd = '1' and fg_clk_edge_detect = '1' then
        rd_port <=  std_logic_vector(accu_reg(31 downto 16));
      end if;
      if sw2_rd = '1' and fg_clk_edge_detect = '1' then
        rd_port <=  std_logic_vector(accu_reg(15 downto 0));
      end if;
      if status1_rd = '1' and fg_clk_edge_detect = '1' then
        rd_port <=  s_status1_out;
      end if;
      if status2_rd = '1' and fg_clk_edge_detect = '1' then
        rd_port <=  s_status2_out;
      end if;
      if hw_data_point_rd = '1' and fg_clk_edge_detect = '1' then
        rd_port <= std_logic_vector(data_point_mem(31 downto 16));
      end if;
      if lw_data_point_rd = '1' and fg_clk_edge_detect = '1' then
        rd_port <= std_logic_vector(data_point_mem(15 downto 0));
      end if;
--    else
--        rd_port <= (others => '-');
--    end if;
    end if;
  end process read_mux;

Data_to_SCUB <= rd_port;
    
    
    
-- State Machine für Kontroll Ablauf
p_fg_sm: process (sys_clk, res_sys_clk_sync)
  begin
    if res_sys_clk_sync = '1' then
      fg_sm <= idle;

    elsif rising_edge(sys_clk) then
      sw3_timeout         <= '0';
      internal_error      <= '0';
      first_data_point    <= '0';
      ramp_modus          <= '0';
      next_step_value_en  <= '0';
        
      case fg_sm is
        when idle =>
          s_dreq          <= '0';

          -- start ramp mode by receiving an STP or by broadcast
          if broadcast_en = '1' and (broadcast_wr_fg = '1' or nStartSignal = '0') then        -- an external Signal or a write cycle to broadcast_wr_fg address, starts the FG
            first_data_point <= '1';
            fg_sm <= drq;
          end if;
            
        when drq =>
          s_dreq          <= '1'; -- IRQ for the next SW3
          ramp_modus      <= '1'; -- we are now in ramp mode
          next_step_value_en <= '1'; -- load next step value
            
          fg_sm   <= run;
    
        when run =>
          ramp_modus <= '1'; -- we are still in ramp mode

          if sw1_wr_fg = '1' then
            fg_sm <= idle;
          end if;
            
          if sw2_wr_fg = '1' then
            fg_sm <= idle;
          end if;
            
          if sw3_wr_fg = '1' then
             s_dreq <= '0';
          end if;
  
          if data_point_reached = '1' then  -- we have reached the new next STP
            if s_dreq = '1' then -- if there is no received STP, set timeout failure
              fg_sm <= err;
            else
              fg_sm <= drq;
            end if;
          end if;
            
        when err =>
          sw3_timeout <= '1';
            
            -- bei Status1 lesen werden die Fehlerbits 8 und 9 rückgesetzt
          if status1_rd_fg = '1' then
            fg_sm <= idle;
          end if;
            
        when others =>
          internal_error  <= '1';
            
          -- bei Status1 lesen werden die Fehlerbits 8 und 9 rückgesetzt
          if status1_rd_fg = '1' then
            fg_sm <= idle;
          end if;

          end case;

        end if;
  end process p_fg_sm;
    

    -- Funktion nicht implementiert
    -- Signale auf GND
    s_gate_out_active   <= '0';
    s_gate_in_nactive   <= '0';
    s_ext_freq_active   <= '0';
        
    
    -- Mapping für Status1 Lesen
    s_status1_out <=    s_gate_out_active
                    &   s_gate_in_nactive
                    &   s_ext_freq_active
                    &   only_data_points
                    &   broadcast_en
                    &   ramp_modus
                    &   internal_error
                    &   sw3_timeout
                    &   s_status1_reg(7 downto 0);
                    
    -- Mapping für Status2 Lesen
    s_status2_out <=    s_gate_out_active
                    &   s_gate_in_nactive
                    &   s_ext_freq_active
                    &   only_data_points
                    &   broadcast_en
                    &   ramp_modus
                    &   internal_error
                    &   sw3_timeout
                    &   sw3_mul_by_32       -- bit 8
                    &   c_fg_version;       -- achtung nur 7 bit breit


-- Register für die Stützpunkt Sollwerte
p_reg_sw3: process(sys_clk, res_sys_clk_sync)
  begin
    if res_sys_clk_sync = '1' then
      sw3_reg <= (others => '0');
      raw_step_value <= (others => '0');
    elsif rising_edge(sys_clk) then
      if sw3_wr_fg = '1' then
        sw3_reg <= signed(Data_from_SCUB_LA);
      end if;
      if next_step_value_en = '1' then
        raw_step_value <= sw3_reg;
      end if;
    end if;
  end process p_reg_sw3;


-- Register für Zwischenspeichern des letzten Stützpunktes
p_data_point_mem: process(sys_clk, res_sys_clk_sync)
  begin
    if res_sys_clk_sync = '1' then
      data_point_mem <= (others => '0');
    elsif rising_edge(sys_clk) then
      if next_step_value_en = '1' then
        data_point_mem <= accu_reg;
      end if;
    end if;
  end process p_data_point_mem;


-- Auswahl Komplement und Dynamikbereich
calc_step_value: process (sw3_mul_by_32, s_twos_comp, raw_step_value)
  begin
    if sw3_mul_by_32 = '1' then
      -- Multiplikation mit 32
      if s_twos_comp = '1' then
        step_value <= (31 downto 20 => raw_step_value(raw_step_value'high)) & raw_step_value(14 downto 0) & (4 downto 0 => '0');
      else
        -- 1er Komplement Darstellung
        step_value <= (31 downto 20 => raw_step_value(raw_step_value'high)) & raw_step_value(14 downto 0) & (4 downto 0 => raw_step_value(raw_step_value'high));
      end if;
    else
      -- Multiplikation mit 2
      if s_twos_comp = '1' then
        step_value <= (31 downto 16 => raw_step_value(raw_step_value'high)) & raw_step_value(14 downto 0) & '0';
      else
        -- 1er Komplement Darstellung
        step_value <= (31 downto 16 => raw_step_value(raw_step_value'high)) & raw_step_value(14 downto 0) & raw_step_value(raw_step_value'high);
      end if;
    end if;
  end process calc_step_value;
    

status_1_2: process (sys_clk, res_sys_clk_sync)
  begin
    if res_sys_clk_sync = '1' then
      s_status1_reg   <=  (others => '0');
      s_status2_reg   <=  (others => '0');
    elsif rising_edge(sys_clk) then
      if sw4_wr_fg = '1' then
        s_status1_reg <= std_logic_vector(signed(Data_from_SCUB_LA));
      end if;
      if sw5_wr_fg = '1' then
        s_status2_reg <= std_logic_vector(signed(Data_from_SCUB_LA));           
      end if;
      if (sw1_wr_fg = '1') or (sw2_wr_fg = '1') then
        broadcast_en <= '0';
      end if;
    end if;
  end process status_1_2;


p_adder: process (s_twos_comp, step_value, accu_reg)
  begin
    if s_twos_comp = '0' and step_value(step_value'high) = '1' then
      -- Bei 1er Komplement muß das MSB auf Carry-In
      adder_result <= step_value + accu_reg + 1;
    else
      adder_result <= step_value + accu_reg;
    end if;
  end process p_adder;


-- Accu kann direkt mit SW1(HW) oder SW2(LW) geladen werden. Diese können
-- auch wieder zurückgelsen werden. Ansonsten übernimmt das accu_reg das Ergebnis
-- der Addition step_value + adder_result
                
p_accu_reg: process (sys_clk, res_sys_clk_sync)
  begin
    if res_sys_clk_sync = '1' then
      accu_reg <= (others => '0');
    elsif rising_edge(sys_clk) then
      adder_active <= '0';
      if ramp_modus = '1' and s_freq_en = '1' then
        accu_reg <= adder_result;
        adder_active <= '1';                    -- markiert Additionsschritt, wird im process "p_out_reg" zur Erzeugung des --
                                                -- Übernahmesignals des Sollwertausgangsregisters verwendet.               --
      end if;
      if sw1_wr_fg = '1' then
         accu_reg(31 downto 16) <= signed(Data_from_SCUB_LA);
      end if;
      if sw2_wr_fg = '1' then
        accu_reg(15 downto 0) <= signed(Data_from_SCUB_LA);
      end if;
    end if; 
  end process p_accu_reg;


-- bei ausgeschaltetem Interpolationsmode nur die Stützpunkte
p_out_reg: process(sys_clk, res_sys_clk_sync)
  begin
    if res_sys_clk_sync = '1' then
      sw_out_reg      <= (others => '0');
      dly_new_data    <= (others => '0');
      dly_data_point  <= (others => '0');
      sw_reg_en       <= '0';
    elsif rising_edge(sys_clk) then
        
      sw_reg_en <=        (    only_data_points and data_point_reached and adder_active)
                      or  (not only_data_points and adder_active)
                      or sw1_wr_fg
                      or sw2_wr_fg;

      if sw_reg_en = '1' then
        sw_out_reg <= accu_reg;
      end if;

      dly_new_data <= dly_new_data(dly_new_data'high-1 downto 0) & sw_reg_en;

      dly_data_point <= dly_data_point(dly_data_point'high-1 downto 0) & ((data_point_reached and adder_active) or first_data_point);

    end if;
  end process;


dreq <= s_dreq;

read_cycle_act <= s_read_cycle_act;

sw_out <= std_logic_vector(sw_out_reg);

new_data_strobe <= dly_new_data(dly_new_data'high);

data_point_strobe <=  dly_data_point(dly_data_point'high);

end architecture;