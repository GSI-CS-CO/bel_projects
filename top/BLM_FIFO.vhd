-- FIFO Interface
--Todos: add Info frames (see 3.2 )

library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

USE work.BLM_counter_pkg.all;


entity BLM_FIFO is
    generic
    (
      FIFO_Start_addr  : Integer := 16#09F0#
    );

  port
    (
    Adr_from_SCUB_LA : in    std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
    Data_from_SCUB_LA: in    std_logic_vector(15 downto 0);  -- latched data from SCU_Bus
    Ext_Adr_Val      : in    std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active    : in    std_logic;                      -- '1' => Rd-Cycle is active
    Ext_Wr_active    : in    std_logic;                      -- '1' => Wr-Cycle is active
    clk              : in    std_logic;                      -- should be the same clk, used by SCU_Bus_Slave
    nReset           : in    std_logic := '1';    

    DatatoFIFOrequ   : in    std_logic;                      -- request signal of data to be saved in FIFO - 
    DataFIFOin       : in    std_logic_vector(31 downto 0);  -- new FIFO Data ( 31..0) to be saved
    FIFOAckout       : out   std_logic;                      -- Ack save request / value saved in FIFO done - one cycle
    ResetRegs        : out   std_logic;                       -- Reset all BLM Regs - active HIGH

    user_rd_active   : out   std_logic;                      -- '1' = read data available at 'Data_to_SCUB'-output
    Data_to_SCUB     : out   std_logic_vector(15 downto 0);  -- connect read sources to SCUB-Macro
    Dtack_to_SCUB    : out   std_logic
    );

end BLM_FIFO;


architecture  BLM_FIFO of BLM_FIFO is

CONSTANT ADDR_WIDTH     :  INTEGER := Adr_from_SCUB_LA'length;
CONSTANT FIFO_WIDTH     :  INTEGER := 8;  -- 2^n words in fifo

CONSTANT FIFO_STATUS_R  :  unsigned (ADDR_WIDTH-1  downto 0) := to_unsigned(FIFO_Start_addr+0, ADDR_WIDTH); --for test purposes
CONSTANT FIFO_FILL_RW   :  unsigned (ADDR_WIDTH-1  downto 0) := to_unsigned(FIFO_Start_addr+1, ADDR_WIDTH);  -- write to  will flush fifo
CONSTANT FIFO_HIGH_R    :  unsigned (ADDR_WIDTH-1  downto 0) := to_unsigned(FIFO_Start_addr+2, ADDR_WIDTH);
CONSTANT FIFO_LOW_R     :  unsigned (ADDR_WIDTH-1  downto 0) := to_unsigned(FIFO_Start_addr+3, ADDR_WIDTH);
CONSTANT FIFO_FLUSH     :  unsigned (ADDR_WIDTH-1  downto 0) := to_unsigned(FIFO_Start_addr+4, ADDR_WIDTH);  -- write to  will flush fifo

--for test purposes
--register not in doc
CONSTANT FIFO_TSTREG    :  unsigned (ADDR_WIDTH-1  downto 0) := to_unsigned(FIFO_Start_addr+5, ADDR_WIDTH); -- will transmit a 0xAA55



component BLM_FIFO_IP IS
   PORT
   (
      aclr     : IN STD_LOGIC ;
      clock    : IN STD_LOGIC ;
      data     : IN STD_LOGIC_VECTOR (31 DOWNTO 0);
      rdreq    : IN STD_LOGIC ;
      wrreq    : IN STD_LOGIC ;
      empty    : OUT STD_LOGIC ;
      full     : OUT STD_LOGIC ;
      q        : OUT STD_LOGIC_VECTOR (31 DOWNTO 0);
      usedw    : OUT STD_LOGIC_VECTOR (FIFO_WIDTH-1 DOWNTO 0)
   );
END component BLM_FIFO_IP;


 ---write io
signal S_Dtack    : STD_LOGIC := '0';
signal aclr       : STD_LOGIC := '0';
signal data       : STD_LOGIC_VECTOR (31 DOWNTO 0):= (others =>'0');
signal rdreq      : STD_LOGIC := '0';     --switch to next FIFO word
signal wrreq      : STD_LOGIC := '0';
signal q          : STD_LOGIC_VECTOR (31 DOWNTO 0):= (others =>'0');
signal full       : STD_LOGIC := '0';
signal empty      : STD_LOGIC := '0';
signal usedw      : STD_LOGIC_VECTOR (FIFO_WIDTH-1 DOWNTO 0):= (others =>'0');
signal AllowLSW   : STD_LOGIC := '0';

type t_StateWr is (WRstate0, WRstate1, WRstate2);
signal StateWr    : t_StateWr := WRstate0;

signal AddrSCU    :  unsigned(ADDR_WIDTH-1 downto 0) := to_unsigned((0), ADDR_WIDTH);

