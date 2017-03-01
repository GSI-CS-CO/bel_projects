--TITLE "'atr_puls_n' Autor: R.Hartmann";

library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;

ENTITY atr_puls_n IS
		
	port(
    clk:                  IN  STD_LOGIC;                      -- 250 MHz
    nReset:               IN  STD_LOGIC;                      -- 
--
    atr_puls_start:       IN  STD_LOGIC;                      -- Starte Ausgangspuls (Puls = 4ns)
		ATR_verz:             IN  std_logic_VECTOR(15 DOWNTO 0);  -- Counter für Verzögerung   (4ns Schritte)
		ATR_pulsw:            IN  std_logic_VECTOR(15 DOWNTO 0);  -- Counter für Pulsbreite  (100ns Schritte)
--
    atr_puls_out:         out STD_LOGIC;                      -- Ausgangspuls Kanal n
    atr_puls_config_err:  out STD_LOGIC                       -- Config-Error: Pulsbreite/Pulsverzögerung
		);	
	end atr_puls_n;

	

ARCHITECTURE Arch_atr_puls_n OF atr_puls_n IS

  --------------------------------- Verzögerungszeit ---------------------------------------

signal  Vz_Cnt_Pre:      integer range 0 to 255;     -- Verzögerungszeit: Counter-Prescale
signal  Vz_Cnt:          integer range 0 to 65535;   -- Verzögerungszeit: Counter
signal  Vz_cnt_aktiv:    std_logic;                  -- Verzögerungszeit: Counter=aktiv
signal  Vz_Start:        std_logic;                  -- Verzögerungszeit: Start_Counter

  ----------------------------------- Pulsbreite  ------------------------------------------

signal  Pb_Cnt_Pre:      integer range 0 to 255;     -- Pulsbreite: Counter-Prescale
signal  Pb_Cnt:          integer range 0 to 65535;   -- Pulsbreite: Counter
signal  Pb_cnt_aktiv:    std_logic;                  -- Pulsbreite: Counter=aktiv
signal  Pb_Start:        std_logic;                  -- Pulsbreite: Start_Counter
        
        
signal  Puls_Vz_Cnt:     integer range 0 to 65535;    -- 0-FFFF -- Counter Verzögerung
signal  Puls_Pb_Cnt:     integer range 0 to 65535;    -- 0-FFFF -- Counter Pulsbreite
signal  Puls_Pre_VZ:     integer range 0 to 65535;    -- 0-FFFF -- Prescale Verzögerungszeit
signal  Puls_Pre_Pb:     integer range 0 to 65535;    -- 0-FFFF -- Prescale Pulsbreite

signal  s_atr_puls_out:         std_logic;           -- Ausgangspuls Kanal n
signal  s_atr_puls_config_err:  std_logic;           -- Config-Error: Pulsbreite/Pulsverzögerung



type type_t is   ( sm_idle,
                   sm_vz_start, sm_vz_wait, sm_vz_wait1,
                   sm_laufz1, sm_laufz2,
                   sm_pb_start, sm_pb_wait, sm_pb_wait1,                    
                   sm_end );
                  
signal sm_state:  type_t := sm_idle;



begin


  ---------------------------------------- Verzögerungszeit -------------------------------------------

P_Vz:  process (clk, nReset, Vz_Cnt_Pre, Puls_Vz_Cnt, VZ_Start)

    begin
      if (nReset = '0') then
        Vz_Cnt_Pre    <=  0 ;   -- Verzögerungszeit_Counter-Prescale
        Vz_Cnt        <=  0 ;   -- Verzögerungszeit_Counter
        Vz_cnt_aktiv  <= '0';   -- Verzögerungszeit_Gate

      ELSIF rising_edge(clk) then

          if (Vz_Start = '1') then
              Vz_Cnt_Pre      <= Puls_Pre_VZ;    -- Verzögerungszeit_Counter-Prescale          
              Vz_Cnt          <= Puls_Vz_Cnt;    -- Verzögerungszeit_Counter
              Vz_cnt_aktiv    <= '1';            -- Counter aktiv (ungleich 0)

          elsif (Vz_Cnt  > 0) then
              Vz_Cnt    <=  Vz_Cnt-1;            -- Counter -1
          else
            if (Vz_Cnt_Pre  > 1) then
              Vz_Cnt_Pre    <= Vz_Cnt_Pre-1;    -- Counter -1

              if Vz_Cnt = 0 then
                 Vz_Cnt        <= Puls_Vz_Cnt;      -- Pulsbreite_Counter (keine Fehlerkorrektur bei Pb_Cnt=0)
              else
                 Vz_Cnt        <= Puls_Vz_Cnt-1;    -- Pulsbreite_Counter (-1 ist Fehlerkorrektur)
              end if;

            else
              Vz_cnt_aktiv   <= '0';
            end if;
      end if;
    end if;
  end process P_Vz;
  
  
  ---------------------------------------- Pulsbreite  -------------------------------------------

