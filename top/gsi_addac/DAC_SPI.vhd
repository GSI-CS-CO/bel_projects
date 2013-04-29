LIBRARY ieee;
use ieee.std_logic_1164.all;
use IEEE.numeric_std.all;
use work.aux_functions_pkg.all;

----------------------------------------------------------------------------------------------------------------------
--  Vers: 1 Revi: 0: erstellt am 25.04.2013, Autor: W.Panschow                                                      --

entity DAC_SPI is
  generic (
    Base_addr:        unsigned(15 downto 0) := X"0300";
    CLK_in_Hz:        integer := 100_000_000;
    SPI_CLK_in_Hz:    integer := 10_000_000
    );
  port
    (
    Adr_from_SCUB_LA:   in    std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
    Data_from_SCUB_LA:  in    std_logic_vector(15 downto 0);  -- latched data from SCU_Bus 
    Ext_Adr_Val:        in    std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:      in    std_logic;                      -- '1' => Rd-Cycle is active
    Ext_Wr_active:      in    std_logic;                      -- '1' => Wr-Cycle is active
    Ext_Wr_fin:         in    std_logic;                      -- '1' => Wr-Cycle is finished
    clk:                in    std_logic;                      -- should be the same clk, used by SCU_Bus_Slave
    nReset:             in    std_logic := '1';
    nExt_Trig_DAC:      in    std_logic;                      -- external trigger input over optocoupler,
                                                              -- led on -> nExt_Trig_DAC is low
    DAC_SI:             out   std_logic;                      -- connect to DAC-SDI
    nDAC_CLK:           out   std_logic;                      -- spi-clock of DAC
    nCS_DAC:            out   std_logic;                      -- '0' enable shift of internal shift register of DAC
    nLD_DAC:            out   std_logic;                      -- '0' copy shift register to output latch of DAC
    nCLR_DAC:           out   std_logic;                      -- '0' resets the DAC, Clear Pulsewidth min 200ns
                                                              -- resets both the input latch and the D/A latch to 0000H (midscale).
    Rd_Port:            out   std_logic_vector(15 downto 0);
    Rd_Activ:           out   std_logic;
    Dtack:              out   std_logic
    );
end DAC_SPI;



architecture arch_DAC_SPI OF DAC_SPI IS


  constant  rw_dac_Cntrl_addr_offset:   unsigned(15 downto 0) := X"0000";
  constant  wr_dac_addr_offset:         unsigned(15 downto 0) := X"0001";


  constant  rw_dac_cntrl_addr:  unsigned(15 downto 0) := Base_addr + rw_dac_cntrl_addr_offset;
  constant  wr_dac_addr:        unsigned(15 downto 0) := Base_addr + wr_dac_addr_offset;

  constant  c_spi_clk_ena_cnt:  integer := (clk_in_hz / spi_clk_in_hz) / 2;

  signal    spi_clk_ena:        std_logic;

  signal    Shift_Reg:        unsigned(15 downto 0);
  signal    Wr_Shift_Reg:     std_logic;
  signal    Wr_DAC_Cntrl:     std_logic;
  signal    Rd_DAC_Cntrl:     std_logic;

  signal    S_Dtack:          std_logic;

  TYPE      T_SPI_SM  IS (
              Idle,
              Sel_On,   
              Clk_Lo, 
              Clk_Hi,
              Sel_Off,
              Load,
              load_Wait,
              Load_End
              );  

  signal    SPI_SM:           T_SPI_SM;

  signal    Bit_Cnt:          unsigned(4 DOWNTO 0);

  signal    spi_clk:          std_logic;  

  signal    S_nCLR_DAC:         std_logic;
  signal    dac_conv_extern:    std_logic;
  signal    dac_neg_edge_conv:  std_logic;
  
  signal    build_edge:             std_logic_vector(2 downto 0);   -- shift_reg to detect external tirgger edge
  signal    Trig_DAC_with_old_data: std_logic;
  signal    Trig_DAC_during_shift:  std_logic;
  signal    Trig_DAC:               std_logic;
  signal    SPI_TRM_during_previous_cycle_active: std_logic;

  signal    SPI_TRM:          std_logic;
  

  
begin


