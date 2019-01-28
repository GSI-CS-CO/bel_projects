
MIAN_MODULE := stackoverflow.c

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

CFLAGS += -fstack-protector-all
CODE_OPTIMIZATION = 0

DEFINES += MICO32_FULL_CONTEXT_SAVE_RESTORE

include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
