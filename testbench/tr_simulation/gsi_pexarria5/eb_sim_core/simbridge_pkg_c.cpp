#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>

#include <errno.h>

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <deque>
#include <cstdint>

// std_logic values
typedef enum { 
	STD_LOGIC_U,
	STD_LOGIC_X,
	STD_LOGIC_0,
	STD_LOGIC_1,
	STD_LOGIC_Z,
	STD_LOGIC_W,
	STD_LOGIC_L,
	STD_LOGIC_H,
	STD_LOGIC_DASH
} std_logic_t;


class EBslave 
{
public:
	void init() {
		wb_stbs.clear();
		wb_wait_for_acks.clear();
		input_word_buffer.clear();
		input_word_buffer2.clear();
		output_word_buffer.clear();

		pfds[0].fd = open("/dev/ptmx", O_RDWR );//| O_NONBLOCK);

		// put it in raw mode
		struct termios raw;
		if (tcgetattr(pfds[0].fd, &raw) == 0)
		{
			// input modes - clear indicated ones giving: no break, no CR to NL, 
			//   no parity check, no strip char, no start/stop output (sic) control 
			raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

			// output modes - clear giving: no post processing such as NL to CR+NL 
			raw.c_oflag &= ~(OPOST);

			// control modes - set 8 bit chars 
			raw.c_cflag |= (CS8);

			// local modes - clear giving: echoing off, canonical off (no erase with 
			//   backspace, ^U,...),  no extended functions, no signal chars (^Z,^C) 
			raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

			// control chars - set return condition: min number of bytes and timer 
			raw.c_cc[VMIN] = 5; raw.c_cc[VTIME] = 8; // after 5 bytes or .8 seconds
			//                                          //   after first byte seen   
			raw.c_cc[VMIN] = 0; raw.c_cc[VTIME] = 0; // immediate - anything      
			raw.c_cc[VMIN] = 2; raw.c_cc[VTIME] = 0; // after two bytes, no timer 
			raw.c_cc[VMIN] = 0; raw.c_cc[VTIME] = 8; // after a byte or .8 seconds

			// put terminal in raw mode after flushing 
			if (tcsetattr(pfds[0].fd,TCSAFLUSH,&raw) < 0) 
			{
				int err = errno;
				printf("Error, cant set raw mode: %s\n", strerror(err));
				return;
			}
		}
		word_count = 0;
		grantpt(pfds[0].fd);
		unlockpt(pfds[0].fd);
		state = EB_SLAVE_STATE_IDLE;
		std::ofstream tmpfile("/tmp/simbridge-eb-device");
		tmpfile << pts_name().substr(1) << std::endl;
		std::cerr << "eb-device: " << pts_name() << std::endl;
		if (_stop_until_connected) {
			std::cerr << "waiting for client, simulation stopped ... ";
		} else {
			std::cerr << "device is ready, simulation is running" << std::endl;
		}
		if (_stop_until_connected)
		{
			pfds[0].events = POLLIN;
			poll(pfds,1,-1);
			std::cerr << " connected, simulation continues" << std::endl;
			_stop_until_connected = false;
		}
		error_shift_reg = 0;

	}


	EBslave(bool stop_until_connected, bool polled, uint32_t sdb_adr, uint32_t msi_addr_first, uint32_t msi_addr_last) 
	{
		std::cerr << "EBslave: sdb_adr=0x" << std::hex << std::setw(8) << std::setfill('0') << sdb_adr 
                  << "         msi_addr_first=0x" << std::hex << std::setw(8) << std::setfill('0') << msi_addr_first
                  << "         msi_addr_last=0x" << std::hex << std::setw(8) << std::setfill('0') << msi_addr_last
                  << std::endl;
		_stop_until_connected = stop_until_connected;
		_polled = polled;
		eb_sdb_adr       = sdb_adr;
		eb_msi_adr_first = msi_addr_first;
		eb_msi_adr_last  = msi_addr_last;
		init();
	}

	std::string pts_name() {
		return std::string(ptsname(pfds[0].fd));
	}


