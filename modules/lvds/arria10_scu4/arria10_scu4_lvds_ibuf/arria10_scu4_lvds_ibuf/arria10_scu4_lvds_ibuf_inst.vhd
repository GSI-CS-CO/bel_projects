	component arria10_scu4_lvds_ibuf is
		port (
			dout     : out std_logic_vector(0 downto 0);                    -- export
			pad_in   : in  std_logic_vector(0 downto 0) := (others => 'X'); -- export
			pad_in_b : in  std_logic_vector(0 downto 0) := (others => 'X')  -- export
		);
	end component arria10_scu4_lvds_ibuf;

	u0 : component arria10_scu4_lvds_ibuf
		port map (
			dout     => CONNECTED_TO_dout,     --     dout.export
			pad_in   => CONNECTED_TO_pad_in,   --   pad_in.export
			pad_in_b => CONNECTED_TO_pad_in_b  -- pad_in_b.export
		);

