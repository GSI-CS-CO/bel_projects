


library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


package BLM_counter_pkg is

CONSTANT CHAN_NUM    : INTEGER := 30;   -- number of channels , max is 30
CONSTANT TIMER_END   : std_logic_vector(15 downto 0) := x"00BF";



CONSTANT BLM_BASE_ADDR:    unsigned(15 downto 0)    := x"0800"; -- BLM
CONSTANT BLM_Base_Addrx:   unsigned(15 downto 0)    := x"0800"; -- BLM

CONSTANT GCFG_REG_OFFS   : unsigned(15 downto 0) := x"0180";
CONSTANT FIFO_REG_OFFS   : unsigned(15 downto 0) := x"01F0";

--new
--CONSTANT GCFG_REG_OFFS   : std_logic_vector(15 downto 0) := x"01E0";
--CONSTANT FIFO_REG_OFFS   : std_logic_vector(15 downto 0) := x"01FD";



CONSTANT CNT_CFG_START_ADDR   :    unsigned(15 downto 0)    := x"0000"   + BLM_BASE_ADDR;  -- Registerset for Counter thresholds etc.
CONSTANT GLB_CFG_START_ADDR   :    unsigned(15 downto 0)    := GCFG_REG_OFFS + BLM_BASE_ADDR;  -- Global Configuration register + Start stop register
CONSTANT FIFO_START_ADDR      :    unsigned(15 downto 0)    := FIFO_REG_OFFS + BLM_BASE_ADDR;  -- FIFO readout Register


constant INSTRUCTION_BUFFER_ADDRESS : integer := 4;  --bits wide
constant INSTRUCTION_BUFFER_DATA    : integer := 16; --bits wide

type RegisterPuffer     is array(0 to 2**INSTRUCTION_BUFFER_ADDRESS -1) of std_logic_vector (INSTRUCTION_BUFFER_DATA -1 downto 0);

type t_arr_FiFoData     is array (0 to 5)  of std_logic_vector(31 downto 0);
type t_arr_FF           is array (0 to 15) of t_arr_FiFoData;

type t_arr_CntData      is array (0 to 5)  of std_logic_vector(31 downto 0);
type t_arr_Trigout      is array (0 to CHAN_NUM-1) of std_logic_vector(5 downto 0);
type t_arr_Trigoutmax   is array (0 to 23) of std_logic_vector(5 downto 0);

type t_arr_Data_Word    is array (0 to 15) of std_logic_vector(15 downto 0);
type t_arr_allRegs      is array (0 to CHAN_NUM-1) of t_arr_Data_Word;
type t_arr_Data_Word_C  is array (0 to CHAN_NUM-1) of std_logic_vector(15 downto 0);

TYPE t_IO_Reg_1_to_7_Array is array (1 to 7)  of std_logic_vector(15 downto 0);
TYPE t_IO_Reg_0_to_7_Array is array (0 to 7)  of std_logic_vector(15 downto 0);


component BLM_FIFO is
    generic
    (
      FIFO_Start_addr  : Integer := 0
    );

  port
    (
    Adr_from_SCUB_LA:   in    std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
    Data_from_SCUB_LA:  in    std_logic_vector(15 downto 0);  -- latched data from SCU_Bus
    Ext_Adr_Val:        in    std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:      in    std_logic;                      -- '1' => Rd-Cycle is active
    Ext_Wr_active:      in    std_logic;                      -- '1' => Wr-Cycle is active
    clk:                in    std_logic;                      -- should be the same clk, used by SCU_Bus_Slave
    nReset:             in    std_logic := '1';

    DatatoFIFOrequ    : in    std_logic;                       -- request signals to be saved in fifo    
    DataFIFOin        : in    std_logic_vector(31 downto 0);   -- all fifo Data ( 31..0) to be saved
    FIFOAckout        : out   std_logic;                       -- Ack save request / value saved in FIFO done
    ResetRegs         : out   std_logic;                       -- Reset all BLM Regs - active HIGH

    user_rd_active:     out   std_logic;                      -- '1' = read data available at 'Data_to_SCUB'-output
    Data_to_SCUB:       out   std_logic_vector(15 downto 0);  -- connect read sources to SCUB-Macro
    Dtack_to_SCUB:      out   std_logic
    );