	void fill_input_buffer() {
		uint8_t buffer[1024];
		uint8_t *value;
		pfds[0].events = POLLIN;
		int timeout_ms = 0;
		int result = poll(pfds,1,timeout_ms);
		if (result != 1) {
			return;
		}
		if (pfds[0].revents == POLLHUP) {
			close(pfds[0].fd);
			pfds[0].fd = 0;
			// exit(1);
			init();
			return;
		}
		result = read(pfds[0].fd, (void*)buffer, sizeof(buffer));
		if (result == -1 && errno == EAGAIN) {
			return;
		} else if (result == -1) {
			std::cerr << "unexpected error " << errno << " " << strerror(errno) << std::endl;
			switch(errno) {
				case EBADF:  std::cerr << "EBADF" << std::endl; break;
				case EFAULT: std::cerr << "EFAULT" << std::endl; break;
				case EINTR:  std::cerr << "EINTR" << std::endl; break;
				case EINVAL: std::cerr << "EINVAL" << std::endl; break;
				case EIO:    std::cerr << "EIO" << std::endl; break;
				case EISDIR: std::cerr << "EISDIR" << std::endl; break;
			}
			close(pfds[0].fd);
			pfds[0].fd = 0;
			init();

		} else if (result >= 4) {
			value = buffer;
			while (result > 0) {
				uint32_t value32  = value[0]; value32 <<=8;
				         value32 |= value[1]; value32 <<=8;
				         value32 |= value[2]; value32 <<=8;
				         value32 |= value[3]; 
				// std::cerr << "<= 0x" << std::hex << std::setw(8) << std::setfill('0') << (uint32_t)value32 << std::endl;
				input_word_buffer.push_back(value32);
				input_word_buffer2.push_back(value32);
				result -= 4;
				value  += 4; 
				++word_count;
			}
		}
	}
	// return true if a word is available
	// return false if there is nothing
	bool next_word(uint32_t &result) {
		// try to read from input stream
		for (;;) {
			if (input_word_buffer.size() > 0) {
				result = input_word_buffer.front();
				input_word_buffer.pop_front();
				return true;
			} else {
				fill_input_buffer();
				if (input_word_buffer.size() > 0) continue;
				return false;
			}
		}
	}


