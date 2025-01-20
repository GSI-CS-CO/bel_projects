library ieee;
USE ieee.std_logic_1164.all;
use ieee.math_real.all;
use ieee.numeric_std.all;

library work;
use work.fg_quad_pkg.all;

entity fg_quad_scu_bus is
  generic (
    Base_addr:            unsigned(15 downto 0);
    clk_in_hz:            integer := 50_000_000;        -- 50Mhz
    diag_on_is_1:         integer range 0 to 1 := 0;    -- if 1 then diagnosic information is generated during compilation
    fw_version:           integer range 0 to 65535 := 5;
    ACU:                  boolean := true
    );
  port (
    -- SCUB interface
    Adr_from_SCUB_LA:   in    std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
    Data_from_SCUB_LA:  in    std_logic_vector(15 downto 0);  -- latched data from SCU_Bus
    Ext_Adr_Val:        in    std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:      in    std_logic;                      -- '1' => Rd-Cycle is active
    Ext_Wr_active:      in    std_logic;                      -- '1' => Wr-Cycle is active
    user_rd_active:     out   std_logic;                      -- '1' = read data available at 'Data_to_SCUB'-output
    clk:                in    std_logic;                      -- should be the same clk, used by SCU_Bus_Slave
    sysclk:             in    std_logic;                      -- 12.5 MHz Backplane clock
    nReset:             in    std_logic;
    tag:                in    std_logic_vector(31 downto 0);  -- 32Bit tag from timing
    tag_valid:          in    std_logic;                      -- tag valid
    ext_trigger:        in    std_logic;                      -- external trigger for ramp start
    Rd_Port:            out   std_logic_vector(15 downto 0);  -- output for all read sources of this macro
    Dtack:              out   std_logic;                      -- connect Dtack to SCUB-Macro
    -- fg_quad
    irq:                out   std_logic;

    sw_out:             out   std_logic_vector(31 downto 0);  -- function generator output
    sw_strobe:          out   std_logic
    );
end entity;


architecture fg_quad_scu_bus_arch of fg_quad_scu_bus is

  constant cntrl_reg_adr:     unsigned(15 downto 0) := Base_addr + x"0000";
  constant coeff_a_reg_adr:   unsigned(15 downto 0) := Base_addr + x"0001";
  constant coeff_b_reg_adr:   unsigned(15 downto 0) := Base_addr + x"0002";
  constant broad_start_adr:   unsigned(15 downto 0) := Base_addr + x"0003";
  constant shift_reg_adr:     unsigned(15 downto 0) := Base_addr + x"0004";
  constant start_hi_reg_adr:  unsigned(15 downto 0) := Base_addr + x"0005";
  constant start_lo_reg_adr:  unsigned(15 downto 0) := Base_addr + x"0006";
  constant ramp_cnt_lo_adr:   unsigned(15 downto 0) := Base_addr + x"0007";
  constant ramp_cnt_hi_adr:   unsigned(15 downto 0) := Base_addr + x"0008";
  constant tag_low_reg_adr:   unsigned(15 downto 0) := Base_addr + x"0009";
  constant tag_high_reg_adr:  unsigned(15 downto 0) := Base_addr + x"000a";
  constant fw_version_adr:    unsigned(15 downto 0) := Base_addr + x"000b";


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
  signal  rd_ramp_cnt_lo:   std_logic;
  signal  rd_ramp_cnt_hi:   std_logic;
  signal  wr_ramp_cnt_lo:   std_logic;
  signal  wr_tag_low:       std_logic;
  signal  wr_tag_high:      std_logic;
  signal  rd_tag_low:       std_logic;
  signal  rd_tag_high:      std_logic;
  signal  rd_fw_version:    std_logic;

  signal  fg_is_running:    std_logic;
  signal  ramp_sec_fin:     std_logic;
  signal  state_change_irq: std_logic;
  signal  dreq:             std_logic;
  signal  tag_start:        std_logic;

  type tag_state_type is(IDLE, TAG_RECEIVED);
	signal tag_state	:	tag_state_type;

