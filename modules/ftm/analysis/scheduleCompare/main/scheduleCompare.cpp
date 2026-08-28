#include "scheduleCompare.h"

#include <stdio.h>
#include <unistd.h>

#include "scheduleIsomorphism.h"

int main(int argc, char* argv[]) {
  int error = 0;
  int opt;
  char* program = argv[0];
  configuration config;
  while ((opt = getopt(argc, argv, "chnstuvV")) != -1) {
    switch (opt) {
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
      case 'V':
        version(program);
        error = VERSION_MESSAGE;
        break;
      case 'n':
        config.compareNames = false;
        break;
      case 'c':
        config.check = true;
        break;
      case 'u':
        config.undefinedAsEmpty = true;
        break;
      case 't':
        config.test = true;
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
      if (config.test) {
        return testSingleGraph(std::string(argv[argc - 1]), config);
      } else {
        // use the last two arguments for the dot files after getopt permuted the arguments.
        return scheduleIsomorphic(std::string(argv[argc - 2]), std::string(argv[argc - 1]), config);
      }
    }
  }
}

void usage(char* program) {
  std::cerr << "Usage: " << program << " <dot file 1> <dot file 2>" << std::endl;
  std::cerr << "Checks that graphs in <dot file 1> and <dot file 2> are isomorphic, i.e. describe the same schedule." << std::endl;
  std::cerr << "Options: " << std::endl;
  std::cerr << "        -c: check dot syntax (stops parsing on all unknown attributes)." << std::endl;
  std::cerr << "        -h: help and usage." << std::endl;
  std::cerr << "        -n: do not compare names of vertices. Not applicable with option -t." << std::endl;
  std::cerr << "        -s: silent mode, no output, only return code. Usefull for automated tests." << std::endl;
  std::cerr << "        -t: test a single graph: compare each vertex with itself. This tests the vertex comparator." << std::endl;
  std::cerr << "        -u: handle attribute value 'undefined' as an empty string." << std::endl;
  std::cerr << "        -v: verbose output." << std::endl;
  std::cerr << "        -vv: super verbose, more output than verbose." << std::endl;
  std::cerr << "        -V: print version and exit." << std::endl;
  std::cerr << "Return codes: " << std::endl;
  std::cerr << EXIT_SUCCESS << " EXIT_SUCCESS, graphs are isomorphic." << std::endl;
  std::cerr << NOT_ISOMORPHIC << " NOT_ISOMORPHIC, graphs are not isomorphic." << std::endl;
  std::cerr << SUBGRAPH_ISOMORPHIC << " SUBGRAPH_ISOMORPHIC, graph is isomorphic to a subgraph of the larger graph." << std::endl;
  std::cerr << BAD_ARGUMENTS << " BAD_ARGUMENTS, unknown arguments on command line." << std::endl;
  std::cerr << MISSING_ARGUMENT << " MISSING_ARGUMENT, at least one of the file names is missing." << std::endl;
  std::cerr << FILE_NOT_FOUND << " FILE_NOT_FOUND, one of the dot files not found." << std::endl;
  std::cerr << USAGE_MESSAGE << " USAGE_MESSAGE, usage message displayed." << std::endl;
  std::cerr << PARSE_ERROR << " PARSE_ERROR, error while parsing, unknown tag or attribute." << std::endl;
  std::cerr << PARSE_ERROR_GRAPHVIZ << " PARSE_ERROR_GRAPHVIZ, error while parsing Graphviz syntax." << std::endl;
  std::cerr << TEST_SUCCESS << " TEST_SUCCESS, test a single graph with success." << std::endl;
  std::cerr << TEST_FAIL << " TEST_FAIL, test a single graph with failure." << std::endl;
  std::cerr << VERSION_MESSAGE << " VERSION_MESSAGE, version displayed." << std::endl;
  std::cerr << "negative values are UNIX signals" << std::endl;
}

void version(char* program) {
  std::cerr << program << ", version 1.0.1" << std::endl;
}
