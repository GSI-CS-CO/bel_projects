--! @file ftm_priority_queue.vhd
--! @brief WB interface for generic heap, also adds optional priority queue features for FAIR Timing master
--! @author Mathias Kreider <m.kreider@gsi.de>
--!
--! Copyright (C) 2013 GSI Helmholtz Centre for Heavy Ion Research GmbH 
--!
--! Provides control interface and generic width serial Wishbone data interface for heap
--! Register map:
--!
--! 0x00  RST        wo, reset
--! 0x04  FORCE_POP  wo, pops one element from the heap if heap is not empty
--! 0x08  DEBUG_GET  ro, shows heap consistency check result
--! 0x0C  DEBUG_SET  wo, start heap consistency check
--! 0x10  CLEAR      wo, clears heap
--! 0x14  T_TRN_HI   rw, transmission time window in 8ns, high word. Used for autopop
--! 0x18  T_TRN_LO   rw, transmission time window in 8ns, low word
--! 0x1C  CAPACITY   ro, total heap capacity
--! 0x20  MSG_MAX    rw, maximum number of elements to dequeue before send & wait for EBM readiness
--! 0x24  MSG_CNT    ro, number of elements already popped to EBM before 'send' was issued
--! 0x28  CFG_GET    ro, get    cfg bits. b5 irq, b4 care for T_Trn, b3 care for msg cnt, b2 chk time before insert,  b1 auto-pop 
--! 0x2C  CFG_SET    wo, set    cfg bits
--! 0x30  CFG_CLR    wo, clear  cfg bits
--! 0x34  ENA_GET    ro, get    enable bit. ENA sets wether heap will accept queue/dequeue commands
--! 0x38  ENA_SET    wo, set    enable bit
--! 0x3C  ENA_CLR    wo, clear  enable bit
--! 0x40  HEAP_CNT   ro, current number of elements on the heap
--! 0x44  EBM_ADR    rw, wishbone address of the Etherbone Master
--! 
--! Also provides optional time/packet load based auto dequeue features for FAIR Timing master
--! 
--------------------------------------------------------------------------------
--! This library is free software; you can redistribute it and/or
--! modify it under the terms of the GNU Lesser General Public
--! License as published by the Free Software Foundation; either
--! version 3 of the License, or (at your option) any later version.
--!
--! This library is distributed in the hope that it will be useful,
--! but WITHOUT ANY WARRANTY; without even the implied warranty of
--! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
--! Lesser General Public License for more details.
--!  
--! You should have received a copy of the GNU Lesser General Public
--! License along with this library. If not, see <http://www.gnu.org/licenses/>.
---------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


library work;
use work.genram_pkg.all;
use work.wishbone_pkg.all;
use work.gencores_pkg.all;
use work.heap_pkg.all;

entity ftm_priority_queue is
generic(
   g_is_ftm       : boolean := false;  
   g_idx_width    : natural := 8;
   g_key_width    : natural := 64;
   g_val_width    : natural := 192  
);            
port(
   clk_sys_i   : in  std_logic;
   rst_n_i     : in  std_logic;

   time_sys_i  : in  std_logic_vector(63 downto 0) := (others => '1');

   ctrl_i      : in  t_wishbone_slave_in;
   ctrl_o      : out t_wishbone_slave_out;
   
   snk_i       : in  t_wishbone_slave_in;
   snk_o       : out t_wishbone_slave_out;
   
   src_o       : out t_wishbone_master_out;
   src_i       : in  t_wishbone_master_in
  
);
end ftm_priority_queue;

architecture behavioral of ftm_priority_queue is

--**************************************************************************--
-- Constants
------------------------------------------------------------------------------

