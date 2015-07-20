library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.aux_functions_pkg.all;

entity tmr_scu_bus is
  generic (
    Base_addr:      unsigned(15 downto 0);
    diag_on_is_1:   integer range 0 to 1 := 0);   -- if 1 then diagnosic information is generated during compilation
  port (
    clk:                in std_logic;
    nrst:               in std_logic;
    
    tmr_irq:            out std_logic;
    
    -- SCUB interface
    Adr_from_SCUB_LA:   in    std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
    Data_from_SCUB_LA:  in    std_logic_vector(15 downto 0);  -- latched data from SCU_Bus 
    Ext_Adr_Val:        in    std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:      in    std_logic;                      -- '1' => Rd-Cycle is active
    Ext_Wr_active:      in    std_logic;                      -- '1' => Wr-Cycle is active
    user_rd_active:     out   std_logic;                      -- '1' = read data available at 'Data_to_SCUB'-output
    Data_to_SCUB:       out   std_logic_vector(15 downto 0);  -- connect read sources to SCUB-Macro
    Dtack_to_SCUB:      out   std_logic                       -- connect Dtack to SCUB-Macro
    );

end entity;


architecture tmr_scu_bus_arch of tmr_scu_bus is

constant cntrl_reg_adr:       unsigned(15 downto 0) := Base_addr + x"0000";
constant tmr_irq_cnt_reg_adr: unsigned(15 downto 0) := Base_addr + x"0001";
constant tmr_valuel_adr:      unsigned(15 downto 0) := Base_addr + x"0002";
constant tmr_valueh_adr:      unsigned(15 downto 0) := Base_addr + x"0003";
constant tmr_repeat_adr:      unsigned(15 downto 0) := Base_addr + x"0004";

signal dtack:             std_logic;
signal wr_tmr_cntrl:      std_logic;
signal rd_tmr_cntrl:      std_logic;
signal rd_tmr_irq_cnt:    std_logic;
signal tmr_cntrl_reg:     std_logic_vector(15 downto 0);
signal tmr_cntrl_rd_reg:  std_logic_vector(15 downto 0);
signal tmr_valuel_reg:    std_logic_vector(15 downto 0);
signal tmr_valueh_reg:    std_logic_vector(15 downto 0);
signal rd_tmr_valuel:     std_logic;
signal rd_tmr_valueh:     std_logic;
signal wr_tmr_valuel:     std_logic;
signal wr_tmr_valueh:     std_logic;
signal wr_tmr_repeat:     std_logic;
signal rd_tmr_repeat:     std_logic;
signal tmr_repeat_reg:    std_logic_vector(15 downto 0);
signal irqcnt:            unsigned(32 downto 0);
signal tmr_irq_cnt:       unsigned(15 downto 0);


begin

tmr_decoder: process (clk, nrst)
begin
  if nrst = '0' then
    wr_tmr_cntrl    <= '0';
    rd_tmr_cntrl    <= '0';
    rd_tmr_irq_cnt  <= '0';
    rd_tmr_valuel   <= '0';
    rd_tmr_valueh   <= '0';
    wr_tmr_valuel   <= '0';
    wr_tmr_valueh   <= '0';
    wr_tmr_repeat   <= '0';
    rd_tmr_repeat   <= '0';
    dtack           <= '0';
  elsif rising_edge(clk) then
    wr_tmr_cntrl    <= '0';
    rd_tmr_cntrl    <= '0';
    rd_tmr_irq_cnt  <= '0';
    rd_tmr_valuel   <= '0';
    rd_tmr_valueh   <= '0';
    wr_tmr_valuel   <= '0';
    wr_tmr_valueh   <= '0';
    wr_tmr_repeat   <= '0';
    rd_tmr_repeat   <= '0';
    dtack           <= '0';
    
    if Ext_Adr_Val = '1' then
      case unsigned(Adr_from_SCUB_LA) is
        when cntrl_reg_adr =>
          if Ext_Wr_active = '1' then
            wr_tmr_cntrl  <= '1';
            dtack         <= '1';
          end if;
          if Ext_Rd_active = '1' then
            rd_tmr_cntrl  <= '1';
            dtack         <= '1';
          end if;
        
        when tmr_irq_cnt_reg_adr =>
          if Ext_Rd_active = '1' then
            rd_tmr_irq_cnt  <= '1';
            dtack           <= '1';
          end if;
        
        when tmr_valuel_adr =>
          if Ext_Wr_active = '1' then
            wr_tmr_valuel  <= '1';
            dtack         <= '1';
          end if;
          if Ext_Rd_active = '1' then
            rd_tmr_valuel  <= '1';
            dtack         <= '1';
          end if;
          
        when tmr_valueh_adr =>
          if Ext_Wr_active = '1' then
            wr_tmr_valueh  <= '1';
            dtack         <= '1';
          end if;
          if Ext_Rd_active = '1' then
            rd_tmr_valueh  <= '1';
            dtack         <= '1';
          end if;
          
        when tmr_repeat_adr =>
          if Ext_Wr_active = '1' then
            wr_tmr_repeat <= '1';
            dtack <= '1';
          end if;
          if Ext_Rd_active = '1' then
            rd_tmr_repeat <= '1';
            dtack <= '1';
          end if;
          
        when others =>
          wr_tmr_cntrl    <= '0';
          rd_tmr_cntrl    <= '0';
          rd_tmr_irq_cnt  <= '0';
          rd_tmr_valuel   <= '0';
          rd_tmr_valueh   <= '0';
          wr_tmr_valuel   <= '0';
          wr_tmr_valueh   <= '0';
          wr_tmr_repeat   <= '0';
          rd_tmr_repeat   <= '0';
          dtack           <= '0';
      end case;
    end if;
  end if;
