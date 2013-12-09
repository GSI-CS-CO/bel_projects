library ieee;
use ieee.std_logic_1164.all;
use IEEE.numeric_std.all;
use ieee.math_real.all;


--+-----------------------------------------------------------------------------------------------------------------+
--| "led_n",    Autor: W.Panschow                                                                                   |
--|                                                                                                                 |
--| "led_n" provides puls streching of the signal "Sig_In" to the active zero outputs "nLED" and "nLED_opdrn"       |
--| "Sig_In"  = '1' resets puls streching counter asynchron. If "Sig_In" change to '0' counter hold the outputs     |
--| "nLED" and "nLED_opdrn" at active zero, while the "stretch_cnt" isn't reached.                                  |
--+-----------------------------------------------------------------------------------------------------------------+ 

entity led_n is
generic
    (
    stretch_cnt:    integer := 3
    ); 
port
    (
    ena:        in  std_logic;  -- if you use ena for a reduction, signal should be generated from the same 
                                -- clock domain and should be only one clock period active.
    CLK:        in  std_logic;  -- clk = clock.
    Sig_In:     in  std_logic;  -- '1' holds "nLED" and "nLED_opdrn" on active zero. "Sig_in" changeing to '0' 
                                -- "nLED" and "nLED_opdrn" change to inactive State after stretch_cnt clock periodes.
    nLED:       out std_logic;  -- Push-Pull output, active low, inactive high.
    nLed_opdrn: out std_logic   -- open drain output, active low, inactive tristate.
    );
end led_n;

ARCHITECTURE arch_led_n OF led_n IS 

constant  c_cnt_width:  integer := integer(ceil(log2(real(stretch_cnt-1))));  -- Anzahl der Bits um 'stretch_cnt' binär darzustellen.
                                                                              -- Aufrunden mit (ceil)!
                                                                              -- Da der Zähler mit (stretch_cnt - 2) geladen wird 
                                                                              -- (s. c_load_val) reicht (stretch_cnt-1) für die
                                                                              -- Breitenberechnung.

signal    cnt:  unsigned(c_cnt_width DOWNTO 0); -- Der Zähler cnt muss um ein bit größer definiert werden.
                                                -- Dadurch ist das oberste Bit des Zählers nach laden des
                                                -- Teilungswertes c_load_val sicher '0'. Die um ein Bit
                                                -- größere Breite wird automatisch durch den
                                                -- Range (xxx downto 0) erreicht.

constant  c_load_val: unsigned(c_cnt_width DOWNTO 0):= to_unsigned(stretch_cnt - 2, c_cnt_width+1); -- Zähler arbeitet als
                                                                        -- Down-Counter. Der Underflow soll das  
                                                                        -- erreichen des Counts (stretch_cnt) signalisieren.
                                                                        -- Dafür werden zwei Takte (counts) mehr 
                                                                        -- gebraucht als im Generic (stretch_cnt) vereinbart.
                                                                        -- Der Zähler erreicht noch die Null und
                                                                        -- nach dem nächsten Takt erst den Unterlauf 
                                                                        -- (das oberste Bit wird eins), deshalb stretch_cnt - 2.


BEGIN

ASSERT NOT(stretch_cnt < 2 )
    REPORT " Produktions-Count muss >= 2 sein"
SEVERITY ERROR;


p_stretch:  process (clk, sig_in)
  begin
    if sig_in = '1' then
      cnt <= c_load_val;
    elsif rising_edge(clk) then
      if ena = '1' and cnt(cnt'high) = '0' then
        cnt <= cnt - 1;
      end if;
    end if;
  end process p_stretch;

nLed <= cnt(cnt'high);

nLed_opdrn <= '0' when (cnt(cnt'high)) else 'Z';

end arch_led_n;