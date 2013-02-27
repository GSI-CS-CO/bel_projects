
LIBRARY ieee;
use ieee.std_logic_1164.all;
use IEEE.numeric_std.all;

library work;
use work.aux_functions_pkg.led_n;

--+-----------------------------------------------------------------------------------------------------------------+
--| "lemo_io",    Autor: W.Panschow                                                                                 |
--|                                                                                                                 |
--| "lemon_io" ist fuer einen Icoupler-Baustein geschrieben, der in seiner Richtung umgeschaltet werden kann.       |
--| Die Schaltung ist zur Zeit im "FG900111_SCU2" und "FG900151_SCU_SIO2" und weiteren Projekten implementiert,     |
--| deshalb macht es Sinn, das Interface als allgemeine Funktion bereitzustellen.                                   |
--+-----------------------------------------------------------------------------------------------------------------+
ENTITY lemo_io IS
generic
    (
    stretch_cnt:    integer := 3    -- Anzahl der Taktperioden von led_clk, um die die activity_led_n nach 
                                    -- einer Aktivität (Flanke) an lemo_io gestretcht wird.
    );
port
    (
    reset:                  in      std_logic;
    clk:                    in      std_logic;
    led_clk:                in      std_logic;
    lemo_io_is_output:      in      std_logic;  -- '0' => lemo_io ist Eingang; '1' => lemo_io ist Ausgang
    stretch_led_off:        in      std_logic;  -- '0' => activity_led_n ist gestretcht;
                                                -- '1' => activity_led_n nicht gestretcht
    to_lemo:                in      std_logic;  -- wenn lemo_io ein Ausgang ist, wird das Signal to_lemo ausgegeben
    lemo_io:                inout   std_logic;  -- hier ist der ICoupler angeschlossen
    lemo_en_in:             out     std_logic;  -- '1' => lemo_io ist Eingang; '0' => lemo_io ist Ausgang
    activity_led_n:         out     std_logic;  -- für Aktivitätsanzeige von lemo_io vorgesehen (push-pull, aktiv low)
    activity_led_n_opdrn:   out     std_logic   -- für Aktivitätsanzeige von lemo_io vorgesehen   (opendrain, aktiv low)
    );
end lemo_io;


architecture arch_lemo_io of lemo_io is


signal      edge_detect:            std_logic_vector(1 downto 0);
signal      pos_edge, neg_edge:     std_logic;
signal      stretched_led_n:        std_logic;
signal      stretched_led_n_opdrn:  std_logic;
signal      edge_modelsim:          std_logic;

begin

p_edge_detect:  process (clk, reset)
    begin
        if reset = '1' then
            edge_detect <= (OTHERS => '0');
        elsif rising_edge(clk) then
            edge_detect <= (edge_detect(0) & lemo_io);
        end if;
    end process p_edge_detect;

pos_edge <=     lemo_io and not edge_detect(1);
neg_edge <= not lemo_io and     edge_detect(1);

edge_modelsim <= pos_edge or neg_edge;

p_led: led_n
  generic map (
              stretch_cnt     => stretch_cnt
              )
  port map    (
              ena         => '1',
              CLK         => led_clk,
              Sig_In      => edge_modelsim,
              nLED        => stretched_led_n,
              nLED_opdrn  => stretched_led_n_opdrn
              );


p_lemo_io_tristate: process (lemo_io_is_output, to_lemo)
  begin
    if lemo_io_is_output = '0' then
      lemo_en_in <= '1';
      lemo_io <= 'Z';
    else
      lemo_en_in <= '0';
      lemo_io <= to_lemo;
    end if;
  end process p_lemo_io_tristate;


p_lemo_led: process (stretch_led_off, stretched_led_n, pos_edge, neg_edge)
  begin
    if stretch_led_off = '0' then
      activity_led_n          <= stretched_led_n;
      activity_led_n_opdrn    <= stretched_led_n_opdrn;
    else
      activity_led_n <= pos_edge or neg_edge;
      if (pos_edge = '1') or (neg_edge = '1') then
        activity_led_n_opdrn <= '0';
      else
        activity_led_n_opdrn <= 'Z';
      end if;
    end if;
  end process p_lemo_led;

end arch_lemo_io;
