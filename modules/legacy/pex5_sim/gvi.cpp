#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <set>
#include <exception>
#include <cassert>
#include <cstdint>

std::string usage = " [-vv <verilator-version>] -v <verilog-source> -t <top-module> { -c <clk-port> } { -I <verilog-include-path> } { -G <verilator-parameter>=<value> } { -o <verilator-option> }[-g] [-n]\n    -vv Specify the verilator version (default is 5.012)\n    -g  Extends modulename with a hash of the given generics.\n        If not top module and verilator fiel is given, only\n        output the hash value and exit.\n    -n  No trace output (.vcd-file) of the verilated module";


struct Options
{
	template<class T>
	T get_value(int &i, int argc, char *argv[], const std::string &expected, bool verbose = false) {
		if (i+1 < argc) {
			++i;
			T value; 
			if (verbose) {
				value = argv[i];
			} else {
				std::istringstream value_in(argv[i]);
				value_in >> value;
				if (!value_in || value[0] == '-') { throw std::runtime_error(std::string("cannot read ") + expected + " from argument " + argv[i-1] + " " + argv[i]);}
			}
			return value;
		}
		throw std::runtime_error(std::string("expecting ") + expected + " after " + argv[i]);
		return T();
	}
	std::string verilator_version;
	std::string verilog_source;
	std::vector<std::string> system_verilog_sources;
	std::string top_module;
	std::vector<std::string> clk_ports;
	std::vector<std::string> verilog_include_paths;
	std::vector<std::string> verilog_parameter_args;
	std::vector<std::string> verilator_options;
	bool add_generics_hash;
	std::string generics_hash;
	bool no_traces;
	bool unittest;
	bool help;
	Options(int argc, char *argv[])
		: verilog_source(""), top_module(""), add_generics_hash(false), generics_hash(""), no_traces(false), unittest(false), help(false)
	{
		for (int i = 1; i < argc; ++i) {
			std::string argvi = argv[i];
			     if (argvi == "-v") verilog_source = get_value<std::string>(i,argc,argv, "<verilog-source>");
			else if (argvi == "-vv")verilator_version = get_value<std::string>(i,argc,argv, "<verilator-version>");
			else if (argvi == "-t") top_module = get_value<std::string>(i,argc,argv, "<top-module>");
			else if (argvi == "-c") clk_ports.push_back(get_value<std::string>(i,argc,argv, "<clk-port>"));
			else if (argvi == "-I") verilog_include_paths.push_back(get_value<std::string>(i,argc,argv,"<verilog-include-path>"));
			else if (argvi == "-G") verilog_parameter_args.push_back(get_value<std::string>(i,argc,argv,"<verilog-parameter-arg>",true));
			else if (argvi == "-o") verilator_options.push_back(get_value<std::string>(i,argc,argv,"<verilator-option>",true));
			else if (argvi == "-n") no_traces = true;
			else if (argvi == "-g") add_generics_hash = true;
			else if (argvi == "-u") unittest = true;
			else if (argvi == "-h") help = true;
			else if (argvi[0] != '-') system_verilog_sources.push_back(argvi); 
			else throw std::runtime_error(std::string("unknown option ") + argv[i]);
		}
		if (add_generics_hash) {
			generics_hash = generate_generics_hash(verilog_parameter_args);
		}
		if (verilog_source.size()*top_module.size() == 0 && !unittest && !help) {
			if (add_generics_hash && generics_hash.size()) {
				std::cout << generics_hash << std::endl;
				exit(0);
			}
			throw std::runtime_error(std::string("usage: ")+argv[0]+" "+usage);
		}
		if (verilator_version.size() == 0) {
			verilator_version = "5.012";
		}
	}
	std::string generate_generics_hash(const std::vector<std::string> &generics) 
	{
		uint32_t result = 1;
		int i = 0;
		for (auto &gen: generics) {
			for (auto &ch: gen) {
				if (i%2) {
					result *= (uint32_t)ch;
				} else {
					result += (uint32_t)ch;
				}
				++i; 
			}
		}
		std::ostringstream out;
		out << "_" << std::hex << std::setw(8) << std::setfill('0') << result << std::dec;
		return out.str();
	}
};



void extract_portname_and_bitsize(const std::string &token, std::string &portname, int &left_bit, int &right_bit, int &bitsize) {
	// extract portname
	auto begin = token.find('('); ++begin;
	auto end   = token.find(',');
	portname = token.substr(begin,end-begin);
	if (portname[0] == '&') portname = portname.substr(1);
	std::cerr << "portname " << portname << std::endl;

	// extract bit size of port
	begin = end+1;
	end = token.find(')');
	std::istringstream bits_in(token.substr(begin,end-begin));
	char comma;
	bits_in >> left_bit >> comma >> right_bit;
	bitsize = 1+std::max(left_bit,right_bit)-std::min(left_bit,right_bit);
}
void extract_portname_and_bitsize_unittest() {
	std::string portname; 
	int left_bit, right_bit, bitsize;
	// test 1
	extract_portname_and_bitsize("VL_IN8(portname,1,0)", portname, left_bit, right_bit, bitsize);
	assert(portname=="portname");
	assert(left_bit==1);
	assert(right_bit==0);
	assert(bitsize==2);
	// test 2
	extract_portname_and_bitsize("VL_OUT32(stb_o,31,0)", portname, left_bit, right_bit, bitsize);
	assert(portname=="stb_o");
	assert(left_bit==31);
	assert(right_bit==0);
	assert(bitsize==32);
	// test 3
	extract_portname_and_bitsize("VL_OUT32(&stb_o,31,0)", portname, left_bit, right_bit, bitsize);
	assert(portname=="stb_o");
	assert(left_bit==31);
	assert(right_bit==0);
	assert(bitsize==32);
	// something like this is not supported (yet)
	// extract_portname_and_bitsize("VL_OUT32((&trace)[32],0,0)", portname, left_bit, right_bit, bitsize);
}

bool in_token(const std::string &token) {
	
	return token.size() >= 5 && 
	       token.substr(0,5) == "VL_IN" && 
	       token.find("[") == token.npos;  // something like VL_IN8((&events)[32],0,0) are not supported
}
void in_token_unittest() {
	assert(in_token("VL_IN8") == true);
	assert(in_token("VL_IN32") == true);
	assert(in_token("VL_OUT8") == false);
	assert(in_token("VL_OUT32") == false);
	assert(in_token("VL_IN8((&events)[32],0,0)") == false);
}

