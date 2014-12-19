--TITLE "'tag_n' Autor: R.Hartmann, Stand: 19.12.2014, Vers: V01 ";

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


ENTITY tag_n IS
  port(
    clk:                   in   std_logic;                        -- should be the same clk, used by SCU_Bus_Slave
    nReset:                in   std_logic;
    Timing_Pattern_LA:     in   std_logic_vector(31 downto 0);    -- latched timing pattern from SCU_Bus for external user functions
    Timing_Pattern_RCV:    in   std_logic;                        -- timing pattern received
    Tag_n_hw:              in   std_logic_vector(15 downto 0);    -- 
    Tag_n_lw:              in   std_logic_vector(15 downto 0);    -- 
    Tag_n_Maske:           in   std_logic_vector(15 downto 0);    -- 
    Tag_n_Lev_Reg:         in   std_logic_vector(15 downto 0);    -- 
    Tag_n_Delay_Cnt:       in   std_logic_vector(15 downto 0);    -- 
    Tag_n_Puls_Width:      in   std_logic_vector(15 downto 0);    -- 
    Tag_n_Prescale:        in   std_logic_vector(15 downto 0);    -- 
    Tag_n_Trigger:         in   std_logic_vector(15 downto 0);    -- 
    AWOut_Reg_1:           in   std_logic_vector(15 downto 0);    -- 
    AWOut_Reg_2:           in   std_logic_vector(15 downto 0);    -- 
    AWOut_Reg_3:           in   std_logic_vector(15 downto 0);    -- 
    AWOut_Reg_4:           in   std_logic_vector(15 downto 0);    -- 
    AWOut_Reg_5:           in   std_logic_vector(15 downto 0);    -- 
    AWOut_Reg_6:           in   std_logic_vector(15 downto 0);    -- 
    AWOut_Reg_7:           in   std_logic_vector(15 downto 0);    -- 
    AWIn1:                 in   std_logic_vector(15 downto 0);    -- 
    AWIn2:                 in   std_logic_vector(15 downto 0);    -- 
    AWIn3:                 in   std_logic_vector(15 downto 0);    -- 
    AWIn4:                 in   std_logic_vector(15 downto 0);    -- 
    AWIn5:                 in   std_logic_vector(15 downto 0);    -- 
    AWIn6:                 in   std_logic_vector(15 downto 0);    -- 
    AWIn7:                 in   std_logic_vector(15 downto 0);    -- 
    Max_AWOut_Reg_Nr:      in   integer range 0 to 7;           -- Maximale AWOut-Reg-Nummer der Anwendung
    Max_AWIn_Reg_Nr:       in   integer range 0 to 7;           -- Maximale AWIn-Reg-Nummer der Anwendung
    Spare0_Strobe:         in   std_logic;                        -- 
    Spare1_Strobe:         in   std_logic;                        -- 
      
    Tag_n_Reg_Nr:          out  integer range 0 to 7;            -- AWOut-Reg-Pointer
    Tag_n_New_AWOut_Data:  out  boolean;                         -- AWOut-Reg. werden mit AWOut_Reg_Array-Daten überschrieben
    Tag_n_New_Data:        out  std_logic_vector(15 downto 0);   -- Copy der AWOut-Register 
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

constant  Timeout_Trigger:    INTEGER := 1250;   -- Counter Timeout (1250 x 8ns = 10us)


TYPE   t_Word_Array     is array (0 to 7) of std_logic_vector(15 downto 0);
TYPE   t_Boolean_Array  is array (0 to 7) of boolean;

signal  Tag_Level:              std_logic;
signal  Tag_New_Data:           std_logic_vector(15 downto 0);
    
signal  Wait_Counter:           integer range 0 to 16777215;    -- 0-FFFFFF -- Wait_Counter
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


signal  AWIn_Reg_Array:     t_Word_Array;   -- Copy der AWIn-Register 
signal  AWOut_Reg_Array:    t_Word_Array;   -- Copy der AWOut-Register 
signal  Tag_New_AWOut_Data: boolean;        -- AWOut-Reg. werden mit AWOut_Reg_Array-Daten überschrieben

signal  Tag_Timeout_Cnt:    integer range 0 to 65535;  -- 0-FFFF -- Counter Timeout


signal Tag_Reg_Bit_Out:       std_logic_vector(15 downto 0); -- Zwischenspeicher
signal Tag_Reg_Bit_Strobe_i:  std_logic;        -- input  
signal Tag_Reg_Bit_Strobe_o:  std_logic;        -- Output 
signal Tag_Reg_Bit_shift:     std_logic_vector(2  downto 0); -- Shift-Reg.


type type_t is   (sm_idle,   sm_trigger, sm_trig_S0, sm_trig_S1, sm_trig_Reg,
                  sm_vz_delay, sm_vz_wait,  sm_rd1,  sm_wr1,  sm_mf_delay,  sm_mf_wait,
                  sm_rd2,  sm_wr2, sm_fg_start,
                  sm_to_err,  sm_reg_err,  sm_reg_max_err,  sm_trig_err,  sm_trig_max_err,  sm_end);
signal sm_state:  type_t := sm_idle;


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



  
P_Tag_Deco:  process (clk, nReset)

    begin
      if (nReset = '0') then
        sm_state   <= sm_idle;

        AWOut_Reg_Array        <= (others => (others => '0'));
        AWIn_Reg_Array         <= (others => (others => '0'));

        Tag_Level              <= '0';                -- clear Tag_Level
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
        Tag_New_Data           <= (others => '0');    -- clear Tag_New_Data-Register
        Tag_Timeout_Cnt        <= 0;                  -- Counter Timeout
        Tag_New_AWOut_Data     <= false;              -- clear Tag_New_AWOut_Data-Flag
        Tag_Reg_Bit_Strobe_i   <= '0';
        Wait_Counter           <= 0;                  -- 0-FFFFFF -- Wait_Counter

  ELSIF rising_edge(clk) then

-------------------------------------Copy Input- und Output-Register in Array's -------------------------------------------

        AWOut_Reg_Array(1)  <=  AWOut_Reg_1;      -- copy Daten-Reg. AWOut1
        AWOut_Reg_Array(2)  <=  AWOut_Reg_2;      -- copy Daten-Reg. AWOut2
        AWOut_Reg_Array(3)  <=  AWOut_Reg_3;      -- copy Daten-Reg. AWOut3
        AWOut_Reg_Array(4)  <=  AWOut_Reg_4;      -- copy Daten-Reg. AWOut4
        AWOut_Reg_Array(5)  <=  AWOut_Reg_5;      -- copy Daten-Reg. AWOut5
        AWOut_Reg_Array(6)  <=  AWOut_Reg_6;      -- copy Daten-Reg. AWOut6
        AWOut_Reg_Array(7)  <=  AWOut_Reg_7;      -- copy Daten-Reg. AWOut7
      
        AWIn_Reg_Array(1)    <=  AWIn1;            -- copy Input-Reg. AWIn1
        AWIn_Reg_Array(2)    <=  AWIn2;            -- copy Input-Reg. AWIn2
        AWIn_Reg_Array(3)    <=  AWIn3;            -- copy Input-Reg. AWIn3
        AWIn_Reg_Array(4)    <=  AWIn4;            -- copy Input-Reg. AWIn4
        AWIn_Reg_Array(5)    <=  AWIn5;            -- copy Input-Reg. AWIn5
        AWIn_Reg_Array(6)    <=  AWIn6;            -- copy Input-Reg. AWIn6
        AWIn_Reg_Array(7)    <=  AWIn7;            -- copy Input-Reg. AWIn7
  
---------------------------------------------------------- TAG 0 -----------------------------------------------------------
      case sm_state is
        when sm_idle  =>      if ((Timing_Pattern_RCV = '1') and (Timing_Pattern_LA(31 downto 0) = (Tag_n_hw & Tag_n_lw))) then

                                  Tag_Level             <= Tag_n_Lev_Reg(15);                                    -- Level für Output-Bits
                                  Tag_AWOut_Reg_Nr      <= to_integer(unsigned(Tag_n_Lev_Reg)(2 downto 0));      -- Output-Reg. Nr. 0..7         
                                  Tag_FG_Start          <= Tag_n_Lev_Reg(3);                                     -- Start-Flag für FG         
                                  Tag_Delay_Cnt         <= to_integer(unsigned(Tag_n_Delay_Cnt)(15 downto 0));   -- Counter Delay
                                  Tag_Puls_Width        <= to_integer(unsigned(Tag_n_Puls_Width)(15 downto 0));  -- Counter Delay
                                  Tag_Pre_VZ            <= to_integer(unsigned(Tag_n_Prescale)(15 downto 8));    -- Prescale Verzögerungszeit
                                  Tag_Pre_MF            <= to_integer(unsigned(Tag_n_Prescale)(7 downto 0));     -- Prescale Pulsbreite
                                  Tag_Trig_Reg_Nr       <= to_integer(unsigned(Tag_n_Trigger)(9 downto 7));      -- Register-Nummer Trigger-Input
                                  Tag_Trig_Reg_Bit      <= to_integer(unsigned(Tag_n_Trigger)(6 downto 3));      -- Bit-Nummer vom Trigger-Register
                                  Tag_Trig_Pol          <= Tag_n_Trigger(10);                                    -- Bit-Polatität, 0 = neg. Flanke, 1 = pos. Flanke
                                  Tag_Timeout_Loop_Cnt  <= to_integer(unsigned(Tag_n_Trigger)(13 downto 11));    -- Anzahl der Timeoutzyklen in 10us Schritten
                                  Tag_Trig_Mux          <= Tag_n_Trigger(2 downto 0);                            -- Bit-Nummer vom Trigger-Register
                          -----------------------------------------------------------------------------------------
                                  if  Tag_Timeout_Loop_Cnt > 1 then
                                    Tag_Timeout_Cnt       <= (Timeout_Trigger * Tag_Timeout_Loop_Cnt);           -- Counter Timeout
                                  else
                                    Tag_Timeout_Cnt       <=  Timeout_Trigger;                                   -- Counter Timeout
                                  end if;
                          -----------------------------------------------------------------------------------------
                                  sm_state <= sm_trigger;
                              else
                                sm_state <= sm_idle;
                              end if;

        when sm_trigger =>    CASE Tag_Trig_Mux is
                                WHEN "001"   =>  sm_state     <= sm_trig_S0;     -- Trigger mit Spare0   
                                WHEN "010"   =>  sm_state     <= sm_trig_S1;     -- Trigger mit Spare1     
                                WHEN "100"   =>  IF Tag_Trig_Reg_Nr = 0  THEN    -- Input-Trigger-Reg. nicht definiert
                                                    sm_state  <= sm_trig_err;    -- Trigger-Error 
                                                 elsif Tag_Trig_Reg_Nr > Max_AWIn_Reg_Nr  THEN    -- Input-Trigger-Reg. nicht unterstützt
                                                    sm_state  <= sm_trig_max_err;    -- Trigger max Reg. Error
                                                 else
                                                    sm_state  <= sm_trig_Reg;    -- Trigger mit Bit vom Input-Register 
                                                 end if;
                                WHEN OTHERS  =>  sm_state     <= sm_vz_delay;
                              END CASE;

                            
        when sm_trig_S0  =>   IF Spare0_Strobe = '1' THEN                        -- warte auf Spare0_Input
                                sm_state              <= sm_vz_delay;
                              ELSIF Tag_Timeout_Cnt    = 0 THEN
                                sm_state              <= sm_to_err;
                              ELSE   Tag_Timeout_Cnt  <= (Tag_Timeout_Cnt - 1);
                                sm_state              <= sm_trig_S0;
                              end if;
  
  
        when sm_trig_S1  =>   IF Spare1_Strobe = '1' THEN                        -- warte auf Spare1_Input
                                sm_state             <= sm_vz_delay;
                              ELSIF Tag_Timeout_Cnt   = 0 THEN
                                sm_state             <= sm_to_err;
                              ELSE   Tag_Timeout_Cnt <= (Tag_Timeout_Cnt - 1);
                                sm_state             <= sm_trig_S1;
                              end if;
  
  
        when sm_trig_Reg =>   IF Tag_Trig_Pol  = '0' THEN                        -- Ist die TriggerPolarität negativ ?
                                Tag_Reg_Bit_Strobe_i <=      AWIn_Reg_Array(Tag_Trig_Reg_Nr)(Tag_Trig_Reg_Bit);  -- Trigger-Input-Bit(pos. Flanke) ==> Strobe eine Clockbreite
                              else
                                Tag_Reg_Bit_Strobe_i <= not (AWIn_Reg_Array(Tag_Trig_Reg_Nr)(Tag_Trig_Reg_Bit)); -- Trigger-Input-Bit(neg. Flanke) ==> Strobe eine Clockbreite
                              end if;

                              IF Tag_Reg_Bit_Strobe_o  = '1' THEN                 -- warte auf Kanal_Bit_Input
                                sm_state              <= sm_vz_delay;
                              ELSIF Tag_Timeout_Cnt    = 0 THEN
                                sm_state              <= sm_to_err;
                              ELSE   Tag_Timeout_Cnt  <= (Tag_Timeout_Cnt - 1);
                                sm_state              <= sm_trig_Reg;
                              end if;
  
       
        when sm_vz_delay =>   IF Tag_Pre_VZ > 1 THEN
                                Wait_Counter  <=  (Tag_Delay_Cnt * Tag_Pre_VZ);   -- bei Tag_Pre_VZ größer 1
                                sm_state      <= sm_vz_wait;
                              else
                                Wait_Counter  <=  (Tag_Delay_Cnt);                -- bei Tag_Pre_VZ = 0
                                sm_state      <= sm_vz_wait;
                              end if;

        when sm_vz_wait   =>  IF Wait_Counter > 0 then                            -- Wait-Loop
                                Wait_Counter <=  (Wait_Counter -1);       
                                sm_state     <= sm_vz_wait;
                              else
                                sm_state     <= sm_rd1;
                              end if;

        when sm_rd1     =>    IF    ((Tag_FG_Start  = '1') and (Tag_AWOut_Reg_Nr  = 0)) then   -- Start FG-Trigger gesetzt
                                sm_state        <= sm_fg_start;
                              ELSIF ((Tag_FG_Start  = '1') and (Tag_AWOut_Reg_Nr /= 0)) then   -- Register-Nr. ungleich 0 ==> Error
                                sm_state        <= sm_reg_err;
                              ELSIF ((Tag_FG_Start  = '0') and (Tag_AWOut_Reg_Nr  = 0)) then   -- Register-Nr. gleich 0 ==> Error
                                sm_state        <= sm_reg_err;
                              ELSIF Tag_AWOut_Reg_Nr > Max_AWOut_Reg_Nr then                   -- Register-Nr. im unzulässigen Bereich
                                sm_state        <= sm_reg_max_err;
                              ELSE
                                IF Tag_Level     = '0' then                       -- Register-Nr. = ok 
                                Tag_New_Data  <=  (AWOut_Reg_Array(Tag_AWOut_Reg_Nr) and (not Tag_n_Maske));  -- die "1"-Bits der Maske, werden auf 0 gesetzt
                                else
                                  Tag_New_Data  <=  (AWOut_Reg_Array(Tag_AWOut_Reg_Nr) or Tag_n_Maske);         -- aie "1"-Bits der Maske, werden auf 1 gesetzt
                                end if; 
                                  sm_state        <= sm_wr1;
                              end if; 
                              
        when sm_wr1   =>      AWOut_Reg_Array(Tag_AWOut_Reg_Nr)     <= Tag_New_Data; -- neue Daten nach "Bit-Manipulation" zurückschreiben
                              Tag_New_AWOut_Data  <= true;                           -- Set Flag: neue Daten für AWOut_Reg_Nr. im AWOut_Reg_Array
 
                              IF Tag_Puls_Width = 0 then
                                sm_state     <= sm_end;
                              else
                                  sm_state   <= sm_mf_delay;
                              end if;

    
        when sm_mf_delay =>   Tag_New_AWOut_Data  <= false;                          -- Reset Flag: neue Daten für AWOut_Reg_Nr. im AWOut_Reg_Array

                              IF Tag_Pre_MF   > 1 THEN                               -- bei Tag_Pre_MF größer 1
                                Wait_Counter  <=  (Tag_Puls_Width * Tag_Pre_MF);
                                sm_state      <=  sm_mf_wait;
                              else
                                Wait_Counter  <=  (Tag_Puls_Width);                  -- bei Tag_Pre_MF = 0
                                sm_state      <=  sm_mf_wait;
                              end if;

        when sm_mf_wait   =>  IF Wait_Counter > 0 then
                                Wait_Counter  <=  (Wait_Counter -1);       
                                sm_state      <= sm_mf_wait;
                              else
                                  sm_state    <= sm_rd2;
                              end if;

  
        when sm_rd2     =>    IF Tag_AWOut_Reg_Nr  /= 0 then  -- IF Register-Nr. im Bereich 1-7 

                                IF Tag_Level      = not '0' then -- das inverse von rd1/wr1 (zum ausschalten)
                                  Tag_New_Data  <= (AWOut_Reg_Array(Tag_AWOut_Reg_Nr) and (not Tag_n_Maske));  -- die "1"-Bits der Maske, werden auf 0 gesetzt
                                else
                                    Tag_New_Data  <= (AWOut_Reg_Array(Tag_AWOut_Reg_Nr) or Tag_n_Maske);         -- aie "1"-Bits der Maske, werden auf 1 gesetzt
                                end if;   
                              sm_state     <= sm_wr2;
                              else
                                sm_state     <= sm_reg_err;
                              end if; 
                              
        when sm_wr2     =>    AWOut_Reg_Array(Tag_AWOut_Reg_Nr) <= Tag_New_Data;  -- neue Daten nach "Bit-Manipulation" zurückschreiben
                              Tag_New_AWOut_Data  <= true;                        -- Set Flag: neue Daten für AWOut_Reg_Nr. im AWOut_Reg_Array
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
  

Tag_n_New_Data        <=  Tag_New_Data;           -- Übergabe-Daten: Flag, das Register mit der Nummer (Tag_AWOut_Reg_Nr) soll überschrieben werden
Tag_n_Reg_Nr          <=  Tag_AWOut_Reg_Nr;       -- Übergabe-Daten: Nr. des Registers, das überschriebe werden soll
Tag_n_New_AWOut_Data  <=  Tag_New_AWOut_Data;     -- Übergabe-Daten: Daten, mit denen das Registers überschrieben werden soll
Tag_n_ext_Trig_Strobe <=  Tag_Reg_Bit_Strobe_o;   -- Übergabe-Daten: ext. Trigger-Eingang, aus Input-Register-Bit
Tag_n_LA              <=  (others => '0');        -- Übergabe-Daten: Anschuß für Logic-Analyse (z.Z. frei)

end Arch_tag_n;