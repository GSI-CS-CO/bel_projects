----------------------------------------------------------------------------------
-- Company: GSI
-- Engineer: V. Kleipa
--
-- Create Date:
-- Design Name:
-- Module Name:    IF_Mode_a.vhd - Behavioral
-- Project Name:  A6408_decoder
-- Target Devices:
-- Tool versions:
-- Description:
-- Steuert den 6408 decoder
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- Umwandlung des alten TDFs nach VHDL und
-- zusätzlich ist DSC nicht mehr als Taktquelle für die Register ---> keine Timing-Probleme mehr


LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE IEEE.STD_LOGIC_arith.all;
USE IEEE.STD_LOGIC_unsigned.all;


entity A6408_decoder is

  Port
  (
  --system controls
    sys_clk    : in  STD_LOGIC;  -- systemclock
    sys_reset  : in  STD_LOGIC;
    enable     : in  STD_LOGIC;  -- '1' - enable this modul

    -- decode data controls
    DSC        : in  STD_LOGIC;  -- Decoder-Shift-Clock
    SDO        : in  STD_LOGIC;  -- serielle Daten vom 6408
    TD         : in  STD_LOGIC;  -- Take-Data -Frame für datenbits vom MIL-Baustein
    CDS        : in  STD_LOGIC;  -- Data/Command Worttyp vom MIL-Baustein
    VW         : in  STD_LOGIC;  -- Valid Word
    Res_VW_Err : in  STD_LOGIC;  -- Reset Fehlerspeicher für VW-Test --NC?

---
    -- decoded signals
    rcv_ready  : out std_logic; -- Strobe für neue gültige Daten
    cds_out    : out std_logic; -- Data/Command Wort empfangen 0-Data 1- command
    mil_rcv_d  : out std_logic_vector(15 downto 0); -- Interne Daten
    nrcv_en    : out std_logic;  --?  -- Enable Signal zur Empfänger-Freigabe
    rcv_err    : out std_logic;  --default 0 ? --

    dec_busy   : out STD_LOGIC    -- ongoing decoding, active from TD to VW end

  );

end A6408_decoder;


architecture Behavioral of A6408_decoder is

signal DSC1             : STD_LOGIC := '0';  --DSC clk Speicher für für HL/LH detection
signal v_Data_in        : std_logic_vector(15 downto 0)  := ( others =>'0'); --Eingansschieberegister
signal v_VW_Count       : std_logic_vector(4 downto 0)   := ( others =>'0'); --
signal v_VW_Count_syn   : std_logic_vector(4 downto 0)   := ( others =>'0'); --
signal v_Di             : std_logic_vector(15 downto 0)  := ( others =>'0'); --Speicher Eingansschieberegister

signal VW_ok            : STD_LOGIC := '0';
signal VW_ok1           : STD_LOGIC := '0';
signal VW_ok2           : STD_LOGIC := '0';
signal v_VW_OK_syn      : STD_LOGIC := '0';


begin

rcv_err<='0';

--generiert DSC HL Detection
DSCClk:process (sys_reset,sys_clk,DSC)
begin
   if (sys_reset = '1') then
      DSC1 <= '0';
   elsif rising_edge(sys_clk) then
      DSC1 <= DSC;
   end if;
end process;


--+----------------------------------------------------------------------------------------------------------------
--| Shift-Reg. für Empfangene Daten und SYC -Typ (CDS)
--+----------------------------------------------------------------------------------------------------------------
--v_Data_in.   (clock, shiftin, sclr)     = ((DSC AND TD), SDO, SCLR);
--v_CMD_Word.  (D, CLK, CLRN) = (CDS, (TD AND DSC), NOT SCLR);
-- Daten vom MIL-Device holen
SerinSd:process (sys_reset,sys_clk,DSC,DSC1,v_Data_in,SDO,CDS)

  begin

   if (sys_reset = '1') then
      v_Data_in <= (others =>'0');
      cds_out <= '0';               --sync data oder command anzeiger
   elsif rising_edge(sys_clk) then
      if TD = '1' then           --shifter so lange laden bis SD vom 6408 aktiv wird
         if DSC = '1' and DSC1 ='0' then --DSC LH detected ?
            v_Data_in <= v_Data_in(14 downto 0) & SDO;  --daten einschiften
            cds_out<=CDS;           --sync data oder command anzeiger setzen
         end if;
      end if;
    end if;
