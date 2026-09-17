library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

package a10vs_ip_pkg is
	component a10vs_ip_altera_voltage_sensor_231_dihpyaa is
		generic (
			MEM_TYPE : integer := 0
		);
		port (
			clock_clk                  : in  std_logic                     := 'X';             -- clk
			reset_sink_reset           : in  std_logic                     := 'X';             -- reset
			controller_csr_address     : in  std_logic                     := 'X';             -- address
			controller_csr_read        : in  std_logic                     := 'X';             -- read
			controller_csr_write       : in  std_logic                     := 'X';             -- write
			controller_csr_writedata   : in  std_logic_vector(31 downto 0) := (others => 'X'); -- writedata
			controller_csr_readdata    : out std_logic_vector(31 downto 0);                    -- readdata
			sample_store_csr_address   : in  std_logic_vector(3 downto 0)  := (others => 'X'); -- address
			sample_store_csr_read      : in  std_logic                     := 'X';             -- read
			sample_store_csr_write     : in  std_logic                     := 'X';             -- write
			sample_store_csr_writedata : in  std_logic_vector(31 downto 0) := (others => 'X'); -- writedata
			sample_store_csr_readdata  : out std_logic_vector(31 downto 0);                    -- readdata
			sample_store_irq_irq       : out std_logic                                         -- irq
		);
	end component a10vs_ip_altera_voltage_sensor_231_dihpyaa;

end a10vs_ip_pkg;
