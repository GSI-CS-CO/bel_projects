
----------
 --Generates 100us .. 1sec time base
 --generates Timing events
 --todo: - at wich Time base should the Time event value be gererated?


library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

USE work.BLM_counter_pkg.all;


entity BLM_Tbase is

  port
    (
    clk           :     in    std_logic;                    -- should be the same clk, used by SCU_Bus_Slave
    nReset        :     in    std_logic := '1';

    Tclk1us       :     in    std_logic := '1';             -- 1us clock input
    Cycletimers   :     out   std_logic_vector(4 downto 0); -- generate 100us, 1ms 10ms 100ms, 1sec  timing pulses
    
    TimeAct       :     out   std_logic_vector(47 downto 0);   -- sign new counter value is available

    Glb_CFGReg    :     in    std_logic_vector(15 downto 0);-- config

    Tokin         :     in    std_logic := '0';             -- Token in
    Tokout        :     out   std_logic := '0';             -- Token for next Module

    TimetoFF      :     out   std_logic := '0';             -- sign new counter value is available
    TimeVal       :     out   std_logic_vector(31 downto 0);-- counter at / stop/ trigger
    TimeAck       :     in    std_logic := '0'              -- FIFO acknowledge - counter value is stored in FIFO
    
    );
end BLM_Tbase;


architecture BLM_Tbase of BLM_Tbase is

----------------------
--signals for Clock Timings

signal cnt_div100us  : integer range 0 to 101 :=0;
signal cnt_div1ms    : integer range 0 to 10 :=0;
signal cnt_div10ms   : integer range 0 to 10 :=0;
signal cnt_div100ms  : integer range 0 to 10 :=0;
signal cnt_div1sec   : integer range 0 to 10 :=0;

constant CNTTO10     : integer := 10;
constant CNTTO100    : integer := 100;

signal Cycletimer_loc:  std_logic_vector(4 downto 0):= (others => '0');



----------------------
--signals for Time-Event generation
signal LocTimeVal    : std_logic_vector(31 downto 0):= (others => '0');    -- counter at / stop/ trigger
signal FFwr_outL     : std_logic := '0';
signal FFwr_outM     : std_logic := '0';
signal DoSaveTimeVal : std_logic := '0';

signal TimeCounter   : std_logic_vector(47 downto 0):= (others => '0');    -- Timer register

signal EventReg      : std_logic_vector(2 downto 0):= (others => '0');     -- setup of  Timer events
signal TimeValLSW    : std_logic_vector(31 downto 0) := (others => '0');   -- LSW counter at / stop/ trigger
signal TimeValMSW    : std_logic_vector(31 downto 0) := (others => '0');   -- LXSW counter at / stop/ trigger

type t_StateTimEv is (Timestate0, Timestate1, Timestate2, Timestate3);
signal Timestate     : t_StateTimEv;

type t_StateTOkEv is (Tok0, Tok1, Tok2, Tok3, Tok4, Tok5, Tok6, Tok7, Tok8);
signal Tokstate  : t_StateTOkEv := Tok0;



begin
TimeAct <= TimeCounter;

Cycletimers <= Cycletimer_loc;

--This structure will toggle simultaneously all Cycletimer_locsconstant CNTTO : integer range 0 to 10 := 10;
Cntall: process (nReset, clk,Tclk1us,Cycletimer_loc)
begin
   if (nReset = '0')then
      cnt_div100us<= 0;
      cnt_div1ms <= 0;
      cnt_div10ms <= 0;
      cnt_div100ms <= 0;
      cnt_div1sec <= 0;
      Cycletimer_loc <=(others => '0');
   elsif rising_edge(clk) then
      Cycletimer_loc <=(others => '0');
      if (Tclk1us = '1') then
         if cnt_div100us = CNTTO100 then
            Cycletimer_loc(0) <= '1';
             cnt_div100us<=0;
            if cnt_div1ms = CNTTO10 then
              Cycletimer_loc(1) <= '1';
              cnt_div1ms<=0;
              if cnt_div10ms = CNTTO10 then
                  Cycletimer_loc(2) <= '1';
                  cnt_div10ms<=0;
                  if cnt_div100ms = CNTTO10 then
                     Cycletimer_loc(3) <= '1';
                     cnt_div100ms<=0;
                     if cnt_div1sec = CNTTO10 then
                        Cycletimer_loc(4) <= '1';
                        cnt_div1sec<=0;
                     else
                        cnt_div1sec<=cnt_div1sec+1;
                     end if;
                  else
                     cnt_div100ms<=cnt_div100ms+1;
                  end if;
               else
                  cnt_div10ms<=cnt_div10ms+1;
               end if;
            else
               cnt_div1ms<=cnt_div1ms+1;
            end if;
         else
            cnt_div100us<=cnt_div100us+1;
         end if;
      end if;
   end if;   --rising_edge(clk) then
