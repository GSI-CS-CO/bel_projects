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
#include <exception>

struct LM32testbench : public saftlib::OpenDevice
                     , public saftlib::Mailbox 
{
	int cpu_msi_slot;
	int host_msi_slot;
	eb_address_t msi_adr;

	LM32testbench(saftlib::SAFTd &saftd, const std::string &eb_path) 
		: OpenDevice(saftd.get_etherbone_socket(), eb_path, 500, &saftd)
		, Mailbox(OpenDevice::device)
	{
		const uint32_t CPU_MSI=0x0;
		msi_adr = saftd.request_irq(*this, std::bind(&LM32testbench::receiveMSI,this, std::placeholders::_1));
		std::cerr << std::hex << "msi_adr of host: 0x" << msi_adr << std::dec << std::endl;
		cpu_msi_slot = ConfigureSlot(CPU_MSI);
		host_msi_slot = ConfigureSlot(msi_adr);
		std::cerr << "SLOT of LM32: " << cpu_msi_slot  << std::endl;
		std::cerr << "SLOT of HOST: " << host_msi_slot << std::endl;
	}
	~LM32testbench() {
		FreeSlot(cpu_msi_slot);
	}
	bool triggerMSI() {	
		static int cnt = 0;
		std::cerr << "triggerMSI: " << std::dec << cnt << std::endl;
		UseSlot(cpu_msi_slot, (host_msi_slot<<16)|cnt);
		++cnt;
		return true;
	}
	void receiveMSI(eb_data_t data) { 
		std::cerr << "receiveMSI:     " << std::dec << data <<  std::endl;
		// triggerMSI();
	}

};

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << "usage: " << argv[0] << " <eb-device>" << std::endl;
		return 1;
	}
	saftlib::SAFTd saftd;

	try {

		LM32testbench testbench(saftd, argv[1]);
		// testbench.triggerMSI();

		saftbus::Loop::get_default().connect<saftbus::TimeoutSource>(std::bind(&LM32testbench::triggerMSI,&testbench), std::chrono::milliseconds(100));

		saftbus::Loop::get_default().run();
	} catch (std::runtime_error &e ) {
		std::cerr << "exception: " << e.what() << std::endl;
	}

	return 0;
}
