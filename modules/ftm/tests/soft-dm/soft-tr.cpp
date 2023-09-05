

#include <etherbone.h>



#include <cstring>

#include <functional>

#include <iostream>

#include <iomanip>



class Handler : public etherbone::Handler {

public:

	eb_status_t read(eb_address_t address, eb_width_t width, eb_data_t* data);

	eb_status_t write(eb_address_t address, eb_width_t width, eb_data_t data);

};



eb_status_t Handler::read(eb_address_t address, eb_width_t width, eb_data_t* data) {

	std::cerr << "Handler::read(" << std::hex << address << ")" << std::endl;

	*data = 0x12345;



	return EB_OK;

}



eb_status_t Handler::write(eb_address_t address, eb_width_t width, eb_data_t data) {

	std::cerr << "Handler::write(" << std::hex << address << "," << std::hex << data << ")" << std::endl;

	return EB_OK;

}





int main(int argc, char *argv[]) {





	etherbone::Socket socket;

	socket.open("12345");





	etherbone::sdb_msi_device eb_slave_sdb;

	eb_slave_sdb.msi_first     = 0;

	eb_slave_sdb.msi_last      = 0xffff;

	eb_slave_sdb.abi_class     = 0;

	eb_slave_sdb.abi_ver_major = 0;

	eb_slave_sdb.abi_ver_minor = 0;

	eb_slave_sdb.bus_specific  = 0x8;//SDB_WISHBONE_WIDTH;

	eb_slave_sdb.sdb_component.addr_first = 0;

	eb_slave_sdb.sdb_component.addr_last  = UINT32_C(0xffffffff);

	eb_slave_sdb.sdb_component.product.vendor_id = 0x651;

	eb_slave_sdb.sdb_component.product.device_id = 0xefaa70;

	eb_slave_sdb.sdb_component.product.version   = 1;

	eb_slave_sdb.sdb_component.product.date      = 0x20150225;

	memcpy(eb_slave_sdb.sdb_component.product.name, "SAFTLIB           ", 19);





	Handler handler;

	socket.attach(&eb_slave_sdb, &handler);



	for (;;) {

		socket.run(1000000);	

		std::cerr << ".";

	}

	// compile: g++ soft-tr.cpp -o soft-tr `pkg-config etherbone --libs --cflags`

	//

	// run program: ./soft-tr

	// 

	// trigger read/write callbacks like this:

	//

	// eb-read -pb tcp/127.0.0.1/12345 0x1000/4

	// eb-write -pb tcp/127.0.0.1/12345 0x1000/4 0xaf12345



	return 0;

}

