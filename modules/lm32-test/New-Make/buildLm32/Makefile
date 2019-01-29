

MIAN_MODULE := ../helloworld.c

# TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)

CODE_OPTIMIZATION = 2
CFLAGS += -fstack-protector

REPOSITORY_DIR = $(shell git rev-parse --show-toplevel)
DEPLOY_DIR  ?= ../deploy_$(CPU)



include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
