

MIAN_MODULE := ../helloworld.c

# TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)
 # TOOLCHAIN_DIR = /home/bel/ubecker/lnx/src/extern/lm32gccRubini/lm32-gcc-8.2.0/bin/
CODE_OPTIMIZATION = 2
# CFLAGS += -fstack-protector

NO_LTO=1

REPOSITORY_DIR = $(shell git rev-parse --show-toplevel)
DEPLOY_DIR  ?= ../deploy_$(CPU)


include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