	int master_out(std_logic_t *cyc, std_logic_t *stb, std_logic_t *we, int *adr, int *dat, int *sel) {
		// std::cerr << "out " << state << std::endl;
		int end_cyc = 0;

		// std::cerr << "control_out" << std::endl;
		*cyc = STD_LOGIC_0;
		*stb = STD_LOGIC_0;
		*we  = STD_LOGIC_0;
		*adr = 0xaffe1234;
		*dat = 0xbabe1234;
		*sel = 0xf;

		uint32_t word;
		switch(state) {
			case EB_SLAVE_STATE_IDLE:
				if (next_word(word)) {
					if (word == 0x4e6f11ff) {
						wb_stbs.push_back(wb_stb(0,0x4e6f1644,false,true)); // not a real strobe, just a pass-through
						if (next_word(word)) {
							wb_stbs.push_back(wb_stb(0,word,false,true)); // not a real strobe, just a pass-through
							state = EB_SLAVE_STATE_EB_HEADER;
						}
					}
				}
			break;
			case EB_SLAVE_STATE_EB_HEADER:
				if (next_word(word)) { // eb record header
					eb_flag_bca =  word & 0x80000000;
					eb_flag_rca =  word & 0x40000000;
					eb_flag_rff =  word & 0x20000000;
					eb_flag_cyc =  word & 0x08000000;
					eb_flag_wca =  word & 0x04000000;
					eb_flag_wff =  word & 0x02000000;
					eb_byte_en  = (word & 0x00ff0000) >> 16;
					eb_wcount   = (word & 0x0000ff00) >>  8;
					eb_rcount   = (word & 0x000000ff) >>  0;
					uint32_t response  = (word & 0x00ff0000); // echo byte_enable
					         //response |= (word & 0x0000ff00) >> 8; // wcount becomes rcount (no! wcount becomes zero)
					         response |= (word & 0x000000ff) << 8; // rcount becomes wcount
					         response |= (eb_flag_cyc << 27); // response rca <= request bca
					         response |= (eb_flag_bca << 26); // response rff <= request rff
					         response |= (eb_flag_rff << 25); // response wca <= request wca;


					// if we have a write request, the response must be zero and a new header has to be inserted 
					// in front of the read response (if there was any read request)
					if (eb_wcount > 0) {
						new_header = response & 0xffffff00; // delete the read count;
						response = 0;
					}
					wb_stbs.push_back(wb_stb(response,response,false,true)); // not a real strobe, just a pass-through
					wb_stbs.back().comment = "header";
					wb_stbs.back().end_cyc = eb_flag_cyc;

					// std::cerr << "header " << std::hex << std::setw(8) << std::setfill('0') << word 
					//           << "   response " << std::setw(8) << std::setfill('0') << response << std::endl;
					if (eb_wcount > 0) {
					 	if (eb_flag_wca) {
							state = EB_SLAVE_STATE_EB_CONFIG_FIRST;						
						} else {
							state = EB_SLAVE_STATE_EB_WISHBONE_FIRST;
						}
					} else {
						if (eb_flag_rca) { // access to config space
							state = EB_SLAVE_STATE_EB_CONFIG_FIRST;
						} else {
							state = EB_SLAVE_STATE_EB_WISHBONE_FIRST;
						}
					}
				}
			break;
			case EB_SLAVE_STATE_EB_CONFIG_FIRST:
				if (eb_wcount > 0) {
					uint32_t base_write_adr;
					if (next_word(base_write_adr)) {
						wb_stbs.push_back(wb_stb(0x0,0x0,false,true)); // not a real strobe, just a pass-through
						wb_stbs.back().end_cyc = eb_flag_cyc;
						state = EB_SLAVE_STATE_EB_CONFIG_REST;
					}
				} else if (eb_rcount > 0) {
					uint32_t base_ret_adr;
					if (next_word(base_ret_adr)) {
						wb_stbs.push_back(wb_stb(base_ret_adr,base_ret_adr,false,true)); // not a real strobe, just a pass-through
						wb_stbs.back().end_cyc = eb_flag_cyc;
						state = EB_SLAVE_STATE_EB_CONFIG_REST;
					}
				} else {
					state = EB_SLAVE_STATE_EB_HEADER;
				}
			break;
			case EB_SLAVE_STATE_EB_CONFIG_REST:
				if (eb_wcount > 0) {
					uint32_t write_val;
					if (next_word(write_val)) {
						--eb_wcount;
						if (eb_wcount == 0) {
							wb_stbs.push_back(wb_stb(new_header,new_header,false,true)); // not a real strobe, just a pass-through
							wb_stbs.back().end_cyc = eb_flag_cyc;
						} else {
							wb_stbs.push_back(wb_stb(0x0,0x0,false,true)); // not a real strobe, just a pass-through
							wb_stbs.back().end_cyc = eb_flag_cyc;
						}
						if (eb_wcount == 0) {
							if (eb_rcount == 0) {
								state = EB_SLAVE_STATE_EB_HEADER;
							} else {
								state = EB_SLAVE_STATE_EB_CONFIG_FIRST; // handle the rcount values
							}
						}
					}
				} else if (eb_rcount > 0) {
					uint32_t read_adr;
					if (next_word(read_adr)) {
						--eb_rcount;
						uint32_t err = 0x0;
						switch(read_adr) {
							case 0x0: 
								wb_stbs.push_back(wb_stb(error_shift_reg,error_shift_reg,false,true)); // not a real strobe, just a pass-through
								wb_stbs.back().end_cyc = eb_flag_cyc;
								error_shift_reg = 0; // clear the error shift register 
							break;
							case 0xc: 
							    // this should return the sdb address
								wb_stbs.push_back(wb_stb(eb_sdb_adr,eb_sdb_adr,false,true)); // not a real strobe, just a pass-through
								wb_stbs.back().end_cyc = eb_flag_cyc;
							break;
							case 0x2c: 
								wb_stbs.push_back(wb_stb(0x1,0x1,false,true)); // not a real strobe, just a pass-through
								wb_stbs.back().end_cyc = eb_flag_cyc;
							break;
							case 0x34:
								wb_stbs.push_back(wb_stb(eb_msi_adr_first,eb_msi_adr_first,false,true)); // not a real strobe, just a pass-through
								wb_stbs.back().end_cyc = eb_flag_cyc;
							break;
							case 0x3c:
								wb_stbs.push_back(wb_stb(eb_msi_adr_last,eb_msi_adr_last,false,true)); // not a real strobe, just a pass-through
								wb_stbs.back().end_cyc = eb_flag_cyc;
							break;
							case 0x40: // msi_adr
								if (msi_queue.size() > 0 && _polled) {
									msi_adr = msi_queue.front().adr;
									msi_dat = msi_queue.front().dat;
									msi_cnt = 1;
									if (msi_queue.size() > 1) {
										msi_cnt = 3;
									}
									msi_queue.pop_front();
								} else {
									msi_cnt = 0;
								}
								wb_stbs.push_back(wb_stb(msi_adr,msi_adr,false,true)); // not a real strobe, just a pass-through
								wb_stbs.back().end_cyc = eb_flag_cyc;
							break;
							case 0x44: // msi_dat
								wb_stbs.push_back(wb_stb(msi_dat,msi_dat,false,true)); // not a real strobe, just a pass-through
								wb_stbs.back().end_cyc = eb_flag_cyc;
							break;
							case 0x48:
								wb_stbs.push_back(wb_stb(msi_cnt,msi_cnt,false,true)); // not a real strobe, just a pass-through
								wb_stbs.back().end_cyc = eb_flag_cyc;
							break;

    // x"00000000"                                      when "01000", -- 0x20 = 0[010 00]00
    // x"00000000"                                      when "01001", -- 0x24 = 0[010 01]00
    // x"00000000"                                      when "01010", -- 0x28
    // x"00000001"                                      when "01011", -- 0x2c
    // x"00000000"                                      when "01100", -- 0x30
    // c_ebs_msi.sdb_component.addr_first(31 downto  0) when "01101", -- 0x34
    // x"00000000"                                      when "01110", -- 0x38
    // c_ebs_msi.sdb_component.addr_last(31 downto  0)  when "01111", -- 0x3c
    // msi_adr                                          when "10000", -- 0x40 = 0[100 00]00
    // msi_dat                                          when "10001", -- 0x44 = 0[100 01]00
    // msi_cnt                                          when "10010", -- 0x48 = 0[100 10]00
    // x"00000000"                                      when others;
							default: 
								wb_stbs.push_back(wb_stb(0x0,0x0,false,true)); // not a real strobe, just a pass-through
								wb_stbs.back().end_cyc = eb_flag_cyc;
								wb_stbs.back().err = true;
						}
						if (eb_rcount == 0) {
							state = EB_SLAVE_STATE_EB_HEADER;
						}
					}

				}
			break;
			case EB_SLAVE_STATE_EB_WISHBONE_FIRST:
				if (eb_wcount > 0) {
					if (next_word(base_write_adr)) {
						// output_word_buffer.push_back(base_write_adr);
						wb_stbs.push_back(wb_stb(0x0,0x0,false,true)); // not a real strobe, just a pass-through
						wb_stbs.back().end_cyc = eb_flag_cyc;
						// wb_stbs.back().zero = true;
						state = EB_SLAVE_STATE_EB_WISHBONE_REST;
					}
				} else if (eb_rcount > 0) {
					if (next_word(base_ret_adr)) {
						// output_word_buffer.push_back(base_ret_adr);
						wb_stbs.push_back(wb_stb(base_ret_adr,base_ret_adr,false,true)); // not a real strobe, just a pass-through
						wb_stbs.back().end_cyc = eb_flag_cyc;
						state = EB_SLAVE_STATE_EB_WISHBONE_REST;
					}
				} else {
					state = EB_SLAVE_STATE_EB_HEADER;
				}
			break;
			case EB_SLAVE_STATE_EB_WISHBONE_REST:
				if (eb_wcount > 0) {
					uint32_t write_val;
					if (next_word(write_val)) {
						--eb_wcount;
						// put the write strobe into the queue
						wb_stbs.push_back(wb_stb(base_write_adr,write_val,true));
						wb_stbs.back().end_cyc = eb_flag_cyc;
						if (eb_wcount == 0) {
							wb_stbs.back().new_header = true;
							wb_stbs.back().new_header_value = new_header;
						} else {
							wb_stbs.back().zero = true;
						}
						// increment base_write_adr unless we are writing into a fifo
						if (!eb_flag_wff) base_write_adr += 4; 
						if (eb_wcount == 0) {
							if (eb_rcount == 0) {
								state = EB_SLAVE_STATE_EB_HEADER;
							} else {
								if (eb_flag_rca) { // access to config space
									state = EB_SLAVE_STATE_EB_CONFIG_FIRST;
								} else {
									state = EB_SLAVE_STATE_EB_WISHBONE_FIRST;
								}	
							}
						}
					}
				} else if (eb_rcount > 0) {
					uint32_t read_adr;
					if (next_word(read_adr)) {
						// std::cerr << "read_adr " << std::hex << std::setw(8) << std::setfill('0') << read_adr 
						//           << "    rcnt " << std::dec << (int)eb_rcount << std::endl;
						--eb_rcount;
						wb_stbs.push_back(wb_stb(read_adr,0,false));
						wb_stbs.back().end_cyc = eb_flag_cyc;
						if (eb_rcount == 0) {
							state = EB_SLAVE_STATE_EB_HEADER;
						}
					}

				}
			break;
		}


		if (handle_pass_through()) end_cyc = 1;
		send_output_buffer();

		if (eb_flag_cyc) {
			*cyc = STD_LOGIC_0;
		}
		if (wb_stbs.size() > 0 || wb_wait_for_acks.size() > 0) {
			*cyc = STD_LOGIC_1;
		}

		strobe = false;
		bool write_enable = false;
		*we = STD_LOGIC_0;
		if (wb_stbs.size() > 0) {
			strobe = true;
			if (wb_stbs.front().end_cyc) {
				end_cyc = 1;
			}
			if (wb_stbs.front().we) {
				*we = STD_LOGIC_1;
				write_enable = true;
			} else {
				*we = STD_LOGIC_0;
			}
			*adr = wb_stbs.front().adr;
			*dat = wb_stbs.front().dat;
			*sel = 0xf;
		}
		*stb = strobe ? STD_LOGIC_1 : STD_LOGIC_0;
		*we  = write_enable ? STD_LOGIC_1 : STD_LOGIC_0;
		return end_cyc;
	}

