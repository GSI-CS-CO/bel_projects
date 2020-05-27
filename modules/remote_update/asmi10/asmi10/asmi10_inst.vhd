	component asmi10 is
		port (
			addr          : in  std_logic_vector(31 downto 0) := (others => 'X'); -- addr
			busy          : out std_logic;                                        -- busy
			clkin         : in  std_logic                     := 'X';             -- clk
			data_valid    : out std_logic;                                        -- data_valid
			datain        : in  std_logic_vector(7 downto 0)  := (others => 'X'); -- datain
			dataout       : out std_logic_vector(7 downto 0);                     -- dataout
			en4b_addr     : in  std_logic                     := 'X';             -- en4b_addr
			fast_read     : in  std_logic                     := 'X';             -- fast_read
			illegal_erase : out std_logic;                                        -- illegal_erase
			illegal_write : out std_logic;                                        -- illegal_write
			rden          : in  std_logic                     := 'X';             -- rden
			rdid_out      : out std_logic_vector(7 downto 0);                     -- rdid_out
			read_address  : out std_logic_vector(31 downto 0);                    -- read_address
			read_rdid     : in  std_logic                     := 'X';             -- read_rdid
			read_status   : in  std_logic                     := 'X';             -- read_status
			reset         : in  std_logic                     := 'X';             -- reset
			sce           : in  std_logic_vector(2 downto 0)  := (others => 'X'); -- sce
			sector_erase  : in  std_logic                     := 'X';             -- sector_erase
			shift_bytes   : in  std_logic                     := 'X';             -- shift_bytes
			status_out    : out std_logic_vector(7 downto 0);                     -- status_out
			wren          : in  std_logic                     := 'X';             -- wren
			write         : in  std_logic                     := 'X';             -- write
			read_dummyclk : in  std_logic                     := 'X'              -- read_dummyclk
		);
	end component asmi10;

	u0 : component asmi10
		port map (
			addr          => CONNECTED_TO_addr,          --          addr.addr
			busy          => CONNECTED_TO_busy,          --          busy.busy
			clkin         => CONNECTED_TO_clkin,         --         clkin.clk
			data_valid    => CONNECTED_TO_data_valid,    --    data_valid.data_valid
			datain        => CONNECTED_TO_datain,        --        datain.datain
			dataout       => CONNECTED_TO_dataout,       --       dataout.dataout
			en4b_addr     => CONNECTED_TO_en4b_addr,     --     en4b_addr.en4b_addr
			fast_read     => CONNECTED_TO_fast_read,     --     fast_read.fast_read
			illegal_erase => CONNECTED_TO_illegal_erase, -- illegal_erase.illegal_erase
			illegal_write => CONNECTED_TO_illegal_write, -- illegal_write.illegal_write
			rden          => CONNECTED_TO_rden,          --          rden.rden
			rdid_out      => CONNECTED_TO_rdid_out,      --      rdid_out.rdid_out
			read_address  => CONNECTED_TO_read_address,  --  read_address.read_address
			read_rdid     => CONNECTED_TO_read_rdid,     --     read_rdid.read_rdid
			read_status   => CONNECTED_TO_read_status,   --   read_status.read_status
			reset         => CONNECTED_TO_reset,         --         reset.reset
			sce           => CONNECTED_TO_sce,           --           sce.sce
			sector_erase  => CONNECTED_TO_sector_erase,  --  sector_erase.sector_erase
			shift_bytes   => CONNECTED_TO_shift_bytes,   --   shift_bytes.shift_bytes
			status_out    => CONNECTED_TO_status_out,    --    status_out.status_out
			wren          => CONNECTED_TO_wren,          --          wren.wren
			write         => CONNECTED_TO_write,         --         write.write
			read_dummyclk => CONNECTED_TO_read_dummyclk  -- read_dummyclk.read_dummyclk
		);

