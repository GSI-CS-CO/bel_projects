library ieee;
use ieee.std_logic_1164.all;
use ieee.math_real.all;
use ieee.numeric_std.all;

library work;
use work.fg2_pkg.all;

entity fg_scubus is
  generic (
    Base_addr:            unsigned(15 downto 0);
    clk_in_hz:            integer := 50_000_000);
  port (
    -- SCUB interface
    Adr_from_SCUB_LA:   in    std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
    Data_from_SCUB_LA:  in    std_logic_vector(15 downto 0);  -- latched data from SCU_Bus 
    Ext_Adr_Val:        in    std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:      in    std_logic;                      -- '1' => Rd-Cycle is active
    Ext_Wr_active:      in    std_logic;                      -- '1' => Wr-Cycle is active
    user_rd_active:     out   std_logic;                      -- '1' = read data available at 'Data_to_SCUB'-output
    clk:                in    std_logic;                      -- should be the same clk, used by SCU_Bus_Slave
    nReset:             in    std_logic;
    tag:                in    std_logic_vector(31 downto 0);  -- 32Bit tag from timing 
    tag_valid:          in    std_logic;                      -- tag valid
    ext_trigger:        in    std_logic;                      -- external trigger for ramp start
    Rd_Port:            out   std_logic_vector(15 downto 0);  -- output for all read sources of this macro
    Dtack:              out   std_logic;                      -- connect Dtack to SCUB-Macro
    -- fg_quad
    irq:                out   std_logic;
      
    sw_out:             out   std_logic_vector(31 downto 0);  -- function generator output
    sw_strobe:          out   std_logic
    );
end entity;


architecture rtl of fg_scubus is

  constant cntrl_adr:         unsigned(15 downto 0) := Base_addr + x"0000";
  constant a_0_adr:           unsigned(15 downto 0) := Base_addr + x"0001";
  constant a_1_adr:           unsigned(15 downto 0) := Base_addr + x"0002";
  constant a_2_adr:           unsigned(15 downto 0) := Base_addr + x"0003";
  constant a_3_adr:           unsigned(15 downto 0) := Base_addr + x"0004";
  
  constant b_0_adr:           unsigned(15 downto 0) := Base_addr + x"0005";
  constant b_1_adr:           unsigned(15 downto 0) := Base_addr + x"0006";
  constant b_2_adr:           unsigned(15 downto 0) := Base_addr + x"0007";
  constant b_3_adr:           unsigned(15 downto 0) := Base_addr + x"0008";

  constant c_0_adr:           unsigned(15 downto 0) := Base_addr + x"0009";
  constant c_1_adr:           unsigned(15 downto 0) := Base_addr + x"000a";
  constant c_2_adr:           unsigned(15 downto 0) := Base_addr + x"000b";
  constant c_3_adr:           unsigned(15 downto 0) := Base_addr + x"000c";

  constant d_0_adr:           unsigned(15 downto 0) := Base_addr + x"000d";
  constant d_1_adr:           unsigned(15 downto 0) := Base_addr + x"000e";
  constant d_2_adr:           unsigned(15 downto 0) := Base_addr + x"000f";
  constant d_3_adr:           unsigned(15 downto 0) := Base_addr + x"0010";

  constant shift_adr:         unsigned(15 downto 0) := Base_addr + x"0011";
  constant n_adr:             unsigned(15 downto 0) := Base_addr + x"0012";
  constant rampcnt_0_adr:     unsigned(15 downto 0) := Base_addr + x"0013";
  constant rampcnt_1_adr:     unsigned(15 downto 0) := Base_addr + x"0014";
  constant tag_0_adr:         unsigned(15 downto 0) := Base_addr + x"0015";
  constant tag_1_adr:         unsigned(15 downto 0) := Base_addr + x"0016";

  
  signal  fg_cntrl_reg:     std_logic_vector(15 downto 0);
  signal  fg_cntrl_rd_reg:  std_logic_vector(15 downto 0);
  signal  ramp_cnt_reg:     unsigned(31 downto 0);
  signal  ramp_cnt_shadow:  unsigned(31 downto 0);
  signal  tag_low_reg:      std_logic_vector(15 downto 0);
  signal  tag_high_reg:     std_logic_vector(15 downto 0);

  signal  a_o:              std_logic_vector(63 downto 0);
  signal  b_o:              std_logic_vector(63 downto 0);
  signal  c_o:              std_logic_vector(63 downto 0);
  signal  d_o:              std_logic_vector(63 downto 0);
  signal  n_o:              std_logic_vector(15 downto 0);
  signal  a_we_i:           std_logic_vector(7 downto 0);
  signal  b_we_i:           std_logic_vector(7 downto 0);
  signal  c_we_i:           std_logic_vector(7 downto 0);
  signal  d_we_i:           std_logic_vector(7 downto 0);
  signal  s_we_i:           std_logic_vector(1 downto 0);
  signal  n_we_i:           std_logic_vector(1 downto 0);
  signal  a_i:              std_logic_vector(63 downto 0);
  signal  b_i:              std_logic_vector(63 downto 0);
  signal  c_i:              std_logic_vector(63 downto 0);
  signal  d_i:              std_logic_vector(63 downto 0);
  signal  s_i:              std_logic_vector(63 downto 0);
  signal  n_i:              std_logic_vector(63 downto 0);
  
  signal  fg_is_running:    std_logic;
  signal  tag_start:        std_logic;
  signal  ramp_sec_fin:     std_logic;

  type tag_state_type is(IDLE, TAG_RECEIVED);
	signal tag_state	:	tag_state_type;

