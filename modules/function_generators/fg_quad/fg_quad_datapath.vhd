library ieee;
USE ieee.std_logic_1164.all;
use ieee.math_real.all;
use ieee.numeric_std.all;

library work;
use work.fg_quad_pkg.all;

entity fg_quad_datapath is
  generic (
    CLK_in_Hz:  integer := 125000000);
  port (
  data_a:             in  std_logic_vector(15 downto 0);
  data_b:             in  std_logic_vector(15 downto 0);
  clk:                in  std_logic;
  nrst:               in  std_logic;
  a_en, b_en:         in  std_logic;                      -- data register enable
  load_start, s_en:   in  std_logic;
  status_reg_changed: in  std_logic;   
  step_sel:           in  std_logic_vector(2 downto 0);   -- shiftvalue coeff a
  b_shift:            in  integer range 0 to 16;          -- shiftvalue coeff b
  freq_sel:           in  std_logic_vector(2 downto 0);
  sw_out:             out std_logic_vector(23 downto 0);
  set_out:            out std_logic);                     -- debug out
end entity;

architecture fg_quad_dp_arch of fg_quad_datapath is

-- shifter signals
signal s_a_shifted:   signed(39 downto 0);
signal s_b_shifted:   signed(39 downto 0);

-- registerblock
signal s_a_reg:       signed(39 downto 0);
signal s_b_reg:       signed(39 downto 0);
signal s_Q_reg:       signed(39 downto 0);
signal s_X_reg:       signed(39 downto 0);
signal s_start_reg:   signed(39 downto 0);


-- signals statemachine
signal s_load_start:    std_logic;
signal s_inc_quad:      std_logic;
signal s_inc_lin:       std_logic;
signal s_add_lin_quad:  std_logic;

type cntrl_type is (idle, load, quad_inc, lin_inc, addXQ);
signal control_state: cntrl_type;

signal s_cnt_out:   std_logic_vector(7 downto 0);
signal s_reached:   std_logic;
signal s_set:       std_logic;
signal s_cnt_set:   std_logic := '0';

-- constants for frequency and step value counters
type int_array is array(7 downto 0) of integer;
  
constant c_freq_cnt: int_array := (   
                                    (CLK_in_Hz / 2000000) - 2,
                                    (CLK_in_Hz / 1000000) - 2,
                                    (CLK_in_Hz / 500000) -2,
                                    (CLK_in_Hz / 250000) - 2,
                                    (CLK_in_Hz / 125000) - 2,
                                    (CLK_in_Hz / 64000) - 2,
                                    (CLK_in_Hz / 32000) - 2,
                                    (CLK_in_Hz / 16000) - 2
                                    );

constant c_add_cnt: int_array := (
                                   32000 - 2, 
                                   16000 - 2, 
                                   8000 - 2,
                                   4000 - 2,
                                   2000 - 2,
                                   1000 - 2,
                                   500 - 2,
                                   250 - 2
                                   );

-- calculate width from the biggest value
constant  c_freq_cnt_width:       integer := integer(ceil(log2(real(c_freq_cnt(0))))) + 1;
constant  c_add_cnt_width:        integer := integer(ceil(log2(real(c_add_cnt(0))))) + 1;


signal  s_cnt:	       unsigned(7 downto 0);
signal  s_freq_cnt:    unsigned(c_freq_cnt_width - 1 downto 0);
signal  s_freq_en:     std_logic;
signal  s_freq_cnt_en: std_logic := '1';

signal s_add_cnt:     unsigned(c_add_cnt_width - 1 downto 0);
signal s_add_cnt_en:  std_logic := '1';
signal s_stp_reached: std_logic;

begin


