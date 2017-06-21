library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;


entity ad7606  is
  generic (
        clk_in_hz:          integer := 50_000_000;      -- 50Mhz
        sclk_in_hz:         integer := 14_500_000;      -- 14,5Mhz
        cs_delay_in_ns:     integer := 16;              -- 16ns
        cs_high_in_ns:      integer := 22;              -- 22ns
        rd_low_in_ns:       integer := 16;              -- 16ns
        reset_delay_in_ns:  integer := 50;              -- 50ns
        conv_wait_in_ns:    integer := 25;              -- 25ns
        inter_cycle_in_ns:  integer := 6000;            -- 6us
        diag_on_is_1:       integer range 0 to 1 := 1   -- if 1 then diagnosic information is generated during compilation
      );
  port (
      clk:            in std_logic;
      nrst:           in std_logic;
      sync_rst:       in std_logic;
      ext_trg_i:      in std_logic;
      tag_trg_i:      in std_logic;
      trigger_mode:   in std_logic;                         -- 0: continuous conversion 1: triggered by extern trig input
      transfer_mode:  in std_logic_vector(1 downto 0);      -- select communication mode
                                                            --  00: par
                                                            --  01: ser
                                                  
      db:             in std_logic_vector(13 downto 0);     -- databus from the ADC
      db14_hben:      inout std_logic;                      -- hben in mode ser
      db15_byte_sel:  inout std_logic;                      -- byte sel in mode ser
      convst_a:       out std_logic;                        -- start conversion for channels 1-4
      convst_b:       out std_logic;                        -- start conversion for channels 5-8
      n_cs:           out std_logic;                        -- chipselect, enables tri state databus
      n_rd_sclk:      out std_logic;                        -- first falling edge after busy clocks data out
      busy:           in std_logic;                         -- falling edge signals end of conversion
      adc_reset:      out std_logic;
      par_ser_sel:    out std_logic;                        -- parallel/serial/byte serial
      firstdata:      in std_logic;
      reg_busy:       out std_logic;                        -- active when adc data is written to register block
      channel_1:      out std_logic_vector(15 downto 0);
      channel_2:      out std_logic_vector(15 downto 0);
      channel_3:      out std_logic_vector(15 downto 0);
      channel_4:      out std_logic_vector(15 downto 0);
      channel_5:      out std_logic_vector(15 downto 0);
      channel_6:      out std_logic_vector(15 downto 0);
      channel_7:      out std_logic_vector(15 downto 0);
      channel_8:      out std_logic_vector(15 downto 0)
    );
end entity;

