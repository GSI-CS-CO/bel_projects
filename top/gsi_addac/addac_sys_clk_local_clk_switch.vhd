library ieee;
use ieee.std_logic_1164.all; 
use IEEE.numeric_std.all;
use work.addac_sys_clk_local_clk_switch_pkg.all;

library work;

entity addac_sys_clk_local_clk_switch is 
  port(
    local_clk_i:          in    std_logic;
    sys_clk_i:            in    std_logic;
    nReset:               in    std_logic;
    master_clk_o:         out   std_logic;
    sys_clk_is_bad:       out   std_logic;
    local_clk_is_bad:     out   std_logic;
    local_clk_is_running: out   std_logic;
    sys_clk_failed:       out   std_logic;
    sys_clk_deviation:    out   std_logic;
    Adr_from_SCUB_LA:     in    std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
    Data_from_SCUB_LA:    in    std_logic_vector(15 downto 0);  -- latched data from SCU_Bus
    Ext_Adr_Val:          in    std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:        in    std_logic;                      -- '1' => Rd-Cycle is active
    Ext_Wr_active:        in    std_logic;                      -- '1' => Wr-Cycle is active
    Rd_Port:              out   std_logic_vector(15 downto 0);  -- output for all read sources of this macro
    Rd_Activ:             out   std_logic;                      -- this acro has read data available at the Rd_Port.
    Dtack:                out   std_logic
    );
end addac_sys_clk_local_clk_switch;

architecture arch_addac_sys_clk_local_clk_switch of addac_sys_clk_local_clk_switch is

component addac_local_clk_to_12p5_mhz
  port(
    inclk0:   in    std_logic;
    c0:       out   std_logic;
    locked:   out   std_logic
    );
end component;

component sys_clk_or_local_clk
  port(
    inclk0:   in    std_logic;
    inclk1:   in    std_logic;
    c0:       out   std_logic;
    locked:   out   std_logic;
    activeclock:  out   std_logic;
    clkbad0:  out   std_logic;
    clkbad1:  out   std_logic
    );
end component;

signal  master_clk:         std_logic;
signal  f_local_12p5_mhz:   std_logic;
signal  s_rd_active:        std_logic;
signal  s_dtack:            std_logic;
signal  clk_switch_cntrl:   std_logic;
signal  err_sys_clk_failed: std_logic;

signal  start_pll_control:  std_logic;

constant  test_time_cnt_max:  integer := 125000; 
signal    test_time_cnt:      integer range 0 to test_time_cnt_max;

constant  sys_clk_cnt_max:    integer := test_time_cnt_max + 1000; 
signal    sys_clk_cnt:        integer range 0 to sys_clk_cnt_max;

signal  test_time_cnt_sync1, test_time_cnt_sync2: integer range 0 to test_time_cnt_max;
signal  clk_switch_cntrl_sync:  std_logic_vector(1 downto 0);
signal  data_sync:              std_logic_vector(1 downto 0);
signal  s_sys_clk_deviation:    std_logic;
signal  s_sys_clk_deviation_la: std_logic;

signal  f_local_12p5_mhz_sync:  std_logic_vector(2 downto 0);
signal  sys_clk_i_sync:         std_logic_vector(2 downto 0);

begin 

local_clk: addac_local_clk_to_12p5_mhz
  port map(
    inclk0  => local_clk_i,
    c0      => f_local_12p5_mhz
    );


