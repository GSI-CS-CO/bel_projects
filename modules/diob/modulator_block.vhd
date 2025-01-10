library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity modulator_block is
    generic( 
        width: integer:= 16
        );
    port (
        clk : in STD_LOGIC;
        nRST : in STD_LOGIC;
        mod_enable: in STD_LOGIC_VECTOR(width -1 downto 0);
        in_sig : in STD_LOGIC_VECTOR(width -1 downto 0);
        out_sig: out STD_LOGIC_VECTOR(width-1 downto 0)
        );
    end modulator_block;

architecture rtl of modulator_block is

component incoming_signals_modulator is
    Port ( clk : in STD_LOGIC;
           nRST : in STD_LOGIC;
           in_sig : in STD_LOGIC;
           out_sig: out STD_LOGIC)
;
end component incoming_signals_modulator;

signal modulator_out: std_logic_vector(width -1 downto 0);
signal output_mod: std_logic_vector(width -1 downto 0);
begin

   mod_block: for i in 0 to (width-1) generate
        mod_el: incoming_signals_modulator
        port map 
        (clk => clk,
        nRST => nRST,
        in_sig =>in_sig(i),
        out_sig => modulator_out(i)
        );
    end generate mod_block;

process(mod_enable, modulator_out, in_sig)    
begin
    for i in 0 to (width-1) loop
        if mod_enable(i) ='1' then
            output_mod(i) <= modulator_out(i);
        else
            output_mod(i) <= in_sig(i);
        end if;
    end loop;
end process;

out_sig <= output_mod;

end architecture;