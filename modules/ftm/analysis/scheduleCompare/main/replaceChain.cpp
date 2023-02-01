#include <stdio.h>
#include <unistd.h>
#include <regex>

#include "replaceChain.h"
#include "replaceChainImpl.h"

int main(int argc, char* argv[]) {
  int error = 0;
  int opt;
  char* program = argv[0];
  configuration config;
  while ((opt = getopt(argc, argv, "1c:ho:sv")) != -1) {
    switch (opt) {
      case '1':
        config.firstVersion = true;
        break;
      case 'o':
        config.outputFile = std::string(optarg);
        break;
      case 'v':
        if (config.silent) {
          std::cerr << program << ": silent is true, verbose ignored." << std::endl;
        } else {
          if (config.verbose) {
            config.superverbose = true;
          }
          config.verbose = true;
        }
        break;
      case 's':
        if (config.verbose) {
          std::cerr << program << ": verbose is true, silent ignored." << std::endl;
        } else {
          config.silent = true;
        }
        break;
      case 'h':
        usage(program);
        error = USAGE_MESSAGE;
        break;
      case 'c':
        {
          int count = atoi(optarg);
          if (count < 0) {
            std::cerr << "Number of chains to replace is negative (" << count << "), ignored." << std::endl;
          } else {
            config.chainCount = count;
          }
        }
        break;
      default:
        std::cerr << program << ": bad option " << std::endl;
        error = BAD_ARGUMENTS;
        break;
    }
  }
  if (error) {
    return error;
  } else {
    if (argc < 2) {
      usage(program);
      return USAGE_MESSAGE;
    } else {
      std::string inputFile = std::string(argv[argc - 1]);
      if (config.outputFile.size() == 0) {
        config.outputFile = std::regex_replace(inputFile, std::regex(".dot"), std::string("-chain-1.dot"));
      }
      return compactSingleGraph(inputFile, config);
    }
  }
}

int compactSingleGraph(std::string dotFile1, configuration& config) {
  ScheduleGraph graph1;
  bool parse1 = false;
  int result = -1;
  try {
    config.extraProperties = true;
    boost::dynamic_properties dp1 = setDynamicProperties(graph1, config);
    parse1 = parseSchedule(dotFile1, graph1, dp1, config);
    printSchedule("Graph:", graph1, dp1, config);
  } catch (boost::property_not_found &excep) {
    std::cerr << "Parsing graph: Property not found" << excep.what() << std::endl;
    result = PARSE_ERROR;
  } catch (boost::bad_graphviz_syntax &excep) {
    std::cerr << "Parsing graph: Bad Graphviz syntax: " << excep.what() << std::endl;
    result = PARSE_ERROR_GRAPHVIZ;
  }
  if (parse1) {
    if (config.firstVersion) {
      return compactGraph(graph1, config);
    } else {
      return replaceChain(graph1, config);
    }
  } else {
    return (result == -1) ? FILE_NOT_FOUND : result;
  }
}

void usage(char* program) {
  std::cerr << "Usage: " << program << " <dot file>" << std::endl;
  std::cerr << "Replace chains in the schedule graph with a single vertex." << std::endl;
  std::cerr << "Options: " << std::endl;
  std::cerr << "        -c <n>: optional, replace n chains. Default is to replace all chains." << std::endl;
  std::cerr << "        -h: help and usage." << std::endl;
  std::cerr << "        -s: silent mode, no output, only return code. Usefull for automated tests." << std::endl;
  std::cerr << "        -v: verbose output." << std::endl;
  std::cerr << "        -vv: super verbose, more output than verbose." << std::endl;
  std::cerr << "Return codes: " << std::endl;
  std::cerr << EXIT_SUCCESS << " EXIT_SUCCESS, chains are replaced." << std::endl;
  std::cerr << BAD_ARGUMENTS << " BAD_ARGUMENTS, unknown arguments on command line." << std::endl;
  std::cerr << MISSING_ARGUMENT << " MISSING_ARGUMENT, at least one of the file names is missing." << std::endl;
  std::cerr << FILE_NOT_FOUND << " FILE_NOT_FOUND, one of the dot files not found." << std::endl;
  std::cerr << USAGE_MESSAGE << " USAGE_MESSAGE, usage message displayed." << std::endl;
  std::cerr << PARSE_ERROR << " PARSE_ERROR, error while parsing, unknown tag or attribute." << std::endl;
  std::cerr << PARSE_ERROR_GRAPHVIZ << " PARSE_ERROR_GRAPHVIZ, error while parsing Graphviz syntax." << std::endl;
  std::cerr << "negative values are UNIX signals" << std::endl;
}
