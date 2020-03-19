library ieee;
USE ieee.std_logic_1164.all;
use ieee.math_real.all;
use ieee.numeric_std.all;
use ieee.std_logic_misc.all;

library work;
use work.fg_quad_pkg.all;

entity fg_quad_ifa is
  generic (
    clk_in_hz:            integer := 50_000_000;        -- 50Mhz
    diag_on_is_1:         integer range 0 to 1 := 0;    -- if 1 then diagnosic information is generated during compilation
    fw_version:           integer range 0 to 65535 := 2
    );
  port (
    -- ifa interface
    fc:                 in    std_logic_vector(7 downto 0);   -- latched function code from mil interface
    data_i:             in    std_logic_vector(15 downto 0);  -- latched data from mil interface
    fc_str:             in    std_logic;                      -- '1' => fc is valid


    clk:                in    std_logic;                      -- should be the same clk, used by SCU_Bus_Slave
    nReset:             in    std_logic;
    ext_trigger:        in    std_logic;                      -- external trigger for ramp start

    user_rd_active:     out   std_logic;                      -- '1' = read data available
    Rd_Port:            out   std_logic_vector(15 downto 0);  -- output for all read sources of this macro

    -- fg_quad
    nirq:               out   std_logic;

    sw_out:             out   std_logic_vector(31 downto 8);  -- function generator output
    sw_strobe:          out   std_logic;
    gate_o_bc:          out   std_logic;
    fg_version:         out   std_logic_vector(6 downto 0)
   );
end entity;


architecture fg_quad_scu_bus_arch of fg_quad_ifa is
  constant c_irq_cnt:         integer := 250;

  constant cntrl_wr_fc:       unsigned(7 downto 0) := x"14";
  constant coeff_a_wr_fc:     unsigned(7 downto 0) := x"15";
  constant coeff_b_wr_fc:     unsigned(7 downto 0) := x"16";
  constant shift_wr_fc:       unsigned(7 downto 0) := x"17";
  constant start_hi_wr_fc:    unsigned(7 downto 0) := x"18";
  constant start_lo_wr_fc:    unsigned(7 downto 0) := x"19";
  constant brdcst_wr_fc:      unsigned(7 downto 0) := x"20";
  constant irq_act_wr_fc:     unsigned(7 downto 0) := x"21";

  constant cntrl_rd_fc:       unsigned(7 downto 0) := x"a0";
  constant coeff_a_rd_fc:     unsigned(7 downto 0) := x"a1";
  constant coeff_b_rd_fc:     unsigned(7 downto 0) := x"a2";
  constant shift_rd_fc:       unsigned(7 downto 0) := x"a3";
  constant start_hi_rd_fc:    unsigned(7 downto 0) := x"a4";
  constant start_lo_rd_fc:    unsigned(7 downto 0) := x"a5";
  constant fw_version_rd_fc:  unsigned(7 downto 0) := x"a6";
  constant irq_act_rd_fc:     unsigned(7 downto 0) := x"a7";


  signal  fg_cntrl_reg:     std_logic_vector(15 downto 0);
  signal  fg_cntrl_rd_reg:  std_logic_vector(15 downto 0);
  signal  coeff_a_reg:      std_logic_vector(15 downto 0);
  signal  coeff_b_reg:      std_logic_vector(15 downto 0);
  signal  start_value_reg:  std_logic_vector(31 downto 0);
  signal  shift_reg:        std_logic_vector(15 downto 0);
  signal  ramp_cnt_reg:     unsigned(31 downto 0);
  signal  ramp_cnt_shadow:  unsigned(31 downto 0);
  signal  tag_low_reg:      std_logic_vector(15 downto 0);
  signal  tag_high_reg:     std_logic_vector(15 downto 0);
  signal  irq_act_reg:      std_logic_vector(15 downto 0);

  signal  wr_fg_cntrl:      std_logic;
  signal  rd_fg_cntrl:      std_logic;
  signal  wr_coeff_a:       std_logic;
  signal  rd_coeff_a:       std_logic;
  signal  wr_coeff_b:       std_logic;
  signal  rd_coeff_b:       std_logic;
  signal  wr_start_value_h: std_logic;
  signal  rd_start_value_h: std_logic;
  signal  wr_start_value_l: std_logic;
  signal  rd_start_value_l: std_logic;
  signal  wr_shift:         std_logic;
  signal  rd_shift:         std_logic;
  signal  wr_brc_start:     std_logic;
  signal  rd_fw_version:    std_logic;
  signal  wr_irq_act:       std_logic;
  signal  rd_irq_act:       std_logic;

  signal  fg_is_running:    std_logic;
  signal  ramp_sec_fin:     std_logic;
  signal  state_change_irq: std_logic;
  signal  dreq:             std_logic;
  signal  tag_start:        std_logic;

  type tag_state_type is(IDLE, TAG_RECEIVED);
	signal tag_state	:	tag_state_type;

  signal s_sw_out:          std_logic_vector(31 downto 0);

  type blk_type is (idle, cntrl, coeff_a, coeff_b, start_l, start_h, shift, end_blk_mode);
  signal blk_sm         : blk_type;
  signal blk_cntrl_wr   : std_logic;
  signal blk_coeff_a_wr : std_logic;
  signal blk_coeff_b_wr : std_logic;
  signal blk_shift_wr   : std_logic;
  signal blk_start_l_wr : std_logic;
  signal blk_start_h_wr : std_logic;

  signal fc_edge1    : std_logic;
  signal fc_edge2    : std_logic;
  signal fc_str_edge : std_logic;

