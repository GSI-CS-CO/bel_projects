----------
 --single counter module


library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

USE work.BLM_counter_pkg.all;


entity BLM_Counter is

  generic
    (
      CH_N     : INTEGER := 0;         -- which channel?
      CNT_TYPE : INTEGER := 0                          -- 0 no divider and 100ys, 1-4 standard counter , 5 - start stop type
    );

  port
    (
    clk        : in    std_logic;                      -- should be the same clk, used by SCU_Bus_Slave
    nReset     : in    std_logic := '1';

    Puls_in    : in    std_logic := '0';               -- Input pulse
    cycle      : in    std_logic := '0';               -- input value each LH/HL starts new cycle
    SS_scale   : in    std_logic_vector(3 downto 0);   -- counter scale value

    Threshold  : in    std_logic_vector(31 downto 0);  -- Threshold value
    TimeAct    : in    std_logic_vector(47 downto 0);  -- local Time
    
    
    Trig_out_en: in    std_logic := '0';               -- enable FIFO storage Tima @ Trigger signal   and FBAS PIN
    FIFO_out_en: in    std_logic := '0';               -- enable FIFO storage of counter value at end of cycle    
    EventtoFIFO_en:in  std_logic := '0';               -- enable FIFO storage of time when Trigger reached
    
    Tokin      : in    std_logic := '0';               -- Token in
    Tokout     : out   std_logic := '0';               -- Token for next Module
    
    FIFO_wr_req: out   std_logic := '0';               -- sign new counter value is availiable to be stored in FIFO
    NewFIFOVal : out   std_logic_vector(31 downto 0);  -- counter at / stop/ trigger
    FFwrACK_in : in    std_logic := '0';               -- acknowledge counter value processed
        
    Trig_out   : out   std_logic := '0'                -- Sign threshold trigger - duration time?

    );
  end BLM_Counter;


architecture BLM_Counter of BLM_Counter is

signal   CntStart_0     : std_logic:='0';
signal   Puls_LH        : std_logic:='0';

signal   Puls_LH0       : std_logic:='0';
signal   Puls_LH1       : std_logic:='0';

signal   up_Counter     : std_logic_vector(23 downto 0):=(others => '0');

signal   threshold_reg  : std_logic_vector(23 downto 0):=(others => '0');

signal   cnt_div        : std_logic_vector (3 downto 0) := X"0";

signal   CntStartStop   : std_logic:='0'; -- store and start new counter
signal   CntStartStop_0 : std_logic:='0'; -- store and start new counter
signal   CntStop        : std_logic:='0';

signal   FFwr_out       : std_logic:='0'; --request a data write
signal   FFwr_outrst    : std_logic:='0'; --reset FF store request
signal   FFov_out       : std_logic:='0'; --overflow when Register was not Acknowleged / not serviced

signal   FFregister_out : std_logic_vector(23 downto 0) :=(others => '0'); -- output latch for io[31..24]
signal   enable_counter : std_logic:='0';               -- sync counter  with cycle

constant COUNTER_FULL   : std_logic_vector(23 downto 0) :=(others => '1');
constant COUNTER_EMPTY  : std_logic_vector(23 downto 0) :=(others => '0');

type t_StateTOkEv is (Tok0, Tok1, Tok2, Tok3, Tok4, Tok5, Tok6, Tok7, Tok8, Tok9, Tok10);
signal Tokstate         : t_StateTOkEv := Tok0;

signal TimeTres         : std_logic_vector(47 downto 0):= (others => '0');   -- local Time at threshold

signal TimeValLSW       : std_logic_vector(31 downto 0) := (others => '0');   -- LSW counter at /  Threshold
signal TimeValMSW       : std_logic_vector(31 downto 0) := (others => '0');   -- MSW counter at / Threshold
signal TimetoFIFO       : std_logic:='0';
signal TimetoFIFOrst    : std_logic:='0';


-------------------------------------------------------------------------------
begin


threshold_reg<= Threshold(23 downto 0); -- to do set up at 10us change or at start of next cycle otherwise problems

