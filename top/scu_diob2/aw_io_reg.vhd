--TITLE "'aw_io_reg' Autor: R.Hartmann, Stand: 20.03.2015";
--

library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
--USE IEEE.std_logic_arith.all;

library work;
use work.scu_diob_pkg.all;



ENTITY aw_io_reg IS
  generic
      (
  CLK_sys_in_Hz:   integer := 125000000;
  AW_Base_addr:    INTEGER := 16#0510#  );
    
  port(
    Adr_from_SCUB_LA:     in   std_logic_vector(15 downto 0);    -- latched address from SCU_Bus
    Data_from_SCUB_LA:    in   std_logic_vector(15 downto 0);    -- latched data from SCU_Bus 
    Ext_Adr_Val:          in   std_logic;                        -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:        in   std_logic;                        -- '1' => Rd-Cycle is active
    Ext_Rd_fin:           in   std_logic;                        -- marks end of read cycle, active one for one clock period of sys_clk
    Ext_Wr_active:        in   std_logic;                        -- '1' => Wr-Cycle is active
    Ext_Wr_fin:           in   std_logic;                        -- marks end of write cycle, active one for one clock period of sys_clk
    clk:                  in   std_logic;                        -- should be the same clk, used by SCU_Bus_Slave
    Ena_every_1us:        in   std_logic;                        -- Clock-Enable-Puls alle Mikrosekunde, 1 Clock breit
    nReset:               in   std_logic;

    SCU_AW_Input_Reg:     in   t_IO_Reg_1_to_7_Array;    
    SCU_AW_Output_Reg:    out  t_IO_Reg_1_to_7_Array;

    AWOut_Reg1_wr:        out  std_logic;                      -- Daten-Reg. AWOut1
    AWOut_Reg2_wr:        out  std_logic;                      -- Daten-Reg. AWOut2
    AWOut_Reg3_wr:        out  std_logic;                      -- Daten-Reg. AWOut3
    AWOut_Reg4_wr:        out  std_logic;                      -- Daten-Reg. AWOut4
    AWOut_Reg5_wr:        out  std_logic;                      -- Daten-Reg. AWOut5
    AWOut_Reg6_wr:        out  std_logic;                      -- Daten-Reg. AWOut6
    AWOut_Reg7_wr:        out  std_logic;                      -- Daten-Reg. AWOut7
    
    Rd_active:            out  std_logic;                        -- read data available at 'Data_to_SCUB'-AWOut
    Data_to_SCUB:         out  std_logic_vector(15 downto 0);    -- connect read sources to SCUB-Macro
    Dtack_to_SCUB:        out  std_logic;                        -- connect Dtack to SCUB-Macro
    LA:                   out  std_logic_vector(15 downto 0)
    );  
  end aw_io_reg;


ARCHITECTURE Arch_aw_io_reg OF aw_io_reg IS


  constant  Clk_in_ns:            integer := 1000000000 /  clk_sys_in_Hz;      -- (=8ns,    bei 125MHz)
  constant  C_Strobe_1us:         integer := 1000  / Clk_in_ns;                -- Anzahl der Clocks für 1us
	CONSTANT	c_Ena_every_1us_cnt:  INTEGER	:= 1000  / CLK_in_ns;
  constant  C_Loop_cnt_10us:      integer := 10;                               -- Anzahl der Counts für 10us

  
COMPONENT flanke
	PORT
	(
		nReset		  :	 IN  STD_LOGIC;
		clk		      :	 IN  STD_LOGIC;
    Sign_in     :  IN  std_logic;     -- Input Signal
    Pegel       :  IN  std_logic;     -- 0=pos. 1=neg. Flanke
    Strobe_out  :  out std_logic      -- Output-Strobe
	);
END COMPONENT;



COMPONENT outpuls
	PORT
	(
		nReset		    :	 IN STD_LOGIC;
		clk		        :	 IN STD_LOGIC;
		Start		      :	 IN STD_LOGIC;
    cnt_ena       :  IN STD_LOGIC;              -- Enable für die Basis_Verzögerungszeit
		Base_cnt		  :	 IN INTEGER RANGE 0 TO 15;
		Mult_cnt		  :	 IN INTEGER RANGE 0 TO 65535;
		Sign_Out		  :	 OUT STD_LOGIC
	);
END COMPONENT;



constant  addr_width:                INTEGER := Adr_from_SCUB_LA'length;
--
constant  AWOut_Reg_1_addr_offset:   INTEGER := 00;    -- RD/WR ADR-Offset zur Base_Addr für AWOut_Reg_1
constant  AWOut_Reg_2_addr_offset:   INTEGER := 01;    -- RD/WR ADR-Offset zur Base_Addr für AWOut_Reg_2
constant  AWOut_Reg_3_addr_offset:   INTEGER := 02;    -- RD/WR ADR-Offset zur Base_Addr für AWOut_Reg_3
constant  AWOut_Reg_4_addr_offset:   INTEGER := 03;    -- RD/WR ADR-Offset zur Base_Addr für AWOut_Reg_4
constant  AWOut_Reg_5_addr_offset:   INTEGER := 04;    -- RD/WR ADR-Offset zur Base_Addr für AWOut_Reg_5
constant  AWOut_Reg_6_addr_offset:   INTEGER := 05;    -- RD/WR ADR-Offset zur Base_Addr für AWOut_Reg_6
constant  AWOut_Reg_7_addr_offset:   INTEGER := 06;    -- RD/WR ADR-Offset zur Base_Addr für AWOut_Reg_7

--
constant  AWIn_1_addr_offset:        INTEGER := 16;    -- RD    ADR-Offset zur Base_Addr vom AWIN_Port1
constant  AWIn_2_addr_offset:        INTEGER := 17;    -- RD    ADR-Offset zur Base_Addr vom AWIN_Port2
constant  AWIn_3_addr_offset:        INTEGER := 18;    -- RD    ADR-Offset zur Base_Addr vom AWIN_Port3
constant  AWIn_4_addr_offset:        INTEGER := 19;    -- RD    ADR-Offset zur Base_Addr vom AWIN_Port4
constant  AWIn_5_addr_offset:        INTEGER := 20;    -- RD    ADR-Offset zur Base_Addr vom AWIN_Port5
constant  AWIn_6_addr_offset:        INTEGER := 21;    -- RD    ADR-Offset zur Base_Addr vom AWIN_Port6
constant  AWIn_7_addr_offset:        INTEGER := 22;    -- RD    ADR-Offset zur Base_Addr vom AWIN_Port7

---------------------- Pulse an den Outputs von AWOut_Reg_1 und Speichern von C_AWIN_1 + C_AWIN_2 --------------------------------------
constant  AW_Reg1_Msk_addr_offset:   INTEGER := 07;    -- RD/WR ADR-Offset zur Base_Addr für die Output-Multiplexer von AWOut_Reg_1
constant  AW_Reg1_PW1_addr_offset:   INTEGER := 08;    -- RD/WR ADR-Offset zur Base_Addr für die Pulsweite von AWOut_Reg_1 Bit [0..3]
constant  AW_Reg1_PW2_addr_offset:   INTEGER := 09;    -- RD/WR ADR-Offset zur Base_Addr für die Pulsweite von AWOut_Reg_1 Bit [4..7]
constant  AW_Reg1_PW3_addr_offset:   INTEGER := 10;    -- RD/WR ADR-Offset zur Base_Addr für die Pulsweite von AWOut_Reg_1 Bit [8..11]
constant  AW_Reg1_PW4_addr_offset:   INTEGER := 11;    -- RD/WR ADR-Offset zur Base_Addr für die Pulsweite von AWOut_Reg_1 Bit [12..15]

constant  AWIn1_Level_addr_offset:   INTEGER := 23;    -- RD/WR ADR-Offset zur Base_Addr vom "Flanken-Sel-Reg" für AWIn_Reg_1
constant  AWIn1_Store_addr_offset:   INTEGER := 24;    -- RD    ADR-Offset zur Base_Addr vom gespeicherten AWIn_Reg_1
constant  AWIn2_Level_addr_offset:   INTEGER := 25;    -- RD/WR ADR-Offset zur Base_Addr vom "Flanken-Sel-Reg" für AWIn_Reg_2
constant  AWIn2_Store_addr_offset:   INTEGER := 26;    -- RD    ADR-Offset zur Base_Addr vom gespeicherten AWIn_Reg_2
----------------------------------------------------------------------------------------------------------------------------------------

constant  C_AWOut_Reg_1_Addr:   unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWOut_Reg_1_addr_offset), addr_width);  -- RD/WR, AWOut_Reg_1
constant  C_AWOut_Reg_2_Addr:   unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWOut_Reg_2_addr_offset), addr_width);  -- RD/WR, AWOut_Reg_2
constant  C_AWOut_Reg_3_Addr:   unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWOut_Reg_3_addr_offset), addr_width);  -- RD/WR, AWOut_Reg_3
constant  C_AWOut_Reg_4_Addr:   unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWOut_Reg_4_addr_offset), addr_width);  -- RD/WR, AWOut_Reg_4
constant  C_AWOut_Reg_5_Addr:   unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWOut_Reg_5_addr_offset), addr_width);  -- RD/WR, AWOut_Reg_5
constant  C_AWOut_Reg_6_Addr:   unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWOut_Reg_6_addr_offset), addr_width);  -- RD/WR, AWOut_Reg_6
constant  C_AWOut_Reg_7_Addr:   unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWOut_Reg_7_addr_offset), addr_width);  -- RD/WR, AWOut_Reg_7

