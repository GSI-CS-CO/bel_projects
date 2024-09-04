library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

package asmi10_pkg is
	component asmi10_altera_asmi_parallel_231_47nf7hy is
		port (
			clkin         : in  std_logic                     := 'X';             -- clk
			fast_read     : in  std_logic                     := 'X';             -- fast_read
			rden          : in  std_logic                     := 'X';             -- rden
			addr          : in  std_logic_vector(31 downto 0) := (others => 'X'); -- addr
			read_status   : in  std_logic                     := 'X';             -- read_status
			write         : in  std_logic                     := 'X';             -- write
			datain        : in  std_logic_vector(7 downto 0)  := (others => 'X'); -- datain
			shift_bytes   : in  std_logic                     := 'X';             -- shift_bytes
			sector_erase  : in  std_logic                     := 'X';             -- sector_erase
			wren          : in  std_logic                     := 'X';             -- wren
			read_rdid     : in  std_logic                     := 'X';             -- read_rdid
			en4b_addr     : in  std_logic                     := 'X';             -- en4b_addr
			ex4b_addr     : in  std_logic                     := 'X';             -- ex4b_addr
			reset         : in  std_logic                     := 'X';             -- reset
			read_dummyclk : in  std_logic                     := 'X';             -- read_dummyclk
			sce           : in  std_logic_vector(2 downto 0)  := (others => 'X'); -- sce
			dataout       : out std_logic_vector(7 downto 0);                     -- dataout
			busy          : out std_logic;                                        -- busy
			data_valid    : out std_logic;                                        -- data_valid
			status_out    : out std_logic_vector(7 downto 0);                     -- status_out
			illegal_write : out std_logic;                                        -- illegal_write
			illegal_erase : out std_logic;                                        -- illegal_erase
			read_address  : out std_logic_vector(31 downto 0);                    -- read_address
			rdid_out      : out std_logic_vector(7 downto 0)                      -- rdid_out
		);
	end component asmi10_altera_asmi_parallel_231_47nf7hy;

end asmi10_pkg;