--signal NextFifo   :  STD_LOGIC  := '0'; -- request next Fifo read
signal FiFoStatus :  STD_LOGIC_VECTOR(15 DOWNTO 0) := (others =>'0');
signal LSWLocal   :  STD_LOGIC_VECTOR(15 downto 0) := (others =>'0'); --local register of LSW to be read by SCU

-----------------------------------------------------------------------------------

begin

AddrSCU <= unsigned(Adr_from_SCUB_LA);

Dtack_to_SCUB <= S_Dtack;

P_Adr_ff: process (nReset, clk,Ext_Adr_Val,Ext_Rd_active,AddrSCU,AllowLSW,FiFoStatus,usedw,LSWLocal)
  begin

    if nReset = '0' then
      Data_to_SCUB   <= (others =>'0');
      S_Dtack        <= '0';
        
      user_rd_active <= '0';
      --NextFifo       <= '0';
      FiFoStatus     <= (others =>'0');
      aclr           <= '0';    
      AllowLSW       <= '0';
      rdreq          <= '0';
      LSWLocal       <= (others =>'0');
      ResetRegs      <= '0';

    elsif rising_edge(clk) then

      aclr           <= '0';
      S_Dtack        <= '0';
      user_rd_active <= '0';
      ResetRegs      <= '0';
     -- Data_to_SCUB   <= (others =>'0');
      --NextFifo       <= '0';
      FiFoStatus(0)  <= empty;
      FiFoStatus(1)  <= full;
      rdreq          <= '0';

      if Ext_Adr_Val = '1' then
            if Ext_Rd_active = '1' then
                case AddrSCU is
                  when FIFO_STATUS_R =>
                                S_Dtack <= '1';
                                user_rd_active <= '1';
                                Data_to_SCUB <= FiFoStatus;

                  when FIFO_FILL_RW =>
                                S_Dtack <= '1';
                                user_rd_active <= '1';
                                Data_to_SCUB   <= (others =>'0'); --preset not used bits
                                Data_to_SCUB(FIFO_WIDTH-1 DOWNTO 0) <= usedw(FIFO_WIDTH-1 DOWNTO 0);

                  when FIFO_HIGH_R => -- use pre-read FIFO to fix the actuel 32bit value
                                S_Dtack <= '1';
                                user_rd_active <= '1';
                                Data_to_SCUB <= q(31 downto 16);
                                LSWLocal     <= q(15 downto 0); --save LSW too
                                AllowLSW<='1';

                  when FIFO_LOW_R =>
                                S_Dtack <= '1';
                                user_rd_active <= '1';                           
                                Data_to_SCUB <= LSWLocal;
                                if AllowLSW = '1' then                                  
                                    rdreq    <= '1'; --switch to next FIFO word
                                end if;
                                AllowLSW <= '0';

                    when FIFO_TSTREG =>
                                S_Dtack <= '1';
                                user_rd_active <= '1';
                                Data_to_SCUB <= x"aa55";
                    
                    when others =>
                                Data_to_SCUB <=(others =>'0');
                                
                end case; --case AddrSCU is
            end if; -- if Ext_Rd_active = '1' then

            --force reset fifo by writing 0 to FIFO_FILL Register
            if Ext_Wr_active = '1' then
                if (AddrSCU = FIFO_FLUSH) then
                    if Data_from_SCUB_LA = x"0000" then
                        aclr     <= '1';
                        S_Dtack  <= '1';
                        ResetRegs <= '1';
                    end if;
                end if;
            end if; --if Ext_Wr_active = '1' then

      end if; --  if Ext_Adr_Val = '1' then
   end if; --if rising_edge(clk) then
end process;


-- FIFO connection
-- Data Out is pre-read
myFifo: BLM_FIFO_IP
port map (
      aclr     => aclr,-- and not nReset,
      data     => DataFIFOin,
      clock    => clk,
      rdreq    => rdreq,
      wrreq    => wrreq,
      q        => q,
      empty    => empty,
      full     => full,
      usedw  =>   usedw(FIFO_WIDTH-1 DOWNTO 0)
);

---------------------------
--Write data to fifo
WritetoFF: process (nReset, clk,StateWr,DatatoFIFOrequ)

  begin

    if nReset = '0' then

      wrreq       <= '0';
      StateWr     <= WRstate0;
      FIFOAckout  <= '0';

    elsif rising_edge(clk) then

        wrreq       <= '0';

        case StateWr is
            when WRstate0 =>
                          FIFOAckout  <= '0';
                          if DatatoFIFOrequ = '1' then --write to FIFO request
                            StateWr<= WRstate1;
                            wrreq  <= '1';
                          end if ;

            when WRstate1 =>
                           FIFOAckout <= '1'; -- sign done to Request
                           StateWr <= WRstate2;

            when WRstate2 => 
                           if DatatoFIFOrequ = '0' then --request must  also be resetted
                              FIFOAckout  <= '0';         --also reset ack
                              StateWr <= WRstate0;
                           end if;

            when others => 
                           StateWr <= WRstate0;
        end case;

     end if;
end process;

end BLM_FIFO;