constant  C_AWIN_1_Addr:        unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn_1_addr_offset), addr_width);  -- Adresse zum Lesen des AWIn1
constant  C_AWIN_2_Addr:        unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn_2_addr_offset), addr_width);  -- Adresse zum Lesen des AWIn2
constant  C_AWIN_3_Addr:        unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn_3_addr_offset), addr_width);  -- Adresse zum Lesen des AWIn3
constant  C_AWIN_4_Addr:        unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn_4_addr_offset), addr_width);  -- Adresse zum Lesen des AWIn4
constant  C_AWIN_5_Addr:        unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn_5_addr_offset), addr_width);  -- Adresse zum Lesen des AWIn5
constant  C_AWIN_6_Addr:        unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn_6_addr_offset), addr_width);  -- Adresse zum Lesen des AWIn6
constant  C_AWIN_7_Addr:        unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn_7_addr_offset), addr_width);  -- Adresse zum Lesen des AWIn7

----------------------
constant  C_AW_Reg1_Msk_addr:   unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AW_Reg1_Msk_addr_offset), addr_width);  -- RD/WR, AW1-Output1-Multiplexer
constant  C_AW_Reg1_PW1_addr:   unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AW_Reg1_PW1_addr_offset), addr_width);  -- RD/WR, Pulsweite von AWOut_Reg_1
constant  C_AW_Reg1_PW2_addr:   unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AW_Reg1_PW2_addr_offset), addr_width);  -- RD/WR, Pulsweite von AWOut_Reg_1
constant  C_AW_Reg1_PW3_addr:   unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AW_Reg1_PW3_addr_offset), addr_width);  -- RD/WR, Pulsweite von AWOut_Reg_1
constant  C_AW_Reg1_PW4_addr:   unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AW_Reg1_PW4_addr_offset), addr_width);  -- RD/WR, Pulsweite von AWOut_Reg_1

constant  C_AWIn1_Level_addr:   unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn1_Level_addr_offset), addr_width);  -- RD/WR, Level-Reg. für AWOut_Reg_1
constant  C_AWIn1_Store_addr:   unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn1_Store_addr_offset), addr_width);  -- Adresse zum Lesen des gesp. AWIn1
constant  C_AWIn2_Level_addr:   unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn2_Level_addr_offset), addr_width);  -- RD/WR, Level-Reg. für AWOut_Reg_2
constant  C_AWIn2_Store_addr:   unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn2_Store_addr_offset), addr_width);  -- Adresse zum Lesen des gesp. AWIn2
----------------------
            
             
signal    S_AWOut_Reg_1:     std_logic_vector(15 downto 0);
signal    S_AWOut_Reg_1_Rd:  std_logic;
signal    S_AWOut_Reg_1_Wr:  std_logic;

signal    S_AWOut_Reg_2:     std_logic_vector(15 downto 0);
signal    S_AWOut_Reg_2_Rd:  std_logic;
signal    S_AWOut_Reg_2_Wr:  std_logic;

signal    S_AWOut_Reg_3:     std_logic_vector(15 downto 0);
signal    S_AWOut_Reg_3_Rd:  std_logic;
signal    S_AWOut_Reg_3_Wr:  std_logic;

