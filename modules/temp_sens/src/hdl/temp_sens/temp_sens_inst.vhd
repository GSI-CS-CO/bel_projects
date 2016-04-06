	component temp_sens is
		port (
			clk        : in  std_logic                    := 'X'; -- clk
			tsdcalo    : out std_logic_vector(7 downto 0);        -- tsdcalo
			tsdcaldone : out std_logic;                           -- tsdcaldone
			ce         : in  std_logic                    := 'X'; -- ce
			clr        : in  std_logic                    := 'X'  -- reset
		);
	end component temp_sens;

	u0 : component temp_sens
		port map (
			clk        => CONNECTED_TO_clk,        --        clk.clk
			tsdcalo    => CONNECTED_TO_tsdcalo,    --    tsdcalo.tsdcalo
			tsdcaldone => CONNECTED_TO_tsdcaldone, -- tsdcaldone.tsdcaldone
			ce         => CONNECTED_TO_ce,         --         ce.ce
			clr        => CONNECTED_TO_clr         --        clr.reset
		);