P_Pb:  process (clk, nReset, Pb_Cnt_Pre, Puls_Pb_Cnt, Pb_Start)

    begin
      if (nReset = '0') then
        Pb_Cnt_Pre    <=  0 ;   -- Pulsbreite_Counter-Prescale
        Pb_Cnt        <=  0 ;   -- Pulsbreite_Counter
        Pb_cnt_aktiv  <= '0';   -- Pulsbreite_Gate

      ELSIF rising_edge(clk) then

          if (Pb_Start = '1') then
              Pb_Cnt_Pre      <= Puls_Pre_Pb;    -- Pulsbreite_Counter-Prescale          
              Pb_Cnt          <= Puls_Pb_Cnt;    -- Pulsbreite_Counter
              Pb_cnt_aktiv    <= '1';            -- Counter aktiv (ungleich 0)

          elsif (Pb_Cnt  > 0) then
              Pb_Cnt    <=  Pb_Cnt-1;            -- Counter -1
          else
            if (Pb_Cnt_Pre  > 1) then
              Pb_Cnt_Pre    <= Pb_Cnt_Pre-1;    -- Counter -1

              if Pb_Cnt = 0 then
                 Pb_Cnt        <= Puls_Pb_Cnt;      -- Pulsbreite_Counter (keine Fehlerkorrektur bei Pb_Cnt=0)
              else
                 Pb_Cnt        <= Puls_Pb_Cnt-1;    -- Pulsbreite_Counter (-1 ist Fehlerkorrektur)
              end if;

            else
              Pb_cnt_aktiv   <= '0';
            end if;
      end if;
    end if;
  end process P_Pb;
  

  
  
P_Puls_SM:  process (clk, nReset,  sm_state,
                      Puls_Vz_Cnt,  Puls_Pre_VZ, Puls_Pb_Cnt, Puls_Pre_Pb,
                      Vz_cnt_aktiv, Pb_cnt_aktiv)
                      
    begin
      if (nReset = '0') then

          Puls_Vz_Cnt             <=  0;       -- Counter  Verzögerungszeit
          Puls_Pre_VZ             <=  0;       -- Prescale Verzögerungszeit (4ns Schritte)
          Puls_Pb_Cnt             <=  0;       -- Pulsweite
          Puls_Pre_Pb             <=  0;       -- Prescale Pulsweite (25x4ns=100ns Schritte)
          s_atr_puls_out          <= '0';      -- Ausgangspuls Kanal n
          s_atr_puls_config_err   <= '0';      -- Config-Error: Pulsbreite/Pulsverzögerung
          sm_state                <= sm_idle;
        
        
        
  ELSIF rising_edge(clk) then

     
---------------------------------------------------------- Puls 'n' ---------------------------------------------------------
      case sm_state is
        when sm_idle    =>    if atr_puls_start = '1' then

                                Puls_Vz_Cnt     <= to_integer(unsigned(ATR_verz)(15 downto 0));   -- Counter  Verzögerungszeit
                                Puls_Pre_VZ     <= 1;                                             -- Prescale Verzögerungszeit (1x Clk)
                                Puls_Pb_Cnt     <= to_integer(unsigned(ATR_pulsw)(15 downto 0));  -- Pulsweite
                                Puls_Pre_Pb     <= 1;                                             -- Prescale Pulsweite (1x Clk)
--                              Puls_Pre_Pb     <= 25;                                            -- Prescale Pulsweite (25x4ns=100ns Schritte)
                                s_atr_puls_out  <= '0';                                           -- Ausgangspuls Kanal n
                                sm_state        <= sm_vz_start;
                              end if;

                              
                --============== Verzögerungszeit ==============             
               
        when sm_vz_start  =>  IF Puls_Vz_Cnt     =  0  THEN 
                                sm_state        <=  sm_laufz1;
                              else
                                Puls_Vz_Cnt     <=  (Puls_Vz_Cnt -1);   -- 
                                VZ_Start        <= '1';                 -- Start VZ-Counter
                                sm_state        <=  sm_vz_wait;   
                              end if;   
      
        when sm_vz_wait   =>  VZ_Start          <= '0';                 -- Stop VZ-Counter
                              sm_state          <= sm_vz_wait1;   
      
        when sm_vz_wait1  =>  IF VZ_cnt_aktiv    = '1' then             -- Wait-Loop
                                sm_state        <= sm_vz_wait1;
                              else
                                sm_state        <= sm_pb_start;
                              end if;

           --============== Laufzeitausgleich bei VZ = 0  ==============             

        when sm_laufz1    =>  sm_state          <= sm_laufz2;   
        when sm_laufz2    =>  sm_state          <= sm_pb_start;   

                --============== Pulsbreite ==============             

        when sm_pb_start  =>  IF Puls_pb_Cnt     =  0  THEN 
                                sm_state        <=  sm_end;
                              else
                                Puls_Pb_Cnt     <=  (Puls_Pb_Cnt -1);   -- 
                                Pb_Start        <= '1';                 -- Start Pw-Counter
                                s_atr_puls_out  <= '1';                 -- Ausgangspuls Kanal n
                                sm_state        <=  sm_pb_wait;   
                              end if;   
    
        when sm_pb_wait   =>  Pb_Start          <= '0';                 -- Stop Pw-Counter
                              sm_state          <= sm_pb_wait1;   
    
        when sm_pb_wait1  =>  IF Pb_cnt_aktiv    = '1' then             -- Wait-Loop
                                sm_state        <= sm_pb_wait1;   
                              else          
                                sm_state        <= sm_end;    
                              end if;   
      
               --============== Ende ==============                 
    
        when sm_end       =>  s_atr_puls_out    <= '0';                 -- Ausgangspuls Kanal n
                              sm_state          <= sm_idle;
            
       when others        =>  sm_state          <= sm_idle;
      end case;


    end if;
  end process P_Puls_SM;



-- atr_puls_out          <= s_atr_puls_out;        -- Ausgangspuls Kanal n
   atr_puls_out          <= Pb_cnt_aktiv;          -- genauere Zeit für die Pulsbreite (Ausgangspuls Kanal n)
-- atr_puls_config_err   <= s_atr_puls_config_err; -- Config-Error: Pulsbreite/Pulsverzögerung
   atr_puls_config_err   <= '0';                   -- z.Z kein Fehler möglich


end Arch_atr_puls_n;
