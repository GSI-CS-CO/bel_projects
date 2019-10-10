	component arria10_scu4_lvds_tx is
		port (
			ext_coreclock : in  std_logic                    := 'X';             -- export
			ext_fclk      : in  std_logic                    := 'X';             -- export
			ext_loaden    : in  std_logic                    := 'X';             -- export
			tx_coreclock  : out std_logic;                                       -- export
			tx_in         : in  std_logic_vector(7 downto 0) := (others => 'X'); -- export
			tx_out        : out std_logic_vector(0 downto 0)                     -- export
		);
	end component arria10_scu4_lvds_tx;

	u0 : component arria10_scu4_lvds_tx
		port map (
			ext_coreclock => CONNECTED_TO_ext_coreclock, -- ext_coreclock.export
			ext_fclk      => CONNECTED_TO_ext_fclk,      --      ext_fclk.export
			ext_loaden    => CONNECTED_TO_ext_loaden,    --    ext_loaden.export
			tx_coreclock  => CONNECTED_TO_tx_coreclock,  --  tx_coreclock.export
			tx_in         => CONNECTED_TO_tx_in,         --         tx_in.export
			tx_out        => CONNECTED_TO_tx_out         --        tx_out.export
		);

