--TITLE "'config_status' Autor: R.Hartmann, Stand: 08.02.2016";
--

library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
--USE IEEE.std_logic_arith.all;

ENTITY config_status IS
  generic
      (
  CS_Base_addr:    INTEGER := 16#0500# );
    
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

    Diob_Status1:         in   std_logic_vector(15 downto 0);    -- Input-Port 1
    Diob_Status2:         in   std_logic_vector(15 downto 0);    -- Input-Port 2
    AW_Status1:           in   std_logic_vector(15 downto 0);    -- Input-Port 3
    AW_Status2:           in   std_logic_vector(15 downto 0);    -- Input-Port 4
   
    Diob_Config1:         out  std_logic_vector(15 downto 0);    -- Daten-Reg. AWOut1
    Diob_Config2:         out  std_logic_vector(15 downto 0);    -- Daten-Reg. AWOut2
    AW_Config1:           out  std_logic_vector(15 downto 0);    -- Daten-Reg. AWOut3
    AW_Config2:           out  std_logic_vector(15 downto 0);    -- Daten-Reg. AWOut4

    Mirr_OutReg_Maske:    out  std_logic_vector(15 downto 0);    -- Maske für den Spiegel-Modus des Ausgangsregisters
    
    Diob_Config1_wr:      out  std_logic;                        -- write-Strobe, Daten-Reg. Diob_Config1
    Diob_Config2_wr:      out  std_logic;                        -- write-Strobe, Daten-Reg. Diob_Config2
    AW_Config1_wr:        out  std_logic;                        -- write-Strobe, Daten-Reg. AW_Config1  
    AW_Config2_wr:        out  std_logic;                        -- write-Strobe, Daten-Reg. AW_Config2  
    Clr_Tag_Config:       out  std_logic;                        -- clear alle Tag-Konfigurationen

    Rd_active:            out  std_logic;                        -- read data available at 'Data_to_SCUB'-AWOut
    Data_to_SCUB:         out  std_logic_vector(15 downto 0);    -- connect read sources to SCUB-Macro
    Dtack_to_SCUB:        out  std_logic;                        -- connect Dtack to SCUB-Macro
    LA:                   out  std_logic_vector(15 downto 0)
    );  
  end config_status;


ARCHITECTURE Arch_config_status OF config_status IS



constant  addr_width:                 INTEGER := Adr_from_SCUB_LA'length;
--
constant  Diob_Config1_addr_offset:   INTEGER := 16#00#;    -- Offset zur Base_addr zum Setzen oder Rücklesen des Diob_Config1 Registers
constant  Diob_Config2_addr_offset:   INTEGER := 16#01#;    -- Offset zur Base_addr zum Setzen oder Rücklesen des Diob_Config2 Registers
constant  Diob_Status1_addr_offset:   INTEGER := 16#02#;    -- Offset zur Base_addr zum Rücklesen des AWIN_Port1
constant  Diob_Status2_addr_offset:   INTEGER := 16#03#;    -- Offset zur Base_addr zum Rücklesen des AWIN_Port2
  
constant  AW_Config1_addr_offset:     INTEGER := 16#07#;    -- Offset zur Base_addr zum Setzen oder Rücklesen des AW_Config1 Registers
constant  AW_Config2_addr_offset:     INTEGER := 16#08#;    -- Offset zur Base_addr zum Setzen oder Rücklesen des AW_Config2 Registers
constant  AW_Status1_addr_offset:     INTEGER := 16#09#;    -- Offset zur Base_addr zum Rücklesen des AWIN_Port3
constant  AW_Status2_addr_offset:     INTEGER := 16#0A#;    -- Offset zur Base_addr zum Rücklesen des AWIN_Port4

constant  Mirr_OutReg_Maske_offset:   INTEGER := 16#0E#;    -- Offset zur Base_addr zum Setzen oder Rücklesen der Maske für den Spiegel-Modus des Ausgangsregisters 1


