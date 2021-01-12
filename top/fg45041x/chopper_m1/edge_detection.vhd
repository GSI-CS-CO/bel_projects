library ieee;
use ieee.std_logic_1164.all;


entity edge_detection is
  port (
    input    : in std_logic;
    pos_edge : out std_logic;
    neg_edge : out std_logic;

    reset    : in std_logic;
    Clk      : in std_logic
);
end edge_detection;


architecture edge_detect_arch of edge_detection is
  signal s_sig1, s_sig2:	std_logic;
begin

  edge_detect: process (Clk, Reset)
    begin
      if rising_edge(Clk) then
        if reset = '1' then
          s_sig1 <= '0';
          s_sig2 <= '0';
        else
          s_sig1 <= input;
          s_sig2 <= s_sig1;
        end if;
      end if;
  end process;

  pos_edge <= s_sig1 and not s_sig2;
  neg_edge <= not s_sig1 and s_sig2;

end edge_detect_arch;

