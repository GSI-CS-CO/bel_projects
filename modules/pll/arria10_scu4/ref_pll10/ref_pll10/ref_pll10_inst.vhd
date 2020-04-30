	component ref_pll10 is
		port (
			cntsel           : in  std_logic_vector(4 downto 0) := (others => 'X'); -- cntsel
			loaden           : out std_logic_vector(1 downto 0);                    -- loaden
			locked           : out std_logic;                                       -- export
			lvds_clk         : out std_logic_vector(1 downto 0);                    -- lvds_clk
			num_phase_shifts : in  std_logic_vector(2 downto 0) := (others => 'X'); -- num_phase_shifts
			outclk_2         : out std_logic;                                       -- clk
			outclk_3         : out std_logic;                                       -- clk
			outclk_4         : out std_logic;                                       -- clk
			phase_done       : out std_logic;                                       -- phase_done
			phase_en         : in  std_logic                    := 'X';             -- phase_en
			refclk           : in  std_logic                    := 'X';             -- clk
			rst              : in  std_logic                    := 'X';             -- reset
			scanclk          : in  std_logic                    := 'X';             -- clk
			updn             : in  std_logic                    := 'X'              -- updn
		);
	end component ref_pll10;

	u0 : component ref_pll10
		port map (
			cntsel           => CONNECTED_TO_cntsel,           --           cntsel.cntsel
			loaden           => CONNECTED_TO_loaden,           --           loaden.loaden
			locked           => CONNECTED_TO_locked,           --           locked.export
			lvds_clk         => CONNECTED_TO_lvds_clk,         --         lvds_clk.lvds_clk
			num_phase_shifts => CONNECTED_TO_num_phase_shifts, -- num_phase_shifts.num_phase_shifts
			outclk_2         => CONNECTED_TO_outclk_2,         --          outclk2.clk
			outclk_3         => CONNECTED_TO_outclk_3,         --          outclk3.clk
			outclk_4         => CONNECTED_TO_outclk_4,         --          outclk4.clk
			phase_done       => CONNECTED_TO_phase_done,       --       phase_done.phase_done
			phase_en         => CONNECTED_TO_phase_en,         --         phase_en.phase_en
			refclk           => CONNECTED_TO_refclk,           --           refclk.clk
			rst              => CONNECTED_TO_rst,              --            reset.reset
			scanclk          => CONNECTED_TO_scanclk,          --          scanclk.clk
			updn             => CONNECTED_TO_updn              --             updn.updn
		);

