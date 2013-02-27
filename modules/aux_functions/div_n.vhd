
-- TITLE "'div_n' => Autor: W. Panschow, V01, Stand: 26.10.2012";

library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.numeric_std.all;
use ieee.math_real.all;

--+-----------------------------------------------------------------------------------------------------------------+
--| div_n soll nach 'n-1' Takten den Ausgang 'div_o' für eine Taktperiode akiv eins setzen.                         |
--|                                                                                                                 |
--| Die Funktion ist mit einem Abwärtszähler realisiert worden. Dadurch werden zwei Nachteile gegeüber einer Lösung |
--| mit Aufwärtszähler vermieden.                                                                                   |
--|                                                                                                                 |
--| Die Nachteile des Aufwärtszählers:                                                                              |
--|  1) Zustätzlich zum eigentlichen Zähler braucht es noch einen Komparator, der das Erreichen des gewünschten     |
--|     Untersetzungswerts abfragt. Bei größeren Zählerbreiten wird der Komparator mehrere Logoikgatter tief        |
--|     (für 8 Bit werden bei einer Vierer-LUT schon zwei Ebenen gebraucht).                                        |
--|  2) Der Komperator produziert Glitches die mit einem weiteren Register entfernt werden müssen. Das vom          |
--|     Komparator zu testende Untersetzungverhältnis muesste deshalb um eins verringert werden.                    |
--|                                                                                                                 |
--| Der Abwärtszähler muss auch ein Bit breiter definiert werden, als eigentlich zur Darstellung des                |
--| Untersetzungswerts benötigt wird. Dafür liegt mit dem 'Unterlaufbit' dem höchsten Bit des Zählers ein getaktetes|
--| (glitchfreies) Signal vor. Der Zähler braucht keinen Komparator! Das gewünsche Untersetzungsverhältnis 'n' muss |
--| mit -2 korrigiert werden, da der Zähler zusätzlich die Null erreichen muss und einen weiteren Takt braucht, um  |
--| das 'Unterlauf'-Bit zu setzen.                                                                                  |
--+-----------------------------------------------------------------------------------------------------------------+

entity div_n is

generic
    (
    n:          integer range 2 to integer'high := 2;   -- Vorgabe der Taktuntersetzung. n = 2 ist das Minimum.
    diag_on:    integer range 0 to 1 := 0               -- diag_on = 1 die Breite des Untersetzungzaehlers
                                                        -- mit assert .. note ausgegeben.
    );

port
    (
    res:    in      std_logic := '0';
    clk:    in      std_logic;
    ena:    in      std_logic := '1';   -- das untersetzende enable muss in der gleichen Clockdomäne erzeugt werden.
                                        -- Das enable sollte nur ein Takt lang sein.
                                        -- Z.B. könnte eine weitere div_n-Instanz dieses Signal erzeugen.  
    div_o:  out     std_logic           -- Wird nach Erreichen von n-1 fuer einen Takt aktiv.
    );

constant  c_div_n_width:  integer := integer(ceil(log2(real(n-1))));  -- Anzahl der Bits um 'n' binär darzustellen.
                                                                      -- Aufrunden mit (ceil)!
                                                                      -- Da der Zähler mit (n - 2) geladen wird 
                                                                      -- (s. c_load_val) reicht (n-1) für die
                                                                      -- Breitenberechnung.

signal    s_div_n:  unsigned(c_div_n_width DOWNTO 0); -- Der Zähler s_div_n muss um ein bit größer definiert werden.
                                                      -- Dadurch ist das oberste Bit des Zählers nach laden des
                                                      -- Teilungswertes c_load_val sicher '0'. Die um ein Bit
                                                      -- größere Breite wird automatisch durch den
                                                      -- Range (xxx downto 0) erreicht.

constant  c_load_val: unsigned(c_div_n_width DOWNTO 0):= to_unsigned(n - 2, c_div_n_width+1); -- Zähler arbeitet als
                                                                        -- Down-Counter. Der Underflow soll das  
                                                                        -- erreichen des Counts (n) signalisieren.
                                                                        -- Dafür werden zwei Takte (counts) mehr 
                                                                        -- gebraucht als im Generic (n) vereinbart.
                                                                        -- Der Zähler erreicht noch die Null und
                                                                        -- nach dem nächsten Takt erst den Unterlauf 
                                                                        -- (das oberste Bit wird eins), deshalb n - 2.
end div_n;


architecture arch_div_n of div_n is

begin

ASSERT NOT(n < 2 )
    REPORT " Produktions-Count muss >= 2 sein"
SEVERITY ERROR;

ASSERT not(diag_on = 1)
    REPORT " constante: " & integer'image(n) &  " load_val: " & integer'image(to_integer(c_load_val))
            & "  width: " & integer'image(c_div_n_width) & "  length:  " & integer'image(s_div_n'length)
SEVERITY note;


p_div_n:    process (clk, Res)
  begin
    if res = '1' THEN
      s_div_n <= c_load_val;
    elsif   rising_edge(clk) THEN
      if ena = '1' then
        if s_div_n(s_div_n'high) = '1' THEN   -- Bei underflow wird neu geladen   --
          s_div_n <= c_load_val;
        else
          s_div_n <= s_div_n - 1;             -- subtrahieren.                    --
        end if;
      end if;
    end if;
  end process p_div_n;

div_o <= s_div_n(s_div_n'high);

end arch_div_n;