	int handle_pass_through() {
		int end_cyc = 0;
		// std::cerr << "handle_pass_through " << wb_stbs.size() << std::endl;
		while(wb_stbs.size() > 0 && wb_stbs.front().passthrough) {
			wb_wait_for_acks.push_back(wb_stbs.front());
			if (wb_stbs.front().end_cyc) end_cyc = 1;
			wb_stbs.pop_front();
		}
		// std::cerr << "handle_pass_through " << wb_wait_for_acks.size() << std::endl;
		// remove all pass-through values
		while (wb_wait_for_acks.size() > 0 && wb_wait_for_acks.front().passthrough) {
			// std::cerr << "pass-through" << std::endl;
			output_word_buffer.push_back(wb_wait_for_acks.front().dat);
			int err = wb_wait_for_acks.front().err;
			error_shift_reg = (error_shift_reg << 1) | err;
			wb_wait_for_acks.pop_front();
		}
		// std::cerr << "handle_pass_through " << output_word_buffer.size() << std::endl;
		return end_cyc;
	}

	void send_output_buffer()
	{
		bool wrote_something = false;
		// std::cerr << "send_output_buffer " << wb_wait_for_acks.size() << " " << output_word_buffer.size() << " " << word_count << std::endl;
		if (wb_wait_for_acks.size() == 0 && output_word_buffer.size() >= word_count) {
			std::vector<uint8_t> write_buffer;
			while (output_word_buffer.size() > 0) {
				--word_count;
				uint32_t word_out = output_word_buffer.front();
				uint32_t word_in  = input_word_buffer2.front();
				output_word_buffer.pop_front();
				input_word_buffer2.pop_front();
				std::cerr << std::hex << std::setw(8) << std::setfill('0') << (uint32_t)word_in
				          << " => 0x" << std::hex << std::setw(8) << std::setfill('0') << (uint32_t)word_out 
				          << std::endl;
				wrote_something = true;
				for (int i = 0; i < 4; ++i) {
					uint8_t val = word_out >> (8*(3-i));
					//std::cerr << "  >" << std::hex << std::setw(2) << std::setfill('0') << (uint32_t)val << std::endl;
					//write(pfds[0].fd, (void*)&val, sizeof(val));
					write_buffer.push_back(val);
				}
			}
			write(pfds[0].fd, (void*)&write_buffer[0], write_buffer.size());
			if (wrote_something) {
				std::cerr << "----------------------" << std::endl;
			}
		}
		if (word_count == 0 && !_polled) {
			// std::cerr << "all bytes sent" << std::endl;
			for (unsigned i = 0; i < msi_queue.size(); ++i) {
				std::vector<uint8_t> msi_buffer;
				uint32_t adr = msi_queue[i].adr - eb_msi_adr_first;
				uint32_t dat = msi_queue[i].dat;
				std::cerr << "send msi ";
				std::cerr << "adr=0x" << std::hex << std::setw(8) << std::setfill('0') << adr << " ";
				std::cerr << "dat=0x" << std::hex << std::setw(8) << std::setfill('0') << dat << " ";
				std::cerr << std::dec << std::endl;
				
				msi_buffer.push_back(0xa8);
				msi_buffer.push_back(0x0f);
				msi_buffer.push_back(0x01);
				msi_buffer.push_back(0x00);

				msi_buffer.push_back(adr>>24);
				msi_buffer.push_back(adr>>16);
				msi_buffer.push_back(adr>>8);
				msi_buffer.push_back(adr>>0);

				msi_buffer.push_back(dat>>24);
				msi_buffer.push_back(dat>>16);
				msi_buffer.push_back(dat>>8);
				msi_buffer.push_back(dat>>0);		

				write(pfds[0].fd, (void*)&msi_buffer[0], msi_buffer.size());
			}
			msi_queue.clear();
		}		
	}

