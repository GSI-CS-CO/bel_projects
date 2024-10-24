############################
# ASL build paths          #
############################

ifeq ($(findstring asl75,$(shell hostname)),asl75)
  BOOSTPATH	=/common/usr/timing/libs_for_generator_fesa/boost_1_69_0/installation
else
ifeq ($(findstring asl,$(shell hostname)),asl)
  BOOSTPATH	=/common/usr/timing/libs_for_generator_fesa/boost_1_69_0/installation
endif
endif

CXX = g++
CXXFLAGS = -g -std=c++11 -fPIC -Wall -I../../include -I$(BOOSTPATH)/include

LDFLAGS = -Wl,-rpath,/usr/local/lib,-rpath,$(BOOSTPATH)/lib
LDLIBS = -Wl,-rpath,$(BOOSTPATH)/lib -L$(BOOSTPATH)/lib -lstdc++ -lboost_serialization -lboost_graph -lboost_regex

# check that make runs with  sudo rights
RIGHTS := $(shell id -u)

ifeq ($(RIGHTS),0)
  PREFIX         ?= /usr/local
else
  PREFIX ?= $$HOME/.local
endif
INSTALLPATH     = $(PREFIX)/bin

STYLE = --style="{BasedOnStyle: Google, ColumnLimit: 180}"

.PHONY: clean format doc test

all: scheduleCompare replaceChain

replaceChain: replaceChain.o replaceChainImpl.o scheduleCompact.o printSchedule.o parseSchedule.o ScheduleVertex.o ScheduleEdge.o ScheduleGraph.o

scheduleCompare: scheduleCompare.o scheduleIsomorphism.o printSchedule.o parseSchedule.o ScheduleVertex.o ScheduleEdge.o ScheduleGraph.o

scheduleCompare.o: scheduleIsomorphism.h scheduleCompare.h configuration.h

scheduleIsomorphism.o: scheduleIsomorphism.h scheduleCompare.h printSchedule.h parseSchedule.h ScheduleVertex.h ScheduleEdge.h scheduleCompact.h

printSchedule.o: printSchedule.h scheduleCompare.h scheduleIsomorphism.h

parseSchedule.o: parseSchedule.h scheduleCompare.h scheduleIsomorphism.h ScheduleVertex.h

ScheduleVertex.o: ScheduleVertex.h

ScheduleEdge.o: ScheduleEdge.h

ScheduleGraph.o: ScheduleGraph.h

scheduleCompact.o: scheduleCompact.h scheduleCompare.h scheduleIsomorphism.h printSchedule.h

replaceChain.o: replaceChain.h configuration.h

replaceChainImpl.o: replaceChainImpl.h replaceChain.h configuration.h

install: scheduleCompare replaceChain
	cp scheduleCompare $(INSTALLPATH)
	cp replaceChain $(INSTALLPATH)

clean:
	rm -f *.o *.gcda *.gcno *.gcov scheduleCompare replaceChain

format:
	for i in *.h   ; do clang-format-10 $(STYLE) -i $$i; done
	for i in *.cpp ; do clang-format-10 $(STYLE) -i $$i; done

doc:
	$(MAKE) -C ../doc/

test:
	$(MAKE) scheduleCompare replaceChain
	$(MAKE) -C ../test/
