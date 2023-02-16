--TITLE "'tag_n' Autor: R.Hartmann, Stand: 19.06.2015, Vers: V01 ";

-- KK: added tag_matched pulse for really fast response for ATR use

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


library work;
use work.scu_diob_pkg.all;


ENTITY tag_n IS
  port(
    clk:                   in   std_logic;                        -- should be the same clk, used by SCU_Bus_Slave
    nReset:                in   std_logic;
    Timing_Pattern_LA:     in   std_logic_vector(31 downto 0);    -- latched timing pattern from SCU_Bus for external user functions
    Timing_Pattern_RCV:    in   std_logic;                        -- timing pattern received
    Tag_n_hw:              in   std_logic_vector(15 downto 0);    --
    Tag_n_lw:              in   std_logic_vector(15 downto 0);    --
    Tag_n_Maske:           in   std_logic_vector(15 downto 0);    --
    Tag_n_Register:        in   std_logic_vector(15 downto 0);    -- Tag-Output-Register-Nummer
    Tag_n_Level:           in   std_logic_vector(15 downto 0);    -- Tag-Level-Maske, Bit 15..0
    Tag_n_Delay_Cnt:       in   std_logic_vector(15 downto 0);    --
    Tag_n_Puls_Width:      in   std_logic_vector(15 downto 0);    --
    Tag_n_Prescale:        in   std_logic_vector(15 downto 0);    --
    Tag_n_Trigger:         in   std_logic_vector(15 downto 0);    --

    SCU_AW_Input_Reg:      in   t_IO_Reg_1_to_7_Array;          -- Input-Port's wie zum SCU-Bus

    Max_AWOut_Reg_Nr:      in   integer range 0 to 7;           -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr:       in   integer range 0 to 7;           -- Maximale AWIn-Reg-Nummer der Anwendung
    Spare0_Strobe:         in   std_logic;                        --
    Spare1_Strobe:         in   std_logic;                        --

    Tag_matched:           out  std_logic;                       -- Tag_matched is high for one clock pulse on matching Tags

    Tag_n_Reg_Nr:          out  integer range 0 to 7;            -- AWOut-Reg-Pointer
    Tag_n_New_AWOut_Data:  out  boolean;                         -- AWOut-Reg. werden mit AWOut_Reg_Array-Daten überschrieben
    Tag_n_Maske_Hi_Bits:   out  std_logic_vector(15 downto 0);   -- Maske für "High-Aktive" Bits im Ausgangs-Register
    Tag_n_Maske_Lo_Bits:   out  std_logic_vector(15 downto 0);   -- Maske für "Low-Aktive"  Bits im Ausgangs-Register
    Tag_n_Reg_Err:         out  std_logic;                       -- Config-Error: TAG-Reg-Nr
    Tag_n_Reg_max_Err:     out  std_logic;                       -- Config-Error: TAG-Reg-Nr
    Tag_n_Trig_Err:        out  std_logic;                       -- Config-Error: Trig-Reg
    Tag_n_Trig_max_Err:    out  std_logic;                       -- Config-Error: Trig-Reg
    Tag_n_Timeout:         out  std_logic;                       -- Timeout-Error ext. Trigger, Spare0/Spare1
    Tag_n_ext_Trig_Strobe: out  std_logic;                       -- ext. Trigger-Eingang, aus Input-Register-Bit
    Tag_n_FG_Start:        out  std_logic;                       -- Funktionsgenerator Start
    Tag_n_LA:              out  std_logic_vector(15 downto 0)
    );
  end tag_n;

ARCHITECTURE Arch_tag_n OF tag_n IS

constant  c_Timeout_10us:    INTEGER := 1250;   -- Counter Timeout (1250 x 8ns = 10us)


TYPE   t_Word_Array     is array (0 to 7) of std_logic_vector(15 downto 0);

TYPE   t_Boolean_Array  is array (0 to 7) of boolean;

signal  Tag_Maske_Hi_Bits:      std_logic_vector(15 downto 0);  -- Maske für "High-Aktive" Bits im Ausgangs-Register
signal  Tag_Maske_Lo_Bits:      std_logic_vector(15 downto 0);  -- Maske für "Low-Aktive"  Bits im Ausgangs-Register

