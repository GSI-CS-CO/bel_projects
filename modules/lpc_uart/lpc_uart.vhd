library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;

entity lpc_uart is
	port (
		lpc_clk:			in std_logic;
		lpc_serirq:		inout std_logic;
		lpc_ad:			inout std_logic_vector(3 downto 0);
		lpc_frame_n:	in std_logic;
		lpc_reset_n:	in std_logic;

		serial_rxd:		in std_logic;
		serial_txd:		out std_logic;
		serial_dtr:		out std_logic;
		serial_dcd:		in std_logic;
		serial_dsr:		in std_logic;
		serial_ri:		in std_logic;
		serial_cts:		in std_logic;
		serial_rts:		out std_logic;
		
		      
		seven_seg_L:	out std_logic_vector(7 downto 0);    -- SSeg Data output
		seven_seg_H:	out std_logic_vector(7 downto 0)    -- SSeg Data output  
	
	);
end lpc_uart;



architecture lpc_uart_arch of lpc_uart is


	component lpc_decoder is
		Port (
		      lclk:         in std_logic;                      -- LPC: 33MHz clock (rising edge)
		      lframe_n:   in std_logic;                        -- LPC: frame, active low
		      lreset_n:   in std_logic;                        -- LPC: reset, active low
		      lad:         in std_logic_vector(3 downto 0);    -- LPC: multiplexed bus
		      
		      paddr:		out std_logic_vector(15 downto 0);	-- port addr
		      pdata_in: 	out std_logic_vector(7 downto 0);	-- data to the slave
		      pdata_out:	in std_logic_vector(7 downto 0);	-- data from the slave
		      paddr_valid:	out std_logic;
		      pdata_valid:	out std_logic
		);
	end component;

	component lpc_peripheral is
	port (
		
		clk_i:		in std_logic;
		nrst_i:		in std_logic;
		lframe_i:	in std_logic;						-- LPC Frame input (active high)
		lad_oe:		out std_logic;						-- LPC AD Output Enable
		lad_i:		in std_logic_vector(3 downto 0);	-- LPC AD Input Bus
		lad_o:		out std_logic_vector(3 downto 0);	-- LPC AD Output Bus
		
		dma_chan_o:	out std_logic_vector(2 downto 0);	-- DMA Channel
		dma_tc_o:	out std_logic;						-- DMA Terminal Count
		

		wbm_err_i:	in std_logic;
		
		io_bus_dat_o:		out std_logic_vector(7 downto 0);
		io_bus_dat_i:		in std_logic_vector(7 downto 0);
		io_bus_addr:	out std_logic_vector(15 downto 0);
		io_bus_we:		out std_logic;
		io_ack:			in std_logic;
		io_data_valid:	out std_logic
		);
	end component;
   
	component postcode is
		port (
		lclk:		in std_logic;
		
		paddr:		in std_logic_vector(15 downto 0);
		pdata:		in std_logic_vector(7 downto 0);
		addr_hit:	out std_logic;
		data_valid:	in std_logic;
		      
		seven_seg_L:	out std_logic_vector(7 downto 0);    -- SSeg Data output
		seven_seg_H:	out std_logic_vector(7 downto 0)    -- SSeg Data output  
	
	);
	end component;
	
	component uart_16750 is
	    port (
		CLK         : in std_logic;                             -- Clock 24Mhz
		RST         : in std_logic;                             -- Reset
		BAUDCE      : in std_logic;                             -- Baudrate generator clock enable
		CS          : in std_logic;                             -- Chip select
		WR          : in std_logic;                             -- Write to UART
		RD          : in std_logic;                             -- Read from UART
		A           : in std_logic_vector(2 downto 0);          -- Register select
		DIN         : in std_logic_vector(7 downto 0);          -- Data bus input
		DOUT        : out std_logic_vector(7 downto 0);         -- Data bus output
		DDIS        : out std_logic;                            -- Driver disable
		INT         : out std_logic;                            -- Interrupt output
		OUT1N       : out std_logic;                            -- Output 1
		OUT2N       : out std_logic;                            -- Output 2
		RCLK        : in std_logic;                             -- Receiver clock (16x baudrate)
		BAUDOUTN    : out std_logic;                            -- Baudrate generator output (16x baudrate)
		RTSN        : out std_logic;                            -- RTS output
		DTRN        : out std_logic;                            -- DTR output
		CTSN        : in std_logic;                             -- CTS input
		DSRN        : in std_logic;                             -- DSR input
		DCDN        : in std_logic;                             -- DCD input
		RIN         : in std_logic;                             -- RI input
		SIN         : in std_logic;                             -- Receiver input
		SOUT        : out std_logic                             -- Transmitter output
	    );
	end component;
	
	component uart_pll IS
	PORT
	(
		inclk0	: IN STD_LOGIC  := '0';
		c0		: OUT STD_LOGIC;
		c1		: out std_logic
	);
	END component;
	
	component serirq_slave is
	port (
			clk_i : in std_logic;
			nrst_i : in std_logic;
         irq_i : in std_logic_vector(31 downto 0);
         serirq_o : out std_logic;
			serirq_i : in std_logic;
			serirq_oe : out std_logic
	);
	end component;
	
	constant uart_base_addr:	unsigned(15 downto 0) := x"03F8";
   
	signal io_addr:			std_logic_vector(15 downto 0);
	signal io_to_slave:		std_logic_vector(7 downto 0);
	signal io_from_slave:	std_logic_vector(7 downto 0);
	signal s_paddr_valid:	std_logic;
	signal s_pdata_valid:	std_logic;
	signal clk_24:		std_logic;
	signal s_baudout:	std_logic;
	signal s_uart_addr:	std_logic_vector(2 downto 0);
	signal s_uart_cs:	std_logic;
	signal s_addr_hit:	std_logic;
	signal s_rd_en:		std_logic;
	signal s_wr_en:		std_logic;
	signal io_bus_we:	std_logic;
	signal s_lad_i:		std_logic_vector(3 downto 0);
	signal s_lad_o:		std_logic_vector(3 downto 0);
	signal lad_oe:		std_logic;
	signal io_data_valid:	std_logic;
	signal rst:	std_logic;
	
	signal serirq_i: std_logic;
	signal serirq_o: std_logic;
	signal serirq_oe: std_logic;
	signal irq_vector: std_logic_vector(31 downto 0);
	signal uart_int: std_logic;
   
