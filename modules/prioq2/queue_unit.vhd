library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


library work;
use work.genram_pkg.all;
use work.wishbone_pkg.all;
use work.gencores_pkg.all;


entity queue_unit is
generic(
  g_depth     : natural := 16;
  g_words     : natural := 8
);
port(
  clk_i       : in  std_logic;
  rst_n_i     : in  std_logic;
  
  slave_i     : in t_wishbone_slave_in;
  slave_o     : out t_wishbone_slave_out;
  
  master_o    : out t_wishbone_master_out;
  master_i    : in t_wishbone_master_in;
  full_o      : out std_logic;
  ts_o        : out std_logic_vector(63 downto 0);
  ts_valid_o  : out std_logic

);
end entity;

architecture behavioral of queue_unit is
  -- range and bit position contants for prio data type: 
  -- standard, timestamp high word, timestamp low word, ts high word w drop, ts low word w drop,

  -- #define PRIO_DAT_STD     0x00
  -- #define PRIO_DAT_TS_HI   0x04    
  -- #define PRIO_DAT_TS_LO   0x08 
  -- #define PRIO_DRP_TS_HI   0x14    
  -- #define PRIO_DRP_TS_LO   0x18 
  
  constant c_PRIO_TYPE_WIDTH : natural := 3; -- 32b aligned, cut off unnecessary low two bit
  constant c_PRIO_TYPE_HI    : natural := c_PRIO_TYPE_WIDTH-1+2;
  constant c_PRIO_TYPE_LO    : natural :=                   0+2;

  constant c_TS_HI    : natural := 0; 
  constant c_TS_LO    : natural := 1;
  constant c_TS_DROP  : natural := 2;
  
  signal  s_d_ts, s_q_ts_hi, s_q_ts_lo : t_wishbone_data;
  signal  s_d, s_q : std_logic_vector(t_wishbone_data'length+c_PRIO_TYPE_WIDTH-1 downto 0);
  signal  s_q_type, s_d_type : std_logic_vector(c_PRIO_TYPE_WIDTH-1 downto 0);
  signal  s_q_data : t_wishbone_data;

  signal  s_master_o : t_wishbone_master_out;
  signal  s_slave_o : t_wishbone_slave_out;
  
  signal  s_pop, s_pop_ts,
          s_push, s_push_main, s_push_ts_hi, s_push_ts_lo,
          s_full, s_full_ts_hi, s_full_ts_lo,
          s_empty, s_empty_ts_hi, s_empty_ts_lo,
          s_done, r_done,
          r_sel, s_sel,
          s_ts_valid,
          s_ackwait,
          s_reloadwait,
          r_cyc,
          s_cyc_re,
          s_push_out,
          s_drop_bit  : std_logic;
  
  signal r_ack_cnt,
         r_stb_cnt : unsigned(7 downto 0);
  signal r_reload_cnt : unsigned(1 downto 0);





begin

  
  
  fifo : generic_sync_fifo
  generic map (
    g_data_width    => t_wishbone_data'length+c_PRIO_TYPE_WIDTH, 
    g_size          => g_depth,
    g_show_ahead    => true,
    g_with_empty    => true,
    g_with_full     => true)
  port map (
    rst_n_i        => rst_n_i,         
    clk_i          => clk_i,
    d_i            => s_d,
    we_i           => s_push_main,
    q_o            => s_q,
    rd_i           => s_pop,
    empty_o        => s_empty,
    full_o         => s_full,
    almost_empty_o => open,
    almost_full_o  => open,
    count_o        => open);


  ts_hi_fifo : generic_sync_fifo
  generic map (
    g_data_width    => 32, 
    g_size          => g_depth/4,
    g_show_ahead    => true,
    g_with_empty    => true,
    g_with_full     => true)
  port map (
    rst_n_i        => rst_n_i,         
    clk_i          => clk_i,
    d_i            => s_d_ts,
    we_i           => s_push_ts_hi,
    q_o            => s_q_ts_hi,
    rd_i           => s_pop_ts,
    empty_o        => s_empty_ts_hi,
    full_o         => s_full_ts_hi,
    almost_empty_o => open,
    almost_full_o  => open,
    count_o        => open);

  ts_lo_fifo : generic_sync_fifo
  generic map (
    g_data_width    => 32, 
    g_size          => g_depth/4,
    g_show_ahead    => true,
    g_with_empty    => true,
    g_with_full     => true)
  port map (
    rst_n_i        => rst_n_i,         
    clk_i          => clk_i,
    d_i            => s_d_ts,
    we_i           => s_push_ts_lo,
    q_o            => s_q_ts_lo,
    rd_i           => s_pop_ts,
    empty_o        => s_empty_ts_lo,
    full_o         => s_full_ts_lo,
    almost_empty_o => open,
    almost_full_o  => open,
    count_o        => open);

  full_o <= s_full;

  s_drop_bit      <= s_q_type(c_TS_DROP);
  s_done          <= s_q_type(c_TS_LO) and s_pop;
  s_cyc_re        <= s_master_o.cyc and not r_cyc; --rising edge of master out cycle line
  s_push_out      <= s_master_o.cyc and s_master_o.stb and not master_i.stall;

  s_d             <= slave_i.adr(c_PRIO_TYPE_HI downto c_PRIO_TYPE_LO) & slave_i.dat;
  s_push          <= slave_i.cyc and slave_i.stb and not s_slave_o.stall;
  s_push_main     <= s_push and not s_full;
  s_pop           <= not s_empty and (s_push_out or s_drop_bit);
 
  s_d_type        <= s_d(c_PRIO_TYPE_WIDTH+t_wishbone_data'length-1 downto t_wishbone_data'length);
  s_q_type        <= s_q(c_PRIO_TYPE_WIDTH+t_wishbone_data'length-1 downto t_wishbone_data'length);
  s_q_data        <= s_q(t_wishbone_data'length-1 downto 0);
  
  s_d_ts          <= slave_i.dat;
  s_push_ts_hi    <= s_push and s_d_type(c_TS_HI) and not s_full_ts_hi;
  s_push_ts_lo    <= s_push and s_d_type(c_TS_LO) and not s_full_ts_lo;
  s_pop_ts        <= s_done and not (s_empty_ts_hi or s_empty_ts_lo);


  ts_o            <= s_q_ts_hi & s_q_ts_lo;
  s_ts_valid      <= not (s_empty_ts_hi or s_empty_ts_lo);
  ts_valid_o      <= s_ts_valid ; -- and not r_ts_valid;  

  --master if
  master_o        <= s_master_o;
  s_master_o.we   <= '1';
  s_master_o.dat  <= s_q_data;
  s_master_o.cyc  <= s_sel and (s_ackwait or s_reloadwait); -- only raise cycle if our queue was selected by ts comparator
  s_master_o.stb  <= not (s_empty or r_done or s_drop_bit); -- only strobe if our queue was selected by ts comparator
  s_master_o.sel  <= x"f";
  s_master_o.adr  <= (others => '0');

  --slave if
  s_slave_o.stall <= s_full or s_full_ts_hi or s_full_ts_lo; --output flow control. bit wasteful, but safe
  s_slave_o.err   <= '0';
  s_slave_o.dat   <= (others => '0');

  slave_o         <= s_slave_o;

  
     
  s_sel <= not s_empty or r_sel;
  



  reg : process(clk_i)
  begin
    if(rising_edge(clk_i)) then
      
      if(rst_n_i = '0') then
        s_slave_o.ack <= '0';
        r_cyc         <= '0'; 
        r_done        <= '0';
        r_sel         <= '0';
      else
        s_slave_o.ack <= s_push;
        r_cyc         <= s_master_o.cyc;
        r_done        <= (r_done or s_done) and s_master_o.cyc;
        r_sel         <= (r_sel or not s_empty) and s_master_o.cyc;          
      end if;  
    end if; 
  end process;

  s_reloadwait <=  not r_reload_cnt(r_reload_cnt'high);

  reloadwait : process(clk_i)
  begin
    if(rising_edge(clk_i)) then
      if(rst_n_i = '0') then
        r_reload_cnt <= (others => '1');
      else
        if (s_ackwait = '1') then
          r_reload_cnt <= to_unsigned(0, r_reload_cnt'length);
        elsif(r_reload_cnt(r_reload_cnt'high) = '0') then 
          r_reload_cnt <= r_reload_cnt-1;
        end if;
      end if;
    end if; 
  end process;





  s_ackwait <= '0' when r_stb_cnt = r_ack_cnt and r_done = '1'
          else '1';


  cnt : process(clk_i)
  begin
    if(rising_edge(clk_i)) then
      if(rst_n_i = '0' or s_master_o.cyc = '0') then
        r_stb_cnt <= (others => '0');
        r_ack_cnt <= (others => '0');
      else
        r_stb_cnt <= r_stb_cnt + ("0000000" & s_push_out);
        r_ack_cnt <= r_ack_cnt + ("0000000" & (master_i.ack or master_i.err));
      end if;
    end if; 
  end process;

  




end behavioral;

