library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

library work;
USE work.BLM_counter_pkg.all;


entity BLM_Head is

  generic
    (
      BLM_Start_addr : unsigned(15 downto 0)    := BLM_BASE_ADDR
    );

  port
    (
    -- system IO
    Adr_from_SCUB_LA:   in    std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
    Data_from_SCUB_LA:  in    std_logic_vector(15 downto 0);  -- latched data from SCU_Bus
    Ext_Adr_Val:        in    std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:      in    std_logic;                      -- '1' => Rd-Cycle is active
    Ext_Wr_active:      in    std_logic;                      -- '1' => Wr-Cycle is active

    clk:                in    std_logic;                      -- should be the same clk, used by SCU_Bus_Slave
    nReset:             in    std_logic := '1';

    Tclk1us:            in    std_logic;                      -- 1us clock in

    PulsCountin:        in    std_logic_vector(CHAN_NUM-1 downto 0);     -- Pulse values to count for each module
    TrigOutreg:         out   t_arr_Trigout;                    -- FBAS trigger out

    user_rd_active:     out   std_logic;                      -- '1' = read data avaliable at 'Data_to_SCUB'-output
    Data_to_SCUB:       out   std_logic_vector(15 downto 0);  -- consignal S_Dtack
    Dtack_to_SCUB:      out   std_logic
    );
  end BLM_Head;



architecture  BLM_Head of BLM_Head is


signal allregisters  : t_arr_allRegs;

signal CounterRequ   : std_logic := '0';
signal TimetoFFrequ  : std_logic := '0';
signal ToFIFO_Requ   : std_logic := '0'; --Request to store something to FIFO
signal FIFO_Ack      : std_logic := '0'; --Request in Fifo

signal TimeToFF      : std_logic_vector(31 downto 0) := (others => '0'); --Timestamp to be stored in FIFO


signal DataToFiFo    : std_logic_vector(31 downto 0) := (others => '0');  --Tiem or Counter to be stored in FIFO

signal TokQue        : std_logic_vector(CHAN_NUM+2 downto 0):= (others => '0');    -- Token que

signal DataFFAck     : std_logic := '0';
signal DataRegsAck   : std_logic_vector(CHAN_NUM-1 downto 0) := (others => '0');

signal DataFFread    : std_logic := '0';
signal DataRegread   : std_logic_vector(CHAN_NUM-1 downto 0) := (others => '0');
signal DataRegRD     : std_logic := '0';

signal CounterStoreC : std_logic_vector(CHAN_NUM-1 downto 0):= (others => '0');

constant all_bitsZero: std_logic_vector(CHAN_NUM-1 downto 0):= (others => '0');

signal DataToSCUFF   : std_logic_vector(15 downto 0):= (others => '0');
signal DataToSCUArr  : t_arr_Data_Word_C := (others =>(others => '0'));

signal StartStopReg  : std_logic_vector(31 downto 0):= (others => '0');

signal Glb_CFGReg    : t_arr_Data_Word := (others =>(others => '0'));

signal Glb_CFGAck    : std_logic := '0';
signal DataToSCU_CFG : std_logic_vector(15 downto 0) := (others => '0');
signal GlbCfgRd      : std_logic := '0';

signal Cycletimers   : std_logic_vector(4 downto 0) := (others => '0');