begin
  fg_accu: fg_accumulator 
    port map (
      clk_i     => clk,
      rstn_i    => nReset,
      dac_stb_i => '0', -- enable from clk divider
      dac_val_o => sw_out,
      start_i   => tag_start, -- 0=>1 means accumulator begins
      running_o => fg_is_running,
      irq_o     => irq, -- 0=>1 means refill
      full_o    => open, -- writes to abcdsn should fail
      stall_o   => open, -- writes are ignored
      a_o       => a_o,
      b_o       => b_o,
      c_o       => c_o,
      d_o       => d_o,
      n_o       => n_o,
      a_we_i    => a_we_i,
      b_we_i    => b_we_i,
      c_we_i    => c_we_i,
      d_we_i    => d_we_i,
      s_we_i    => s_we_i,
      n_we_i    => n_we_i, -- low byte (0) triggers readiness
      a_i       => a_i,
      b_i       => b_i,
      c_i       => c_i,
      d_i       => d_i,
      s_i       => s_i, -- 0, d, c, b
      n_i       => n_i);
    
  
tag_comp: process (clk, nReset) 
begin
  if nReset = '0' then
    tag_start <= '0';
    tag_state <= IDLE;
     
  elsif rising_edge(clk) then
    tag_start <= '0';
    
    case tag_state is
      when IDLE =>
        if tag_valid = '1' then
          tag_state <= TAG_RECEIVED;
        end if;
          
      when TAG_RECEIVED =>       
        if (tag = tag_high_reg & tag_low_reg) then
          tag_start <= '1';
        end if;
        tag_state <= IDLE;
                 
      when others	=>	null;
    end case;
            
  end if;
end process tag_comp;

-- fg_cntrl_reg(0)            : reset, 1 -> active 
-- fg_cntrl_reg(1)            : 1 -> fg enabled, 0 -> fg disabled
-- fg_cntrl_reg(2)            : 1 -> running, 0 -> stopped (ro)
-- fg_cntrl_reg(3)            : not used
-- fg_cntrl_reg(9 downto 4)   : virtual fg number (rw)
-- fg_cntrl_reg(12 downto 10) : not used
-- fg_cntrl_reg(15 downto 13) : add frequency select (wo)
cntrl_reg: process (clk, nReset, fg_cntrl_reg)
  variable reset_cnt: unsigned(1 downto 0) := "00";