	// should be called on falling_edge(clk)
	int master_in(std_logic_t ack, std_logic_t err, std_logic_t rty, std_logic_t stall, int dat) {
		// std::cerr << "in" << std::endl;
		// std::cerr << "control_in wb_stbs.size() = " << std::dec << (int)wb_stbs.size() << std::endl;
		int end_cyc = 0;
		if (handle_pass_through()) end_cyc = 1;
		if (wb_stbs.size() > 0 && (strobe && stall == STD_LOGIC_0)) {
			wb_wait_for_acks.push_back(wb_stbs.front());
			if (wb_stbs.front().end_cyc) end_cyc = 1;
			wb_stbs.pop_front();
		}
		if (wb_wait_for_acks.size() > 0 && (ack == STD_LOGIC_1 || err == STD_LOGIC_1)) {
			if (wb_wait_for_acks.front().we) {
				if (wb_wait_for_acks.front().zero) {
					output_word_buffer.push_back(0x0);
				} else if (wb_wait_for_acks.front().new_header) {
					output_word_buffer.push_back(wb_wait_for_acks.front().new_header_value);
				} else {
					output_word_buffer.push_back(wb_wait_for_acks.front().dat);
				}
			} else {
				output_word_buffer.push_back(dat);
			}
			wb_wait_for_acks.pop_front();
			int err = 0;
			if (err == STD_LOGIC_1) {
				err = 1;
			}
			error_shift_reg = (error_shift_reg << 1) | err;
		}		
		send_output_buffer();
		return end_cyc;
	}



