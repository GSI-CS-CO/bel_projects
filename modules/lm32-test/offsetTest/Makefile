MIAN_MODULE := offsettest.cpp

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

LIBS += boost_system
LIBS += pthread
LIBS += stdc++
LIBS += etherbone

CALL_ARGS = "tcp/$(SCU_URL)"

include $(REPOSITORY_DIR)/makefiles/makefile.scun
#=================================== EOF ======================================
