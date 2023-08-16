
ifeq ($(findstring asl75,$(shell hostname)),asl75)
  # ASL build paths for asl75x
  EBPATH ?=/common/export/timing-rte/tg-fallout-v6.2.x-rocky9/x86_64
  BOOSTPATH =/common/usr/timing/libs_for_generator_fesa/boost_1_69_0/installation
else
ifeq ($(findstring asl,$(shell hostname)),asl)
  # ASL build paths for asl74x
  EBPATH ?=/common/export/timing-rte/tg-fallout-v6.2.0/x86_64
  BOOSTPATH =/common/usr/timing/libs_for_generator_fesa/boost_1_69_0/installation
else
  # this Makefile lives in modules/ftm/tests/singleEdgeTest, thus go down 4 levels.
  EBPATH ?= ../../../../ip_cores/etherbone-core/api/
  BOOSTPATH ?= .
endif
endif

# check that make runs with  sudo rights
RIGHTS := $(shell id -u)

ifeq ($(RIGHTS),0)
  PREFIX ?= /usr/local
else
  PREFIX ?= $$HOME/.local
endif
INSTALLPATH = $(PREFIX)/bin

EBPATH1 ?=../../../ip_cores/etherbone-core/api/

CXX = g++
CXXFLAGS = -g -std=c++11 -fPIC -Wall -I../../include -I$(EBPATH)/include -I../$(EBPATH1) -I$(BOOSTPATH)/include
# for coverage: add to CXXFLAGS: -fprofile-arcs -ftest-coverage

LDFLAGS = -Wl,-rpath,$(BOOSTPATH)/lib,-rpath,../$(EBPATH1)/.libs
LDLIBS = -L../../lib -L../$(EBPATH1)/.libs -L$(BOOSTPATH)/lib -lstdc++ -lboost_serialization -lboost_graph -lboost_regex -ldmtest -lgcov

STYLE = --style="{BasedOnStyle: Google, ColumnLimit: 180}"

.PHONY: clean format coverage doc

singleEdgeTest: SingleEdgeTest
	mv SingleEdgeTest singleEdgeTest

SingleEdgeTest: SingleEdgeGraph.o SingleEdgeTables.o SingleEdgeTest.o SingleEdgeStatus.o

clean:
	rm -f *.o *.gcda *.gcno *.gcov SingleEdgeTest SingleEdgeTest.info

install: singleEdgeTest
	cp singleEdgeTest $(INSTALLPATH)

format:
	clang-format-10 $(STYLE) -i SingleEdgeTest.h
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