begin
  if nReset = '0' then
    fg_cntrl_reg    <= (others => '0');
    ramp_cnt_reg    <= (others => '0');
    tag_low_reg     <= x"babe";
    tag_high_reg    <= x"feed";
    reset_cnt := "00";
  elsif rising_edge(clk) then
    if fg_cntrl_reg(0) = '1' then
      fg_cntrl_reg    <= (others => '0');
      ramp_cnt_reg    <= (others => '0');
      tag_low_reg     <= x"babe";
      tag_high_reg    <= x"feed";
      reset_cnt := "00";
    else
      
      if  Ext_Adr_Val = '1' and Ext_Wr_active = '1' then
        case unsigned(Adr_from_SCUB_LA) is
          when cntrl_adr =>
            fg_cntrl_reg <= Data_from_SCUB_LA; 
 
          when a_0_adr =>
            a_i(15 downto 0)  <= Data_from_SCUB_LA;
            a_we_i            <= x"03";
            dtack             <= '1';
          when a_1_adr =>
            a_i(31 downto 16) <= Data_from_SCUB_LA;
            a_we_i            <= x"0c";
            dtack             <= '1';
          when a_2_adr =>
            a_i(47 downto 32) <= Data_from_SCUB_LA;
            a_we_i            <= x"30";
            dtack             <= '1';
          when a_3_adr =>
            a_i(63 downto 48) <= Data_from_SCUB_LA;
            a_we_i            <= x"c0";
            dtack             <= '1';
      
          when b_0_adr =>
            b_i(15 downto 0)  <= Data_from_SCUB_LA;
            b_we_i            <= x"03";
            dtack             <= '1';
          when b_1_adr =>
            b_i(31 downto 16) <= Data_from_SCUB_LA;
            b_we_i            <= x"c0";
            dtack             <= '1';
          when b_2_adr =>
            b_i(47 downto 32) <= Data_from_SCUB_LA;
            b_we_i            <= x"30";
            dtack             <= '1';
          when b_3_adr =>
            b_i(63 downto 48) <= Data_from_SCUB_LA;
            b_we_i            <= x"c0";
            dtack             <= '1';
          
          when c_0_adr =>
            c_i(15 downto 0)  <= Data_from_SCUB_LA;
            c_we_i            <= x"03";
            dtack             <= '1';
          when c_1_adr =>
            c_i(31 downto 16) <= Data_from_SCUB_LA;
            c_we_i            <= x"c0";
            dtack             <= '1';
          when c_2_adr =>
            c_i(47 downto 32) <= Data_from_SCUB_LA;
            c_we_i            <= x"03";
            dtack             <= '1';
          when c_3_adr =>
            c_i(63 downto 48) <= Data_from_SCUB_LA;
            c_we_i            <= x"c0";
            dtack             <= '1';
          
          when d_0_adr =>
            d_i(15 downto 0)  <= Data_from_SCUB_LA;
            d_we_i            <= x"03";
            dtack             <= '1';
          when d_1_adr =>
            d_i(31 downto 16) <= Data_from_SCUB_LA;
            d_we_i            <= x"0c";
            dtack             <= '1';
          when d_2_adr =>
            d_i(47 downto 32) <= Data_from_SCUB_LA;
            d_we_i            <= x"30";
            dtack             <= '1';
          when d_3_adr =>
            d_i(63 downto 48) <= Data_from_SCUB_LA;
            d_we_i            <= x"c0";
            dtack             <= '1';
          
          when shift_adr =>
            s_i     <= Data_from_SCUB_LA;
            s_we_i  <= x"3";
            dtack   <= '1';

          when n_adr =>
            n_i     <= Data_from_SCUB_LA;
            n_we_i  <= x"3";
            dtack   <= '1';

          when tag_0_adr =>
            tag_low_reg <= Data_from_SCUB_LA;
            dtack <= '1';

          when tag_1_adr =>
            tag_high_reg <= Data_from_SCUB_LA;
            dtack <= '1';
        
        end case;
      end if;
        
      if  fg_cntrl_reg(0) = '1' then
        if reset_cnt < 3 then
          reset_cnt := reset_cnt + 1;
        else
          fg_cntrl_reg(0) <= '0';
          reset_cnt := "00";
        end if;
      end if;
    
      if ramp_sec_fin = '1' and fg_is_running = '1' then -- increment with every finished ramp section
        ramp_cnt_reg <= ramp_cnt_reg + 1;
      end if;
    
      if tag_start = '1' and fg_cntrl_reg(1) = '1' then -- disable after Started. Prevents unintended triggering by the next tag.
        fg_cntrl_reg(1) <= '0';
      end if;
      
      if rd_ramp_cnt_lo = '1' then -- save counter to shadow register
        ramp_cnt_shadow <= ramp_cnt_reg;
      end if;
      
    end if;
    
  end if;
end process;

fg_cntrl_rd_reg <= fg_cntrl_reg(15 downto 13) & '0' & '0' & '0' &
                    fg_cntrl_reg(9 downto 4) & '0' & fg_is_running & fg_cntrl_reg(1 downto 0);

user_rd_active <= '1' when Ext_Adr_Val = '1' and Ext_Rd_active = '1' and
            (Adr_from_SCUB_LA > Base_addr) and (Adr_from_SCUB_LA < Base_addr + tag_1_adr)
            else '0';


dtack <= '1' when Ext_Adr_Val = '1' and (Ext_Rd_active = '1' or Ext_Wr_active = '1') and 
            (Adr_from_SCUB_LA > Base_addr) and (Adr_from_SCUB_LA < Base_addr + tag_1_adr)
            else '0';

with Adr_from_SCUB_LA select Rd_Port <=
  fg_cntrl_rd_reg   when cntrl_reg_adr,
  a_o(15 downto 0)  when a_0_adr,
  a_o(31 downto 16) when a_1_adr,
  a_o(47 downto 32) when a_2_adr,
  a_o(63 downto 48) when a_3_adr,
  
  b_o(15 downto 0)  when b_0_adr,
  b_o(31 downto 16) when b_1_adr,
  b_o(47 downto 32) when b_2_adr,
  b_o(63 downto 48) when b_3_adr,
  
  c_o(15 downto 0)  when c_0_adr,
  c_o(31 downto 16) when c_1_adr,
  c_o(47 downto 32) when c_2_adr,
  c_o(63 downto 48) when c_3_adr,
  
  d_o(15 downto 0)  when d_0_adr,
  d_o(31 downto 16) when d_1_adr,
  d_o(47 downto 32) when d_2_adr,
  d_o(63 downto 48) when d_3_adr,

  n_o               when n_adr,
  tag_low_reg       when tag_0_adr,
  tag_high_reg      when tag_1_adr,
  std_logic_vector(ramp_cnt_shadow(31 downto 16)) when rampcnt_1_adr,
  std_logic_vector(ramp_cnt_shadow(15 downto 0)) when rampcnt_0_adr;


end architecture;