bool out_token(const std::string &token) {
	return token.size() >= 6 && 
	       token.substr(0,6) == "VL_OUT" &&
	       token.find("[") == token.npos;  // something like VL_OUT8((&events)[32],0,0) are not supported
}
void out_token_unittest() {
	assert(out_token("VL_IN8") == false);
	assert(out_token("VL_IN32") == false);
	assert(out_token("VL_OUT8") == true);
	assert(out_token("VL_OUT32") == true);
	assert(out_token("VL_OUT8((&events)[32],0,0)") == false);
}

struct Port
{
	Port() {}
	Port(const std::string &token) {
		extract_portname_and_bitsize(token, name_orig, left_bit, right_bit, bitsize);
		if (in_token(token)) direction = "in";
		if (out_token(token)) direction = "out";
		tk = token;
		is_array = (bitsize!=1);
		int start_ch = 0;
        // remove "__SYM__"-prefix that is put by veriltor to avoid collisions with c++ keywords
        if (name_orig.find("__SYM__")==0) {
			start_ch = 7; 
		}
		for (int i = start_ch; i < name_orig.size(); ++i) {
			// remove leading underscores and two consecutive underscores
			if (name.size() == 0 && name_orig[i] == '_') continue;
			if (name.size() != 0 && name_orig[i] == '_' && name_orig[i-1] == '_') continue;
			name.push_back(name_orig[i]);
		}
	}
	Port(const Port &p) 
		: name_orig(p.name_orig), name(p.name), direction(p.direction), left_bit(p.left_bit), right_bit(p.right_bit), bitsize(p.bitsize), is_array(p.is_array), tk(p.tk)
		{}
	std::string name_orig;
	std::string name;
	std::string direction;
	int left_bit;
	int right_bit;
	int bitsize;
	bool is_array;
	std::string tk;
};
void port_unittest() {
	Port p("VL_OUT32(out1,31,0");
	assert(p.name_orig == "out1");
	assert(p.name == "out1");
	assert(p.direction == "out");
	assert(p.left_bit == 31);
	assert(p.right_bit == 0);
	assert(p.bitsize == 32);
	assert(p.tk == "VL_OUT32(out1,31,0");
	Port p2(p); // test copy constructor
	assert(p.name_orig == p2.name_orig);
	assert(p.name == p2.name);
	assert(p.direction == p2.direction);
	assert(p.left_bit == p2.left_bit);
	assert(p.right_bit == p2.right_bit);
	assert(p.bitsize == p2.bitsize);
	assert(p.tk == p2.tk);
	Port p3("VL_IN32(__SYS__in1,31,0");
	assert(p3.name_orig == "__SYS__in1");
	assert(p3.name == "SYS_in1");
	assert(p3.direction == "in");
	assert(p3.left_bit == 31);
	assert(p3.right_bit == 0);
	assert(p3.bitsize == 32);
	assert(p3.tk == "VL_IN32(__SYS__in1,31,0");
}

std::string function_name_prefix(const std::string modulename) {
	return modulename + "_gvi_";
}

//////////////////////////////////
// VHDL FILE
//////////////////////////////////

std::string ghdl_verilator_interface_preface(const std::string &modulename) {
	std::string prefix = function_name_prefix(modulename);
	std::ostringstream out;
	out << "library ieee;" << std::endl;
	out << "use ieee.std_logic_1164.all;" << std::endl;
	out << "use ieee.numeric_std.all;" << std::endl;
	out << std::endl;
	out << "package " << modulename << " is" << std::endl;
	out << "\tfunction to_integer(logic_value : std_logic) return integer;" << std::endl;
	out << "\tfunction to_std_logic(integer_value: integer) return std_logic;" << std::endl;
	out << std::endl;
	out << "\tfunction " << prefix << "init return integer;" << std::endl;
	out << "\tattribute foreign of " << prefix << "init : function is \"VHPIDIRECT " << prefix << "init\";";
	out << std::endl;
	out << "\tprocedure " << prefix << "eval(idx : integer);" << std::endl;
	out << "\tattribute foreign of " << prefix << "eval : procedure is \"VHPIDIRECT " << prefix << "eval\";";
	out << std::endl;
	out << "\tprocedure " << prefix << "dump(idx : integer);" << std::endl;
	out << "\tattribute foreign of " << prefix << "dump : procedure is \"VHPIDIRECT " << prefix << "dump\";";
	out << std::endl;
	out << "\tprocedure " << prefix << "timestep(idx : integer; t : time);" << std::endl;
	out << "\tattribute foreign of " << prefix << "timestep : procedure is \"VHPIDIRECT " << prefix << "timestep\";";
	return out.str();
}
void ghdl_verilator_interface_preface_unittest() {
	assert( ghdl_verilator_interface_preface("simple") ==
		"library ieee;\n"
		"use ieee.std_logic_1164.all;\n"
		"use ieee.numeric_std.all;\n"
		"\n"
		"package simple is\n"
		"	function to_integer(logic_value : std_logic) return integer;\n"
		"	function to_std_logic(integer_value: integer) return std_logic;\n"
		"\n"
		"	function simple_gvi_init return integer;\n"
		"	attribute foreign of simple_gvi_init : function is \"VHPIDIRECT simple_gvi_init\";"
		"\n"
		"	procedure simple_gvi_eval(idx : integer);\n"
		"	attribute foreign of simple_gvi_eval : procedure is \"VHPIDIRECT simple_gvi_eval\";"
		"\n"
		"	procedure simple_gvi_dump(idx : integer);\n"
		"	attribute foreign of simple_gvi_dump : procedure is \"VHPIDIRECT simple_gvi_dump\";"
		"\n"
		"	procedure simple_gvi_timestep(idx : integer; t : time);\n"
		"	attribute foreign of simple_gvi_timestep : procedure is \"VHPIDIRECT simple_gvi_timestep\";"
	);
}

