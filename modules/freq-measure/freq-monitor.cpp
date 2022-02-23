#include "dic.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include "nu-measurement.h"

std::string get_service_name(const std::string &instance, const std::string &ring, int sid) {
	std::ostringstream service_name;
	service_name << "b2b_" << instance << "_" << ring << "-other-rf_sid" << std::setw(2) << std::setfill('0') << std::dec << sid << "_ext";
	return service_name.str();
}

void service_updated(void* tag, void* data, int *size) {
	int sid = *((int*)tag);
	std::cerr << "sid=" << sid << std::endl;
	NuMeasurement result = *(NuMeasurement*)data;
	std::cerr << result.nuMean << std::endl;
}

std::string error_format(double x, double dx) {
	int precision=log(100.0/dx)/log(10);
	if (precision<0) {
		precision = 0;
	}
	std::ostringstream out;
	out //<< x << " " << dx << " " << precision << " " 
		<< std::fixed << std::setprecision(precision) << x 
	    << "(" << (int)round(dx*pow(10,precision)) << ")";
	return out.str();
}

int main()
{
	std::cerr << error_format(1,20000) << std::endl;
	std::cerr << error_format(1,2000) << std::endl;
	std::cerr << error_format(1,200) << std::endl;
	std::cerr << error_format(1,20) << std::endl;
	std::cerr << error_format(1,2) << std::endl;
	std::cerr << error_format(1,0.2) << std::endl;
	std::cerr << error_format(1,0.02) << std::endl;
	std::cerr << error_format(1,0.002) << std::endl;
	std::cerr << error_format(1,0.0002) << std::endl;

	for (int sid = 0; sid < 16; ++sid) {
		dic_info_service(get_service_name("pro","sis18",sid).c_str(), MONITORED, 1000, 0, 0, &service_updated, sid, 0,0);
	}

	for(;;) {
		sleep(1);
	}
	return 0;
}