library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.genram_pkg.all;
use work.wishbone_pkg.all;
use work.gencores_pkg.all;
use work.heap_pkg.all;

entity heap_tb is
end heap_tb;

architecture rtl of heap_tb is 

   constant c_dummy_slave_in     : t_wishbone_slave_in   := ('0', '0', x"00000000", x"F", '0', x"00000000");
   constant c_dummy_slave_out    : t_wishbone_slave_out  := ('0', '0', '0', '0', '0', x"00000000");
   constant c_dummy_master_out   : t_wishbone_master_out := c_dummy_slave_in;
   constant c_dummy_master_in    : t_wishbone_master_in  := c_dummy_slave_out;

   constant c_key_width : natural := 64;
   constant c_val_width : natural := 192;

   constant c_data_width : natural := c_key_width + c_val_width;
   
   constant c_words_per_entry : natural := (c_data_width + t_wishbone_data'length -1) / t_wishbone_data'length;
   
   subtype t_val   is std_logic_vector(c_val_width  -1 downto 0);
   subtype t_key   is std_logic_vector(c_key_width  -1 downto 0);
   subtype t_data  is std_logic_vector(c_data_width -1 downto 0);
   subtype t_skey  is std_logic_vector(c_key_width  -1 downto 0);
   subtype t_sval  is t_val;

  
   signal clk_sys 			            : std_logic := '0';
	signal rst_n,rst_lm32_n             : std_logic := '0';
	
   -- Clock period definitions
   constant clk_period     : time      := 8 ns;
   constant c_width        : natural   := 5;
   
   signal s_data_in, s_data_out, sample_in, sample_out : t_data;
   
   signal s_count : std_logic_vector(c_width-1 downto 0); --x"1555AAAAFFFFFFAA";
   signal s_push, s_pop, s_dbg, s_busy, s_full, s_out, s_empty, s_dbg_ok, s_dbg_err : std_logic := '0';
   signal r_push : std_logic := '0';
   
   signal s_ctrl_i : t_wishbone_slave_in;
   signal s_ctrl_o : t_wishbone_slave_out;
   
   signal s_snk_i : t_wishbone_slave_in;
   signal s_snk_o : t_wishbone_slave_out;
   
   signal s_src_i : t_wishbone_master_in;
   signal s_src_o : t_wishbone_master_out;
   
   signal master_i : t_wishbone_master_in;
   signal master_o : t_wishbone_master_out;
   
   signal count, r_ack_cnt, r_stb_cnt         : natural;
signal r_ack_clr  : std_logic := '1';
   
   signal r_time : std_logic_vector(63 downto 0);
   
   component ftm_priority_queue is
generic(
   g_is_ftm       : boolean := false;  
   g_idx_width    : natural := 8;
   g_key_width    : natural := 64;
   g_val_width    : natural := 192  
);            
port(
   clk_sys_i   : in  std_logic;
   rst_n_i     : in  std_logic;

   time_sys_i  : std_logic_vector(63 downto 0) := (others => '1');

   ctrl_i      : in  t_wishbone_slave_in;
   ctrl_o      : out t_wishbone_slave_out;
   
   snk_i       : in  t_wishbone_slave_in;
   snk_o       : out t_wishbone_slave_out;
   
   src_o       : out t_wishbone_master_out;
   src_i       : in  t_wishbone_master_in
  
);
end component;
   
   

begin
  
   heap : heap_top
   generic map (
      g_idx_width    => c_width,
      g_val_width    => c_val_width,
      g_key_width    => c_key_width 
   )            
   port map(clk_sys_i   => '0',
            rst_n_i     => rst_n,
            
            dbg_show_i  => s_dbg,
            dbg_ok_o    => s_dbg_ok,       
            dbg_err_o   => s_dbg_err,
    
          
            push_i      => s_push,
            pop_i       => s_pop,
            busy_o      => s_busy,
            full_o      => s_full,
            empty_o     => s_empty,
            count_o     => s_count,
           
            
            data_i     => s_data_in,
            data_o     => s_data_out, 
            out_o      => s_out 
    
    );
   
  
   wb_heap : ftm_priority_queue
   generic map(
      g_idx_width    => c_width,
      g_key_width    => c_key_width,
      g_val_width    => c_val_width
       
   )           
   port map(
      clk_sys_i   => clk_sys,
      rst_n_i     => rst_n,

      time_sys_i  => r_time,
      ctrl_i      => s_ctrl_i,
      ctrl_o      => open,
      
      snk_i       => s_snk_i,
      snk_o       => s_snk_o,
      
      src_o       => s_src_o,
      src_i       => s_src_i
     
   );
 
    
 
    
    
   -- Clock process definitions( clock with 50% duty cycle is generated here.
   clk_process :process
   begin
        clk_sys <= '0';
        wait for clk_period/2;  --for 0.5 ns signal is '0'.
        clk_sys <= '1';
        wait for clk_period/2;  --for next 0.5 ns signal is '1'.
        
   end process;
   
   sample :process(clk_sys)
   begin
   if(rising_edge(clk_sys)) then
      s_src_i.stall <= '0';
      s_src_i.ack <= s_src_o.stb and s_src_o.cyc and not s_src_i.stall;
   if(s_push = '1') then
    sample_in <= s_data_in;
   end if;
   if(s_out = '1') then
      sample_out <= s_data_out;
   end if;
   end if;  
   end process;  
     
  
   
    -- Stimulus process
  rst: process
  begin        
        
        rst_n  <= '0';
        r_time <= std_logic_vector(to_unsigned(0,64));
        wait until rising_edge(clk_sys);
        wait for clk_period*5;
        rst_n <= '1';
        wait for clk_period*10;
        wait until rst_n = '0';
   end process;
   

  ack_process :process(clk_sys)
   variable add_ack : std_logic_vector( 0 downto 0);
   begin
      if(rising_edge(clk_sys)) then
         add_ack(0) := master_i.ack or master_i.err;
         if (r_ack_clr = '1') then
          r_ack_cnt <= 0;
         else 
          r_ack_cnt <= r_ack_cnt + to_integer(unsigned(add_ack));      
         end if;
      end if;
   end process;
   
   

   
   
   
   
   -- Stimulus process
  s_snk_i <= master_o;
  master_i <= s_snk_o;
   

   
    -- Stimulus process
  stim_WB_proc: process
  variable i, j : natural;
  variable v_key  : std_logic_vector(c_key_width-1 downto 0);
  variable v_val  : std_logic_vector(c_val_width-1 downto 0);
  variable v_data : std_logic_vector(c_words_per_entry * t_wishbone_data'length-1 downto 0); 
  
  procedure wb_wr( adr : in unsigned(31 downto 0);
                    dat : in std_logic_vector(31 downto 0);
                    hold : in std_logic 
                  ) is
  begin
    
    if(master_o.cyc = '0') then
       r_ack_clr <= '1';
       r_stb_cnt <= 1;
    else
      r_ack_clr <= '0';
      r_stb_cnt <= r_stb_cnt +1;
    end if;
    
    master_o.cyc <= '1';
    master_o.stb  <= '1';
    master_o.we   <= '1';
    master_o.adr  <= std_logic_vector(adr);
    master_o.dat  <= dat;
    wait for clk_period; 
    r_ack_clr <= '0';
    while master_i.stall= '1'loop
      wait for clk_period; 
    end loop;
    master_o.stb  <= '0';
    
    
    if(hold = '0') then
       while r_ack_cnt < r_stb_cnt loop
          wait for clk_period;
       end loop;
       master_o.cyc <= '0'; 
       wait for clk_period;      
    end if;
  end procedure wb_wr;
  
  begin        
         master_o         <= c_dummy_master_out;
        i := 3;
        s_ctrl_i <= ('0', '0', x"00000000", x"F", '0', x"00000000"); 
        wait until rst_n = '1';
        wait until rising_edge(clk_sys);
        wait for clk_period*5; 
        
        s_ctrl_i <= ('1', '1', x"00000020", x"F", '1', x"7FFFFFF0"); -- set dst adr
        wait for clk_period;
        s_ctrl_i <= ('1', '1', x"00000018", x"F", '1', x"00000003"); -- set cfg enable and fifo mode
        wait for clk_period;
        s_ctrl_i <= ('1', '1', x"00000030", x"F", '1', x"00000000"); -- set cfg enable and fifo mode
        wait for clk_period;
        s_ctrl_i <= ('1', '1', x"00000034", x"F", '1', x"00000100"); -- set cfg enable and fifo mode
        wait for clk_period;
        s_ctrl_i <= ('1', '1', x"00000038", x"F", '1', x"00000000"); -- set cfg enable and fifo mode
        wait for clk_period;
        s_ctrl_i <= ('1', '1', x"0000003C", x"F", '1', x"00000100"); -- set cfg enable and fifo mode
        wait for clk_period;
        
         
        s_ctrl_i <= ('0', '0', x"00000000", x"F", '0', x"00000000"); 
        wait for clk_period*1;
        
        report "+++++++++++++++ +++++++++++++++ +++++++++++++ Start INSERT" severity warning;
        
        v_key := std_logic_vector(to_unsigned(0+1000, v_key'length));
        v_val := std_logic_vector(to_unsigned(4, v_val'length));
        v_data := v_val & std_logic_vector( to_unsigned(0, (c_words_per_entry * t_wishbone_data'length - c_data_width))) & v_key ;
        
        for j in 0 to c_words_per_entry-1-1 loop 
           wb_wr(x"00000000", v_data(v_data'left - j*t_wishbone_data'length downto  v_data'length - (j+1)*t_wishbone_data'length), '1');
        end loop;
        wb_wr(x"00000000", v_data(v_data'left - (c_words_per_entry-1)*t_wishbone_data'length downto  v_data'length - (c_words_per_entry)*t_wishbone_data'length), '0');
        
        
        while (i > 0) 
        loop
        
        v_key := std_logic_vector(to_unsigned(i*2+1000, v_key'length));
        v_val := std_logic_vector(to_unsigned(i, v_val'length));
        v_data := v_val & std_logic_vector( to_unsigned(0, (c_words_per_entry * t_wishbone_data'length - c_data_width))) & v_key ;
        
        for j in 0 to c_words_per_entry-1-1 loop 
           wb_wr(x"00000000", v_data(v_data'left - j*t_wishbone_data'length downto  v_data'length - (j+1)*t_wishbone_data'length), '1');
        end loop;
        wb_wr(x"00000000", v_data(v_data'left - (c_words_per_entry-1)*t_wishbone_data'length downto  v_data'length - (c_words_per_entry)*t_wishbone_data'length), '0');
        if(i > 50) then
           s_ctrl_i <= ('1', '1', x"00000004", x"F", '1', x"00000001"); -- force-pop
           wait for clk_period;
           s_ctrl_i <= ('0', '0', x"00000000", x"F", '0', x"00000000"); 
           wait for clk_period;
        end if;  
        i := i -1;
        end loop;
          
        
        wait for clk_period*10;
        
        -- i := 50;
        -- report "+++++++++++++++ +++++++++++++++ +++++++++++++ Start REPLACE" severity warning;
        -- while (i > 0) 
        -- loop
        -- s_data_in <= std_logic_vector(to_unsigned(i*2, t_key'length)) & s_data_in(t_sval'range);
        -- wait for clk_period*3;
        -- s_push <= '1';
        -- s_pop <= '1';
        -- wait for clk_period;
        -- s_push <= '0';
        -- s_pop <= '0'; 
        -- wait for clk_period*20;
        
        -- s_dbg <= '1';
        -- wait for clk_period;
        -- s_dbg <= '0';
        -- wait for clk_period*150;
        -- i := i -1;
        -- end loop;
        
        s_ctrl_i <= ('1', '1', x"00000004", x"F", '1', x"00000001"); -- force-pop
           wait for clk_period;
           s_ctrl_i <= ('0', '0', x"00000000", x"F", '0', x"00000000"); 
           wait for clk_period*14;
        
        s_ctrl_i <= ('1', '1', x"00000004", x"F", '1', x"00000002"); -- force-send
           wait for clk_period;
           s_ctrl_i <= ('0', '0', x"00000000", x"F", '0', x"00000000"); 
           wait for clk_period*1;
     
        
        i := 3;
        report "+++++++++++++++ +++++++++++++++ +++++++++++++ Start REMOVE" severity warning;
        while (i > 0) 
        loop
        
           s_ctrl_i <= ('1', '1', x"00000004", x"F", '1', x"00000001"); -- force-pop
           wait for clk_period;
           s_ctrl_i <= ('0', '0', x"00000000", x"F", '0', x"00000000"); 
           wait for clk_period*8;
           i := i -1;
           end loop;
         
        wait until rst_n = '0';
  end process;

   
   
   


end rtl;  
