--TITLE "'flanke' Autor: R.Hartmann, Stand: 23.03.2015";
--

library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;

ENTITY flanke IS
  port
    (
    nReset:       in   std_logic;
    clk:          in   std_logic;     -- should be the same clk, used by SCU_Bus_Slave
    Sign_in:      in   std_logic;     -- Input Signal
    Pegel:        in   std_logic;     -- 0=pos. 1=neg. Flanke
    Strobe_out:   out  std_logic      -- Output-Strobe
    );  
  end flanke;


ARCHITECTURE arch_flanke OF flanke IS


  signal Flanke_Out:       std_logic_vector(15 downto 0); -- Zwischenspeicher
  signal Flanke_Strobe_o:  std_logic;                     -- Output "Start-Puls für den Stobe vom DAC (1 CLK breit)"
  signal Flanke_shift:     std_logic_vector(2  downto 0); -- Shift-Reg.


begin

 
---------  Flankendetektor als Signal (1 Clock breit) --------------------

p_Flanke_Strobe_Start:  PROCESS (clk, nReset, Sign_in)
  BEGin
      if (nReset = '0') then
      Flanke_shift  <= (OTHERS => '0');
      Flanke_Strobe_o    <= '0';

    ELSIF rising_edge(clk) THEN
      Flanke_shift <= (Flanke_shift(Flanke_shift'high-1 downto 0) & (Sign_in xor Pegel));

      IF Flanke_shift(Flanke_shift'high) = '0' AND Flanke_shift(Flanke_shift'high-1) = '1' THEN
        Flanke_Strobe_o <= '1';
      ELSE
        Flanke_Strobe_o <= '0';
      END IF;
    END IF;
  END PROCESS p_Flanke_Strobe_Start;
  

Strobe_out  <=  Flanke_Strobe_o; 
 

end arch_flanke;  