signal    S_AWOut_Reg_4:     std_logic_vector(15 downto 0);
signal    S_AWOut_Reg_4_Rd:  std_logic;
signal    S_AWOut_Reg_4_Wr:  std_logic;

signal    S_AWOut_Reg_5:     std_logic_vector(15 downto 0);
signal    S_AWOut_Reg_5_Rd:  std_logic;
signal    S_AWOut_Reg_5_Wr:  std_logic;

signal    S_AWOut_Reg_6:     std_logic_vector(15 downto 0);
signal    S_AWOut_Reg_6_Rd:  std_logic;
signal    S_AWOut_Reg_6_Wr:  std_logic;

signal    S_AWOut_Reg_7:     std_logic_vector(15 downto 0);
signal    S_AWOut_Reg_7_Rd:  std_logic;
signal    S_AWOut_Reg_7_Wr:  std_logic;

signal    S_AWIn1_Rd:      std_logic;
signal    S_AWIn2_Rd:      std_logic;
signal    S_AWIn3_Rd:      std_logic;
signal    S_AWIn4_Rd:      std_logic;
signal    S_AWIn5_Rd:      std_logic;
signal    S_AWIn6_Rd:      std_logic;
signal    S_AWIn7_Rd:      std_logic;

---------------------- Pulse an den Outputs von AWOut_Reg_1 und Speichern von C_AWIN_1 + C_AWIN_2 --------------------------------------

signal    S_AW_Reg1_Puls_in:   std_logic_vector(15 downto 0);
signal    S_AW_Reg1_Puls_out:  std_logic_vector(15 downto 0);

signal    S_AW_Reg1_Msk:     std_logic_vector(15 downto 0);
signal    S_AW_Reg1_Msk_Rd:  std_logic;
signal    S_AW_Reg1_Msk_Wr:  std_logic;

signal    S_AW_Reg1_PW1:     std_logic_vector(15 downto 0);
signal    S_AW_Reg1_PW1_Rd:  std_logic;
signal    S_AW_Reg1_PW1_Wr:  std_logic;

signal    S_AW_Reg1_PW2:     std_logic_vector(15 downto 0);
signal    S_AW_Reg1_PW2_Rd:  std_logic;
signal    S_AW_Reg1_PW2_Wr:  std_logic;

signal    S_AW_Reg1_PW3:     std_logic_vector(15 downto 0);
signal    S_AW_Reg1_PW3_Rd:  std_logic;
signal    S_AW_Reg1_PW3_Wr:  std_logic;

signal    S_AW_Reg1_PW4:     std_logic_vector(15 downto 0);
signal    S_AW_Reg1_PW4_Rd:  std_logic;
signal    S_AW_Reg1_PW4_Wr:  std_logic;

signal    S_AWIn1_Level:     std_logic_vector(15 downto 0);
signal    S_AWIn1_Level_Rd:  std_logic;
signal    S_AWIn1_Level_Wr:  std_logic;

signal    S_AWIn2_Level:     std_logic_vector(15 downto 0);
signal    S_AWIn2_Level_Rd:  std_logic;
signal    S_AWIn2_Level_Wr:  std_logic;

signal    S_AWIn1_Store:     std_logic_vector(15 downto 0);
signal    S_AWIn1_Store_Rd:  std_logic;

signal    S_AWIn2_Store:     std_logic_vector(15 downto 0);
signal    S_AWIn2_Store_Rd:  std_logic;

signal    s_AWIn1_Strobe_Flanke:  std_logic_vector(15 downto 0);  -- Output-Signal des Flakendetektors von AWIn1              
signal    s_AWIn2_Strobe_Flanke:  std_logic_vector(15 downto 0);  -- Output-Signal des Flakendetektors von AWIn1              


signal    S_Str_AWOut_Reg_1: std_logic;
signal  	s_every_1us:       STD_LOGIC;

TYPE   t_Integer_Array     is array (0 to 15) of integer range 0 to 65535;

constant Wert_2_Hoch_n : t_Integer_Array :=
            (00001, 00002, 00004, 00008, 00016, 00032, 00064, 00128,
             00256, 00512, 01024, 02048, 04096, 08192, 16384, 32768);
           
signal    S_Mult_cnt_Reg1:   t_Integer_Array;
             
             
-----------------------------------------------------------------------------------------------------------------------------------------

