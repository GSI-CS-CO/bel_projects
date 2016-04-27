--! @file        prio.xml
--  DesignUnit   prio
--! @author      M. Kreider <m.kreider@gsi.de>
--! @date        02/11/2015
--! @version     0.0.1
--! @copyright   2015 GSI Helmholtz Centre for Heavy Ion Research GmbH
--!

--! @brief Top file of the DM Priority Queue v2
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
--------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;
use work.matrix_pkg.all;
use work.genram_pkg.all;
use work.prio_auto_pkg.all;
use work.prio_pkg.all;

entity prio is
generic(
  g_ebm_bits    : natural := 10;
  g_depth       : natural := 16;    
  g_num_masters : natural := 8
);
Port(
  clk_i         : std_logic;                                          -- Clock input for sys domain
  rst_n_i       : std_logic;                                          -- Reset input (active low) for sys domain

  time_i        : std_logic_vector(64-1 downto 0);    

  ctrl_i        : in  t_wishbone_slave_in;
  ctrl_o        : out t_wishbone_slave_out;
  slaves_i      : in  t_wishbone_slave_in_array(g_num_masters-1 downto 0);
  slaves_o      : out t_wishbone_slave_out_array(g_num_masters-1 downto 0);
  master_o      : out t_wishbone_master_out;
  master_i      : in  t_wishbone_master_in
  
);
end entity;