constant  C_Diob_Config1_Addr:      unsigned(addr_width-1 downto 0) := to_unsigned((CS_Base_addr + Diob_Config1_addr_offset), addr_width);  -- Adresse zum Setzen oder Rücklesen des Diob_Config1 Registers
constant  C_Diob_Config2_Addr:      unsigned(addr_width-1 downto 0) := to_unsigned((CS_Base_addr + Diob_Config2_addr_offset), addr_width);  -- Adresse zum Setzen oder Rücklesen des Diob_Config2 Registers
constant  C_AW_Config1_Addr:        unsigned(addr_width-1 downto 0) := to_unsigned((CS_Base_addr + AW_Config1_addr_offset), addr_width);  -- Adresse zum Setzen oder Rücklesen des AW_Config1 Registers
constant  C_AW_Config2_Addr:        unsigned(addr_width-1 downto 0) := to_unsigned((CS_Base_addr + AW_Config2_addr_offset), addr_width);  -- Adresse zum Setzen oder Rücklesen des AW_Config2 Registers
      
constant  C_Diob_Status1_Addr:      unsigned(addr_width-1 downto 0) := to_unsigned((CS_Base_addr + Diob_Status1_addr_offset), addr_width);  -- Adresse zum Lesen des Diob_Status1
constant  C_Diob_Status2_Addr:      unsigned(addr_width-1 downto 0) := to_unsigned((CS_Base_addr + Diob_Status2_addr_offset), addr_width);  -- Adresse zum Lesen des Diob_Status2
constant  C_AW_Status1_Addr:        unsigned(addr_width-1 downto 0) := to_unsigned((CS_Base_addr + AW_Status1_addr_offset), addr_width);  -- Adresse zum Lesen des AW_Status1
constant  C_AW_Status2_Addr:        unsigned(addr_width-1 downto 0) := to_unsigned((CS_Base_addr + AW_Status2_addr_offset), addr_width);  -- Adresse zum Lesen des AW_Status2

constant  C_Mirr_OutReg_Maske_Addr: unsigned(addr_width-1 downto 0) := to_unsigned((CS_Base_addr + Mirr_OutReg_Maske_offset), addr_width);  -- Adresse zum Lesen der Maske für den Spiegel-Modus des Ausgangsregisters

signal    S_Diob_Config1:           std_logic_vector(15 downto 0);
signal    S_Diob_Config1_Rd:        std_logic;
signal    S_Diob_Config1_Wr:        std_logic;
      
signal    S_Diob_Config2:           std_logic_vector(15 downto 0);
signal    S_Diob_Config2_Rd:        std_logic;
signal    S_Diob_Config2_Wr:        std_logic;
      
signal    S_AW_Config1:             std_logic_vector(15 downto 0);
signal    S_AW_Config1_Rd:          std_logic;
signal    S_AW_Config1_Wr:          std_logic;
          
signal    S_AW_Config2:             std_logic_vector(15 downto 0);
signal    S_AW_Config2_Rd:          std_logic;
signal    S_AW_Config2_Wr:          std_logic;
      
signal    S_Diob_Status1_Rd:        std_logic;
signal    S_Diob_Status1_Wr:        std_logic;
signal    S_Diob_Status2_Rd:        std_logic;
signal    S_Diob_Status2_Wr:        std_logic;
signal    S_AW_Status1_Rd:          std_logic;
signal    S_AW_Status2_Rd:          std_logic;

signal    S_Mirr_OutReg_Maske:      std_logic_vector(15 downto 0);
signal    S_Mirr_OutReg_Maske_Rd:   std_logic;
signal    S_Mirr_OutReg_Maske_Wr:   std_logic;

signal    DIOB_Sts1_rd_Out:         std_logic_vector(15 downto 0); -- Zwischenspeicher
signal    DIOB_Sts1_rd_Strobe_o:    std_logic;        -- Output 
signal    DIOB_Sts1_rd_shift:       std_logic_vector(2  downto 0); -- Shift-Reg.
  
