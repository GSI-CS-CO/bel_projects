--TITLE "'addac_reg' Autor: R.Hartmann, Stand: 01.04.2015;
--

library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
--USE IEEE.std_logic_arith.all;

ENTITY addac_reg IS
  generic
      (
    Base_addr:    INTEGER := 16#0200# );
    
  port(
    Adr_from_SCUB_LA:     in   std_logic_vector(15 downto 0);    -- latched address from SCU_Bus
    Data_from_SCUB_LA:    in   std_logic_vector(15 downto 0);    -- latched data from SCU_Bus 
    Ext_Adr_Val:          in   std_logic;                        -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:        in   std_logic;                        -- '1' => Rd-Cycle is active
    Ext_Rd_fin:           in   std_logic;                        -- marks end of read cycle, active one for one clock period of sys_clk
    Ext_Wr_active:        in   std_logic;                        -- '1' => Wr-Cycle is active
    Ext_Wr_fin:           in   std_logic;                        -- marks end of write cycle, active one for one clock period of sys_clk
    clk:                  in   std_logic;                        -- should be the same clk, used by SCU_Bus_Slave
    nReset:               in   std_logic;

    DAC1_Config:          out  std_logic_vector(15 downto 0);    -- Daten-Reg. DAC1_Config
    DAC1_Config_wr:       out  std_logic;                        -- write DAC1_Config
    DAC1_Out:             out  std_logic_vector(15 downto 0);    -- Daten-Reg. DAC1
    DAC1_Out_wr:          out  std_logic;                        -- write Daten-Reg. DAC1
    DAC2_Config:          out  std_logic_vector(15 downto 0);    -- Daten-Reg. DAC2_Config
    DAC2_Config_wr:       out  std_logic;                        -- write DAC2_Config
    DAC2_Out:             out  std_logic_vector(15 downto 0);    -- Daten-Reg. DAC2
    DAC2_Out_wr:          out  std_logic;                        -- write Daten-Reg. DAC1
    --
    ADC_Config:           out  std_logic_vector(15 downto 0);    -- Daten-Reg. ADC_Config
    ADC_In1:              in   std_logic_vector(15 downto 0);    -- Input-Port 1
    ADC_In2:              in   std_logic_vector(15 downto 0);    -- Input-Port 2
    ADC_In3:              in   std_logic_vector(15 downto 0);    -- Input-Port 3
    ADC_In4:              in   std_logic_vector(15 downto 0);    -- Input-Port 4
    ADC_In5:              in   std_logic_vector(15 downto 0);    -- Input-Port 5
    ADC_In6:              in   std_logic_vector(15 downto 0);    -- Input-Port 6
    ADC_In7:              in   std_logic_vector(15 downto 0);    -- Input-Port 7
    ADC_In8:              in   std_logic_vector(15 downto 0);    -- Input-Port 8
    
    Rd_active:            out  std_logic;                        -- read data available at 'Data_to_SCUB'-AWOut
    Data_to_SCUB:         out  std_logic_vector(15 downto 0);    -- connect read sources to SCUB-Macro
    Dtack_to_SCUB:        out  std_logic;                        -- connect Dtack to SCUB-Macro
    LA:                   out  std_logic_vector(15 downto 0)
    );  
  end addac_reg;


ARCHITECTURE Arch_addac_reg OF addac_reg IS



constant  addr_width:                INTEGER := Adr_from_SCUB_LA'length;


constant  DAC1_Config_addr_offset:   INTEGER := 16#00#;    -- Offset zur Base_addr zum Setzen oder Rücklesen des DAC1 Registers
constant  DAC1_addr_offset:          INTEGER := 16#01#;    -- Offset zur Base_addr zum Setzen oder Rücklesen des DAC2 Registers
constant  DAC2_Config_addr_offset:   INTEGER := 16#10#;    -- Offset zur Base_addr zum Setzen oder Rücklesen des DAC1 Registers
constant  DAC2_addr_offset:          INTEGER := 16#11#;    -- Offset zur Base_addr zum Setzen oder Rücklesen des IO_Config Registers
--  
constant  ADC_Config_addr_offset:    INTEGER := 16#30#;    -- Offset zur Base_addr zum Setzen oder Rücklesen des ADC_Config Registers
constant  ADC_In_1_addr_offset:      INTEGER := 16#31#;    -- Offset zur Base_addr zum Rücklesen des ADC_In_Port1
constant  ADC_In_2_addr_offset:      INTEGER := 16#32#;    -- Offset zur Base_addr zum Rücklesen des ADC_In_Port2
constant  ADC_In_3_addr_offset:      INTEGER := 16#33#;    -- Offset zur Base_addr zum Rücklesen des ADC_In_Port3
constant  ADC_In_4_addr_offset:      INTEGER := 16#34#;    -- Offset zur Base_addr zum Rücklesen des ADC_In_Port4
constant  ADC_In_5_addr_offset:      INTEGER := 16#35#;    -- Offset zur Base_addr zum Rücklesen des ADC_In_Port5
constant  ADC_In_6_addr_offset:      INTEGER := 16#36#;    -- Offset zur Base_addr zum Rücklesen des ADC_In_Port6
constant  ADC_In_7_addr_offset:      INTEGER := 16#37#;    -- Offset zur Base_addr zum Rücklesen des ADC_In_Port7
constant  ADC_In_8_addr_offset:      INTEGER := 16#38#;    -- Offset zur Base_addr zum Rücklesen des ADC_In_Port8
--
--

constant  C_DAC1_Config_Addr: unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + DAC1_Config_addr_offset), addr_width);  -- Adresse zum Setzen oder Rücklesen des DAC1_Config Registers
constant  C_DAC1_Addr:        unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + DAC1_addr_offset),       addr_width);  -- Adresse zum Setzen oder Rücklesen des DAC1 Registers
constant  C_DAC2_Config_Addr: unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + DAC2_Config_addr_offset), addr_width);  -- Adresse zum Setzen oder Rücklesen des DAC1_Config Registers
constant  C_DAC2_Addr:        unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + DAC2_addr_offset),       addr_width);  -- Adresse zum Setzen oder Rücklesen des DAC2 Registers

