library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

--  A testbench has no ports.
entity prio_tb is
end prio_tb;

architecture behav of prio_tb is
  --  Declaration of the component that will be instantiated.
  component prio_encoder_256_8 is
  port (
    input : in std_logic_vector(255 downto 0);
    index : out std_logic_vector(7 downto 0);
    valid : out std_logic
  );
  end component;

  --  Specifies which entity is bound with the component.
  for prio_0: prio_encoder_256_8 use entity work.prio_encoder_256_8;
  signal input : std_logic_vector(255 downto 0);
  signal index : std_logic_vector(7 downto 0);
  signal valid : std_logic;


  
begin
  --  Component instantiation.
  prio_0: prio_encoder_256_8 port map (input => input, index => index, valid => valid);

  --  This process does the real job.
  process
    type pattern_type is record
      --  The inputs of the adder.
      input : std_logic_vector(255 downto 0);
      --  The expected outputs of the adder.
      index : std_logic_vector(7 downto 0);
      valid : std_logic;
    end record;
    --  The patterns to apply.
    type pattern_array is array (natural range <>) of pattern_type;
    constant patterns : pattern_array :=
      ((x"0000_0000_0000_0000" & x"0000_0000_0000_0000" & x"0000_0000_0000_0000" & x"0000_0000_0000" & "1000000000100000", "00000101", '1'),
       (x"0000_0000_0000_0000" & x"0000_0000_0000_0000" & x"0000_0000_0000_0000" & x"0000_0000_0000" & "0000000000000001", "00000000", '1'),
       (x"8000_0000_0000_0000" & x"0000_0000_0000_0000" & x"0000_0000_0700_0000" & x"0000_0000_0000" & "0000000000000001", "00000000", '1'),
       (x"0000_0000_0000_0000" & x"8000_0000_0000_0000" & x"0000_0000_0700_0000" & x"0000_0000_0000" & "0000000000000001", "00000000", '1'),
       (x"0000_0000_0000_0000" & x"8000_0000_0000_0000" & x"0000_0000_0700_0000" & x"0000_0000_0003" & "0000000000000000", "00010000", '1'),
       (x"0000_0000_0000_0000" & x"0000_0000_0000_0000" & x"0000_0000_0000_0000" & x"0000_0000_0000" & "0000000000000000", "00000000", '0'));
  begin
    --  Check each pattern.
    for i in patterns'range loop
      --  Set the inputs.
      input <= patterns(i).input;
      --  Wait for the results.
      wait for 1 ns;
      --  Check the outputs.
      assert valid = patterns(i).valid
        report "valid signal wrong" severity error;
      if patterns(i).valid = '1' then
        assert index = patterns(i).index
          report "index wrong" severity error;
      end if;
    end loop;
    assert false report "end of test" severity note;
    --  Wait forever; this will finish the simulation.
    wait;
  end process;
end behav;