signal    s_Diob_Status1:           std_logic_vector(15 downto 0);
signal    s_Diob_Status2:           std_logic_vector(15 downto 0);
 
signal    S_Dtack:                  std_logic;
signal    S_Read_Port:              std_logic_vector(Data_to_SCUB'range);


begin

P_Adr_Deco:  process (nReset, clk)
  begin
    if nReset = '0' then
      
      S_Diob_Config1_Rd       <= '0';  S_Diob_Config1_Wr <= '0';
      S_Diob_Config2_Rd       <= '0';  S_Diob_Config2_Wr <= '0';
      S_AW_Config1_Rd         <= '0';  S_AW_Config1_Wr   <= '0';
      S_AW_Config2_Rd         <= '0';  S_AW_Config2_Wr   <= '0';
      
      S_Diob_Status1_Rd       <= '0';  S_Diob_Status1_Wr <= '0';
      S_Diob_Status2_Rd       <= '0';  S_Diob_Status2_Wr <= '0';
      S_AW_Status1_Rd         <= '0';
      S_AW_Status2_Rd         <= '0';

      S_Mirr_OutReg_Maske_Rd  <= '0';  S_Mirr_OutReg_Maske_Wr   <= '0';
      
      S_Dtack <= '0';
      Rd_active <= '0';
    
    elsif rising_edge(clk) then

      S_Diob_Config1_Rd       <= '0';  S_Diob_Config1_Wr        <= '0';
      S_Diob_Config2_Rd       <= '0';  S_Diob_Config2_Wr        <= '0';
      S_AW_Config1_Rd         <= '0';  S_AW_Config1_Wr          <= '0';
      S_AW_Config2_Rd         <= '0';  S_AW_Config2_Wr          <= '0';
              
      S_Diob_Status1_Rd       <= '0';  S_Diob_Status1_Wr        <= '0';
      S_Diob_Status2_Rd       <= '0';  S_Diob_Status2_Wr        <= '0';
      S_AW_Status1_Rd         <= '0';
      S_AW_Status2_Rd         <= '0';
      S_Mirr_OutReg_Maske_Rd  <= '0';  S_Mirr_OutReg_Maske_Wr   <= '0';

      S_Dtack <= '0';
      Rd_active <= '0';
      
      if Ext_Adr_Val = '1' then

        CASE unsigned(ADR_from_SCUB_LA) IS
            
          when C_Diob_Config1_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_Diob_Config1_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_Diob_Config1_Rd <= '1';
              Rd_active <= '1';
            end if;

          when C_Diob_Config2_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_Diob_Config2_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_Diob_Config2_Rd <= '1';
              Rd_active <= '1';
            end if;

          when C_AW_Config1_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AW_Config1_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AW_Config1_Rd <= '1';
              Rd_active <= '1';
            end if;

          when C_AW_Config2_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';
              S_AW_Config2_Wr <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack <= '1';
              S_AW_Config2_Rd <= '1';
              Rd_active <= '1';
            end if;

            
            when C_Diob_Status1_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';        
              S_Diob_Status1_Wr   <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_Diob_Status1_Rd   <= '1';
              Rd_active <= '1';
            end if;

            when C_Diob_Status2_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';        
              S_Diob_Status2_Wr   <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_Diob_Status2_Rd   <= '1';
              Rd_active <= '1';
            end if;

            when C_AW_Status1_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_AW_Status1_Rd   <= '1';
              Rd_active <= '1';
            end if;

            when C_AW_Status2_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '0';        -- kein DTACK beim Lese-Port
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_AW_Status2_Rd   <= '1';
              Rd_active <= '1';
            end if;
            
            when C_Mirr_OutReg_Maske_Addr =>
            if Ext_Wr_active = '1' then
              S_Dtack <= '1';        
              S_Mirr_OutReg_Maske_Wr   <= '1';
            end if;
            if Ext_Rd_active = '1' then
              S_Dtack     <= '1';
              S_Mirr_OutReg_Maske_Rd   <= '1';
              Rd_active <= '1';
            end if;
   
          when others => 
            
            S_Diob_Config1_Rd <= '0';  S_Diob_Config1_Wr <= '0';
            S_Diob_Config2_Rd <= '0';  S_Diob_Config2_Wr <= '0';
            S_AW_Config1_Rd   <= '0';  S_AW_Config1_Wr   <= '0';
            S_AW_Config2_Rd   <= '0';  S_AW_Config2_Wr   <= '0';

            S_Diob_Status1_Rd <= '0';  S_Diob_Status1_Wr <= '0'; 
            S_Diob_Status2_Rd <= '0';  S_Diob_Status2_Wr <= '0';   
            S_AW_Status1_Rd   <= '0';  S_AW_Status2_Rd   <= '0';

            S_Mirr_OutReg_Maske_Rd  <= '0';  S_Mirr_OutReg_Maske_Wr   <= '0';
                      
            S_Dtack <= '0';
            Rd_active <= '0';

        end CASE;
      end if;
    end if;
  
  end process P_Adr_Deco;

  
  
P_AWOut_Reg:  process (nReset, clk, 
                       S_Diob_Config1_Wr, S_Diob_Config2_Wr, S_AW_Config1_Wr, S_AW_Config2_Wr)
  begin
    if nReset = '0' then
      S_Diob_Config1 <= (others => '0');
      S_Diob_Config2 <= (others => '0');
      S_AW_Config1   <= (others => '0');
      S_AW_Config2   <= (others => '0');
    
    elsif rising_edge(clk) then
      
      if S_Diob_Config1_Wr = '1' then
        S_Diob_Config1(15 downto 1) <= Data_from_SCUB_LA(15 downto 1);   -- Output-Daten vom SCU-Bus
        S_Diob_Config1(0)           <= '0';                              -- mit Bit-0 werden die Tag-Masken gelöscht, wird nicht gespeichert
      end if;
      if S_Diob_Config2_Wr      = '1' then  S_Diob_Config2 <= Data_from_SCUB_LA;
      end if;
      if S_AW_Config1_Wr        = '1' then  S_AW_Config1 <= Data_from_SCUB_LA;
      end if;       
      if S_AW_Config2_Wr        = '1' then  S_AW_Config2 <= Data_from_SCUB_LA;
      end if;
      if S_Mirr_OutReg_Maske_Wr = '1' then  S_Mirr_OutReg_Maske <= Data_from_SCUB_LA;
      end if;

  end if;
  end process P_AWOut_Reg;
  

  P_read_mux:  process (S_Diob_Config1_Rd,      S_Diob_Config1,
                        S_Diob_Config2_Rd,      S_Diob_Config2,
                        S_AW_Config1_Rd,        S_AW_Config1,
                        S_AW_Config2_Rd,        S_AW_Config2,
                        S_Diob_Status1_Rd,      s_Diob_Status1,
                        S_Diob_Status2_Rd,      s_Diob_Status2,
                        S_AW_Status1_Rd,        AW_Status1,
                        S_AW_Status2_Rd,        AW_Status2,
                        S_Mirr_OutReg_Maske_Rd, S_Mirr_OutReg_Maske)

  begin

    if    S_Diob_Config1_Rd       = '1' then  S_Read_port <= S_Diob_Config1;
    elsif S_Diob_Config2_Rd       = '1' then  S_Read_port <= S_Diob_Config2;
    elsif S_AW_Config1_Rd         = '1' then  S_Read_port <= S_AW_Config1;
    elsif S_AW_Config2_Rd         = '1' then  S_Read_port <= S_AW_Config2;
      
    elsif S_Diob_Status1_Rd       = '1' then  S_Read_port <= s_Diob_Status1;    -- read Input-Port1
    elsif S_Diob_Status2_Rd       = '1' then  S_Read_port <= s_Diob_Status2;
    elsif S_AW_Status1_Rd         = '1' then  S_Read_port <= AW_Status1;
    elsif S_AW_Status2_Rd         = '1' then  S_Read_port <= AW_Status2;

    elsif S_Mirr_OutReg_Maske_Rd  = '1' then  S_Read_port <= s_Mirr_OutReg_Maske;

  else
      S_Read_Port <= (others => '-');
    end if;
  end process P_Read_mux;

  
--  
--
--
--  
----------- Reset DIOB-Status1 nach read (1 Clock breit) --------------------
--
--p_AW_Config1_Rd:  PROCESS (clk, nReset)
--  BEGin
--    IF  nReset                = '0' then
--        DIOB_Sts1_rd_shift    <= (OTHERS => '0');
--        DIOB_Sts1_rd_Strobe_o <= '0';
--
--    ELSIF rising_edge(clk) THEN
--
--      DIOB_Sts1_rd_shift <= (DIOB_Sts1_rd_shift(DIOB_Sts1_rd_shift'high-1 downto 0) & (Not S_Diob_Status1_Rd)); -- DIOB_Sts1_rd_Strobe_o = Puls, nach der neg. Flanke von S_AW_Config_Rd 
--
--      IF DIOB_Sts1_rd_shift(DIOB_Sts1_rd_shift'high) = '0' AND DIOB_Sts1_rd_shift(DIOB_Sts1_rd_shift'high-1) = '1' THEN
--        DIOB_Sts1_rd_Strobe_o <= '1';
--      ELSE
--        DIOB_Sts1_rd_Strobe_o <= '0';
--      END IF;
--    END IF;
--  END PROCESS p_AW_Config1_Rd;
--
--
--


  
--------- DIOB_Staus_1 --------------------

P_DIOB_Staus_1:  process (clk, nReset)
  begin
    IF  nReset  = '0' then   s_Diob_Status1  <= (OTHERS => '0');

    elsif rising_edge(clk) then
      
      if        (                               Diob_Status1     (15) = '1')   then s_Diob_Status1(15)  <= '1';
        else if ((S_Diob_Status1_Wr = '1') and (Data_from_SCUB_LA(15) = '1'))  then s_Diob_Status1(15)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status1     (14) = '1')   then s_Diob_Status1(14)  <= '1';
        else if ((S_Diob_Status1_Wr = '1') and (Data_from_SCUB_LA(14) = '1'))  then s_Diob_Status1(14)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status1     (13) = '1')   then s_Diob_Status1(13)  <= '1';
        else if ((S_Diob_Status1_Wr = '1') and (Data_from_SCUB_LA(13) = '1'))  then s_Diob_Status1(13)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status1     (12) = '1')   then s_Diob_Status1(12)  <= '1';
        else if ((S_Diob_Status1_Wr = '1') and (Data_from_SCUB_LA(12) = '1'))  then s_Diob_Status1(12)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status1     (11) = '1')   then s_Diob_Status1(11)  <= '1';
        else if ((S_Diob_Status1_Wr = '1') and (Data_from_SCUB_LA(11) = '1'))  then s_Diob_Status1(11)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status1     (10) = '1')   then s_Diob_Status1(10)  <= '1';
        else if ((S_Diob_Status1_Wr = '1') and (Data_from_SCUB_LA(10) = '1'))  then s_Diob_Status1(10)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status1     ( 9) = '1')   then s_Diob_Status1( 9)  <= '1';
        else if ((S_Diob_Status1_Wr = '1') and (Data_from_SCUB_LA( 9) = '1'))  then s_Diob_Status1( 9)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status1     ( 8) = '1')   then s_Diob_Status1( 8)  <= '1';
        else if ((S_Diob_Status1_Wr = '1') and (Data_from_SCUB_LA( 8) = '1'))  then s_Diob_Status1( 8)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status1     ( 7) = '1')   then s_Diob_Status1( 7)  <= '1';
        else if ((S_Diob_Status1_Wr = '1') and (Data_from_SCUB_LA( 7) = '1'))  then s_Diob_Status1( 7)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status1     ( 6) = '1')   then s_Diob_Status1( 6)  <= '1';
        else if ((S_Diob_Status1_Wr = '1') and (Data_from_SCUB_LA( 6) = '1'))  then s_Diob_Status1( 6)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status1     ( 5) = '1')   then s_Diob_Status1( 5)  <= '1';
        else if ((S_Diob_Status1_Wr = '1') and (Data_from_SCUB_LA( 5) = '1'))  then s_Diob_Status1( 5)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status1     ( 4) = '1')   then s_Diob_Status1( 4)  <= '1';
        else if ((S_Diob_Status1_Wr = '1') and (Data_from_SCUB_LA( 4) = '1'))  then s_Diob_Status1( 4)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status1     ( 3) = '1')   then s_Diob_Status1( 3)  <= '1';
        else if ((S_Diob_Status1_Wr = '1') and (Data_from_SCUB_LA( 3) = '1'))  then s_Diob_Status1( 3)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status1     ( 2) = '1')   then s_Diob_Status1( 2)  <= '1';
        else if ((S_Diob_Status1_Wr = '1') and (Data_from_SCUB_LA( 2) = '1'))  then s_Diob_Status1( 2)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status1     ( 1) = '1')   then s_Diob_Status1( 1)  <= '1';
        else if ((S_Diob_Status1_Wr = '1') and (Data_from_SCUB_LA( 1) = '1'))  then s_Diob_Status1( 1)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status1     ( 0) = '1')   then s_Diob_Status1( 0)  <= '1';
        else if ((S_Diob_Status1_Wr = '1') and (Data_from_SCUB_LA( 0) = '1'))  then s_Diob_Status1( 0)  <= '0'; end if; end if;  -- Clear Stausbit


      end if;
  end process P_DIOB_Staus_1;
  
  
