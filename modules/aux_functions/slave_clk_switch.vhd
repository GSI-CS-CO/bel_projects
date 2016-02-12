library ieee;
use ieee.std_logic_1164.all; 
use IEEE.numeric_std.all;
use work.aux_functions_pkg.all;
library work;

LIBRARY altera_mf;
USE altera_mf.altera_mf_components.all;



--+------------------------------------------------------------------------------------------------------------------+
--|  "sio3_sys_clk_local_clk_switch"    Autor: W.Panschow                                                            |
--|                                                                                                                  |
--|                                                                                                                  |
--|   Supports Clock monitoring and Clock Switch from local generated Clock to SCU Bus SYS_CLK                       |
--|   When PLL gets alive at power up , Macro is running on local clock.                                             |
--|   When SYS_CLK is healthy and power-on time is completed, PLL switches to SYS_CLK reference.                     |
--|   When running on SYS_CLK reference, SYS_CLK_IS_BAD can cause an automatic switchover to local clock             |
--|   When running on LOCAL_CLK reference, SYS_CLK_IS_BAD must be '0' to switch back to SYS_CLK reference            |
--|                                                                                                                  |  
--|   For switching back to SYS_CLK as reference, start_switch_clk_input signal has to be triggered by write access  |
--|   to clock status register (write causes clk_switch_cntrl=1, which causes s_sys_clk_deviation_la=0               |
--|   (healthy SYS_CLK assumed)                                                                                      |
--|                                                                                                                  |
--|   Macro monitors two clock error critera:                                                                        |
--|      a) sys_clk_deviation: SYS_CLK is checked for Accuracy (+/- 3 Periods within 12500 clock cycles)             |
--|      b) sys_clk_is_bad   : PLL detects, that  SYS_CLK PLL input has 3 missing edges and does automatic switch    |
--|   Both are triggering an clock error interrupt in combination with set of status bits and led control            |
--|                                                                                                                  |
--|..................................................................................................................|
--|   V.1 W.Panschow: Initial Version                                                                                            |
--|..................................................................................................................|
--|   V.2 K.Kaiser:   Connecting dtack to s_dtack, adding description header                                                     |
--+------------------------------------------------------------------------------------------------------------------+




entity slave_clk_switch is 
  generic (
    Base_Addr:  unsigned(15 downto 0)  := x"0040";
    card_type:  string -- "diob" "addac" or "sio"
    );

  port(
    local_clk_i:          in      std_logic;
    sys_clk_i:            in      std_logic;
    nReset:               in      std_logic;
    master_clk_o:         out     std_logic;
    pll_locked:           out     std_logic;
    sys_clk_is_bad:       buffer  std_logic;
    sys_clk_is_bad_la:    out     std_logic;
    local_clk_is_bad:     buffer  std_logic;
    local_clk_is_running: buffer  std_logic;
    sys_clk_deviation:    out     std_logic;
    sys_clk_deviation_la: out     std_logic;
    Adr_from_SCUB_LA:     in      std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
    Data_from_SCUB_LA:    in      std_logic_vector(15 downto 0);  -- latched data from SCU_Bus
    Ext_Adr_Val:          in      std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:        in      std_logic;                      -- '1' => Rd-Cycle is active
    Ext_Wr_active:        in      std_logic;                      -- '1' => Wr-Cycle is active
    Rd_Port:              out     std_logic_vector(15 downto 0);  -- output for all read sources of this macro
    Rd_Activ:             out     std_logic;                      -- this acro has read data available at the Rd_Port.
    Dtack:                out     std_logic;
    signal_tap_clk_250mhz:out     std_logic;
    clk_update:           out     std_logic;
    clk_flash:            out     std_logic;
    clk_encdec:           out     std_logic
    );
end slave_clk_switch;

architecture arch_slave_clk_switch of slave_clk_switch is


component sys_clk_or_local_clk
  port(
    clkswitch:    in    std_logic := '0';
    inclk0:       in    std_logic;
    inclk1:       in    std_logic;
    c0:           out   std_logic;
    c1:           out   std_logic;
    c2:           out   std_logic;
    c3:           out   std_logic;
    locked:       out   std_logic;
    activeclock:  out   std_logic;
    clkbad0:      out   std_logic;
    clkbad1:      out   std_logic
    );
