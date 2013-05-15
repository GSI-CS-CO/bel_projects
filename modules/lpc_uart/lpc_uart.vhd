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
    
    kbc_out_port:   out std_logic_vector(7 downto 0);
    kbc_in_port:   in std_logic_vector(7 downto 0);

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
		      
		seven_seg_L:	out std_logic_vector(7 downto 0);   -- SSeg Data output
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
  
  component i8042_kbc is
    port (
      clk       : in std_logic;
      nrst      : in std_logic;
      cs        : in std_logic;
      rd        : in std_logic;
      wr        : in std_logic;
      data      : in std_logic;                     -- command /data register select
      stat_cmd  : in std_logic;
      int       : out std_logic;                    -- irq from kbc
      out_buffer  : out std_logic_vector(7 downto 0);  -- data out port to host
      status_buffer : out std_logic_vector(7 downto 0);
      in_buffer   : in std_logic_vector(7 downto 0);  -- data port from host
      out_port      : out std_logic_vector(7 downto 0);
      in_port      : in std_logic_vector(7 downto 0)
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
  constant kbc_status:	    unsigned(15 downto 0) := x"0064";
  constant kbc_data:	      unsigned(15 downto 0) := x"0060";
  constant base_bank_sel:   unsigned(15 downto 0) := x"004e";
  constant vendor_id:       unsigned(15 downto 0) := x"004f";
  
  constant vendor_high_byte:  std_logic_vector(7 downto 0) := x"5c";
  constant vendor_low_byte:  std_logic_vector(7 downto 0) := x"a3";
  
  constant EFER:            unsigned(15 downto 0) := x"002e";
  constant EFIR:            unsigned(15 downto 0) := EFER;
  constant EFDR:            unsigned(15 downto 0) := EFIR + 1;
  
   
	signal io_addr:			std_logic_vector(15 downto 0);
	signal io_to_slave:		std_logic_vector(7 downto 0);
	signal io_from_slave:	std_logic_vector(7 downto 0);
	signal s_paddr_valid:	std_logic;
	signal s_pdata_valid:	std_logic;
	signal clk_24:		std_logic;
	signal s_baudout:	std_logic;
	signal s_uart_addr:	std_logic_vector(2 downto 0);
	signal s_uart_cs:	std_logic;
  signal s_uart_out: std_logic_vector(7 downto 0);
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

  signal s_bsel_wr: std_logic;
  signal s_bsel_rd: std_logic;
  signal bank_sel_reg: std_logic_vector(7 downto 0) := x"80";
  signal s_vendorid_rd: std_logic;
  
  type ef_type is ( ef_idle, ef_first, ef_sec);
			
	signal ef_state:		ef_type;					-- Current state
  signal ef_active: std_logic;
  
  signal efir_reg: std_logic_vector(7 downto 0);
  signal ld_reg:  std_logic_vector(7 downto 0);
  
  signal kbc_irq:         std_logic;
  signal s_kbc_cs:        std_logic;
  signal kbc_data_out:    std_logic_vector(7 downto 0);
  signal kbc_status_out:  std_logic_vector(7 downto 0);
  signal s_kbc_status:    std_logic;
  signal s_kbc_data:      std_logic;
  

   
begin

	rst <= not lpc_reset_n;
	s_wr_en <= io_bus_we and io_data_valid;
	s_rd_en <= not io_bus_we and io_data_valid;
	
	irq_vector <= x"FFFFFF" & "111" & not uart_int & "11" & not kbc_irq & "1";	-- IRQ4 is IRQ frame 4 on CA945
	
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
		dout => s_uart_out,
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
	
  i8042: i8042_kbc port map (
    clk => lpc_clk,
    nrst => lpc_reset_n,
    cs => s_kbc_cs,
    data => s_kbc_data,
    stat_cmd => s_kbc_status,
    rd => not io_bus_we,
    wr => io_bus_we,
    int => kbc_irq,
    status_buffer => kbc_status_out,
    out_buffer => kbc_data_out,
    in_buffer => io_to_slave,
    out_port => kbc_out_port,
    in_port => kbc_in_port
    );
	
	tri_lad: process (lad_oe, s_lad_o)
	begin
		if lad_oe = '1' then
			lpc_ad <= s_lad_o;
		else
			lpc_ad <= (others => 'Z');
		end if;
	end process;
	
	s_lad_i <= lpc_ad;
	
	
	tri_serirq: process (serirq_oe, serirq_o)
	begin
		if serirq_oe = '1' then
			lpc_serirq <= serirq_o;
		else
			lpc_serirq <= 'Z';
		end if;
	end process;
	
	serirq_i <= lpc_serirq;

	
	
	addr_deco:	process (lpc_clk, io_addr)
	begin
		if rising_edge(lpc_clk) then
        s_uart_addr <= "000";
        s_uart_cs <= '0';
        s_kbc_cs <= '0';
        s_kbc_data <= '0';
        s_kbc_status <= '0';
      
				case unsigned(io_addr) is
				
				when (uart_base_addr + 0) =>
          s_uart_addr <= "000";
					s_uart_cs <= '1';				
				when (uart_base_addr + 1) =>
          s_uart_addr <= "001";
					s_uart_cs <= '1';
				when (uart_base_addr + 2) =>
          s_uart_addr <= "010";
					s_uart_cs <= '1';
				when (uart_base_addr + 3) =>
          s_uart_addr <= "011";
					s_uart_cs <= '1';
				when (uart_base_addr + 4) =>
          s_uart_addr <= "100";
          s_uart_cs <= '1';
				when (uart_base_addr + 5) =>
          s_uart_addr <= "101";
					s_uart_cs <= '1';
				when (uart_base_addr + 6) =>
          s_uart_addr <= "110";
					s_uart_cs <= '1';
				when (uart_base_addr + 7) =>
          s_uart_addr <= "111";
					s_uart_cs <= '1';
        when (kbc_data) =>
          s_kbc_cs <= '1';
          s_kbc_data <= '1';
        when (kbc_status) =>
          s_kbc_cs <= '1';
          s_kbc_status <= '1';
				when others => s_uart_addr <= "000";
						s_uart_cs <= '0';
				
				end case;
			
		end if;
	end process;
  
  extended_function: process (lpc_clk, lpc_reset_n)
  begin
    if lpc_reset_n = '0' then
      ef_state <= ef_idle;
      ef_active <= '0';
    elsif rising_edge(lpc_clk) then
      ef_active <= '0';
      
      case ef_state is
        when ef_idle =>
          if (io_addr = std_logic_vector(EFER)) and io_bus_we = '1' and io_to_slave = x"87" then
            ef_state <= ef_first;
          end if;
        when ef_first =>
          if (io_addr = std_logic_vector(EFER)) and io_bus_we = '1' and io_to_slave = x"87" then
            ef_state <= ef_sec;
          end if;
          if (io_addr = std_logic_vector(EFER)) and io_bus_we = '1' and io_to_slave = x"AA" then
            ef_state <= ef_idle;
          end if;
        when ef_sec =>
          ef_active <= '1';
          if (io_addr = std_logic_vector(EFER)) and io_bus_we = '1' and io_to_slave = x"AA" then
            ef_state <= ef_idle;
          end if;
      
      end case;
    end if;
  end process;
  
 
  extended_access: process (lpc_clk, lpc_reset_n)
  begin
    if lpc_reset_n = '0' then
      efir_reg <= x"00";
    elsif rising_edge(lpc_clk) then
      -- save config register address in EFIR
      if ef_active = '1' and (io_addr = std_logic_vector(EFIR)) and io_bus_we = '1' then
        efir_reg <= io_to_slave;
      end if;
      -- save logical device number
      if ef_active = '1' and (io_addr = std_logic_vector(EFDR)) and efir_reg = x"07" and io_bus_we = '1' then
        ld_reg <= io_to_slave;
      end if;
    end if;
  end process;
 
  
  read_mux: process(s_uart_cs, s_bsel_rd, io_bus_we, ef_active, efir_reg, ld_reg, io_addr,
                    s_vendorid_rd, s_kbc_data, s_kbc_status, kbc_status_out, kbc_data_out,
                    s_uart_out)
  begin
    if s_kbc_status = '1' then
      io_from_slave <= kbc_status_out;
    elsif s_kbc_data = '1' then
      io_from_slave <= kbc_data_out;
    elsif s_uart_cs = '1' then
      io_from_slave <= s_uart_out;
    elsif s_bsel_rd = '1' then
      io_from_slave <= bank_sel_reg;
    elsif s_vendorid_rd = '1' then
      if bank_sel_reg(7) then
        io_from_slave <= vendor_high_byte;
      else
        io_from_slave <= vendor_low_byte;
      end if;
    elsif ef_active = '1' and (io_addr = std_logic_vector(EFDR)) and io_bus_we = '0' then
      if efir_reg = x"20" then
        io_from_slave <= x"52";
      end if;
      
      if efir_reg = x"22" then
        io_from_slave <= x"ff";
      end if;
      if efir_reg = x"28" then
        io_from_slave <= x"00";
      end if;
      if efir_reg = x"2b" then
        io_from_slave <= x"00";
      end if;
      if ld_reg = x"02" then
        if efir_reg = x"30" then
          io_from_slave <= x"01"; -- UART A active
        end if;
        if efir_reg = x"60" then
          io_from_slave <= x"03"; -- UART base 0x3f8
        end if;
        if efir_reg = x"61" then
          io_from_slave <= x"f8"; -- UART base 0x3f8
        end if;
        if efir_reg = x"70" then
          io_from_slave <= x"04"; -- UART IRQ 4
        end if;
        if efir_reg = x"F0" then
          io_from_slave <= x"00"; -- clk source
        end if;
      end if;
      if ld_reg = x"03" then
        if efir_reg = x"30" then
          io_from_slave <= x"00"; -- UART B inactive
        end if;
        if efir_reg = x"60" then
          io_from_slave <= x"00"; -- UART base 0x3f8
        end if;
        if efir_reg = x"61" then
          io_from_slave <= x"00"; -- UART base 0x3f8
        end if;
        if efir_reg = x"70" then
          io_from_slave <= x"00"; -- UART IRQ 4
        end if;
        if efir_reg = x"F0" then
          io_from_slave <= x"00"; -- clk source
        end if;
      end if;
      if ld_reg = x"01" then
        if efir_reg = x"30" then
          io_from_slave <= x"00"; -- parallel port inactive
        end if;
        if efir_reg = x"60" then
          io_from_slave <= x"03"; -- base 0x378
        end if;
        if efir_reg = x"61" then
          io_from_slave <= x"78"; -- base 0x378
        end if;
        if efir_reg = x"70" then
          io_from_slave <= x"00"; -- IRQ
        end if;
        if efir_reg = x"74" then
          io_from_slave <= x"04"; -- no DMA active
        end if;
        if efir_reg = x"F0" then
          io_from_slave <= x"3f"; -- interface mode ECP and EPP 1.7 mode
        end if;
      end if;
      if ld_reg = x"05" then
        if efir_reg = x"30" then
          io_from_slave <= x"00"; -- kbc inactive
        end if;
        if efir_reg = x"60" then
          io_from_slave <= x"00"; -- base 0x0060
        end if;
        if efir_reg = x"61" then
          io_from_slave <= x"60"; -- base 0x0060
        end if;
        if efir_reg = x"62" then
          io_from_slave <= x"00"; -- 
        end if;
        if efir_reg = x"63" then
          io_from_slave <= x"64"; -- 
        end if;
        if efir_reg = x"70" then
          io_from_slave <= x"01"; -- 
        end if;
        if efir_reg = x"72" then
          io_from_slave <= x"0c"; -- 
        end if;
        if efir_reg = x"F0" then
          io_from_slave <= x"80"; -- 
        end if;
      end if;
    else
      io_from_slave <= x"00";
    end if;
    
  end process;
  
				
end architecture;