	void msi_slave_out(std_logic_t *ack, std_logic_t *err, std_logic_t *rty, std_logic_t *stall, int *dat) {
		*ack = STD_LOGIC_0;
		*err = STD_LOGIC_0;
		*rty = STD_LOGIC_0;
		*stall = STD_LOGIC_0;
		if (msi_slave_out_ack) *ack = STD_LOGIC_1;
		if (msi_slave_out_err) *err = STD_LOGIC_1;
		*dat = 0x0;
	}

	void msi_slave_in(std_logic_t cyc, std_logic_t stb, std_logic_t we, int adr, int dat, int sel) {
		msi_slave_out_ack = false;
		msi_slave_out_err = false;
		if (cyc == STD_LOGIC_1 && stb == STD_LOGIC_1) {
			if (we == STD_LOGIC_1) {
				msi_slave_out_ack = true;
				adr = adr&(eb_msi_adr_last-eb_msi_adr_first);
				msi_queue.push_back(MSI(adr,dat));
				std::cerr << "got MSI" << std::endl;
				// ignore sel
			} else {
				msi_slave_out_err = true; // msi_slave is write-only!
			}
		} 
	}

private:
	struct pollfd pfds[1];	
	std::deque<uint32_t> input_word_buffer;
	std::deque<uint32_t> input_word_buffer2; // only used to echo the input next to the output (not used for bridge logic)
	std::deque<uint32_t> output_word_buffer;
	bool eb_flag_bca;
	bool eb_flag_rca;
	bool eb_flag_rff;
	bool eb_flag_cyc;
	bool eb_flag_wca;
	bool eb_flag_wff;
	uint8_t eb_byte_en, eb_wcount, eb_rcount;

