library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.cellular_ram_pkg.all;

entity cellular_ram is
  generic(
    g_rams     : natural := 1;
    g_bits     : natural := 24);
  port(
    clk_i      : in    std_logic;
    rstn_i     : in    std_logic;
    slave_i    : in    t_wishbone_slave_in;
    slave_o    : out   t_wishbone_slave_out;
    cr_clk_o   : out   std_logic;
    cr_addr_o  : out   std_logic_vector(g_bits-1 downto 0);
    cr_data_io : inout std_logic_vector(15 downto 0);
    cr_ubn_o   : out   std_logic_vector(3 downto 0);
    cr_lbn_o   : out   std_logic_vector(3 downto 0);
    cr_cen_o   : out   std_logic_vector(3 downto 0);
    cr_oen_o   : out   std_logic_vector(3 downto 0);
    cr_wen_o   : out   std_logic_vector(3 downto 0);
    cr_cre_o   : out   std_logic_vector(3 downto 0);
    cr_advn_o  : out   std_logic_vector(3 downto 0);
    cr_wait_i  : in    std_logic_vector(3 downto 0));
end entity;

architecture rtl of cellular_ram is

  type t_state      is (S_INITIAL, S_IDLE, S_READ, S_WRITE);
  type t_state_word is (S_LOW_WORD, S_PREP_NEXT_WORD, S_HIGH_WORD);
  signal r_state      : t_state := S_INITIAL;
  signal r_state_word : t_state_word := S_LOW_WORD;
  signal r_ram_out    : t_cellular_ram_out;
  signal r_ack        : std_logic := '0';
  constant c_trc      : natural := 5;
  constant c_tcem     : natural := 5;
  signal r_counter_r  : unsigned(f_ceil_log2(c_trc+1)-1 downto 0) := (others => '0');
  signal r_counter_w  : unsigned(f_ceil_log2(c_tcem+1)-1 downto 0) := (others => '0');
  signal r_selector   : std_logic_vector(3 downto 0);

  -- Configuration registers
  constant c_cfg_reg_ram0_id : std_logic_vector (7 downto 0) := x"00";
  constant c_cfg_reg_ram1_id : std_logic_vector (7 downto 0) := x"04";
  constant c_cfg_reg_ram2_id : std_logic_vector (7 downto 0) := x"08";
  constant c_cfg_reg_ram3_id : std_logic_vector (7 downto 0) := x"0C";
  constant c_cfg_reg_devices : std_logic_vector (7 downto 0) := x"10";
  constant c_cfg_reg_trc     : std_logic_vector (7 downto 0) := x"14";
  constant c_cfg_reg_tcrem   : std_logic_vector (7 downto 0) := x"18";
  constant c_cfg_reg_debug   : std_logic_vector (7 downto 0) := x"1C";

