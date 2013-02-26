library ieee;
use ieee.std_logic_1164.all;

library work;
use work.aux_functions_pkg.div_n;

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

signal  nled_active:    std_logic;
signal  ena_modelsim: std_logic;

BEGIN

ena_modelsim <= ena and not nled_active;

P_stretch:  div_n
generic map (
            n       => stretch_cnt,
            diag_on => 0
            )
port map    (
            res     => sig_in,          -- loescht div_n; div_n zaehlt bis zum Ueberlauf
            clk     => clk,
            ena     => ena_modelsim,    -- wenn der Ueberlauf erreicht ist, wird wird div_n gestoppt.
            div_o   => nled_active
            );

nLed <= nled_active;

nLed_opdrn <= '0' when (nled_active = '0') else 'Z';

end arch_led_n;