std::string ghdl_verilator_interface_middle(const std::string &modulename) {
	std::string prefix = function_name_prefix(modulename);
	std::ostringstream out;
	out << "end package;" << std::endl;
	out << std::endl;
	out << "package body " << modulename << " is" << std::endl;

	out << "\tfunction to_integer(logic_value: std_logic) return integer is" << std::endl;
	out << "\tbegin" << std::endl;
	out << "\t\tif logic_value = '1' then " << std::endl;
	out << "\t\t\treturn 1;" << std::endl;
	out << "\t\telse " << std::endl;
	out << "\t\t\treturn 0;" << std::endl;
	out << "\t\tend if;" << std::endl;
	out << "\tend function;" << std::endl;
	out << std::endl;
	out << "\tfunction to_std_logic(integer_value: integer) return std_logic is" << std::endl;
	out << "\tbegin" << std::endl;
	out << "\t\tif integer_value = 0 then " << std::endl;
	out << "\t\t\treturn '0';" << std::endl;
	out << "\t\telse " << std::endl;
	out << "\t\t\treturn '1';" << std::endl;
	out << "\t\tend if;" << std::endl;
	out << "\tend function;" << std::endl;
	out << std::endl;
	out << "\tfunction " << prefix << "init return integer is" << std::endl;
	out << "\tbegin" << std::endl;
	out << "\t\tassert false report \"VHPI\" severity failure;" << std::endl;
	out << "\tend function;" << std::endl;
	out << std::endl;
	out << "\tprocedure " << prefix << "eval(idx : integer) is" << std::endl;
	out << "\tbegin" << std::endl;
	out << "\t\tassert false report \"VHPI\" severity failure;" << std::endl;
	out << "\tend procedure;" << std::endl;
	out << std::endl;
	out << "\tprocedure " << prefix << "dump(idx : integer) is" << std::endl;
	out << "\tbegin" << std::endl;
	out << "\t\tassert false report \"VHPI\" severity failure;" << std::endl;
	out << "\tend procedure;" << std::endl;
	out << std::endl;
	out << "\tprocedure " << prefix << "timestep(idx : integer; t : time) is" << std::endl;
	out << "\tbegin" << std::endl;
	out << "\t\tassert false report \"VHPI\" severity failure;" << std::endl;
	out << "\tend procedure;" << std::endl;
	return out.str();
}

std::string ghdl_verilator_interface_end(const std::string &modulename) {
	std::string prefix = function_name_prefix(modulename);
	std::ostringstream out;
	out << "end package body;" << std::endl;
	return out.str();
}

std::string ghdl_verilator_entity_begin(const std::string &modulename) {
	std::string prefix = function_name_prefix(modulename);
	std::ostringstream out;
	out << "library ieee;" << std::endl;
    out << "use ieee.std_logic_1164.all;" << std::endl;
    out << "use ieee.numeric_std.all;  " << std::endl;
    out << std::endl;
    out << "use work." << modulename << ".all;" << std::endl;
    out << std::endl;
    // remove the V from the start of the modulename
    out << "entity " << modulename.substr(1) << " is" << std::endl;
    out << "port(" << std::endl;
	return out.str();
}

std::string ghdl_verilator_entity_middle(const std::string &modulename, const std::vector<Port> &ports) {
	std::string prefix = function_name_prefix(modulename);
	std::ostringstream out;
	out << ");" << std::endl;
    out << "end entity;" << std::endl;
    out << std::endl;
    out << "architecture simulation of " << modulename.substr(1) << " is" << std::endl;
    out << "\tsignal " << modulename << "_idx : integer := " << prefix << "init;" << std::endl;
    out << "begin" << std::endl;
    out << "\tmain: process" << std::endl;
    out << "\tbegin" << std::endl;
	return out.str();
}

std::string ghdl_verilator_entity_end(const std::string &modulename) {
	std::string prefix = function_name_prefix(modulename);
	std::ostringstream out;
	out << "\tend process;" << std::endl;
    out << "end architecture;" << std::endl;
    out << std::endl;
	return out.str();
}




std::string ghdl_verilator_interface_function_declaration_in(const std::string &modulename, const Port &port)
{
	std::string prefix = function_name_prefix(modulename);
	// only bitsize of <= 32 for now
	std::ostringstream out;
	if (port.bitsize <= 32) {
		out << "\t--" << port.tk << std::endl;
		out << "\tprocedure " << prefix << port.name << "(idx : integer; " << port.name << " : integer);" << std::endl;
		out << "\tattribute foreign of " << prefix << port.name << " : procedure is \"VHPIDIRECT " << prefix << port.name << "\";";
	} else if (port.bitsize <= 64) {
		out << "\t--" << port.tk << std::endl;
		out << "\tprocedure " << prefix << port.name << "(idx : integer; " << port.name << "_gvi_lo, " << port.name << "_gvi_hi " << " : integer);" << std::endl;
		out << "\tattribute foreign of " << prefix << port.name << " : procedure is \"VHPIDIRECT " << prefix << port.name << "\";";		
	}
	return out.str();
}
std::string ghdl_verilator_interface_function_definition_in(const std::string &modulename, const Port &port)
{
	std::string prefix = function_name_prefix(modulename);
	// only bitsize of <= 32 for now
	std::ostringstream out;
	if (port.bitsize <= 32) {
		out << "\t--" << port.tk << std::endl;
		out << "\tprocedure " << prefix << port.name << "(idx : integer; " << port.name << " : integer) is" << std::endl;
		out << "\tbegin" << std::endl;
		out << "\t\tassert false report \"VHPI\" severity failure;" << std::endl;
		out << "\tend procedure;";
	} else if (port.bitsize <= 64) {
		out << "\t--" << port.tk << std::endl;
		out << "\tprocedure " << prefix << port.name << "(idx : integer; " << port.name << "_gvi_lo, " << port.name << "_gvi_hi " << " : integer) is" << std::endl;
		out << "\tbegin" << std::endl;
		out << "\t\tassert false report \"VHPI\" severity failure;" << std::endl;
		out << "\tend procedure;";		
	}
	return out.str();
}
std::string ghdl_verilator_interface_function_declaration_out(const std::string &modulename, const Port &port)
{
	std::string prefix = function_name_prefix(modulename);
	// only bitsize of <= 32 for now
	std::ostringstream out;
	if (port.bitsize <= 32) {
		out << "\t--" << port.tk << std::endl;
		out << "\tfunction " << prefix << port.name << "(idx : integer) return integer;" << std::endl;
		out << "\tattribute foreign of " << prefix << port.name << " : function is \"VHPIDIRECT " << prefix << port.name << "\";";
	} else if (port.bitsize <= 64) {
		out << "\t--" << port.tk << std::endl;
		out << "\tfunction " << prefix << port.name << "_gvi_lo(idx : integer) return integer;" << std::endl;
		out << "\tattribute foreign of " << prefix << port.name << "_gvi_lo : function is \"VHPIDIRECT " << prefix << port.name << "_gvi_lo\";";
		out << "\tfunction " << prefix << port.name << "_gvi_hi(idx : integer) return integer;" << std::endl;
		out << "\tattribute foreign of " << prefix << port.name << "_gvi_hi : function is \"VHPIDIRECT " << prefix << port.name << "_gvi_hi\";";
	}
	return out.str();
}
std::string ghdl_verilator_interface_function_definition_out(const std::string &modulename, const Port &port)
{
	std::string prefix = function_name_prefix(modulename);
	// only bitsize of <= 32 for now
	std::ostringstream out;
	if (port.bitsize <= 32) {
		out << "\t--" << port.tk << std::endl;
		out << "\tfunction " << prefix << port.name << "(idx : integer) return integer is" << std::endl;
		out << "\tbegin" << std::endl;
		out << "\t\tassert false report \"VHPI\" severity failure;" << std::endl;
		out << "\tend function;";
	} else if (port.bitsize <= 64) {
		out << "\t--" << port.tk << std::endl;
		out << "\tfunction " << prefix << port.name << "_gvi_lo(idx : integer) return integer is" << std::endl;
		out << "\tbegin" << std::endl;
		out << "\t\tassert false report \"VHPI\" severity failure;" << std::endl;
		out << "\tend function;";
		out << "\tfunction " << prefix << port.name << "_gvi_hi(idx : integer) return integer is" << std::endl;
		out << "\tbegin" << std::endl;
		out << "\t\tassert false report \"VHPI\" severity failure;" << std::endl;
		out << "\tend function;";
	}
	return out.str();
}