--------- DIOB_Staus_2 --------------------

P_DIOB_Staus_2:  process (clk, nReset)
  begin
    IF  nReset  = '0' then   s_Diob_Status2  <= (OTHERS => '0');

    elsif rising_edge(clk) then
      
      if        (                               Diob_Status2     (15) = '1')   then s_Diob_Status2(15)  <= '1';
        else if ((S_Diob_Status2_Wr = '1') and (Data_from_SCUB_LA(15) = '1'))  then s_Diob_Status2(15)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status2     (14) = '1')   then s_Diob_Status2(14)  <= '1';
        else if ((S_Diob_Status2_Wr = '1') and (Data_from_SCUB_LA(14) = '1'))  then s_Diob_Status2(14)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status2     (13) = '1')   then s_Diob_Status2(13)  <= '1';
        else if ((S_Diob_Status2_Wr = '1') and (Data_from_SCUB_LA(13) = '1'))  then s_Diob_Status2(13)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status2     (12) = '1')   then s_Diob_Status2(12)  <= '1';
        else if ((S_Diob_Status2_Wr = '1') and (Data_from_SCUB_LA(12) = '1'))  then s_Diob_Status2(12)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status2     (11) = '1')   then s_Diob_Status2(11)  <= '1';
        else if ((S_Diob_Status2_Wr = '1') and (Data_from_SCUB_LA(11) = '1'))  then s_Diob_Status2(11)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status2     (10) = '1')   then s_Diob_Status2(10)  <= '1';
        else if ((S_Diob_Status2_Wr = '1') and (Data_from_SCUB_LA(10) = '1'))  then s_Diob_Status2(10)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status2     ( 9) = '1')   then s_Diob_Status2( 9)  <= '1';
        else if ((S_Diob_Status2_Wr = '1') and (Data_from_SCUB_LA( 9) = '1'))  then s_Diob_Status2( 9)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status2     ( 8) = '1')   then s_Diob_Status2( 8)  <= '1';
        else if ((S_Diob_Status2_Wr = '1') and (Data_from_SCUB_LA( 8) = '1'))  then s_Diob_Status2( 8)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status2     ( 7) = '1')   then s_Diob_Status2( 7)  <= '1';
        else if ((S_Diob_Status2_Wr = '1') and (Data_from_SCUB_LA( 7) = '1'))  then s_Diob_Status2( 7)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status2     ( 6) = '1')   then s_Diob_Status2( 6)  <= '1';
        else if ((S_Diob_Status2_Wr = '1') and (Data_from_SCUB_LA( 6) = '1'))  then s_Diob_Status2( 6)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status2     ( 5) = '1')   then s_Diob_Status2( 5)  <= '1';
        else if ((S_Diob_Status2_Wr = '1') and (Data_from_SCUB_LA( 5) = '1'))  then s_Diob_Status2( 5)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status2     ( 4) = '1')   then s_Diob_Status2( 4)  <= '1';
        else if ((S_Diob_Status2_Wr = '1') and (Data_from_SCUB_LA( 4) = '1'))  then s_Diob_Status2( 4)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status2     ( 3) = '1')   then s_Diob_Status2( 3)  <= '1';
        else if ((S_Diob_Status2_Wr = '1') and (Data_from_SCUB_LA( 3) = '1'))  then s_Diob_Status2( 3)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status2     ( 2) = '1')   then s_Diob_Status2( 2)  <= '1';
        else if ((S_Diob_Status2_Wr = '1') and (Data_from_SCUB_LA( 2) = '1'))  then s_Diob_Status2( 2)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status2     ( 1) = '1')   then s_Diob_Status2( 1)  <= '1';
        else if ((S_Diob_Status2_Wr = '1') and (Data_from_SCUB_LA( 1) = '1'))  then s_Diob_Status2( 1)  <= '0'; end if; end if;  -- Clear Stausbit
                                                                                                  
      if        (                               Diob_Status2     ( 0) = '1')   then s_Diob_Status2( 0)  <= '1';
        else if ((S_Diob_Status2_Wr = '1') and (Data_from_SCUB_LA( 0) = '1'))  then s_Diob_Status2( 0)  <= '0'; end if; end if;  -- Clear Stausbit


      end if;
  end process P_DIOB_Staus_2;

  
  
  
