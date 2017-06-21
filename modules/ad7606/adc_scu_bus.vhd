library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.adc_pkg.all;

entity adc_scu_bus is
  generic (
    Base_addr:            unsigned(15 downto 0);
    clk_in_hz:            integer := 50_000_000;        -- 50Mhz
    diag_on_is_1:         integer range 0 to 1 := 0;   -- if 1 then diagnosic information is generated during compilation
    fw_version:           integer range 0 to 65535 := 2);
  port (
    clk:            in std_logic;
    nrst:           in std_logic;
    tag_i:          in std_logic_vector(31 downto 0);
    tag_valid:      in std_logic;
    
    -- ADC interface
    db:             in    std_logic_vector(13 downto 0);  -- databus from the ADC
    db14_hben:      inout std_logic;                      -- hben in mode ser
    db15_byte_sel:  inout std_logic;                      -- byte sel in mode ser
    convst_a:       out   std_logic;                      -- start conversion for channels 1-4
    convst_b:       out   std_logic;                      -- start conversion for channels 5-8
    n_cs:           out   std_logic;                      -- chipselect, enables tri state databus
    n_rd_sclk:      out   std_logic;                      -- first falling edge after busy clocks data out
    busy:           in    std_logic;                      -- falling edge signals end of conversion
    adc_reset:      out   std_logic;
    os:             out   std_logic_vector(2 downto 0);   -- oversampling config
    par_ser_sel:    out   std_logic;                      -- parallel/serial/byte serial
    adc_range:      out   std_logic;                      -- logic high 10V/-10V or logic low 5V/-5V
    firstdata:      in    std_logic;                      -- is high during transmit of the frist channel
    nDiff_In_En:    out   std_logic;                      -- logic low enables diff input for chn 3-8
    ext_trg_i:      in    std_logic;                      -- trigger input
    ext_trgd_o:     out   std_logic;                      -- triggered while in ext trigger mode
    
    -- SCUB interface
    Adr_from_SCUB_LA:   in    std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
    Data_from_SCUB_LA:  in    std_logic_vector(15 downto 0);  -- latched data from SCU_Bus 
    Ext_Adr_Val:        in    std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:      in    std_logic;                      -- '1' => Rd-Cycle is active
    Ext_Wr_active:      in    std_logic;                      -- '1' => Wr-Cycle is active
    user_rd_active:     out   std_logic;                      -- '1' = read data available at 'Data_to_SCUB'-output
    Data_to_SCUB:       out   std_logic_vector(15 downto 0);  -- connect read sources to SCUB-Macro
    Dtack_to_SCUB:      out   std_logic;                      -- connect Dtack to SCUB-Macro
    
    channel_1:          out   std_logic_vector(15 downto 0);
    channel_2:          out   std_logic_vector(15 downto 0);
    channel_3:          out   std_logic_vector(15 downto 0);
    channel_4:          out   std_logic_vector(15 downto 0);
    channel_5:          out   std_logic_vector(15 downto 0);
    channel_6:          out   std_logic_vector(15 downto 0);
    channel_7:          out   std_logic_vector(15 downto 0);
    channel_8:          out   std_logic_vector(15 downto 0));
end entity;


architecture adc_scu_bus_arch of adc_scu_bus is

signal  chn_1, chn_2, chn_3, chn_4,
        chn_5, chn_6, chn_7, chn_8: std_logic_vector(15 downto 0);
        
type channel_reg_type is array(0 to 7) of std_logic_vector(15 downto 0);    -- array for 8 registers
  signal s_ext_regs:  channel_reg_type;
  
constant cntrl_reg_adr:   unsigned(15 downto 0)   := Base_addr + x"0000";
constant chn_1_reg_adr:   unsigned(15 downto 0)   := Base_addr + x"0001";
constant chn_2_reg_adr:   unsigned(15 downto 0)   := Base_addr + x"0002";
constant chn_3_reg_adr:   unsigned(15 downto 0)   := Base_addr + x"0003";
constant chn_4_reg_adr:   unsigned(15 downto 0)   := Base_addr + x"0004";
constant chn_5_reg_adr:   unsigned(15 downto 0)   := Base_addr + x"0005";
constant chn_6_reg_adr:   unsigned(15 downto 0)   := Base_addr + x"0006";
constant chn_7_reg_adr:   unsigned(15 downto 0)   := Base_addr + x"0007";
constant chn_8_reg_adr:   unsigned(15 downto 0)   := Base_addr + x"0008";
constant fw_version_adr:  unsigned(15 downto 0)   := Base_addr + x"0009";
constant tag_low_adr:     unsigned(15 downto 0)   := Base_addr + x"000a";
constant tag_high_adr:    unsigned(15 downto 0)   := Base_addr + x"000b";

