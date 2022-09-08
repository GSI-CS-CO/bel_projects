############################
# ASL build paths          #
############################

ifeq ($(findstring asl75,$(shell hostname)),asl75)
  BOOSTPATH	=/common/usr/timing/libs_for_generator_fesa/boost_1_69_0/installation
  GRAPH_LIB	=$(BOOSTPATH)/lib/libboost_graph.so
  SER_LIB	=$(BOOSTPATH)/lib/libboost_serialization.so
  RE_LIB	=$(BOOSTPATH)/lib/libboost_regex.so
else
ifeq ($(findstring asl,$(shell hostname)),asl)
  BOOSTPATH	=/common/usr/timing/libs_for_generator_fesa/boost_1_69_0/installation
  GRAPH_LIB	=$(BOOSTPATH)/lib/libboost_graph.so
  SER_LIB	=$(BOOSTPATH)/lib/libboost_serialization.so
  RE_LIB	=$(BOOSTPATH)/lib/libboost_regex.so
endif
endif

SER_LIB		?=-lboost_serialization
GRAPH_LIB	?=-lboost_graph
RE_LIB		?=-lboost_regex


CXX = g++
CXXFLAGS = -g -std=c++11 -fPIC -Wall -I../../include -I$(BOOSTPATH)/include

LDFLAGS = -Wl,-rpath,/usr/local/lib,-rpath,$(BOOSTPATH)/lib
LDLIBS = -L../../lib -lstdc++ $(SER_LIB) $(GRAPH_LIB) $(RE_LIB)

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

scheduleCompare: scheduleCompare.o scheduleIsomorphism.o printSchedule.o parseSchedule.o ScheduleVertex.o ScheduleEdge.o

scheduleCompare.o: scheduleIsomorphism.h scheduleCompare.h

scheduleIsomorphism.o: scheduleIsomorphism.h scheduleCompare.h printSchedule.h parseSchedule.h ScheduleVertex.h ScheduleEdge.h

printSchedule.o: printSchedule.h scheduleCompare.h scheduleIsomorphism.h

parseSchedule.o: parseSchedule.h scheduleCompare.h scheduleIsomorphism.h ScheduleVertex.h

ScheduleVertex.o: ScheduleVertex.h

ScheduleEdge.o: ScheduleEdge.h

install: scheduleCompare
	cp scheduleCompare $(INSTALLPATH)

clean:
	rm -f *.o *.gcda *.gcno *.gcov scheduleCompare

format:
	for i in *.h   ; do clang-format-10 $(STYLE) -i $$i; done
	for i in *.cpp ; do clang-format-10 $(STYLE) -i $$i; done

doc:
	$(MAKE) -C ../doc/

test:
	$(MAKE) scheduleCompare
	$(MAKE) -C ../test/