-- memory map for ctrl wb
     
   type t_state is (e_IDLE, e_SET_ADHI_TLU, e_DATA0_TLU, e_DATA1_TLU, e_SET_ADHI_ECA, e_DATA_ECA, e_END_CYC_WAIT, e_EBM_FLUSH, e_ERROR); 
  
   constant c_RST        : natural := 0;                 --00 wo, reset. writing 1 will reset the core
   constant c_FORCE      : natural := c_RST        +4;   --04 wo, pops one element from the heap if heap is not empty
   constant c_DBG_SET    : natural := c_FORCE      +4;   --08 wo, start heap consistency check when writing 1
   constant c_DBG_GET    : natural := c_DBG_SET    +4;   --0c ro, shows heap consistency check result 
   constant c_CLEAR      : natural := c_DBG_GET    +4;   --10 wo, clear heap on writing 1 
   constant c_CFG_GET    : natural := c_CLEAR      +4;   --14 ro, get   cfg bits, see below 
   constant c_CFG_SET    : natural := c_CFG_GET    +4;   --18 wo, set   cfg bits
   constant c_CFG_CLR    : natural := c_CFG_SET    +4;   --1c wo, clear cfg bits
   constant c_DST_ADR    : natural := c_CFG_CLR    +4;   --20 rw, wishbone address of src output
   constant c_HEAP_CNT   : natural := c_DST_ADR    +4;   --24 ro, current number of elements on the heap
   constant c_MSG_CNT_O  : natural := c_HEAP_CNT   +4;   --28 ro, total number of elements dequeued since last clear/reset
   constant c_MSG_CNT_I  : natural := c_MSG_CNT_O  +4;   --2c ro, total number of elements queued since last clear/reset
   
   constant c_T_TRN_HI   : natural := c_MSG_CNT_I  +4;   --30 rw, transmission time window in 8ns, high word. Used for autopop
   constant c_T_TRN_LO   : natural := c_T_TRN_HI   +4;   --34 rw, transmission time window in 8ns, low word
   constant c_T_DUE_HI   : natural := c_T_TRN_LO   +4;   --38 rw, transmission time window in 8ns, high word. Used for autopop
   constant c_T_DUE_LO   : natural := c_T_DUE_HI   +4;   --3C rw, transmission time window in 8ns, low word
   
   constant c_CAPACITY   : natural := c_T_DUE_LO   +4;   --40 ro, total heap capacity
   constant c_MSG_MAX    : natural := c_CAPACITY   +4;   --44 rw, maximum number of elements to dequeue before send & wait for EBM readiness. used for autopop
   constant c_EBM_ADR    : natural := c_MSG_MAX    +4;   --48 rw, wishbone address of the Etherbone Master
   constant c_TS_ADR     : natural := c_EBM_ADR    +4;   --4C rw, wishbone address of the TLU for capture of msg arrival timestamp. dbg feature
   constant c_TS_CH      : natural := c_TS_ADR     +4;   --50 rw, channel number at the TLU for capture of msg arrival timestamp. dbg feature
   
-- cfg reg bits
   constant c_CFG_BIT_ENA              : natural := 0;
   constant c_CFG_BIT_FIFO             : natural := 1;    
   constant c_CFG_BIT_IRQ              : natural := 2;
   constant c_CFG_BIT_AUTOPOP          : natural := 3;
   constant c_CFG_BIT_AUTOFLUSH_TIME   : natural := 4;
   constant c_CFG_BIT_AUTOFLUSH_MSGS   : natural := 5;
   constant c_CFG_BIT_MSG_ARR_TS       : natural := 6;
   constant c_CFG_BITS                 : natural := c_CFG_BIT_MSG_ARR_TS;


-- force reg bits
   constant c_FORCE_BIT_POP            : natural := 0;
   constant c_FORCE_BIT_FLUSH          : natural := 1;  