end component;

constant clk_switch_status_cntrl_addr: unsigned (15 downto 0) := base_Addr;
signal  master_clk:           std_logic;
signal  f_local_12p5_mhz:     std_logic;

signal  s_rd_active:          std_logic;
signal  s_dtack:              std_logic;
signal  clk_switch_cntrl:     std_logic;
signal  s_sys_clk_is_bad_la:  std_logic;

signal  start_pll_control:    std_logic;

constant  test_time_cnt_max:  integer := 12_500;  -- die Testzeit soll eine Millisekunde betragen. Ein Zaehler muss bei 12,5Mhz
                                                  -- einen Zaehlerstand von 12500 erreichen.
signal    test_time_cnt:      integer range 0 to test_time_cnt_max;

constant  sys_clk_cnt_max:    integer := 21_000;  -- waehrend der Testzeit von einer Millisekunde, kann bei einer angenommenen
                                                  -- Externen-Maximalfrequenz von 21MHz ein Zaehlerstand von 21000 erreicht werden.
signal    sys_clk_cnt:        integer range 0 to sys_clk_cnt_max;

signal  s_sys_clk_deviation:    std_logic;
signal  s_sys_clk_deviation_la: std_logic;

signal  f_local_12p5_mhz_sync:  std_logic_vector(2 downto 0);
signal  sys_clk_i_sync:         std_logic_vector(2 downto 0);
signal  compare:                std_logic;

signal  start_switch_clk_input: std_logic;
constant  clk_switch_cnt_max:   integer := 3;
signal    clk_switch_cnt:       integer range 0 to clk_switch_cnt_max;


begin 


clk_encdec  <= f_local_12p5_mhz;

pll_125 : if card_type = "addac" generate
    local_clk: local_125_to_12p5
      port map(
        inclk0  => local_clk_i,
        c0      => f_local_12p5_mhz
      );
end generate;

pll_20 : if card_type = "diob" or card_type = "sio" generate
    local_clk: local_20_to_12p5
      port map(
        inclk0  => local_clk_i,
        c0      => f_local_12p5_mhz
      );
end generate;

sys_or_local_pll: sys_clk_or_local_clk
  port map(
    clkswitch		=> start_switch_clk_input,
    inclk0      => sys_clk_i,
    inclk1      => f_local_12p5_mhz,
    c0          => master_clk,
    c1          => clk_update,
    c2          => clk_flash,
    c3          => signal_tap_clk_250mhz, 
    locked      => pll_locked,
    activeclock => local_clk_is_running,
    clkbad0     => sys_clk_is_bad,
    clkbad1     => local_clk_is_bad
    );


p_adr_deco: process (master_clk, nReset)
  begin
    if nReset = '0' then
      s_rd_active       <= '0';
      s_dtack           <= '0';
      clk_switch_cntrl  <= '0';

    elsif rising_edge(master_clk) then
    
      s_rd_active       <= '0';
      s_dtack           <= '0';
      clk_switch_cntrl  <= '0';

      if Ext_Adr_Val = '1' then

        case unsigned(Adr_from_SCUB_LA) IS

          when clk_switch_status_cntrl_addr =>
            if Ext_Wr_active = '1' then
              clk_switch_cntrl  <= '1';
              s_dtack           <= '1';
            end if;
            if Ext_Rd_active = '1' then
              Rd_Port       <=  x"00"
                              & '0' & '0' & s_sys_clk_deviation & s_sys_clk_deviation_la
                              & local_clk_is_running & local_clk_is_bad & sys_clk_is_bad & s_sys_clk_is_bad_la;
              s_rd_active   <= '1';
              s_dtack       <= '1';
            end if;

          when others =>
            s_rd_active       <= '0';
            s_dtack           <= '0';
            clk_switch_cntrl  <= '0';

        end case;
      end if;
    end if;
  end process p_adr_deco;
  
