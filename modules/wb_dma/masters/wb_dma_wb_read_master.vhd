library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wb_dma_pkg.all;
use work.gencores_pkg.all;

entity wb_dma_wb_read_master is
generic(
    g_block_size : integer
);
port(
    clk_i : in std_logic;
    rstn_i : in std_logic;

    -- config signals
    transfer_size_i       : in std_logic_vector(log2_floor(g_block_size) downto 0);
    start_address_i       : in t_wishbone_address;
    descriptor_address_i  : in t_wishbone_address;

    -- communication signals
    dma_active_i        : in std_logic;
    descriptor_active_i : in std_logic;
    rd_buffer_ready_i   : in std_logic;
    buffer_we_o         : out std_logic;
    read_descriptor_i   : in std_logic;

    master_idle_o : out std_logic;

    master_i  : in t_wishbone_master_in;
    master_o  : out t_wishbone_master_out
);
end entity;

architecture rtl of wb_dma_wb_read_master is
  -- read FSM signals
  type t_send_state is (IDLE, SEND, LISTEN, STALL);
  signal r_send_state : t_send_state := IDLE;
  signal s_send_state_next : t_send_state := IDLE;

  signal s_start_transfer : std_logic;

  signal s_transfer_size : std_logic_vector(log2_floor(g_block_size) downto 0);
  signal s_start_address : t_wishbone_address;

  signal s_block_done   : std_logic;
  signal s_ack_complete : std_logic;

  signal s_cnt_en       : std_logic;

  component limit_counter is
  generic(
    g_max_block_size: integer
  );
  port(
    clk_i : in std_logic;
    rstn_i : in std_logic;

    upper_limit_i : in std_logic_vector(log2_floor(g_max_block_size) downto 0) := (others => '0');
    limit_reached_o : out std_logic;

    cnt_en : in std_logic
  );
  end component;
begin

descriptor_data_switch : process(rstn_i, clk_i) begin
  if(rstn_i = '0') then
    s_transfer_size <= (others => '0');
    s_start_address <= (others => '0');
  elsif rising_edge(clk_i) then
    if(read_descriptor_i = '1') then
      s_transfer_size <= std_logic_vector(to_unsigned(4, s_transfer_size'length));
      s_start_address <= descriptor_address_i;
    else
      s_transfer_size <= transfer_size_i;
      s_start_address <= start_address_i;
    end if;
  end if;
end process;

  -- count the number of sent addresses/data
sent_counter : limit_counter
generic map(
  g_max_block_size => g_block_size
)
port map(
  clk_i => clk_i,
  rstn_i => rstn_i,

  upper_limit_i => transfer_size_i,
  limit_reached_o => s_block_done,

  cnt_en => s_cnt_en
);

s_cnt_en <= master_o.stb and not master_i.stall;

-- count the number of received acknowledges
ack_counter : limit_counter
generic map(
  g_max_block_size => g_block_size
)
port map(
  clk_i => clk_i,
  rstn_i => rstn_i,

  upper_limit_i => transfer_size_i,
  limit_reached_o => s_ack_complete,

  cnt_en => master_i.ACK
);

p_start_FSM : process(rstn_i, r_send_state, descriptor_active_i, dma_active_i, rd_buffer_ready_i) begin
if(rstn_i = '0') then
  s_start_transfer <= '0';
else
  if(r_send_state = IDLE) then
    if(dma_active_i <= '1' and descriptor_active_i = '1' and rd_buffer_ready_i = '1') then
      s_start_transfer <= '1';
    else
      s_start_transfer <= '0';
    end if;
  else
    s_start_transfer <= '0';
  end if;
end if;
end process;

p_send_state_comb: process(rstn_i, r_send_state, dma_active_i, descriptor_active_i, master_i.stall, s_block_done, s_ack_complete, s_start_transfer)
begin
if (rstn_i = '0') then
  master_o.cyc <= '0'; --r_send_state = SEND or r_send_state = LISTEN or r_send_state = STALL;
  master_o.stb <= '0'; --r_send_state = SEND or r_send_state = STALL;
  master_o.sel <= (others => '0');
  master_o.we  <= '0';
  master_o.dat <= (others => '0');
  buffer_we_o <= '0';

  s_send_state_next <= IDLE;
else
  master_o.cyc <= '0'; --r_send_state = SEND or r_send_state = LISTEN or r_send_state = STALL;
  master_o.stb <= '0'; --r_send_state = SEND or r_send_state = STALL;
  master_o.sel <= (others => '0');
  master_o.we  <= '0';
  master_o.dat <= (others => '0');
  buffer_we_o <= '0';
  
  case r_send_state is
    when IDLE =>
  
      if(s_start_transfer = '1') then
        if(master_i.stall = '1') then
          s_send_state_next <= STALL;
        else
          s_send_state_next <= SEND;
        end if;
      else
        s_send_state_next <= IDLE;
      end if;
  
    -- send address and data if this is a write cycle
    when SEND =>
      master_o.cyc <= '1';
      if(s_block_done = '1') then
        master_o.stb <= '0';
        s_send_state_next <= LISTEN;
      elsif(master_i.stall = '1') then
        master_o.stb <= '1';
        s_send_state_next <= STALL;
      else
        master_o.stb <= '1';
        s_send_state_next <= SEND;
      end if;
  
    -- keep current output until stall goes to 0
    when STALL =>
      master_o.cyc <= '1';
      master_o.stb <= '1';
  
      if(s_block_done = '1') then
        master_o.stb <= '0';
        s_send_state_next <= LISTEN;
      elsif(master_i.stall = '1') then
        s_send_state_next <= STALL;
      else
        s_send_state_next <= SEND;
      end if;
      
    -- listen to the slave until the count of acknowledge, error and retry signals is correct
    when LISTEN =>
      master_o.cyc <= '1';
  
      if(s_ack_complete = '1') then
        s_send_state_next <= IDLE;
      else
        s_send_state_next <= LISTEN;
      end if;
  end case;

  if (master_o.cyc = '1' and master_i.ack = '1') then  -- not checking for full buffer as it should be as big as the max transfer size and after the read gets swapped to the write master
    buffer_we_o <= '1';
  end if;
end if;
end process;

p_send_fsm_synch : process(clk_i, rstn_i, start_address_i)
begin
  if(rstn_i = '0') then
    master_o.adr <= start_address_i;
  else
    if rising_edge(clk_i) then
      case (s_send_state_next) is
        when IDLE =>
          master_o.adr <= start_address_i;

        when SEND =>
          master_o.adr <= std_logic_vector(unsigned(master_o.adr) + 4);
      
        when others =>
          master_o.adr <= master_o.adr;

      end case;
    
      r_send_state <= s_send_state_next;
    end if; --clk_i
  end if;
end process;

-- only allow burst length changes when the master is idle, but buffers the signal one cycle --> avoid this to avoid buffering all signals to keep synchronization
-- p_set_burst_length : process(clk_i, rstn_i)
-- begin
--   if(rstn_i = '0') then
--     r_transfer_size <= g_block_size;
--   elsif rising_edge(clk_i) then
--     r_transfer_size <= transfer_size_i;
--   end if;
-- end process;

idle_signal : process(r_send_state, rstn_i) begin
  if(rstn_i = '0') then
    master_idle_o <= '0';
  else
    if(r_send_state = IDLE) then
      master_idle_o <= '1';
    else
      master_idle_o <= '0';
    end if;
  end if;
end process;

end architecture;