library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;



entity QuD_masken_reg is
    Port ( clk : in STD_LOGIC;
           nReset : in STD_LOGIC;      
           reg_addr : in STD_LOGIC_VECTOR(5 downto 0);
           out_reset: in std_logic;
           write : in std_logic;
           quench_en: in STD_LOGIC_VECTOR (53 downto 0);

           QuD_mask : out STD_LOGIC_VECTOR(53 downto 0));

end QuD_masken_reg;

architecture Arch_QuD_mask of QuD_masken_reg is

    
       type reg_type is array (0 to 23) of std_logic_vector(53 downto 0);
       signal reg : reg_type;
       signal read_addr : std_logic_vector(5 downto 0);
       signal selected_mask: std_logic_vector(53 downto 0);
    
    begin
    
    data_out_process: process (clk, nReset)
        begin
            if nReset = '0' then
                for i in 0 to 23 loop
                    reg(i) <= (others =>'0');
                end loop;
                read_addr <= (others =>'0');
                selected_mask <= (others =>'0');
        
            elsif rising_edge (clk) then
                if out_reset ='1'then
                    for i in 0 to 23 loop
                        reg(i) <= (others =>'0');
                    end loop;
                    read_addr <= (others =>'0');
                   -- selected_mask <= (others =>'0');
                else
                    if write = '1' then               
                        reg(to_integer(unsigned(reg_addr))) <= quench_en;
                        selected_mask   <= (others =>'0');
                    else
                        read_addr <= reg_addr;
                        selected_mask <= reg(to_integer(unsigned(read_addr)));
                    end if;
                end if;
            end if;
    end process data_out_process;
    
    QuD_mask <= selected_mask;
    
    end architecture Arch_QuD_mask;