library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

USE work.BLM_counter_pkg.all;



entity BLM_cntarray is

 generic
 (
   CH_N     : integer := 0       -- which channel
 );

  port
    (
    clk           : in    std_logic;                     -- should be the same clk, used by SCU_Bus_Slave
    nReset        : in    std_logic := '1';

    PulsCount     : in    std_logic := '0';              -- Input pulse
    Cycletimers   : in    std_logic_vector(5 downto 0);  -- input value each LH/HL starts new cycle (0)-100us (1)-1ms, (2)-10ms, (3)-100ms, (4)-1sec, or (5)startsop signal

    Config        : in    t_arr_Data_Word;               -- configuration Register array (address 00-0xF)

    Tokin         : in    std_logic := '0';              -- Token in
    Tokout        : out   std_logic := '0';              -- Token for next Module
    TimeAct       : in    std_logic_vector(47 downto 0); -- local Time


    CounterStore  : out   std_logic := '0';              -- sign new counter value is availiable
    CounterAck    : in    std_logic := '0';              -- acknowledge counter value processed

    Counter_Tr    : out   std_logic_vector(31 downto 0); -- counter at / stop/ trigger/threshold
    Trig_out      : out   std_logic_vector(5 downto 0)   -- Sign threshold trigger - duration time?

    );
  end BLM_cntarray;


architecture BLM_cntarray  of BLM_cntarray is

signal Ctl_reg          : std_logic_vector(15 downto 0) := (others => '0');
signal CountertoFF      : std_logic_vector(6 downto 0)  := (others => '0');

signal TokQue           : std_logic_vector(7 downto 0)  := (others => '0');    -- Token que

type t_arrcnt_Data  is array (0 to 6)  of std_logic_vector(31 downto 0);

signal Counter_TrZ      : t_arrcnt_Data := (others =>(others => 'Z'));
signal Counter_Trx      : std_logic_vector(31 downto 0) := (others => '0');

signal DoStore          : std_logic :='0';



-------------------------------------------------------------------------
Begin

TokQue(0)   <= Tokin;

gench: if  CH_N = 0  generate
   Tokout      <= TokQue(7); --add testchanel token
end generate;   

gench1: if  CH_N > 0  generate
   Tokout      <= TokQue(6);
end generate;

--Counter_Tr <=Counter_Trx when DoStore='1' else  (others => 'Z');

Ctl_reg<= Config(0);


CounterStore<=DoStore;
DoStore<= CountertoFF(0) or CountertoFF(1) or CountertoFF(2) OR CountertoFF(3) or CountertoFF(4) or CountertoFF(5) or  CountertoFF(6);

newloop: for I in 0 to 5 generate --add counter for 100ys(0) to 1sec(4) (5)start stop counter

tmr: BLM_Counter
  generic map (
    CH_N     => CH_N,        -- which channel
    CNT_TYPE => I
    )

  port map (
      clk               => clk,
      nReset            => nReset,

      Puls_in           => PulsCount,
      cycle             => Cycletimers(I),         -- input value ech LH/HL starts new cycle
      SS_scale          => Config(1)(3 downto 0),

      Threshold         => Config((I*2)+2) & Config((I*2)+3), -- threshold val
      TimeAct           => TimeAct,
      
      
      Trig_out_en       => Ctl_reg(I),             -- enable FIFO storage Tima @ Trigger signal and FBAS PIN
      FIFO_out_en       => Ctl_reg(I+8),           -- enable FIFO storage of counter value at end of cycle     
      EventtoFIFO_en    => Ctl_reg(14),            -- enable FIFO storage of time when Trigger reached
      
      Tokin             => TokQue(I),
      Tokout            => TokQue(I+1),

      FIFO_wr_req       => CountertoFF(I),         -- sign new counter value is available
      NewFIFOVal        => Counter_TrZ(I),
      FFwrACK_in        => CounterAck,             -- acknowledge counter value processed               
      Trig_out          => Trig_out(I)             -- Sign threshold trigger - duration time?
   );

   Counter_Tr <=       Counter_TrZ(I) when CountertoFF(I) ='1' else  (others => 'Z');

   end generate;

--   select ?----
--Testcounter für chan 0
gentst:
if  CH_N = 0  generate

 testcnt: BLM_Counter
        generic map (
          CH_N     => CH_N, -- which channel
          CNT_TYPE => 5     -- do with a start stop
          )

        port map (
         clk               => clk,
         nReset            => nReset,         

         Puls_in           => Cycletimers(0),       -- use 100 ys pulse to fill
         cycle             => Cycletimers(2),       -- LH stop and starts an new start stop sequence at every 10ms 
         SS_scale          => Config(1)(3 downto 0),-- use scale from Startstop register

         Threshold         => Config((6*2)+2) & Config((6*2)+3), -- threshold  adress is 0xE, 0xF        
         
         Trig_out_en       => Ctl_reg(7),           -- enable Threshold for Trigger signal for PIN HW output              
         FIFO_out_en       => Ctl_reg(15),          -- enable Threshold to time event for all enabled Trigger outputs
         EventtoFIFO_en    => Ctl_reg(14),          -- enable FIFO storage any value at end of cycle   or Timer
         
         TimeAct           => TimeAct,
         
         Tokin             => TokQue(6),
         Tokout            => TokQue(6+1),                

         FiFO_wr_req       => CountertoFF(6),      -- sign new counter value is available to be stored in FIFO
         NewFIFOVal        => Counter_TrZ(6),
         FFwrACK_in        => CounterAck,          -- acknowledge of counter value processed in FIFO         
         
         Trig_out          => open                 -- Sign threshold trigger - duration time?

         );
    Counter_Tr <=       Counter_TrZ(6) when CountertoFF(6) ='1' else  (others => 'Z');
end generate;


end BLM_cntarray;
