library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.adc_pkg.all;

entity adc_scu_bus is
  generic (
    Base_addr:            unsigned(15 downto 0);
    clk_in_hz:            integer := 50_000_000;        -- 50Mhz
    sclk_in_hz:           integer := 14_500_000;        -- 14,5Mhz
    cs_delay_in_ns:       integer := 16;                -- 16ns
    cs_high_in_ns:        integer := 22;                -- 22ns
    rd_low_in_ns:         integer := 16;                -- 16ns
    reset_delay_in_ns:    integer := 50;                -- 50ns
    conv_wait_in_ns:      integer := 25;                -- 25ns
    inter_cycle_in_ns:    integer := 6000;              -- 6us
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

signal channel_1, channel_2, channel_3, channel_4, channel_5, channel_6, channel_7, channel_8: std_logic_vector(15 downto 0);

begin

adc: ad7606
  generic map (
    clk_in_Hz     => clk_in_Hz,
    diag_on_is_1  => 0)
  port map (
    clk           =>  clk,
    nrst          =>  nrst,
    conv_en       => '1',
    transfer_mode => "01",
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
    
user_rd_active <= '0';
Data_to_SCUB <= x"0000";
Dtack_to_SCUB <= '0';

end architecture;
