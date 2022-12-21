############################
# ASL build paths          #
############################

ifeq ($(findstring asl75,$(shell hostname)),asl75)
  EBPATH =/common/export/timing-rte/tg-fallout-v6.2.x-rocky8/x86_64
  BOOSTPATH =/common/usr/timing/libs_for_generator_fesa/boost_1_69_0/installation
else
ifeq ($(findstring asl,$(shell hostname)),asl)
  EBPATH =/common/export/timing-rte/tg-fallout-v6.2.0/x86_64
  BOOSTPATH =/common/usr/timing/libs_for_generator_fesa/boost_1_69_0/installation
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

EBPATH1 ?=/usr/local/include

CXX = g++
CXXFLAGS = -g -std=c++11 -fPIC -Wall -I../../include -I$(EBPATH)/include -I$(EBPATH1) -I$(BOOSTPATH)/include -fprofile-arcs -ftest-coverage

LDFLAGS = -Wl,-rpath,/usr/local/lib,-rpath,$(BOOSTPATH)/lib
LDLIBS = -L../../lib -L$(BOOSTPATH)/lib -lstdc++ -lboost_serialization -lboost_graph -lboost_regex -ldmtest -lgcov

STYLE = --style="{BasedOnStyle: Google, ColumnLimit: 180}"

.PHONY: clean format coverage doc

SingleEdgeTest: SingleEdgeGraph.o SingleEdgeTables.o SingleEdgeTest.o SingleEdgeStatus.o

clean:
	rm -f *.o *.gcda *.gcno *.gcov SingleEdgeTest SingleEdgeTest.info

install: SingleEdgeTest
	cp SingleEdgeTest $(INSTALLPATH)

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