std::string filename_to_modulename(const std::string &filename)
{
	auto begin = filename.find_last_of('/');
	if (begin == filename.npos) begin = 0;
	else ++begin;
	auto end = filename.find_last_of('.');
	return filename.substr(begin,end-begin);
}
void filename_to_modulename_unittest() {
	assert(filename_to_modulename("simple/Vsimple.h") == "Vsimple");
	assert(filename_to_modulename("generated/lm32_wb/Vlm32_wb.h") == "Vlm32_wb");
}

std::string ghdl_verilator_interface_set_inputs(const std::vector<Port> &ports, const std::string &modulename) {
	std::ostringstream out;
	for (auto port: ports) {
		if (port.bitsize <= 64) {
			if (port.direction == "in") {
				if (port.bitsize == 1 && !port.is_array) {
					out << "\t\t" << function_name_prefix(modulename) << port.name << "(" << modulename << "_idx, to_integer(" << port.name << "));" << std::endl;
				} else if (port.bitsize <= 32) {
					out << "\t\t" << function_name_prefix(modulename) << port.name << "(" << modulename << "_idx, to_integer(signed(" << port.name << ")));" << std::endl;
				} else {
					// the case for bitsize > 32 and <= 64
					if (port.left_bit > port.right_bit) { // the 'downto' case
						out << "\t\t" << function_name_prefix(modulename) << port.name << "(" << modulename << "_idx, to_integer(signed(" << port.name << "(" << port.right_bit+31 << " downto " << port.right_bit << "))), to_integer(signed(" << port.name << "(" << port.left_bit << " downto " << port.right_bit+32 << "))));" << std::endl;
					} else { // the 'to' case
						out << "\t\t" << function_name_prefix(modulename) << port.name << "(" << modulename << "_idx, to_integer(signed(" << port.name << "(" << port.right_bit << " to " << port.right_bit+31 << "))), to_integer(signed(" << port.name << "(" << port.right_bit+32 << " downto " << port.left_bit << "))));" << std::endl;
					}
				}
			}
		}
	}
	return out.str();	
}

std::string ghdl_verilator_interface_get_outputs(const std::vector<Port> &ports, const std::string &modulename) {
	std::ostringstream out;
	for (auto port: ports) {
		if (port.bitsize <= 64) {
			if (port.direction == "out") {
				out << "\t\t" << port.name << " <= ";
				if (port.bitsize == 1 && !port.is_array) {
					out << "to_std_logic(" << function_name_prefix(modulename) << port.name << "(" << modulename << "_idx));" << std::endl;
				} else if (port.bitsize <= 32) {
					out << "std_logic_vector(to_signed(" << function_name_prefix(modulename) << port.name << "(" << modulename << "_idx), " << port.bitsize << "));" << std::endl;					
				} else {
					// case for bitsize > 32 and <= 64
					out << "std_logic_vector(to_signed(" << function_name_prefix(modulename) << port.name << "_gvi_hi(" << modulename << "_idx), " << port.bitsize-32 << ")) & std_logic_vector(to_signed(" << function_name_prefix(modulename) << port.name << "_gvi_lo(" << modulename << "_idx), " << 32 << "));" << std::endl;					
				}
			}
		}
	}
	return out.str();	
}