COUNTER: process (nReset, clk,Puls_LH0,Puls_LH1,threshold_reg,FFwrACK_in,enable_counter,CntStartStop,CntStartStop_0,Trig_out_en,TimetoFIFOrst)
    begin
        if (nReset = '0')  then
            up_Counter      <= (others => '0');         -- Reset up_counter register
            TimetoFIFO      <= '0';
            Trig_out        <= '0'; 
            TimetoFIFO      <= '0';
            TimeValLSW      <= (others => '0'); 
            TimeValMSW      <= (others => '0'); 
            FFregister_out  <= (others => '0');
            FFwr_out        <='0';
            enable_counter  <='0';
            
        elsif rising_edge(clk) then                    
            
            if TimetoFIFOrst='1' then
               TimetoFIFO <='0';
            end if; 

            if FFwr_outrst = '1' then -- reset this ack and rst overflow and reset request signal
               FFwr_out<='0'; -- reset store request
            end if;
            -- todo sync Module enable with cycle start count when new cycle starts            
            if (CntStartStop_0='0' and CntStartStop='1') then --new cycle HL detected -reset cnt value                 
               if (enable_counter='0') then -- counter activated?
                  enable_counter <= '1';     --do nothing at first  CntStartStop='1' -> collect pulse first
                  FFwr_out   <='0';            -- reset any store request                  
                  TimetoFIFO <='0';
               end if;
               Trig_out <= '0'; 
               if (enable_counter='1') then -- counter activated?
                  --- check at every count pulse               -- Pulse values to count for each module
                  if FIFO_out_en ='1' then  --request to store               
                     if (CNT_TYPE = 0) then --Im a 100us timer
                        if cycle='0' then --Msb (1st) Cycle --check if ist starts at '0'
                           FFregister_out(11 downto 0)  <= up_Counter(11 downto 0);
                           FFregister_out(23 downto 12) <= (others => '0');
                        else -- 2nd CntStartStop='1'
                           FFregister_out(23 downto 12) <= up_Counter(11 downto 0);
                           FFwr_out<='1';   -- set store request
                        end if;
                     else 
                        FFregister_out(23 downto 0) <= up_Counter(23 downto 0);
                        FFwr_out<='1';   -- set store request                                                               
                     end if;
                  else -- if FIFO_out_en ='1' then 
                     FFwr_out <='0'; -- reset store request
                  end if;-- if FIFO_out_en ='1
               end if;                              
               up_Counter <= (others => '0');            -- Reset up_counter register finally
            end if;    -- if (CntStartStop_0='0' and CntStartStop='1') then --new cycle HL detected -reset cnt value  
                              
            if (enable_counter='1') then              --counter active? check for theshold
               if ((Puls_LH0= '1') and (Puls_LH1 = '0') )  then -- LH on pulse in (after divider) detected ?                     
                     if (up_Counter = COUNTER_FULL) then    -- check for overflow, only if below maxlen
                        --here overflow event to FIFO?
                     else                                   --not yet specified: treshold at overflow continue?
                        up_Counter <= up_Counter + '1';     -- inc
                        if (threshold_reg /= COUNTER_EMPTY) then --check threshold if value >0
                           if (up_Counter = (threshold_reg+'1')) then -- pos_threshold reached only once
                              if Trig_out_en='1' and EventtoFIFO_en = '1'then --threshold-event to FIFO?
                                 Trig_out <= '1';   --how long should it last? -> until module reset -- or rst fuction / register required   - one clkcycle                                 
                                 TimetoFIFO <= '1'; --Mark new values are present 
                                 TimeValLSW <= std_logic_vector(to_unsigned(CH_N,4))& x"7" & TimeAct(23 downto 0);
                                 TimeValMSW <= std_logic_vector(to_unsigned(CH_N,4))& x"6" & TimeAct(47 downto 24);                              
                              end if;--if Trig_out_en='1' then--if EventtoFIFO_en = '1' then --threshold-event to FIFO?
                           end if;--if (up_Counter = (threshold_reg+'1')) then -- pos_threshold reached only once
                        end if;--if (threshold_reg /= COUNTER_EMPTY) then --check threshold if value >0
                     end if;--if (not (up_Counter = COUNTER_FULL)) then
               end if; --if ((Puls_LH0= '1') and (Puls_LH1 = '0') )  then -- LH on pulse in (after divider) detected ?                                
            else
                Trig_out <= '0'; 
                TimetoFIFO <= '0'; --Mark new values are present  
                TimeValLSW <= (others => '0'); 
                TimeValMSW <= (others => '0');                
            end if; --if not (enable_counter='1') then --counter active?

            if EventtoFIFO_en = '0' then
               TimetoFIFO <= '0';
               FFwr_out<='0';               
            end if;
                  
        end if;--rising_edge(clk) then
 end process;


