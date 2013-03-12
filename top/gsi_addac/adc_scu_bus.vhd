library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.adc_pkg.all;

entity adc_scu_bus is
  generic (
    Base_addr:            unsigned(15 downto 0);
    clk_in_hz:            integer := 50_000_000;        -- 50Mhz
    diag_on_is_1:         integer range 0 to 1 := 0);   -- if 1 then diagnosic information is generated during compilation
  port (
    clk:            in std_logic;
    nrst:           in std_logic;
    
    -- ADC interface
    db:             in std_logic_vector(13 downto 0); -- databus from the ADC
    db14_hben:      inout std_logic;                  -- hben in mode ser
    db15_byte_sel:  inout std_logic;                  -- byte sel in mode ser
    convst_a:       out std_logic;                    -- start conversion for channels 1-4
    convst_b:       out std_logic;                    -- start conversion for channels 5-8
    n_cs:           out std_logic;                    -- chipselect, enables tri state databus
    n_rd_sclk:      out std_logic;                    -- first falling edge after busy clocks data out
    busy:           in std_logic;                     -- falling edge signals end of conversion
    adc_reset:      out std_logic;
    os:             out std_logic_vector(2 downto 0); -- oversampling config
    par_ser_sel:    out std_logic;                    -- parallel/serial/byte serial
    adc_range:      out std_logic;                    -- 10V/-10V or 5V/-5V
    firstdata:      in std_logic;
    
    -- SCUB interface
    Adr_from_SCUB_LA:   in    std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
    Data_from_SCUB_LA:  in    std_logic_vector(15 downto 0);  -- latched data from SCU_Bus 
    Ext_Adr_Val:        in    std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:      in    std_logic;                      -- '1' => Rd-Cycle is active
    Ext_Wr_active:      in    std_logic;                      -- '1' => Wr-Cycle is active
    user_rd_active:     out   std_logic;                      -- '1' = read data available at 'Data_to_SCUB'-output
    Data_to_SCUB:       out   std_logic_vector(15 downto 0);  -- connect read sources to SCUB-Macro
    Dtack_to_SCUB:      out   std_logic);                     -- connect Dtack to SCUB-Macro
end entity;


architecture adc_scu_bus_arch of adc_scu_bus is

signal  channel_1, channel_2, channel_3, channel_4,
        channel_5, channel_6, channel_7, channel_8: std_logic_vector(15 downto 0);
constant cntrl_reg_adr: unsigned(15 downto 0) := Base_addr + x"0000";
constant chn_1_reg_adr: unsigned(15 downto 0) := Base_addr + x"0001";
constant chn_2_reg_adr: unsigned(15 downto 0) := Base_addr + x"0002";
constant chn_3_reg_adr: unsigned(15 downto 0) := Base_addr + x"0003";
constant chn_4_reg_adr: unsigned(15 downto 0) := Base_addr + x"0004";
constant chn_5_reg_adr: unsigned(15 downto 0) := Base_addr + x"0005";
constant chn_6_reg_adr: unsigned(15 downto 0) := Base_addr + x"0006";
constant chn_7_reg_adr: unsigned(15 downto 0) := Base_addr + x"0007";
constant chn_8_reg_adr: unsigned(15 downto 0) := Base_addr + x"0008";

signal wr_adc_cntrl: std_logic;
signal rd_adc_cntrl: std_logic;
signal rd_adc_chn_1: std_logic;
signal rd_adc_chn_2: std_logic;
signal rd_adc_chn_3: std_logic;
signal rd_adc_chn_4: std_logic;
signal rd_adc_chn_5: std_logic;
signal rd_adc_chn_6: std_logic;
signal rd_adc_chn_7: std_logic;
signal rd_adc_chn_8: std_logic;

signal dtack:         std_logic;

begin

adc: ad7606
  generic map (
    clk_in_Hz     => clk_in_Hz,
    diag_on_is_1  => 0)
  port map (
    clk           =>  clk,
    nrst          =>  nrst,
    conv_en       => '1',
    transfer_mode => "00",
    db            => db,
    db14_hben     => db14_hben,
    db15_byte_sel => db15_byte_sel,
    convst_a      => convst_a,
    convst_b      => convst_b,
    n_cs          => n_cs,
    n_rd_sclk     => n_rd_sclk,
    busy          => busy,
    adc_reset     => adc_reset,
    os            => os,
    par_ser_sel   => par_ser_sel,
    adc_range     => adc_range,
    firstdata     => firstdata,
    channel_1     => channel_1,
    channel_2     => channel_2,
    channel_3     => channel_3,
    channel_4     => channel_4,
    channel_5     => channel_5,
    channel_6     => channel_6,
    channel_7     => channel_7,
    channel_8     => channel_8);
    