-- io if width constants   
   constant c_entry_width              : natural := (g_val_width + g_key_width);
   constant c_words_per_entry          : natural := (c_entry_width + t_wishbone_data'length -1) / t_wishbone_data'length;
   constant c_ebm_adr_bits_hi          : natural := 10;
   constant c_dat_bit                  : natural := t_wishbone_address'left - c_ebm_adr_bits_hi +2;
   constant c_rw_bit                   : natural := t_wishbone_address'left - c_ebm_adr_bits_hi +1;
   constant c_msg_n_max                : natural := (1500 - 20 - 8 - 4) / ((8 + 2) * 4); -- (max eth length - ip - udp - eb) / (eb rec + eb wr adr + 8 words playload) 
   
     -- ebm if constants for FAIR Timing master
   constant c_EBM_ADR_CFG_BASE : unsigned(31 downto 0) := x"00000000";
   constant c_EBM_DAT_OFFS     : unsigned(31 downto 0) := to_unsigned(2**c_dat_bit, t_wishbone_address'length);
   constant c_EBM_RW_OFFS      : unsigned(31 downto 0) := to_unsigned(2**c_rw_bit, t_wishbone_address'length);
   --constant c_ebm_msk        : unsigned(31 downto 0) := not std_logic_vector(to_unsigned(2**(32-g_adr_bits_hi+1)-1, 32));
   constant c_EBM_ADR_FLUSH    : unsigned(31 downto 0) := x"00000004";
   constant c_EBM_ADR_OPA_HI   : unsigned(31 downto 0) := x"00000030";
   constant c_EBM_ADR_CNT      : unsigned(31 downto 0) := x"00000008";
   constant c_EBM_ADR_MSK      : unsigned(31 downto 0) := to_unsigned(0, c_ebm_adr_bits_hi) & unsigned(to_signed(-1, t_wishbone_address'length-c_ebm_adr_bits_hi));
   constant c_ECA_ADR          : unsigned(31 downto 0) := x"7FFFFFFF";
	constant c_TLU_ADR_CHSEL    : unsigned(31 downto 0) := x"00000058";
	constant c_TLU_CH_TEST      : unsigned(31 downto 0) := x"00000060";
------------------------------------------------------------------------------

   attribute syn_keep: boolean;
--**************************************************************************--
-- Signals
------------------------------------------------------------------------------
   signal s_src_o       : t_wishbone_master_out;
   -- heap io   
   signal s_heap_cnt       : std_logic_vector(g_idx_width-1 downto 0);
   signal s_full           : std_logic;
   signal s_empty          : std_logic;
   signal s_busy           : std_logic;
   signal s_pop            : std_logic;
   signal s_push           : std_logic;
   signal s_data_out_rdy, 
          r_data_out_rdy   : std_logic;
   signal s_dbg_stat       : std_logic_vector(1 downto 0);
   signal s_clear_heap     : std_logic;
   signal s_data_in        : std_logic_vector(c_entry_width -1 downto 0);
   signal s_data_out       : std_logic_vector(c_entry_width -1 downto 0);
   
   signal s_send           : std_logic;
   signal s_auto_pop       : std_logic;
	signal s_due            : std_logic;
   attribute syn_keep of s_due: signal is true;
   
   signal s_max_time_reached  : std_logic;
   signal s_max_msg_reached   : std_logic;
   signal s_max_space_reached : std_logic;
   signal s_packet_not_empty  : std_logic;
	signal s_packet_start      : std_logic;
	
   signal s_src_mux           : std_logic;
   signal s_shift_out         : t_wishbone_data;
   ------------------------------------------------------------------------------


--**************************************************************************-- 
-- Registers
-- ctrl regs
------------------------------------------------------------------------------
   signal r_force             : std_logic_vector(1 downto 0);
   signal r_clr               : std_logic_vector(0 downto 0);
   signal r_rst               : std_logic_vector(0 downto 0);
   signal r_dbg               : std_logic_vector(0 downto 0);
   signal r_cfg               : std_logic_vector(c_CFG_BITS downto 0);
   signal r_src_o_adr         : t_wishbone_address;
   signal r_dst_adr           : t_wishbone_address;
	signal r_ts_adr            : t_wishbone_address;
	signal r_ts_ch             : std_logic_vector(4 downto 0);
   signal r_adr_inc           : t_wishbone_address;
   signal r_msg_cnt_out       : std_logic_vector(31 downto 0);
   signal r_msg_cnt_in        : std_logic_vector(31 downto 0);
   signal r_msg_cnt_packet    : std_logic_vector(31 downto 0);
   
   signal c_heap_cap          : std_logic_vector(g_idx_width-1 downto 0) := std_logic_vector(to_unsigned(2**g_idx_width-1, g_idx_width));
   signal r_msg_max           : std_logic_vector(31 downto 0);
   signal r_t_trn             : std_logic_vector(63 downto 0);
   alias  a_t_trn_hi          : std_logic_vector(31 downto 0) is r_t_trn(63 downto 32);
   alias  a_t_trn_lo          : std_logic_vector(31 downto 0) is r_t_trn(31 downto 0);
   signal r_t_due             : std_logic_vector(63 downto 0);
   alias  a_t_due_hi          : std_logic_vector(31 downto 0) is r_t_due(63 downto 32);
   alias  a_t_due_lo          : std_logic_vector(31 downto 0) is r_t_due(31 downto 0);
   signal r_ebm_adr           : std_logic_vector(31 downto 0);
   signal r_ebm_msk           : std_logic_vector(31 downto 0);
   signal r_ebm_ops           : t_wishbone_data;
  
-- heap wb io
   signal r_sreg_in           : std_logic_vector(c_words_per_entry * t_wishbone_data'length-1 downto 0);
   signal r_sreg_out          : std_logic_vector(c_words_per_entry * t_wishbone_data'length-1 downto 0);
   signal r_reg_out          : std_logic_vector(c_words_per_entry * t_wishbone_data'length-1 downto 0);
   signal r_ctrl_out          : t_wishbone_slave_out;
   signal r_snk_out           : t_wishbone_slave_out;
   signal r_snk_out_fsm_stall : std_logic;
   signal r_push              : std_logic;
   signal r_sin_sel           : t_wishbone_byte_select;
   signal r_sin_dat           : t_wishbone_data;
   signal r_sin_cnt           : unsigned(3 downto 0);
   signal r_sout_cnt          : unsigned(3 downto 0);
   signal r_pop_req           : unsigned(g_idx_width downto 0);
   signal r_pop_req_inc       : unsigned(g_idx_width downto 0);
   signal r_pop_req_dec       : unsigned(g_idx_width downto 0);
   signal r_command_out       : t_wishbone_data;
   signal r_pop_ack            : std_logic;
   signal r_empty            : std_logic;
   signal s_adr               : unsigned(6 downto 0);
   
   attribute syn_keep of s_adr: signal is true;
   
-- autopop
   signal r_state_out,
          r_state_out_next : t_state;
          
   signal r_ackerr_in, r_stb_out : unsigned(3 downto 0);
          
   signal r_send, 
			 r_send0,
			 r_send1             : std_logic;
   
   signal r_src_ack_cnt_clr   : std_logic;
   signal r_src_ack_cnt,
          r_src_push_cnt      : unsigned(5 downto 0);
   
   signal  s_out_key          : std_logic_vector(g_key_width -1 downto 0); 
   attribute syn_keep of s_out_key: signal is true;
   
   signal r_due0,
          r_due1              : std_logic;
   signal s_duetime           : unsigned(g_key_width -1 downto 0);
   
   
   signal r_earliest_key      : std_logic_vector(g_key_width -1 downto 0);
   signal r_packet_not_empty  : std_logic;
   signal s_deadline          : unsigned(g_key_width -1 downto 0);       
   ------------------------------------------------------------------------------
   
begin

--**************************************************************************--
-- Heap Instance
------------------------------------------------------------------------------
   s_clear_heap <= not r_clr(0) and rst_n_i;

   heap : heap_top
   generic map (
      g_idx_width    => g_idx_width,
      g_val_width    => g_val_width,
      g_key_width    => g_key_width 
   )            
   port map(clk_sys_i   => clk_sys_i,
            rst_n_i     => s_clear_heap,
            
            dbg_show_i  => r_dbg(0),
            dbg_ok_o    => s_dbg_stat(0),       
            dbg_err_o   => s_dbg_stat(1),
    
            push_i      => s_push,
            pop_i       => s_pop,
            busy_o      => s_busy,
            full_o      => s_full,
            empty_o     => s_empty,
            count_o     => s_heap_cnt,
           
            data_i     => s_data_in,
            data_o     => s_data_out, 
            out_o      => s_data_out_rdy 
    );
    
    s_out_key <= r_reg_out(g_key_width-1 downto 0);
   ------------------------------------------------------------------------------
   -- this lowers empty flag only after the first element is present on output port
   flag : process(clk_sys_i)
   begin
      if rising_edge(clk_sys_i) then
         if(rst_n_i = '0' or r_rst = "1") then
            r_empty <= '1';
         elsif s_data_out_rdy = '1' then
            r_empty <= s_empty;
         end if; -- rst       
      end if; -- clk edge
   end process;  

--**************************************************************************--
-- Control Registers and Wishbone Slave If
------------------------------------------------------------------------------
   ctrl_o   <= r_ctrl_out;
   
   ctrl_if : process(clk_sys_i)
      variable v_dat_i  : t_wishbone_data;
      variable v_dat_o  : t_wishbone_data;
      variable v_adr    : natural; 
      variable v_sel    : t_wishbone_byte_select;
      variable v_we     : std_logic;
      variable v_en     : std_logic; 
   begin
       if rising_edge(clk_sys_i) then
         if(rst_n_i = '0' or r_rst = "1") then
            r_clr             <= (others => '0');
            r_rst             <= (others => '0');
            r_cfg             <= (others => '0');
            r_force           <= (others => '0');
            r_ctrl_out        <= ('0', '0', '0', '0', '0', x"00000000");
            r_msg_max         <= std_logic_vector(to_unsigned(c_msg_n_max, r_msg_max'length));
            r_t_due           <= (others => '0'); 
            r_t_trn           <= (others => '0');
         else
            -- short names 
            v_dat_i           := ctrl_i.dat;
            v_adr             := to_integer(unsigned(ctrl_i.adr(6 downto 2)) & "00"); 
            v_sel             := ctrl_i.sel;
            v_en              := ctrl_i.cyc and ctrl_i.stb and not r_ctrl_out.stall;
            v_we              := ctrl_i.we; 
            
            --interface outputs
            r_ctrl_out.stall  <= '0';
            r_ctrl_out.ack    <= '0';
            r_ctrl_out.err    <= '0';
            r_ctrl_out.dat    <= (others => '0');

            --fire and forget regs
            r_dbg             <= (others => '0');
            r_clr             <= (others => '0');
            r_rst             <= (others => '0');
            r_force           <= (others => '0');
            
            if(v_en = '1') then
               r_ctrl_out.ack <= '1'; -- ack is default, we'll change it if an error occurs
               -- control registers
               if(v_we = '1') then
                  case v_adr is
                     when c_RST        => r_rst       <= f_wb_wr(r_rst,       v_dat_i, v_sel, "set");
                     when c_FORCE      => r_force     <= f_wb_wr(r_force,     v_dat_i, v_sel, "set");
                     when c_DBG_SET    => r_dbg       <= f_wb_wr(r_dbg,       v_dat_i, v_sel, "set");                           
                     when c_CLEAR      => r_clr       <= f_wb_wr(r_clr,       v_dat_i, v_sel, "set");                    
                     when c_CFG_SET    => r_cfg       <= f_wb_wr(r_cfg,       v_dat_i, v_sel, "set");
                     when c_CFG_CLR    => r_cfg       <= f_wb_wr(r_cfg,       v_dat_i, v_sel, "clr");
                     when c_DST_ADR    => r_dst_adr   <= f_wb_wr(r_dst_adr,   v_dat_i, v_sel, "owr");
                     when c_T_TRN_HI   => a_t_trn_hi  <= f_wb_wr(a_t_trn_hi,  v_dat_i, v_sel, "owr");
                     when c_T_TRN_LO   => a_t_trn_lo  <= f_wb_wr(a_t_trn_lo,  v_dat_i, v_sel, "owr");
                     when c_T_DUE_HI   => a_t_due_hi  <= f_wb_wr(a_t_due_hi,  v_dat_i, v_sel, "owr");
                     when c_T_DUE_LO   => a_t_due_lo  <= f_wb_wr(a_t_due_lo,  v_dat_i, v_sel, "owr");
                     when c_CAPACITY   => c_heap_cap  <= f_wb_wr(c_heap_cap,  v_dat_i, v_sel, "owr");
                     when c_MSG_MAX    => r_msg_max   <= f_wb_wr(r_msg_max,   v_dat_i, v_sel, "owr");
                     when c_EBM_ADR    => r_ebm_adr   <= f_wb_wr(r_ebm_adr,   v_dat_i, v_sel, "owr");
                     when c_TS_ADR     => r_ts_adr    <= f_wb_wr(r_ts_adr,    v_dat_i, v_sel, "owr");
							when c_TS_CH      => r_ts_ch     <= f_wb_wr(r_ts_ch,     v_dat_i, v_sel, "owr");
                     when others => r_ctrl_out.ack  <= '0'; r_ctrl_out.err <= '1';
                  end case;
               else
                  case v_adr is
                     when c_DBG_GET    => r_ctrl_out.dat(s_dbg_stat'range)       <= s_dbg_stat;
                     when c_CFG_GET    => r_ctrl_out.dat(r_cfg'range)            <= r_cfg;
                     when c_DST_ADR    => r_ctrl_out.dat(r_dst_adr'range)        <= r_dst_adr;
                     when c_HEAP_CNT   => r_ctrl_out.dat(s_heap_cnt'range)       <= s_heap_cnt;
                     when c_MSG_CNT_O  => r_ctrl_out.dat(r_msg_cnt_packet'range) <= r_msg_cnt_packet;
                     when c_MSG_CNT_I  => r_ctrl_out.dat(r_msg_cnt_in'range)     <= r_msg_cnt_in;
                     when c_T_TRN_HI   => r_ctrl_out.dat(a_t_trn_hi'range)       <= a_t_trn_hi;
                     when c_T_TRN_LO   => r_ctrl_out.dat(a_t_trn_lo'range)       <= a_t_trn_lo;
                     when c_T_DUE_HI   => r_ctrl_out.dat(a_t_due_hi'range)       <= a_t_due_hi;
                     when c_T_DUE_LO   => r_ctrl_out.dat(a_t_due_lo'range)       <= a_t_due_lo;
                     when c_CAPACITY   => r_ctrl_out.dat(c_heap_cap'range)       <= c_heap_cap;
                     when c_MSG_MAX    => r_ctrl_out.dat(r_msg_max'range)        <= r_msg_max;
                     when c_EBM_ADR    => r_ctrl_out.dat(r_ebm_adr'range)        <= r_ebm_adr;
                     when c_TS_ADR     => r_ctrl_out.dat(r_ts_adr'range)         <= r_ts_adr;
							when c_TS_CH      => r_ctrl_out.dat(r_ts_ch'range)          <= r_ts_ch;
                     when others => r_ctrl_out.ack <= '0'; r_ctrl_out.err <= '1';
                  end case;
               end if; --we      
            end if; -- en
         end if; -- rst       
      end if; -- clk edge
      
   end process;
   ------------------------------------------------------------------------------

   
--**************************************************************************--
-- Input Data Interface
------------------------------------------------------------------------------
   snk_o             <= r_snk_out;
   s_push            <= r_push;
   r_snk_out.stall   <= r_snk_out_fsm_stall or s_full or s_busy or (not snk_i.cyc and s_pop); -- stall when buffer full, heap busy or 
   r_snk_out.dat     <= (others => '0');
   s_data_in         <= r_sreg_in(r_sreg_in'left downto r_sreg_in'length-c_entry_width);
      
   data_in : process(clk_sys_i)
      variable v_dat_i  : t_wishbone_data;
      variable v_dat_o  : t_wishbone_data;
      variable v_adr    : t_wishbone_address; 
      variable v_sel    : t_wishbone_byte_select;
      variable v_we     : std_logic;
      variable v_en     : std_logic; 
   begin
      if rising_edge(clk_sys_i) then
         if(rst_n_i = '0' or r_rst = "1") then
            r_sin_cnt      <= (others => '0');
            r_push         <= '0';
            r_sin_sel      <= (others => '0');
            r_sin_dat      <= (others => '0');
            r_sreg_in      <= (others => '0');
            r_msg_cnt_in   <= (others => '0');
         else
            -- short names 
            v_dat_i := snk_i.dat;
            v_adr   := snk_i.adr;
            v_sel   := snk_i.sel;
            v_en    := snk_i.cyc and snk_i.stb and not r_snk_out.stall;
            v_we    := snk_i.we;
            --interface outputs
            r_snk_out_fsm_stall  <= '0';
            r_snk_out.ack        <= '0';
            r_snk_out.err        <= '0';
            
            r_push <= '0';
            
            if( r_clr = "1" ) then
               r_msg_cnt_in   <= (others => '0');
            elsif( r_push = '1') then
               r_msg_cnt_in         <= std_logic_vector(unsigned(r_msg_cnt_in) +1);     
            end if;
            
            if(v_en = '1') then
               r_snk_out.ack  <= '1'; 
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
               else
                  r_snk_out.err  <= '1'; 
               end if; --we      
            end if; -- en
         end if; -- rst       
      end if; -- clk edge
   end process;
------------------------------------------------------------------------------




--**************************************************************************--
-- Output Data Interface
------------------------------------------------------------------------------
-- FSM für output -> block ebm, check EBM status msg count, set ebm hi adr reg, output element(s), trigger ebm flush if necessary
   src_o       <= s_src_o;

   s_pop       <= '1' when (r_pop_req > 0) and s_busy = '0' and r_cfg(c_CFG_BIT_ENA) = '1' and s_empty = '0' and r_pop_ack = '0'
             else '0';
   s_shift_out <= r_sreg_out(r_sreg_out'left downto r_sreg_out'length-t_wishbone_data'length);

   src_mux : with s_src_mux select
   s_src_o.dat <= s_shift_out when '1',
                  r_command_out when others;

   -- make sure to wait for all acks on src before transfer to ebm is complete
   src_ack_cnt : process(clk_sys_i)
   begin
      if rising_edge(clk_sys_i) then
         if(r_src_ack_cnt_clr = '1' or rst_n_i = '0' or r_rst = "1") then
               r_src_ack_cnt  <= (others => '0');
               r_src_push_cnt <= (others => '0');
         else
            if(src_i.ack = '1') then
               r_src_ack_cnt  <= r_src_ack_cnt  + 1;
            end if;
            
            if((s_src_o.cyc and s_src_o.stb and not src_i.stall) = '1') then
               r_src_push_cnt  <= r_src_push_cnt  + 1;
            end if;
         end if;
      end if;
   end process src_ack_cnt;

   data_out : process(clk_sys_i)
      variable v_rdy : std_logic;
      variable v_state_out : t_state;
      variable v_ackerr_in, v_stb_out : unsigned(r_ackerr_in'range);
   begin
      if rising_edge(clk_sys_i) then
         if(rst_n_i = '0' or r_rst = "1") then
            r_sout_cnt        <= (others => '0');
            s_src_o.adr       <= (others => '0');
            s_src_o.sel       <= (others => '1');
            s_src_o.cyc       <= '0';
            s_src_o.stb       <= '0';
            s_src_o.we        <= '0';
            r_sreg_out(r_sreg_out'left downto r_sreg_out'length - s_data_out'length) <= s_data_out;
            r_pop_req_dec     <= (others => '0');
            r_data_out_rdy    <= '0';
            r_send            <= '0';
            r_state_out       <= e_IDLE;
            r_state_out_next      <= e_IDLE;
            r_ebm_ops         <= (others => '0');
            r_msg_cnt_packet  <= (others => '0');
            r_src_ack_cnt_clr <= '0';
            r_pop_ack         <= '0';

            r_ackerr_in       <= (others => '0');
            r_stb_out         <= (others => '0');
         else
            v_rdy := s_data_out_rdy or r_data_out_rdy;
            r_data_out_rdy    <= v_rdy; 
            r_pop_req_dec     <= (others => '0');
            r_src_ack_cnt_clr <= '0';
            v_state_out       := r_state_out;
            if(s_data_out_rdy = '1') then
               r_reg_out      <= s_data_out;
            end if;
         
            r_send <= (r_send or s_send) and s_packet_not_empty;
            
            
            
            
            v_stb_out   := r_stb_out;
            v_ackerr_in := r_ackerr_in;
            
            if((src_i.ack or src_i.err) = '1') then
               v_ackerr_in := v_ackerr_in +1;   
            end if;
           
            if((s_src_o.cyc and s_src_o.stb and not src_i.stall) = '1') then
               v_stb_out := v_stb_out +1;   
            end if;
           
            r_ackerr_in <= v_ackerr_in;   
            r_stb_out   <= v_stb_out;
            
            
           
            case r_state_out is
               when e_IDLE       => s_src_o.cyc <= '0';
                                    s_src_o.stb <= '0';
                                    r_pop_ack <= '0';
                                    if(r_send = '1') then
                                         s_src_o.adr   <= std_logic_vector(unsigned(r_ebm_adr) + c_EBM_ADR_FLUSH);
                                         r_command_out <= x"00000001";
                                         v_state_out := e_EBM_FLUSH;
                                    elsif((s_pop and v_rdy)= '1') then
                                          r_pop_ack            <= '1';
                                          
                                          if(r_cfg(c_CFG_BIT_MSG_ARR_TS) = '1') then 
                                             v_state_out := e_SET_ADHI_TLU;
                                          else
                                             v_state_out := e_SET_ADHI_ECA;
                                          end if;
                                    end if;
               
               when e_SET_ADHI_TLU  => s_src_o.cyc       <= '1';
                                       s_src_o.stb       <= '1';
                                       s_src_o.adr       <= std_logic_vector(unsigned(r_ebm_adr) + c_EBM_ADR_OPA_HI);
                                       r_command_out     <= std_logic_vector(r_ts_adr);
                                       r_state_out_next  <= e_DATA0_TLU;
                                       v_state_out       := e_END_CYC_WAIT;
                                       
               when e_DATA0_TLU     => s_src_o.cyc <= '1';
                                       s_src_o.stb <= '1';
                                       s_src_o.adr    <= std_logic_vector(unsigned(r_ebm_adr) or c_EBM_DAT_OFFS 
                                                         or c_EBM_RW_OFFS or ((unsigned(r_ts_adr) + c_TLU_ADR_CHSEL) and c_EBM_ADR_MSK));
                                       r_command_out  <= std_logic_vector(to_unsigned(0,(32 - r_ts_ch'length))) & r_ts_ch;
                                       v_state_out    := e_DATA1_TLU;  
               
               when e_DATA1_TLU     => s_src_o.cyc <= '1';
                                       s_src_o.stb <= '1';
                                       if(src_i.stall = '0') then
                                          s_src_o.adr       <= std_logic_vector(unsigned(r_ebm_adr) or c_EBM_DAT_OFFS 
                                                            or c_EBM_RW_OFFS or ((unsigned(r_ts_adr)+c_TLU_CH_TEST) and c_EBM_ADR_MSK));
                                          r_command_out     <= x"ffffffff";      
                                          
                                          r_state_out_next  <= e_SET_ADHI_ECA;
                                          v_state_out       := e_END_CYC_WAIT;
                                       end if;     
               
               when e_SET_ADHI_ECA  => s_src_o.cyc <= '1';
                                       s_src_o.stb <= '1';
                                       
                                       r_sreg_out(r_sreg_out'left downto r_sreg_out'length - s_data_out'length) <= r_reg_out;
                                       if(r_sreg_out'length /= s_data_out'length) then 
                                          r_sreg_out(r_sreg_out'left - s_data_out'length downto 0) 
                                          <= std_logic_vector(to_unsigned(0, r_sreg_out'length - s_data_out'length));
                                       end if;
                                       r_data_out_rdy <= s_data_out_rdy;
                                       r_pop_req_dec  <= to_unsigned(1, r_pop_req_dec'length);
                                       --s_src_o.adr    <= std_logic_vector(unsigned(r_ebm_adr) or c_EBM_DAT_OFFS 
                                       --                  or c_EBM_RW_OFFS or (unsigned(r_dst_adr) and c_EBM_ADR_MSK));
                                       r_state_out_next  <= e_DATA_ECA;
                                       v_state_out       := e_END_CYC_WAIT;
                                       
               when e_DATA_ECA     =>  s_src_o.cyc <= '1';
                                       s_src_o.stb <= '1';
                                       
                                       if((s_src_o.cyc and s_src_o.stb and not src_i.stall) = '1') then
                                          
                                          if(c_words_per_entry > 1) then
                                             r_sreg_out <= r_sreg_out(r_sreg_out'left - t_wishbone_data'length downto 0) & x"00000000";
                                          end if;
                                          if(r_sout_cnt = c_words_per_entry-1) then
                                             r_sout_cnt <= (others => '0');
                                             s_src_o.stb <= '0';
                                             r_msg_cnt_packet <= std_logic_vector(unsigned(r_msg_cnt_packet) +1);
                                             
                                             if(r_send = '1') then
                                               r_state_out_next   <= e_EBM_FLUSH;
                                             else
                                                r_state_out_next  <= e_IDLE;
                                             end if;
                                             v_state_out       := e_END_CYC_WAIT;
                                          else
                                             r_sout_cnt <= r_sout_cnt +1;
                                          end if;
                                       end if;

               when e_EBM_FLUSH    =>  s_src_o.cyc       <= '1';
                                       s_src_o.stb       <= '1';
                                       s_src_o.adr       <= std_logic_vector(unsigned(r_ebm_adr) + c_EBM_ADR_FLUSH);
                                       r_command_out     <= x"00000001";
                                       r_state_out_next  <= e_IDLE;
                                       v_state_out       := e_END_CYC_WAIT;
               
               
               when e_END_CYC_WAIT  => if(src_i.stall = '0') then
                                         s_src_o.stb <= '0'; 
                                       end if;
                                       
                                       if(v_stb_out <= v_ackerr_in) then
                                          s_src_o.cyc <= '0';
                                          s_src_o.stb <= '0';
                                          r_ackerr_in <= (others => '0');
                                          r_stb_out   <= (others => '0'); 
                                          v_state_out := r_state_out_next;
                                       end if;
               
               when e_ERROR      => v_state_out := e_IDLE;
                                                         
               when others       => v_state_out := e_ERROR;
            end case;
            
            r_state_out <= v_state_out;
            
            
            if(v_state_out = e_ERROR) then
              s_src_o.we <= '0';
            else
              s_src_o.we <= '1';
            end if;
            
            if(v_state_out = e_DATA_ECA) then
              s_src_mux <= '1';
            else
              s_src_mux <= '0';
            end if;
            
         end if; -- rst       
      end if; -- clk edge
   end process;
------------------------------------------------------------------------------


--**************************************************************************--
-- Keep Track of Pop requests
------------------------------------------------------------------------------
   pop_request : process(clk_sys_i)
      variable v_rdy : std_logic;
      variable v_pop_req_inc : unsigned(g_idx_width downto 0);
   begin
      if rising_edge(clk_sys_i) then
         if(rst_n_i = '0' or r_rst = "1") then
            r_pop_req     <= (others => '0');
            v_pop_req_inc := (others => '0');
         else
            v_pop_req_inc := (others => '0');
            assert not (s_auto_pop = '1') report "AUTOPOP" severity note;
            if( ((r_force(c_FORCE_BIT_POP) or s_auto_pop) and r_cfg(c_CFG_BIT_ENA)) = '1') then
               v_pop_req_inc := to_unsigned(1, v_pop_req_inc'length);
            end if;
            r_pop_req <= r_pop_req - r_pop_req_dec + v_pop_req_inc;
            
         end if; -- rst       
      end if; -- clk edge
   end process;
------------------------------------------------------------------------------


--**************************************************************************--
-- Autopop Heap
------------------------------------------------------------------------------
   s_due <= '0' when unsigned(time_sys_i) < s_duetime  -- due when top heap element is beyond deadline for sending
       else '1';  

   s_duetime <= unsigned(s_out_key) - unsigned(r_t_due) - unsigned(r_t_trn);

   s_auto_pop <= r_due0 and not r_due1;       -- request autopop on rising edge due

   auto_pop : process(clk_sys_i)
      variable v_rdy : std_logic;
   begin
      if rising_edge(clk_sys_i) then
         if(rst_n_i = '0' or r_rst = "1") then
            r_due0 <= '0';
         else
            r_due0 <= s_due and r_cfg(c_CFG_BIT_AUTOPOP) and not r_empty and not r_pop_ack; 
            r_due1 <= r_due0;
         end if; -- rst       
      end if; -- clk edge
   end process;


--**************************************************************************--
-- Autoflush EBM
------------------------------------------------------------------------------
   s_max_time_reached   <= '0' when unsigned(time_sys_i) < s_deadline -- due when top heap element is beyond deadline for sending
                      else '1';
   
   s_deadline           <= unsigned(r_earliest_key) - unsigned(r_t_trn);
       
   s_max_msg_reached    <= '1' when r_msg_cnt_packet = r_msg_max
                      else '0';

   s_packet_not_empty   <= '0' when unsigned(r_msg_cnt_packet) = 0 
                      else '1';

   s_packet_start       <= s_packet_not_empty and not r_packet_not_empty;

   s_send               <= r_send0 and not r_send1;

   auto_flush : process(clk_sys_i)
      variable v_rdy : std_logic;
      variable v_msg_flush, v_time_flush : std_logic;
   begin
      if rising_edge(clk_sys_i) then
         if(rst_n_i = '0' or r_rst = "1") then
            r_send0  <= '0';
            r_earliest_key <= (others => '0');
         else
            if(s_packet_start = '1') then
               r_earliest_key <= s_out_key;
            end if;
            v_msg_flush          := (s_max_msg_reached   and r_cfg(c_CFG_BIT_AUTOFLUSH_MSGS));
            v_time_flush         := (s_max_time_reached  and r_cfg(c_CFG_BIT_AUTOFLUSH_TIME));
            r_send0              <= r_packet_not_empty   and (v_time_flush or v_msg_flush or r_force(c_FORCE_BIT_FLUSH));
            r_send1              <= r_send0;
            r_packet_not_empty   <= s_packet_not_empty;
         end if; -- rst       
      end if; -- clk edge
   end process;
------------------------------------------------------------------------------
  
end architecture behavioral;      



    