sys_or_local_pll: sys_clk_or_local_clk
  port map(
    inclk0      => sys_clk_i,
    inclk1      => f_local_12p5_mhz,
    c0          => master_clk,
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
                              & b"00" & s_sys_clk_deviation & s_sys_clk_deviation_la
                              & local_clk_is_running & local_clk_is_bad & sys_clk_is_bad & err_sys_clk_failed;
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

            
p_err_latch: process (master_clk, nReset)
  constant  wait_n_cnt: integer := 100-1;
  variable  wait_n:     integer range 0 to wait_n_cnt;

  begin
    if nReset = '0' then
      wait_n := 0;
      start_pll_control <= '0';
    elsif rising_edge(master_clk) then
      if wait_n < wait_n_cnt then
        start_pll_control   <= '0';
        err_sys_clk_failed  <= '0';
        wait_n := wait_n + 1;
      else
        start_pll_control <= '1';
        if sys_clk_is_bad = '1' then
          err_sys_clk_failed <= '1';
        elsif clk_switch_cntrl = '1' then
          if Data_from_SCUB_LA(0) = '1' then
            err_sys_clk_failed <= '0';
          end if;
        end if;
      end if;
    end if;      
  end process p_err_latch;


--p_test_time: process  (master_clk, nReset)
--  begin
--    if nReset = '0' then
--      test_time_cnt <= 0;
--    elsif rising_edge(f_Local_12p5_mhz) then
--      if start_pll_control = '1' then
--        if test_time_cnt < test_time_cnt_max then
--          test_time_cnt <= test_time_cnt + 1;
--        else
--          test_time_cnt <= 0;
--        end if;
--      else
--        test_time_cnt <= 0;
--      end if;
--    end if;
--  end process p_test_time;

  
p_sys_freq_test:  process (master_clk, nReset)
  begin
    if nReset = '0' then
      f_local_12p5_mhz_sync <= b"000";  
      sys_clk_i_sync        <= b"000"; 
      test_time_cnt         <= 0;
      sys_clk_cnt           <= 0;
      s_sys_clk_deviation     <= '0';
      s_sys_clk_deviation_la  <= '0';
 
    elsif rising_edge(master_clk) then
      f_local_12p5_mhz_sync   <= f_local_12p5_mhz_sync(1 downto 0) & f_local_12p5_mhz;  
      sys_clk_i_sync          <= sys_clk_i_sync(1 downto 0) & sys_clk_i;
      
      if start_pll_control = '1' then

        if f_local_12p5_mhz_sync(2 downto 1) = "01" then
          if test_time_cnt < test_time_cnt_max then
            test_time_cnt <= test_time_cnt + 1;
          else
            test_time_cnt <= 0;
          end if;
        end if;

        if sys_clk_i_sync(2 downto 1) = "01" then
          sys_clk_cnt <= sys_clk_cnt + 1;
        end if;
        
        if test_time_cnt = test_time_cnt_max then
          if (sys_clk_cnt < test_time_cnt_max - 5) or (sys_clk_cnt > test_time_cnt_max + 5) then
            s_sys_clk_deviation <= '1';
            s_sys_clk_deviation_la <= '1';
            sys_clk_cnt <= 0;
          else
            s_sys_clk_deviation <= '0';
            sys_clk_cnt <= 0;
          end if;
        end if;
        
        if (clk_switch_cntrl = '1') and (Data_from_SCUB_LA(4) = '1') then
          s_sys_clk_deviation_la <= '0';
        end if;
 
      end if;
    end if;
  end process p_sys_freq_test;

--p_sys_freq_test:  process (sys_clk_i, nReset)
--  begin
--    if nReset = '0' then
--      test_time_cnt_sync1     <= 0;
--      test_time_cnt_sync2     <= 0;
--      sys_clk_cnt             <= 0;
--      s_sys_clk_deviation     <= '0';
--      s_sys_clk_deviation_la  <= '0';
--      clk_switch_cntrl_sync   <= b"00";
--      data_sync               <= b"00";
--      
--    elsif rising_edge(sys_clk_i) then
--      test_time_cnt_sync1 <= test_time_cnt;
--      test_time_cnt_sync2 <= test_time_cnt_sync1;
--      clk_switch_cntrl_sync <= clk_switch_cntrl_sync(0) & clk_switch_cntrl;
--      data_sync <= data_sync(0) & Data_from_SCUB_LA(4);
--      if test_time_cnt_sync2 = test_time_cnt_max then
--        if (sys_clk_cnt < test_time_cnt_max - 5) or (sys_clk_cnt > test_time_cnt_max + 7) then
--          s_sys_clk_deviation <= '1';
--          s_sys_clk_deviation_la <= '1';
--        else
--          sys_clk_cnt <= sys_clk_cnt + 1;
--          s_sys_clk_deviation <= '0';
--          if (clk_switch_cntrl_sync(1) = '1') and (data_sync(1) = '1') then
--            s_sys_clk_deviation_la <= '0';
--          end if;
--        end if;
--      end if;
--    end if;
--  end process p_sys_freq_test;

sys_clk_deviation <= s_sys_clk_deviation;
sys_clk_failed <= err_sys_clk_failed;
master_clk_o <= master_clk;

end arch_addac_sys_clk_local_clk_switch;