begin

	rst <= not lpc_reset_n;
	s_wr_en <= io_bus_we and io_data_valid;
	s_rd_en <= not io_bus_we and io_data_valid;
	
	
	irq_vector <= x"FFFFFF" & "111" & not uart_int & "1111";	-- IRQ4 is IRQ frame 4 on CA945
	
	decoder: lpc_peripheral port map (
		clk_i => lpc_clk,
		lframe_i => lpc_frame_n,
		nrst_i => lpc_reset_n,
		lad_oe => lad_oe,
		lad_i => s_lad_i,
		lad_o => s_lad_o,
		dma_chan_o => open,
		dma_tc_o => open,
		wbm_err_i => '0',
		
		
		
		
		io_bus_dat_o => io_to_slave,
		io_bus_dat_i => io_from_slave,
		io_bus_addr	=> io_addr,
		io_bus_we => io_bus_we,
		io_ack => '1',
		io_data_valid => io_data_valid
		);
		
	pcode: postcode port map (
	
		lclk => lpc_clk,
		data_valid => io_data_valid,
		paddr => io_addr,
		pdata => io_to_slave,
		addr_hit => s_addr_hit,
		      
		seven_seg_L => seven_seg_L,
		seven_seg_H => seven_seg_H
	);
	
	uart: uart_16750 port map (
		clk => lpc_clk,
		rst => rst,
		baudce => '1',
		cs => s_uart_cs,
		wr => s_wr_en,
		rd => s_rd_en,
		a => s_uart_addr,
		din => io_to_slave,
		dout => io_from_slave,
		ddis => open,
		int => uart_int,
		out1n => open,
		out2n => open,
		rclk => s_baudout,
		baudoutn => s_baudout,
		rtsn => serial_rts,
		dtrn => serial_dtr,
		ctsn => serial_cts,
		dsrn => serial_dsr,
		dcdn => serial_dcd,
		rin => serial_ri,
		sin => serial_rxd,
		sout => serial_txd
	);
	
	serirq: serirq_slave port map (
			clk_i => lpc_clk,
			nrst_i => lpc_reset_n,
			irq_i => irq_vector,
         serirq_o => serirq_o,
			serirq_i =>  serirq_i,
			serirq_oe => serirq_oe
		);
	
	
	tri_lad: process (lad_oe)
	begin
		if lad_oe = '1' then
			lpc_ad <= s_lad_o;
		else
			lpc_ad <= (others => 'Z');
		end if;
	end process;
	
	s_lad_i <= lpc_ad;
	
	
	tri_serirq: process (serirq_oe)
	begin
		if serirq_oe = '1' then
			lpc_serirq <= serirq_o;
		else
			lpc_serirq <= 'Z';
		end if;
	end process;
	
	serirq_i <= lpc_serirq;
	
	
	
	
	uart_addr_deco:	process (lpc_clk, io_addr)
	begin
		if rising_edge(lpc_clk) then
			
				case unsigned(io_addr) is
				
				when (uart_base_addr + 0) => 	s_uart_addr <= "000";
								s_uart_cs <= '1';
									
				when (uart_base_addr + 1) => 	s_uart_addr <= "001";
								s_uart_cs <= '1';
				when (uart_base_addr + 2) => 	s_uart_addr <= "010";
								s_uart_cs <= '1';
				when (uart_base_addr + 3) => 	s_uart_addr <= "011";
								s_uart_cs <= '1';
				when (uart_base_addr + 4) => 	s_uart_addr <= "100";
								s_uart_cs <= '1';
				when (uart_base_addr + 5) => 	s_uart_addr <= "101";
								s_uart_cs <= '1';
				when (uart_base_addr + 6) => 	s_uart_addr <= "110";
								s_uart_cs <= '1';
				when (uart_base_addr + 7) => 	s_uart_addr <= "111";
								s_uart_cs <= '1';
				when others => s_uart_addr <= "000";
						s_uart_cs <= '0';
				
				end case;
			
		end if;
	end process;
				
end architecture;