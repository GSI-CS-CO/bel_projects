library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;
use work.scu_diob_pkg.all;
use IEEE.std_logic_misc.all;

entity threshold_trigger_input is
    port (
        clk : in std_logic;
        nRST : in std_logic;

        in_trigger : in std_logic;
        in_group_dataset : in std_logic_vector(11 downto 0);

        out_trigger : out std_logic;
        out_group_dataset : out std_logic_vector(11 downto 0);

        ready : in std_logic

    );
end threshold_trigger_input;

architecture behavioral of threshold_trigger_input is
    
    type state_m is (idle, wait_read, wait_nread, wait_ntrig); --wait_state,

    signal state : state_m;

begin

    state_proc : process (clk, nRST)
    begin
        if not nRST = '1' then
            --   blm_trigger <= '0';

            state <= idle;
            out_trigger <= '0';
            out_group_dataset <= (others=> '0');

        elsif (clk'EVENT and clk = '1') then

            case (state) is
                when idle =>

                    if in_trigger = '1' then
                        out_trigger <= '1';
                        out_group_dataset <= in_group_dataset;
                        state <= wait_read;
                    end if;

                when wait_read =>
                    if ready = '1' then
                        out_trigger <= '0';
                        state <= wait_nread;
                    end if;

                when wait_nread =>
                    if ready = '0' then
                        state <= wait_ntrig;
                    end if;

                when wait_ntrig =>
                    if in_trigger = '0' then
                        state <= idle;
                    end if;    
                   
                when others => null;

            end case;

        end if;

    end process;

end architecture;    