constant  C_ADC_Config_addr:  unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + ADC_Config_addr_offset), addr_width);  -- Adresse zum Setzen oder Rücklesen des IO6 Registers
constant  C_ADC_In_1_Addr:    unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + ADC_In_1_addr_offset),   addr_width);  -- Adresse zum Lesen des ADC_In1
constant  C_ADC_In_2_Addr:    unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + ADC_In_2_addr_offset),   addr_width);  -- Adresse zum Lesen des ADC_In2
constant  C_ADC_In_3_Addr:    unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + ADC_In_3_addr_offset),   addr_width);  -- Adresse zum Lesen des ADC_In3
constant  C_ADC_In_4_Addr:    unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + ADC_In_4_addr_offset),   addr_width);  -- Adresse zum Lesen des ADC_In4
constant  C_ADC_In_5_Addr:    unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + ADC_In_5_addr_offset),   addr_width);  -- Adresse zum Lesen des ADC_In5
constant  C_ADC_In_6_Addr:    unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + ADC_In_6_addr_offset),   addr_width);  -- Adresse zum Lesen des ADC_In6
constant  C_ADC_In_7_Addr:    unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + ADC_In_7_addr_offset),   addr_width);  -- Adresse zum Lesen des ADC_In7
constant  C_ADC_In_8_Addr:    unsigned(addr_width-1 downto 0) := to_unsigned((Base_addr + ADC_In_8_addr_offset),   addr_width);  -- Adresse zum Lesen des ADC_In8


signal    S_DAC1_Config:     std_logic_vector(15 downto 0);
signal    S_DAC1_Config_Rd:  std_logic;
signal    S_DAC1_Config_Wr:  std_logic;

