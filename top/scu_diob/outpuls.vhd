--TITLE "'outpuls' Autor: R.Hartmann, Stand: 20.03.2015";
--

library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;

ENTITY outpuls IS
  port
    (
    nReset:       in   std_logic;
    clk:          in   std_logic;                 -- should be the same clk, used by SCU_Bus_Slave
    Start:        in   std_logic;                 -- Input Signal
    Base_cnt:     in   Integer range 0 to 255;    -- Basis_Verzögerungszeit in Clocks für 1us
    Mult_cnt:     in   Integer range 0 to 65535;  -- Multiplikator für die Basis_Verzögerungszeit 
    Sign_Out:     out  std_logic                  -- Output-Strobe
    );  
  end outpuls;


ARCHITECTURE arch_outpuls OF outpuls IS

signal  s_Base_cnt:     integer range 0 to 255;     -- Basis_Verzögerungszeit in Clocks für 1us
signal  s_Mult_cnt:     integer range 0 to 65535;   -- Multiplikator für die Basis_Verzögerungszeit 
signal  s_cnt_akt:      std_logic;                  -- Verzögerungszeit: Counter=aktiv
signal  s_Sign_Out:     std_logic;                  -- Verzögerungszeit: Counter=aktiv


begin

  ---------------------------------------- Pulsbreite -------------------------------------------

P_Vz:  process (clk, nReset, Start, Base_cnt, Mult_cnt)

    begin
      if (nReset = '0') then
        s_Base_cnt    <=  0 ;   -- Basis_Verzögerungszeit in Clocks für 1us
        s_Mult_cnt    <=  0 ;   -- Multiplikator für die Basis_Verzögerungszeit
        s_Sign_Out    <= '0';   -- Output-Signal

      ELSIF rising_edge(clk) then

          if (Start = '1') then
              s_Mult_cnt      <= Mult_cnt;  -- Multiplikator für die Basis_Verzögerungszeit          
              s_Base_cnt      <= Base_cnt;  -- Basis_Verzögerungszeit in Clocks für 1us
              s_Sign_Out      <= '1';       -- Counter aktiv (ungleich 0)

          elsif (s_Base_cnt  > 0) then
              s_Base_cnt    <=  s_Base_cnt-1;            -- Counter -1
          else
            if (s_Mult_cnt  > 1) then
              s_Mult_cnt    <=  s_Mult_cnt-1;  -- Multiplikator_Counter (s_Mult_cnt) -1
              s_Base_cnt    <=  Base_cnt;      -- Basis_Verzögerungszeit in Clocks für 1us
            else
              s_Sign_Out   <= '0';
            end if;
      end if;
    end if;
  end process P_Vz;
  

Sign_Out  <=  s_Sign_Out; 
 

end arch_outpuls;  
