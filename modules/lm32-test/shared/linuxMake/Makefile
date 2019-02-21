

MIAN_MODULE := ../shared_linux.c

SOURCE += $(SCU_LIB_SRC_DIR)/eb_object_transfer.c

CODE_OPTIMIZATION = 2

LIBS += etherbone
DEBUG = 1

CALL_ARGS = "tcp/$(SCU_URL)"

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)
#DEPLOY_DIR  ?= ../deploy_$(CPU)

include $(REPOSITORY_DIR)/makefiles/makefile.scun
#=================================== EOF ======================================