void write_vhdl_file(std::ofstream &vhd_out, const std::vector<Port> &ports, const std::vector<Port> &clk_ports, const std::string &modulename)
{
	vhd_out << ghdl_verilator_interface_preface(modulename) << std::endl << std::endl;
	for(auto port: ports) {
		if (port.direction == "in") vhd_out << ghdl_verilator_interface_function_declaration_in(modulename, port) << std::endl << std::endl;
		if (port.direction == "out") vhd_out << ghdl_verilator_interface_function_declaration_out(modulename, port) << std::endl << std::endl;
	}
	vhd_out << ghdl_verilator_interface_middle(modulename) << std::endl << std::endl;
	for(auto port: ports) {
		if (port.direction == "in") vhd_out << ghdl_verilator_interface_function_definition_in(modulename, port) << std::endl << std::endl;
		if (port.direction == "out") vhd_out << ghdl_verilator_interface_function_definition_out(modulename, port) << std::endl << std::endl;
	}
	vhd_out << ghdl_verilator_interface_end(modulename) << std::endl;

	vhd_out << ghdl_verilator_entity_begin(modulename);	
	for (int i = 0; i < ports.size(); ++i) {
		if (ports[i].bitsize > 64) continue;
		if (i > 0) vhd_out << ";" << std::endl;
		vhd_out << "\t" << ports[i].name << " : " << ports[i].direction;
		if (ports[i].bitsize == 1 && !ports[i].is_array) vhd_out << " std_logic";
		else {
			vhd_out << " std_logic_vector(" << ports[i].left_bit;
			if (ports[i].left_bit > ports[i].right_bit) vhd_out << " downto ";
			else                                        vhd_out << " to ";
			vhd_out << ports[i].right_bit << ")";
		} 
		// if (i < ports.size()-1) vhd_out << ";";
		// vhd_out << std::endl;
	}
	vhd_out << ghdl_verilator_entity_middle(modulename, ports);
	vhd_out << "\t\twait for 0 ns;" << std::endl;
	vhd_out << "\t\twhile true loop" << std::endl;
	vhd_out << ghdl_verilator_interface_set_inputs(ports, modulename);
	vhd_out << "\t\t" << function_name_prefix(modulename) << "timestep(" << modulename << "_idx, now);" << std::endl;
	vhd_out << "\t\t" << function_name_prefix(modulename) << "eval(" << modulename << "_idx);" << std::endl;
	vhd_out << "\t\t" << function_name_prefix(modulename) << "dump(" << modulename << "_idx);" << std::endl;
	vhd_out << ghdl_verilator_interface_get_outputs(ports, modulename);
	vhd_out << "\t\twait until " << clk_ports.front().name << "'event";
	if (clk_ports.size() > 1) {
		for (int i = 1; i < clk_ports.size(); ++i) {
			vhd_out << " or " << clk_ports[i].name << "'event";
		}
	}
	vhd_out << ";" << std::endl;
	vhd_out << "\t\tend loop;" << std::endl;

	vhd_out << ghdl_verilator_entity_end(modulename);	

}

////////////////////////////
//// CPP FILE
////////////////////////////

std::string cpp_verilator_interface_preface(const std::string &modulename, bool no_traces) {
	std::string prefix = function_name_prefix(modulename);
	std::ostringstream out;

	out << "#include <verilated.h>          // Defines common routines" << std::endl;
	out << "#include \"" << modulename << ".h\"          // From Verilating \"lm32_top.v\"" << std::endl;
	out << "" << std::endl;
	if (!no_traces) {
		out << "#if VM_TRACE" << std::endl;
		out << "# include <verilated_vcd_c.h>   // Trace file format header" << std::endl;
		out << "#endif" << std::endl;
		out << "" << std::endl;
	}
	out << "#include <iostream>" << std::endl;
	out << "#include <iomanip>" << std::endl;
	out << "#include <vector>" << std::endl;
	out << "#include <memory>" << std::endl;
	out << "#include <cstdint>" << std::endl;
	out << "#include <climits>" << std::endl;
	out << "" << std::endl;
	out << "// Container for all lm32 instances that will ever be instantiated" << std::endl;
	out << "// Users will work with an index into this container." << std::endl;
	out << "std::vector<" << modulename << "*> " << modulename << "_top_instances;" << std::endl;
	if (!no_traces) {
		out << "std::vector<VerilatedVcdC*> " << modulename << "_tfp_instances;" << std::endl;
	}
	out << "" << std::endl;
	out << "extern double main_time;       // Current simulation time" << std::endl;
	out << "double sc_time_stamp();" << std::endl;
	// out << "double main_time = 0;       // Current simulation time" << std::endl;
	// out << "// This is a 64-bit integer to reduce wrap over issues and" << std::endl;
	// out << "// allow modulus.  You can also use a double, if you wish." << std::endl;
	// out << "double sc_time_stamp () {       // Called by $time in Verilog" << std::endl;
	// out << "\treturn main_time;           // converts to double, to match" << std::endl;
	// out << "                                // what SystemC does" << std::endl;
	// out << "}" << std::endl;
	out << "" << std::endl;
	out << "// GHDL interface" << std::endl;
	out << "extern \"C\" {" << std::endl;
	out << "\tint " << function_name_prefix(modulename) << "init(int *pts) {" << std::endl;
	out << "\t\tint idx = " << modulename << "_top_instances.size();" << std::endl;
	out << "\t\t" << modulename << "_top_instances.push_back(new "<< modulename<< ");" << std::endl;
	if (!no_traces) {
		out << "\t\tVerilated::traceEverOn(true);   // Verilator must compute traced signals" << std::endl;
		out << "\t\t" << modulename << "_tfp_instances.push_back(new VerilatedVcdC);" << std::endl;
		out << "\t\t" << modulename << "_top_instances[idx]->trace(" << modulename << "_tfp_instances[idx], 99);   // Trace 99 levels of hierarchy" << std::endl;
	}
	out << "\t\tstd::ostringstream filename;" << std::endl;
	out << "\t\tfilename << \"" << modulename << "_vlt_dump_\" << std::setw(2) << std::setfill('0') << std::dec << idx << \".vcd\";" << std::endl;
	if (!no_traces) {
		out << "\t\t" << modulename << "_tfp_instances[idx]->open(filename.str().c_str()); // Open the dump file" << std::endl;
	}
	out << "\t\t//std::cout << \"interface_lm32_init in C++ called. returing index \" << idx << std::endl;" << std::endl;
	out << "\t\treturn idx;" << std::endl;
	out << "\t}" << std::endl;

	out << "\tvoid " << function_name_prefix(modulename) << "eval(int idx) {" << std::endl;
	out << "\t\t" << modulename << "_top_instances[idx]->eval();" << std::endl;
	out << "\t}" << std::endl;

	out << "\tvoid " << function_name_prefix(modulename) << "dump(int idx) {" << std::endl;
	if (!no_traces) {
		out << "\t\tif (" << modulename << "_tfp_instances[idx]) " << modulename << "_tfp_instances[idx]->dump(main_time); // Create waveform trace for this timestamp" << std::endl;
	}
	out << "\t}" << std::endl;

	out << "\tvoid " << function_name_prefix(modulename) << "timestep(int idx, uint64_t time) {" << std::endl;
	out << "\t\tmain_time = time/1000.0;" << std::endl;
	out << "\t}" << std::endl;

	return out.str();
}

