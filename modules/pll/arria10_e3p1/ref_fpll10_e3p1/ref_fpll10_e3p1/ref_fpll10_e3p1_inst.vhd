	component ref_fpll10_e3p1 is
		port (
			pll_refclk0   : in  std_logic := 'X'; -- clk
			pll_powerdown : in  std_logic := 'X'; -- pll_powerdown
			pll_locked    : out std_logic;        -- pll_locked
			pll_cal_busy  : out std_logic;        -- pll_cal_busy
			outclk0       : out std_logic;        -- clk
			outclk1       : out std_logic;        -- clk
			outclk2       : out std_logic;        -- clk
			outclk3       : out std_logic         -- clk
		);
	end component ref_fpll10_e3p1;

	u0 : component ref_fpll10_e3p1
		port map (
			pll_refclk0   => CONNECTED_TO_pll_refclk0,   --   pll_refclk0.clk
			pll_powerdown => CONNECTED_TO_pll_powerdown, -- pll_powerdown.pll_powerdown
			pll_locked    => CONNECTED_TO_pll_locked,    --    pll_locked.pll_locked
			pll_cal_busy  => CONNECTED_TO_pll_cal_busy,  --  pll_cal_busy.pll_cal_busy
			outclk0       => CONNECTED_TO_outclk0,       --       outclk0.clk
			outclk1       => CONNECTED_TO_outclk1,       --       outclk1.clk
			outclk2       => CONNECTED_TO_outclk2,       --       outclk2.clk
			outclk3       => CONNECTED_TO_outclk3        --       outclk3.clk
		);