begin

  -- Wishbone
  slave_o.stall <= not(r_ack);
  slave_o.ack   <= r_ack;

  -- Unused Wishbone signals
  slave_o.rty <= '0';
  slave_o.err <= '0';

 -- Cellular RAM
  quad_ram : for i in 0 to 3 generate
    cr_cre_o(i) <= r_ram_out.cre when (r_selector(i) = '1') else '1';
    cr_oen_o(i) <= r_ram_out.oen when (r_selector(i) = '1') else '1';
    cr_wen_o(i) <= r_ram_out.wen when (r_selector(i) = '1') else '1';
    cr_cen_o(i) <= r_ram_out.cen when (r_selector(i) = '1') else '1';
    cr_ubn_o(i) <= r_ram_out.ubn when (r_selector(i) = '1') else '1';
    cr_lbn_o(i) <= r_ram_out.lbn when (r_selector(i) = '1') else '1';
  end generate;

  -- Unused cellular RAM pins (asynchronous mode)
  cr_clk_o  <= '0';
  cr_advn_o <= (others => '0');

  -- Select RAM 0..3 based on the Wishbone address, see cellular_ram_regs.h
  p_ram_selector : process(clk_i, rstn_i) is
  begin
    if rstn_i = '0' then
      r_selector <= "0001";
    elsif rising_edge(clk_i) then
      if (slave_i.cyc and slave_i.stb) = '1' then
        case to_integer(unsigned(slave_i.adr(27 downto 24))) is
          -- RAM Access
          when 0 to 1 =>
            r_selector <= "0001";
          when 2 to 3 =>
            r_selector <= "0010";
          when 4 to 5 =>
            r_selector <= "0100";
          when 6 to 7 =>
            r_selector <= "1000";
          -- Configuration or access
          when 8 =>
            case to_integer(unsigned(slave_i.adr(3 downto 2))) is
              -- Read Chip ID
              when 0 =>
                r_selector <= "0001";
              when 1 =>
                r_selector <= "0010";
              when 2 =>
                r_selector <= "0100";
              when 3 =>
                r_selector <= "1000";
              -- Access configruation registers
              when others =>
                r_selector <= (others => '0');
            end case;
          when others =>
            -- Mistake
            r_selector <= (others => '0');
        end case;
      end if;
    end if;
  end process;

  -- Handle Wishbone requests and read or write from RAM
  p_wishbone_handler : process(clk_i, rstn_i) is
  begin
    if rstn_i = '0' then
      -- Wishbone
      r_ack        <= '0';
      slave_o.dat  <= (others => '0');
      -- Intenral state machine
      r_state      <= S_INITIAL;
      r_state_word <= S_LOW_WORD;
      r_counter_r  <= (others => '0');
      r_counter_w  <= (others => '0');
      -- Data and addr
      cr_data_io   <= (others => 'Z');
      cr_addr_o    <= (others => '0');
      r_ram_out    <= f_cellular_ram_set_standby;
    elsif rising_edge(clk_i) then
      -- Access RAM
      if slave_i.adr(27) = '0' then
        case r_state is
          when S_INITIAL =>
            r_state      <= S_IDLE;
            r_state_word <= S_LOW_WORD;
            cr_data_io   <= (others => 'Z');
            cr_addr_o    <= (others => '0');
            r_ram_out    <= f_cellular_ram_set_standby;
            r_ack        <= '0';
          when S_IDLE =>
            if (slave_i.cyc and slave_i.stb) = '1' then
              if slave_i.we = '1' then
              r_state    <= S_WRITE;
              cr_addr_o  <= slave_i.adr(g_bits downto 1);
              cr_data_io <= slave_i.dat(15 downto 0);
              r_ram_out  <= f_cellular_ram_set_write;
            else
              r_state    <= S_READ;
              cr_addr_o  <= slave_i.adr(g_bits downto 1);
              cr_data_io <= (others => 'Z');
              r_ram_out  <= f_cellular_ram_set_read;
            end if;
            r_state_word <= S_LOW_WORD;
            r_ack        <= '0';
          else
            cr_data_io  <= (others => 'Z');
            cr_addr_o   <= (others => '0');
            r_ram_out   <= f_cellular_ram_set_standby;
            r_ack       <= '0';
          end if;
        when S_WRITE =>
          r_counter_w <= r_counter_w + 1;
          if r_counter_w = c_tcem then
            r_counter_w <= (others => '0');
            case r_state_word is
              when S_LOW_WORD =>
                r_state_word              <= S_PREP_NEXT_WORD;
                r_ram_out                 <= f_cellular_ram_set_standby;
                cr_addr_o                 <= std_logic_vector(unsigned(slave_i.adr(g_bits downto 1)) + 1);
                cr_data_io                <= slave_i.dat(31 downto 16);
              when S_PREP_NEXT_WORD =>
                r_state_word              <= S_HIGH_WORD;
                r_ram_out                 <= f_cellular_ram_set_write;
              when S_HIGH_WORD =>
                r_state_word              <= S_LOW_WORD;
                r_ram_out                 <= f_cellular_ram_set_standby;
                r_ack                     <= '1';
                r_state                   <= S_INITIAL;
             end case;
           end if;
         when S_READ =>
           r_counter_r <= r_counter_r + 1;
           if r_counter_r = c_tcem then
             r_counter_r <= (others => '0');
             case r_state_word is
               when S_LOW_WORD =>
                 r_state_word              <= S_PREP_NEXT_WORD;
                 r_ram_out                 <= f_cellular_ram_set_standby;
                 cr_addr_o                 <= std_logic_vector(unsigned(slave_i.adr(g_bits downto 1)) + 1);
                 slave_o.dat(15 downto 0)  <= cr_data_io;
               when S_PREP_NEXT_WORD =>
                 r_state_word              <= S_HIGH_WORD;
                 r_ram_out                 <= f_cellular_ram_set_read;
               when S_HIGH_WORD =>
                 r_state_word              <= S_LOW_WORD;
                 r_ram_out                 <= f_cellular_ram_set_standby;
                 slave_o.dat(31 downto 16) <= cr_data_io;
                 r_ack                     <= '1';
                 r_state                   <= S_INITIAL;
            end case;
          end if;
        end case;
      -- Access configruation area
      else -- slave_i.adr(23) = '1'
        if (slave_i.cyc and slave_i.stb) = '1' then
          -- This is a placeholder
          if (slave_i.we = '1') then
            null;
          else
            null;
          end if;
          -- Controll all pins directly
          r_ack         <= '1';
          slave_o.dat   <= std_logic_vector(to_unsigned(g_rams, 32));
          r_ram_out.cre <= slave_i.dat(31);
          r_ram_out.oen <= slave_i.dat(30);
          r_ram_out.wen <= slave_i.dat(29);
          r_ram_out.cen <= slave_i.dat(28);
          r_ram_out.ubn <= slave_i.dat(26);
          r_ram_out.lbn <= slave_i.dat(25);
          cr_addr_o     <= slave_i.dat(23 downto 0);
        else
          r_ack <= '0';
        end if;
      end if; -- slave_i.adr(23) = '0' then
    end if; -- rising_edge(clk_i)
  end process;

end rtl;
