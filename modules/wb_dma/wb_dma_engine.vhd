library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wb_dma_pkg.all;
use work.gencores_pkg.all;

entity wb_dma_engine is
  port(
    clk_i : in std_logic;
    rstn_i : in std_logic;

    desc_write_select_o : out std_logic_vector(1 downto 0);
    desc_write_enable_o : out std_logic;

    init_pointer_addr_i : in std_logic_vector(31 downto 0);
    next_pointer_i      : in std_logic_vector(31 downto 0);
    pointer_addr_o      : out std_logic_vector(31 downto 0);

    -- communication signals
    dma_active_i        : in std_logic; -- enabled
    dma_idle_i          : in std_logic; -- actively transferring data
    descriptor_active_i : in std_logic;
    init_transfer_i     : in std_logic
  );
end entity;

architecture wb_dma_engine_arch of wb_dma_engine is
  type t_engine_state is (IDLE, INIT_TRANSFER, LOAD_DESC_CSR, LOAD_DESC_ADDR1, LOAD_DESC_ADDR2, LOAD_DESC_NEXT);
  signal r_engine_state : t_engine_state;
  signal s_next_state : t_engine_state;
begin


  -- TODO: make the FSM dynamic to account for loading delays over wishbone.
p_engine : process (rstn_i, r_engine_state, init_transfer_i) begin
if(rstn_i = '0') then
  s_next_state <= IDLE;
else
  s_next_state <= IDLE;

  case r_engine_state is
    when IDLE =>
      if(init_transfer_i = '1') then
        s_next_state <= INIT_TRANSFER;
      end if;

    when INIT_TRANSFER =>
      s_next_state <= LOAD_DESC_ADDR1;

    when LOAD_DESC_CSR =>
      s_next_state <= LOAD_DESC_ADDR1;

    when LOAD_DESC_ADDR1 =>
      s_next_state <= LOAD_DESC_ADDR2;

    when LOAD_DESC_ADDR2 =>
      s_next_state <= LOAD_DESC_NEXT;

    when LOAD_DESC_NEXT =>
      s_next_state <= IDLE;

    end case;
end if;
end process;

p_state_transition : process(rstn_i, clk_i) begin
  if(rstn_i = '0') then
    r_engine_state <= IDLE;
  elsif rising_edge(clk_i) then
    r_engine_state <= s_next_state;
  end if;
end process;

p_outputs : process (rstn_i, clk_i, r_engine_state, init_pointer_addr_i, next_pointer_i) begin
if(rstn_i = '0') then
  desc_write_select_o <= b"00";
  desc_write_enable_o <= '0';
  pointer_addr_o      <= (others => '0');-- init_pointer_addr_i;
else
if rising_edge(clk_i) then
  case s_next_state is
    when INIT_TRANSFER =>
      pointer_addr_o <= init_pointer_addr_i;

    when LOAD_DESC_CSR =>
      desc_write_select_o <= b"00";
      desc_write_enable_o <= '1';

    when LOAD_DESC_ADDR1 =>
      desc_write_select_o <= b"01";
      desc_write_enable_o <= '1';
      pointer_addr_o      <= std_logic_vector(unsigned(pointer_addr_o) + 4);

    when LOAD_DESC_ADDR2 =>
      desc_write_select_o <= b"10";
      desc_write_enable_o <= '1';
      pointer_addr_o      <= std_logic_vector(unsigned(pointer_addr_o) + 4);

    when LOAD_DESC_NEXT =>
      desc_write_select_o <= b"11";
      desc_write_enable_o <= '1';
      pointer_addr_o      <= std_logic_vector(unsigned(pointer_addr_o) + 4);

    when others =>
      desc_write_select_o <= b"00";
      desc_write_enable_o <= '0';
      pointer_addr_o      <= next_pointer_i; -- TODO

    end case;
end if;
end if;
end process;

end architecture;