Tokenque: process (nReset, clk,FFwrACK_in,Tokin,FFwr_out,Tokstate,FFregister_out,TimetoFIFOrst)

   begin
    if (nReset = '0') then
      Tokout        <= '0';
      NewFIFOVal    <= (others=> '0');
      FIFO_wr_req   <= '0';
      Tokstate      <= Tok0;
      
      FFwr_outrst   <= '0';
      TimetoFIFOrst <= '0';

    elsif rising_edge(clk) then
            
         Tokout      <= '0';   
         FFwr_outrst <= '0';

         case  Tokstate is

            when Tok0 =>   FIFO_wr_req <= '0';                          
                           if Tokin = '1' then -- check here now?
                              if FFwr_out = '1' then --anything to write to FIFO?
                                 Tokstate <= Tok1;
                              else
                                 if TimetoFIFO = '1' then --Threshold request?
                                    Tokstate <= Tok4; 
                                 else   
                                    Tokout <= '1';  --continue
                                 end if;                                                                  
                              end if;
                           end if;

            when Tok1 =>   if FFwrACK_in='0' then     -- fifo ack before must be 0
                              Tokstate <= Tok2;
                           end if;

            when Tok2 =>   FIFO_wr_req  <= '1';    -- sign new FIFO value is avaliable
                           NewFIFOVal(23 downto 0)  <= FFregister_out(23 downto 0);
                           NewFIFOVal(27 downto 24) <= std_logic_vector(to_unsigned(CNT_TYPE,4));
                           NewFIFOVal(31 downto 28) <= std_logic_vector(to_unsigned(CH_N,4));
                           FFwr_outrst <= '1';     -- FFwr_out can now be resetted
                           Tokstate <= Tok3;

            when Tok3 =>  if FFwrACK_in = '1' then -- wait for FIFO ack
                              FIFO_wr_req <='0';
                              FFwr_outrst <='0';
                              Tokstate <= Tok4;
                           end if;

            when Tok4 =>   if FFwrACK_in = '0' then  -- wait for FF ack  release
                              Tokstate <= Tok5;
                           end if;
                                                      
------------------------                           
-- now check for threshold event to fifo 

            when Tok5 =>   if TimetoFIFO = '1' then     -- anything to write to FIFO?
                              TimetoFIFOrst <= '1';     -- reset requirement
                              FIFO_wr_req   <= '1';     -- sign new counter value is avaliable
                              NewFIFOVal    <= TimeValLSW;  -- setup output value
                              Tokstate      <= Tok6;
                           else
                              Tokout   <= '1';
                              Tokstate <= Tok0;
                           end if;
                           
            when Tok6 =>   if FFwrACK_in = '1' then     -- FIFO ack must be 0 before next step
                              TimetoFIFOrst <='0';      -- reset requirement             
                              FIFO_wr_req   <='0';
                              Tokstate <= Tok7;
                           end if;

            when Tok7 =>   if FFwrACK_in = '0' then   -- FIFO ack before must be 0
                              Tokstate <= Tok8;      -- do 2nd request
                           end if;

            when Tok8 =>   --write MSW to FF
                           FIFO_wr_req <= '1';        -- sign new FIFO value is avaliable
                           NewFIFOVal  <= TimeValMSW; -- setup 2nd output value
                           Tokstate <= Tok9;

            when Tok9 =>  if FFwrACK_in = '1' then   -- wait for FIFO ack
                              FIFO_wr_req <= '0';
                              Tokstate <= Tok10;
                           end if;

            when Tok10 =>  if FFwrACK_in = '0' then   -- wait for FIFO ack  release
                              Tokout   <= '1';         -- sign FIFO token for next one cycle store was done or not required
                              Tokstate <= Tok0;
                           end if;

            when others => Tokstate <= Tok0;
         end case;
       
  end if; --elsif rising_edge(clk) then

end process;

