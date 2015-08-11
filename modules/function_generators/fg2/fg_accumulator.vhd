library ieee;
use ieee.std_logic_1164.all;
use ieee.math_real.all;
use ieee.numeric_std.all;

library work;
use work.fg2_pkg.all;

-- a + bx + cx^2 + dx^3
entity fg_accumulator is
  port(
    clk_i     : in  std_logic;
    rstn_i    : in  std_logic;
    dac_stb_i : in  std_logic;
    dac_val_o : out std_logic_vector(63 downto 0);
    start_i   : in  std_logic; -- 0=>1 means accumulator begins
    running_o : out std_logic;
    irq_o     : out std_logic; -- 0=>1 means refill
    full_o    : out std_logic; -- writes to abcdsn should fail
    stall_o   : out std_logic; -- writes are ignored
    a_o       : out std_logic_vector(63 downto 0);
    b_o       : out std_logic_vector(63 downto 0);
    c_o       : out std_logic_vector(63 downto 0);
    d_o       : out std_logic_vector(63 downto 0);
    n_o       : out std_logic_vector(15 downto 0);
    a_we_i    : in  std_logic_vector(7 downto 0);
    b_we_i    : in  std_logic_vector(7 downto 0);
    c_we_i    : in  std_logic_vector(7 downto 0);
    d_we_i    : in  std_logic_vector(7 downto 0);
    s_we_i    : in  std_logic_vector(1 downto 0);
    n_we_i    : in  std_logic_vector(1 downto 0); -- low byte (0) triggers readiness
    a_i       : in  std_logic_vector(63 downto 0);
    b_i       : in  std_logic_vector(63 downto 0);
    c_i       : in  std_logic_vector(63 downto 0);
    d_i       : in  std_logic_vector(63 downto 0);
    s_i       : in  std_logic_vector(15 downto 0); -- 0, d, c, b
    n_i       : in  std_logic_vector(15 downto 0));
end entity;

architecture arch of fg_accumulator is
  signal r_accum_a : signed(63 downto 0);
  signal r_accum_b : signed(63 downto 0);
  signal r_accum_c : signed(63 downto 0);
  signal r_accum_d : signed(63 downto 0);
  signal r_accum_n : unsigned(15 downto 0);
  
  signal r_next_a : signed(63 downto 0);
  signal r_next_b : signed(63 downto 0);
  signal r_next_c : signed(63 downto 0);
  signal r_next_d : signed(63 downto 0);
  signal r_next_n : unsigned(15 downto 0);
  
  signal r_modified_a : std_logic;
  signal r_modified_b : std_logic;
  signal r_modified_c : std_logic;
  signal r_modified_d : std_logic;

  signal r_shift_b : unsigned(4 downto 0);
  signal r_shift_c : unsigned(4 downto 0);
  signal r_shift_d : unsigned(4 downto 0);
  signal r_shifting : std_logic;
  
  signal r_full         : std_logic;
  signal r_running      : std_logic;
  signal r_start        : std_logic;
  signal r_stop         : std_logic;
  signal r_irq          : std_logic;
  signal r_tick         : std_logic;
  signal r_clear        : std_logic;
  signal r_dac_stb      : std_logic;
  signal s_end_of_tuple : std_logic;
  
  signal r_load_a     : std_logic;
  signal r_load_b     : std_logic;
  signal r_load_c     : std_logic;
  signal r_load_d     : std_logic;
  signal r_load_n     : std_logic;