fast_cnt: process(clk, nrst)
begin
	if nrst = '0' then
		s_cnt <= "01000111";
	elsif rising_edge(clk) then
		if s_cnt(s_cnt'high) = '1' or s_cnt_set = '1' then
			s_cnt <= "01000111";
		else	
			s_cnt <= s_cnt - 1;
		end if;
	end if;
end process;
	
s_reached <= s_cnt(s_cnt'high);
s_cnt_out <= std_logic_vector(s_cnt);



-- shifting for linear coefficient a
shift_a: process(step_sel, data_a)
begin
  case step_sel is
    when "000"  =>  s_a_shifted <= resize(signed(data_a), 40) sll 17;
    when "001"  =>  s_a_shifted <= resize(signed(data_a), 40) sll 16; 
    when "010"  =>  s_a_shifted <= resize(signed(data_a), 40) sll 15; 
    when "011"  =>  s_a_shifted <= resize(signed(data_a), 40) sll 14;
    when "100"  =>  s_a_shifted <= resize(signed(data_a), 40) sll 13; 
    when "101"  =>  s_a_shifted <= resize(signed(data_a), 40) sll 12; 
    when "110"  =>  s_a_shifted <= resize(signed(data_a), 40) sll 11; 
    when "111"  =>  s_a_shifted <= resize(signed(data_a), 40) sll 10;
    when others =>  s_a_shifted <= resize(signed(data_a), 40) sll 10;
  end case;
end process;

-- shifting for quadratic coefficient b
s_b_shifted <= shift_left(resize(signed(data_b), 40), b_shift);

-- registers a, b, Q, X, start und adder for lin und quad term
reg_file: process (clk, nrst, a_en, b_en, s_en, load_start)
begin
  if nrst = '0' then
    s_a_reg     <=  (others => '0');
    s_b_reg     <=  (others => '0');
    s_Q_reg     <=  (others => '0');
    s_X_reg     <=  (others => '0');
    s_start_reg <=  (others => '0');
  elsif rising_edge(clk) then
    if a_en = '1' then
      s_a_reg <= (s_a_shifted);
    end if;
    
    if b_en = '1' then
      s_b_reg <= (s_b_shifted);
    end if;
    
    if s_en = '1' then
      s_start_reg <= signed(x"000000" & data_a); -- TEST --
    end if;
  
  -- init quad term with start value
    if s_load_start = '1' then 
      s_Q_reg <= s_start_reg; 
    end if;
    
    -- increment quad term
    if s_inc_quad = '1' then
      s_Q_reg <= signed(s_Q_reg) + signed(s_b_reg);
    end if; 
    
    -- increment linear term
    if s_inc_lin = '1' then
      s_X_reg <= signed(s_X_reg) + signed(s_a_reg);
    end if;
    
    -- sum of linear and quadratic term
    if s_add_lin_quad = '1' then
      s_X_reg <= signed(s_X_reg) + signed(s_Q_reg);
    end if; 
  end if;

end process;


  -- downcounter for frequency division
  freq_cnt: process(clk, nrst)
  begin
    -- important for synced start 
    --if nrst = '0' or s_reset_freq_cnt = '1' then
    if nrst = '0' then
      s_freq_cnt <= (others => '0');
    elsif rising_edge(clk) then
      -- reload count at overflow
      if s_freq_en = '1' then      
        -- choosing constant from array
        s_freq_cnt <= to_unsigned(c_freq_cnt(to_integer(unsigned(freq_sel))), c_freq_cnt_width);      
      elsif s_freq_cnt_en = '1' then
        s_freq_cnt <= s_freq_cnt - 1;
      end if;
    end if;
  end process freq_cnt; 
  
  s_freq_en <= s_freq_cnt(s_freq_cnt'high);

  -- downcounter for step values
  add_cnt: process(clk, nrst)
  begin
    if nrst = '0' then
      s_add_cnt <= (others => '0');
    elsif rising_edge(clk) then
      -- reload counter at overflow or status register access 
      if s_stp_reached = '1' or status_reg_changed = '1' then
          -- choosing constant from array
          s_add_cnt <= to_unsigned(c_add_cnt(to_integer(unsigned(step_sel))), c_add_cnt_width);
        
      elsif s_freq_en = '1' and s_add_cnt_en = '1' then
        s_add_cnt <= s_add_cnt - 1;
      end if;
    end if;
  end process add_cnt;
  
  s_stp_reached <= std_logic(s_add_cnt(s_add_cnt'high));

-- control state machine
  control_sm: process (clk, nrst)
  begin
    if nrst = '0' then
      control_state <= load;
      
    elsif rising_edge(clk) then
      s_load_start  <= '0';
      s_inc_quad    <= '0';
      s_inc_lin     <= '0';
      s_add_lin_quad  <= '0';
    
      case control_state is
        when load =>
          s_load_start <= '1';
          
          control_state <= quad_inc;
    
        when quad_inc =>
          s_inc_quad <= '1';
          
          control_state <= lin_inc;
          
        when lin_inc =>
          s_inc_lin <= '1';
          
          control_state <= addXQ;
        
        when addXQ =>
          s_add_lin_quad <= '1';
          
          if load_start = '1' then
            control_state <= load;
          else
            control_state <= quad_inc;
          end if;
          
        when others =>
          
      end case;
    end if;
  end process;


-- output register for the 24 most significant bits
sw_out <= std_logic_vector(s_X_reg(39 downto 16));

set_out <= s_set;

end FG_quad_DP_arch;
