library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wb_dma_pkg.all;
use work.gencores_pkg.all;

entity wb_dma_wb_master is
generic(
    g_block_size : integer
);
port(
    clk_i : in std_logic;
    rstn_i : in std_logic;

    -- config signals
    burst_length_i : in std_logic_vector(log2_ceil(g_block_size)-1 downto 0);
    start_address_i : in t_wishbone_address;
    we_i : in std_logic;

    -- communication signals
    dma_active_i : in std_logic;
    descriptor_active_i : in std_logic;

    master_idle_o : out std_logic;

    master_i  : in t_wishbone_master_in;
    master_o  : out t_wishbone_master_out
);
end entity;

architecture rtl of wb_dma_wb_master is
  -- read FSM signals
  type t_send_state is (IDLE, SEND, LISTEN, STALL);
  signal r_send_state : t_send_state := IDLE;

  signal r_burst_length : std_logic_vector(log2_ceil(g_block_size)-1 downto 0);

  signal s_block_done   : std_logic;
  signal s_ack_complete : std_logic;

  component limit_counter is
  generic(
    g_max_block_size: integer
  );
  port(
    clk_i : in std_logic;
    rstn_i : in std_logic;

    upper_limit : in std_logic_vector(log2_ceil(g_max_block_size)-1 downto 0) := (others => '0');
    limit_reached : out std_logic;

    cnt_en : in std_logic
  );
  end component;
begin

  -- count the number of sent addresses/data
sent_counter : limit_counter
generic map(
  g_max_block_size => g_block_size
)
port map(
  clk_i => clk_i,
  rstn_i => rstn_i,

  upper_limit => r_burst_length,
  limit_reached => s_block_done,

  cnt_en => master_o.stb = '1' and not master_i.stall = '0'
);

-- count the number of received acknowledges
ack_counter : limit_counter
generic map(
  g_max_block_size => g_block_size
)
port map(
  clk_i => clk_i,
  rstn_i => rstn_i,

  limit_reached => s_ack_complete,

  cnt_en => master_i.ACK = '1'
);

p_send_fsm: process (clk_i, rstn_i)
begin
if rstn_i = '0' then
  r_send_state <= IDLE;
elsif rising_edge(clk_i) then

  case r_send_state is
  when IDLE =>
    if(dma_active_i and descriptor_active_i) then
      r_send_state <= SEND;
    else
      r_send_state <= IDLE;
    end if;

  -- send address and data if this is a write cycle
  when SEND =>

    if(master_i.stall = '1') then
      r_send_state <= STALL;
    elsif(s_block_done) then
      r_send_state <= LISTEN;
    else
      r_send_state <= SEND;
    end if;

  -- keep current output until stall goes to 0
  when STALL =>
    if(master_i.stall = '1') then
      r_send_state <= STALL;
    else
      r_send_state <= SEND;
    end if;
    
  -- listen to the slave until the count of acknowledge, error and retry signals is correct
  when LISTEN =>
    if(s_ack_complete) then
      r_send_state <= IDLE;
    else
      r_send_state <= LISTEN;
    end if;
  end case;
end if;
end process;

p_send_outputs : process
begin
  master_o.cyc <= r_send_state = SEND or r_send_state = LISTEN or r_send_state = STALL;
  master_o.stb <= r_send_state = SEND or r_send_state = STALL;
  master_o.adr <= (others => '0');
  master_o.sel <= (others => '0');
  master_o.we  <= '0';
  master_o.dat <= (others => '0');
end process;

p_set_burst_length : process(clk_i, rstn_i)
begin
  if(rstn_i = '0') then
    r_burst_length <= g_block_size;
  elsif rising_edge(clk_i) then
    r_burst_length <= burst_length_i;
  end if;
end process;

master_idle_o <= r_send_state = IDLE;

end architecture;