architecture rtl of prio is

  -- ebm control arbitration signals
  signal s_ebm_ctrl_o : t_wishbone_master_out;
  signal s_ebm_ctrl_i : t_wishbone_master_in;
  signal s_arbiter_o  : t_wishbone_master_out;
  signal s_arbiter_i  : t_wishbone_master_in;

  -- EBM if constants
  constant c_ebm_ahi            : natural := g_ebm_bits;
  constant c_dat_bit            : natural := t_wishbone_address'left - c_ebm_ahi +2;
  constant c_rw_bit             : natural := t_wishbone_address'left - c_ebm_ahi +1;
  constant c_EBM_DAT_OFFS       : std_logic_vector(31 downto 0) := std_logic_vector(to_unsigned(2**c_dat_bit, t_wishbone_address'length));
  constant c_EBM_RW_OFFS        : std_logic_vector(31 downto 0) := std_logic_vector(to_unsigned(2**c_rw_bit, t_wishbone_address'length));
  constant c_EBM_CLEAR          : std_logic_vector(31 downto 0) := x"00000000";  
  constant c_EBM_FLUSH          : std_logic_vector(31 downto 0) := x"00000004";
  constant c_EBM_ADHI           : std_logic_vector(31 downto 0) := x"00000030";
  constant c_EBM_ADR_MSK        : std_logic_vector(31 downto 0) := std_logic_vector(to_unsigned(0, c_ebm_ahi) 
                                                                                  & unsigned(to_signed(-1, t_wishbone_address'length-c_ebm_ahi)));
  -- wb ctrl if signals
  signal s_ctrl_ctrl_stall_i    : std_logic_vector(1-1 downto 0);               -- flow control
  signal s_ctrl_reset_o         : std_logic_vector(1-1 downto 0);               -- Resets the Priority Queue
  signal s_ctrl_mode_o          : std_logic_vector(3-1 downto 0);               -- b2: Time limit, b1: Msg limit, b0 enable
  constant c_TIME_LIMIT : natural := 2;                                         -- bit indices
  constant c_MSG_LIMIT  : natural := 1;
  constant c_ENABLE     : natural := 0;
  signal s_ctrl_clear_o         : std_logic_vector(1-1 downto 0);               -- Clears counters and status
  signal s_ctrl_st_full_i       : std_logic_vector(g_num_masters-1 downto 0);      -- Channel Full flag (n..0) 
  signal s_ctrl_st_late_i       : std_logic_vector(0 downto 0);      -- Late message detected
  signal s_ctrl_ebm_adr_o       : std_logic_vector(32-1 downto 0);              -- Etherbone Master address
  signal s_ctrl_eca_adr_o       : std_logic_vector(32-1 downto 0);              -- Event Condition Action Unit address  
  signal s_ctrl_tx_max_msgs_o   : std_logic_vector(8-1 downto 0);               -- Max msgs per packet
  signal s_ctrl_tx_max_wait_o   : std_logic_vector(32-1 downto 0);              -- Max wait time for non empty packet
  signal s_ctrl_tx_rate_limit_o : std_logic_vector(32-1 downto 0);              -- Max msgs per milliseconds
  signal s_ctrl_cnt_late_i      : std_logic_vector(32-1 downto 0);              -- Sum of all late messages
  signal s_ctrl_ts_late_i       : std_logic_vector(64-1 downto 0);              -- First late Timestamp  
  signal s_ctrl_offs_late_o     : std_logic_vector(64-1 downto 0);              -- Time offset before message is late
  signal s_ctrl_cnt_out_all_i   : std_logic_vector(64-1 downto 0);              -- Sum of all outgoing messages
  signal s_ctrl_ch_sel_o        : std_logic_vector(4-1 downto 0);               -- Channel select
  
  signal s_ctrl_cnt_out_i       : matrix(g_num_masters-1 downto 0, 32-1 downto 0); -- Outgoing messages per Channel
  signal s_ctrl_cnt_in_i        : matrix(g_num_masters-1 downto 0, 32-1 downto 0); -- Incoming messages per Channel
  
  signal s_ts_valid_o           : std_logic;
  signal s_ts_o                 : std_logic_vector(64-1 downto 0);
  signal r_late_check           : unsigned(64-1 downto 0); 
  signal s_arbiter_dst          : std_logic_vector(32-1 downto 0);
 
  -- aux and fsm signals 
  signal s_reset_n              : std_logic;
  signal s_en                   : std_logic;
  type t_state is (st_IDLE, st_PACKET_PREP, st_PACKET_EMPTY, st_PACKET_GATHER, st_PACKET_SEND, st_ERROR, st_WAIT_FOR_EBM);  
  signal r_state                : t_state;
  signal r_state_after_wait     : t_state;
  signal r_allow_sending        : std_logic;
  signal r_dhol                 : unsigned(64-1 downto 0); 
  signal r_msg_cnt              : unsigned(7 downto 0); 
  signal r_cnt_late             : std_logic_vector(31 downto 0);

begin

  s_reset_n <= not s_ctrl_reset_o(0)  and rst_n_i;
  s_en      <= s_ctrl_mode_o(c_ENABLE) and r_allow_sending;

  fsm : process(clk_i)
    variable v_state : t_state;
  begin
    if rising_edge(clk_i) then
      if s_reset_n = '0' then
        s_ebm_ctrl_o.cyc  <= '0';
        s_ebm_ctrl_o.stb  <= '0';
        s_ebm_ctrl_o.we   <= '1';
        s_ebm_ctrl_o.sel  <= (others => '1');        
        s_ebm_ctrl_o.adr  <= (others => '0');
        s_ebm_ctrl_o.dat  <= (others => '0');
        r_dhol            <= (others => '0');
        r_allow_sending   <= '0';
        r_cnt_late        <= (others => '0');
        s_ctrl_st_late_i  <= (others => '0');
        s_ctrl_ts_late_i  <= (others => '0');
        r_state           <= st_PACKET_PREP;
        --r_state           <= st_EBM_CLEAR;
      else

        r_late_check <= unsigned(s_ts_o) - unsigned(s_ctrl_offs_late_o);
  
        v_state := r_state;
        
        case r_state is
          when st_IDLE          =>  if(s_ctrl_mode_o(c_ENABLE) = '1') then
                                      v_state := st_PACKET_PREP;  
                                    end if;

          when st_PACKET_PREP   =>  --set ebm high address
                                    r_msg_cnt   <= (others => '0');
                                    if s_ebm_ctrl_i.stall = '0' then
                                      r_state_after_wait <= st_PACKET_EMPTY;
                                      v_state := st_WAIT_FOR_EBM;
                                    end if;

          when st_PACKET_EMPTY  =>  if(s_ctrl_mode_o(c_ENABLE) = '0') then
                                      v_state := st_IDLE;  
                                    elsif s_ts_valid_o = '1' then
                                      r_dhol  <= unsigned(time_i) + resize(unsigned(s_ctrl_tx_max_wait_o), time_i'length);
                                      v_state := st_PACKET_GATHER;
                                    end if;
                                           
                                    
          when st_PACKET_GATHER =>  --time limit reached?    
                                    if (r_dhol  <= unsigned(time_i)) and s_ctrl_mode_o(c_TIME_LIMIT) = '1' then
                                      v_state := st_PACKET_SEND;
                                    end if;
                                    --message limit reached?
                                    if (r_msg_cnt >= unsigned(s_ctrl_tx_max_msgs_o))
                                    and s_ctrl_mode_o(c_MSG_LIMIT) = '1' then
                                      v_state := st_PACKET_SEND;
                                    end if;
          
           when st_PACKET_SEND  =>  if s_ebm_ctrl_i.stall = '0' then
                                      report "Sending Packet containing " & integer'image(to_integer(r_msg_cnt)) severity note;
                                      r_msg_cnt           <= (others => '0');
                                      r_state_after_wait  <= st_PACKET_EMPTY;
                                      v_state := st_WAIT_FOR_EBM;
                                    end if;

           when st_ERROR        =>  v_state := st_PACKET_PREP;

           when st_WAIT_FOR_EBM =>  if (s_ebm_ctrl_i.ack = '1') then
                                      v_state := r_state_after_wait;
                                    end if;
                                    if (s_ebm_ctrl_i.err = '1') then
                                      v_state := st_ERROR;
                                    end if;     

            when others =>          v_state := st_ERROR;
         end case; 
    
         --default behaviour
         r_allow_sending   <= '0';
         s_ebm_ctrl_o.cyc  <= '0';
         s_ebm_ctrl_o.stb  <= '0';       
          
         if v_state = st_PACKET_EMPTY or 
            v_state = st_PACKET_GATHER then
            -- In EMPTY and GATHER state, allow arbiter to send messages out            
            r_allow_sending <= '1';
         end if;
         
         if v_state = st_WAIT_FOR_EBM then
            s_ebm_ctrl_o.cyc <= '1'; 
         end if;       
  
     
         if v_state = st_PACKET_PREP  then
            -- set EBM high address to ECA 
            s_ebm_ctrl_o.cyc <= '1';
            s_ebm_ctrl_o.stb <= '1'; 
            s_ebm_ctrl_o.adr <= std_logic_vector(unsigned(s_ctrl_ebm_adr_o) + unsigned(c_EBM_ADHI));
            s_ebm_ctrl_o.dat <= s_ctrl_eca_adr_o;      
         end if; 
        
         if v_state = st_PACKET_SEND then
            -- flush EBM
            s_ebm_ctrl_o.cyc <= '1';
            s_ebm_ctrl_o.stb <= '1'; 
            s_ebm_ctrl_o.adr <= std_logic_vector(unsigned(s_ctrl_ebm_adr_o) + unsigned(c_EBM_FLUSH));
            s_ebm_ctrl_o.dat <= std_logic_vector(to_unsigned(1, s_ebm_ctrl_o.dat'length));     
         end if;


         if (s_ctrl_clear_o = "1") then
            s_ctrl_st_late_i(0) <= '0';
            r_cnt_late          <= (others => '0');
         end if;


        if (s_ts_valid_o and r_allow_sending) = '1' then
          r_msg_cnt <= r_msg_cnt + 1;
          if r_late_check <= unsigned(time_i) then
            -- this message is late
            s_ctrl_st_late_i(0) <= '1';
            r_cnt_late          <= std_logic_vector(unsigned(r_cnt_late) +1);
            if (s_ctrl_st_late_i(0) = '0') then  
              s_ctrl_ts_late_i <= s_ts_o;
            end if;
            --s_ctrl_id_late_i <=    
          end if;
        end if;

         -- assign current state to state register
         r_state <= v_state; 
      end if;
    end if;
  end process fsm;


  -- arbiter / ebm ctrl arbitration
  Master_Out : xwb_crossbar 
  generic map(
    g_num_masters => 2,
    g_num_slaves  => 1,
    g_registered  => true,
    -- Address of the slaves connected
    g_address     => (0 => x"00000000"),
    g_mask        => (0 => x"00000000"))
  port map(
     clk_sys_i     => clk_i,
     rst_n_i       => rst_n_i,
        -- Master connections (INTERCON is a slave)
     slave_i(0)       => s_ebm_ctrl_o,
     slave_i(1)       => s_arbiter_o,      
     slave_o(0)       => s_ebm_ctrl_i,
     slave_o(1)       => s_arbiter_i, 
     -- Slave connections (INTERCON is a master)
     master_i(0)      => master_i,
     master_o(0)      => master_o);

  
  --arbiter dst address on EBM
  s_arbiter_dst <= s_ctrl_ebm_adr_o or c_EBM_DAT_OFFS or c_EBM_RW_OFFS or (s_ctrl_eca_adr_o and c_EBM_ADR_MSK);

  --Arbitration of Interface to EBM between the 8 sources by EDF scheduler
  arb : arbiter
  generic map(
    g_depth       => g_depth,
    g_num_masters => g_num_masters
  )
  port map (
    clk_i         => clk_i,
    rst_n_i       => s_reset_n,
    en_i          => s_en,
    cnt_clear_i   => s_ctrl_clear_o(0),
    dst_adr_i     => s_arbiter_dst,
    full_o        => s_ctrl_st_full_i,
    cnt_ch_in_o   => s_ctrl_cnt_in_i,
    cnt_ch_out_o  => s_ctrl_cnt_out_i,
    cnt_all_out_o => s_ctrl_cnt_out_all_i,
    ts_o          => s_ts_o,
    ts_valid_o    => s_ts_valid_o,
    
    slaves_i      => slaves_i,
    slaves_o      => slaves_o,
    master_o      => s_arbiter_o,
    master_i      => s_arbiter_i
    
    ------------------------------
  );

  -- Wishbone Control Interface
  INST_prio_auto : prio_auto
  generic map (
    g_channels => g_num_masters   
  )
  port map (
    clk_sys_i         => clk_i,
    rst_sys_n_i       => rst_n_i,
    ctrl_stall_i      => "0",
    reset_o           => s_ctrl_reset_o,
    mode_o            => s_ctrl_mode_o,
    clear_o           => s_ctrl_clear_o,
    st_full_i         => s_ctrl_st_full_i,
    st_late_i         => s_ctrl_st_late_i,
    ebm_adr_o         => s_ctrl_ebm_adr_o,
    eca_adr_o         => s_ctrl_eca_adr_o,
    tx_max_msgs_o     => s_ctrl_tx_max_msgs_o,
    tx_max_wait_o     => s_ctrl_tx_max_wait_o,
    tx_rate_limit_o   => s_ctrl_tx_rate_limit_o,
    offs_late_o       => s_ctrl_offs_late_o,
    ts_late_i         => s_ctrl_ts_late_i,
    cnt_late_i        => r_cnt_late,  
    cnt_out_all_i     => s_ctrl_cnt_out_all_i,
    ch_sel_o          => s_ctrl_ch_sel_o,
    
    cnt_out_i         => s_ctrl_cnt_out_i,
    cnt_in_i          => s_ctrl_cnt_in_i,
    ctrl_i            => ctrl_i,
    ctrl_o            => ctrl_o   );






end architecture;
