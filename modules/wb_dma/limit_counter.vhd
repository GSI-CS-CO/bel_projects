library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wb_dma_pkg.all;

entity limit_counter is
generic(
    g_max_block_size: integer
);
port(
    clk_i : in std_logic;
    rstn_i : in std_logic;

    upper_limit_i : in std_logic_vector(log2_floor(g_max_block_size) downto 0) := (others => '0');
    limit_reached_o : out std_logic;

    cnt_en : in std_logic
);
end entity;

architecture behavioural of limit_counter is
signal block_counter : unsigned(log2_floor(g_max_block_size) downto 0) := (others => '0');
begin
counter: process(clk_i, rstn_i)
begin
    if(rstn_i = '0') then
        block_counter <= (others => '0');
        -- s_block_done <= '0';
    elsif rising_edge(clk_i) then
        -- s_block_done <= '0';
    
        if(cnt_en = '1') then -- CYC instead of STB here, because they are the same line in the signal implementation of this project
            block_counter <= (block_counter + 1);
        end if; -- STB/CYC, STALL

        if(std_logic_vector(block_counter) = upper_limit_i) then --check if the block has been fully transmitted
            -- s_block_done <= '1';
            block_counter <= (others => '0');
        end if; -- cnt
    end if; -- clk
    end process;

limit_reached_o <= '1' when block_counter = unsigned(upper_limit_i) else '0';
end architecture;