end component BLM_FIFO;



component BLM_Counter is

  Generic
    (
      CH_N     : INTEGER := 0;  -- which channel
      CNT_TYPE : INTEGER := 0   -- 0 no divider and 100ys, 1-4 standard counter , 5 - with divider   and start stop
    );

  port
    (
    clk           : in    std_logic;                      -- should be the same clk, used by SCU_Bus_Slave
    nReset        : in    std_logic := '1';

    Puls_in       : in    std_logic := '0';               -- Input pulse
    cycle         : in    std_logic := '0';               -- input value each LH/HL starts new cycle
    
    SS_scale      : in    std_logic_vector(3 downto 0);  -- counter scale value
    Threshold     : in    std_logic_vector(31 downto 0);  -- Threshold value
    TimeAct       : in    std_logic_vector(47 downto 0);   -- local Time
    
    Tokin         : in    std_logic := '0';               -- Token in
    Tokout        : out   std_logic := '0';               -- Token for next Module

    EventtoFIFO_en: in    std_logic := '0';               -- enable event to FIFO storage
    
    FIFO_out_en   : in    std_logic := '0';               -- enable Counter FIFO storage
    FiFO_wr_req   : out   std_logic := '0';               -- sign new counter value is availiable to be stored in FIFO
    NewFIFOVal    : out   std_logic_vector(31 downto 0);  -- counter at / stop/ trigger
    FFwrACK_in    : in    std_logic := '0';               -- acknowledge counter value procSelTest2         => SelTest2,  essed
            
    Trig_out_en   : in    std_logic := '0';               -- enable Trigger signal
    Trig_out      : out   std_logic := '0'
    );
  end component;


component BLM_cntarray is
   generic
    (
      CH_N    :  INTEGER := 0                -- which channel
    );
  port
    (
    clk			: in    std_logic;                      -- should be the same clk, used by SCU_Bus_Slave
    nReset		: in    std_logic := '1';

    PulsCount	: in    std_logic := '0';               -- Input pulse
    Cycletimers: in    std_logic_vector(5 downto 0);   -- input value ech LH/HL starts new cycle 0 -10ys, ..4->100ms and (5)startstop

    Tokin   	: in    std_logic := '0';               -- Token in
    Tokout  	: out   std_logic := '0';              -- Token for next Module

    Config		: in    t_arr_Data_Word;                -- configuration Register array (address 00-0xF)
    TimeAct       :     in   std_logic_vector(47 downto 0);   -- local Time

 --   Trig_out_en: in    std_logic_vector(5 downto 0);    -- enable Trigger signal

    CounterStore:out   std_logic := '0';                -- sign new counter value is availiable
    CounterAck	: in    std_logic := '0';                -- acknowledge counter value processed

   Counter_Tr	: out   std_logic_vector(31 downto 0);   -- counter at / stop/ trigger/threshold
   Trig_out		: out   std_logic_vector(5 downto 0)     -- Sign threshold trigger - duration time?

 );
end component;



component ETB_Register is

   generic
       (
         Base_addr : Integer := 0;
         Reg_Length: Integer := 16     --range
       );

     port
       (
       Adr_from_SCUB_LA:   in    std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
       Data_from_SCUB_LA:  in    std_logic_vector(15 downto 0);  -- latched data from SCU_Bus
       Ext_Adr_Val:        in    std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
       Ext_Rd_active:      in    std_logic;                      -- '1' => Rd-Cycle is active
       Ext_Wr_active:      in    std_logic;                      -- '1' => Wr-Cycle is active
       clk:                in    std_logic;                      -- should be the same clk, used by SCU_Bus_Slave
       nReset:             in    std_logic := '1';

       regs_out          :  out   t_arr_Data_Word;

       user_rd_active:     out   std_logic;                      -- '1' = read data available at 'Data_to_SCUB'-output
       Data_to_SCUB:       out   std_logic_vector(15 downto 0);  -- connect read sources to SCUB-Macro
       Dtack_to_SCUB:      out   std_logic
       );