begin

  fsm : process(clk_i, rstn_i) is
  begin
    if rstn_i = '0' then
      r_start   <= '0';
      r_running <= '0';
      r_tick    <= '0';
      r_clear   <= '0';
      r_dac_stb <= '0';
    elsif rising_edge(clk_i) then
      if r_tick = '1' then
        r_start <= '0';
      end if;
      
      if r_running = '1' then
        if r_stop = '1' and r_tick = '1' then
          r_running <= '0';
        end if;
      else
        if start_i = '1' and r_full = '1' then
          r_running <= '1';
          r_start <= '1';
        end if;
      end if;
      
      r_tick    <= r_running and dac_stb_i and not r_dac_stb;
      r_clear   <= (r_running and dac_stb_i and not r_dac_stb) and r_load_n;
      r_dac_stb <= dac_stb_i;
    end if;
  end process;

  s_end_of_tuple <= '1' when r_accum_n = 2 else '0';
  acccumulate : process(clk_i, rstn_i) is
  begin
    if rstn_i = '0' then -- asynchronous clear
    
      r_accum_a <= (others => '0');
      r_accum_b <= (others => '0');
      r_accum_c <= (others => '0');
      r_accum_d <= (others => '0');
      r_accum_n <= (others => '0');
      
      r_load_a <= '0';
      r_load_b <= '0';
      r_load_c <= '0';
      r_load_d <= '0';
      r_load_n <= '0';
      r_stop   <= '0';
      r_irq    <= '0';
    
    elsif rising_edge(clk_i) then
      if r_tick = '1' then -- clock enable
      
        r_load_a <= (s_end_of_tuple or r_start) and r_full and r_modified_a;
        r_load_b <= (s_end_of_tuple or r_start) and r_full and r_modified_b;
        r_load_c <= (s_end_of_tuple or r_start) and r_full and r_modified_c;
        r_load_d <= (s_end_of_tuple or r_start) and r_full and r_modified_d;
        r_load_n <= (s_end_of_tuple or r_start) and r_full;
        r_stop   <= (s_end_of_tuple or r_start) and not r_full;
        r_irq    <= (r_load_n or r_stop) and r_tick;
        
        -- a
        if r_load_a = '1' then -- load enable
          r_accum_a <= r_next_a;
        else
          r_accum_a <= r_accum_a + r_accum_b;
        end if;
        
        -- b
        if r_load_b = '1' then
          r_accum_b <= r_next_b;
        else
          r_accum_b <= r_accum_b + r_accum_c;
        end if;
        
        -- c
        if r_load_c = '1' then
          r_accum_c <= r_next_c;
        else
          r_accum_c <= r_accum_c + r_accum_d;
        end if;
        
        -- d
        if r_load_d = '1' then
          r_accum_d <= r_next_d;
        else
          r_accum_d <= r_accum_d;
        end if;
        
        -- n
        if r_load_n = '1' then
          r_accum_n <= r_next_n;
        else
          r_accum_n <= r_accum_n - 1;
        end if;
      end if;
    end if;
  end process;
  
  shifters : process(clk_i, rstn_i) is
  begin
    if rstn_i = '0' then
      r_shift_b <= (others => '0');
      r_shift_c <= (others => '0');
      r_shift_d <= (others => '0');
      r_shifting <= '0';
    elsif rising_edge(clk_i) then
      if r_shifting = '1' then
        if r_shift_b = 0 and r_shift_c = 0 and r_shift_c = 0 then
          r_shifting <= '0';
        end if;
        
        if r_shift_b /= 0 then
          r_shift_b <= r_shift_b - 1;
          r_next_b  <= r_next_b(63) & r_next_b(63 downto 1);
        end if;
        
        if r_shift_c /= 0 then
          r_shift_c <= r_shift_c - 1;
          r_next_c  <= r_next_c(63) & r_next_c(63 downto 1);
        end if;
        
        if r_shift_d /= 0 then
          r_shift_d <= r_shift_d - 1;
          r_next_d  <= r_next_d(63) & r_next_d(63 downto 1);
        end if;
      else
        if s_we_i(0) = '1' then
          r_shifting <= '1';
          r_shift_b             <= unsigned(s_i(4 downto 0));
          r_shift_c(2 downto 0) <= unsigned(s_i(7 downto 5));
        end if;
        
        if s_we_i(1) = '1' then
          r_shifting <= '1';
          r_shift_c(4 downto 3) <= unsigned(s_i(9 downto 8));
          r_shift_d             <= unsigned(s_i(14 downto 10));
        end if;
      end if;
    end if;
  end process;

  start_reg : process(clk_i, rstn_i) is
  begin
    if rstn_i = '0' then
      r_next_n <= (others => '0');
      r_full   <= '0';
    elsif rising_edge(clk_i) then
      if r_clear = '1' then
        r_full <= '0';
        r_next_n <= (others => '0');
      end if;
      
      if n_we_i(0) = '1' then
        r_full <= '1';
        r_next_n(7 downto 0) <= unsigned(n_i(7 downto 0));
      end if;
      if n_we_i(1) = '1' then
        r_full <= '1';
        r_next_n(15 downto 8) <= unsigned(n_i(15 downto 8));
      end if;
    end if;
  end process;
  
  other_regs : process(clk_i, rstn_i) is
  begin
    if rstn_i = '0' then
      r_next_a <= (others => '0');
      r_next_b <= (others => '0');
      r_next_c <= (others => '0');
      r_next_d <= (others => '0');
      r_modified_a <= '0';
      r_modified_b <= '0';
      r_modified_c <= '0';
      r_modified_d <= '0';
    elsif rising_edge(clk_i) then
      if r_clear = '1' then
        r_next_a <= (others => '0');
        r_next_b <= (others => '0');
        r_next_c <= (others => '0');
        r_next_d <= (others => '0');
        r_modified_a <= '0';
        r_modified_b <= '0';
        r_modified_c <= '0';
        r_modified_d <= '0';
      end if;
      
      for i in 0 to 7 loop
        if a_we_i(i) = '1' then
          r_next_a(8*(i+1)-1 downto 8*i) <= signed(a_i(8*(i+1)-1 downto 8*i));
        end if;
        if b_we_i(i) = '1' then
          r_next_b(8*(i+1)-1 downto 8*i) <= signed(b_i(8*(i+1)-1 downto 8*i));
        end if;
        if c_we_i(i) = '1' then
          r_next_c(8*(i+1)-1 downto 8*i) <= signed(c_i(8*(i+1)-1 downto 8*i));
        end if;
        if d_we_i(i) = '1' then
          r_next_d(8*(i+1)-1 downto 8*i) <= signed(d_i(8*(i+1)-1 downto 8*i));
        end if;
      end loop;
      
      if a_we_i /= x"00" then
        r_modified_a <= '1';
      end if;
      if b_we_i /= x"00" then
        r_modified_b <= '1';
      end if;
      if c_we_i /= x"00" then
        r_modified_c <= '1';
      end if;
      if d_we_i /= x"00" then
        r_modified_d <= '1';
      end if;
    end if;
  end process;
    
  
  dac_val_o <= std_logic_vector(r_accum_a);
  
  running_o <= r_running;
  irq_o     <= r_irq;
  full_o    <= r_full;
  stall_o   <= r_shifting;
  
  a_o <= std_logic_vector(r_next_a);
  b_o <= std_logic_vector(r_next_b);
  c_o <= std_logic_vector(r_next_c);
  d_o <= std_logic_vector(r_next_d);
  n_o <= std_logic_vector(r_next_n);
  
end arch;