spi_clk_gen:  div_n
  generic map (
    n       => c_spi_clk_ena_cnt,
    diag_on => 0
    )
  port map (
    res     => not nReset,      -- in, '1' => set "div_n"-counter asynchron to generic-value "n"-2, so the 
                                --     countdown is "n"-1 clocks to activate the "div_o"-output for one clock periode. 
    clk     => clk,             -- clk = clock
    ena     => '1',             -- in, can be used for a reduction, signal should be generated from the same 
                                --     clock domain and should be only one clock period active.
    div_o   => spi_clk_ena      -- out, div_o becomes '1' for one clock period, if "div_n" arrive n-1 clocks
                                --      (if ena is permanent '1').
    );


P_DAC_SPI_Adr_Deco: process (clk, nReset)
  begin
    if nReset = '0' then

      Wr_DAC_Cntrl  <= '0';
      Rd_DAC_Cntrl  <= '0';
      Wr_Shift_Reg  <= '0';
      S_Dtack     <= '0';

    elsif rising_edge(clk) then
    
      Wr_DAC_Cntrl  <= '0';
      Rd_DAC_Cntrl  <= '0';
      Wr_Shift_Reg  <= '0';
      S_Dtack     <= '0';

      if Ext_Adr_Val = '1' then

        case unsigned(Adr_from_SCUB_LA) IS

          when rw_dac_cntrl_addr =>
            if Ext_Wr_active = '1' then
              Wr_DAC_Cntrl  <= '1';
              S_Dtack     <= '1';
            end if;
            if Ext_Rd_active = '1' then
              Rd_DAC_Cntrl  <= '1';
              S_Dtack     <= '1';
            end if;
            
          when wr_dac_addr =>
            if Ext_Wr_active = '1' then
              Wr_Shift_Reg  <= '1';
              S_Dtack     <= '1';
            end if;

          when others =>
            Wr_DAC_Cntrl  <= '0';
            Rd_DAC_Cntrl  <= '0';
            Wr_Shift_Reg  <= '0';
            S_Dtack     <= '0';

        end case;
      end if;
    end if;
  end process P_DAC_SPI_Adr_Deco;


P_SPI_SM: process (clk, nReset, S_nCLR_DAC)
  
  begin
    if (nReset = '0') OR (S_nCLR_DAC = '0') then
      SPI_SM <= Idle;
      Bit_Cnt <= (others => '0');
      nCS_DAC <= '1';
      spi_clk <= '1';
      nLD_DAC <= '1';
      SPI_TRM <= '0';
      SPI_TRM_during_previous_cycle_active <= '0';
  
    elsif rising_edge(clk) then

      if (Wr_Shift_Reg = '1') and (S_Dtack = '1') then
        if (SPI_TRM = '1') and (SPI_SM /= Idle) then
          SPI_TRM_during_previous_cycle_active <= '1';
        else
          SPI_TRM <= '1';
        end if;
      end if;

      if spi_clk_ena = '1' then
      
        case SPI_SM IS

          when Idle =>
            Bit_Cnt <= (others => '0');
            nCS_DAC <= '1';
            spi_clk <= '1';
            nLD_DAC <= '1';
            if SPI_TRM = '1' then
              SPI_SM <= Sel_On;
            end if;

          when Sel_On =>
            nCS_DAC <= '0';
            SPI_SM <= CLK_Lo;

          when CLK_Lo =>
            if Bit_Cnt <= Shift_Reg'length-1 then
              spi_clk <= '0';
              SPI_SM <= CLK_Hi;
            ELSE
              spi_clk <= '0';
              nCS_DAC <= '1';
              SPI_SM <= Sel_Off;
            end if;

          when CLK_Hi =>
            spi_clk <= '1';
            Bit_Cnt <= Bit_Cnt + 1;
            SPI_SM <= CLK_Lo;

          when Sel_Off =>
            spi_clk <= '1';
            if dac_conv_extern = '1' then
              SPI_TRM <= '0';               -- during extern trigger wait, reset the SPI_TRM,
                                            -- so you can recognize an new SPI_TRM during wait for external Trigger
            end if;
            SPI_SM <= Load;

          when Load =>
-- hier noch den Abbruch des waits einbauen, wenn neues Datum geschrieben wurde.
            spi_clk <= '0';
            if dac_conv_extern = '0' then
              nLD_DAC <= '0';             -- software driven so load directly
              SPI_SM <= Load_end;
            else
              if Trig_DAC = '1' then      -- extenal driven load, so wait on Trig_DAC
                nLD_DAC <= '0';
                SPI_SM <= Load_End;
              else
                SPI_SM <= Load_wait;
              end if;
            end if;

          when Load_wait =>               -- wait external driven load
            spi_clk <= '1';
            SPI_SM <= Load;

          when Load_End =>
            spi_clk <= '1';
            SPI_TRM <= '0';
            nLD_DAC <= '0';
            SPI_SM <= Idle;

        end case;
      end if;
    end if;
  end process P_SPI_SM;


