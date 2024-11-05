library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wb_dma_pkg.all;

entity wb_dma_engine is
  port(
    clk_i : in std_logic;
    rstn_i : in std_logic;

    s_load_descriptor_en_o      : out std_logic;

    s_desc_csr_sz_we_o  : out std_logic;
    s_desc_addr0_we_o   : out std_logic;
    s_desc_addr1_we_o   : out std_logic;
    s_pointer_we_o      : out std_logic;

    -- read logic
    s_queue_full_i              : in std_logic;
    s_queue_empty_i             : in std_logic;
    s_read_enable_o             : out std_logic := '0';
    s_data_cache_write_enable_o : out std_logic := '0';
    s_read_ack                  : in std_logic;

    --only for testing!!!!
    s_start_desc                : in std_logic;
    s_read_init_address         : in std_logic_vector(c_wishbone_address_width-1 downto 0);
    s_descriptor_active         : in std_logic;

    s_data_in_ack_i             : in std_logic
  );
end entity;

architecture behavioral of wb_dma_engine is

  -- read op contstructor signals
  signal s_read_addr    : std_logic_vector(c_wishbone_address_width-1 downto 0);

  -- read op FIFO FSM signals
  
  type t_state is (IDLE, READ, WRITE, UPDATE, LD_DESC1, LD_DESC2, LD_DESC3, LD_DESC4, LD_DESC5, WB, PAUSE);
  signal r_state : t_state := IDLE;

begin

p_state_machine: process (clk_i)
begin
if rstn_i = '0' then
  r_state <= IDLE;
elsif rising_edge(clk_i) then
  s_desc_csr_sz_we_o <= '0';
  s_desc_addr0_we_o  <= '0';
  s_desc_addr1_we_o  <= '0';
  s_pointer_we_o     <= '0';
  
  case r_state is
  
  when IDLE =>

  when LD_DESC1 =>
    s_load_descriptor_en_o <= '1';
    
    if s_data_in_ack_i = '1' then
      s_desc_csr_sz_we_o <= '1';
      r_state <= LD_DESC2;
    end if;
  
  when LD_DESC2 =>
    s_load_descriptor_en_o <= '1';
    
    if s_data_in_ack_i = '1' then
      s_desc_addr0_we_o  <= '1';
      r_state <= LD_DESC3;
    end if;
  
  when LD_DESC3 =>
    s_load_descriptor_en_o <= '1';
    
    if s_data_in_ack_i = '1' then
      s_desc_addr1_we_o  <= '1';
      r_state <= LD_DESC4;
    end if;
  
  when LD_DESC4 =>
    s_load_descriptor_en_o <= '1';
    
    if s_data_in_ack_i = '1' then
      s_pointer_we_o     <= '1';
      r_state <= LD_DESC5;
    end if;
  
  when LD_DESC5 =>
    s_load_descriptor_en_o <= '1';
    
    if s_data_in_ack_i = '1' then
     r_state <= READ; --TODO
    end if;
  
  end case;
end if;
end process;

end architecture;