signal  Tag_Delay_Cnt:          integer range 0 to 65535;       -- 0-FFFF -- Counter Delay
signal  Tag_Puls_Width:         integer range 0 to 65535;       -- 0-FFFF -- Counter Delay
signal  Tag_Pre_VZ:             integer range 0 to 65535;       -- 0-FFFF -- Prescale Verzögerungszeit
signal  Tag_Pre_MF:             integer range 0 to 65535;       -- 0-FFFF -- Prescale Pulsbreite
signal  Tag_Trig_Reg_Nr:        integer range 0 to 7;           -- 0-7    -- Register-Nummer
signal  Tag_Trig_Reg_Bit:       integer range 0 to 15;          -- Bit-Nummer
signal  Tag_Trig_Pol:           std_logic;                      -- Bit-Polatität, 0 = neg. Flanke, 1 = pos. Flanke
signal  Tag_Timeout_Loop_Cnt:   integer range 0 to 7;           -- 0-7    -- Anzahl der Timeoutzyklen in 10us Schritten
signal  Tag_Trig_Mux:           std_logic_vector(2 downto 0);   -- Mux-Sel
signal  Tag_AWOut_Reg_Nr:       integer range 0 to 7;           -- AWOut-Reg-Pointer
signal  Tag_FG_Start:           std_logic;                      -- FG-Start Flag

signal  Tag_New_AWOut_Data: boolean;        -- AWOut-Reg. werden mit AWOut_Reg_Array-Daten überschrieben

signal  Tag_Timeout_Cnt:    integer range 0 to 65535;  -- 0-FFFF -- Counter Timeout


signal Tag_Reg_Bit_Out:       std_logic_vector(15 downto 0); -- Zwischenspeicher
signal Tag_Reg_Bit_Strobe_i:  std_logic;        -- input
signal Tag_Reg_Bit_Strobe_o:  std_logic;        -- Output
signal Tag_Reg_Bit_shift:     std_logic_vector(2  downto 0); -- Shift-Reg.


type type_t is   (sm_idle,   sm_trigger, sm_trig_S0, sm_trig_S0_w, sm_trig_S1_w, sm_trig_S1, sm_trig_Reg,  sm_trig_Reg_w,
                  sm_vz_delay_t, sm_vz_delay, sm_vz_wait, sm_vz_wait1,  sm_rd1,  sm_wr1,  sm_mf_delay, sm_mf_wait, sm_mf_wait1,
                  sm_rd2,  sm_wr2, sm_fg_start,
                  sm_to_err,  sm_reg_err,  sm_reg_max_err,  sm_trig_err,  sm_trig_max_err,  sm_end);
signal sm_state:  type_t := sm_idle;

  ------------------------------------ Timeout -------------------------------------------

signal  To_Cnt_Pre:      integer range 0 to 7;       -- Timeout: Counter-Prescale
signal  To_Cnt:          integer range 0 to 65535;   -- Timeout: Counter
signal  To_cnt_akt:      std_logic;                  -- Timeout: Counter=aktiv
signal  To_Start:        std_logic;                  -- Timeout: Start_Counter

  --------------------------------- Verzögerungszeit ---------------------------------------

signal  Vz_Cnt_Pre:      integer range 0 to 255;     -- Verzögerungszeit: Counter-Prescale
signal  Vz_Cnt:          integer range 0 to 65535;   -- Verzögerungszeit: Counter
signal  Vz_cnt_akt:      std_logic;                  -- Verzögerungszeit: Counter=aktiv
signal  Vz_Start:        std_logic;                  -- Verzögerungszeit: Start_Counter

  ------------------------- Pulsbreite (Monoflop) -------------------------------------------

signal  Mf_Cnt_Pre:      integer range 0 to 255;     -- Pulsbreite: Counter-Prescale
signal  Mf_Cnt:          integer range 0 to 65535;   -- Pulsbreite: Counter
signal  Mf_cnt_akt:      std_logic;                  -- Pulsbreite: Counter=aktiv
signal  Mf_Start:        std_logic;                  -- Pulsbreite: Start_Counter





begin

  --------- Tag: Puls aus Input-Trigger-Register-Bit (1 Clock breit) --------------------

