
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


library work;
use work.genram_pkg.all;
use work.wishbone_pkg.all;
use work.gencores_pkg.all;
use work.heap_pkg.all;

entity xwb_heap is
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

   ctrl_i      : t_wishbone_slave_in;
   ctrl_o      : t_wishbone_slave_out;
   
   snk_i       : t_wishbone_slave_in
   snk_o       : t_wishbone_slave_out;
   
   src_o       : t_wishbone_master_out;
   src_i       : t_wishbone_master_in
  
);


constant c_dummy_data_out : std_logic_vector(32*4-1 downto 0) := x"DEADBEEF1234CAFEBABE5678FEEDDEAL"
    
-------------------------------------------------------------------------
--memory map for ctrl wb
-------------------------------------------------------------------------
constant c_RST        : natural := 0;             --wo
constant c_FORCE_POP  : natural := c_RST+4;    --wo, b0 
constant c_CLEAR      : natural := c_FORCE_POP+4; 
constant c_T_TRN_HI   : natural := c_CLEAR+4;    --wo, b0
constant c_T_TRN_LO   : natural := c_T_TRN_HI+4;
constant c_MSG_MIN    : natural := c_T_TRN_LO+4;
constant c_MSG_MAX    : natural := c_MSG_MIN+4;
constant c_MSG_CNT    : natural := c_MSG_MAX+4;
constant c_CFG_GET    : natural := c_MSG_CNT+4; --rw, b5 irq, b4 care for T_Trn, b3 care for msg cnt, b2 chk time before insert,  b1 auto-pop, b0 enable, 
constant c_CFG_SET    : natural := c_CFG_GET+4;
constant c_CFG_CLR    : natural := c_CFG_CLR+4;
constant c_ENA_GET    : natural := c_CFG_CLR+4;     --ro, queue enable status (1 bit per queue) default to enabled
constant c_ENA_SET    : natural := c_ENA_GET+4;   --wo, queue enable set
constant c_ENA_CLR    : natural := c_ENA_SET+4;   --wo, queue enabl
constant c_HEAP_CNT   : natural := c_ENA_CLR+4;
----------------------------------------------------------------------