signal    S_Dtack:        std_logic;
signal    S_Read_Port:    std_logic_vector(Data_to_SCUB'range);


begin

P_Adr_Deco:  process (nReset, clk)
  begin
    if nReset = '0' then
      
      S_AWOut_Reg_1_Rd <= '0';  S_AWOut_Reg_1_Wr <= '0';
      S_AWOut_Reg_2_Rd <= '0';  S_AWOut_Reg_2_Wr <= '0';
      S_AWOut_Reg_3_Rd <= '0';  S_AWOut_Reg_3_Wr <= '0';
      S_AWOut_Reg_4_Rd <= '0';  S_AWOut_Reg_4_Wr <= '0';
      S_AWOut_Reg_5_Rd <= '0';  S_AWOut_Reg_5_Wr <= '0';
      S_AWOut_Reg_6_Rd <= '0';  S_AWOut_Reg_6_Wr <= '0';
      S_AWOut_Reg_7_Rd <= '0';  S_AWOut_Reg_7_Wr <= '0';

      S_AWIn1_Rd <= '0';  S_AWIn2_Rd <= '0';  S_AWIn3_Rd <= '0';  S_AWIn4_Rd <= '0';
      S_AWIn5_Rd <= '0';  S_AWIn6_Rd <= '0';  S_AWIn7_Rd <= '0';  

      S_AW_Reg1_Msk_Rd <= '0';  S_AW_Reg1_Msk_Wr <= '0';  ----+-- Masken-Reg., welches Outputbit von AWOut_Reg_1 "gepulset" werden soll.
      S_AW_Reg1_PW1_Rd <= '0';  S_AW_Reg1_PW1_Wr <= '0';  ----+-- Pulsbreite der Outputs von AWOut_Reg_1
      S_AW_Reg1_PW2_Rd <= '0';  S_AW_Reg1_PW2_Wr <= '0';  --  |
      S_AW_Reg1_PW3_Rd <= '0';  S_AW_Reg1_PW3_Wr <= '0';  --  |
      S_AW_Reg1_PW4_Rd <= '0';  S_AW_Reg1_PW4_Wr <= '0';  ----+
                                                          
      S_AWIn1_Level_Rd <= '0';  S_AWIn1_Level_WR <= '0';  ---+-- Speichern von C_AWIN_1 + C_AWIN_2
      S_AWIn2_Level_Rd <= '0';  S_AWIn2_Level_WR <= '0';  -- |
      S_AWIn1_Store_Rd <= '0';  S_AWIn2_Store_Rd <= '0';  ---+
        
      S_Dtack <= '0';
      Rd_active <= '0';
    
    elsif rising_edge(clk) then

      S_AWOut_Reg_1_Rd <= '0';  S_AWOut_Reg_1_Wr <= '0';
      S_AWOut_Reg_2_Rd <= '0';  S_AWOut_Reg_2_Wr <= '0';
      S_AWOut_Reg_3_Rd <= '0';  S_AWOut_Reg_3_Wr <= '0';
      S_AWOut_Reg_4_Rd <= '0';  S_AWOut_Reg_4_Wr <= '0';
      S_AWOut_Reg_5_Rd <= '0';  S_AWOut_Reg_5_Wr <= '0';
      S_AWOut_Reg_6_Rd <= '0';  S_AWOut_Reg_6_Wr <= '0';
      S_AWOut_Reg_7_Rd <= '0';  S_AWOut_Reg_7_Wr <= '0';

      S_AWIn1_Rd <= '0';  S_AWIn2_Rd <= '0';  S_AWIn3_Rd <= '0';  S_AWIn4_Rd <= '0';
      S_AWIn5_Rd <= '0';  S_AWIn6_Rd <= '0';  S_AWIn7_Rd <= '0';
      
      S_AW_Reg1_Msk_Rd <= '0';  S_AW_Reg1_Msk_Wr <= '0';  ----+-- Masken-Reg., welches Outputbit von AWOut_Reg_1 "gepulset" werden soll.  
      S_AW_Reg1_PW1_Rd <= '0';  S_AW_Reg1_PW1_Wr <= '0';  ----+-- Pulsbreite der Outputs von AWOut_Reg_1
      S_AW_Reg1_PW2_Rd <= '0';  S_AW_Reg1_PW2_Wr <= '0';  --  |
      S_AW_Reg1_PW3_Rd <= '0';  S_AW_Reg1_PW3_Wr <= '0';  --  |
      S_AW_Reg1_PW4_Rd <= '0';  S_AW_Reg1_PW4_Wr <= '0';  ----+
                                                          
      S_AWIn1_Level_Rd <= '0';  S_AWIn1_Level_WR <= '0';  ---+-- Speichern von C_AWIN_1 + C_AWIN_2
      S_AWIn2_Level_Rd <= '0';  S_AWIn2_Level_WR <= '0';  -- |
      S_AWIn1_Store_Rd <= '0';  S_AWIn2_Store_Rd <= '0';  ---+
      
      S_Dtack <= '0';
      Rd_active <= '0';
      
      if Ext_Adr_Val = '1' then

        CASE unsigned(ADR_from_SCUB_LA) IS
            
          when C_AWOut_Reg_1_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_1_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_1_Rd <= '1';
              Rd_active <= '1';
            end if;

          when C_AWOut_Reg_2_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_2_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_2_Rd <= '1';
              Rd_active <= '1';
            end if;

          when C_AWOut_Reg_3_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_3_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_3_Rd <= '1';
              Rd_active <= '1';
            end if;

          when C_AWOut_Reg_4_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_4_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_4_Rd <= '1';
              Rd_active <= '1';
            end if;

            when C_AWOut_Reg_5_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_5_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_5_Rd <= '1';
              Rd_active <= '1';
            end if;

            when C_AWOut_Reg_6_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_6_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_6_Rd <= '1';
              Rd_active <= '1';
            end if;

            when C_AWOut_Reg_7_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_7_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AWOut_Reg_7_Rd <= '1';
              Rd_active <= '1';
            end if;

            
            when C_AWIN_1_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_AWIn1_Rd   <= '1';
              Rd_active <= '1';
            end if;

            when C_AWIN_2_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_AWIn2_Rd   <= '1';
              Rd_active <= '1';
            end if;

            when C_AWIN_3_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_AWIn3_Rd   <= '1';
              Rd_active <= '1';
            end if;

            when C_AWIN_4_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_AWIn4_Rd   <= '1';
              Rd_active <= '1';
            end if;

            when C_AWIN_5_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_AWIn5_Rd   <= '1';
              Rd_active <= '1';
            end if;

            when C_AWIN_6_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_AWIn6_Rd   <= '1';
              Rd_active <= '1';
            end if;

            when C_AWIN_7_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_AWIn7_Rd   <= '1';
              Rd_active <= '1';
            end if;

---------------------- Pulse an den Outputs von AWOut_Reg_1 und Speichern von C_AWIN_1 + C_AWIN_2 --------------------------------------
          when C_AW_Reg1_Msk_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AW_Reg1_Msk_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AW_Reg1_Msk_Rd <= '1';
              Rd_active <= '1';
            end if;

          when C_AW_Reg1_PW1_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AW_Reg1_PW1_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AW_Reg1_PW1_Rd <= '1';
              Rd_active <= '1';
            end if;

          when C_AW_Reg1_PW2_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AW_Reg1_PW2_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AW_Reg1_PW2_Rd <= '1';
              Rd_active <= '1';
            end if;

          when C_AW_Reg1_PW3_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AW_Reg1_PW3_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AW_Reg1_PW3_Rd <= '1';
              Rd_active <= '1';
            end if;

          when C_AW_Reg1_PW4_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AW_Reg1_PW4_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AW_Reg1_PW4_Rd <= '1';
              Rd_active <= '1';
            end if;


          when C_AWIn1_Level_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AWIn1_Level_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AWIn1_Level_Rd <= '1';
              Rd_active <= '1';
            end if;

            when C_AWIn1_Store_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_AWIn1_Store_Rd   <= '1';
              Rd_active <= '1';
            end if;


          when C_AWIn2_Level_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AWIn2_Level_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AWIn2_Level_Rd <= '1';
              Rd_active <= '1';
            end if;

 
            when C_AWIn2_Store_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_AWIn2_Store_Rd   <= '1';
              Rd_active <= '1';
            end if;

   
          when others => 
            
            S_AWOut_Reg_1_Rd <= '0';  S_AWOut_Reg_1_Wr <= '0';
            S_AWOut_Reg_2_Rd <= '0';  S_AWOut_Reg_2_Wr <= '0';
            S_AWOut_Reg_3_Rd <= '0';  S_AWOut_Reg_3_Wr <= '0';
            S_AWOut_Reg_4_Rd <= '0';  S_AWOut_Reg_4_Wr <= '0';
            S_AWOut_Reg_5_Rd <= '0';  S_AWOut_Reg_5_Wr <= '0';
            S_AWOut_Reg_6_Rd <= '0';  S_AWOut_Reg_6_Wr <= '0';
            S_AWOut_Reg_7_Rd <= '0';  S_AWOut_Reg_7_Wr <= '0';

            S_AWIn1_Rd <= '0';  S_AWIn2_Rd <= '0';  S_AWIn3_Rd <= '0';  S_AWIn4_Rd <= '0';
            S_AWIn5_Rd <= '0';  S_AWIn6_Rd <= '0';  S_AWIn7_Rd <= '0';


            S_AW_Reg1_Msk_Rd <= '0';  S_AW_Reg1_Msk_Wr <= '0';  ----+-- Masken-Reg., welches Outputbit von AWOut_Reg_1 "gepulset" werden soll.
            S_AW_Reg1_PW1_Rd <= '0';  S_AW_Reg1_PW1_Wr <= '0';  ----+-- Pulsbreite der Outputs von AWOut_Reg_1
            S_AW_Reg1_PW2_Rd <= '0';  S_AW_Reg1_PW2_Wr <= '0';  --  |
            S_AW_Reg1_PW3_Rd <= '0';  S_AW_Reg1_PW3_Wr <= '0';  --  |
            S_AW_Reg1_PW4_Rd <= '0';  S_AW_Reg1_PW4_Wr <= '0';  ----+
      
            S_AWIn1_Level_Rd <= '0';  S_AWIn1_Level_WR <= '0';  ---+-- Speichern von C_AWIN_1 + C_AWIN_2
            S_AWIn2_Level_Rd <= '0';  S_AWIn2_Level_WR <= '0';  -- |
            S_AWIn1_Store_Rd <= '0';  S_AWIn2_Store_Rd <= '0';  ---+

            
            S_Dtack <= '0';
            Rd_active <= '0';

        end CASE;
      end if;
    end if;
  
  end process P_Adr_Deco;

  
  
P_AWOut_Reg:  process (nReset, clk, 
                       S_AWOut_Reg_1_Wr, S_AWOut_Reg_2_Wr, S_AWOut_Reg_3_Wr, S_AWOut_Reg_4_Wr,
                       S_AWOut_Reg_5_Wr, S_AWOut_Reg_6_Wr, S_AWOut_Reg_7_Wr,
                       S_AW_Reg1_Msk_Wr, S_AW_Reg1_PW1_Wr, S_AW_Reg1_PW2_Wr, S_AW_Reg1_PW3_Wr, S_AW_Reg1_PW4_Wr, -- Pulse an den Outputs von AWOut_Reg_1
                       S_AWIn1_Level_WR, S_AWIn2_Level_WR)                                                       -- Speichern von C_AWIN_1 + C_AWIN_2


 begin
    if nReset = '0' then
      S_AWOut_Reg_1   <= (others => '0');
      S_AWOut_Reg_2   <= (others => '0');
      S_AWOut_Reg_3   <= (others => '0');
      S_AWOut_Reg_4   <= (others => '0');
      S_AWOut_Reg_5   <= (others => '0');
      S_AWOut_Reg_6   <= (others => '0');
      S_AWOut_Reg_7   <= (others => '0');

      S_AW_Reg1_Msk   <= (others => '0');   ----+-- Masken-Reg., welches Outputbit von AWOut_Reg_1 "gepulset" werden soll.  
      S_AW_Reg1_PW1   <= (others => '0');   ----+-- Pulsbreite der Outputs von AWOut_Reg_1
      S_AW_Reg1_PW2   <= (others => '0');   --  |
      S_AW_Reg1_PW3   <= (others => '0');   --  |
      S_AW_Reg1_PW4   <= (others => '0');   ----+
      S_AWIn1_Level   <= (others => '0');   -- Speichern der Flanke von C_AWIN_1
      S_AWIn2_Level   <= (others => '0');   -- Speichern der Flanke von C_AWIN_2                
  
    elsif rising_edge(clk) then
      
      if S_AWOut_Reg_1_Wr = '1'   then  S_AWOut_Reg_1 <= Data_from_SCUB_LA;   -- Output-Daten vom SCU-Bus
      end if;
      if S_AWOut_Reg_2_Wr = '1'   then  S_AWOut_Reg_2 <= Data_from_SCUB_LA;
      end if;
      if S_AWOut_Reg_3_Wr = '1'   then  S_AWOut_Reg_3 <= Data_from_SCUB_LA;
      end if;
      if S_AWOut_Reg_4_Wr = '1'   then  S_AWOut_Reg_4 <= Data_from_SCUB_LA;
      end if;
      if S_AWOut_Reg_5_Wr = '1'   then  S_AWOut_Reg_5 <= Data_from_SCUB_LA;
      end if;
      if S_AWOut_Reg_6_Wr = '1'   then  S_AWOut_Reg_6 <= Data_from_SCUB_LA;
      end if;
      if S_AWOut_Reg_7_Wr = '1'   then  S_AWOut_Reg_7 <= Data_from_SCUB_LA;
      end if;

      
      if S_AW_Reg1_Msk_Wr = '1'   then  S_AW_Reg1_Msk <= Data_from_SCUB_LA;   
      end if;
      if S_AW_Reg1_PW1_Wr = '1'   then  S_AW_Reg1_PW1 <= Data_from_SCUB_LA;
      end if;
      if S_AW_Reg1_PW2_Wr = '1'   then  S_AW_Reg1_PW2 <= Data_from_SCUB_LA;
      end if;
      if S_AW_Reg1_PW3_Wr = '1'   then  S_AW_Reg1_PW3 <= Data_from_SCUB_LA;
      end if;
      if S_AW_Reg1_PW4_Wr = '1'   then  S_AW_Reg1_PW4 <= Data_from_SCUB_LA;
      end if;

      if S_AWIn1_Level_Wr = '1'   then  S_AWIn1_Level <= Data_from_SCUB_LA;
      end if;
      if S_AWIn2_Level_Wr = '1'   then  S_AWIn2_Level <= Data_from_SCUB_LA;
      end if;
      
  end if;
  end process P_AWOut_Reg;
  

  

  P_read_mux:  process (S_AWOut_Reg_1_Rd,  S_AWOut_Reg_1,
                        S_AWOut_Reg_2_Rd,  S_AWOut_Reg_2,
                        S_AWOut_Reg_3_Rd,  S_AWOut_Reg_3,
                        S_AWOut_Reg_4_Rd,  S_AWOut_Reg_4,
                        S_AWOut_Reg_5_Rd,  S_AWOut_Reg_5,
                        S_AWOut_Reg_6_Rd,  S_AWOut_Reg_6,
                        S_AWOut_Reg_7_Rd,  S_AWOut_Reg_7,
                        S_AWIn1_Rd,        
                        S_AWIn2_Rd,        
                        S_AWIn3_Rd,        
                        S_AWIn4_Rd,        
                        S_AWIn5_Rd,        
                        S_AWIn6_Rd,        
                        S_AWIn7_Rd,        
                        SCU_AW_Input_Reg,
                        S_AW_Reg1_Msk_Rd,  S_AW_Reg1_Msk,      ----+-- Pulse an den Outputs von AWOut_Reg_1       
                        S_AW_Reg1_PW1_Rd,  S_AW_Reg1_PW1,      --  |   
                        S_AW_Reg1_PW2_Rd,  S_AW_Reg1_PW2,      --  |                
                        S_AW_Reg1_PW3_Rd,  S_AW_Reg1_PW3,      --  | 
                        S_AW_Reg1_PW4_Rd,  S_AW_Reg1_PW4,      ----+
                        S_AWIn1_Level_Rd,  S_AWIn1_Level,      -- Speichern der Flanke von C_AWIN_1            
                        S_AWIn2_Level_Rd,  S_AWIn2_Level,      -- Speichern der Flanke von C_AWIN_2                   
                        S_AWIn1_Store_Rd,  S_AWIn1_Store,      -- gespeicherte Daten von C_AWIN_1       
                        S_AWIn2_Store_Rd,  S_AWIn2_Store       -- gespeicherte Daten von C_AWIN_2
                        )

  begin

    if    S_AWOut_Reg_1_Rd = '1' then  S_Read_port <= S_AWOut_Reg_1;  -- read AWOut_Reg_1
    elsif S_AWOut_Reg_2_Rd = '1' then  S_Read_port <= S_AWOut_Reg_2;
    elsif S_AWOut_Reg_3_Rd = '1' then  S_Read_port <= S_AWOut_Reg_3;
    elsif S_AWOut_Reg_4_Rd = '1' then  S_Read_port <= S_AWOut_Reg_4;
    elsif S_AWOut_Reg_5_Rd = '1' then  S_Read_port <= S_AWOut_Reg_5;
    elsif S_AWOut_Reg_6_Rd = '1' then  S_Read_port <= S_AWOut_Reg_6;
    elsif S_AWOut_Reg_7_Rd = '1' then  S_Read_port <= S_AWOut_Reg_7;

    elsif S_AWIn1_Rd = '1' then  S_Read_port <= SCU_AW_Input_Reg(1);    -- read Input-Port1
    elsif S_AWIn2_Rd = '1' then  S_Read_port <= SCU_AW_Input_Reg(2);
    elsif S_AWIn3_Rd = '1' then  S_Read_port <= SCU_AW_Input_Reg(3);
    elsif S_AWIn4_Rd = '1' then  S_Read_port <= SCU_AW_Input_Reg(4);
    elsif S_AWIn5_Rd = '1' then  S_Read_port <= SCU_AW_Input_Reg(5);
    elsif S_AWIn6_Rd = '1' then  S_Read_port <= SCU_AW_Input_Reg(6);
    elsif S_AWIn7_Rd = '1' then  S_Read_port <= SCU_AW_Input_Reg(7);
    
    elsif S_AW_Reg1_Msk_Rd = '1' then  S_Read_port <= S_AW_Reg1_Msk;    ----+-- Pulse an den Outputs von AWOut_Reg_1
    elsif S_AW_Reg1_PW1_Rd = '1' then  S_Read_port <= S_AW_Reg1_PW1;    --  |   
    elsif S_AW_Reg1_PW2_Rd = '1' then  S_Read_port <= S_AW_Reg1_PW2;    --  |
    elsif S_AW_Reg1_PW3_Rd = '1' then  S_Read_port <= S_AW_Reg1_PW3;    --  | 
    elsif S_AW_Reg1_PW4_Rd = '1' then  S_Read_port <= S_AW_Reg1_PW4;    ----+
    
    elsif S_AWIn1_Level_Rd = '1' then  S_Read_port <= S_AWIn1_Level;    -- Speichern der Flanke von C_AWIN_1   
    elsif S_AWIn2_Level_Rd = '1' then  S_Read_port <= S_AWIn2_Level;    -- Speichern der Flanke von C_AWIN_2
    elsif S_AWIn1_Store_Rd = '1' then  S_Read_port <= S_AWIn1_Store;    -- gespeicherte Daten von C_AWIN_1   
    elsif S_AWIn2_Store_Rd = '1' then  S_Read_port <= S_AWIn2_Store;    -- gespeicherte Daten von C_AWIN_2
    
  else
      S_Read_Port <= (others => '-');
    end if;
  end process P_Read_mux;


  
-------------- "Errechnen" des Multiplikationsfaktors  (Output-Puls Breite (S_Mult_cnt_Reg1(n)) = n mal 1yS) -----------------------------------
  
P_Mult_cnt_Reg1:  process (nReset, clk) 

  begin
    if nReset = '0' then
      S_Mult_cnt_Reg1   <= (others => 0);
  
    elsif rising_edge(clk) then

      S_Mult_cnt_Reg1( 0)  <=  Wert_2_Hoch_n(to_integer(unsigned(S_AW_Reg1_PW1)( 3 downto  0))); -- Multiplikationswert für 1us aus Wertetabelle 2^n     
      S_Mult_cnt_Reg1( 1)  <=  Wert_2_Hoch_n(to_integer(unsigned(S_AW_Reg1_PW1)( 7 downto  4)));      
      S_Mult_cnt_Reg1( 2)  <=  Wert_2_Hoch_n(to_integer(unsigned(S_AW_Reg1_PW1)(11 downto  8)));      
      S_Mult_cnt_Reg1( 3)  <=  Wert_2_Hoch_n(to_integer(unsigned(S_AW_Reg1_PW1)(15 downto 12)));      
      S_Mult_cnt_Reg1( 4)  <=  Wert_2_Hoch_n(to_integer(unsigned(S_AW_Reg1_PW2)( 3 downto  0)));      
      S_Mult_cnt_Reg1( 5)  <=  Wert_2_Hoch_n(to_integer(unsigned(S_AW_Reg1_PW2)( 7 downto  4)));      
      S_Mult_cnt_Reg1( 6)  <=  Wert_2_Hoch_n(to_integer(unsigned(S_AW_Reg1_PW2)(11 downto  8)));      
      S_Mult_cnt_Reg1( 7)  <=  Wert_2_Hoch_n(to_integer(unsigned(S_AW_Reg1_PW2)(15 downto 12)));      
      S_Mult_cnt_Reg1( 8)  <=  Wert_2_Hoch_n(to_integer(unsigned(S_AW_Reg1_PW3)( 3 downto  0)));      
      S_Mult_cnt_Reg1( 9)  <=  Wert_2_Hoch_n(to_integer(unsigned(S_AW_Reg1_PW3)( 7 downto  4)));      
      S_Mult_cnt_Reg1(10)  <=  Wert_2_Hoch_n(to_integer(unsigned(S_AW_Reg1_PW3)(11 downto  8)));      
      S_Mult_cnt_Reg1(11)  <=  Wert_2_Hoch_n(to_integer(unsigned(S_AW_Reg1_PW3)(15 downto 12)));      
      S_Mult_cnt_Reg1(12)  <=  Wert_2_Hoch_n(to_integer(unsigned(S_AW_Reg1_PW4)( 3 downto  0)));      
      S_Mult_cnt_Reg1(13)  <=  Wert_2_Hoch_n(to_integer(unsigned(S_AW_Reg1_PW4)( 7 downto  4)));      
      S_Mult_cnt_Reg1(14)  <=  Wert_2_Hoch_n(to_integer(unsigned(S_AW_Reg1_PW4)(11 downto  8)));      
      S_Mult_cnt_Reg1(15)  <=  Wert_2_Hoch_n(to_integer(unsigned(S_AW_Reg1_PW4)(15 downto 12)));      
      
    end if;
  end process P_Mult_cnt_Reg1;


  
-------------- Startpulse für die Zähler der "gepulsten" Ausgänge von S_AWOut_Reg_1 -----------------------------------
  
P_AWOut_Puls:  process (nReset, clk, S_AWOut_Reg_1_Wr, Data_from_SCUB_LA, Ext_Wr_fin) 

  begin
    if nReset = '0' then
      S_AW_Reg1_Puls_in   <= (others => '0');
  
    elsif rising_edge(clk) then

    for i in 0 to 15 loop
      if  (Ext_Wr_fin and S_AWOut_Reg_1_Wr and Data_from_SCUB_LA(i)) = '1'   then
                S_AW_Reg1_Puls_in(i) <= '1';   -- Start-Strobe für Counter "Ausgang-Puls-Breite"
        else    S_AW_Reg1_Puls_in(i) <= '0';
      end if;
    end loop;  
      
    end if;
  end process P_AWOut_Puls;
  


-------------- Übergabe der Zälerstände für die Outputpulsbreite -----------------------------------


Puls_Reg1: for I in 0 to 15 generate 
   Puls_n: outpuls port map(nReset => nReset, CLK => CLK, Cnt_ena => Ena_every_1us, Start => S_AW_Reg1_Puls_in(i),
                            Base_cnt => C_Loop_cnt_10us, Mult_cnt => S_Mult_cnt_Reg1(i), Sign_Out => S_AW_Reg1_Puls_out(i));
  end generate Puls_Reg1;
  
  
-------------------  Flakendetektor für AWIn1 und AWIn2--------------------

Flanke_Reg1: for I in 0 to 15 generate 
   Reg1_n: flanke port map(nReset => nReset, CLK => CLK, Sign_In => SCU_AW_Input_Reg(1)(i), Pegel => S_AWIn1_Level(i), Strobe_out => s_AWIn1_Strobe_Flanke(i));
  end generate Flanke_Reg1;

Flanke_Reg2: for I in 0 to 15 generate 
   Reg2_n: flanke port map(nReset => nReset, CLK => CLK, Sign_In => SCU_AW_Input_Reg(2)(i), Pegel => S_AWIn2_Level(i), Strobe_out => s_AWIn2_Strobe_Flanke(i));
  end generate Flanke_Reg2;


  
-------------------  Speicher für Inputsignale von AWIn1 und AWIn2--------------------
  

P_AWIn1_Store:  process (clk, nReset)
  begin
    IF  nReset  = '0' then   s_AWIn1_Store  <= (OTHERS => '0');

    elsif rising_edge(clk) then

      if    ((Ext_Rd_fin = '1') and
             (unsigned(ADR_from_SCUB_LA) = C_AWIn1_Store_addr)) then  s_AWIn1_Store    <= (OTHERS => '0'); -- reset Register nach dem Read
 
      elsif (s_AWIn1_Strobe_Flanke(15) = '1')    then s_AWIn1_Store(15)  <= '1';
      elsif (s_AWIn1_Strobe_Flanke(14) = '1')    then s_AWIn1_Store(14)  <= '1';
      elsif (s_AWIn1_Strobe_Flanke(13) = '1')    then s_AWIn1_Store(13)  <= '1';
      elsif (s_AWIn1_Strobe_Flanke(12) = '1')    then s_AWIn1_Store(12)  <= '1';
      elsif (s_AWIn1_Strobe_Flanke(11) = '1')    then s_AWIn1_Store(11)  <= '1';
      elsif (s_AWIn1_Strobe_Flanke(10) = '1')    then s_AWIn1_Store(10)  <= '1';
      elsif (s_AWIn1_Strobe_Flanke(9)  = '1')    then s_AWIn1_Store(9)   <= '1';
      elsif (s_AWIn1_Strobe_Flanke(8)  = '1')    then s_AWIn1_Store(8)   <= '1';
      elsif (s_AWIn1_Strobe_Flanke(7)  = '1')    then s_AWIn1_Store(7)   <= '1';
      elsif (s_AWIn1_Strobe_Flanke(6)  = '1')    then s_AWIn1_Store(6)   <= '1';
      elsif (s_AWIn1_Strobe_Flanke(5)  = '1')    then s_AWIn1_Store(5)   <= '1';
      elsif (s_AWIn1_Strobe_Flanke(4)  = '1')    then s_AWIn1_Store(4)   <= '1';
      elsif (s_AWIn1_Strobe_Flanke(3)  = '1')    then s_AWIn1_Store(3)   <= '1';
      elsif (s_AWIn1_Strobe_Flanke(2)  = '1')    then s_AWIn1_Store(2)   <= '1';
      elsif (s_AWIn1_Strobe_Flanke(1)  = '1')    then s_AWIn1_Store(1)   <= '1';
      elsif (s_AWIn1_Strobe_Flanke(0)  = '1')    then s_AWIn1_Store(0)   <= '1';
        
      end if;
     end if;

  end process P_AWIn1_Store;

P_AWIn2_Store:  process (clk, nReset)
  begin
    IF  nReset  = '0' then   s_AWIn2_Store  <= (OTHERS => '0');

    elsif rising_edge(clk) then

      if    ((Ext_Rd_fin = '1') and
             (unsigned(ADR_from_SCUB_LA) = C_AWIn2_Store_addr)) then  s_AWIn2_Store    <= (OTHERS => '0'); -- reset Register nach dem Read

      elsif (s_AWIn2_Strobe_Flanke(15) = '1')    then s_AWIn2_Store(15)  <= '1';
      elsif (s_AWIn2_Strobe_Flanke(14) = '1')    then s_AWIn2_Store(14)  <= '1';
      elsif (s_AWIn2_Strobe_Flanke(13) = '1')    then s_AWIn2_Store(13)  <= '1';
      elsif (s_AWIn2_Strobe_Flanke(12) = '1')    then s_AWIn2_Store(12)  <= '1';
      elsif (s_AWIn2_Strobe_Flanke(11) = '1')    then s_AWIn2_Store(11)  <= '1';
      elsif (s_AWIn2_Strobe_Flanke(10) = '1')    then s_AWIn2_Store(10)  <= '1';
      elsif (s_AWIn2_Strobe_Flanke(9)  = '1')    then s_AWIn2_Store(9)   <= '1';
      elsif (s_AWIn2_Strobe_Flanke(8)  = '1')    then s_AWIn2_Store(8)   <= '1';
      elsif (s_AWIn2_Strobe_Flanke(7)  = '1')    then s_AWIn2_Store(7)   <= '1';
      elsif (s_AWIn2_Strobe_Flanke(6)  = '1')    then s_AWIn2_Store(6)   <= '1';
      elsif (s_AWIn2_Strobe_Flanke(5)  = '1')    then s_AWIn2_Store(5)   <= '1';
      elsif (s_AWIn2_Strobe_Flanke(4)  = '1')    then s_AWIn2_Store(4)   <= '1';
      elsif (s_AWIn2_Strobe_Flanke(3)  = '1')    then s_AWIn2_Store(3)   <= '1';
      elsif (s_AWIn2_Strobe_Flanke(2)  = '1')    then s_AWIn2_Store(2)   <= '1';
      elsif (s_AWIn2_Strobe_Flanke(1)  = '1')    then s_AWIn2_Store(1)   <= '1';
      elsif (s_AWIn2_Strobe_Flanke(0)  = '1')    then s_AWIn2_Store(0)   <= '1';
        
      end if;
     end if;

  end process P_AWIn2_Store;

  
  
--------- Multiplexer für das SCU_AW_Output_Reg(1) --------------------

p_AW_Out_Mux:  PROCESS (S_AW_Reg1_Msk, S_AWOut_Reg_1, S_AW_Reg1_Puls_out) 
    BEGin
    for i in 0 to 15 loop
      IF  S_AW_Reg1_Msk(i)  = '0' then SCU_AW_Output_Reg(1)(i)  <= S_AWOut_Reg_1(i);
      else                             SCU_AW_Output_Reg(1)(i)  <= S_AW_Reg1_Puls_out(i);
      end if;
  end loop;  
END PROCESS p_AW_Out_Mux;

  
---------------------- Outputs --------------------------------------------------

--SCU_AW_Output_Reg(1)      <=  S_AWOut_Reg_1;     -- Daten-Reg. AWOut1
SCU_AW_Output_Reg(2)      <=  S_AWOut_Reg_2;     -- Daten-Reg. AWOut2
SCU_AW_Output_Reg(3)      <=  S_AWOut_Reg_3;     -- Daten-Reg. AWOut3
SCU_AW_Output_Reg(4)      <=  S_AWOut_Reg_4;     -- Daten-Reg. AWOut4
SCU_AW_Output_Reg(5)      <=  S_AWOut_Reg_5;     -- Daten-Reg. AWOut5
SCU_AW_Output_Reg(6)      <=  S_AWOut_Reg_6;     -- Daten-Reg. AWOut6
SCU_AW_Output_Reg(7)      <=  S_AWOut_Reg_7;     -- Daten-Reg. AWOut7



AWOut_Reg1_wr   <=  S_AWOut_Reg_1_wr;  -- Daten-Reg. AWOut1
AWOut_Reg2_wr   <=  S_AWOut_Reg_2_wr;  -- Daten-Reg. AWOut2
AWOut_Reg3_wr   <=  S_AWOut_Reg_3_wr;  -- Daten-Reg. AWOut3
AWOut_Reg4_wr   <=  S_AWOut_Reg_4_wr;  -- Daten-Reg. AWOut4
AWOut_Reg5_wr   <=  S_AWOut_Reg_5_wr;  -- Daten-Reg. AWOut5
AWOut_Reg6_wr   <=  S_AWOut_Reg_6_wr;  -- Daten-Reg. AWOut6
AWOut_Reg7_wr   <=  S_AWOut_Reg_7_wr;  -- Daten-Reg. AWOut7

Dtack_to_SCUB <= S_Dtack;
Data_to_SCUB <= S_Read_Port;
  
-- Testport für Logic-Analysator
  
LA    <=      (x"00" & 
               s_Str_AWOut_Reg_1  &  S_AWOut_Reg_7_wr   & S_AWOut_Reg_6_wr  &  S_AWOut_Reg_5_wr    &
               S_AWOut_Reg_4_wr   & S_AWOut_Reg_3_wr    & S_AWOut_Reg_2_wr  &  S_AWOut_Reg_1_wr     );   
 

end Arch_AW_IO_Reg;