signal ResetRegs     : std_logic := '0';  --reset all cnts, and registers reset by external control --(s. fifo FILL_ADR written 0

signal DataToSCU_Z   : std_logic_vector(15 downto 0) := (others => '0');  -- z-bus for data read
signal TimeAct       : std_logic_vector(47 downto 0) := (others => '0');   -- local Time

signal SelTest        : std_logic_vector(31 downto 0):= (others => '0');    
signal PulsCountSelect: std_logic_vector(CHAN_NUM-1 downto 0):= (others => '0');     -- Pulse values to count for each module   

signal Count_outToFF  : std_logic_vector(31 downto 0) := (others => '0'); --Counter to be stored in FIFO

type t_arr_OutData      is array (0 to CHAN_NUM-1) of std_logic_vector(31 downto 0);
signal Count_outToFFArr : t_arr_OutData := (others =>(others => '0'));  --Counter to be stored in FIFO


---------------------------------------------------------------------------------------------
Begin


tstmux: for I in 0 to CHAN_NUM-1 generate

--switch testclocks
--mux testclocks ti inputs
process (nReset, clk,Cycletimers,PulsCountin,Glb_CFGReg)
begin
   if (nReset = '0')then     
   
      PulsCountSelect(I) <= '0'; 
      
   elsif rising_edge(clk) then
      case Glb_CFGReg(5)(2 downto 0) is
         when  "000" => PulsCountSelect(I) <= PulsCountin(I) ;
         when  "001" => PulsCountSelect(I) <= Cycletimers(0) ;
         when  "010" => PulsCountSelect(I) <= Cycletimers(1) ;
         when  "011" => PulsCountSelect(I) <= Cycletimers(2) ;
         when  "100" => PulsCountSelect(I) <= Cycletimers(3) ;
         when  "101" => PulsCountSelect(I) <= Cycletimers(4) ;
        when others =>  PulsCountSelect(I) <= PulsCountin(I) ;
      end case;         
   end if;
end process;   
   

--tstmux: for I in 0 to CHAN_NUM-1 generate
--      PulsCountSelect(I) <= PulsCountin(I) when SelTest(I)='0' else  TstClk;
end generate; 

TokQue(0)<=Cycletimers(1);  --start que check each 100us


DataToFiFo  <= TimeToFF when (CounterStoreC = all_bitsZero) else Count_outToFF;
CounterRequ <= '0' when (CounterStoreC = all_bitsZero) else '1'; --Store to FiFo when at least one CounterValue is present
ToFIFO_Requ <= TimetoFFrequ or CounterRequ;


cntloop: for I in 0 to CHAN_NUM-1 generate


Cntarr: BLM_cntarray
    generic map
    (
      CH_N => I
    )
  port map
    (
         clk               => clk,
         nReset            => nReset and not ResetRegs,

         Cycletimers       => StartStopReg(I) & Cycletimers(4 downto 0),   -- input value ech LH/HL starts new cycle
         Config            => allregisters(I),        -- set configuration --arr_allRegs   is array (0 to CHAN_NUM-1) of t_arr_Data_Word;

         Tokin             => TokQue(I),              -- Token in
         Tokout            => TokQue(I+1),            -- Token for next Module

         PulsCount         => PulsCountSelect(I),
         TimeAct           => TimeAct,                -- local Time
       
---  Save results
         CounterStore      => CounterStoreC(I),       -- Sign new counter value is available
         CounterAck        => FIFO_Ack,               -- value saved in FIFO done
         Counter_Tr        => Count_outToFF,          -- Value (31 downto 0)

         Trig_out          => TrigOutreg(I)           -- Sign threshold trigger - duration time?
 );


AdrRegs: ETB_Register
  generic map
    (--   Cycletimers(5)<= StartStopReg(I);
       Base_addr  => to_integer(unsigned(CNT_CFG_START_ADDR)+ (I*16))
    ) 
  port map
    (
    Adr_from_SCUB_LA => Adr_from_SCUB_LA,    -- latched address from SCU_Bus
    Data_from_SCUB_LA=> Data_from_SCUB_LA,   -- latched data from SCU_Bus
    Ext_Adr_Val      => Ext_Adr_Val,         -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active    => Ext_Rd_active,       -- '1' => Rd-Cycle is active
    Ext_Wr_active    => Ext_Wr_active,       -- '1' => Wr-Cycle is active
    clk              => clk,                                -- should be the same clk used by SCU_Bus_Slave
    nReset           => nReset and not ResetRegs,

    regs_out         => allregisters(I),     -- setup for counters out

    user_rd_active   => DataRegread(I),      -- '1' = read data available at 'Data_to_SCUB'-output
    Data_to_SCUB     => DataToSCUArr(I),     -- connect read sources to SCUB-Macro, Z Bus
    Dtack_to_SCUB    => DataRegsAck(I)
);

DataToSCU_Z <= DataToSCUArr(I) when DataRegread(I)='1' else (others =>'Z');

--Count_outToFF <= Count_outToFFArr(I) when CounterStoreC(I) ='1' else  (others => 'Z');


end generate;



StartStopReg<= Glb_CFGReg(2) & Glb_CFGReg(1);
SelTest     <= Glb_CFGReg(4) & Glb_CFGReg(3);

CfgRegs :ETB_Register

   generic map
    (
       Base_addr  => to_integer(unsigned(GLB_CFG_START_ADDR))
    ) 
  port map
    (
    Adr_from_SCUB_LA => Adr_from_SCUB_LA,    -- latched address from SCU_Bus
    Data_from_SCUB_LA=> Data_from_SCUB_LA,   -- latched data from SCU_Bus
    Ext_Adr_Val      => Ext_Adr_Val,         -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active    => Ext_Rd_active,       -- '1' => Rd-Cycle is active
    Ext_Wr_active    => Ext_Wr_active,       -- '1' => Wr-Cycle is active
    clk              => clk,                                -- should be the same clk used by SCU_Bus_Slave
    nReset           => nReset and not ResetRegs,

    regs_out         => Glb_CFGReg,          -- setup for counters out

    user_rd_active   => GlbCfgRd,            -- '1' = read data available at 'Data_to_SCUB'-output
    Data_to_SCUB     => DataToSCU_CFG,       -- connect read sources to SCUB-Macro, Z Bus
    Dtack_to_SCUB    => Glb_CFGAck
);



rd_mux:process (nReset, clk,DataRegsAck,DataToSCUArr,DataToSCU_CFG,DataToSCUFF,Glb_CFGAck,DataFFAck,DataFFread,DataRegread,GlbCfgRd)
begin
   if (nReset = '0')then

      Data_to_SCUB   <= (others => '0');
      user_rd_active <= '0';
      Dtack_to_SCUB  <= '0';

   elsif rising_edge(clk) then

      Dtack_to_SCUB  <= '0';
      if (DataRegsAck/=all_bitsZero) then
         Dtack_to_SCUB <= '1';        
      else
         Dtack_to_SCUB <= DataFFAck or Glb_CFGAck;
      end if;

      Data_to_SCUB <= (others => '0');
      user_rd_active <= '0';

      if (DataRegread/=all_bitsZero) then
         Data_to_SCUB <= DataToSCU_Z;
         user_rd_active <= '1';
      elsif  DataFFread='1' then
         Data_to_SCUB <= DataToSCUFF;
         user_rd_active <= '1';
      elsif  GlbCfgRd ='1' then
         Data_to_SCUB <= DataToSCU_CFG;
         user_rd_active <= '1';
      end if;
   end if; --clk
end process rd_mux;


-- Generate TimeEvents 100us 1ms, 10ms, 100ms, 1sec
--   Generate TimeEvents (0)-100us (1)-1ms, (2)-10ms, (3)-100ms, (4)-1sec
-- Input based on 1us timer
BLM_Baseclks: BLM_Tbase

  port map
    (
      clk         => clk,
      nReset      => nReset,
      Tclk1us     => Tclk1us,
      Cycletimers => Cycletimers(4 downto 0),   -- output value each LH/HL starts new cycle

      Glb_CFGReg  => Glb_CFGReg(0),
      
      TimeAct      =>TimeAct,

      Tokin       => TokQue(CHAN_NUM-1),        -- Token in from last output
      Tokout      => OPEN,                      -- final output - end of token que

      TimetoFF    => TimetoFFrequ,              -- sign new value is available to be stored in FIFO
      TimeVal     => TimeToFF,                  -- Timestamp to be storen in FIFO
      TimeAck     => FIFO_Ack                   -- value saved in FIFO done
);


--FIFO
--Store counter ValuesCounterAck
--access to FIFO buffer by scu


Fifoio: BLM_FIFO
    generic map
    (
       FIFO_Start_addr  => to_integer(unsigned(FIFO_START_ADDR))
    )

  port map
  (
    Adr_from_SCUB_LA => Adr_from_SCUB_LA,    -- latched address from SCU_Bus
    Data_from_SCUB_LA=> Data_from_SCUB_LA,   -- latched data from SCU_Bus
    Ext_Adr_Val      => Ext_Adr_Val,         -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active    => Ext_Rd_active,       -- '1' => Rd-Cycle is active
    Ext_Wr_active    => Ext_Wr_active,       -- '1' => Wr-Cycle is active
    clk              => clk,                 -- should be the same clk, used by SCU_Bus_Slave
    nReset           => nReset,

    DatatoFIFOrequ   => ToFIFO_Requ,         -- request Counter to be saved in fifo
    DataFIFOin       => DataToFiFo,          -- all fifo Data ( 31..0) to be saved
    FIFOAckout       => FIFO_Ack,            -- value saved in FIFO done
    ResetRegs        => ResetRegs,           -- reset all BLM regs 

    user_rd_active   => DataFFread,          -- '1' = read data available at 'Data_to_SCUB'-output
    Data_to_SCUB     => DataToSCUFF,         -- connect read sources to SCUB-Macro
    Dtack_to_SCUB    => DataFFAck
    );



end BLM_Head;