begin
  quad_fg: fg_quad_datapath
    generic map (
      ClK_in_hz => clk_in_hz,
      ACU => false
      )
    port map (
      data_a              => coeff_a_reg,
      data_b              => coeff_b_reg,
      data_c              => start_value_reg(31 downto 0),
      clk                 => clk,
      sysclk              => '0',
      nrst                => nReset,
      sync_rst            => fg_cntrl_reg(0),
      a_en                => wr_coeff_a or blk_start_h_wr,
      sync_start          => (wr_brc_start or ext_trigger) and fg_cntrl_reg(1),   -- start with broadcast or from external signal
      load_start          => wr_start_value_h or blk_start_h_wr,                  -- when high word was written, load into datapath
      step_sel            => fg_cntrl_reg(12 downto 10),
      shift_b             => to_integer(unsigned(shift_reg(5 downto 0))),
      shift_a             => to_integer(unsigned(shift_reg(11 downto 6))),
      freq_sel            => fg_cntrl_reg(15 downto 13),
      state_change_irq    => state_change_irq,
      dreq                => dreq,
      ramp_sec_fin        => ramp_sec_fin,
      sw_out              => s_sw_out,
      sw_strobe           => sw_strobe,
      fg_is_running       => fg_is_running
    );


  adr_decoder: process (clk, nReset)
  begin
    if nReset = '0' then
      wr_fg_cntrl       <= '0';
      rd_fg_cntrl       <= '0';
      wr_coeff_a        <= '0';
      rd_coeff_a        <= '0';
      wr_coeff_b        <= '0';
      rd_coeff_b        <= '0';
      wr_start_value_h  <= '0';
      rd_start_value_h  <= '0';
      wr_start_value_l  <= '0';
      rd_start_value_l  <= '0';
      wr_shift          <= '0';
      rd_shift          <= '0';
      rd_fw_version     <= '0';
      wr_brc_start      <= '0';
      rd_irq_act        <= '0';
      wr_irq_act        <= '0';


    elsif rising_edge(clk) then
      wr_fg_cntrl       <= '0';
      rd_fg_cntrl       <= '0';
      wr_coeff_a        <= '0';
      rd_coeff_a        <= '0';
      wr_coeff_b        <= '0';
      rd_coeff_b        <= '0';
      wr_start_value_h  <= '0';
      rd_start_value_h  <= '0';
      wr_start_value_l  <= '0';
      rd_start_value_l  <= '0';
      wr_shift          <= '0';
      rd_shift          <= '0';
      rd_fw_version     <= '0';
      wr_brc_start      <= '0';
      rd_irq_act        <= '0';
      wr_irq_act        <= '0';


      if fc_str = '1' then

        case unsigned(fc) is

          when cntrl_wr_fc =>
            wr_fg_cntrl <= '1';
          when cntrl_rd_fc =>
            rd_fg_cntrl <= '1';

          when coeff_a_wr_fc =>
            wr_coeff_a  <= '1';
          when coeff_a_rd_fc =>
            rd_coeff_a  <= '1';

          when coeff_b_wr_fc =>
            wr_coeff_b  <= '1';
          when coeff_b_rd_fc =>
            rd_coeff_b  <= '1';

          when start_hi_wr_fc =>
            wr_start_value_h  <= '1';
          when start_hi_rd_fc =>
            rd_start_value_h  <= '1';

          when start_lo_wr_fc =>
            wr_start_value_l  <= '1';
          when start_lo_rd_fc =>
            rd_start_value_l  <= '1';

          when shift_wr_fc =>
            wr_shift  <= '1';
          when shift_rd_fc =>
            rd_shift  <= '1';

          when fw_version_rd_fc =>
            rd_fw_version <= '1';

          when brdcst_wr_fc =>
            wr_brc_start <= '1';

          when irq_act_rd_fc =>
            rd_irq_act <= '1';
          when irq_act_wr_fc =>
            wr_irq_act <= '1';
          when others =>
            wr_fg_cntrl       <= '0';
            rd_fg_cntrl       <= '0';
            wr_coeff_a        <= '0';
            rd_coeff_a        <= '0';
            wr_coeff_b        <= '0';
            rd_coeff_b        <= '0';
            wr_start_value_h  <= '0';
            rd_start_value_h  <= '0';
            wr_start_value_l  <= '0';
            rd_start_value_l  <= '0';
            wr_shift          <= '0';
            rd_shift          <= '0';
            rd_fw_version     <= '0';
            wr_brc_start      <= '0';
            rd_irq_act        <= '0';
            wr_irq_act        <= '0';
        end case;
      end if;
    end if;
  end process adr_decoder;

  fc_edge: process(clk, nreset)
  begin
    if rising_edge(clk) then
      fc_edge1 <= fc_str;
      fc_edge2 <= fc_edge1;
    end if;
  end process;
  fc_str_edge <= not fc_edge2 and fc_edge1;

  block_mode: process(clk, nreset)
  begin
    if nreset = '0' then
        blk_sm         <= idle;
        blk_cntrl_wr   <= '0';
        blk_coeff_a_wr <= '0';
        blk_coeff_b_wr <= '0';
        blk_shift_wr   <= '0';
        blk_start_l_wr <= '0';
        blk_start_h_wr <= '0';
    elsif rising_edge(clk) then
      if fg_cntrl_reg(0) = '1' then
        blk_sm <= idle;
      end if;

      blk_cntrl_wr   <= '0';
      blk_coeff_a_wr <= '0';
      blk_coeff_b_wr <= '0';
      blk_shift_wr   <= '0';
      blk_start_l_wr <= '0';
      blk_start_h_wr <= '0';

      case blk_sm is
        when idle =>
          if fc_str_edge = '1' and fc = x"6b" then
            blk_sm <= cntrl;
          end if;

        when cntrl =>
          blk_cntrl_wr <= '1';
          blk_sm <= coeff_a;

        when coeff_a =>
          if fc_str_edge = '1' and fc = x"6b" then
            blk_coeff_a_wr <= '1';
            blk_sm <= coeff_b;
          end if;

        when coeff_b =>
          if fc_str_edge = '1' and fc = x"6b" then
            blk_coeff_b_wr <= '1';
            blk_sm <= shift;
          end if;

        when shift =>
          if fc_str_edge = '1' and fc = x"6b" then
            blk_shift_wr <= '1';
            blk_sm <= start_l;
          end if;

        when start_l =>
          if fc_str_edge = '1' and fc = x"6b" then
            blk_start_l_wr <= '1';
            blk_sm <= start_h;
          end if;

        when start_h =>
          if fc_str_edge = '1' and fc = x"6b" then
            blk_start_h_wr <= '1';
            blk_sm <= end_blk_mode;
          end if;

        when end_blk_mode =>
          if fc_str_edge = '1' and fc = x"6b" then
            blk_sm <= idle;
          end if;

      end case;
    end if;
  end process;




