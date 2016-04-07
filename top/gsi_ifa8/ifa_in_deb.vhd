library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity ifa_in_deb is
  port (
    clk:        in std_logic;
    cnt_ena:    in std_logic;
    sclr:       in std_logic;
    sig_i:      in std_logic_vector(7 downto 0);
    sig_o:      out std_logic_vector(7 downto 0));
end entity;

architecture ifa_in_arch of ifa_in_deb is
  component debounce_ifa
    generic
      (
      cnt:     integer := 4 );
    port
      (
       sig:     in std_logic;
       sel:     in std_logic;
       cnt_en:  in std_logic;
       clk:     in std_logic;
       res:     in std_logic;
       sig_deb: out std_logic);
   end component;

begin

  deb_gen:
  for I in 0 to 7 generate
    ena_deb: debounce_ifa
    generic map (cnt => 4)
    port map (  sig => sig_i(I),
                sel => '0',
                cnt_en => cnt_ena,
                clk => clk,
                res => sclr,
                sig_deb => sig_o(I));
  end generate deb_gen;
        
end architecture;
  