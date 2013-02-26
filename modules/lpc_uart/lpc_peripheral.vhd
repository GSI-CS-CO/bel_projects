library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;

--              I/O Write       I/O Read        DMA Read        DMA Write
--                                                         
--  States - 1. H Start         H Start         H Start         H Start
--           2. H CYCTYPE+DIR   H CYCTYPE+DIR   H CYCTYPE+DIR   H CYCTYPE+DIR
--           3. H Addr (4)      H Addr (4)      H CHAN+TC       H CHAN+TC
--                                              H SIZE          H SIZE
--           4. H Data (2)      H TAR  (2)    +-H DATA (2)      H TAR  (2)
--           5. H TAR  (2)      P SYNC (1+)   | H TAR  (2)    +-P SYNC (1+)
--           6. P SYNC (1+)     P DATA (2)    | H SYNC (1+)   +-P DATA (2)
--           7. P TAR  (2)      P TAR  (2)    +-P TAR  (2)      P TAR


entity lpc_peripheral is
	port (
		
		clk_i:		in std_logic;
		nrst_i:		in std_logic;
		lframe_i:	in std_logic;						-- LPC Frame input (active low)
		lad_oe:		out std_logic;						-- LPC AD Output Enable
		lad_i:		in std_logic_vector(3 downto 0);	-- LPC AD Input Bus
		lad_o:		out std_logic_vector(3 downto 0);	-- LPC AD Output Bus
		
		dma_chan_o:	out std_logic_vector(2 downto 0);	-- DMA Channel
		dma_tc_o:	out std_logic;						-- DMA Terminal Count
		
		-- Wishbone master interface
		wbm_err_i:	in std_logic;
		
		io_bus_dat_o:		out std_logic_vector(7 downto 0);
		io_bus_dat_i:		in std_logic_vector(7 downto 0);
		io_bus_addr:		out std_logic_vector(15 downto 0);
		io_bus_we:			out std_logic;
		io_ack:				in std_logic;
		io_data_valid:		out std_logic
		);
end entity;
		
		
architecture lpc_peri_arch of lpc_peripheral is
	
	type lpc_type is ( LPC_ST_IDLE, LPC_ST_START, LPC_ST_CYTYP, LPC_ST_ADDR, LPC_ST_CHAN,
			LPC_ST_SIZE, LPC_ST_H_DATA, LPC_ST_P_DATA, LPC_ST_H_TAR1, LPC_ST_H_TAR2, LPC_ST_P_TAR1, LPC_ST_P_TAR2,
			LPC_ST_SYNC, LPC_ST_P_WAIT1, LPC_ST_FWW_SYNC);
			
	signal state:		lpc_type;					-- Current state
	
	constant LPC_START:			std_logic_vector(3 downto 0) := "0000";
	constant LPC_STOP:			std_logic_vector(3 downto 0) := "1111";
	constant LPC_FW_READ:		std_logic_vector(3 downto 0) := "1101";
	constant LPC_FW_WRITE:		std_logic_vector(3 downto 0) := "1110";
	constant LPC_SYNC_READY:	std_logic_vector(3 downto 0) := "0000"; -- LPC Sync Ready
	constant LPC_SYNC_SWAIT:	std_logic_vector(3 downto 0) := "0101"; -- LPC Sync Short Wait (up to 8 cycles)
	constant LPC_SYNC_LWAIT:	std_logic_vector(3 downto 0) := "0110"; -- LPC Sync Long Wait (no limit)
	constant LPC_SYNC_MORE:		std_logic_vector(3 downto 0) := "1001"; -- LPC Sync Ready More (DMA only)
	constant LPC_SYNC_ERROR:	std_logic_vector(3 downto 0) := "1010"; -- LPC Sync Error
	
	constant WB_SEL_BYTE:		std_logic_vector(3 downto 0) := "0001"; -- Byte Transfer
	constant WB_SEL_SHORT:		std_logic_vector(3 downto 0) := "0011"; -- short transfer
	constant WB_SEL_WORD:		std_logic_vector(3 downto 0) := "1111"; -- word transfer
	constant WB_TGA_MEM:		std_logic_vector(1 downto 0) := "00"; 	-- memory cycle
	constant WB_TGA_IO:			std_logic_vector(1 downto 0) := "01"; 	-- I/O cycle
	constant WB_TGA_FW:			std_logic_vector(1 downto 0) := "10";	-- firmware cycle
	constant WB_TGA_DMA:		std_logic_vector(1 downto 0) := "11";  	--DMA cycle
	
	signal adr_cnt:		unsigned(2 downto 0);				-- Address nibble counter
	signal dat_cnt:		unsigned(3 downto 0);				-- Data nibble counter
	alias byte_cnt:		unsigned(2 downto 0) is dat_cnt(3 downto 1);	-- Byte counter
	alias nibble_cnt:	std_logic is dat_cnt(0);			-- Nibble counter
	
	signal lpc_dat_i:	std_logic_vector(31 downto 0);		-- Temp storage for LPC input data
	signal mem_xfr:		std_logic;							-- LPC memory transfer
	signal dma_xfr:		std_logic;							-- LPC DMA transfer
	signal fw_xfr:		std_logic;							-- LPC Firmware memory read/write
	signal xfr_len:		unsigned(2 downto 0);				-- number of nibbles for transfer
	signal dma_tc:		std_logic;							-- DMA terminal count
	signal dma_chan:	std_logic_vector(2 downto 0);		-- DMA Channel
	
	signal lpc_adr_reg:	std_logic_vector(31 downto 0);		-- Temporary storage for address received on LPC bus.
	signal lpc_dat_o:	std_logic_vector(31 downto 0);		-- Temporary storage for LPC output data.
	signal lpc_write:	std_logic;							-- 	Holds current LPC transfer direction
	signal lpc_tga_o:	std_logic_vector(1 downto 0);
	signal got_ack:		std_logic;							-- Set when ack has been received from wbm
	
	signal data_valid:	std_logic;							-- high if all nibbles have been received
	