signal    S_DAC1:     std_logic_vector(15 downto 0);
signal    S_DAC1_Rd:  std_logic;
signal    S_DAC1_Wr:  std_logic;

signal    S_DAC2_Config:     std_logic_vector(15 downto 0);
signal    S_DAC2_Config_Rd:  std_logic;
signal    S_DAC2_Config_Wr:  std_logic;

signal    S_DAC2:     std_logic_vector(15 downto 0);
signal    S_DAC2_Rd:  std_logic;
signal    S_DAC2_Wr:  std_logic;
--
--
signal    S_ADC_Config:     std_logic_vector(15 downto 0);
signal    S_ADC_Config_Rd:  std_logic;
signal    S_ADC_Config_Wr:  std_logic;

signal    S_ADC_In1_Rd:      std_logic;
signal    S_ADC_In2_Rd:      std_logic;
signal    S_ADC_In3_Rd:      std_logic;
signal    S_ADC_In4_Rd:      std_logic;
signal    S_ADC_In5_Rd:      std_logic;
signal    S_ADC_In6_Rd:      std_logic;
signal    S_ADC_In7_Rd:      std_logic;
signal    S_ADC_In8_Rd:      std_logic;

signal    S_Dtack:        std_logic;
signal    S_Read_Port:    std_logic_vector(Data_to_SCUB'range);
signal    S_Read_Port_buf:    std_logic_vector(Data_to_SCUB'range);

signal	 rd_active_pulse:	std_logic;
signal	 rd_active_ff1:		std_logic;
signal	 rd_active_ff2:		std_logic;


begin

P_Adr_Deco:  process (nReset, clk)
  begin
    if nReset = '0' then
      
      S_DAC1_Config_Rd  <= '0'; S_DAC1_Config_Wr  <= '0';
      S_DAC1_Rd        <= '0';  S_DAC1_Wr        <= '0';
      S_DAC2_Config_Rd  <= '0'; S_DAC2_Config_Wr  <= '0';
      S_DAC2_Rd        <= '0';  S_DAC2_Wr        <= '0';

      S_ADC_Config_Rd  <= '0';
      S_ADC_Config_Wr  <= '0';
      S_ADC_In1_Rd <= '0';  S_ADC_In2_Rd <= '0';  S_ADC_In3_Rd <= '0';  S_ADC_In4_Rd <= '0';
      S_ADC_In5_Rd <= '0';  S_ADC_In6_Rd <= '0';  S_ADC_In7_Rd <= '0';  S_ADC_In8_Rd <= '0';
      
      S_Dtack <= '0';
      Rd_active <= '0';
    
    elsif rising_edge(clk) then

      S_DAC1_Config_Rd  <= '0';  S_DAC1_Config_Wr  <= '0';
      S_DAC1_Rd         <= '0';  S_DAC1_Wr        <= '0';
      S_DAC2_Config_Rd  <= '0';  S_DAC2_Config_Wr  <= '0';
      S_DAC2_Rd         <= '0';  S_DAC2_Wr        <= '0';

      S_ADC_Config_Rd  <= '0';
      S_ADC_Config_Wr  <= '0';
      S_ADC_In1_Rd <= '0';  S_ADC_In2_Rd <= '0';  S_ADC_In3_Rd <= '0';  S_ADC_In4_Rd <= '0';
      S_ADC_In5_Rd <= '0';  S_ADC_In6_Rd <= '0';  S_ADC_In7_Rd <= '0';  S_ADC_In8_Rd <= '0';

      S_Dtack <= '0';
      Rd_active <= '0';
      
      if Ext_Adr_Val = '1' then

        CASE unsigned(ADR_from_SCUB_LA) IS
            
--------------------- DAC ----------------------------------
          when C_DAC1_Config_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_DAC1_Config_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_DAC1_Config_Rd <= '1';
              Rd_active <= '1';
            end if;

          when C_DAC1_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_DAC1_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_DAC1_Rd <= '1';
              Rd_active <= '1';
            end if;

          when C_DAC2_Config_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_DAC2_Config_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_DAC2_Config_Rd <= '1';
              Rd_active <= '1';
            end if;

          when C_DAC2_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_DAC2_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_DAC2_Rd <= '1';
              Rd_active <= '1';
            end if;
--------------------- ADC ----------------------------------
          when C_ADC_Config_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_ADC_Config_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_ADC_Config_Rd <= '1';
              Rd_active <= '1';
            end if;

            
            when C_ADC_In_1_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_ADC_In1_Rd   <= '1';
              Rd_active <= '1';
            end if;

            when C_ADC_In_2_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_ADC_In2_Rd   <= '1';
              Rd_active <= '1';
            end if;

            when C_ADC_In_3_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_ADC_In3_Rd   <= '1';
              Rd_active <= '1';
            end if;

            when C_ADC_In_4_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_ADC_In4_Rd   <= '1';
              Rd_active <= '1';
            end if;

            when C_ADC_In_5_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_ADC_In5_Rd   <= '1';
              Rd_active <= '1';
            end if;

            when C_ADC_In_6_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_ADC_In6_Rd   <= '1';
              Rd_active <= '1';
            end if;

            when C_ADC_In_7_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_ADC_In7_Rd   <= '1';
              Rd_active <= '1';
            end if;

            when C_ADC_In_8_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_ADC_In8_Rd   <= '1';
              Rd_active <= '1';
            end if;
         
          when others => 
            
            S_DAC1_Config_Rd <= '0';  S_DAC1_Config_Wr <= '0';
            S_DAC1_Rd        <= '0';  S_DAC1_Wr        <= '0';
            S_DAC2_Config_Rd <= '0';  S_DAC2_Config_Wr <= '0';
            S_DAC2_Rd        <= '0';  S_DAC2_Wr        <= '0';

            S_ADC_Config_Rd  <= '0';
            S_ADC_Config_Wr  <= '0';
            S_ADC_In1_Rd <= '0';  S_ADC_In2_Rd <= '0';  S_ADC_In3_Rd <= '0';  S_ADC_In4_Rd <= '0';
            S_ADC_In5_Rd <= '0';  S_ADC_In6_Rd <= '0';  S_ADC_In7_Rd <= '0';  S_ADC_In8_Rd <= '0';
                      
            S_Dtack <= '0';
            Rd_active <= '0';

        end CASE;
      end if;
    end if;
  
  end process P_Adr_Deco;

  
  
P_Out_Reg:  process (nReset, clk, Data_from_SCUB_LA, 
                       S_DAC1_Config_Wr, S_DAC1_Wr, S_DAC2_Config_Wr, S_DAC2_Wr, S_ADC_Config_Wr)
  begin
    if nReset = '0' then
      S_DAC1_Config  <= (others => '0');
      S_DAC1         <= (others => '0');
      S_DAC2_Config  <= (others => '0');
      S_DAC2         <= (others => '0');
      S_ADC_Config   <= (others => '0');
    
    elsif rising_edge(clk) then
      
      if S_DAC1_Config_Wr = '1' then  S_DAC1_Config <= Data_from_SCUB_LA;   -- Output-Daten vom SCU-Bus
      end if;
      if S_DAC1_Wr = '1'        then  S_DAC1        <= Data_from_SCUB_LA;
      end if;            
      if S_DAC2_Config_Wr = '1' then  S_DAC2_Config <= Data_from_SCUB_LA;  
      end if;
      if S_DAC2_Wr = '1'        then  S_DAC2        <= Data_from_SCUB_LA;
      end if;
      if S_ADC_Config_Wr = '1'  then  S_ADC_Config  <= Data_from_SCUB_LA;
      end if;

  end if;
  end process P_Out_Reg;
  

  

  P_read_mux:  process (S_DAC1_Config_Rd,   S_DAC1_Config,
                        S_DAC1_Rd,          S_DAC1,
                        S_DAC2_Config_Rd,   S_DAC2_Config,
                        S_DAC2_Rd,          S_DAC2,
                        S_ADC_Config_Rd,    S_ADC_Config,
                        S_ADC_In1_Rd,       ADC_In1,
                        S_ADC_In2_Rd,       ADC_In2,
                        S_ADC_In3_Rd,       ADC_In3,
                        S_ADC_In4_Rd,       ADC_In4,
                        S_ADC_In5_Rd,       ADC_In5,
                        S_ADC_In6_Rd,       ADC_In6,
                        S_ADC_In7_Rd,       ADC_In7,
                        S_ADC_In8_Rd,       ADC_In8 )

  begin

    if    S_DAC1_Config_Rd  = '1' then  S_Read_Port <= S_DAC1_Config;
    elsif S_DAC1_Rd         = '1' then  S_Read_Port <= S_DAC1;
    elsif S_DAC2_Config_Rd  = '1' then  S_Read_Port <= S_DAC2_Config;
    elsif S_DAC2_Rd         = '1' then  S_Read_Port <= S_DAC2;
    elsif S_ADC_Config_Rd   = '1' then  S_Read_Port <= S_ADC_Config;

    elsif S_ADC_In1_Rd = '1' then  S_Read_Port <= ADC_In1;    -- read Input-Port1
    elsif S_ADC_In2_Rd = '1' then  S_Read_Port <= ADC_In2;
    elsif S_ADC_In3_Rd = '1' then  S_Read_Port <= ADC_In3;
    elsif S_ADC_In4_Rd = '1' then  S_Read_Port <= ADC_In4;
    elsif S_ADC_In5_Rd = '1' then  S_Read_Port <= ADC_In5;
    elsif S_ADC_In6_Rd = '1' then  S_Read_Port <= ADC_In6;
    elsif S_ADC_In7_Rd = '1' then  S_Read_Port <= ADC_In7;
    elsif S_ADC_In8_Rd = '1' then  S_Read_Port <= ADC_In8;

  else
      S_Read_Port <= (others => '-');
    end if;
  end process P_Read_mux;
  
  ADC_IN_Reg:  process (nReset, clk) 
  
  --Value change, at the beginning of rd_active (adress is valid) for just one clock cycle.
  --No change is allowed during reading.                      
  begin
    if rising_edge(clk) then
      rd_active_ff1 <= rd_active;
		  rd_active_ff2 <= rd_active_ff1;
	    rd_active_pulse <= rd_active and not rd_active_ff1 and not rd_active_ff2;
      if rd_active_pulse = '1' then 
		    S_Read_Port_buf <= S_Read_Port;
      end if;
    end if;
  end process ADC_IN_Reg;


  
-- Testport für Logic-Analysator
  
LA    <=      (x"000" & S_DAC1_Config_wr   & S_DAC2_wr    & S_DAC1_wr  &  S_ADC_Config_wr );   
 
Dtack_to_SCUB <= S_Dtack;
Data_to_SCUB <= S_Read_Port_buf;

DAC1_Config     <=  S_DAC1_Config;      -- Daten-Reg. DAC1_Config
DAC1_Config_wr  <=  S_DAC1_Config_Wr;   -- write Config-Reg. DAC1
DAC1_Out        <=  S_DAC1;             -- Daten-Reg. DAC1
DAC1_Out_wr     <=  S_DAC1_Wr;          -- write Daten-Reg. DAC1
DAC2_Config     <=  S_DAC2_Config;      -- Daten-Reg. DAC2_Config
DAC2_Config_wr  <=  S_DAC2_Config_Wr;   -- write Config-Reg. DAC2
DAC2_Out        <=  S_DAC2;             -- Daten-Reg. DAC2
DAC2_Out_wr     <=  S_DAC2_Wr;          -- write Daten-Reg. DAC2
ADC_Config      <=  S_ADC_Config;       -- Daten-Reg. ADC_Config

end Arch_addac_reg;