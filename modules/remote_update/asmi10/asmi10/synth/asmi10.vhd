-- asmi10.vhd

-- Generated using ACDS version 18.1 625

library IEEE;
library asmi10_altera_asmi_parallel_181;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;
use asmi10_altera_asmi_parallel_181.asmi10_pkg.all;

entity asmi10 is
	port (
		addr          : in  std_logic_vector(31 downto 0) := (others => '0'); --          addr.addr
		busy          : out std_logic;                                        --          busy.busy
		clkin         : in  std_logic                     := '0';             --         clkin.clk
		data_valid    : out std_logic;                                        --    data_valid.data_valid
		datain        : in  std_logic_vector(7 downto 0)  := (others => '0'); --        datain.datain
		dataout       : out std_logic_vector(7 downto 0);                     --       dataout.dataout
		en4b_addr     : in  std_logic                     := '0';             --     en4b_addr.en4b_addr
		fast_read     : in  std_logic                     := '0';             --     fast_read.fast_read
		illegal_erase : out std_logic;                                        -- illegal_erase.illegal_erase
		illegal_write : out std_logic;                                        -- illegal_write.illegal_write
		rden          : in  std_logic                     := '0';             --          rden.rden
		rdid_out      : out std_logic_vector(7 downto 0);                     --      rdid_out.rdid_out
		read_address  : out std_logic_vector(31 downto 0);                    --  read_address.read_address
		read_dummyclk : in  std_logic                     := '0';             -- read_dummyclk.read_dummyclk
		read_rdid     : in  std_logic                     := '0';             --     read_rdid.read_rdid
		read_status   : in  std_logic                     := '0';             --   read_status.read_status
		reset         : in  std_logic                     := '0';             --         reset.reset
		sce           : in  std_logic_vector(2 downto 0)  := (others => '0'); --           sce.sce
		sector_erase  : in  std_logic                     := '0';             --  sector_erase.sector_erase
		shift_bytes   : in  std_logic                     := '0';             --   shift_bytes.shift_bytes
		status_out    : out std_logic_vector(7 downto 0);                     --    status_out.status_out
		wren          : in  std_logic                     := '0';             --          wren.wren
		write         : in  std_logic                     := '0'              --         write.write
	);
end entity asmi10;

architecture rtl of asmi10 is
begin

	asmi_parallel_0 : component asmi10_altera_asmi_parallel_181.asmi10_pkg.asmi10_altera_asmi_parallel_181_gdoqleq
		port map (
			clkin         => clkin,         --         clkin.clk
			fast_read     => fast_read,     --     fast_read.fast_read
			rden          => rden,          --          rden.rden
			addr          => addr,          --          addr.addr
			read_status   => read_status,   --   read_status.read_status
			write         => write,         --         write.write
			datain        => datain,        --        datain.datain
			shift_bytes   => shift_bytes,   --   shift_bytes.shift_bytes
			sector_erase  => sector_erase,  --  sector_erase.sector_erase
			wren          => wren,          --          wren.wren
			read_rdid     => read_rdid,     --     read_rdid.read_rdid
			en4b_addr     => en4b_addr,     --     en4b_addr.en4b_addr
			reset         => reset,         --         reset.reset
			read_dummyclk => read_dummyclk, -- read_dummyclk.read_dummyclk
			sce           => sce,           --           sce.sce
			dataout       => dataout,       --       dataout.dataout
			busy          => busy,          --          busy.busy
			data_valid    => data_valid,    --    data_valid.data_valid
			status_out    => status_out,    --    status_out.status_out
			illegal_write => illegal_write, -- illegal_write.illegal_write
			illegal_erase => illegal_erase, -- illegal_erase.illegal_erase
			read_address  => read_address,  --  read_address.read_address
			rdid_out      => rdid_out       --      rdid_out.rdid_out
		);

end architecture rtl; -- of asmi10