-- fg_cntrl_reg(0)            : reset, 1 -> active
-- fg_cntrl_reg(1)            : 1 -> fg enabled, 0 -> fg disabled
-- fg_cntrl_reg(2)            : 1 -> running, 0 -> stopped (ro)
-- fg_cntrl_reg(3)            :
-- fg_cntrl_reg(9 downto 4)   : virtual fg number (rw)
-- fg_cntrl_reg(12 downto 10) : step value M (wo)
-- fg_cntrl_reg(15 downto 13) : add frequency select (wo)
--
-- mirror some bits to save register access over dev bus
-- irq_act_reg(0)             : dreq, 1 -> active
-- irq_act_reg(1)             : state_change_irq, 1 -> active
-- irq_act_reg(2)             : 1 -> running, 0 -> stopped (ro)
-- irq_act_reg(3)             :
-- irq_act_reg(9 downto 4)    : virtual fg number (rw)
cntrl_reg: process (clk, nReset, rd_fg_cntrl, fg_cntrl_reg, wr_fg_cntrl)
begin
  if nReset = '0' then
    fg_cntrl_reg    <= (others => '0');
    coeff_a_reg     <= (others => '0');
    coeff_b_reg     <= (others => '0');
    shift_reg       <= (others => '0');
    start_value_reg <= (others => '0');
    irq_act_reg     <= (others => '0');
  elsif rising_edge(clk) then
    if fg_cntrl_reg(0) = '1' then
      fg_cntrl_reg    <= (others => '0');
      coeff_a_reg     <= (others => '0');
      coeff_b_reg     <= (others => '0');
      shift_reg       <= (others => '0');
      start_value_reg <= (others => '0');
      irq_act_reg     <= (others => '0');
    else

      if wr_fg_cntrl = '1' or blk_cntrl_wr = '1' then
        fg_cntrl_reg <= data_i;
      end if;

      if wr_coeff_a = '1' or blk_coeff_a_wr = '1' then
        coeff_a_reg <= data_i;
      end if;

      if wr_coeff_b = '1' or blk_coeff_b_wr = '1' then
        coeff_b_reg <= data_i;
      end if;

      if wr_shift = '1'  or blk_shift_wr = '1' then
        shift_reg <= data_i;
      end if;

      if wr_start_value_h = '1'  or blk_start_h_wr = '1' then
        start_value_reg(31 downto 16) <= data_i;
      end if;

      if wr_start_value_l = '1' or blk_start_l_wr = '1' then
        start_value_reg(15 downto 0) <= data_i;
      end if;

      if wr_brc_start = '1' and fg_cntrl_reg(1) = '1' then -- disable after Started. Prevents unintended triggering by the next broadcast.
        fg_cntrl_reg(1) <= '0';
      end if;

      if dreq = '1' then
        irq_act_reg(0) <= '1';
      elsif state_change_irq = '1' then
        irq_act_reg(1) <= '1';
      elsif wr_irq_act = '1' then
        -- clear the pending irqs
        irq_act_reg(1 downto 0) <= "00";
      end if;
      irq_act_reg(2)          <= fg_is_running;
      irq_act_reg(9 downto 4) <= fg_cntrl_reg(9 downto 4);

    end if;

  end if;
