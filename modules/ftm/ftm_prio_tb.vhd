library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.genram_pkg.all;
use work.wishbone_pkg.all;
use work.gencores_pkg.all;
use work.heap_pkg.all;
use work.etherbone_pkg.all;
use work.wr_fabric_pkg.all;

entity ftm_prio_tb is
end ftm_prio_tb;

architecture rtl of ftm_prio_tb is 

   constant c_RST        : natural := 0;                 --wo, reset. writing 1 will reset the core
   constant c_FORCE      : natural := c_RST        +4;   --wo, pops one element from the heap if heap is not empty
   constant c_DBG_SET    : natural := c_FORCE      +4;   --wo, start heap consistency check when writing 1
   constant c_DBG_GET    : natural := c_DBG_SET    +4;   --ro, shows heap consistency check result 
   constant c_CLEAR      : natural := c_DBG_GET    +4;   --wo, clear heap on writing 1 
   constant c_CFG_GET    : natural := c_CLEAR      +4;   --ro, get   cfg bits, see below 
   constant c_CFG_SET    : natural := c_CFG_GET    +4;   --wo, set   cfg bits
   constant c_CFG_CLR    : natural := c_CFG_SET    +4;   --wo, clear cfg bits
   constant c_DST_ADR    : natural := c_CFG_CLR    +4;   --rw, wishbone address of src output
   constant c_HEAP_CNT   : natural := c_DST_ADR    +4;   --ro, current number of elements on the heap
   constant c_MSG_CNT_O  : natural := c_HEAP_CNT   +4;   --ro, total number of elements dequeued since last clear/reset
   constant c_MSG_CNT_I  : natural := c_MSG_CNT_O  +4;   --ro, total number of elements queued since last clear/reset
   
   constant c_T_TRN_HI   : natural := c_MSG_CNT_I  +4;   --rw, transmission time window in 8ns, high word. Used for autopop
   constant c_T_TRN_LO   : natural := c_T_TRN_HI   +4;   --rw, transmission time window in 8ns, low word
   constant c_T_DUE_HI   : natural := c_T_TRN_LO   +4;   --rw, transmission time window in 8ns, high word. Used for autopop
   constant c_T_DUE_LO   : natural := c_T_DUE_HI   +4;   --rw, transmission time window in 8ns, low word
   
   constant c_MSG_MIN    : natural := c_T_DUE_LO   +4;   --rw, minimum number of elements to dequeue before send & wait for EBM readiness. used for autopop
   constant c_MSG_MAX    : natural := c_MSG_MIN    +4;   --rw, maximum number of elements to dequeue before send & wait for EBM readiness. used for autopop
   constant c_EBM_ADR    : natural := c_MSG_MAX    +4;   --rw, wishbone address of the Etherbone Master
   
