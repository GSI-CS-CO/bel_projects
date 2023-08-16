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

int received_MSI = 0;

struct LM32testbench : public saftlib::OpenDevice
                     , public saftlib::Mailbox 
{
	std::unique_ptr<Mailbox::Slot> cpu_msi_slot;
	std::unique_ptr<Mailbox::Slot> host_msi_slot;
	std::unique_ptr<saftlib::IRQ>  msi;

	LM32testbench(saftlib::SAFTd &saftd, const std::string &eb_path) 
		: OpenDevice(saftd.get_etherbone_socket(), eb_path, 1, &saftd)
		, Mailbox(OpenDevice::device)
	{
		const uint32_t CPU_MSI=0x0;
		msi = saftd.request_irq(*this, std::bind(&LM32testbench::receiveMSI,this, std::placeholders::_1));
		std::cerr << std::hex << "msi_adr of host: 0x" << msi->address() << std::dec << std::endl;
		cpu_msi_slot = ConfigureSlot(CPU_MSI);
		host_msi_slot = ConfigureSlot(msi->address());
		std::cerr << "SLOT of LM32: " << cpu_msi_slot->getIndex()  << std::endl;
		std::cerr << "SLOT of HOST: " << host_msi_slot->getIndex() << std::endl;
	}
	bool triggerMSI() {	
		static uint16_t cnt = 0;
		// if (cnt > 5 && received_MSI < cnt - 5) { exit(1); }
		std::cerr << "triggerMSI: " << std::dec << cnt << std::endl;
		cpu_msi_slot->Use((host_msi_slot->getIndex()<<16)|cnt);
		++cnt;
		return true;
	}
	void receiveMSI(eb_data_t data) { 
		std::cerr << "receiveMSI:     " << std::dec << data <<  std::endl;
		received_MSI = data;
		// send another MSI to the LM32. It will respond by sending back an MSI and "receiveMSI" will be called
		triggerMSI();
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

		// start the endless MSI-ping-pong game
		testbench.triggerMSI();

		saftbus::Loop::get_default().run();
	} catch (std::runtime_error &e ) {
		std::cerr << "exception: " << e.what() << std::endl;
	}

	return 0;
}
