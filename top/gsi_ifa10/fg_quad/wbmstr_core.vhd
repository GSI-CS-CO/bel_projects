------------------------------------------------------------------------------
-- Title      : Raw WB master
-- Project    : Wishbone
------------------------------------------------------------------------------
-- File       : wbm_core.vhd
-- Author     : Stefan Rauch
-- Company    : GSI
-- Created    : 2014-09-15
-- Last update: 2014-09-15
-- Platform   : FPGA-generic
-- Standard   : VHDL'93
-------------------------------------------------------------------------------
-- Description: WB message generator
-------------------------------------------------------------------------------
-- Copyright (c) 2014 GSI
-------------------------------------------------------------------------------
--
-- Revisions  :
-- Date        Version  Author          Description
-- 2013-08-10  1.0      mkreider        Created
-- 2014-06-05  1.01     mkreider        fixed bug in sending fsm
-- 2014-06-05  1.1      mkreider        bullet proofed irq inputs with sync chains
-- 2014-09-15  2.0      srauch          stripped version for just generating wb message
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.genram_pkg.all;
use work.wb_irq_pkg.all;

entity wbmstr_core is 
port    (clk_i          : in  std_logic;   -- clock
         rst_n_i        : in  std_logic;   -- reset, active LO
         --msi if
         irq_master_o   : out t_wishbone_master_out;  -- Wishbone msi irq interface
         irq_master_i   : in  t_wishbone_master_in;
         --config 
         -- we assume these as stable, they won't be synced!      
         wb_dst         : in  std_logic_vector(31 downto 0);
         wb_msg         : in  std_logic_vector(31 downto 0);

         strobe         : in std_logic
);
end entity;

architecture behavioral of wbmstr_core is

signal r_cyc       : std_logic;
signal r_stb       : std_logic;
signal r_sel       : std_logic_vector(3 downto 0) := "0011";
signal r_strobe    : std_logic;

type wb_state_type is (idle, low_word, high_word, low_done, high_done);
signal wb_state    : wb_state_type;

signal wb_adr      : unsigned(31 downto 0);

begin


-- always write 
irq_master_o.cyc  <= r_cyc;
irq_master_o.stb  <= r_stb;
irq_master_o.sel  <= r_sel;
irq_master_o.we   <= '1';
wb_adr            <= unsigned(wb_dst);

wb_sm: process(clk_i)
begin
  if rising_edge(clk_i) then
    if(rst_n_i = '0') then
      wb_state  <= idle;
      r_strobe  <= '0';
      r_sel     <= "1111";
    else
      r_strobe  <= '0';
      r_sel     <= "1111";
      case wb_state is
        when idle => 
          if r_cyc = '0' and strobe = '1' then
            r_strobe <= '1';
            wb_state <= high_word;
          end if;
        when high_word =>
          r_sel <= "1100";
          wb_state <= high_done;
        when high_done =>
          r_sel <= "1100";
          if r_cyc = '0' then
            r_strobe <= '1';
            wb_state <= low_word;
          end if;
        when low_word =>
          r_sel <= "0011"; 
          wb_state <= low_done;
        when low_done =>
          r_sel <= "0011";
          if r_cyc = '0' then 
            wb_state <= idle;
          end if;
       end case;
     end if;
  end if;
end process;

-------------------------------------------------------------------------
-- WB master generating IRQ msgs
-------------------------------------------------------------------------
-- send pending MSI IRQs over WB
  wb_irq_master : process(clk_i)
  begin
   if rising_edge(clk_i) then
      if(rst_n_i = '0') then
         r_cyc <= '0';
         r_stb <= '0';
      else
         if r_cyc = '1' then
           if irq_master_i.stall = '0' then
             r_stb <= '0';
           end if;
           if (irq_master_i.ack or irq_master_i.err) = '1' then
             r_cyc <= '0';
           end if;
         else
           r_cyc <= r_strobe;
           r_stb <= r_strobe;
           if wb_state = low_word then
            irq_master_o.dat <= wb_msg(15 downto 0) & wb_msg(15 downto 0);
            irq_master_o.adr <= t_wishbone_address(wb_adr);
           elsif wb_state = high_word then
            irq_master_o.dat <= wb_msg(31 downto 16) & wb_msg(31 downto 16);
            irq_master_o.adr <= t_wishbone_address(wb_adr);
           end if;
         end if;
      end if;
    end if;
  end process;
          
                 

end architecture;