-- divider for Puls_in inputs  / divider only for start Stop module
-- other modules will cnt puls 1:1
CntDivider: process (nReset, clk,Puls_in, SS_scale,cnt_div)

   begin
    if (nReset = '0') then
      cnt_div <= (others=> '0');
      Puls_LH0 <='0';
      Puls_LH1 <='0';
      Puls_LH  <='0';
    elsif rising_edge(clk) then
       if CNT_TYPE =5 then
         Puls_LH <= Puls_in;
         if (Puls_LH='0' and Puls_in = '1') then --LH slope -> cnt+1
           cnt_div <= cnt_div + '1'; -- free running
         end if;

         case  SS_scale is
            when "0000" => Puls_LH0 <= Puls_in;
            when "0001" => Puls_LH0 <= cnt_div(0);
            when "0010" => Puls_LH0 <= cnt_div(1);
            when "0011" => Puls_LH0 <= cnt_div(2);
            when "0100" => Puls_LH0 <= cnt_div(3);
            when others => Puls_LH0 <='0';
         end case;
       else
        Puls_LH0 <= Puls_in;     -- Pulse 1:1 - no divider     
     end if;--if CNT_TYPE =5 then
     Puls_LH1 <= Puls_LH0;     
    end if;-- elsif rising_edge(clk) then
end process;


--------------------------------------------------------------------------
--------------------------------------------------------------------------
gecnt: if CNT_TYPE = 5 generate  
      -- Start stop counter if CNT_TYPE =5 


Startstopcnt0: process (nReset, clk,cycle,CntStartStop,CntStart_0)
    begin
        if (nReset = '0') then
            CntStartStop  <= '0';
            CntStartStop_0<= '0';
            CntStart_0<= '0';
        elsif rising_edge(clk) then
            CntStart_0<=cycle;
            CntStartStop<='0';            
            if CntStart_0='0' and cycle='1' then --lh an cycle
               CntStartStop<='1';
            end if;
            if CntStart_0='1' and cycle='0' then --hl an cycle
               CntStartStop<='1';
            end if;            
            CntStartStop_0 <= CntStartStop;
        end if;
end process;


---------------------------------
end generate;
---------------------------------
-- und standard counter
gecnt2: if (CNT_TYPE /=5 )generate  

Startstopcnt1: process (nReset, clk,cycle)

    begin
        if (nReset = '0') then
            CntStartStop<='0';
            CntStartStop_0<='0';
        elsif rising_edge(clk) then
            CntStartStop<=cycle;
            CntStartStop_0 <= CntStartStop;
        end if;
end process;


end generate;


--Cyclend: process (nReset, clk,threshold_reg,FFwrACK_in,CntStartStop,CntStartStop_0,FFwr_out,FFwr_outrst,enable_counter)
--    begin
--        if (nReset = '0') then
--        
--            FFregister_out <= (others => '0');
--            FFwr_out       <='0';
----            FFov_out       <='0';
--            enable_counter <='0';
--            
--        elsif rising_edge(clk) then
--         --setup channel type
--            if FFwr_outrst = '1' then -- reset this ack and rst overflow and reset request signal
--               FFwr_out<='0'; -- reset store request
----               FFov_out<='0';
--            end if;
--          -- todo sync Module enable with cycle start count when new cycle starts
--          --  if (CntStartStop='1') then --new cycle detected -store last cnt value and reset counter
--          if (CntStartStop_0='0' and CntStartStop='1') then --new cycle detected LH -store last cnt value and reset counter
--            if enable_counter= '0' then --and do nothing at first start
--                 enable_counter <= '1';  --do nothing at first  CntStartStop='1' -> collect pulse first
--                 FFwr_out <='0'; -- reset store request
--            else
--               if FIFO_out_en ='1' then  --request to store               
--                  if (CNT_TYPE = 0) then --Im a 100us timer
--                     if cycle='0' then --Msb (1st) Cycle --check if ist starts at '0'
--                        FFregister_out(11 downto 0)  <= up_Counter(11 downto 0);
--                        FFregister_out(23 downto 12) <= (others => '0');
--                     else -- 2nd CntStartStop='1'
--                        FFregister_out(23 downto 12) <= up_Counter(11 downto 0);
----                        if FFwr_out='1' then        -- request not serviced before?
----                           FFov_out<='1'; --set overflow markerFFwr_outrst <= '1';              -- FFwr_out can now be resetted
----                        end if;
--                        FFwr_out<='1';   -- set store request
--                     end if;
--                  else 
--                     FFregister_out(23 downto 0) <= up_Counter(23 downto 0);
--                     FFwr_out<='1';   -- set store request                                                               
--                  end if;
--               else -- if FIFO_out_en ='1' then 
--                  FFwr_out <='0'; -- reset store request
--               end if;-- if FIFO_out_en ='1' then 
--               
--            end if;-- else if enable_counter='0' then
--          end if;
--        end if;
-- end process;







end BLM_Counter;
