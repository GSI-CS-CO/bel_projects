	component arria10_scu4_lvds_rx is
		port (
			ext_fclk      : in  std_logic                    := 'X';             -- export
			ext_loaden    : in  std_logic                    := 'X';             -- export
			ext_coreclock : in  std_logic                    := 'X';             -- export
			rx_in         : in  std_logic_vector(0 downto 0) := (others => 'X'); -- export
			rx_out        : out std_logic_vector(7 downto 0)                     -- export
		);
	end component arria10_scu4_lvds_rx;

	u0 : component arria10_scu4_lvds_rx
		port map (
			ext_fclk      => CONNECTED_TO_ext_fclk,      --      ext_fclk.export
			ext_loaden    => CONNECTED_TO_ext_loaden,    --    ext_loaden.export
			ext_coreclock => CONNECTED_TO_ext_coreclock, -- ext_coreclock.export
			rx_in         => CONNECTED_TO_rx_in,         --         rx_in.export
			rx_out        => CONNECTED_TO_rx_out         --        rx_out.export
		);

