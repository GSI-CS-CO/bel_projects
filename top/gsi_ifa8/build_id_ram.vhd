library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library altera_mf;
use altera_mf.altera_mf_components.all;

entity build_id_ram is
  port ( 
    clk:      in std_logic;
    sclr:     in std_logic;
    str:      in std_logic;
    
    build_id_out: out std_logic_vector(15 downto 0)
  );
end entity;

architecture arch of build_id_ram is
  signal rom_addr:        unsigned(8 downto 0);
  signal str_reg:         std_logic_vector(1 downto 0);
  signal str_edge:        std_logic;
  signal s_build_id_out:  std_logic_vector(31 downto 0);
begin
  str_pulse: process(clk)
  begin
    if rising_edge(clk) then
      str_reg(0) <= str;
      str_reg(1) <= str_reg(0);
    end if;
  end process;
  
  str_edge <= str_reg(0) and not str_reg(1);

  ram_cnt: process(clk)
  begin
    if sclr = '1' then
      rom_addr <= (others => '0');
    elsif rising_edge(clk) then
      if str_edge = '1' then
        rom_addr <= rom_addr + 1;
        end if;
    end if;
  end process;

  build_id_rom: altsyncram
  generic map (
    operation_mode  => "ROM",
    width_a         => 32,
    widthad_a       => 8,
    init_file       => "build_id.mif")
  port map (
    clock0          => clk,
    address_a       => std_logic_vector(rom_addr(8 downto 1)),
    q_a             => s_build_id_out,
    q_b             => open);
  
  out_reg: process(clk, sclr)
  begin
    if rising_edge(clk) then
      if str_edge = '1' then
        if rom_addr(0) = '0' then
          build_id_out <= s_build_id_out(31 downto 16);
        else
          build_id_out <= s_build_id_out(15 downto 0);
        end if;
      end if;
    end if;
  end process;

end architecture;