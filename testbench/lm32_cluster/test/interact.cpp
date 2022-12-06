#ifndef ETHERBONE_THROWS
#define ETHERBONE_THROWS 1
#define __STDC_FORMAT_MACROS
#define __STDC_CONSTANT_MACROS
#endif
#include <etherbone.h>

#include <saftlib/SAFTd.hpp>
#include <saftlib/Mailbox.hpp>
#include <saftlib/OpenDevice.hpp>

#include <saftbus/loop.hpp>

#include <iostream>
#include <vector>

struct LM32testbench : public saftlib::OpenDevice
                    , public saftlib::Mailbox 
{
	LM32testbench(saftlib::SAFTd &saftd, const std::string &eb_path) 
		: OpenDevice(saftd.get_etherbone_socket(), eb_path)
		, Mailbox(OpenDevice::device)
	{}

};

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << "usage: " << argv[0] << " <eb-device>" << std::endl;
		return 1;
	}
	saftlib::SAFTd saftd;

	LM32testbench testbench(saftd, argv[1]);

	int slot_idx = testbench.ConfigureSlot(0x0);
	for (int i = 1;;++i) {
		testbench.UseSlot(slot_idx, i);
	}
	
	return 0;
}