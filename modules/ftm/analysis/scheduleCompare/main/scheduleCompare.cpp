#include "scheduleCompare.h"

#include <stdio.h>
#include <unistd.h>

#include "scheduleIsomorphism.h"

const int BAD_ARGUMENTS = -3;

int main(int argc, char* argv[]) {
  int error = 0;
  int opt;
  char* program = argv[0];
  configuration config;
  if (argc < 3) {
    usage(program);
    return BAD_ARGUMENTS;
  } else {
    while ((opt = getopt(argc, argv, "vsh")) != -1) {
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
          error = EXIT_SUCCESS;
        default:
          std::cerr << program << ": bad option " << argv[optind] << std::endl;
          error = BAD_ARGUMENTS;
      }
    }
    if (error) {
      return error;
    } else {
      // use the last two arguments for the dot files after getopt permuted the arguments.
      return scheduleIsomorphic(std::string(argv[argc - 2]), std::string(argv[argc - 1]), config);
    }
  }
}

void usage(char* program) {
  std::cerr << "Usage: " << program << " <dot file 1> <dot file 2>" << std::endl;
  std::cerr << "Checks that graphs in <dot file 1> and <dot file 2> are isomorphic, i.e. describe the same schedule." << std::endl;
  std::cerr << "Options: " << std::endl;
  std::cerr << "        -h: help and usage." << std::endl;
  std::cerr << "        -v: verbose output." << std::endl;
  std::cerr << "        -vv: super verbose, more output than verbose." << std::endl;
  std::cerr << "        -s: silent mode, no output." << std::endl;
}