-- cfg reg bits
   constant c_CFG_BIT_ENA            : natural := 0;
   constant c_CFG_BIT_FIFO           : natural := 1;    
   constant c_CFG_BIT_IRQ            : natural := 2;
   constant c_CFG_BIT_AUTOPOP        : natural := 3;
   constant c_CFG_BIT_AUTOFLUSH_TIME : natural := 4;
   constant c_CFG_BIT_AUTOFLUSH_MSGS : natural := 5;
   constant c_CFG_BITS               : natural := c_CFG_BIT_AUTOFLUSH_MSGS;


   constant c_dummy_slave_in     : t_wishbone_slave_in   := ('0', '0', x"00000000", x"F", '0', x"00000000");
   constant c_dummy_slave_out    : t_wishbone_slave_out  := ('0', '0', '0', '0', '0', x"00000000");
   constant c_dummy_master_out   : t_wishbone_master_out := c_dummy_slave_in;
   constant c_dummy_master_in    : t_wishbone_master_in  := c_dummy_slave_out;

   constant c_key_width : natural := 64;
   constant c_val_width : natural := 192;

   constant c_data_width : natural := c_key_width + c_val_width;
   
   constant c_words_per_entry : natural := (c_data_width + t_wishbone_data'length -1) / t_wishbone_data'length;
   
   constant c_test_msgs : natural    := 20;
   constant c_test_trn : natural    :=  2000; 
   constant c_test_due : natural    :=  250;
   constant c_test_margin : natural := 350;
   
   subtype t_val   is std_logic_vector(c_val_width  -1 downto 0);
   subtype t_key   is std_logic_vector(c_key_width  -1 downto 0);
   subtype t_data  is std_logic_vector(c_data_width -1 downto 0);
   subtype t_skey  is std_logic_vector(c_data_width -1 downto c_val_width);
   subtype t_sval  is t_val;

   signal r_sys_time : t_key;

   type t_key_array is array (natural range <>) of t_key;
   type t_data_array is array (natural range <>) of t_data;
  
   signal r_verify_array : t_data_array(c_test_msgs downto 0);
   signal r_time_array   : t_key_array(c_test_msgs downto 0);
  signal testme : t_key;
   signal clk_sys 			            : std_logic := '0';
	signal rst_n,rst_lm32_n             : std_logic := '0';
	
	  signal s_rec 			            : std_logic := '0';
   -- Clock period definitions
   constant clk_period     : time      := 8 ns;
   constant c_width        : natural   := 10;
   
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
   
   signal s_ebs_wbm_i : t_wishbone_master_in;
   signal s_ebs_wbm_o : t_wishbone_master_out;
   
   signal s_fsrc_i          : t_wrf_source_in;
   signal s_fsrc_o          : t_wrf_source_out;
   
   
   signal r_sreg_in           : std_logic_vector(c_words_per_entry * t_wishbone_data'length-1 downto 0);
   signal r_sin_cnt           : unsigned(3 downto 0);
   signal r_msg_cnt_in        : std_logic_vector(31 downto 0);
   
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
   

component packet_capture is
generic(g_filename : string := "123.pcap"; g_buffersize : natural := 1600);
port(
   clk_i          : in   std_logic;                                        --clock
   rst_n_i        : in   std_logic;
   sys_time_i     : in std_logic_vector(63 downto 0);
   rec_i          : in  std_logic;
   snk_i          : in  t_wrf_sink_in;
   snk_o          : out t_wrf_sink_out
   
);   
end component;

component noise_gen is
	Generic (
				W : integer := 16;					-- LFSR scaleable from 24 down to 4 bits
				V : integer := 18;					-- LFSR for non uniform clocking scalable from 24 down to 18 bit
				g_type : integer := 0;			-- gausian distribution type, 0 = unimodal, 1 = bimodal, from g_noise_out
				u_type : integer := 1			-- uniform distribution type, 0 = uniform, 1 =  ave-uniform, from u_noise_out
			);
    Port ( 
			clk 						: 		in  STD_LOGIC;
			n_reset 			: 		in  STD_LOGIC;
			enable				: 		in  STD_LOGIC;
			g_noise_out 	:		out STD_LOGIC_VECTOR (W-1 downto 0);					-- port for bimodal/unimodal gaussian distributions
			u_noise_out 	: 		out  STD_LOGIC_VECTOR (W-1 downto 0)						-- port for uniform/ave-uniform distributions
			);
end component;

   

begin
  
     
  
   wb_heap : ftm_priority_queue
   generic map(
      g_idx_width    => c_width,
      g_key_width    => c_val_width,
      g_val_width    => c_key_width
       
   )           
   port map(
      clk_sys_i   => clk_sys,
      rst_n_i     => rst_n,

      time_sys_i  => r_sys_time,

      ctrl_i      => s_ctrl_i,
      ctrl_o      => open,
      
      snk_i       => s_snk_i,
      snk_o       => open,
      
      src_o       => s_src_o,
      src_i       => s_src_i
     
   );
 
   
   eb : eb_master_slave_wrapper
    generic map(
      g_with_master     => true,
      g_ebs_sdb_address => (others => '0'),
      g_ebm_adr_bits_hi => 10)
    port map(
      clk_i           => clk_sys,
      nRst_i          => rst_n,
      snk_i           => s_fsrc_o,
      snk_o           => s_fsrc_i,
      src_o           => s_fsrc_o,
      src_i           => s_fsrc_i,
      ebs_cfg_slave_o => open,
      ebs_cfg_slave_i => c_dummy_slave_in,
      ebs_wb_master_o => s_ebs_wbm_o,
      ebs_wb_master_i => s_ebs_wbm_i,
      ebm_wb_slave_i  => s_src_o,
      ebm_wb_slave_o  => s_src_i);
    
    
    
dump : packet_capture 
generic map(g_filename   => "prio_queue.pcap", 
            g_buffersize => 1600)
port map(
   clk_i          => clk_sys,
   rst_n_i        => rst_n,
   sys_time_i     => r_sys_time,
   rec_i          => s_rec,
   snk_i          => s_fsrc_o,
   snk_o          => s_fsrc_i
   
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
      if(rst_n = '0') then
         r_sys_time <= (others => '0');
      else
         r_sys_time <= std_logic_vector(unsigned(r_sys_time) +2);
      end if;
   end if;  
   end process;  
    
  
    
  
   data_in : process(clk_sys)
   variable v_dat  : t_wishbone_data;
   variable v_en   : std_logic;
   variable v_cnt, i  : natural;
   variable v_entry_max, v_entry, v_arrival : natural; 
begin
   if rising_edge(clk_sys) then
      if(rst_n = '0') then
         r_sin_cnt      <= (others => '0');
         r_push         <= '0';
         r_sreg_in      <= (others => '0');
         r_msg_cnt_in   <= (others => '0');
         v_entry_max := 0;
      else
         -- short names 
         v_dat := s_ebs_wbm_o.dat;
         v_en  := s_ebs_wbm_o.cyc and s_ebs_wbm_o.stb and not s_ebs_wbm_i.stall;
         v_cnt := to_integer(unsigned(r_msg_cnt_in));
         
         --interface outputs
         s_ebs_wbm_i.stall  <= '0';
         s_ebs_wbm_i.ack        <= '0';
         s_ebs_wbm_i.err        <= '0';
         s_ebs_wbm_i.dat   <= x"CAFEBABE";
         r_push               <= '0';
         
         if(r_push = '1') then
           r_verify_array(v_cnt) <= r_sreg_in;
           r_time_array(v_cnt)   <= r_sys_time;
           r_msg_cnt_in          <= std_logic_vector(unsigned(r_msg_cnt_in) +1);
           i := v_cnt;
           testme <= r_sreg_in(t_skey'range);
           v_entry     := to_integer(unsigned(r_sreg_in(t_skey'range)));
           v_arrival   := to_integer( unsigned(r_sys_time) + to_unsigned(c_test_trn, t_key'length) ); 
           --if( v_entry >= v_arrival ) then
              --error, this is too late!
              --report "ENTRY " & integer'image(i) & " DUE @ " & integer'image(v_entry) & "arrived @ " & integer'image(v_arrival ) severity failure;  
           --else
              --print time left
              --report "ENTRY " & integer'image(i) & " time left to exec: " & integer'image(v_entry-v_arrival) severity note;
           --end if;
           
           if( v_entry < v_entry_max) then
              --error, wrong order!
              report "ENTRY " & integer'image(i) & " DUE @ " & integer'image(v_entry) & " is lower than one of its predecessors! " severity failure;
           else
              v_entry_max := v_entry;
              report "Max: " & integer'image(v_entry_max) severity note; 
           end if; 
          end if;            
         
                
              
            
         --else
            if(v_en = '1') then
               s_ebs_wbm_i.ack  <= '1'; 
               if(c_words_per_entry > 1) then
                     r_sreg_in            <= r_sreg_in(r_sreg_in'left - t_wishbone_data'length downto 0) & v_dat;
                  else
                     r_sreg_in            <= f_wb_wr(r_sreg_in, v_dat, x"f", "owr");
                  end if;
                  
                  if(r_sin_cnt = c_words_per_entry-1) then
                     r_sin_cnt            <= (others => '0');
                     s_ebs_wbm_i.stall    <= '1';
                     r_push               <= '1';
                      
                  else
                     r_sin_cnt            <= r_sin_cnt +1;
                  end if;
               end if; -- en
         --end if; -- verify
      end if; -- rst       
   end if; -- clk edge
end process;
  
   
    -- Stimulus process
  rst: process
  begin        
        
        rst_n  <= '0';
        wait until rising_edge(clk_sys);
        wait for clk_period*5;
        rst_n <= '1';
        wait for clk_period*10;
        wait until rst_n = '0';
   end process;
   

   
    -- Stimulus process
  stim_WB_proc: process
  variable i, j : natural;
  variable v_key  : std_logic_vector(c_key_width-1 downto 0);
  variable v_val  : std_logic_vector(c_val_width-1 downto 0);
  variable v_data : std_logic_vector(c_words_per_entry * t_wishbone_data'length-1 downto 0);
  variable v_cfg  : t_wishbone_data; 
   begin        
        
        i := c_test_msgs;
        s_ctrl_i <= ('0', '0', x"00000000", x"F", '0', x"00000000"); 
        s_snk_i  <= ('0', '0', x"00000000", x"F", '0', x"00000000"); 
        wait until rst_n = '1';
        wait until rising_edge(clk_sys);
        wait for clk_period*5; 
        
        v_cfg := (others => '0');
        v_cfg(c_CFG_BIT_ENA)              := '1';
        v_cfg(c_CFG_BIT_FIFO)             := '1'; 
        
        v_cfg(c_CFG_BIT_AUTOPOP)          := '1';
        v_cfg(c_CFG_BIT_AUTOFLUSH_MSGS)   := '1';
       
        s_ctrl_i <= ('1', '1', std_logic_vector(to_unsigned(c_DST_ADR, 32)), x"F", '1', x"7FFFFFF0"); -- set dst adr
        wait for clk_period;
        s_ctrl_i <= ('1', '1', std_logic_vector(to_unsigned(c_MSG_MAX, 32)), x"F", '1', x"00000005"); -- -- set auto_flush max msgs
        wait for clk_period; 
        s_ctrl_i <= ('1', '1', std_logic_vector(to_unsigned(c_CFG_SET, 32)), x"F", '1', v_cfg); -- set cfg enable, fifo mode and auto_flush msgs
        wait for clk_period;
        s_ctrl_i <= ('1', '1', std_logic_vector(to_unsigned(c_T_DUE_HI, 32)), x"F", '1', x"00000000"); -- set auto_pop duetime HI
        wait for clk_period;
        s_ctrl_i <= ('1', '1', std_logic_vector(to_unsigned(c_T_DUE_LO, 32)), x"F", '1', std_logic_vector(to_unsigned(c_test_due, 32))); -- set auto_pop duetime LO
        wait for clk_period;
        s_ctrl_i <= ('1', '1', std_logic_vector(to_unsigned(c_T_TRN_HI, 32)), x"F", '1', x"00000000"); -- set auto_flush trn time HI
        wait for clk_period;
        s_ctrl_i <= ('1', '1', std_logic_vector(to_unsigned(c_T_TRN_LO, 32)), x"F", '1', std_logic_vector(to_unsigned(c_test_trn, 32))); -- -- set auto_flush trn time LO
         s_rec <= '1';
        wait for clk_period; 
        wait for clk_period;
        s_ctrl_i <= ('0', '0', x"00000000", x"F", '0', x"00000000"); 
        wait for clk_period*1;
        
        report "+++++++++++++++ +++++++++++++++ +++++++++++++ Start INSERT" severity warning;
        
        while (i > 0) 
        loop
        
        --v_key := std_logic_vector(to_unsigned(i*2, v_key'length));
        v_key := std_logic_vector(to_unsigned( to_integer(unsigned(r_sys_time)) + c_test_trn + c_test_margin + (i*100), t_key'length));
        v_val := x"111111112222222233333333444444445555555566666666";
        v_data := v_key & v_val & std_logic_vector( to_unsigned(0, (c_words_per_entry * t_wishbone_data'length - c_data_width)));
        
        for j in 0 to c_words_per_entry-1 loop 
           s_snk_i <= ('1', '1', x"00000000", x"F", '1', v_data(v_data'left - j*t_wishbone_data'length downto  v_data'length - (j+1)*t_wishbone_data'length));
           wait for clk_period*1;
        end loop;
        s_snk_i  <= ('0', '0', x"00000000", x"F", '0', x"00000000"); 
        wait for clk_period*1;
        wait for clk_period*30;
        i := i -1;
        --s_ctrl_i <= ('1', '1', x"00000008", x"F", '1', x"00000001"); -- dbg set
        --wait for clk_period;
        --s_ctrl_i <= ('0', '0', x"00000000", x"F", '0', x"00000000"); 
        --wait for clk_period*150;
        
        end loop;
          
        
        wait for clk_period*100;
       
        wait for clk_period*2;
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
        
        wait for clk_period;
        wait for clk_period*700;
        
        --i := c_test_msgs;
--        report "+++++++++++++++ +++++++++++++++ +++++++++++++ Start REMOVE" severity warning;
--        while (i > 0) 
--        loop
--        
--           s_ctrl_i <= ('1', '1', x"00000004", x"F", '1', x"00000001"); -- force-pop-flush
--           wait for clk_period;
--           s_ctrl_i <= ('0', '0', x"00000000", x"F", '0', x"00000000"); 
--           wait for clk_period*200;
--           i := i -1;
--           --wait until s_busy = '0';
--           --s_ctrl_i <= ('1', '1', x"00000008", x"F", '1', x"00000001"); -- dbg set
--          -- wait for clk_period;
--           --s_ctrl_i <= ('0', '0', x"00000000", x"F", '0', x"00000000"); 
--           --wwait until s_busy = '0';
--        end loop;
         --s_ctrl_i <= ('1', '1', x"00000004", x"F", '1', x"00000003"); -- force-pop-flush
           wait for clk_period;
           s_ctrl_i <= ('0', '0', x"00000000", x"F", '0', x"00000000"); 
           --wait until s_busy = '0';

           --s_ctrl_i <= ('1', '1', x"00000008", x"F", '1', x"00000001"); -- dbg set
           --wait for clk_period;
           --s_ctrl_i <= ('0', '0', x"00000000", x"F", '0', x"00000000"); 
           --wait until s_busy = '0';
        s_ctrl_i <= ('0', '0', x"00000000", x"F", '0', x"00000000");    
         
        --s_rec <= '0';
         
        wait until rst_n = '0';
  end process;

   
   
   



end rtl;  