signal wr_adc_cntrl:  std_logic;
signal rd_adc_cntrl:  std_logic;
signal rd_adc_chn_1:  std_logic;
signal rd_adc_chn_2:  std_logic;
signal rd_adc_chn_3:  std_logic;
signal rd_adc_chn_4:  std_logic;
signal rd_adc_chn_5:  std_logic;
signal rd_adc_chn_6:  std_logic;
signal rd_adc_chn_7:  std_logic;
signal rd_adc_chn_8:  std_logic;
signal rd_fw_version: std_logic;
signal wr_tag_low:    std_logic;
signal wr_tag_high:   std_logic;
signal rd_tag_low:    std_logic;
signal rd_tag_high:   std_logic;

signal dtack:             std_logic;
signal adc_cntrl_reg:     std_logic_vector(15 downto 0);
signal adc_cntrl_rd_reg:  std_logic_vector(15 downto 0);

signal rd_pulse1:   std_logic;
signal rd_pulse2:   std_logic;
signal copy_shadow: std_logic;

type tag_state_type is(IDLE, TAG_RECEIVED);
signal tag_state:	tag_state_type;
signal tag_trg_i: std_logic;

signal tag_low_reg:   std_logic_vector(15 downto 0);
signal tag_high_reg:  std_logic_vector(15 downto 0);
signal sync_ext_trg:  std_logic_vector(1 downto 0);
signal ext_trg_pos:   std_logic;
signal ext_trg_neg:   std_logic;
signal ext_trg:       std_logic;


begin

edge_trg: process (clk, nrst)
begin
  if rising_edge(clk) then
    sync_ext_trg(0) <= ext_trg_i;
    sync_ext_trg(1) <= sync_ext_trg(0);
  end if;
end process;

ext_trg_pos <= sync_ext_trg(0) and not sync_ext_trg(1);
ext_trg_neg <= not sync_ext_trg(0)  and sync_ext_trg(1);

sel_edge: process (clk, nrst)
begin
  if nrst = '0' then
    ext_trg <= '0';
  elsif rising_edge(clk) then
    ext_trg <= '0';
    if (adc_cntrl_reg(9) = '1' and ext_trg_pos = '1' ) or (adc_cntrl_reg(9) = '0' and ext_trg_neg = '1') then
      ext_trg <= '1';
    end if;
  end if;
end process;



tag_comp: process (clk, nrst) 
begin
  if nrst = '0' then
    tag_trg_i <= '0';
    tag_state <= IDLE;
     
  elsif rising_edge(clk) then
    tag_trg_i <= '0';
    
    case tag_state is
      when IDLE =>
        if tag_valid = '1' then
          tag_state <= TAG_RECEIVED;
        end if;
          
      when TAG_RECEIVED =>       
        if (tag_i = tag_high_reg & tag_low_reg) then
          tag_trg_i <= '1';
        end if;
        tag_state <= IDLE;
                 
      when others	=>	null;
    end case;
            
  end if;
end process tag_comp;

adc: ad7606
  generic map (
    clk_in_Hz     => clk_in_Hz,
    diag_on_is_1  => 0)
  port map (

    clk           => clk,
    nrst          => nrst,
    ext_trg_i     => ext_trg and not adc_cntrl_reg(10),
    tag_trg_i     => tag_trg_i,
    sync_rst      => adc_cntrl_reg(0),
    trigger_mode  => adc_cntrl_reg(4),
    transfer_mode => adc_cntrl_reg(2 downto 1),
    db            => db,
    db14_hben     => db14_hben,
    db15_byte_sel => db15_byte_sel,
    convst_a      => convst_a,
    convst_b      => convst_b,
    n_cs          => n_cs,
    n_rd_sclk     => n_rd_sclk,
    busy          => busy,
    adc_reset     => adc_reset,
    par_ser_sel   => par_ser_sel,
    firstdata     => firstdata,
    reg_busy      => open,
    channel_1     => chn_1,
    channel_2     => chn_2,
    channel_3     => chn_3,
    channel_4     => chn_4,
    channel_5     => chn_5,
    channel_6     => chn_6,
    channel_7     => chn_7,
    channel_8     => chn_8);
    
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
    rd_fw_version <= '0';
    wr_tag_low    <= '0';
    wr_tag_high   <= '0';
    rd_tag_low    <= '0';
    rd_tag_high   <= '0';
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
    rd_fw_version <= '0';
    wr_tag_low    <= '0';
    wr_tag_high   <= '0';
    rd_tag_low    <= '0';
    rd_tag_high   <= '0';
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
        
        when fw_version_adr =>
          if Ext_Rd_active = '1' then
            rd_fw_version  <= '1';
            dtack          <= '1'; 
          end if;
        
        when tag_low_adr =>
          if Ext_Wr_active = '1' then
            wr_tag_low <= '1';
            dtack      <= '1';
          end if;
          if Ext_Rd_active = '1' then
            rd_tag_low <= '1';
            dtack      <= '1';
          end if;
          
        when tag_high_adr =>
          if Ext_Wr_active = '1' then
            wr_tag_high <= '1';
            dtack       <= '1';
          end if;
          if Ext_Rd_active = '1' then
            rd_tag_high <= '1';
            dtack       <= '1';
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

rd_pulse: process(clk)
begin
  if rising_edge(clk) then
    rd_pulse1 <= Ext_Rd_active;
    rd_pulse2 <= rd_pulse1;
  end if;