end process;

--------------------------------

--timer counter
TimCnt: process (nReset, clk,Tclk1us,TimeCounter)
    begin
        if (nReset = '0') then
            TimeCounter <= (others => '0');         -- Reset up_counter register
        elsif rising_edge(clk) then
--- check at every count pulse     Cycletimer_loc( 3 downto 0)-- Pulse values to count for each module
            if (Tclk1us = '1') then
               TimeCounter <= TimeCounter + '1';     -- inc
            end if;
        end if;
 end process;


--Time event
-- Check and generate
CheckTimEv: process (nReset, clk,Cycletimer_loc,Tokstate,Glb_CFGReg,TimeCounter,TimeAck,DoSaveTimeVal,Tokin)
    begin
        if (nReset = '0') then

            Tokstate       <= Tok0;
            TimetoFF       <= '0';
            Tokout         <= '0';
            TimeVal        <= (others=> '0');
            DoSaveTimeVal  <= '0';

        elsif rising_edge(clk) then

            Tokout      <= '0';

            TimeValMSW(31 downto 24) <="11111"& "110"; --"F6"  -- save IDs  by default --0x1F+0x6 
            TimeValLSW(31 downto 24) <="11111"& "111"; --"F7"

            case Glb_CFGReg(2 downto 0) is
               when "001" =>  if Cycletimer_loc(1) = '1' then --1ms
                                 DoSaveTimeVal <= '1';
                                 TimeValMSW(23 downto 0) <= TimeCounter(47 downto 24);-- save IDs  by default
                                 TimeValLSW(23 downto 0) <= TimeCounter(23 downto 0);
                              end if;

               when "010" =>  if Cycletimer_loc(2) = '1' then  --10ms
                                 DoSaveTimeVal <= '1';
                                 TimeValMSW(23 downto 0) <= TimeCounter(47 downto 24);
                                 TimeValLSW(23 downto 0) <= TimeCounter(23 downto 0);           -- save IDs  by default
                              end if;

               when "011" =>  if Cycletimer_loc(3) = '1' then  --100ms
                                 DoSaveTimeVal <= '1';
                                 TimeValMSW(23 downto 0) <= TimeCounter(47 downto 24);
                                 TimeValLSW(23 downto 0) <= TimeCounter(23 downto 0);           -- save IDs  by default

                              end if;

               when "100" =>  if Cycletimer_loc(4) = '1' then  --1s
                                 DoSaveTimeVal <= '1';
                                 TimeValMSW(23 downto 0) <= TimeCounter(47 downto 24);
                                 TimeValLSW(23 downto 0) <= TimeCounter(23 downto 0);           -- save IDs  by default
                              end if;

               when others =>
            end case;   -- case Glb_CFGReg(2 downto 0) is

            case  Tokstate is
               when Tok0 =>   TimetoFF    <= '0';
                              if Tokin = '1' then -- check here now?
                                 if DoSaveTimeVal = '1' then --anything to write to FIFO?
                                    DoSaveTimeVal <='0';   -- reset requirement
                                    Tokstate <= Tok1;
                                 else
                                    Tokout <='1';
                                 end if;
                              end if;

               when Tok1 =>   if TimeAck='0' then     -- FIFO ack must be 0 before next step
                                 Tokstate <= Tok2;
                              end if;

               when Tok2 =>   TimetoFF <= '1';        -- sign new counter value is avaliable
                              TimeVal  <= TimeValLSW; -- setup output value
                              Tokstate <= Tok3;

               when Tok3 =>   if TimeAck = '1' then   -- FIFO ack before must be 1
                                 TimetoFF <= '0';     -- Reset 1st request
                                 Tokstate <= Tok4;
                              end if;

               when Tok4 =>   if TimeAck = '0' then   -- FIFO ack before must be 0
                                 TimetoFF  <= '0';    -- Release request
                                 Tokstate <= Tok5;    -- do 2nd request
                              end if;

               when Tok5 =>   --write MSW to FF
                              TimetoFF <= '1';        -- sign next value is avaliable
                              TimeVal  <= TimeValMSW; -- setup 2nd output value
                              Tokstate <= Tok6;

               when Tok6 =>   if TimeAck = '1' then   -- wait for FIFO ack
                                 TimetoFF <='0';
                                 Tokstate <= Tok7;
                              end if;

               when Tok7 =>   if TimeAck = '0' then   -- wait for FIFO ack  release
                                 Tokstate <= Tok8;
                              end if;

               when Tok8 =>   Tokout <= '1';          -- sign FIFO toke for next one cycle store was done or not required
                              Tokstate <= Tok0;

               when others => Tokstate <= Tok0;
            end case; -- case  Tokstate is
        end if; --elsif rising_edge(clk) then
 end process;



end BLM_Tbase;