end component;


COMPONENT BLM_Head

  generic
    (
      BLM_Start_addr : unsigned(15 downto 0)    := x"0000"
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
    Tclk1us:            in    std_logic;                      -- 1us clock

    PulsCountin:        in    std_logic_vector(CHAN_NUM-1 downto 0);      -- Pulse values to count for each module
    TrigOutreg:         out   t_arr_Trigout;                  -- FBAS trigger out

    user_rd_active:     out   std_logic;                      -- '1' = read data avaliable at 'Data_to_SCUB'-output
    Data_to_SCUB:       out   std_logic_vector(15 downto 0);  -- consignal S_Dtack        : std_logic;
    Dtack_to_SCUB:      out   std_logic
    );
END COMPONENT BLM_Head;


COMPONENT BLM_Tbase

  port
    (
    clk           :     in    std_logic;                       -- should be the same clk, used by SCU_Bus_Slave
    nReset        :     in    std_logic := '1';

    Tclk1us       :     in    std_logic := '1';                -- 1us clock input
    Cycletimers   :     out   std_logic_vector(4 downto 0);    -- generate 100us, 1ms 10ms 100ms, 1sec  timing pulses

    Glb_CFGReg    :     in    std_logic_vector(15 downto 0);   -- config
    TimeAct       :     out   std_logic_vector(47 downto 0);   -- sign new counter value is available

    Tokin         :     in    std_logic := '0';                -- Token in
    Tokout        :     out   std_logic := '0';                -- Token for next Module

    TimetoFF      :     out   std_logic := '0';                -- sign new counter value is available    
    TimeVal       :     out   std_logic_vector(31 downto 0);   -- counter at / stop/ trigger
    TimeAck       :     in    std_logic := '0'                 -- acknowledge counter value processed
    );
    
END COMPONENT BLM_Tbase;


 component diob_debounce
  generic (
    DB_Tst_Cnt: integer := 3;
    Test:       integer range 0 TO 1);
  port (
    DB_Cnt:     in  integer range 0 to 16383;
    DB_In:      in  std_logic;
    Reset:      in  std_logic;
    Clk:        in  std_logic;
    DB_Out:     out std_logic
    );
  end component diob_debounce;


