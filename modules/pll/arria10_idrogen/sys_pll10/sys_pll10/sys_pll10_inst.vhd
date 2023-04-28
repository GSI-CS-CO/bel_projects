	component sys_pll10 is
		port (
			rst      : in  std_logic := 'X'; -- reset
			refclk   : in  std_logic := 'X'; -- clk
			locked   : out std_logic;        -- export
			outclk_0 : out std_logic;        -- clk
			outclk_1 : out std_logic;        -- clk
			outclk_2 : out std_logic;        -- clk
			outclk_3 : out std_logic;        -- clk
			outclk_4 : out std_logic         -- clk
		);
	end component sys_pll10;

	u0 : component sys_pll10
		port map (
			rst      => CONNECTED_TO_rst,      --   reset.reset
			refclk   => CONNECTED_TO_refclk,   --  refclk.clk
			locked   => CONNECTED_TO_locked,   --  locked.export
			outclk_0 => CONNECTED_TO_outclk_0, -- outclk0.clk
			outclk_1 => CONNECTED_TO_outclk_1, -- outclk1.clk
			outclk_2 => CONNECTED_TO_outclk_2, -- outclk2.clk
			outclk_3 => CONNECTED_TO_outclk_3, -- outclk3.clk
			outclk_4 => CONNECTED_TO_outclk_4  -- outclk4.clk
		);