-- Testport für Logic-Analysator
  
LA <=      (x"000" & 
             S_AW_Config2_wr      & S_AW_Config1_wr    & S_Diob_Config2_wr  &  S_Diob_Config1_wr );   
 
Dtack_to_SCUB <= S_Dtack;
Data_to_SCUB <= S_Read_Port;

Diob_Config1      <=  s_Diob_Config1;       -- Daten-Reg. AWOut1
Diob_Config2      <=  s_Diob_Config2;       -- Daten-Reg. AWOut2
AW_Config1        <=  s_AW_Config1;         -- Daten-Reg. AWOut3
AW_Config2        <=  s_AW_Config2;         -- Daten-Reg. AWOut4

Mirr_OutReg_Maske <=  s_Mirr_OutReg_Maske;  -- Maskierung für Spiegel-Modus des Ausgangsregisters



Diob_Config1_wr   <=  S_Diob_Config1_Wr;  -- write-Strobe, Daten-Reg. AWOut1
Diob_Config2_wr   <=  S_Diob_Config2_Wr;  -- write-Strobe, Daten-Reg. AWOut2
AW_Config1_wr     <=  S_AW_Config1_Wr;    -- write-Strobe, Daten-Reg. AWOut3
AW_Config2_wr     <=  S_AW_Config2_Wr;    -- write-Strobe, Daten-Reg. AWOut4
Clr_Tag_Config    <= (S_Diob_Config1_Wr and Data_from_SCUB_LA(0)); -- clear alle Tag-Masken


end Arch_config_status;