library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity clk_div_n is

    generic ( n: integer range 0 to 125000);

    Port (
        clk_in : in  STD_LOGIC;
        nrst  : in  STD_LOGIC;
        clk_out: out STD_LOGIC
    );
end clk_div_n;

architecture behavioral of clk_div_n is

    signal clk_temp: std_logic;
    signal clk_cnt : integer range 0 to n;
begin
    n_div: process (nrst, clk_in) begin
        if (nrst = '0') then
            clk_temp <= '0';
            clk_cnt <= 0;
        elsif rising_edge(clk_in) then
            if (clk_cnt = n-1) then
                clk_temp <= NOT(clk_temp);
                clk_cnt <= 0;
            else
                clk_cnt <= clk_cnt + 1;
            end if;
        end if;
    end process;
    
    clk_out <= clk_temp;
end behavioral;