	uint32_t base_write_adr;
	uint32_t base_ret_adr;


	uint32_t error_shift_reg;
	uint32_t eb_sdb_adr;
	uint32_t eb_msi_adr_first;
	uint32_t eb_msi_adr_last;

	bool msi_slave_out_ack;
	bool msi_slave_out_err;

	struct MSI {
		MSI(uint32_t a, uint32_t d) : adr(a), dat(d) {}
		uint32_t adr;
		uint32_t dat;
	};
	std::deque<MSI> msi_queue;
	uint32_t msi_adr;
	uint32_t msi_dat;
	uint32_t msi_cnt;


	// state machine of the EB-slave
	typedef enum{
		EB_SLAVE_STATE_IDLE,
		EB_SLAVE_STATE_EB_HEADER,
		EB_SLAVE_STATE_EB_CONFIG_FIRST,
		EB_SLAVE_STATE_EB_CONFIG_REST,
		EB_SLAVE_STATE_EB_WISHBONE_FIRST,
		EB_SLAVE_STATE_EB_WISHBONE_REST,
	} state_t;
	state_t state;

	uint32_t word_count;

	bool strobe;

	uint32_t new_header;

	struct wb_stb {
		uint32_t adr;
		uint32_t dat;
		bool we;
		bool ack;
		bool err;
		bool passthrough;
		bool zero;
		bool end_cyc;
		bool new_header;
		uint32_t new_header_value;
		std::string comment;
		wb_stb(uint32_t a, uint32_t d, bool w, bool pt = false) 
		: adr(a), dat(d), we(w), ack(false), err(false), passthrough(pt), zero(false), end_cyc(false), new_header(false) {};
	};
	std::deque<wb_stb> wb_stbs;
	std::deque<wb_stb> wb_wait_for_acks;

	bool _stop_until_connected;
	bool _polled;
};




EBslave *slave;

extern "C" 
void eb_simbridge_init(int stop_until_connected, int polled, int sdb_adr, int msi_addr_first, int msi_addr_last) {
	slave = new EBslave(stop_until_connected, polled, sdb_adr, msi_addr_first, msi_addr_last);
}


extern "C" 
void eb_simbridge_master_out(char *cyc, char *stb, char *we, int *adr, int *dat, int *sel, int *end_cyc)
{
	std_logic_t _cyc, _stb, _we;
	*end_cyc = slave->master_out(&_cyc,&_stb,&_we,adr,dat,sel);
	*cyc = (char)_cyc;
	*stb = (char)_stb;
	*we  = (char)_we;
}
extern "C" 
void eb_simbridge_master_in(std_logic_t ack, std_logic_t err, std_logic_t rty, std_logic_t stall, int dat, int *end_cyc)
{
	// std::cerr << "in" << std::endl;
	*end_cyc = slave->master_in(ack,err,rty,stall,dat);
}


extern "C" 
void eb_simbridge_msi_slave_in(std_logic_t cyc, std_logic_t stb, std_logic_t we, int adr, int dat, int sel)
{
	slave->msi_slave_in(cyc,stb,we,adr,dat,sel);
}
extern "C" 
void eb_simbridge_msi_slave_out(char *ack, char *err, char *rty, char *stall, int *dat)
{
	std_logic_t _ack, _err, _rty, _stall;
	slave->msi_slave_out(&_ack,&_err,&_rty,&_stall,dat);
	*ack = (char)_ack;
	*err = (char)_err;
	*rty = (char)_rty;
	*stall = (char)_stall;
}