end process;

--+==========================================================================
--| Es wird geprüft ob VaildWord innerhalb 16-18us nach TakeData kommt.
--+==========================================================================

-- v_VW_Count[].clk  = DSC;
-- v_VW_Count[].clrn = NOT SCLR;
--
-- IF v_VW_Count[] == c_end THEN
--       v_VW_Count[] = 0;
-- ELSIF TD OR (v_VW_Count[] != 0) THEN
--       v_VW_Count[] = v_VW_Count[] + 1;
-- ELSE
--       v_VW_Count[] = v_VW_Count[];
-- END IF;
--

VWtest:process (sys_reset,sys_clk,DSC,DSC1,v_VW_Count,TD)
 begin

   if (sys_reset = '1') then
      v_VW_Count <= ( others =>'0');
   elsif rising_edge(sys_clk) then
      if DSC = '1' and DSC1 ='0' then --DSC LH detected ?
         if v_VW_Count = "10010" then --18
            v_VW_Count <= ( others =>'0');
         elsif TD='1' or v_VW_Count /= "00000" then
            v_VW_Count <= v_VW_Count+'1';
         end if;
      end if;  -- if DSC = '1' and DSC1 ='0'
   end if; --if risig_edge

 end process;


 ----- Test ob während der Zähler auf 17 steht, die pos. Flanke von VW kommt --
--
--v_VW_OK. (D, CLK)      = ( ((v_VW_Cnt_Syn[] == c_vw_cnt) & VW), CLK);
--

----- Zeitverzögerung und Synchronisation ---
--v_VW_OK_t1. (D, CLK)      = ( v_VW_OK.q,    CLK);   -- Zeitvez�gerung
--v_VW_OK_t2. (D, CLK)      = ( v_VW_OK_t1.q, CLK);   -- Zeitvez�gerung
--v_VW_OK_syn. (D, CLK)     = ( v_VW_OK_t1.q AND NOT v_VW_OK_t2.q , CLK); --- Synchronisation
--v_VW_Cnt_Syn[]           =  v_VW_Count[];
--v_VW_Cnt_Syn[].(CLK, CLRN)  = ( clk, NOT sclr);-- Sychrone Z�hler-Bit's
 VWsync:process (sys_reset,sys_clk,DSC,DSC1,v_VW_Count,TD,VW,VW_ok2,VW_ok1)
 begin

   if (sys_reset = '1') then
      v_VW_Count_syn <= ( others =>'0');
      VW_ok <= '0';
   elsif rising_edge(sys_clk) then
      VW_ok1<=VW_ok;
      VW_ok2<=VW_ok1;
      v_VW_Count_syn <= v_VW_Count;

      if v_VW_Count_syn = "10001" and VW = '1' then --17
         VW_ok <= '1';
      else
         VW_ok <= '0';
      end if;
     -- v_VW_OK_syn <= (VW_ok2='0' and VW_ok1 ='1');

      if VW_ok2='0' and VW_ok1 ='1' then
         v_VW_OK_syn <= '1';
      else
         v_VW_OK_syn <= '0';
      end if;
--
   end if; --if risig_edge
end process;

rcv_ready <= v_VW_OK_syn;

dec_busy  <= TD or not VW; -- hier den Wert noch überpüfen vk?

---------------------------- Output-Daten-Register --------------------------------------
--v_DI     .(data[], clock, enable, sclr) = (v_Data_in.q[], clk, vw, SCLR);
VWDIN:process (sys_reset,sys_clk,v_Data_in,VW)
 begin

   if (sys_reset = '1') then
      v_DI <= ( others =>'0');
   elsif rising_edge(sys_clk) then
      if VW ='1' then --
         v_DI <= v_Data_in; --Schieberegister kopieren
      end if;  -- if DSC = '1' and DSC1 ='0'
   end if; --if risig_edge

end process;

mil_rcv_d<=v_DI;
nrcv_en <= '0';


end Behavioral;  --MIL Device encoder


--signal  wurde nicht verwendet