begin

lpc_sm: process (clk_i, nrst_i)
begin
	if nrst_i = '0' then
		state <= LPC_ST_IDLE;
		lpc_adr_reg <= x"00000000";
		lpc_dat_o <= x"00000000";
		lpc_write <= '0';
		lpc_tga_o <= WB_TGA_MEM;
		lad_oe <= '0';
		lad_o <= x"F";
		mem_xfr <= '0';
		dma_xfr <= '0';
		fw_xfr <= '0';
		xfr_len <= "000";
		dma_tc <= '0';
		dma_chan <= "000";
		io_bus_we <= '0';
		--io_bus_addr <= (others => '0');
		io_bus_dat_o <= (others => '0');
		got_ack <= '1';
		
	elsif rising_edge(clk_i) then
		case state is
		
		when LPC_ST_IDLE =>	-- LPC idle state
			dat_cnt <= x"0";
			if lframe_i = '0' then
				lad_oe <= '0';
				xfr_len <= "001";
				if lad_i = LPC_START then
					state <= LPC_ST_CYTYP;
					lpc_write <= '0';
					fw_xfr <= '0';
				elsif lad_i = LPC_FW_WRITE or lad_i = LPC_FW_READ then
					state <= LPC_ST_ADDR;
					if (lad_i = LPC_FW_WRITE) then
						lpc_write <= '1';
					else
						lpc_write <= '0';
					end if;
					adr_cnt <= "000";
					fw_xfr <= '1';
					dma_xfr <= '0';
					lpc_tga_o <= WB_TGA_FW;
				else
					state <= LPC_ST_IDLE;
				end if;
			end if;
			
		when LPC_ST_CYTYP =>
			if lad_i(3) = '1' then			-- invert we_o if we are doing DMA
				lpc_write <= not lad_i(1);
			else
				lpc_write <= lad_i(1);
			end if;
			
			if lad_i(2) = '1' then
				adr_cnt <= "000";
			else
				adr_cnt <= "100";
			end if;
			
			if lad_i(3) = '1' then
				lpc_tga_o <= WB_TGA_DMA;
				dma_xfr <= '1';
				mem_xfr <= '0';
				state <= LPC_ST_CHAN;
			elsif lad_i(2) = '1' then		-- mem_xfr
				lpc_tga_o <= WB_TGA_MEM;
				dma_xfr <= '0';
				mem_xfr <= '1';
				state <= LPC_ST_ADDR;
			else
				lpc_tga_o <= WB_TGA_IO;
				dma_xfr <= '0';
				mem_xfr <= '0';
				state <= LPC_ST_ADDR;
			end if;
			
		when LPC_ST_ADDR =>
			case adr_cnt is
				when "000" => lpc_adr_reg(31 downto 28) <= lad_i;
				when "001" => lpc_adr_reg(27 downto 24) <= lad_i;
				when "010" => lpc_adr_reg(23 downto 20) <= lad_i;
				when "011" => lpc_adr_reg(19 downto 16) <= lad_i;
				when "100" => lpc_adr_reg(15 downto 12) <= lad_i;
				when "101" => lpc_adr_reg(11 downto 8) 	<= lad_i;
				when "110" => lpc_adr_reg(7 downto 4) 	<= lad_i;
				when "111" => lpc_adr_reg(3 downto 0) 	<= lad_i;
				when others => null;
			end case;
		
			adr_cnt <= adr_cnt + 1;
			
			if adr_cnt = "111" then			-- Last address nibble.
				if fw_xfr = '0' then
					if lpc_write = '1' then
						state <= LPC_ST_H_DATA;
					else
						state <= LPC_ST_H_TAR1;
					end if;
				else						-- For firmware read/write, we need to read the MSIZE nibble
					state <= LPC_ST_SIZE;
				end if;
			else
				state <= LPC_ST_ADDR;
			end if;
			
		when LPC_ST_CHAN =>
			lpc_adr_reg <= x"00000000";		-- Address lines not used for DMA.
			dma_tc <= lad_i(3);
			dma_chan <= lad_i(2 downto 0);
			state <= LPC_ST_SIZE;
			
		when LPC_ST_SIZE =>
			case lad_i is
				when x"0" => xfr_len <= "001";
				when x"1" => xfr_len <= "010";
				when x"2" => xfr_len <= "100";	-- Firmware transfer uses '2' for 4-byte transfer.
				when x"3" => xfr_len <= "100";  -- DMA uses '3' for 4-byte transfer.
				when others => xfr_len <= "001";
			end case;
			
			if lpc_write = '1' then
				state <= LPC_ST_H_DATA;
			else
				state <= LPC_ST_H_TAR1;
			end if;
		
		when LPC_ST_H_DATA =>
			
			case dat_cnt is
				when x"0" => lpc_dat_o(3 downto 0) <= lad_i;
				when x"1" => lpc_dat_o(7 downto 4) <= lad_i;
				when x"2" => lpc_dat_o(11 downto 8) <= lad_i;
				when x"3" => lpc_dat_o(15 downto 12) <= lad_i;
				when x"4" => lpc_dat_o(19 downto 16) <= lad_i;
				when x"5" => lpc_dat_o(23 downto 20) <= lad_i;
				when x"6" => lpc_dat_o(27 downto 24) <= lad_i;
				when x"7" => lpc_dat_o(31 downto 28) <= lad_i;
				when others => lpc_dat_o <= x"00000000";
			end case;
			
			dat_cnt <= dat_cnt + 1;
			
			if nibble_cnt = '1' then	-- end of byte
				if fw_xfr = '1' and byte_cnt /= xfr_len - 1  then --Firmware transfer does not have TAR between bytes.
					state <= LPC_ST_H_DATA;
				else
					state <= LPC_ST_H_TAR1;
				end if;
			else
				state <= LPC_ST_H_DATA;
			end if;
		
		when LPC_ST_H_TAR1 =>
			-- // It is ok to start the Wishbone Cycle, done below...
			
		if  ((byte_cnt = xfr_len) and lpc_write = '1') or ((byte_cnt = 0) and lpc_write = '0') then
			--io_bus_addr <= lpc_adr_reg(15 downto 0);
			io_bus_dat_o <= lpc_dat_o(7 downto 0);
			 
			io_bus_we <= lpc_write;
			got_ack <= '1';
		--elsif io_ack = '1' then
		--	got_ack <= '1';
		--	if (lpc_write = '0') then
		--		lpc_dat_i <= io_bus_dat_i & x"000000";
		--	end if;
		end if;
			
			
			
			state <= LPC_ST_H_TAR2;
			
		when LPC_ST_H_TAR2 =>
			if fw_xfr = '1' and lpc_write = '1' then
				state <= LPC_ST_FWW_SYNC;
				lad_o <= LPC_SYNC_READY;
			else
				state <= LPC_ST_SYNC;
				lad_o <= LPC_SYNC_SWAIT;
			end if;
			
			lpc_dat_i <=  x"000000" & io_bus_dat_i; -- padding
			
			lad_oe <= '1';	-- // start driving LAD
			
		when LPC_ST_SYNC =>
			lad_oe <= '1';	-- // start driving LAD
			--// First byte of WB read, last byte of WB write
			if (byte_cnt = xfr_len and lpc_write = '1') or (byte_cnt = 0 and lpc_write = '0') then
				-- // Errors can not be signalled for Firmware Memory accesses according to the spec.
				if wbm_err_i = '1' and fw_xfr = '0' then
					dat_cnt <= xfr_len & '1'; --// Abort remainder of transfer
					lad_o <= LPC_SYNC_ERROR;   --// Bus error
					state <= LPC_ST_P_TAR1;
				elsif got_ack = '1' then
					if lpc_write = '1' then
						lad_o <= LPC_SYNC_READY;	-- Ready
						state <= LPC_ST_P_TAR1;
					else
						--// READY+MORE for multi-byte DMA, except the final byte.
                        --// For non-DMA cycles, only READY
						if (xfr_len = 1 and lpc_write = '0') or dma_xfr = '0' then
							lad_o <= LPC_SYNC_READY;
						else
							lad_o <= LPC_SYNC_MORE;
						end if;
						
						state <= LPC_ST_P_DATA;
					end if;
				else
					state <= LPC_ST_SYNC;
					lad_o <= LPC_SYNC_SWAIT;
				end if;
			else -- Multi-byte transfer, just ack right away.
				if lpc_write = '1' then
					if dma_xfr = '1' then
						lad_o <= LPC_SYNC_MORE;
					else
						lad_o <= LPC_SYNC_READY;
					end if;
					state <= LPC_ST_P_TAR1;
				else
					if (byte_cnt = xfr_len - 1) or dma_xfr = '0' then
						lad_o <= LPC_SYNC_READY;
					else
						lad_o <= LPC_SYNC_MORE;
					end if;
					state <= LPC_ST_P_DATA;
				end if;
			end if;
		when LPC_ST_FWW_SYNC =>	--// Firmware write requires a special SYNC without wait-states.
			lad_o <= x"F";
			state <= LPC_ST_P_TAR2;
			
		when LPC_ST_P_DATA =>
			case dat_cnt is
				when x"0" => lad_o <= lpc_dat_i(3 downto 0);
				when x"1" => lad_o <= lpc_dat_i(7 downto 4);
				when x"2" => lad_o <= lpc_dat_i(11 downto 8);
				when x"3" => lad_o <= lpc_dat_i(15 downto 12);
				when x"4" => lad_o <= lpc_dat_i(19 downto 16);
				when x"5" => lad_o <= lpc_dat_i(23 downto 20);
				when x"6" => lad_o <= lpc_dat_i(27 downto 24);
				when x"7" => lad_o <= lpc_dat_i(31 downto 28);
				when others => lpc_dat_i <= x"00000000";
			end case;
			
			dat_cnt <= dat_cnt + 1;
			
			if nibble_cnt = '1' then -- Byte transfer complete
				if byte_cnt = xfr_len-1 then
					state <= LPC_ST_P_TAR1;
				else
					if fw_xfr = '1' then -- // Firmware transfer does not have TAR between bytes.
						state <= LPC_ST_P_DATA;
					else
						state <= LPC_ST_SYNC;
					end if;
				end if;
			else
				state <= LPC_ST_P_DATA;
				lad_oe <= '1';
			end if;
			
		when LPC_ST_P_TAR1 =>
			lad_oe <= '1';
            lad_o <= x"F";
            state <= LPC_ST_P_TAR2;
			
		when LPC_ST_P_TAR2 =>
			lad_oe <= '0';					-- float LAD
			if byte_cnt = xfr_len then
				state <= LPC_ST_IDLE;
			else
				if lpc_write = '1' then 	-- DMA READ (Host to Peripheral)
					state <= LPC_ST_P_WAIT1;
				else						-- unhandled READ case
					state <= LPC_ST_IDLE;
				end if;
			end if;
			
		when LPC_ST_P_WAIT1 =>
			state <= LPC_ST_H_DATA;
			
		when others =>
		
		end case; -- end of state case

	end if;
end process lpc_sm;


--io_bus: process (clk_i, nrst_i)
--begin
--	if rising_edge(clk_i) then
--		if nrst_i = '0' then
--			io_bus_we <= '0';
--			io_bus_addr <= (others => '0');
--			io_bus_dat_o <= (others => '0');
--		elsif ((state = LPC_ST_H_TAR1) and (((byte_cnt = xfr_len) and lpc_write = '1') or ((byte_cnt = 0) and lpc_write = '0'))) then
--			io_bus_addr <= lpc_adr_reg(15 downto 0);
--			io_bus_dat_o <= lpc_dat_o(7 downto 0);
--			io_bus_we <= lpc_write;
--		else
--			lpc_dat_i <= io_bus_dat_i & x"000000";
--		end if;
--	end if;
--end process;


--data_valid <= 	'1' when dat_cnt = 2 and state = LPC_ST_H_TAR1
data_valid <= 	'1' when state = LPC_ST_H_TAR2
				else '0';
dma_chan_o <= dma_chan;
dma_tc_o <= dma_tc;
io_data_valid <= data_valid;

io_bus_addr <= lpc_adr_reg(15 downto 0);



end architecture;