architecture ad7606_arch of ad7606 is
  type channel_reg_type is array(0 to 7) of std_logic_vector(15 downto 0);    -- array for 8 registers
  signal s_shadow_regs:  channel_reg_type;
  type state_type is (reset, idle,  conv_trigger, conv_st, wait_for_trigger, wait_for_busy, wait_for_conv_fin, read_par,
            read_ser, data_ready, wait_rd_low, wait_cs_high);
  
  signal conv_state:    state_type;
  signal s_convst_a:    std_logic;
  signal s_convst_b:    std_logic;
  signal s_n_cs:        std_logic;
  signal s_n_rd:        std_logic;
  signal s_adc_reset:   std_logic;
  signal s_os:          std_logic_vector(2 downto 0);
  signal s_par_ser_sel: std_logic;
  signal s_channel_cnt: integer range 0 to 8;
  signal s_sclk:        std_logic;
  signal s_sclk_en:     std_logic;
  alias s_ser_data_a:   std_logic is db(7);
  alias s_ser_data_b:   std_logic is db(8);
  signal s_hben:        std_logic;
  signal s_byte_sel:    std_logic;
  signal s_db14:        std_logic;
  signal s_db15:        std_logic;
  signal s_db_in:       std_logic_vector(15 downto 0);
  
  signal s_bit_count:   integer range 0 to 128;
  signal s_shift_reg_a: std_logic_vector(63 downto 0);
  signal s_shift_reg_b: std_logic_vector(63 downto 0);
  
  signal s_channel_latch: std_logic;
  signal sync_busy1:      std_logic;
  signal sync_busy2:      std_logic;
  signal s_busy:          std_logic;
 
  
  
  constant c_wait_d_ready:        integer := integer(ceil(real(clk_in_hz) / real(1_000_000_000) * real(cs_delay_in_ns)));
  constant c_wait_d_ready_width:  integer := integer(floor(log2(real(c_wait_d_ready))));
  signal  s_wait_d_ready:         unsigned(c_wait_d_ready_width downto 0) := (others => '0') ;

  constant  c_wait_cs_high:       integer := integer(ceil(real(clk_in_hz) / real(1_000_000_000) * real(cs_high_in_ns)));
  constant  c_wait_cs_high_width: integer := integer(floor(log2(real(c_wait_cs_high))));
  signal    s_wait_cs_high:       unsigned(c_wait_cs_high_width downto 0) := (others => '0');
  
  constant c_wait_rd_low:         integer := integer(ceil(real(clk_in_hz) / real(1_000_000_000) * real(rd_low_in_ns)));
  constant c_wait_rd_low_width:   integer := integer(floor(log2(real(c_wait_rd_low))));
  signal  s_wait_rd_low:          unsigned( c_wait_rd_low_width downto 0) := (others => '0');
  
  constant c_reset_delay:         integer := integer(ceil(real(clk_in_hz) / real(1_000_000_000) * real(reset_delay_in_ns)));
  constant c_reset_delay_width:   integer := integer(floor(log2(real(c_reset_delay))));
  signal  s_reset_delay:          unsigned( c_reset_delay_width downto 0) := (others => '0');
  
  constant c_conv_wait:           integer := integer(ceil(real(clk_in_hz) / real(1_000_000_000) * real(conv_wait_in_ns)));
  constant c_conv_wait_width:     integer := integer(floor(log2(real(c_conv_wait))));
  signal  s_conv_wait:            unsigned(c_conv_wait_width downto 0) := (others => '0');
  
  constant c_inter_cycle:         integer := integer(ceil(real(clk_in_hz) / real(1_000_000_000) * real(inter_cycle_in_ns)));
  constant c_inter_cycle_width:   integer := integer(floor(log2(real(c_inter_cycle))));
  signal  s_inter_cycle:          unsigned(c_inter_cycle_width downto 0) := (others => '0');
  
  constant c_sclk_cnt:            integer := integer(ceil((real(clk_in_hz) / real(sclk_in_hz)) / real(2)));
  constant c_sclk_cnt_width:      integer := integer(floor(log2(real(c_sclk_cnt))));
  signal s_sclk_cnt:              unsigned(c_sclk_cnt_width downto 0) := (others => '0');
    
  constant c_ser_length:          integer := 64 * 2;  


begin

  assert not (diag_on_is_1 = 1)
    report " c_wait_d_ready: " & integer'image(c_wait_d_ready)
        &  ",   s_wait_d_ready length in bit: " & integer'image(s_wait_d_ready'length)
    severity note;
  assert not (diag_on_is_1 = 1)
    report " c_wait_cs_high: " & integer'image(c_wait_cs_high)
        &  ",   s_wait_cs_high length in bit: " & integer'image(s_wait_cs_high'length)
  severity note;
  assert not (diag_on_is_1 = 1)
    report " c_wait_rd_low: " & integer'image(c_wait_rd_low)
        &  ",    s_wait_rd_low length in bit: " & integer'image(s_wait_rd_low'length)
  severity note;
  assert not (diag_on_is_1 = 1)
    report " c_reset_delay: " & integer'image(c_reset_delay)
        &  ",    s_reset_delay length in bit: " & integer'image(s_reset_delay'length)
  severity note;
  assert not (diag_on_is_1 = 1)
    report " c_conv_wait: " & integer'image(c_conv_wait)
        &  ",    s_conv_wait length in bit: " & integer'image(s_conv_wait'length)
  severity note;
  assert not (diag_on_is_1 = 1)
    report " c_inter_cycle: " & integer'image(c_inter_cycle)
        &  ",    s_inter_cycle length in bit: " & integer'image(s_inter_cycle'length)
  severity note;
  assert not (diag_on_is_1 = 1)
    report " c_sclk_cnt: " & integer'image(c_sclk_cnt)
        &  ",    s_sclk_cnt length in bit: " & integer'image(s_sclk_cnt'length)
  severity note;


  -- tri stating the hben and byte_sel lines
  s_db14        <= db14_hben;
  s_db15        <= db15_byte_sel;
  db14_hben     <= s_hben when s_par_ser_sel = '1' else 'Z';
  db15_byte_sel <= s_byte_sel when s_par_ser_sel = '1' else 'Z';
  
  s_db_in       <= s_db15 & s_db14 & db;
  
  serial_clk_en: process(clk, nrst, s_par_ser_sel)
  begin
    if nrst = '0' then
      s_sclk_cnt <= (others=> '0');
    elsif rising_edge(clk) then
      if s_sclk_cnt = to_unsigned(c_sclk_cnt-1, s_sclk_cnt'length) then
        s_sclk_en   <= '1';
        s_sclk_cnt  <= (others=> '0');
      else
        s_sclk_en   <= '0';
        s_sclk_cnt  <= s_sclk_cnt + 1;
      end if;
    end if;
  end process;
  
  serial_clk: process(clk, nrst, s_sclk_en)
  begin
    if nrst = '0' then
      s_sclk <= '1';
    elsif rising_edge(clk) then
      if s_sclk_en = '1' then
        s_sclk <= not s_sclk;
      end if;
    end if;
  end process;
  
  
  -- shift registers for serial data a and b
  -- adc clocks out data after first rising edge of sclk
  -- data is valid after first falling edge
  
  shift_reg: process(clk, s_sclk, nrst, s_par_ser_sel)
  begin
    if nrst = '0' then
      s_bit_count <= 0;
    elsif rising_edge(clk) then
      
      if conv_state = read_ser then
        if s_sclk_en = '1'  and s_bit_count < (c_ser_length ) then
          if (s_bit_count mod 2) = 0 then
            s_shift_reg_a <= s_shift_reg_a(s_shift_reg_a'high-1 downto 0) & s_ser_data_a;
            s_shift_reg_b <= s_shift_reg_b(s_shift_reg_b'high-1 downto 0) & s_ser_data_b;
          end if;
          s_bit_count <= s_bit_count + 1; 
        elsif s_bit_count = (c_ser_length ) then
          s_shadow_regs(0) <= s_shift_reg_a(63 downto 48);
          s_shadow_regs(1) <= s_shift_reg_a(47 downto 32);
          s_shadow_regs(2) <= s_shift_reg_a(31 downto 16);
          s_shadow_regs(3) <= s_shift_reg_a(15 downto 0);
        
          s_shadow_regs(4) <= s_shift_reg_b(63 downto 48);
          s_shadow_regs(5) <= s_shift_reg_b(47 downto 32);
          s_shadow_regs(6) <= s_shift_reg_b(31 downto 16);
          s_shadow_regs(7) <= s_shift_reg_b(15 downto 0);
          
          s_bit_count <= 0;
          
        end if;
      elsif s_channel_latch = '1' then
        s_shadow_regs(s_channel_cnt-1) <= s_db_in;
      end if;
    end if;
  
  end process;
  
  sync_busy: process (clk, busy)
  begin
    if rising_edge(clk) then
      sync_busy1 <= busy;
      sync_busy2 <= sync_busy1;
    end if;
  end process;
  
conv_cycle: process(clk, nrst, sync_rst)
  begin
  
    if nrst = '0' or sync_rst = '1' then
      conv_state      <= reset;
      s_convst_a      <= '1';
      s_convst_b      <= '1';
      s_n_cs          <= '1';
      s_n_rd          <= '1';
      s_adc_reset     <= '0';
      s_reset_delay   <= (others => '0');
      s_conv_wait     <= (others => '0');
      s_wait_d_ready  <= (others => '0');
      s_wait_rd_low   <= (others => '0');
      s_wait_cs_high  <= (others => '0');
      s_inter_cycle   <= (others => '0');
      
      s_channel_cnt   <= 0;
      s_par_ser_sel   <= '0';
      s_hben          <= '0';
      s_byte_sel      <= '0';
      s_channel_latch <= '0';
      s_busy          <= '0';
      
      
    elsif rising_edge(clk) then
    
      s_convst_a      <= '1';
      s_convst_b      <= '1';
      s_n_cs          <= '1';
      s_n_rd          <= '1';
      s_adc_reset     <= '0';
      s_hben          <= '0';
      s_byte_sel      <= '0';
      s_channel_latch <= '0';
      s_busy          <= '0';
      
      case conv_state is
        when reset =>
          s_adc_reset   <= '1';
          if s_reset_delay = to_unsigned(c_reset_delay, s_reset_delay'length) then
            conv_state <= idle;
            s_reset_delay <= (others => '0');
          else
            s_reset_delay <= s_reset_delay  + 1;
          end if;
          
        when idle =>
          if s_inter_cycle = to_unsigned(c_inter_cycle, s_inter_cycle'length) then
            conv_state <= wait_for_trigger;
            s_inter_cycle <= (others => '0');
          else
            s_inter_cycle <= s_inter_cycle + 1;
          end if;
        
        when wait_for_trigger =>
          if trigger_mode = '0' then
            conv_state <= conv_st;
          else
            if ext_trg_i = '1' then
              conv_state <= conv_st;
            end if;
            if tag_trg_i = '1' then
              conv_state <= conv_st;
            end if;
          end if;
          
        when conv_st =>
          s_convst_a <= '0';
          s_convst_b <= '0';
          if s_conv_wait = to_unsigned(c_conv_wait, s_conv_wait'length) then
            conv_state  <= wait_for_busy;
            s_conv_wait <= (others => '0');
          else
            s_conv_wait <= s_conv_wait + 1;
          end if;
        
        when wait_for_busy =>
          if sync_busy2 = '1' then
            conv_state <= wait_for_conv_fin;
          end if;
        
        when wait_for_conv_fin =>
          
          if sync_busy2 = '0' then
            if transfer_mode = "00" then
              conv_state <= read_par;
            elsif transfer_mode = "01" then
              conv_state <= read_ser;
            end if;
          end if;
        when read_ser =>
          s_par_ser_sel <= '1';
          s_n_cs        <= '0';
          s_n_rd        <= s_sclk;
          
          if s_bit_count = (c_ser_length ) then
            conv_state <= idle;
          end if;
          
        
        when read_par =>
          s_busy        <= '1';
          s_par_ser_sel <= '0';
          -- wait for c_wait_d_ready until data is stable
    
          if s_wait_d_ready = to_unsigned(c_wait_d_ready, s_wait_d_ready'length) then
            conv_state <= data_ready;
            s_wait_d_ready <= (others=> '0');
          else
            s_wait_d_ready <= s_wait_d_ready + 1;
          end if;
          
        when data_ready =>
          s_busy        <= '1';
          s_par_ser_sel <= '0';
          -- now data should be stable
          
          
          if s_channel_cnt < 8 then
            s_n_cs <= '0';
            s_n_rd <= '0';
            
            s_channel_cnt <= s_channel_cnt + 1;
            conv_state <= wait_rd_low;
          else
            s_channel_cnt <= 0;
            conv_state    <= idle;
          end if;
          
            
        when wait_rd_low =>
          s_busy        <= '1';
          s_par_ser_sel <= '0';
          -- hold rd low for c_wait_rd_low
          s_n_cs        <= '0';
          s_n_rd        <= '0';
          
          
          if s_wait_rd_low = to_unsigned(c_wait_rd_low, s_wait_rd_low'length) then
            conv_state      <= wait_cs_high;
            s_wait_rd_low   <= (others=> '0');
            s_channel_latch <= '1';
          else
            s_wait_rd_low <= s_wait_rd_low + 1;
          end if;
        
        when wait_cs_high =>
          s_busy        <= '1';
          s_par_ser_sel <= '0';
          
          -- rd and cs goes high for c_wait_cs_high
          if s_wait_cs_high = to_unsigned(c_wait_cs_high, s_wait_cs_high'length) then
            conv_state      <= data_ready;
            s_wait_cs_high  <= (others=> '0');
          else
            s_wait_cs_high  <= s_wait_cs_high + 1;
          end if;
  
        when others =>
          conv_state <= reset;
        
      end case;
    end if;
  end process;
  
  
  -- ADC signals
  n_cs        <= s_n_cs;
  n_rd_sclk   <= s_n_rd;
  par_ser_sel <= s_par_ser_sel;
  convst_a    <= s_convst_a;
  convst_b    <= s_convst_b;
  adc_reset   <= s_adc_reset;
  
  -- channel order has to be switched for SCU_ADDA
  channel_1 <= s_shadow_regs(7);
  channel_2 <= s_shadow_regs(6);
  channel_3 <= s_shadow_regs(5);
  channel_4 <= s_shadow_regs(4);
  channel_5 <= s_shadow_regs(3);
  channel_6 <= s_shadow_regs(2);
  channel_7 <= s_shadow_regs(1);
  channel_8 <= s_shadow_regs(0);
  
  reg_busy  <= s_busy;
end architecture;