end process;


fg_cntrl_rd_reg <= fg_cntrl_reg(15 downto 13) & fg_cntrl_reg(12 downto 10) &
                    fg_cntrl_reg(9 downto 4) & '0' & fg_is_running & fg_cntrl_reg(1 downto 0);


rd_act: process (clk)
-- generate a pulse for the mil encoder which goes only low, when the data in the rd port register changes
variable user_rd_act: std_logic;
begin
  if rising_edge(clk) then
    user_rd_act := '0';

    if fc_str = '1' then
      user_rd_act := rd_fg_cntrl or rd_coeff_a or rd_coeff_b or rd_start_value_h
                  or rd_start_value_l or rd_shift or rd_fw_version or rd_irq_act;
    end if;
  end if;
  user_rd_active <= user_rd_act;
end process;

rd_mux: process (clk, nreset)
begin
  if nreset = '0' then
    Rd_Port <= (others => '0');
  elsif rising_edge(clk) then

    if rd_fg_cntrl = '1' then
      Rd_Port <= fg_cntrl_rd_reg;
    elsif rd_coeff_a = '1' then
      Rd_Port <= coeff_a_reg;
    elsif rd_coeff_b = '1' then
      Rd_Port <= coeff_b_reg;
    elsif rd_start_value_h = '1' then
      Rd_Port <= start_value_reg(31 downto 16);
    elsif rd_start_value_l = '1' then
      Rd_Port <= start_value_reg(15 downto 0);
    elsif rd_shift = '1' then
      Rd_Port <= shift_reg;
    elsif rd_fw_version = '1' then
      Rd_Port <= std_logic_vector(to_unsigned(fw_version, 16));
    elsif rd_irq_act = '1' then
      Rd_Port <= irq_act_reg;
    end if;
  end if;
end process;

nirq        <= not or_reduce(irq_act_reg(1 downto 0)); -- signal as long as one irq is active
fg_version  <= std_logic_vector(to_unsigned(fw_version, 7));
sw_out      <= s_sw_out(31 downto 8); -- only 24 Bit are needed for the IFA8

end architecture;