Rd_Activ <= s_rd_active;
Dtack    <= s_dtack;

            
p_err_latch: process (master_clk, nReset)
  constant  wait_n_cnt: integer := 125_000-1; -- erst eine Millisekunde nach Powerup soll die PLL ueberwacht werden. Deshalb muss der
                                              -- Zaehler muss bei 125Mhz einen Zaehlerstand von 125000 erreichen.
  variable  wait_n:     integer range 0 to wait_n_cnt;

  begin
    if nReset = '0' then
      wait_n := 0;
      start_pll_control <= '0';
      s_sys_clk_is_bad_la <= '0';
      start_switch_clk_input <= '0';
      clk_switch_cnt <= 0;

    elsif rising_edge(master_clk) then
      if wait_n < wait_n_cnt then
        start_pll_control <= '0';
        s_sys_clk_is_bad_la <= '0';
        wait_n := wait_n + 1;
      else
        start_pll_control <= '1';
        if sys_clk_is_bad = '1' then
          s_sys_clk_is_bad_la <= '1';
        elsif clk_switch_cntrl = '1' then
          if Data_from_SCUB_LA(0) = '1' and s_sys_clk_is_bad_la = '1' then
            -- nur wenn "s_sys_clk_is_bad_la" gesetzt ist, soll es zurueckgesetzt werden, da anschliessend
            -- noch getestet wird, ob auf die sys_clk zurueckgeschaltet werden kann.
            s_sys_clk_is_bad_la <= '0';
            if s_sys_clk_deviation = '0' and local_clk_is_running = '1' then
              -- nur wenn die sys_clk in der vorgegebenen Toleranz ist und die pll "sys_clk_or_local_clk"
              -- mit der localen Clock getrieben wird, soll das Umschalten auf sys_clk erlaubt sein.
              start_switch_clk_input <= '1';
            else
              start_switch_clk_input <= '0';
            end if;
          end if;
        end if;
        if start_switch_clk_input = '1' then
          if f_local_12p5_mhz_sync(2 downto 1) = "01" then
            if clk_switch_cnt < clk_switch_cnt_max then
              clk_switch_cnt <= clk_switch_cnt + 1;
            else
              start_switch_clk_input <= '0';
              clk_switch_cnt <= 0;
            end if;
          end if;
        end if;
      end if;
    end if;      
  end process p_err_latch;


p_sys_freq_test:  process (master_clk, nReset)
  begin
    if nReset = '0' then
      f_local_12p5_mhz_sync   <= b"000";
      sys_clk_i_sync          <= b"000";
      test_time_cnt           <= 0;
      sys_clk_cnt             <= 0;
      s_sys_clk_deviation     <= '0';
      s_sys_clk_deviation_la  <= '0';
      compare                 <= '0';

    elsif rising_edge(master_clk) then
      f_local_12p5_mhz_sync   <= f_local_12p5_mhz_sync(1 downto 0) & f_local_12p5_mhz;  
      sys_clk_i_sync          <= sys_clk_i_sync(1 downto 0) & sys_clk_i;

      
      if start_pll_control = '1' then

        if f_local_12p5_mhz_sync(2 downto 1) = "01" then
          test_time_cnt <= test_time_cnt + 1;
        elsif test_time_cnt = test_time_cnt_max - 1 then
          compare <= '1';
        elsif test_time_cnt = test_time_cnt_max then
          compare <= '0';
          test_time_cnt <= 0;
          sys_clk_cnt <= 0;
        end if;

        if compare = '1' then
          if (sys_clk_cnt < test_time_cnt_max - 3) or (sys_clk_cnt > test_time_cnt_max + 3) then
            s_sys_clk_deviation <= '1';
            s_sys_clk_deviation_la <= '1';
          end if;
        elsif (sys_clk_i_sync(2 downto 1) = "01") then
          sys_clk_cnt <= sys_clk_cnt + 1;
          s_sys_clk_deviation <= '0';
        end if;
        
        if (clk_switch_cntrl = '1') and (Data_from_SCUB_LA(4) = '1') then
          s_sys_clk_deviation_la <= '0';
        end if;
 
      end if;
    end if;
  end process p_sys_freq_test;


sys_clk_deviation     <= s_sys_clk_deviation;
sys_clk_deviation_la  <= s_sys_clk_deviation_la;

sys_clk_is_bad_la     <= s_sys_clk_is_bad_la;
master_clk_o          <= master_clk;

end arch_slave_clk_switch;