end process tmr_decoder;

cntrl_reg: process (clk, nrst, tmr_cntrl_reg, rd_tmr_cntrl, wr_tmr_cntrl, wr_tmr_valuel, wr_tmr_valueh)
  variable reset_cnt: unsigned(1 downto 0) := "00";
begin
  if nrst = '0' then
    tmr_cntrl_reg <= x"0000";
    tmr_valuel_reg <= x"0000";
    tmr_valueh_reg <= x"0000";
    reset_cnt := "00";
  elsif rising_edge(clk) then
    if tmr_cntrl_reg(0) = '1' then
      tmr_cntrl_reg <= x"0000";
      tmr_valuel_reg <= x"0000";
      tmr_valueh_reg <= x"0000";
      reset_cnt := "00";
    else
      if wr_tmr_cntrl = '1' then
        tmr_cntrl_reg <= Data_from_SCUB_LA;
      elsif wr_tmr_valuel = '1' then
        tmr_valuel_reg <= Data_from_SCUB_LA;
      elsif wr_tmr_valueh = '1' then
        tmr_valueh_reg <= Data_from_SCUB_LA;
      elsif wr_tmr_repeat = '1' then
        tmr_repeat_reg <= Data_from_SCUB_LA;
      elsif  tmr_cntrl_reg(0) = '1' then
        if reset_cnt < 3 then
          reset_cnt := reset_cnt + 1;
        else
          tmr_cntrl_reg(0) <= '0';
          reset_cnt := "00";
        end if;
      end if;
    end if;
  end if;
end process;

timer_irq: process (clk, nrst, tmr_cntrl_reg, tmr_valuel_reg, tmr_valueh_reg)
  begin
    if nrst = '0' then
      irqcnt <= (others => '0');
      tmr_irq_cnt <= x"0000";
    elsif rising_edge(clk) then
      if tmr_cntrl_reg(0) = '1' then
        irqcnt <= '0' & unsigned(tmr_valueh_reg & tmr_valuel_reg);
        tmr_irq_cnt <= x"0000";
      else
        if irqcnt(irqcnt'high) = '1' then
          tmr_irq_cnt <= tmr_irq_cnt + 1; -- increment with every interrupt
          irqcnt <= '0' & unsigned(tmr_valueh_reg & tmr_valuel_reg);
        elsif tmr_cntrl_reg(1) = '1' and tmr_irq_cnt < unsigned(tmr_repeat_reg) then
          irqcnt <= irqcnt - 1;
        end if;
       end if;
    end if;
  end process;
 
tmr_cntrl_rd_reg <=  tmr_cntrl_reg;

    
user_rd_active <= rd_tmr_cntrl or rd_tmr_irq_cnt;

Data_to_SCUB <= std_logic_vector(tmr_irq_cnt) when rd_tmr_irq_cnt = '1' else
                tmr_cntrl_rd_reg              when rd_tmr_cntrl = '1' else
                tmr_valuel_reg                when rd_tmr_valuel = '1' else
                tmr_valueh_reg                when rd_tmr_valueh = '1' else
                tmr_repeat_reg                when rd_tmr_repeat = '1' else
                x"0000";
                
Dtack_to_SCUB <= dtack;

tmr_irq <= irqcnt(irqcnt'high);

end architecture;