-----------------------------------------------------------------------
   constant c_entry_width     : natural := (g_val_width + g_key_width);
   constant c_words_per_entry : natural := (c_entry_width + t_wishbone_data'length -1) / t_wishbone_data'length;

   signal s_heap_cnt : std_logic_vector(g_idx_width-1 downto 0);
   signal s_msg_cnt : std_logic_vector(5 downto 0);
   signal r_ctrl_out : t_wishbone_slave_out;

--registers
   signal r_force_pop   : std_logic_vector(0 downto 0);
   signal r_clear       : std_logic_vector(0 downto 0);
   signal r_rst         : std_logic_vector(0 downto 0);
   signal r_enable      : std_logic_vector(0 downto 0);
   signal r_cfg         : std_logic_vector(5 downto 0);
   signal r_msg_min     : std_logic_vector(5 downto 0);
   signal r_msg_max     : std_logic_vector(5 downto 0);
   signal r_t_trn_hi    : std_logic_vector(31 downto 0);
   signal r_t_trn_lo    : std_logic_vector(31 downto 0);

   signal r_sreg_in     : std_logic_vector(c_words_per_entry * t_wishbone_data'length-1 downto 0);
   signal r_sreg_out    : std_logic_vector(c_words_per_entry * t_wishbone_data'length-1 downto 0);
   signal s_data_in     : std_logic_vector(c_entry_width -1 downto 0);
   signal s_data_out    : std_logic_vector(c_entry_width -1 downto 0);

ctrl_if : process(clk_sys_i)
   variable v_dat_i  : t_wishbone_data;
   variable v_dat_o  : t_wishbone_data;
   variable v_adr    : t_wishbone_address; 
   variable v_sel    : t_wishbone_select;
   variable v_we     : std_logic;
   variable v_en     : std_logic; 
begin
   if rising_edge(clk_i) then
      if(rst_n_i = '0' or r_rst = "1") then
         r_clr <= (others => '0');
         r_rst <= (others => '0');
         r_ena <= (others => '1');
         r_cfg <= (others => '0');
      else
         -- short names 
         v_dat   := ctrl_i.dat;
         v_adr   := ctrl_i.adr;
         v_sel   := ctrl_i.sel;
         v_en    := ctrl_i.cyc and ctrl_i.stb and not r_ctrl_out.stall;

         --interface outputs
         r_ctrl_out.stall  <= '0';
         r_ctrl_out.ack    <= '0';
         r_ctrl_out.err    <= '0';
         r_ctrl_out.dat    <= (others => '0');

         --fire and forget regs
         r_clr <= (others => '0');
         r_rst <= (others => '0');
         r_force_pop <= (others => '0');
         
         if(v_en = '1') then
            r_ctrl_out.ack <= '1'; -- ack is default, we'll change it if an error occurs
            -- control registers
            if(v_we = '1') then              
               case to_integer(unsigned(v_adr)) is
                  when c_RST        => r_rst             <= f_wb_wr(r_rst,       v_dat_i, v_sel, "set");
                  when c_FORCE_POP  => r_force_pop       <= f_wb_wr(r_force_pop, v_dat_i, v_sel, "set"); 
                  when c_CLEAR      => r_clear           <= f_wb_wr(r_clear,     v_dat_i, v_sel, "set");                    
                  when c_ENA_SET    => r_ena             <= f_wb_wr(r_ena,       v_dat_i, v_sel, "set");
                  when c_ENA_CLR    => r_ena             <= f_wb_wr(r_ena,       v_dat_i, v_sel, "clr");
                  when c_CFG_SET    => r_cfg             <= f_wb_wr(r_cfg,       v_dat_i, v_sel, "set");
                  when c_CFG_CLR    => r_cfg             <= f_wb_wr(r_cfg,       v_dat_i, v_sel, "clr");
                  if(g_is_ftm) then
                  when c_T_TRN_HI   => r_t_trn_hi        <= f_wb_wr(r_t_trn_hi,  v_dat_i, v_sel, "owr");
                  when c_T_TRN_LO   => r_t_trn_lo        <= f_wb_wr(r_t_trn_lo,  v_dat_i, v_sel, "owr");
                  when c_MSG_MIN    => r_msg_min         <= f_wb_wr(r_msg_min,   v_dat_i, v_sel, "owr");
                  when c_MSG_MAX    => r_msg_max         <= f_wb_wr(r_msg_max,   v_dat_i, v_sel, "owr");
                  end if;
                  when others => r_ctrl_out.ack  <= '0'; r_ctrl_out.err <= '1';
               end case;
            else
               case to_integer(unsigned(v_adr)) is
                  when c_ENA_GET    => r_ctrl_out.dat(r_ena'range)      <= r_ena;
                  when c_CFG_GET    => r_ctrl_out.dat(r_cfg'range)      <= r_cfg;
                  if(g_is_ftm) then
                  when c_T_TRN_HI   => r_ctrl_out.dat(r_t_trn_hi'range) <= r_t_trn_hi;
                  when c_T_TRN_LO   => r_ctrl_out.dat(r_t_trn_lo'range) <= r_t_trn_lo;
                  when c_MSG_MIN    => r_ctrl_out.dat(r_msg_min'range)  <= r_msg_min;
                  when c_MSG_MAX    => r_ctrl_out.dat(r_msg_max'range)  <= r_msg_max;
                  when c_MSG_CNT    => r_ctrl_out.dat(s_msg_cnt'range)  <= s_msg_cnt;
                  end if;
                  when c_HEAP_CNT   => r_ctrl_out.dat(s_heap_cnt'range) <= s_heap_cnt;
                  when others => r_ctrl_out.ack <= '0'; r_ctrl_out.err <= '1';
               end case;
            end if; --we      
         end if; -- en
      end if; -- rst       
   end if; -- clk edge
end process;

-- *** INPUT DATA INTERFACE *** ---
r_snk_out.stall <= r_snk_out_fsm_stall or s_heap_full;

s_data_in <= r_sreg_in(r_sreg_in'left downto r_sreg_in'length-c_entry_width);
   
data_in : process(clk_sys_i)
   variable v_dat_i  : t_wishbone_data;
   variable v_dat_o  : t_wishbone_data;
   variable v_adr    : t_wishbone_address; 
   variable v_sel    : t_wishbone_select;
   variable v_we     : std_logic;
   variable v_en     : std_logic; 
begin
   if rising_edge(clk_i) then
      if(rst_n_i = '0' or r_rst = "1") then
         r_sin_cnt   <= (others => '0');
         r_push      <= '0';
         r_sel  <= (others => '0');
      else
         -- short names 
         v_dat   := snk_i.dat;
         v_adr   := snk_i.adr;
         v_sel   := snk_i.sel;
         v_en    := snk_i.cyc and snk_i.stb and not r_snk_out.stall;

         --interface outputs
         r_snk_out_fsm_stall  <= '0';
         r_snk_out.ack        <= '0';
         r_snk_out.err        <= '0';
         r_snk_out.dat        <= (others => '0');
         
         r_push <= '0';
         
         if(v_en = '1') then
            if(v_we = '1') then              
               if((v_sel or r_sin_sel) = "1111") then
                  r_sin_sel            <= (others => '0');
                  r_sin_dat            <= (others => '0');
                  if(c_words_per_entry > 1) then
                     r_sreg_in            <= r_sreg_in(r_sreg_in'left - t_wishbone_data'length downto 0) & (v_dat_i or r_sin_dat);
                  else
                     r_sreg_in            <= f_wb_wr(r_sreg_in, v_dat_i, v_sel, "owr");
                  end if;
                  
                  if(r_sin_cnt = c_words_per_entry-1) then
                     r_sin_cnt            <= (others => '0');
                     r_snk_out_fsm_stall  <= '1';
                     r_push               <= '1';
                  else
                     r_sin_cnt            <= r_sin_cnt +1;
                  end if;
               else
                  r_sin_sel <= r_sin_sel or v_sel;
                  r_sin_dat <= r_sin_dat or v_dat_i;
               end if;
               r_snk_out.ack  <= '1';         
            else
               r_snk_out.err  <= '1'; 
            end if; --we      
         end if; -- en
      end if; -- rst       
   end if; -- clk edge
end process;


-- *** OUTPUT DATA INTERFACE *** ---
s_pop <= '0' when r_pop_req = 0
    else '1';

s_data_out <= c_dummy_data_out(c_dummy_data_out'left downto c_dummy_data_out-s_data_out'length);
    
src_o.dat <= r_sreg_out(r_sreg_out'left downto r_sreg_out'length-t_wishbone_data'length);

r_sreg_out(r_sreg_out'left downto r_sreg_out'length - s_data_out'length) <= s_data_out;
if(r_sreg_out'length != s_data_out'length) generate 
   r_sreg_out(s_data_out'left downto 0) <= std_logic_vector(to_unsigned(0, r_sreg_out'length - s_data_out'length));
end generate;

data_out : process(clk_sys_i)
   variable v_rdy : std_logic;
begin
   if rising_edge(clk_i) then
      if(rst_n_i = '0' or r_rst = "1") then
         r_sout_cnt     <= (others => '0');
         r_shifting_out <= '0';
      else
         v_rdy := s_data_out_rdy or r_data_out_rdy;
         r_data_out_rdy <= v_rdy; 
         r_pop_req_dec  <= 0;
         
         if(r_shifting_out = '1') then
            src_o.cyc <= '1';
            src_o.stb <= '1';
            if(src_i.stall = '0') then
               if(c_words_per_entry > 1) then
                  r_sreg_out <= r_sreg_out(r_sreg_out'left - t_wishbone_data'length downto 0) & x"00000000";
               end if;
               if(r_sout_cnt = c_words_per_entry-1)
                  r_sout_cnt <= (others => '0');
                  r_shifting_out <= '0';
               else
                  r_sout_cnt <= r_sout_cnt +1
               end if;
            end if;   
         else
            if((s_pop and v_rdy)= '1') then
               r_data_out_rdy <= s_data_out_rdy;
               r_pop_req_dec  <= -1;
               r_shifting_out <= '1';
            end if; -- en
         end if;
      end if; -- rst       
   end if; -- clk edge
end process;

pop_request : process(clk_sys_i)
   variable v_rdy : std_logic;
begin
   if rising_edge(clk_i) then
      if(rst_n_i = '0' or r_rst = "1") then
         r_pop_req     <= (others => '0');
      else
         v_pop_inc := 0;
         if(r_pop = "1" or r_auto_pop = '1') then
            v_pop_inc := 1;
         end if;
         r_pop_req <= r_pop_req + r_pop_req_dec + r_pop_req_inc;
      end if; -- rst       
   end if; -- clk edge
end process;

if(g_is_ftm) generate
   auto_pop : process(clk_sys_i)
      variable v_rdy : std_logic;
   begin
      if rising_edge(clk_i) then
         if(rst_n_i = '0' or r_rst = "1") then
            r_pop_req     <= (others => '0');
         else
            v_pop_inc := 0;
            if(r_pop = "1" or r_auto_pop = '1') then
               v_pop_inc := 1;
            end if;
            r_pop_req <= r_pop_req + r_pop_req_dec + r_pop_req_inc;
         end if; -- rst       
      end if; -- clk edge
   end process;
end generate;
  
end architecture behavioral;      



    