begin
  quad_fg: fg_quad_datapath
    generic map (
                 ClK_in_hz => clk_in_hz,
                       ACU => ACU
                 )
    port map (
      data_a              => coeff_a_reg,
      data_b              => coeff_b_reg,
      data_c              => start_value_reg(31 downto 0),
      clk                 => clk,
      sysclk              => sysclk,
      nrst                => nReset,
      sync_rst            => fg_cntrl_reg(0),
      a_en                => wr_coeff_a,
      sync_start          => (tag_start or ext_trigger) and fg_cntrl_reg(1),   -- start with tag or from external signal
      load_start          => wr_start_value_h,                  -- when high word was written, load into datapath
      step_sel            => fg_cntrl_reg(12 downto 10),
      shift_b             => to_integer(unsigned(shift_reg(5 downto 0))),
      shift_a             => to_integer(unsigned(shift_reg(11 downto 6))),
      freq_sel            => fg_cntrl_reg(15 downto 13),
      state_change_irq    => state_change_irq,
      dreq                => dreq,
      ramp_sec_fin        => ramp_sec_fin,
      sw_out              => sw_out,
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
      wr_ramp_cnt_lo    <= '0';
      rd_ramp_cnt_lo    <= '0';
      rd_ramp_cnt_hi    <= '0';
      wr_tag_low        <= '0';
      wr_tag_high       <= '0';
      rd_tag_low        <= '0';
      rd_tag_high       <= '0';
      rd_fw_version     <= '0';
      dtack             <= '0';

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
      wr_ramp_cnt_lo    <= '0';
      rd_ramp_cnt_lo    <= '0';
      rd_ramp_cnt_hi    <= '0';
      wr_tag_low        <= '0';
      wr_tag_high       <= '0';
      rd_tag_low        <= '0';
      rd_tag_high       <= '0';
      rd_fw_version     <= '0';
      dtack             <= '0';

      if Ext_Adr_Val = '1' then

        case unsigned(Adr_from_SCUB_LA) is

          when cntrl_reg_adr =>
            if Ext_Wr_active = '1' then
              wr_fg_cntrl <= '1';
              dtack       <= '1';
            end if;
            if Ext_Rd_active = '1' then
              rd_fg_cntrl <= '1';
              dtack       <= '1';
            end if;

          when coeff_a_reg_adr =>
            if Ext_Wr_active = '1' then
              wr_coeff_a  <= '1';
              dtack       <= '1';
            elsif Ext_Rd_active = '1' then
              rd_coeff_a  <= '1';
              dtack       <= '1';
            end if;

          when coeff_b_reg_adr =>
            if Ext_Wr_active = '1' then
              wr_coeff_b  <= '1';
              dtack       <= '1';
            elsif Ext_Rd_active = '1' then
              rd_coeff_b  <= '1';
              dtack       <= '1';
            end if;

          when start_hi_reg_adr =>
            if Ext_Wr_active = '1' then
              wr_start_value_h  <= '1';
              dtack             <= '1';
            elsif Ext_Rd_active = '1' then
              rd_start_value_h  <= '1';
              dtack             <= '1';
            end if;

          when start_lo_reg_adr =>
            if Ext_Wr_active = '1' then
              wr_start_value_l  <= '1';
              dtack             <= '1';
            elsif Ext_Rd_active = '1' then
              rd_start_value_l  <= '1';
              dtack             <= '1';
            end if;

          when shift_reg_adr =>
            if Ext_Wr_active = '1' then
              wr_shift  <= '1';
              dtack     <= '1';
            elsif Ext_Rd_active = '1' then
              rd_shift  <= '1';
              dtack     <= '1';
            end if;

          when ramp_cnt_lo_adr =>
            if Ext_Wr_active = '1' then
              wr_ramp_cnt_lo  <= '1';
              dtack           <= '1';
            elsif Ext_Rd_active = '1' then
              rd_ramp_cnt_lo  <= '1';
              dtack       <= '1';
            end if;

          when ramp_cnt_hi_adr =>
            if Ext_Rd_active = '1' then
              rd_ramp_cnt_hi  <= '1';
              dtack       <= '1';
            end if;

          when tag_low_reg_adr =>
            if Ext_Wr_active = '1' then
              wr_tag_low  <= '1';
              dtack       <= '1';
            elsif Ext_Rd_active = '1' then
              rd_tag_low  <= '1';
              dtack       <= '1';
            end if;

          when tag_high_reg_adr =>
            if Ext_Wr_active = '1' then
              wr_tag_high   <= '1';
              dtack         <= '1';
            elsif Ext_Rd_active = '1' then
              rd_tag_high   <= '1';
              dtack         <= '1';
            end if;

          when fw_version_adr =>
            if Ext_Rd_active = '1' then
              rd_fw_version <= '1';
              dtack         <= '1';
            end if;


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
            rd_ramp_cnt_lo    <= '0';
            rd_ramp_cnt_hi    <= '0';
            wr_tag_low        <= '0';
            wr_tag_high       <= '0';
            rd_tag_low        <= '0';
            rd_tag_high       <= '0';
            rd_fw_version     <= '0';
            dtack             <= '0';
        end case;
      end if;
    end if;
  end process adr_decoder;

tag_comp: process (clk, nReset)
begin
  if nReset = '0' then
    tag_start <= '0';
    tag_state <= IDLE;

  elsif rising_edge(clk) then
    tag_start <= '0';

    case tag_state is
      when IDLE =>
        if tag_valid = '1' then
          tag_state <= TAG_RECEIVED;
        end if;

      when TAG_RECEIVED =>
        if (tag = tag_high_reg & tag_low_reg) then
          tag_start <= '1';
        end if;
        tag_state <= IDLE;

      when others	=>	null;
    end case;

  end if;
end process tag_comp;

-- fg_cntrl_reg(0)            : reset, 1 -> active
-- fg_cntrl_reg(1)            : 1 -> fg enabled, 0 -> fg disabled
-- fg_cntrl_reg(2)            : 1 -> running, 0 -> stopped (ro)
-- fg_cntrl_reg(3)            : 1 -> data request
-- fg_cntrl_reg(9 downto 4)   : virtual fg number (rw)
-- fg_cntrl_reg(12 downto 10) : step value M (wo)
-- fg_cntrl_reg(15 downto 13) : add frequency select (wo)
cntrl_reg: process (clk, nReset, rd_fg_cntrl, fg_cntrl_reg, wr_fg_cntrl)
  variable reset_cnt: unsigned(1 downto 0) := "00";
begin
  if nReset = '0' then
    fg_cntrl_reg    <= (others => '0');
    coeff_a_reg     <= (others => '0');
    coeff_b_reg     <= (others => '0');
    shift_reg       <= (others => '0');
    start_value_reg <= (others => '0');
    ramp_cnt_reg    <= (others => '0');
    tag_low_reg     <= x"babe";
    tag_high_reg    <= x"feed";
    reset_cnt := "00";
  elsif rising_edge(clk) then
    if fg_cntrl_reg(0) = '1' then
      fg_cntrl_reg    <= (others => '0');
      coeff_a_reg     <= (others => '0');
      coeff_b_reg     <= (others => '0');
      shift_reg       <= (others => '0');
      start_value_reg <= (others => '0');
      ramp_cnt_reg    <= (others => '0');
      tag_low_reg     <= x"babe";
      tag_high_reg    <= x"feed";
      reset_cnt := "00";
    else

      if wr_fg_cntrl = '1' then
        fg_cntrl_reg <= Data_from_SCUB_LA;
      end if;

      if wr_coeff_a = '1' then
        coeff_a_reg <= Data_from_SCUB_LA;
      end if;

      if wr_coeff_b = '1' then
        coeff_b_reg <= Data_from_SCUB_LA;
      end if;

      if wr_shift = '1' then
        shift_reg <= Data_from_SCUB_LA;
      end if;

      if wr_start_value_h = '1' then
        start_value_reg(31 downto 16) <= Data_from_SCUB_LA;
      end if;

      if wr_start_value_l = '1' then
        start_value_reg(15 downto 0) <= Data_from_SCUB_LA;
      end if;

      if  fg_cntrl_reg(0) = '1' then
        if reset_cnt < 3 then
          reset_cnt := reset_cnt + 1;
        else
          fg_cntrl_reg(0) <= '0';
          reset_cnt := "00";
        end if;
      end if;

      if ramp_sec_fin = '1' and fg_is_running = '1' then -- increment with every finished ramp section
        ramp_cnt_reg <= ramp_cnt_reg + 1;
      end if;

      if tag_start = '1' and fg_cntrl_reg(1) = '1' then -- disable after Started. Prevents unintended triggering by the next tag.
        fg_cntrl_reg(1) <= '0';
      end if;

      if dreq = '1' then
        fg_cntrl_reg(3) <= '1';
      elsif wr_coeff_a = '1' then
        fg_cntrl_reg(3) <= '0';
      end if;

      if wr_tag_high = '1' then
        tag_high_reg <= Data_from_SCUB_LA;
      end if;

      if wr_tag_low = '1' then
        tag_low_reg <= Data_from_SCUB_LA;
      end if;

      if rd_ramp_cnt_lo = '1' then -- save counter to shadow register
        ramp_cnt_shadow <= ramp_cnt_reg;
      end if;

      if wr_ramp_cnt_lo = '1' then
        ramp_cnt_reg <= (others => '0');
      end if;

    end if;

  end if;
end process;

fg_cntrl_rd_reg <= fg_cntrl_reg(15 downto 13) & fg_cntrl_reg(12 downto 10) &
                    fg_cntrl_reg(9 downto 4) & fg_cntrl_reg(3) & fg_is_running & fg_cntrl_reg(1 downto 0);

user_rd_active <= rd_fg_cntrl or rd_coeff_a or rd_coeff_b or rd_start_value_h
                  or rd_start_value_l or rd_shift or rd_ramp_cnt_lo or rd_ramp_cnt_hi
                  or rd_tag_high or rd_tag_low or rd_fw_version;

Rd_Port <= fg_cntrl_rd_reg                  when rd_fg_cntrl = '1' else
            coeff_a_reg                     when rd_coeff_a = '1' else
            coeff_b_reg                     when rd_coeff_b = '1' else
            start_value_reg(31 downto 16)   when rd_start_value_h = '1' else
            start_value_reg(15 downto 0)    when rd_start_value_l = '1' else
            shift_reg                       when rd_shift = '1' else
            tag_low_reg                     when rd_tag_low = '1' else
            tag_high_reg                    when rd_tag_high = '1' else
            std_logic_vector(ramp_cnt_shadow(31 downto 16))   when rd_ramp_cnt_hi = '1' else
            std_logic_vector(ramp_cnt_shadow(15 downto 0))    when rd_ramp_cnt_lo = '1' else
            std_logic_vector(to_unsigned(fw_version, 16)) when rd_fw_version = '1' else
            x"0000";

irq <= state_change_irq or dreq;

end architecture;