std::string cpp_verilator_interface_function_definition_in(const std::string &modulename, const Port &port)
{
	std::string prefix = function_name_prefix(modulename);
	// only bitsize of <= 32 for now
	std::ostringstream out;
	if (port.bitsize <= 32) {
		out << "\tvoid " << function_name_prefix(modulename) << port.name << "(int idx, int " << port.name_orig << ") {" << std::endl;
		out << "\t\t" << modulename << "_top_instances[idx]->" << port.name_orig << " = " << port.name_orig << ";" << std::endl;
		out << "\t}" << std::endl;
	} else if (port.bitsize <= 64) {
		out << "\tvoid " << function_name_prefix(modulename) << port.name << "(int idx, int " << port.name << "_gvi_lo, int " << port.name << "_gvi_hi" << ") {" << std::endl;
		out << "\t\t" << modulename << "_top_instances[idx]->" << port.name_orig << " = (unsigned)" << port.name << "_gvi_hi;" << std::endl;
		out << "\t\t" << modulename << "_top_instances[idx]->" << port.name_orig << " <<= 32;" << std::endl;
		out << "\t\t" << modulename << "_top_instances[idx]->" << port.name_orig << " |= (unsigned)" << port.name << "_gvi_lo;" << std::endl;
		out << "\t}" << std::endl;
	}
	return out.str();
}
std::string cpp_verilator_interface_function_definition_out(const std::string &modulename, const Port &port)
{
	std::string prefix = function_name_prefix(modulename);
	// only bitsize of <= 32 for now
	std::ostringstream out;
	if (port.bitsize <= 32) {
		out << "\tint " << function_name_prefix(modulename) << port.name << "(int idx) {" << std::endl;
		out << "\t\treturn " << modulename << "_top_instances[idx]->" << port.name_orig << ";" << std::endl;
		out << "\t}" << std::endl;
	} else if (port.bitsize <= 64) {
		out << "\tint " << function_name_prefix(modulename) << port.name << "_gvi_lo(int idx) {" << std::endl;
		out << "\t\treturn " << modulename << "_top_instances[idx]->" << port.name_orig << ";" << std::endl;
		out << "\t}" << std::endl;		
		out << "\tint " << function_name_prefix(modulename) << port.name << "_gvi_hi(int idx) {" << std::endl;
		out << "\t\treturn " << modulename << "_top_instances[idx]->" << port.name_orig << " >> 32;" << std::endl;
		out << "\t}" << std::endl;		
	}
	return out.str();
}

void write_cpp_file(std::ofstream &cpp_out, const std::vector<Port> &ports, const std::string &modulename, bool no_traces)
{
	cpp_out << cpp_verilator_interface_preface(modulename, no_traces) << std::endl;
	for (auto port: ports) {
		if (port.bitsize <= 64) {
			if (port.direction == "in") cpp_out << cpp_verilator_interface_function_definition_in(modulename, port) << std::endl;
			if (port.direction == "out") cpp_out << cpp_verilator_interface_function_definition_out(modulename, port) << std::endl;
		}
	}
	cpp_out << "}" << std::endl;

}

void write_common_cpp_file(std::ofstream &cpp_out)
{
	cpp_out << "double main_time;" << std::endl;
	cpp_out << "double sc_time_stamp() { return main_time; }" << std::endl;
}

void unittest() {
	extract_portname_and_bitsize_unittest();
	in_token_unittest();
	out_token_unittest();
	port_unittest();
	ghdl_verilator_interface_preface_unittest();
	filename_to_modulename_unittest();
}

std::set<std::string> extract_module_ports(const Options &options)
{
	std::string result;
	std::set<std::string> array_ports;	
	std::ifstream vin(options.verilog_source);
	std::string token;
	for (;;) {
		vin >> token;
		if (!vin) break;
		if (token == "module") {
			std::string modulename;
			vin >> modulename;
			if (!vin) break;
			if (modulename == options.top_module) {
				char hash;
				vin >> hash;
				if (!vin) break;
				if (hash == '#') {
					int nesting = 0;
					for (;;) {
						char c;
						c = vin.get();
						if (!vin) break;
						if (c == '(') ++nesting;
						if (c == ')') --nesting;
						//result.push_back(c);
						if (nesting == 0) break;
					}
					vin >> hash;
					if (!vin) break;
				}
				if (hash == '(') // there was no parameter list
				{
					result.push_back('(');
					int nesting = 1;
					bool is_array = false;
					for (;;) {
						char c;
						c = vin.get();
						if (!vin) break;
						if (c == '(') ++nesting;
						if (c == ')') --nesting;
						if (c == '[') is_array = true;
						if (c == ',' || (c == ')' && nesting == 0)) {
							if (is_array) {
								// find the last token before the comma ','
								int i_begin = result.size()-1;
								while(i_begin && isspace(result[i_begin])) --i_begin;
								int i_end = i_begin+1;
								while(i_begin && !isspace(result[i_begin])) --i_begin;
								if (isspace(result[i_begin])) ++i_begin;
								std::string arrayport = result.substr(i_begin, i_end-i_begin);
								// std::cerr << "arrayport " << arrayport << std::endl;
								array_ports.insert(arrayport);
							}
							is_array = false;
						}
						if (c == '/') { 
							char c2 = vin.get();
							if (!vin) break;
							if (c2 == '/') { // skip the line comment
								std::string line;
								std::getline(vin,line);
								if (!vin) break;
								c = '\n';
							} else if (c2 == '*') { // skip the block comment
								char c3 = vin.get();
								if (!vin) break;
								for (;;) {
									char c4 = vin.get();
									if (!vin || (c3 == '*' && c4 == '/')) break;
									c3 = c4;
								}
								continue;
							} else {
								vin.putback(c2);
							}
						}
						result.push_back(c);
						if (nesting == 0) return array_ports;
					}
				}
			}
		}
	}
	return array_ports;
}

