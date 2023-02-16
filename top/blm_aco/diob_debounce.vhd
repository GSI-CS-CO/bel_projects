library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.numeric_std.all;
use ieee.math_real.all;

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

entity diob_debounce is
generic
    (
    DB_Tst_Cnt: integer := 3;
    Test:       integer range 0 TO 1 := 0
    );
port(
    DB_Cnt:     in  integer range 0 to 16383 := 3;   
    DB_In:      in  std_logic;
    Reset:      in  std_logic;
    Clk:        in  std_logic;
    DB_Out:     out std_logic
    );
end diob_debounce;

architecture Arch_diob_debounce OF diob_debounce is
    
SIGNAL      S_DB_In_Sync:   std_logic;
--SIGNAL      S_DB_Cnt:       unsigned(DB_Cnt-1 DOWNTO 0);
--SIGNAL      S_DB_Cnt:       integer range 0 to 131072;
SIGNAL      S_DB_Cnt:       integer range 0 to 16383;
SIGNAL      S_DB_Out:       std_logic;
--SIGNAL      S_DB_On_Cnt:    unsigned(DB_Cnt'range);
--SIGNAL      S_DB_On_Cnt:    integer range 0 to 131072;
SIGNAL      S_DB_On_Cnt:    integer range 0 to 16383;
SIGNAL      S_DB_Cnt_Ena:   std_logic;

begin

assert not(DB_Cnt < 3 AND Test = 0)
    report " Produktions-Count muss >= 3 sein"
severity ERROR;

assert not(DB_Tst_Cnt < 3 AND Test = 1)
    report " Test-Count muss >= 3 sein"
severity ERROR;

assert not((DB_Cnt < DB_Tst_Cnt) AND TEST = 1)
    report " Produktions-Count muss >= Test_Count sein!"
severity ERROR;

--assert false
--    report " debounce width is " & integer'image(S_DB_Cnt'length) & " Bit(s)"
--severity note;


--S_DB_On_Cnt <= to_unsigned(DB_Cnt, S_DB_Cnt'length) when (Test = 0) else to_unsigned(DB_Tst_Cnt, S_DB_Cnt'length);
S_DB_On_Cnt <= DB_Cnt when (Test = 0) else DB_Tst_Cnt;


P_Debounce: process (Reset, Clk) 
  begin
    if Reset = '1' then
      S_DB_In_Sync <= '0';
--      S_DB_Cnt <= (Others => '0');
      S_DB_Cnt <=  0;
      S_DB_Out <= '0';
    elsif rising_edge(clk) then
      S_DB_In_Sync <= DB_In;
      if S_DB_In_Sync = '1' then
        if S_DB_Cnt < S_DB_On_Cnt then
          S_DB_Cnt <= S_DB_Cnt + 1;
        else
          S_DB_Out <= '1';
        end if;
      else
        if S_DB_Cnt > 0 then
          S_DB_Cnt <= S_DB_Cnt - 1;
        else
          S_DB_Out <= '0';
        end if;
      end if;
    end if;
  end process P_Debounce;

DB_Out <= S_DB_Out;

end Arch_diob_debounce;
