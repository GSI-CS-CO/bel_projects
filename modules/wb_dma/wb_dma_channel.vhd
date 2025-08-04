library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wb_dma_pkg.all;
use work.gencores_pkg.all;

entity wb_dma_channel is
generic(
    g_max_transfer_size : integer
);

port(
    clk_i                   : in std_logic;
    rstn_i                  : in std_logic;

    data_i                  : in std_logic_vector(31 downto 0);
    
    write_select_i          : in std_logic_vector(1 downto 0);
    write_enable_i          : in std_logic;

    descriptor_csr_o        : out std_logic_vector(31 downto 0); -- unroll into the relevant data (i.e. transfer size)
    transfer_size_o         : out std_logic_vector(log2_floor(g_max_transfer_size) downto 0);
    source_address_o        : out t_wishbone_address;
    destination_address_o   : out t_wishbone_address;
    next_descriptor_o       : out std_logic_vector(31 downto 0);

    descriptor_active_o     : out std_logic
);
end entity;

architecture wb_dma_channel_arch of wb_dma_channel is

    -- descriptor registers
    signal r_descriptor_csr         : std_logic_vector(31 downto 0);
    signal r_source_address         : t_wishbone_address;
    signal r_destination_address    : t_wishbone_address;
    signal r_next_descriptor        : std_logic_vector(31 downto 0); -- change size to RAM address size
begin

    descriptor_csr_o        <= r_descriptor_csr;
    transfer_size_o         <= r_descriptor_csr(log2_floor(g_max_transfer_size) downto 0);
    source_address_o        <= r_source_address;
    destination_address_o   <= r_destination_address;
    next_descriptor_o       <= r_next_descriptor;

    descriptor_active_o <= '0';


p_write_descriptor: process (clk_i, rstn_i) begin
if(rstn_i = '0') then
    r_descriptor_csr        <= (others => '0');
    r_source_address        <= (others => '0');
    r_destination_address   <= (others => '0');
    r_next_descriptor       <= (others => '0');
elsif rising_edge(clk_i) then
    if(write_enable_i = '1') then
        case write_select_i is
            when b"00" =>
                r_descriptor_csr <= data_i;

            when b"01" =>
                r_source_address <= data_i;

            when b"10" =>
                r_destination_address <= data_i;

            when b"11" =>
                r_next_descriptor <= data_i;

        end case;

    end if;
end if;
end process;

end architecture;