// std::string extract_module_parameters(const Options &options)
// {
// 	std::string result;
// 	std::ifstream vin(options.verilog_source);
// 	std::string token;
// 	for (;;) {
// 		vin >> token;
// 		if (!vin) break;
// 		if (token == "module") {
// 			std::string modulename;
// 			vin >> modulename;
// 			if (!vin) break;
// 			if (modulename == options.top_module) {
// 				char hash;
// 				vin >> hash;
// 				if (!vin) break;
// 				if (hash == '#') {
// 					for (;;) {
// 						char c;
// 						c = vin.get();
// 						if (!vin) break;
// 						result.push_back(c);
// 						if (c == ')') return result;
// 					}
// 				}
// 			}
// 		}
// 	}
// 	return result;
// }
// std::string parameter_verilog_to_vhdl(const std::string &par) 
// {
// 	return par;
// }
// std::string transform_module_parameters_verilog_to_vhdl(const std::string &in)
// {
// 	std::istringstream pin(in);
// 	std::string result;
// 	std::string token;
// 	for (;;) {
// 		char c;
// 		pin >> c;
// 		if (!pin) break;
// 		token.push_back(c);
// 		if (token == "(") {
// 			result.append("port map (\n");
// 			token.clear();
// 		} else if (token == ")") {
// 			result.append(")");
// 			token.clear();
// 			break;
// 		} else if (token == "parameter") {
// 			for (;;) {
// 				c = pin.get();
// 				if (c == ',' || c == ')') {
// 					result.append("\t");
// 					result.append(parameter_verilog_to_vhdl(token));
// 					if (c == ')') {
// 						result.append("\n");
// 						result.append(")");
// 					} else {
// 						result.append(";\n");
// 					}
// 					break;
// 				} else if (c != '\n' && c != '\r') {
// 					token.push_back(c);
// 				}
// 			}
// 			token.clear();
// 		}
// 	}
// 	return result;
// }

