library ieee;
use ieee.std_logic_1164.all;

entity i8042_kbc is
  port (
    clk           : in std_logic;
    nrst          : in std_logic;
    cs            : in std_logic;
    rd            : in std_logic;
    wr            : in std_logic;
    data          : in std_logic;                       -- command /data register select
    stat_cmd      : in std_logic;
    int           : out std_logic;                      -- irq from kbc
    out_buffer    : out std_logic_vector(7 downto 0);   -- data out port to host
    status_buffer : out std_logic_vector(7 downto 0);   -- status reg
    in_buffer     : in std_logic_vector(7 downto 0);    -- data port from host
    out_port      : out std_logic_vector(7 downto 0);
    in_port      : in std_logic_vector(7 downto 0)
  );
end entity;

architecture i8042_kbc_arch of i8042_kbc is
  constant read_command_byte:   std_logic_vector(7 downto 0) := x"20";
  constant write_command_byte:  std_logic_vector(7 downto 0) := x"60";
  constant selftest:            std_logic_vector(7 downto 0) := x"aa";
  constant interface_test:      std_logic_vector(7 downto 0) := x"ab";
  constant diag_dump:           std_logic_vector(7 downto 0) := x"ac";
  constant disable_kbd:         std_logic_vector(7 downto 0) := x"ad";
  constant enable_kbd:          std_logic_vector(7 downto 0) := x"ae";
  constant read_input_port:     std_logic_vector(7 downto 0) := x"c0";
  constant read_output_port:    std_logic_vector(7 downto 0) := x"d0";
  constant write_output_port:   std_logic_vector(7 downto 0) := x"d1";
  constant read_test_inputs:    std_logic_vector(7 downto 0) := x"e0";
  

  signal status_reg:                std_logic_vector(7 downto 0);
  signal command_byte:              std_logic_vector(7 downto 0);
  signal out_port_reg:              std_logic_vector(7 downto 0);
  signal in_port_reg:               std_logic_vector(7 downto 0);
  signal write_output_port_active:  std_logic;
  signal write_command_active:      std_logic;

begin
 
  status_register: process (clk, nrst)
  begin
    if nrst = '0' then
      status_reg <= "00010000"; -- keyboard inhibited
      command_byte <= "00000000";
      out_port_reg <= x"02";
      in_port_reg <= x"00";
      int <= '0';
      write_output_port_active <= '0';
      write_command_active <= '0';
    elsif rising_edge(clk) then
      status_reg(2) <= command_byte(2); -- system flag
    
      if stat_cmd = '1' and in_buffer = read_command_byte and wr = '1' then
        out_buffer <= command_byte;
        status_reg(0) <= '1'; --  output buffer full
        if command_byte(0) = '1' then
          int <= '1';
        end if;
      end if;
      
      if stat_cmd = '1' and in_buffer = selftest and wr = '1' then
        out_buffer <= x"55";  -- no error detected
        status_reg(0) <= '1'; --  output buffer full
        if command_byte(0) = '1' then
          int <= '1';
        end if;
      end if;
      
      if data = '1' and rd = '1' then
        status_reg(0) <= '0'; -- output buffer empty
        int <= '0';
      end if;
      
      if data = '1' and wr = '1' then
        if write_output_port_active = '1' then
          write_output_port_active <= '0';
          out_port_reg <= in_buffer;
        elsif write_command_active = '1' then
          command_byte <= in_buffer;
          write_command_active <= '0';
        end if;
      end if;
      
      if stat_cmd = '1' and in_buffer = interface_test and wr = '1' then
        out_buffer <= x"00";  -- no error detectet
        status_reg(0) <= '1'; --  output buffer full
        if command_byte(0) = '1' then
          int <= '1';
        end if; 
      end if;
      
      if stat_cmd = '1' and in_buffer = disable_kbd and wr = '1' then
        command_byte(4) <= '1';
        out_buffer <= x"e0";
        status_reg(0) <= '1'; --  output buffer full
        if command_byte(0) = '1' then
          int <= '1';
        end if;
      end if;
      
      if stat_cmd = '1' and in_buffer = enable_kbd and wr = '1' then
        command_byte(4) <= '0'; 
      end if;
      
      if stat_cmd = '1' and in_buffer = read_input_port and wr = '1' then
        out_buffer <= in_port_reg;
        status_reg(0) <= '1'; --  output buffer full
        if command_byte(0) = '1' then
          int <= '1';
        end if;
      end if;
      
      if stat_cmd = '1' and in_buffer = read_output_port and wr = '1' then
        out_buffer <= out_port_reg;
        status_reg(0) <= '1'; --  output buffer full
        if command_byte(0) = '1' then
          int <= '1';
        end if;
      end if;
      
      if stat_cmd = '1' and in_buffer = read_test_inputs and wr = '1' then
        out_buffer <= x"00";
        status_reg(0) <= '1'; --  output buffer full
        if command_byte(0) = '1' then
          int <= '1';
        end if;
      end if;
      
      if stat_cmd = '1' and in_buffer = write_output_port and wr = '1' then
        write_output_port_active <= '1';
      end if;
      
      if stat_cmd = '1' and in_buffer = write_command_byte and wr = '1' then
        write_command_active <= '1';
      end if;
      
      if data = '1' and in_buffer = x"ff" and wr = '1' then -- reset to keyboard
        out_buffer <= x"e0";  -- ack
        status_reg(0) <= '1'; -- output buffer full
        if command_byte(0) = '1' then
          int <= '1';
        end if;
      end if;
      
      if data = '1' and in_buffer = x"ee" and wr = '1' then
        out_buffer <= x"e0";  -- answer from keyboard
        status_reg(0) <= '1'; --  output buffer full
        if command_byte(0) = '1' then
          int <= '1';
        end if;
      end if;
      
      if data = '1' and in_buffer = x"f5" and wr = '1' then
        out_buffer <= x"e0";  -- answer from keyboard
        status_reg(0) <= '1'; --  output buffer full
        if command_byte(0) = '1' then
          int <= '1';
        end if;
      end if;
      
    end if;
  end process;
  
  status_buffer <= status_reg;
  out_port <= out_port_reg;

end architecture;