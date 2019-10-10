	component arria10_scu4_lvds_pll is
		port (
			rst      : in  std_logic                    := 'X'; -- reset
			refclk   : in  std_logic                    := 'X'; -- clk
			locked   : out std_logic;                           -- export
			lvds_clk : out std_logic_vector(1 downto 0);        -- lvds_clk
			loaden   : out std_logic_vector(1 downto 0);        -- loaden
			outclk_2 : out std_logic                            -- clk
		);
	end component arria10_scu4_lvds_pll;

	u0 : component arria10_scu4_lvds_pll
		port map (
			rst      => CONNECTED_TO_rst,      --    reset.reset
			refclk   => CONNECTED_TO_refclk,   --   refclk.clk
			locked   => CONNECTED_TO_locked,   --   locked.export
			lvds_clk => CONNECTED_TO_lvds_clk, -- lvds_clk.lvds_clk
			loaden   => CONNECTED_TO_loaden,   --   loaden.loaden
			outclk_2 => CONNECTED_TO_outclk_2  --  outclk2.clk
		);