p_Tag_Reg_Bit:  PROCESS (clk, nReset)
  BEGin
    IF  nReset                = '0' then
        Tag_Reg_Bit_shift    <= (OTHERS => '0');
        Tag_Reg_Bit_Strobe_o <= '0';

    ELSIF rising_edge(clk) THEN

    Tag_Reg_Bit_shift <= (Tag_Reg_Bit_shift(Tag_Reg_Bit_shift'high-1 downto 0) & (Tag_Reg_Bit_Strobe_i)); -- Tag0_Tag_Reg_Bit_Strobe = Puls, nach der neg. Flanke von Reg_Bit

      IF Tag_Reg_Bit_shift(Tag_Reg_Bit_shift'high) = '0' AND Tag_Reg_Bit_shift(Tag_Reg_Bit_shift'high-1) = '1' THEN
        Tag_Reg_Bit_Strobe_o <= '1';
      ELSE
        Tag_Reg_Bit_Strobe_o <= '0';
      END IF;
    END IF;
  END PROCESS p_Tag_Reg_Bit;





  ---------------------------------------- Timeout -------------------------------------------

P_To:  process (clk, nReset, Tag_Timeout_Loop_Cnt, To_Start)

    begin
      if (nReset = '0') then
        To_Cnt_Pre    <=  0 ;   -- Timeout_Counter-Prescale
        To_Cnt        <=  0 ;   -- Timeout_Counter
        To_cnt_akt    <= '0';   -- Timeout_Gate

      ELSIF rising_edge(clk) then

          if (To_Start = '1') then
              To_Cnt_Pre      <= Tag_Timeout_Loop_Cnt;  -- Timeout_Counter-Prescale
              To_Cnt          <= c_Timeout_10us;        -- "Konstante für 10us"
              To_cnt_akt      <= '1';                   -- Counter aktiv (ungleich 0)

          elsif (To_Cnt  > 0) then
              To_Cnt    <=  To_Cnt-1;                   -- Counter -1, bis Counter = 0
          else
            if (To_Cnt_Pre  > 1) then
              To_Cnt_Pre    <=  To_Cnt_Pre-1;           -- Counter -1
              To_Cnt        <= c_Timeout_10us;          -- "Konstante für 10us"
            else
              To_cnt_akt   <= '0';
            end if;
      end if;
    end if;
  end process P_To;


  ---------------------------------------- Verzögerungszeit -------------------------------------------

P_Vz:  process (clk, nReset, Tag_Pre_VZ, Tag_Delay_Cnt, VZ_Start)

    begin
      if (nReset = '0') then
        Vz_Cnt_Pre    <=  0 ;   -- Verzögerungszeit_Counter-Prescale
        Vz_Cnt        <=  0 ;   -- Verzögerungszeit_Counter
        Vz_cnt_akt    <= '0';   -- Verzögerungszeit_Gate

      ELSIF rising_edge(clk) then

          if (Vz_Start = '1') then
              Vz_Cnt_Pre      <= Tag_Pre_VZ;     -- Verzögerungszeit_Counter-Prescale
              Vz_Cnt          <= Tag_Delay_Cnt;  -- Verzögerungszeit_Counter
              Vz_cnt_akt      <= '1';            -- Counter aktiv (ungleich 0)

          elsif (Vz_Cnt  > 0) then
              Vz_Cnt    <=  Vz_Cnt-1;            -- Counter -1
          else
            if (Vz_Cnt_Pre  > 1) then
              Vz_Cnt_Pre    <=  Vz_Cnt_Pre-1;    -- Counter -1
--            Vz_Cnt        <= Tag_Delay_Cnt;    -- Verzögerungszeit_Counter
              Vz_Cnt        <= Tag_Delay_Cnt-1;  -- Verzögerungszeit_Counter (-1 ist Fehlerkorrektur)
            else
              Vz_cnt_akt   <= '0';
            end if;
      end if;
    end if;
  end process P_Vz;


  ---------------------------------------- Pulsbreite (Monoflop) -------------------------------------------

P_Mf:  process (clk, nReset, Tag_Pre_Mf, Tag_Puls_Width, Mf_Start)

    begin
      if (nReset = '0') then
        Mf_Cnt_Pre    <=  0 ;   -- Pulsbreite_Counter-Prescale
        Mf_Cnt        <=  0 ;   -- Pulsbreite_Counter
        Mf_cnt_akt    <= '0';   -- Pulsbreite_Gate

      ELSIF rising_edge(clk) then

          if (Mf_Start = '1') then
              Mf_Cnt_Pre      <= Tag_Pre_Mf;      -- Pulsbreite_Counter-Prescale
              Mf_Cnt          <= Tag_Puls_Width;  -- Pulsbreite_Counter
              Mf_cnt_akt      <= '1';             -- Counter aktiv (ungleich 0)

          elsif (Mf_Cnt  > 0) then
              Mf_Cnt    <=  Mf_Cnt-1;           -- Counter -1
          else
            if (Mf_Cnt_Pre  > 1) then
              Mf_Cnt_Pre    <=  Mf_Cnt_Pre-1;    -- Counter -1
--            Mf_Cnt        <= Tag_Puls_Width;   -- Pulsbreite_Counter
              Mf_Cnt        <= Tag_Puls_Width-1; -- Pulsbreite_Counter (-1 ist Fehlerkorrektur)
            else
              Mf_cnt_akt   <= '0';
            end if;
      end if;
    end if;
  end process P_Mf;




P_Tag_Deco:  process (clk, nReset,
                      SCU_AW_Input_Reg,
                      Tag_AWOut_Reg_Nr, Tag_FG_Start, Tag_Delay_Cnt, Tag_Puls_Width,
                      Tag_Pre_VZ, Tag_Pre_MF, Tag_Trig_Reg_Nr, Tag_Trig_Reg_Bit, Tag_Trig_Pol,
                      Tag_Timeout_Loop_Cnt, Tag_Trig_Mux, Tag_Timeout_Cnt,
                      Tag_New_AWOut_Data, Tag_Reg_Bit_Strobe_i)
    begin
      if (nReset = '0') then
        sm_state   <= sm_idle;

        Tag_AWOut_Reg_Nr       <= 0;                  -- clear AWOut_Reg_Nr
        Tag_FG_Start           <= '0';                -- Start-Flag für FG
        Tag_Delay_Cnt          <= 0;                  -- Counter Delay
        Tag_Puls_Width         <= 0;                  -- Counter Puls_Width
        Tag_Pre_VZ             <= 0;                  -- Counter Prescale-Verzögerungszeit
        Tag_Pre_MF             <= 0;                  -- Counter Prescale-Pulsbreite (Mono-Flop)
        Tag_Trig_Reg_Nr        <= 0;                  -- Register-Nummer Trigger-Input
        Tag_Trig_Reg_Bit       <= 0;                  -- Bit-Nummer vom Trigger-Register
        Tag_Trig_Pol           <= '0';                -- Bit-Polatität, 0 = neg. Flanke, 1 = pos. Flanke
        Tag_Timeout_Loop_Cnt   <= 0;                  -- Anzahl der Timeoutzyklen in 10us Schritten
        Tag_Trig_Mux           <= (others => '0');    -- clear Tag_Trigger_Mux
        Tag_Maske_Hi_Bits      <= (others => '0');    -- Maske für "High-Aktive" Bits im Ausgangs-Register
        Tag_Maske_Lo_Bits      <= (others => '0');    -- Maske für "Low-Aktive"  Bits im Ausgangs-Register
        Tag_Timeout_Cnt        <= 0;                  -- Counter Timeout
        Tag_New_AWOut_Data     <= false;              -- clear Tag_New_AWOut_Data-Flag
        Tag_Reg_Bit_Strobe_i   <= '0';

  ELSIF rising_edge(clk) then


---------------------------------------------------------- TAG 'n' ---------------------------------------------------------
      case sm_state is
        when sm_idle  =>      if ((Timing_Pattern_RCV = '1') and (Timing_Pattern_LA(31 downto 0) = (Tag_n_hw & Tag_n_lw))) then

                                  Tag_matched           <= '1';

                                  Tag_AWOut_Reg_Nr      <= to_integer(unsigned(Tag_n_Register)(2 downto 0));     -- Output-Reg. Nr. 0..7
                                  Tag_FG_Start          <= Tag_n_Register(3);                                     -- Start-Flag für FG
                                  Tag_Delay_Cnt         <= to_integer(unsigned(Tag_n_Delay_Cnt)(15 downto 0));   -- Counter Delay
                                  Tag_Puls_Width        <= to_integer(unsigned(Tag_n_Puls_Width)(15 downto 0));  -- Counter Delay
                                  Tag_Pre_VZ            <= to_integer(unsigned(Tag_n_Prescale)(15 downto 8));    -- Prescale Verzögerungszeit
                                  Tag_Pre_MF            <= to_integer(unsigned(Tag_n_Prescale)(7 downto 0));     -- Prescale Pulsbreite
                                  Tag_Trig_Reg_Nr       <= to_integer(unsigned(Tag_n_Trigger)(9 downto 7));      -- Register-Nummer Trigger-Input
                                  Tag_Trig_Reg_Bit      <= to_integer(unsigned(Tag_n_Trigger)(6 downto 3));      -- Bit-Nummer vom Trigger-Register
                                  Tag_Trig_Pol          <= Tag_n_Trigger(10);                                    -- Bit-Polatität, 0 = neg. Flanke, 1 = pos. Flanke
                                  Tag_Timeout_Loop_Cnt  <= to_integer(unsigned(Tag_n_Trigger)(13 downto 11));    -- Anzahl der Timeoutzyklen in 10us Schritten
                                  Tag_Trig_Mux          <= Tag_n_Trigger(2 downto 0);                            -- Bit-Nummer vom Trigger-Register

                                  sm_state <= sm_trigger;
                              else
                                sm_state <= sm_idle;
                              end if;

        when sm_trigger =>
                              Tag_matched                <= '0';
                              CASE Tag_Trig_Mux is
                                WHEN "001"   =>  To_Start     <= '1';            -- Start Timeout-Counter
                                                 sm_state     <= sm_trig_S0;     -- Trigger mit Spare0
                                WHEN "010"   =>  To_Start     <= '1';            -- Start Timeout-Counter
                                                 sm_state     <= sm_trig_S1;     -- Trigger mit Spare1
                                WHEN "100"   =>  IF Tag_Trig_Reg_Nr = 0  THEN    -- Input-Trigger-Reg. nicht definiert
                                                    sm_state  <= sm_trig_err;    -- Trigger-Error
                                                 elsif Tag_Trig_Reg_Nr > Max_AWIn_Reg_Nr  THEN    -- Input-Trigger-Reg. nicht unterstützt
                                                    sm_state  <= sm_trig_max_err;    -- Trigger max Reg. Error
                                                 else
                                                    To_Start     <= '1';         -- Start Timeout-Counter
                                                    sm_state  <= sm_trig_Reg;    -- Trigger mit Bit vom Input-Register
                                                 end if;
                                WHEN OTHERS  =>  sm_state     <= sm_vz_delay;
                              END CASE;



        when sm_trig_S0  =>    To_Start        <= '0';           ------------ Stop Timeout-Counter
                               sm_state        <= sm_trig_S0_w;

        when sm_trig_S0_w =>   IF Spare0_Strobe = '1' THEN       ------------ warte auf Spare0_Input
                                 sm_state      <= sm_vz_delay_t;
                               ELSIF To_cnt_akt = '0' THEN
                                 sm_state      <= sm_to_err;
                               ELSE
                                sm_state      <= sm_trig_S0_w;
                               end if;



        when sm_trig_S1  =>   To_Start        <= '0';            ------------ Stop Timeout-Counter
                              sm_state        <= sm_trig_S1_w;

        when sm_trig_S1_w =>  IF Spare1_Strobe = '1' THEN        ------------ warte auf Spare1_Input
                                sm_state      <= sm_vz_delay_t;
                              ELSIF To_cnt_akt = '0' THEN
                                sm_state      <= sm_to_err;
                              ELSE
                                sm_state      <= sm_trig_S1_w;
                              end if;


        when sm_trig_Reg =>   To_Start        <= '0';           ------------ Stop Timeout-Counter
                              sm_state        <= sm_trig_Reg_w;

        when sm_trig_Reg_w => IF Tag_Trig_Pol  = '0' THEN                        -- Ist die TriggerPolarität negativ ?
                                Tag_Reg_Bit_Strobe_i <=      SCU_AW_Input_Reg(Tag_Trig_Reg_Nr)(Tag_Trig_Reg_Bit);  -- Trigger-Input-Bit(pos. Flanke) ==> Strobe eine Clockbreite
                              else
                                Tag_Reg_Bit_Strobe_i <= not (SCU_AW_Input_Reg(Tag_Trig_Reg_Nr)(Tag_Trig_Reg_Bit)); -- Trigger-Input-Bit(neg. Flanke) ==> Strobe eine Clockbreite
                              end if;

                              IF Tag_Reg_Bit_Strobe_o  = '1' THEN                 -- warte auf Kanal_Bit_Input
                                sm_state              <= sm_vz_delay_t;
                              ELSIF To_cnt_akt  = '0' THEN
                                sm_state       <= sm_to_err;
                              ELSE
                                sm_state       <= sm_trig_Reg_w;
                              end if;


        when sm_vz_delay_t => sm_state       <= sm_vz_delay;     ------------ Laufzeitausgleich für Trigger S0/S1/extern


        when sm_vz_delay =>   IF Tag_Delay_Cnt   =  0 THEN
                                sm_state        <= sm_rd1;
                              else   VZ_Start   <= '1';          ------------ Start VZ-Counter
                                 sm_state       <= sm_vz_wait;
                              end if;

        when sm_vz_wait   =>  VZ_Start   <= '0';                 ------------ Stop VZ-Counter
                              sm_state   <= sm_vz_wait1;


        when sm_vz_wait1  =>  IF VZ_cnt_akt  = '1' then          ------------ Wait-Loop
                                sm_state    <= sm_vz_wait1;
                              else
                                sm_state    <= sm_rd1;
                              end if;


        when sm_rd1     =>    IF    (((Tag_FG_Start  = '0') and (Tag_AWOut_Reg_Nr  = 0))  or      -- Register-Nr. gleich 0 ==> Error
                                     ((Tag_FG_Start  = '1') and (Tag_AWOut_Reg_Nr /= 0))) then    -- Register-Nr. ungleich 0 ==> Error
                                    sm_state        <= sm_reg_err;

                              elsif Tag_AWOut_Reg_Nr > Max_AWOut_Reg_Nr then                      -- Register-Nr. im unzulässigen Bereich
                                        sm_state        <= sm_reg_max_err;

                              elsif  ((Tag_FG_Start  = '1') and (Tag_AWOut_Reg_Nr  = 0)) then     -- Start FG-Trigger gesetzt
                                        sm_state        <= sm_fg_start;

                              ELSE

                                Tag_Maske_Hi_Bits   <=      Tag_n_Level and Tag_n_Maske;  -- Set Maske für "High-Aktive" Bits im Ausgangs-Register
                                Tag_Maske_Lo_Bits   <=  not Tag_n_Level and Tag_n_Maske;  -- Set Maske für "Low-Aktive"  Bits im Ausgangs-Register
                                sm_state        <= sm_wr1;
                              end if;

        when sm_wr1   =>      Tag_New_AWOut_Data  <= true;                           -- Set Flag: neue Daten für AWOut_Reg_Nr. im AWOut_Reg_Array

                              IF Tag_Puls_Width = 0 then
                                sm_state     <= sm_end;
                              else
                                  sm_state   <= sm_mf_delay;
                              end if;


        when sm_mf_delay =>   Tag_New_AWOut_Data  <= false;        -- Reset Flag: neue Daten für AWOut_Reg_Nr. im AWOut_Reg_Array

                              IF Tag_Puls_Width   =  0 THEN
                                sm_state        <= sm_rd2;
                              else   Mf_Start   <= '1';          ------------ Start Mf-Counter
                                 sm_state       <= sm_mf_wait;
                              end if;

        when sm_mf_wait   =>  Mf_Start     <= '0';               ------------ Stop Mf-Counter
                              sm_state     <= sm_mf_wait1;

        when sm_mf_wait1   => IF Mf_cnt_akt    = '1' then           ------------ Wait-Loop
                                sm_state      <= sm_mf_wait1;
                              else
                                sm_state      <= sm_rd2;
                              end if;

        when sm_rd2     =>    IF Tag_AWOut_Reg_Nr  /= 0 then  -- IF Register-Nr. im Bereich 1-7

                              --- zum "Umschalten bei der Monoflop-Funktion, werden aus den "High-Bit" = "Low-Bits" und umgekehrt.
                              Tag_Maske_Hi_Bits   <=  not Tag_n_Level and Tag_n_Maske;  -- Set Maske für "High-Aktive" Bits im Ausgangs-Register
                              Tag_Maske_Lo_Bits   <=      Tag_n_Level and Tag_n_Maske;  -- Set Maske für "Low-Aktive"  Bits im Ausgangs-Register

                              sm_state     <= sm_wr2;
                              else
                                sm_state     <= sm_reg_err;
                              end if;

        when sm_wr2     =>    Tag_New_AWOut_Data  <= true;                        -- Set Flag: neue Daten für AWOut_Reg_Nr. im AWOut_Reg_Array
                              sm_state            <= sm_end;


        when sm_fg_start     =>   Tag_n_FG_Start        <=   '1';        -- Fg-Start
                                  sm_state <= sm_end;

        when sm_reg_err      =>   Tag_n_Reg_Err         <=   '1';        -- Config-Error: TAG-Reg-Nr
                                  sm_state <= sm_end;

        when sm_reg_max_err  =>   Tag_n_Reg_max_Err     <=   '1';        -- Config-Error: TAG-Reg-Nr-Max
                                  sm_state <= sm_end;

        when sm_trig_err     =>   Tag_n_Trig_Err        <=   '1';        -- Config-Error: Trig-Reg
                                  sm_state <= sm_end;

        when sm_trig_max_err =>   Tag_n_Trig_max_Err    <=   '1';        -- Config-Error: Trig-Reg-Max
                                  sm_state <= sm_end;

        when sm_to_err       =>   Tag_n_Timeout         <=   '1';        -- Timeout-Error ext. Trigger, Spare0/Spare1
                                  sm_state <= sm_end;

        when sm_end          =>   Tag_New_AWOut_Data    <= false;        -- Reset Flag: neue Daten für AWOut_Reg_Nr. im AWOut_Reg_Array
                                  sm_state              <= sm_idle;

                                  Tag_n_FG_Start        <=   '0';        -- Fg-Start
                                  Tag_n_Reg_Err         <=   '0';        -- Config-Error: TAG-Reg-Nr
                                  Tag_n_Reg_max_Err     <=   '0';        -- Config-Error: TAG-Reg-Nr-Max
                                  Tag_n_Trig_Err        <=   '0';        -- Config-Error: Trig-Reg
                                  Tag_n_Trig_max_Err    <=   '0';        -- Config-Error: Trig-Reg-Max
                                  Tag_n_Timeout         <=   '0';        -- Timeout-Error ext. Trigger, Spare0/Spare1

      when others          =>   sm_state            <= sm_idle;
      end case;


    end if;
  end process P_Tag_Deco;



Tag_n_Maske_Hi_Bits   <=  Tag_Maske_Hi_Bits;      -- Übergabe-Daten: Maske für "High-Aktive" Bits im Ausgangs-Register
Tag_n_Maske_Lo_Bits   <=  Tag_Maske_Lo_Bits;      -- Übergabe-Daten:  Maske für "Low-Aktive"  Bits im Ausgangs-Register
Tag_n_Reg_Nr          <=  Tag_AWOut_Reg_Nr;       -- Übergabe-Daten: Nr. des Registers, das überschriebe werden soll
Tag_n_New_AWOut_Data  <=  Tag_New_AWOut_Data;     -- Übergabe-Daten: Daten, mit denen das Registers überschrieben werden soll
Tag_n_ext_Trig_Strobe <=  Tag_Reg_Bit_Strobe_o;   -- Übergabe-Daten: ext. Trigger-Eingang, aus Input-Register-Bit
Tag_n_LA              <=  (others => '0');        -- Übergabe-Daten: Anschuß für Logic-Analyse (z.Z. frei)

end Arch_tag_n;
