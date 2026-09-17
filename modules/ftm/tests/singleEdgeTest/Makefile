
ifeq ($(findstring asl75,$(shell hostname)),asl75)
  # ASL build paths for asl75x
  EBPATH ?=/common/export/timing-rte/tg-fallout-v6.3.1-rocky9
else
ifeq ($(findstring asl,$(shell hostname)),asl)
  # ASL build paths for asl74x
  EBPATH ?=/common/export/timing-rte/tg-fallout-v6.2.0/x86_64
else
  # this Makefile lives in modules/ftm/tests/singleEdgeTest, thus go down 4 levels.
  EBPATH ?= ../../../../ip_cores/etherbone-core/api/
endif
endif

EBPATH1 ?=../../../../ip_cores/etherbone-core/api/
BOOSTPATH ?= .

# check that make runs with  sudo rights
RIGHTS := $(shell id -u)

ifeq ($(RIGHTS),0)
  PREFIX ?= /usr/local
else
  PREFIX ?= $$HOME/.local
endif
INSTALLPATH = $(PREFIX)/bin

# assume that Boost is installed on the maschine. Use BOOSTPATH if the global installed version should not be used.
# set BOOST_FLAGS depending on the Boost version
ifeq (,$(wildcard $(BOOSTPATH)/include/boost/version.hpp))
  BOOST_VERSION:=$(shell grep 'BOOST_LIB_VERSION "' /usr/include/boost/version.hpp | cut -b28-31)
else
  BOOST_VERSION:=$(shell grep 'BOOST_LIB_VERSION "' $(BOOSTPATH)/include/boost/version.hpp | cut -b28-31)
endif
ifeq ($(BOOST_VERSION),1_75)
  BOOST_FLAGS=-DBOOST_ALLOW_DEPRECATED_HEADERS -Wno-nonnull
endif

CXX ?= g++
CXXFLAGS = -g -std=c++11 -fPIC -Wall -I../../include -I$(EBPATH)/include -I../$(EBPATH1) -I$(BOOSTPATH)/include $(BOOST_FLAGS)
# for coverage: add to CXXFLAGS: -fprofile-arcs -ftest-coverage

# LDFLAGS = -Wl,-rpath,$(BOOSTPATH)/lib,-rpath,../$(EBPATH1)/.libs
LDLIBS = -L../../lib -L../$(EBPATH1)/.libs -L$(BOOSTPATH)/lib -lstdc++ -lboost_serialization -lboost_graph -lboost_regex -ldmtest
# for coverage: add to LDLIBS: -lgcov

STYLE = --style="{BasedOnStyle: Google, ColumnLimit: 180}"

.PHONY: clean format coverage doc

singleEdgeTest: SingleEdgeGraph.o SingleEdgeTables.o SingleEdgeTest.o SingleEdgeStatus.o
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	rm -f *.o *.gcda *.gcno *.gcov SingleEdgeTest singleEdgeTest SingleEdgeTest.info

install: singleEdgeTest
	cp singleEdgeTest $(INSTALLPATH)

format:
	clang-format-10 $(STYLE) -i SingleEdgeTest.h
	clang-format-10 $(STYLE) -i configuration.h
	clang-format-10 $(STYLE) -i SingleEdgeTest.cpp
	clang-format-10 $(STYLE) -i SingleEdgeTables.h
	clang-format-10 $(STYLE) -i SingleEdgeTables.cpp
	clang-format-10 $(STYLE) -i SingleEdgeGraph.h
	clang-format-10 $(STYLE) -i SingleEdgeGraph.cpp
	clang-format-10 $(STYLE) -i SingleEdgeStatus.h
	clang-format-10 $(STYLE) -i SingleEdgeStatus.cpp

coverage:
	lcov --zerocounters --directory . --directory ../../ftmx86
	./SingleEdgeTest
	lcov --capture --directory . --directory ../../ftmx86 --output-file SingleEdgeTest.info
	genhtml SingleEdgeTest.info --output-directory coverage/

doc:
	doxygen SingleEdgeTest_doxy.conf