end process;

copy_shadow <= '1' when rd_pulse1 = '1' and rd_pulse2 = '0' else '0';

shadow_2_ext: process(clk, nrst)
begin
  if nrst = '0' then
    s_ext_regs(0) <= (others => '0');
    s_ext_regs(1) <= (others => '0');
    s_ext_regs(2) <= (others => '0');
    s_ext_regs(3) <= (others => '0');
    s_ext_regs(4) <= (others => '0');
    s_ext_regs(5) <= (others => '0');
    s_ext_regs(6) <= (others => '0');
    s_ext_regs(7) <= (others => '0');
  elsif rising_edge(clk) then
    if copy_shadow = '1' then
      s_ext_regs(0) <= chn_1;
      s_ext_regs(1) <= chn_2;
      s_ext_regs(2) <= chn_3;
      s_ext_regs(3) <= chn_4;
      s_ext_regs(4) <= chn_5;
      s_ext_regs(5) <= chn_6;
      s_ext_regs(6) <= chn_7;
      s_ext_regs(7) <= chn_8;
    end if;
  end if;
end process;

tag_reg: process (clk, nrst)
begin
  if nrst = '0' then
    tag_low_reg <= (others => '0');
    tag_high_reg <= (others => '0');
  elsif rising_edge(clk) then
    if wr_tag_low = '1' then
      tag_low_reg <= Data_from_SCUB_LA;
    end if;
    if wr_tag_high = '1' then
      tag_high_reg <= Data_from_SCUB_LA;
    end if;
  end if;
end process;

-- adc_cntrl_reg(0)           : reset, 1 -> active
-- adc_cntrl_reg(2 downto 1)  : transfer mode, 00 -> par, 01 -> serial
-- adc_cntrl_reg(3)           : adc range, 1 -> -10/+10, 0 -> -5/+5
-- adc_cntrl_reg(4)           : adc trigger mode: 0 -> continuous, 1 -> triggered by tag or ext input
-- adc_cntrl_reg(7 downto 5)  : oversample config: 000 -> no OS, 110 -> ratio 64
-- adc_cntrl_reg(8)           : enables differential input for channels 3 to 8
-- adc_cntrl_reg(9)           : ext trigger edge: 1 -> pos edge, 0 -> neg edge
-- adc_cntrl_reg(10)          : mask out ext trigger input, for tag only trigger 
cntrl_reg: process (clk, nrst, rd_adc_cntrl, wr_adc_cntrl)
  variable reset_cnt: unsigned(1 downto 0) := "00";
begin
  if nrst = '0' then
    adc_cntrl_reg <= x"0008";
    reset_cnt := "00";
  elsif rising_edge(clk) then
    if wr_adc_cntrl = '1' then
      adc_cntrl_reg <= Data_from_SCUB_LA;
    elsif  adc_cntrl_reg(0) = '1' then
      if reset_cnt < 3 then
        reset_cnt := reset_cnt + 1;
      else
        adc_cntrl_reg(0) <= '0';
        reset_cnt := "00";
      end if;
    end if;
  end if;
end process;

adc_cntrl_rd_reg <=  adc_cntrl_reg;

    
user_rd_active <= rd_adc_cntrl or rd_adc_chn_1 or rd_adc_chn_2 or rd_adc_chn_3
                  or rd_adc_chn_4 or rd_adc_chn_5 or rd_adc_chn_6 or rd_adc_chn_7
                  or rd_adc_chn_8 or rd_fw_version or rd_tag_high or rd_tag_low;

Data_to_SCUB <= s_ext_regs(0) when rd_adc_chn_1 = '1' else
                s_ext_regs(1) when rd_adc_chn_2 = '1' else
                s_ext_regs(2) when rd_adc_chn_3 = '1' else
                s_ext_regs(3) when rd_adc_chn_4 = '1' else
                s_ext_regs(4) when rd_adc_chn_5 = '1' else
                s_ext_regs(5) when rd_adc_chn_6 = '1' else
                s_ext_regs(6) when rd_adc_chn_7 = '1' else
                s_ext_regs(7) when rd_adc_chn_8 = '1' else
                adc_cntrl_rd_reg when rd_adc_cntrl = '1' else
                std_logic_vector(to_unsigned(fw_version, 16)) when rd_fw_version = '1' else
                tag_low_reg when rd_tag_low = '1' else
                tag_high_reg when rd_tag_high = '1' else
                x"0000";
                
Dtack_to_SCUB <= dtack;

channel_1 <= chn_1;
channel_2 <= chn_2;
channel_3 <= chn_3;
channel_4 <= chn_4;
channel_5 <= chn_5;
channel_6 <= chn_6;
channel_7 <= chn_7;
channel_8 <= chn_8;

adc_range   <= adc_cntrl_reg(3);
os          <= adc_cntrl_reg(7 downto 5);
nDiff_In_En <= adc_cntrl_reg(8);
ext_trgd_o  <= ext_trg and adc_cntrl_reg(4) and not adc_cntrl_reg(10); -- signal trigger while in ext trigger mode and not masked

end architecture;