adr_decoder: process (clk, nrst)
begin
  if nrst = '0' then
    wr_adc_cntrl  <= '0';
    rd_adc_cntrl  <= '0';
    rd_adc_chn_1  <= '0';
    rd_adc_chn_2  <= '0';
    rd_adc_chn_3  <= '0';
    rd_adc_chn_4  <= '0';
    rd_adc_chn_5  <= '0';
    rd_adc_chn_6  <= '0';
    rd_adc_chn_7  <= '0';
    rd_adc_chn_8  <= '0';
    dtack         <= '0';
  elsif rising_edge(clk) then
    wr_adc_cntrl  <= '0';
    rd_adc_cntrl  <= '0';
    rd_adc_chn_1  <= '0';
    rd_adc_chn_2  <= '0';
    rd_adc_chn_3  <= '0';
    rd_adc_chn_4  <= '0';
    rd_adc_chn_5  <= '0';
    rd_adc_chn_6  <= '0';
    rd_adc_chn_7  <= '0';
    rd_adc_chn_8  <= '0';
    dtack         <= '0';
    
    if Ext_Adr_Val = '1' then
      case unsigned(Adr_from_SCUB_LA) is
        when cntrl_reg_adr =>
          if Ext_Wr_active = '1' then
            wr_adc_cntrl  <= '1';
            dtack         <= '1';
          end if;
          if Ext_Rd_active = '1' then
            rd_adc_cntrl  <= '1';
            dtack         <= '1';
          end if;
        
        when chn_1_reg_adr =>
          if Ext_Rd_active = '1' then
            rd_adc_chn_1 <= '1';
            dtack        <= '1';
          end if;
          
        when chn_2_reg_adr =>
          if Ext_Rd_active = '1' then
            rd_adc_chn_2 <= '1';
            dtack        <= '1';
          end if;
          
        when chn_3_reg_adr =>
          if Ext_Rd_active = '1' then
            rd_adc_chn_3 <= '1';
            dtack        <= '1';
          end if;
          
        when chn_4_reg_adr =>
          if Ext_Rd_active = '1' then
            rd_adc_chn_4 <= '1';
            dtack        <= '1';
          end if;
        
        when chn_5_reg_adr =>
          if Ext_Rd_active = '1' then
            rd_adc_chn_5 <= '1';
            dtack        <= '1';
          end if;
          
        when chn_6_reg_adr =>
          if Ext_Rd_active = '1' then
            rd_adc_chn_6 <= '1';
            dtack        <= '1';
          end if;
          
        when chn_7_reg_adr =>
          if Ext_Rd_active = '1' then
            rd_adc_chn_7 <= '1';
            dtack        <= '1';
          end if;
          
        when chn_8_reg_adr =>
          if Ext_Rd_active = '1' then
            rd_adc_chn_8 <= '1';
            dtack        <= '1';
          end if;
          
        when others =>
      end case;
    end if;
  end if;
end process adr_decoder;
    
user_rd_active <= rd_adc_cntrl or rd_adc_chn_1 or rd_adc_chn_2 or rd_adc_chn_3
                  or rd_adc_chn_4 or rd_adc_chn_5 or rd_adc_chn_6 or rd_adc_chn_7
                  or rd_adc_chn_8;
Data_to_SCUB <= channel_1 when rd_adc_chn_1 = '1' else
                channel_2 when rd_adc_chn_2 = '1' else
                channel_3 when rd_adc_chn_3 = '1' else
                channel_4 when rd_adc_chn_4 = '1' else
                channel_5 when rd_adc_chn_5 = '1' else
                channel_6 when rd_adc_chn_6 = '1' else
                channel_7 when rd_adc_chn_7 = '1' else
                channel_8 when rd_adc_chn_8 = '1' else
                x"0000" when rd_adc_cntrl = '1' else
                x"0000";
                
Dtack_to_SCUB <= dtack;

end architecture;
