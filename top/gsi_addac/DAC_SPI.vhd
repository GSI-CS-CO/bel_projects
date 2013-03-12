LIBRARY ieee;
use ieee.std_logic_1164.all;
use IEEE.numeric_std.all;
use work.aux_functions_pkg.all;

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
    DAC_SI:             out   std_logic;                      -- connect to DAC-SDI
    nDAC_CLK:           out   std_logic;                      -- spi-clock of DAC
    nCS_DAC:            out   std_logic;                      -- '0' enable shift of internal shift register of DAC
    nLD_DAC:            out   std_logic;                      -- '0' copy shift register to output latch of DAC
    nCLR_DAC:           out   std_logic;
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



  signal    Shift_Reg:        unsigned(15 DOWNTO 0);
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
              Load_End
              );  

  signal    SPI_SM:           T_SPI_SM;

  signal    Bit_Cnt:          unsigned(4 DOWNTO 0);

  signal    spi_clk:          std_logic;  

  signal    S_nCLR_DAC:       std_logic;

  signal    SPI_TRM:          std_logic;
  
  signal    modelsim_nReset:  std_logic;

  
begin

modelsim_nReset <= not nReset;

spi_clk_gen:  div_n
  generic map (
    n       => c_spi_clk_ena_cnt,
    diag_on => 0
    )
  port map (
    res     => modelsim_nReset, -- in, '1' => set "div_n"-counter asynchron to generic-value "n"-2, so the 
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
  
    elsif rising_edge(clk) then

      if Wr_Shift_Reg = '1' AND Ext_Wr_Fin = '1' then
        SPI_TRM <= '1';
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
              spi_clk <= '1';
              SPI_SM <= Sel_Off;
            end if;
            
          when CLK_Hi =>
            spi_clk <= '1';
            Bit_Cnt <= Bit_Cnt + 1;
            SPI_SM <= CLK_Lo;

          when Sel_Off =>
            nCS_DAC <= '1';
            SPI_SM <= Load;

          when Load =>
            nLD_DAC <= '0';
            SPI_SM <= Load_end;

          when Load_End =>
            SPI_TRM <= '0';
            nLD_DAC <= '0';
            SPI_SM <= Idle;

        end case;
      end if;
    end if;
  end process P_SPI_SM;


P_Shift_Reg:  process (clk, nReset)
  begin
    if  nReset = '0' then
      Shift_Reg <= (others => '0');
    elsif rising_edge(clk) then
      if Wr_Shift_Reg = '1' then
        Shift_Reg <= unsigned(Data_from_SCUB_LA);
      elsif (SPI_SM = CLK_Lo) AND (spi_clk_ena = '1') and (Bit_Cnt > 0) then
        Shift_Reg <= (Shift_Reg(Shift_Reg'high-1 DOWNTO 0) & '0');
      end if;
    end if;
  end process P_Shift_Reg;

P_DAC_Cntrl:  process (clk, nReset)
  begin
    if  nReset = '0' then
      S_nCLR_DAC <= '0';
    elsif rising_edge(clk) then
      S_nCLR_DAC <= '1';
      if Wr_DAC_Cntrl = '1' then
        if Data_from_SCUB_LA(0) = '1' then
          S_nCLR_DAC <= '0';
        end if;
      end if;
    end if;
  end process P_DAC_Cntrl;


DAC_SI  <= Shift_Reg(Shift_Reg'high);
nDAC_CLK <= spi_clk; 
nCLR_DAC <= S_nCLR_DAC;
  
Rd_Port <= (X"000" & '0' & '0' & '0' & SPI_TRM);  
  
  
Dtack <= S_Dtack;

Rd_Activ <= Rd_DAC_Cntrl;

end Arch_DAC_SPI;