component config_status
  generic ( CS_Base_addr  : integer );
  port (
    Adr_from_SCUB_LA:     in   std_logic_vector(15 downto 0);    -- latched address from SCU_Bus
    Data_from_SCUB_LA:    in   std_logic_vector(15 downto 0);    -- latched data from SCU_Bus
    Ext_Adr_Val:          in   std_logic;                        -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:        in   std_logic;                        -- '1' => Rd-Cycle is active
    Ext_Rd_fin:           in   std_logic;                        -- marks end of read cycle, active one for one clock period of sys_clk
    Ext_Wr_active:        in   std_logic;                        -- '1' => Wr-Cycle is active
    Ext_Wr_fin:           in   std_logic;                        -- marks end of write cycle, active one for one clock period of sys_clk
    clk:                  in   std_logic;                        -- should be the same clk, used by SCU_Bus_Slave
    nReset:               in   std_logic;

    Diob_Status1:         in   std_logic_vector(15 downto 0);    -- Input-Port 1
    Diob_Status2:         in   std_logic_vector(15 downto 0);    -- Input-Port 2
    AW_Status1:           in   std_logic_vector(15 downto 0);    -- Input-Port 3
    AW_Status2:           in   std_logic_vector(15 downto 0);    -- Input-Port 4

    Diob_Config1:         out  std_logic_vector(15 downto 0);    -- Daten-Reg. AWOut1
    Diob_Config2:         out  std_logic_vector(15 downto 0);    -- Daten-Reg. AWOut2
    AW_Config1:           out  std_logic_vector(15 downto 0);    -- Daten-Reg. AWOut3
    AW_Config2:           out  std_logic_vector(15 downto 0);    -- Daten-Reg. AWOut4

    Mirr_OutReg_Maske:    out  std_logic_vector(15 downto 0);    -- Maskierung für Spiegel-Modus des Ausgangsregisters

    Diob_Config1_wr:      out  std_logic;                        -- write-Strobe, Daten-Reg. AWOut1
    Diob_Config2_wr:      out  std_logic;                        -- write-Strobe, Daten-Reg. AWOut2
    AW_Config1_wr:        out  std_logic;                        -- write-Strobe, Daten-Reg. AWOut3
    AW_Config2_wr:        out  std_logic;                        -- write-Strobe, Daten-Reg. AWOut4
    Clr_Tag_Config:       out  std_logic;                        -- Clear Tag-Konfigurations-Register

    Rd_active:            out  std_logic;                        -- read data available at 'Data_to_SCUB'-AWOut
    Data_to_SCUB:         out  std_logic_vector(15 downto 0);    -- connect read sources to SCUB-Macro
    Dtack_to_SCUB:        out  std_logic;                        -- connect Dtack to SCUB-Macro
    LA:                   out  std_logic_vector(15 downto 0)
      );
end component config_status;



COMPONENT io_reg
  GENERIC ( Base_addr : INTEGER );
  PORT
  (
    Adr_from_SCUB_LA:    IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
    Data_from_SCUB_LA:   IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
    Ext_Adr_Val:         IN  STD_LOGIC;
    Ext_Rd_active:       IN  STD_LOGIC;
    Ext_Rd_fin:          IN  STD_LOGIC;
    Ext_Wr_active:       IN  STD_LOGIC;
    Ext_Wr_fin:          IN  STD_LOGIC;
    clk:                 IN  STD_LOGIC;
    nReset:              IN  STD_LOGIC;
    Reg_IO1:             OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_IO2:             OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_IO3:             OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_IO4:             OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_IO5:             OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_IO6:             OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_IO7:             OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_IO8:             OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_rd_active:       OUT STD_LOGIC;
    Data_to_SCUB:        OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    Dtack_to_SCUB:       OUT STD_LOGIC
  );
END COMPONENT io_reg;

COMPONENT in_reg
  GENERIC ( Base_addr : INTEGER );
  PORT
  (
    Adr_from_SCUB_LA:    IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
    Data_from_SCUB_LA:   IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
    Ext_Adr_Val:         IN  STD_LOGIC;
    Ext_Rd_active:       IN  STD_LOGIC;
    Ext_Rd_fin:          IN  STD_LOGIC;
    Ext_Wr_active:       IN  STD_LOGIC;
    Ext_Wr_fin:          IN  STD_LOGIC;
    clk:                 IN  STD_LOGIC;
    nReset:              IN  STD_LOGIC;
    Reg_In1:             IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_In2:             IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_In3:             IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_In4:             IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_In5:             IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_In6:             IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_In7:             IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_In8:             IN  STD_LOGIC_VECTOR(15 DOWNTO 0);
    Reg_rd_active:       OUT STD_LOGIC;
    Data_to_SCUB:        OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    Dtack_to_SCUB:       OUT STD_LOGIC
  );
END COMPONENT in_reg;


component pu_reset
generic (
    PU_Reset_in_clks : integer
    );
port  (
    Clk:      in    std_logic;
    PU_Res:   out   std_logic
    );
end component;


end package;
