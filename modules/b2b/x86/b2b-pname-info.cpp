/*******************************************************************************************
 *  b2b-pname-info.c
 *
 *  created : 2021
 *  author  : Michael Reese, GSI-Darmstadt
 *  version : 13-Dec-2021
 *
 * a hackish solution providing pattern name information for relevant Sequence IDs
 *
 *********************************************************************************************/
#include <dis.h>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <stdexcept>

// run the command, read the output (stdout, not stderr) and return it as a std::string
std::string execute_and_capture_output(const std::string& command) {
	std::string output;
	char buffer[16];
	FILE *stream = popen(command.c_str(), "r");
	if (!stream) {
		throw std::runtime_error("cannot execute command: " + command);
	}
	while (!feof(stream)) {
		if (fgets(buffer, sizeof(buffer)/sizeof(buffer[0]), stream) != NULL) output.append(buffer);
	}
	int return_code = pclose(stream);
	if (return_code != 0) {
		std::ostringstream msg;
		msg << "command returned error code (" << return_code << ")" << std::endl;
		msg << "     command: " << command << std::endl;
		msg << "     output: " << output << std::endl;
		throw std::runtime_error(msg.str());
	}
	return output;
}

struct PatternNameService {
	std::string                     service_prefix;  // prepend this to name of the each dim service
	std::string                     ring_identifier; // how to recognize the relevant lines in the output of the script
	std::string                     sid_prefix;      // how to recognize the SID in the output of the script
	std::vector<std::vector<char> > buffers;         // the data buffers for the dim service
	std::vector<int>                service_ids;     // as returned from dis_add_service()
	std::vector<std::string>        service_names;

	PatternNameService(const std::string &prefix, const std::string &identifier, const std::string &s_prefix) 
		: service_prefix(prefix) 
		, ring_identifier(identifier)
		, sid_prefix(s_prefix)
		, buffers(16, std::vector<char>(128,0))
		, service_ids(16)
		, service_names(16)
	{
		// build service name and add the service
		for (int sid = 0 ; sid < buffers.size(); ++sid) {
			std::ostringstream service_name;
			service_name << service_prefix << "-pname" << "_sid" << std::setw(2) << std::setfill('0') << sid;
			service_ids[sid] = dis_add_service(service_name.str().c_str(), "C", &buffers[sid][0], buffers[sid].size(), 0, 0);
			service_names[sid] = service_name.str();
		}
	}

	// returns true on success, i.e. if the buffer content could be extracted from line
	bool extract_pattern_name_and_fill_buffer(std::vector<char> &buffer, const std::string& line) {
		// line looks like this:
		//"   | DRYRUN_202111_SIS18_FAST_TE_ESR.C1.SIS18_RING.BEAMOUT_INIT.1   | FAIR.SELECTOR.C=1:T=300:S=1:P=1 | 0 |  226000 | SIS18_RING  | "
		//      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ 
		//      this is what we want to extract (everything before the first '.')
		std::istringstream lin(line);
		// read the '|' character at the beginning of the line, followed by the pattern name
		std::string vertical_bar, pattern_name; 
		lin >> vertical_bar >> pattern_name;
		if (!lin) return false;
		if (vertical_bar != "|") return false; // the vertical bar '|' must be present
		// copy the content of pattern_name into buffer until the first '.' 
		auto length = std::min(std::min(buffer.size(), pattern_name.size()), pattern_name.find("."));
		std::copy_n(pattern_name.begin(), length, buffer.begin());
		// make sure buffer is 0-terminated
		buffer.back() = '\0'; 
		return true;
	}

	void process_script_output(const std::string& script_output) {
		for (int sid = 0; sid < buffers.size(); ++sid) {
			buffers[sid].resize(buffers[sid].size(),'\0'); // clear the buffer content
			// build a SID search string, e.g. "S=10"
			std::ostringstream sid_str;
			sid_str << sid_prefix << sid;
			// go through the script_output line-by-line
			std::istringstream in(script_output);
			std::string line;
			for (std::getline(in,line); in; std::getline(in, line)) {
				if (line.find(ring_identifier) == line.npos) continue;
				if (line.find(sid_str.str())   == line.npos) continue;
				if (extract_pattern_name_and_fill_buffer(buffers[sid], line)) break;
			}
			dis_update_service(service_ids[sid]);
		}		
	}
};

int main()  
{
	try {
		std::string dim_server_name = "b2b_pro_ring_pnames";
		std::vector<PatternNameService> b2b_pro_ring_pnames_services;
		b2b_pro_ring_pnames_services.push_back(PatternNameService("b2b_pro_sis18", "T=300", "S="));
		b2b_pro_ring_pnames_services.push_back(PatternNameService("b2b_pro_esr",   "T=340", "S="));
		b2b_pro_ring_pnames_services.push_back(PatternNameService("b2b_pro_yr",    "T=210", "S="));

		if (!dis_start_serving(dim_server_name.c_str())) {
			throw std::runtime_error("cannot start DIM server");
		}

                std::string magic_command = "/common/usr/lsa/bin/lsa_residump -t";
                // std::string magic_command = "cat test.txt";
		for(;;) { 
                  	std::string script_output = execute_and_capture_output(magic_command);
			for(auto &service: b2b_pro_ring_pnames_services) {
				service.process_script_output(script_output);
				// write the buffer content to stderr 
				// for (int sid = 0; sid < service.buffers.size(); ++sid) {
				// 	std::cerr << service.service_names[sid] << " : " << &service.buffers[sid][0] << std::endl;
				// }
                                }
			sleep(60);  
                        }  
	} catch (std::runtime_error &e) {
		std::cerr << "error in DIM server for ring pattern names: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}  