void generate_ghdl_verilator_interface(const Options &options) 
{

	std::string generated_verilator_header(".gvi/");
	generated_verilator_header.append(options.top_module + options.generics_hash);
	generated_verilator_header.append("/V");
	generated_verilator_header.append(options.top_module + options.generics_hash);
	generated_verilator_header.append(".h");
	std::string basename(generated_verilator_header.c_str());
	basename = basename.substr(0,basename.find_last_of("/"));
	basename.append("/");
	basename.append(options.top_module + options.generics_hash);
	basename.append("_wrapper");

	std::set<std::string> ports_that_are_arrays = extract_module_ports(options);
	// call verilator
	std::string verilator_call;
	verilator_call.append("verilator -Wno-lint --trace --cc ");
	if (options.system_verilog_sources.size() > 0) {
		verilator_call.append(" -sv "); // enable SystemVerilog parsing
	}
	for (int i = 0; i < options.system_verilog_sources.size(); ++i) {
		verilator_call.append(" ");
		verilator_call.append(options.system_verilog_sources[i]);
		verilator_call.append(" ");
	}
	verilator_call.append(options.verilog_source);
	verilator_call.append(" --top-module ");
	verilator_call.append(options.top_module);
	for (int i = 0; i < options.verilator_options.size(); ++i) {
		verilator_call.append(" ");
		verilator_call.append(options.verilator_options[i]);
	}
	for (int i = 0; i < options.verilog_parameter_args.size(); ++i) {
		verilator_call.append(" \'-G");
		verilator_call.append(options.verilog_parameter_args[i]);
		verilator_call.append("\'");
	}
	for (int i = 0; i < options.verilog_include_paths.size(); ++i) {
		verilator_call.append(" -I");
		verilator_call.append(options.verilog_include_paths[i]);
	}

	system("mkdir -p .gvi");
	verilator_call.append(" --Mdir .gvi/");
	verilator_call.append(options.top_module + options.generics_hash);
	verilator_call.append(" --prefix V");
	verilator_call.append(options.top_module + options.generics_hash);
	verilator_call.append(" --exe ");
	verilator_call.append(options.top_module + options.generics_hash);
	verilator_call.append("_wrapper_main.cpp");

	std::cout << "gvi: execute command: " << verilator_call << std::endl;
	int verilator_status = system(verilator_call.c_str());
	if (verilator_status < 0) {
		throw std::runtime_error("failed to run verilator");
	} else {
		// std::cerr << "WEXITSTATUS(verilator_status)=" << WEXITSTATUS(verilator_status) << std::endl;
		if (WEXITSTATUS(verilator_status)) {
			throw std::runtime_error("verilator returned with error");
		}
	}


	std::cout << "gvi: generating ghdl bindings for verilated model" << std::endl;

	std::ifstream in(generated_verilator_header.c_str());
	if (!in) {
		throw std::runtime_error(std::string("cannot open file ") + generated_verilator_header);
	}


	std::string modulename = filename_to_modulename(generated_verilator_header);

	// verilator generated a top module C++ header file
	// look at this header file to find the ports (names and bitsizes)
	std::vector<Port> ports;
	for (;;) {
		std::string token;
		in >> token;
		if (!in) {
			break;
		}
		if (in_token(token) || out_token(token)) {
			ports.push_back(Port(token));
			// fix the arrays that have only 1 element
			if (ports_that_are_arrays.find(ports.back().name)!=ports_that_are_arrays.end()) {
				ports.back().is_array = true;
			}
		}
	}

	std::vector<Port> clk_ports;
	// check if options.clk_ports are present
	for (auto &clk_port: options.clk_ports) {
		bool clk_port_found = false;
		for (auto &port: ports) {
			if (port.name == clk_port && port.direction == "in") {
				clk_port_found = true;
				clk_ports.push_back(port);
			}
		}
		if (!clk_port_found) {
			throw std::runtime_error(std::string("clock port \'-c ") + clk_port + "\' was not found among top module ports");
		}
	}
	// try autodetect clk ports if none are provided by command line arguments
	if (options.clk_ports.size()==0) {
		Port clk_port;
		for (auto port: ports) {
			if (port.direction == "in" && (
					port.name.find("clk") != port.name.npos || 
					port.name.find("clock") != port.name.npos) 
				) {
				if (port.name.find("_en") == port.name.npos) {
					std::cerr << "gvi: autdetected clk port: " << port.name << std::endl;
					clk_ports.push_back(port);
				}
			}
		}
	}
	if (clk_ports.size() == 0) {
		throw std::runtime_error("no clk_ports found, specify at least one port name via \"-c <clk-port-name>\"");
	}

	std::ofstream vhd_out(basename+".vhd");
	std::ofstream cpp_out(basename+"_c.cpp");
	std::ofstream main_out(basename+"_main.cpp");
	std::ofstream flags_out(basename+".flags");
	std::ofstream common_cpp_out(".gvi/common.cpp");
	std::ofstream common_flags_out(".gvi/common.flags");


	main_out << "int main() {}" << std::endl;

	write_vhdl_file(vhd_out, ports, clk_ports, modulename);
	write_cpp_file(cpp_out, ports, modulename, options.no_traces);
	write_common_cpp_file(common_cpp_out);
	// compile common code
	std::string gcc_call_compile_common;
	gcc_call_compile_common.append("gcc -c .gvi/common.cpp -o .gvi/common.o");
	int gcc_common_status = system(gcc_call_compile_common.c_str());
	if (gcc_common_status < 0) {
		throw std::runtime_error("failed to run gcc to compile common code");
	} else {
		if (WEXITSTATUS(gcc_common_status)) {
			throw std::runtime_error("gcc returned with error");
		}
	}
	// all code generation done, call verilator generated makefile
	std::string make_call_verilator;
	make_call_verilator.append("make -C .gvi/");
	make_call_verilator.append(options.top_module + options.generics_hash);
	make_call_verilator.append(" -f V");
	make_call_verilator.append(options.top_module + options.generics_hash);
	make_call_verilator.append(".mk");
	std::cout << "gvi: execute command: " << make_call_verilator << std::endl;
	int make_status = system(make_call_verilator.c_str());
	if (make_status < 0) {
		throw std::runtime_error("failed to run make on the verilator generated makefile");
	} else {
		// std::cerr << "WEXITSTATUS(make_status)=" << WEXITSTATUS(make_status) << std::endl;
		if (WEXITSTATUS(make_status)) {
			throw std::runtime_error("make on the verilator generated makefile returned with error");
		}
	}

	// find verilator installation directory
	std::cerr << "gvi: find verilator prefix: ";
	std::string which_verilator_output;
	FILE *fp;
	if ((fp = popen("which verilator", "r")) == NULL) {
		throw std::runtime_error("cannot determine the location of verilator execuable");
	}
	char buffer[1024];
	while (fgets(buffer, 1024, fp) != NULL) {
		which_verilator_output.append(buffer);
	}
    if(pclose(fp))  {
		throw std::runtime_error("cannot determine the location of verilator execuable");
    }	
	std::string verilator_path = which_verilator_output.substr(0,which_verilator_output.find("/bin/verilator"));
	std::cerr << verilator_path << std::endl;

	std::string compile_vhdl_wrapper;
	compile_vhdl_wrapper.append("g++ -DVM_TRACE -I.gvi/");
	compile_vhdl_wrapper.append(options.top_module + options.generics_hash);
	//compile_vhdl_wrapper.append(" -I/usr/share/verilator/include -c ");
	compile_vhdl_wrapper.append(" -I");
	compile_vhdl_wrapper.append(verilator_path);
	compile_vhdl_wrapper.append("/share/verilator/include/vltstd");
	compile_vhdl_wrapper.append(" -I");
	compile_vhdl_wrapper.append(verilator_path);
	compile_vhdl_wrapper.append("/share/verilator/include -c .gvi/");
	compile_vhdl_wrapper.append(options.top_module + options.generics_hash);
	compile_vhdl_wrapper.append("/");
	compile_vhdl_wrapper.append(options.top_module + options.generics_hash);
	compile_vhdl_wrapper.append("_wrapper_c.cpp");
	compile_vhdl_wrapper.append(" -o .gvi/");
	compile_vhdl_wrapper.append(options.top_module + options.generics_hash);
	compile_vhdl_wrapper.append("/");
	compile_vhdl_wrapper.append(options.top_module + options.generics_hash);
	compile_vhdl_wrapper.append("_wrapper_c.o");
	std::cout << "gvi: execute command: " << compile_vhdl_wrapper << std::endl;
	int gcc_status = system(compile_vhdl_wrapper.c_str());
	// g++ -DVM_TRACE -Ipicorv32_wb -I/usr/share/verilator/include -c picorv32_wb/Vpicorv32_wb_gvi_c.cpp -o picorv32_wb/Vpicorv32_wb_gvi_c.o
	if (gcc_status < 0) {
		throw std::runtime_error("failed to run gcc for compilation of glue code");
	} else {
		// std::cerr << "WEXITSTATUS(gcc_status)=" << WEXITSTATUS(gcc_status) << std::endl;
		if (WEXITSTATUS(gcc_status)) {
			throw std::runtime_error("gcc returned with error");
		}
	}
	std::cout << "gvi: generate ghdl flags" << std::endl;
	flags_out << "-Wl,.gvi/" << options.top_module + options.generics_hash << "/" << options.top_module + options.generics_hash << "_wrapper_c.o " 
	          << "-Wl,.gvi/" << options.top_module + options.generics_hash << "/V" << options.top_module + options.generics_hash << "__ALL.a "
	          << std::endl; 
	common_flags_out << "-Wl,.gvi/common.o "
					 << "-Wl,.gvi/" << options.top_module + options.generics_hash << "/verilated.o ";
	if (options.verilator_version[0] == '5') {
		common_flags_out << "-Wl,.gvi/" << options.top_module + options.generics_hash << "/verilated_threads.o ";
	}
	common_flags_out << "-Wl,.gvi/" << options.top_module + options.generics_hash << "/verilated_vcd_c.o "
					 << "-Wl,-lm -Wl,-lstdc++ ";

}

int main(int argc, char *argv[])
{
	try {
		Options options(argc,argv);

		if (options.help) {
			std::cout << "usage: " << argv[0] << usage << std::endl;
			return 0;
		}

		if (options.add_generics_hash) {
			std::cout << "gvi: generics hash " << options.generics_hash << std::endl;
		}

		if (options.unittest) {
			std::cerr << "gvi: running all unittests" << std::endl;
			unittest();
			std::cerr << "gvi: all unittests successful" << std::endl;
			return 0;
		}

		// do the work
		generate_ghdl_verilator_interface(options);

	} catch (std::exception &e) {
		std::cerr << "gvi error: " << e.what() << std::endl;
		return -1;
	}

	return 0;
}
