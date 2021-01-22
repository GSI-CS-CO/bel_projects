TOP ?= /home/martin/bel_projects/dev
FTMPATH ?= $(TOP)/modules/ftm

CXX = g++
CXXFLAGS = -g -std=c++11 -fPIC -Wall -I$(FTMPATH)/include
# -fprofile-arcs -ftest-coverage

LDFLAGS = -Wl,-rpath,/usr/local/lib
LDLIBS = -L$(FTMPATH)/lib -lstdc++ -lboost_serialization -lboost_graph -lcarpedm
# -lgcov

STYLE = --style="{BasedOnStyle: Google, ColumnLimit: 180}"

.PHONY: clean format doc

scheduleCompare: scheduleCompare.o scheduleIsomorphism.o printSchedule.o parseSchedule.o

scheduleCompare.cpp: scheduleIsomorphism.h

clean:
	rm -f *.o *.gcda *.gcno *.gcov scheduleCompare

format:
	clang-format-10 $(STYLE) -i scheduleCompare.cpp
	clang-format-10 $(STYLE) -i scheduleIsomorphism.cpp
	clang-format-10 $(STYLE) -i printSchedule.cpp
	clang-format-10 $(STYLE) -i parseSchedule.cpp
	clang-format-10 $(STYLE) -i scheduleCompare.h
	clang-format-10 $(STYLE) -i scheduleIsomorphism.h
	clang-format-10 $(STYLE) -i printSchedule.h
	clang-format-10 $(STYLE) -i parseSchedule.h

doc: