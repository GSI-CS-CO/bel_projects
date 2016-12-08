library ieee;
use ieee.numeric_std.all;
use ieee.std_logic_1164.all;
use ieee.math_real.all;

use work.ifa8_pkg.all;

entity sweep is
  generic (
    clk_in_hz:  integer := 120_000_000;
    version:    integer := 5);
  port (
    clk:        std_logic;
    pu_reset:   std_logic;
    d:          std_logic_vector(15 downto 0);
    hw_trig:    std_logic;
    fkt:        std_logic_vector(7 downto 0);
    take_da:    std_logic;
    sweep_sel:  std_logic;
    
    sweep_out:  out std_logic_vector(15 downto 0);
    sweep_stat: out std_logic_vector(15 downto 0);
    stat_sel:   out std_logic;
    sweep_vers: out std_logic_vector(3 downto 0)
  );
end entity;

architecture arch of sweep is
  
  constant sr_w:        integer := 6;
  constant dec_w:       integer := 12;
  constant sw_w:        integer := 21;
  constant rund_w:      integer := dec_w + sr_w;
  constant c_delta_val: unsigned := x"5f3";
  
  constant c_12Mhz_cnt:         integer := clk_in_hz / 12000000 - 2;
  constant c_12Mhz_cnt_width:   integer := integer(ceil(log2(real(c_12Mhz_cnt)))) + 1;
  
  signal s_freq_cnt:    unsigned(c_12Mhz_cnt_width - 1 downto 0);
  signal s_freq_en:     std_logic;
  
  -- signals from decoder
  signal ld_delta:        std_logic;
  signal ld_delay:        std_logic;
  signal ld_flattop_int:  std_logic;
  signal set_flattop:     std_logic;
  signal ena_soft_trig:   std_logic;
  signal stop_fkt:        std_logic;
  signal reset:           std_logic;
  signal sw_trig:         std_logic;
  
  -- signals from control
  signal wr_delta:        std_logic;
  signal s_stop_delta:    std_logic;
  signal wr_ft_int:       std_logic;
  signal wr_flattop:      std_logic;
  signal idle:            std_logic;
  signal wait_start:      std_logic;
  signal work:            std_logic;
  signal stop:            std_logic;
  signal stop_exec:       std_logic;
  signal to_err:          std_logic;
  signal seq_err:         std_logic;
  signal trigger:         std_logic;
  signal init_hw:         std_logic;


  -- signals from datapath
  signal flattop:         unsigned(sw_w-1 downto sw_w-16);
  signal delta:           unsigned(dec_w-1 downto 0);
  signal decr:            unsigned(rund_w-1 downto 0);
  signal round:           std_logic_vector(sr_w downto 0);
  signal s:               unsigned(sw_w-1 downto 0);
  signal sub_rnd_en:      std_logic;
  signal rnd_cnt_val:     unsigned(sr_w downto 0); -- width sr_w + 1
  signal sw:              unsigned(sw_w downto 0); -- width + 1 for cout
  signal sub:             std_logic;
  signal add:             std_logic;
  signal ramp_fin:        std_logic;

  
  
 

begin
  
  sweep_c: sweep_cntrl 
    generic map (
      dw        => dec_w,
      f_in_khz  => 12000)
    port map (
      -- input
      clk             => clk,
      freq_en         => s_freq_en,
      reset           => reset,
      ena_soft_trig   => ena_soft_trig,
      ld_delta        => ld_delta,
      ld_delay        => ld_delay,
      ld_flattop_int  => ld_flattop_int,
      set_flattop     => set_flattop,
      stop_in         => stop_fkt,
      hw_trig         => hw_trig,
      sw_trig         => sw_trig,
      ramp_fin        => ramp_fin,
      delta           => delta,
      d_in            => unsigned(d(dec_w-1 downto 0)),
      
      -- output
      wr_delta        => wr_delta,
      s_stop_delta    => s_stop_delta,
      wr_ft_int       => wr_ft_int,
      wr_flattop      => wr_flattop,
      idle            => idle,
      w_start         => wait_start,
      work            => work,
      stop            => stop,
      stop_exec       => stop_exec,
      to_err          => to_err,
      seq_err         => seq_err,
      trigger         => trigger,
      init_hw         => init_hw       
    );
  

  -- downcounter for frequency division
  freq_cnt: process(clk, pu_reset)
  begin
    if pu_reset = '1' then
      s_freq_cnt <= to_unsigned(c_12Mhz_cnt, c_12Mhz_cnt_width);
    elsif rising_edge(clk) then
        -- reload count at overflow
        if s_freq_en = '1' then      
          -- choosing constant from array
          s_freq_cnt <= to_unsigned(c_12Mhz_cnt, c_12Mhz_cnt_width);      
        else
          s_freq_cnt <= s_freq_cnt - 1;
        end if;
    end if;
  end process freq_cnt; 
  
  s_freq_en <= s_freq_cnt(s_freq_cnt'high);
  
  
  flat_reg: process(clk, s_freq_en, wr_ft_int)
  begin
    if init_hw = '1' then
      flattop <= (others => '0');
    elsif rising_edge(clk) then
      if s_freq_en = '1' and wr_ft_int = '1' then
        flattop <= unsigned(d);
      end if;
    end if;
  end process;
  
  delta_reg: process(clk, s_stop_delta, wr_delta)
  begin
    if rising_edge(clk) then
      if s_freq_en = '1' then
        if s_stop_delta = '1' then
          delta(dec_w-1 downto 0) <= c_delta_val;
        elsif wr_delta = '1' then
          delta(dec_w-1 downto 0) <= unsigned(d(dec_w-1 downto 0));
        end if;
      end if;
    end if;
  end process;
  
  --delta(rund_w-1 downto dec_w) <= "00000000000"; 
  
  sub_rnd_en <= add and not round(sr_w);
  
  sub_reg: process(clk, init_hw, sub_rnd_en)
  begin
    if init_hw = '1' then
      decr <= (others => '0');
    elsif rising_edge(clk) then
      if s_freq_en = '1' and sub_rnd_en = '1' then
         decr <= decr + resize(delta, rund_w);
      end if;  
    end if;
  end process;

  s(dec_w-1 downto 0) <= decr(rund_w-1 downto sr_w);
  s(sw_w-1 downto dec_w) <= (others => '0');
  
  rnd_cnt: process(clk, init_hw, sub_rnd_en)
  begin
    if init_hw = '1' then
      rnd_cnt_val <= to_unsigned(2**sr_w-1, sr_w + 1);
    elsif rising_edge(clk) then
      if s_freq_en = '1' and sub_rnd_en = '1' then
        rnd_cnt_val <= rnd_cnt_val - 1;
      end if;
    end if;
  
  end process;
  
  add_sub: process(clk, work, stop)
  begin
    if work = '0' and stop = '0' then
      add <= '0';
      sub <= '0';
    elsif rising_edge(clk) then
      if s_freq_en = '1' then
        add <= not add;
        sub <= add;
      end if;
    end if;
  
  end process;
  
  sw_reg: process(clk, init_hw, wr_flattop, sub)
  begin
    if init_hw = '1' then
      sw <= (others => '0');
    elsif rising_edge(clk) then
      if s_freq_en = '1' then
        if wr_flattop = '1' then
          sw <= flattop & "000000";
        end if;
        
        if sub = '1' then
          sw <= sw - s;
        end if;
        
        ramp_fin <= not sw(sw_w-1); -- cout
      end if;
    end if; 
  end process;
  
  sweep_out <= std_logic_vector(sw(sw_w-2 downto sw_w-17));
  
  sweep_stat <= '0' & wait_start & work & idle
                & stop_exec & to_err & seq_err & trigger
                & '0' & '0' & '0' & ena_soft_trig
                & std_logic_vector(to_unsigned(version, 4));
  sweep_vers <= std_logic_vector(to_unsigned(version, 4));

end architecture;