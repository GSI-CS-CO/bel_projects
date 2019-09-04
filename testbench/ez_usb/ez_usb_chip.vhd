library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.file_access.all; 

entity ez_usb_chip is
	port (
      rstn_i    : in  std_logic; 
      ebcyc_o   : out std_logic := '0'; 
      readyn_o  : out std_logic := '0'; 
      fifoadr_i : in  std_logic_vector(1 downto 0); 
      fulln_o   : out std_logic := '1'; 
      emptyn_o  : out std_logic := '0'; 
      sloen_i   : in  std_logic; 
      slrdn_i   : in  std_logic; 
      slwrn_i   : in  std_logic; 
      pktendn_i : in  std_logic; 
      fd_io     : inout std_logic_vector(7 downto 0) := (others => 'Z')
    );
end entity;

architecture simulation of ez_usb_chip is
begin

	main: process 
		variable in_value : integer;
	begin
		wait until rising_edge(rstn_i);
		file_access_init(30);
		while true loop
			in_value := file_access_read(timeout=>0);
			wait until fifoadr_i = "00";
			emptyn_o <= '1'; -- show the master that there is data
			wait until falling_edge(slrdn_i);
			fd_io <= std_logic_vector(to_signed(in_value, 8));
			wait until rising_edge(slrdn_i);
			fd_io <= (others => 'Z');
		end loop;
	end process;

  --USB_chip: process 
  --  variable in_value : integer := 0;
  --  variable out_value : integer := 0;
  --begin
  --  usb_readyn_io <= '1';
  --  wait until rising_edge(rstn_i);

  --  for i in 0 to 50 loop wait until rising_edge(clk_sys); end loop;    

  --  -- My cpu booted, I'll tell the world that I'm ready
  --  usb_readyn_io <= '0';
  --  usb_fulln_i <= '1'; -- I'm never full 
  --  wait until rising_edge(clk_sys);

  --  -- In this simulation I'll read data from a file instead of the D+/D- wires like in the real world
  --  file_access_init(PTS_NUMBER);

  --  while true loop    
  --    counter <= counter + 1;
  --    if counter > 50 then 
  --      counter <= 0;
  --      ebcyc_o <= '0';
  --      for i in 0 to 3 loop 
  --        wait until rising_edge(clk_sys); 
  --      end loop;
  --    end if;

  --    in_value := file_access_read;
  --    while in_value >= 0 loop
  --      ebcyc_o <= '1';
  --      counter <= 0;
  --      wait until rising_edge(clk_sys);
  --      wait until rising_edge(clk_sys);
  --      usb_emptyn_i <= '1'; -- we have data -> de-assert empty line
  --      fd_io <= std_logic_vector(to_signed(in_value, 8));
  --      wait until falling_edge(usb_slrdn_o);  wait until rising_edge(usb_slrdn_o);
  --      in_value := file_access_read;
  --    end loop;

  --    fd_io <= (others => 'Z');
  --    usb_emptyn_i <= '0'; -- we are empty
      
  --    wait until rising_edge(clk_sys) or falling_edge(usb_slwrn_o);

  --    if usb_slwrn_o = '0' then 
  --      counter <= 0;
  --      while usb_pktendn_o = '1' loop
  --        wait until rising_edge(usb_slwrn_o) or falling_edge(usb_pktendn_o);
  --        if usb_pktendn_o = '1' then
  --          out_value := to_integer(unsigned(fd_io));
  --          file_access_write(out_value);
  --        end if;
  --      end loop;
  --      wait until rising_edge(usb_pktendn_o);
  --      file_access_flush;
  --    end if;
  --  end loop;
  --  ebcyc_o <= '1';
  --end process;

end architecture;


