library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
library work;
use work.qud_pkg.all;


entity QuD_masken_reg is
    Port ( clk : in STD_LOGIC;
           nReset : in STD_LOGIC;      
           reg_addr : in STD_LOGIC_VECTOR(5 downto 0);
           reg_reset: in std_logic;
           write : in std_logic;
           quench_en: in STD_LOGIC_VECTOR (53 downto 0);

           QuD_mask : out t_qud_mask);

end QuD_masken_reg;

architecture Arch_QuD_mask of QuD_masken_reg is

    
     
       signal reg : t_qud_mask;

    
    begin
    
    data_out_process: process (clk, nReset)
        begin
            if nReset = '0' then
                for i in 0 to 23 loop
                    reg(i) <= (others =>'0');
                end loop;
              
        
            elsif rising_edge (clk) then
                if reg_reset ='1'then
                    for i in 0 to 23 loop
                        reg(i) <= (others =>'0');
                    end loop;
                
                else
                    if write = '1' then               
                        reg(to_integer(unsigned(reg_addr))) <= quench_en;
                        
                    end if;
                    
                  --  selected_mask <= reg(to_integer(unsigned(reg_addr)));
                    QuD_mask <= reg;
                end if;
            end if;
    end process data_out_process;
    
  --  QuD_mask <= selected_mask;
    
    end architecture Arch_QuD_mask;