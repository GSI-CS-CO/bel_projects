/*******************************************************************************************
 *  b2b-pname-info.c
 *
 *  created : 2021
 *  author  : Michael Reese, Dietrich Beck GSI-Darmstadt
 *  version : 21-jan-2026
 *
 * a hackish solution providing pattern name information for relevant Sequence IDs
 *
 *********************************************************************************************/
#define B2B_PNAME_VERSION 0x000813

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

static const char* program;

// display help
static void help(void) {
  std::cout << std::endl << "Usage: " << program << " <environment> [OPTIONS]" << std::endl;
  std::cout << std::endl;
  std::cout << "  -h                   display this help and exit" << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "This tool queries pattern / chain names using the 'residump' script and publishes information via DIM" << std::endl;
  std::cout << std::endl;
  std::cout << "Exampe: 'b2b-pname-info pro' for PRO system" << std::endl << std::endl;
  std::cout << std::endl;
  std::cout << "Report bugs to <d.beck@gsi.de> !!!" << std::endl;
  printf("Version %x. Licensed under the GPL v3.", B2B_PNAME_VERSION);
  std::cout << std::endl;
} // help



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
                for (int sid = 0 ; sid < (int)(buffers.size()); ++sid) {
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
               for (int sid = 0; sid < (int)(buffers.size()); ++sid) {
			// clear the buffer content
			std::fill(buffers[sid].begin(), buffers[sid].end(), '\0');
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

int main(int argc, char** argv)
{
  // variables
  std::string  envName = "";
  int          opt;
  
  // parse for options
  program = argv[0];
  while ((opt = getopt(argc, argv, "h")) != -1) {
    switch (opt) {
      case 'h':
        help();
        return 0;
      default:
        std::cerr << program << ": bad getopt result" << std::endl;
        return 1;
    } // switch opt
  }   // while opt

  if (optind >= argc) {
    std::cerr << program << " expecting one non-optional argument: <environmet>" << std::endl;
    help();
    return 1;
  }
  envName = argv[optind];
  
        
  
  try {
    std::string dim_server_name   = "b2b_" + envName + "_ring_pnames";
    std::string dim_sis18_service = "b2b_" + envName + "_sis18";
    std::string dim_esr_service   = "b2b_" + envName + "_esr";
    std::string dim_yr_service    = "b2b_" + envName + "_yr";

    std::vector<PatternNameService> b2b_ring_pnames_services;
    b2b_ring_pnames_services.push_back(PatternNameService(dim_sis18_service, "T=300", "S="));
    b2b_ring_pnames_services.push_back(PatternNameService(dim_esr_service  , "T=340", "S="));
    b2b_ring_pnames_services.push_back(PatternNameService(dim_yr_service   , "T=210", "S="));

    if (!dis_start_serving(dim_server_name.c_str())) {
      throw std::runtime_error("cannot start DIM server");
    }

    std::string magic_command_pro = "/common/usr/cscoap/bin/lsa_residump -t --config=PRO | grep RING_EXTRACTION";
    std::string magic_command_int = "/common/usr/cscoap/bin/lsa_residump -t --config=INT | grep RING_EXTRACTION";
    std::string magic_command = "";

    if       (envName == "pro") magic_command = magic_command_pro;
    else if  (envName == "int") magic_command = magic_command_int;
    else {
        std::cerr << "illegal environment: " << envName << std::endl;
        return 1;
      } // else
    // std::string magic_command = "cat test.txt";
    // std::cout << magic_command << std::endl;
    for(;;) {
      
      std::string script_output = execute_and_capture_output(magic_command);
      for(auto &service: b2b_ring_pnames_services) {
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
}  // main 