P_Ext_Trig:   process (clk)
  begin
    if rising_edge(clk) then
      if dac_conv_extern = '1' then
        -- shift_reg to detect external tirgger
        build_edge <= build_edge(build_edge'high-1 downto 0) & (dac_neg_edge_conv xor nExt_Trig_DAC);
          if build_edge(build_edge'high) = '0' and build_edge(build_edge'high-1) = '1' then   
            -- external trigger (edge) detected
            if SPI_SM = Idle then
              -- during idle state, so no new DAC data available
              Trig_DAC_with_old_data <= '1';
            elsif (SPI_SM = Sel_On) or (SPI_SM = Clk_Lo) or (SPI_SM = Clk_Hi) then
              -- during DAC data spi-shift-operation, so its to early
              Trig_DAC_during_shift <= '1';
              Trig_DAC <= '1';
            elsif (SPI_SM = Sel_Off) or (SPI_SM = Load) or (SPI_SM = Load_wait) then
              -- during Wait for external trigger, so it's okay
              Trig_DAC <= '1';
            elsif (SPI_SM = load_end) then
              -- tigger is executed, so reset 
              Trig_DAC_during_shift <= '0';
              Trig_DAC <= '0';
            end if;
          else
            Trig_DAC_with_old_data <= '0';
          end if;
      else
        Trig_DAC_with_old_data <= '0';
        Trig_DAC_during_shift <= '0';
        build_edge <= "000";
        Trig_DAC <= '0';
      end if;
    end if;
  end process P_Ext_Trig;


P_Shift_Reg:  process (clk, nReset)
  begin
    if  nReset = '0' then
      Shift_Reg <= (others => '0');
    elsif rising_edge(clk) then
      if Wr_Shift_Reg = '1' and ((SPI_SM = Idle) or (SPI_SM = Load) or (SPI_SM = Load_wait)) then
        Shift_Reg <= unsigned(Data_from_SCUB_LA);
      elsif (SPI_SM = CLK_Lo) AND (spi_clk_ena = '1') and (Bit_Cnt > 0) then
        Shift_Reg <= (Shift_Reg(Shift_Reg'high-1 DOWNTO 0) & '0');
      end if;
    end if;
  end process P_Shift_Reg;


P_DAC_Cntrl:  process (clk, nReset)
  variable  clr_cnt:  unsigned(1 downto 0) := "00";
  begin
    if nReset = '0' then
      S_nCLR_DAC        <= '0';
      dac_conv_extern   <= '0';
      dac_neg_edge_conv <= '0';
      clr_cnt           := "00";
    elsif rising_edge(clk) then
      if Wr_DAC_Cntrl = '1' then
        if Data_from_SCUB_LA(1) = '1' then  -- arm clear signal of DAC
          S_nCLR_DAC <= '0';
          clr_cnt := "00";                  -- reset the counter for generating the correct length of clear signal
        end if;
        if Data_from_SCUB_LA(2) = '1' then  -- enable external convert dac strobe
          dac_conv_extern <= '1';
        end if;
        if Data_from_SCUB_LA(3) = '1' then  -- negative edge convert dac strobe
          dac_neg_edge_conv <= '1';
        end if;
      else
        if S_nCLR_DAC = '0' then
          if spi_clk_ena = '1' then 
            if clr_cnt < 3 then
              clr_cnt := clr_cnt + 1;
            else
              S_nCLR_DAC <= '1';
            end if;
          end if;
        end if;
      end if;
    end if;
  end process P_DAC_Cntrl;

  
DAC_SI  <= Shift_Reg(Shift_Reg'high);


nDAC_CLK <= not spi_clk; 
nCLR_DAC <= S_nCLR_DAC;
  
Rd_Port <= (X"000" & dac_neg_edge_conv & dac_conv_extern & not S_nCLR_DAC & SPI_TRM);  
  
  
Dtack <= S_Dtack;

Rd_Activ <= Rd_DAC_Cntrl;

end Arch_DAC_SPI;
